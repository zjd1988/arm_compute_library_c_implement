#include "NENonLinearFilter.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#if (defined _WIN32) || (defined _WIN64)

#include "NEON_2_SSE.h"

#else

#include <arm_neon.h>

#endif

#define BATCH_EIGHT 8
#define BATCH_FOUR 4

/***************************************************************************************************
*
* median filter 3x3 disk mask
*               F T F
*               T T T
*               F T F
*
***************************************************************************************************/
median_filter_3x3_func median_filter_disk_3x3_u8_non_opt_use_softnet = median_filter_cross_3x3_u8_non_opt_use_softnet;

median_filter_3x3_func median_filter_disk_3x3_u8_opt_with_neon_use_softnet = median_filter_cross_3x3_u8_opt_with_neon_use_softnet;


static void sort_neon(uint8x8_t *a, uint8x8_t *b)
{
	const uint8x8_t min = vmin_u8(*a, *b);
	const uint8x8_t max = vmax_u8(*a, *b);
	*a = min;
	*b = max;
}

static void sort5_neon(uint8x8_t *p0, uint8x8_t *p1, uint8x8_t *p2, uint8x8_t *p3, uint8x8_t *p4)
{
	sort_neon(p0, p1);
	sort_neon(p2, p3);
	sort_neon(p0, p2);
	sort_neon(p1, p3);
	sort_neon(p1, p2);
	sort_neon(p0, p4);
	sort_neon(p1, p4);
	sort_neon(p2, p4);
}

static void sort9_neon(uint8x8_t *p0, uint8x8_t *p1, uint8x8_t *p2,
	uint8x8_t *p3, uint8x8_t *p4, uint8x8_t *p5,
	uint8x8_t *p6, uint8x8_t *p7, uint8x8_t *p8)
{
	sort_neon(p1, p2);
	sort_neon(p4, p5);
	sort_neon(p7, p8);
	sort_neon(p0, p1);
	sort_neon(p3, p4);
	sort_neon(p6, p7);
	sort_neon(p1, p2);
	sort_neon(p4, p5);
	sort_neon(p7, p8);
	sort_neon(p0, p3);
	sort_neon(p5, p8);
	sort_neon(p4, p7);
	sort_neon(p3, p6);
	sort_neon(p1, p4);
	sort_neon(p2, p5);
	sort_neon(p4, p7);
	sort_neon(p4, p2);
	sort_neon(p6, p4);
	sort_neon(p4, p2);
}

static void sort(uint8_t *a, uint8_t *b)
{
	const uint8_t min_value = (*a < *b ? *a : *b);
	const uint8_t max_value = (*a > *b ? *a : *b);
	*a = min_value;
	*b = max_value;
}

// Sorting networks below were generated using http://pages.ripco.net/~jgamble/nw.html
// Calculations that do not affect the median were removed.
inline void sort5(uint8_t *p0, uint8_t *p1, uint8_t *p2, uint8_t *p3, uint8_t *p4)
{
	sort(p0, p1);
	sort(p2, p3);
	sort(p0, p2);
	sort(p1, p3);
	sort(p1, p2);
	sort(p0, p4);
	sort(p1, p4);
	sort(p2, p4);
}

static void sort9(uint8_t *p0, uint8_t *p1, uint8_t *p2,
	uint8_t *p3, uint8_t *p4, uint8_t *p5,
	uint8_t *p6, uint8_t *p7, uint8_t *p8)
{
	sort(p1, p2);
	sort(p4, p5);
	sort(p7, p8);
	sort(p0, p1);
	sort(p3, p4);
	sort(p6, p7);
	sort(p1, p2);
	sort(p4, p5);
	sort(p7, p8);
	sort(p0, p3);
	sort(p5, p8);
	sort(p4, p7);
	sort(p3, p6);
	sort(p1, p4);
	sort(p2, p5);
	sort(p4, p7);
	sort(p4, p2);
	sort(p6, p4);
	sort(p4, p2);
}


static void padding_boder(uint8_t * filter_in, int height, int width, int channel_num, uint8_t * padding_filter_in,
	BORDER_FLAG border_flag, int border_size, int element_size)
{
	int row_index = 0;
	int col_index = 0;
	int row_step_with_border = (width + 2 * border_size) * channel_num * element_size;
	int row_step_without_border = width * channel_num * element_size;
	int col_step = height + 2 * border_size;
	int border_bytes = border_size * channel_num * element_size;
	if (BODER_NON == border_flag)
	{
		row_step_with_border = width * channel_num * element_size;
		col_step = height;
	}

	if (BODER_ZERO == border_flag)
	{
		uint8_t * src = filter_in;
		uint8_t * dst = padding_filter_in;
		for (row_index = 0; row_index < border_size; row_index++, dst += row_step_with_border)
		{
			memset(dst, 0, row_step_with_border);
		}
		for (; row_index < height + border_size; row_index++, src += row_step_without_border, dst += row_step_with_border)
		{
			col_index = 0;
			memset(dst + col_index, 0, border_bytes);

			col_index += border_bytes;
			memcpy(dst + col_index, src, row_step_without_border);

			col_index += row_step_without_border;
			memset(dst + col_index, 0, border_bytes);
		}
		for (; row_index < col_step; row_index++, dst += row_step_with_border)
		{
			memset(dst, 0, row_step_with_border);
		}
	}
	else if (BODER_REPLICATE == border_flag)
	{

		uint8_t *src = filter_in;
		uint8_t *dst = padding_filter_in;
		for (row_index = 0; row_index < border_size; row_index++, dst += row_step_with_border)
		{
			memcpy(dst + border_bytes, src, row_step_without_border);
		}

		for (; row_index < height + border_size; row_index++, src += row_step_without_border, dst += row_step_with_border)
		{
			memcpy(dst + border_bytes, src, row_step_without_border);
		}

		src -= row_step_without_border;
		for (; row_index < col_step; row_index++, dst += row_step_with_border)
		{
			memcpy(dst + border_bytes, src, row_step_without_border);
		}

		
		if (element_size == sizeof(uint8_t))
		{
			uint8_t *temp_dst = (uint8_t *)padding_filter_in;
			for (row_index = 0; row_index < col_step; row_index++, temp_dst += row_step_with_border)
			{
				for (col_index = 0; col_index < border_size; col_index++)
				{
					temp_dst[col_index] = temp_dst[border_size];
					temp_dst[col_index + border_size + width] = temp_dst[border_size + width - 1];
				}
			}
		}
		else if(element_size == sizeof(uint16_t))
		{
			uint16_t *temp_dst = (uint16_t *)padding_filter_in;
			for (row_index = 0; row_index < col_step; row_index++, temp_dst += row_step_with_border)
			{
				for (col_index = 0; col_index < border_size; col_index++)
				{
					temp_dst[col_index] = temp_dst[border_size];
					temp_dst[col_index + border_size + width] = temp_dst[border_size + width - 1];
				}
			}
		}
		else
		{
			uint32_t *temp_dst = (uint32_t *)padding_filter_in;
			for (row_index = 0; row_index < col_step; row_index++, temp_dst += row_step_with_border)
			{
				for (col_index = 0; col_index < border_size; col_index++)
				{
					temp_dst[col_index] = temp_dst[border_size];
					temp_dst[col_index + border_size + width] = temp_dst[border_size + width - 1];
				}
			}
		}

	}
	else
	{

		uint8_t * dst = padding_filter_in;
		for (row_index = 0; row_index < border_size; row_index++, dst += row_step_with_border)
		{
			memset(dst, 0, row_step_with_border);
		}

		for (; row_index < col_step - border_size; row_index++, dst += row_step_with_border)
		{
			for (col_index = 0; col_index < border_bytes; col_index++)
			{
				dst[col_index] = 0;
				dst[row_step_with_border - col_index] = 0;
			}
		}
		for (; row_index < col_step; row_index++, dst += row_step_with_border)
		{
			memset(dst, 0, row_step_with_border);
		}
	}

}

/*********************************************************************************************
* para filter_in : input image ,must be uint8_t type
* para height:     input image height
* para width:      input image width
* para channel_num:channel number
* para filter_out: output filtered image,must be uint8_t type
* note:            filter_out size is same as filter_in
* 
* para border_flag: BODER_ZERO ---padding zeros
*                   BODER_REPLICATE ---- padding replicate
*                   BODER_NON ---- padding non
*
* BODER_ZERO: filter_in padding with zeros
*           0 0 0 0
*           0 x x x
*           0 x x x
*           
* BODER_REPLICATE: filter_in first padding with replicate
*             |
*             v
*           a a c e
*       --> a a c e
*           b b d f
*
* BODER_NON: filter_in padding none
*
* median filter 3x3 box mask
*               T T T
*               T T T
*               T T T
*
**********************************************************************************************/
int median_filter_box_3x3_u8_opt_with_neon_use_softnet(uint8_t * filter_in, uint32_t height, uint32_t width, 
	uint8_t channel_num, uint8_t * filter_out, BORDER_FLAG border_flag)
{
	uint8_t *padding_filter_in_align = NULL;
	int row_index = 0;
	int col_index = 0;
	int border_size = 1;
	//filter body
	int col_end_index = 0;
	int row_end_index = 0;
	int row_step = 0;
	int filter_offset_flag = 0;
	uint8x16_t top_data;
	uint8x16_t mid_data;
	uint8x16_t bot_data;
	uint8x8_t p0_u8x8;
	uint8x8_t p1_u8x8;
	uint8x8_t p2_u8x8;
	uint8x8_t p3_u8x8;
	uint8x8_t p4_u8x8;
	uint8x8_t p5_u8x8;
	uint8x8_t p6_u8x8;
	uint8x8_t p7_u8x8;
	uint8x8_t p8_u8x8;

	uint8_t p0_u8;
	uint8_t p1_u8;
	uint8_t p2_u8;
	uint8_t p3_u8;
	uint8_t p4_u8;
	uint8_t p5_u8;
	uint8_t p6_u8;
	uint8_t p7_u8;
	uint8_t p8_u8;

#ifdef USE_CLOCK
	start = clock();
#endif
	if (NULL == filter_in || NULL == filter_out)
		return -1;

	if (height < 3 || width < 3)
		return -1;

	if (channel_num != 1 && channel_num != 3)
		return -1;

	if (border_flag != BODER_NON && border_flag != BODER_REPLICATE && border_flag != BODER_ZERO)
		border_flag = BODER_NON;

	//padding boder
	if (BODER_ZERO == border_flag || BODER_REPLICATE == border_flag)
	{
		int malloc_bytes = ((height + 2 * border_size) * (width + 2 * border_size) * sizeof(uint8_t) + ALIGN_SIXTEEN_BYTES) * channel_num;
		padding_filter_in_align = (uint8_t *)align_malloc(malloc_bytes, ALIGN_SIXTEEN_BYTES);

		padding_boder(filter_in, height, width, channel_num, padding_filter_in_align, border_flag, border_size, sizeof(uint8_t));
	}
	else
	{
		padding_filter_in_align = filter_in;
		padding_boder(filter_in, height, width, channel_num, filter_out, border_flag, border_size, sizeof(uint8_t));
	}
	if (BODER_NON != border_flag)
	{
		row_end_index = height;
		col_end_index = width;
	}
	else
	{
		row_end_index = height - 2;
		col_end_index = width - 2;
	}
	row_step = (width + 2 * border_size * ((BODER_NON != border_flag) ? 1 : 0)) * channel_num;
	filter_offset_flag = ((BODER_NON == border_flag) ? 1 : 0);

	if (channel_num == 1)
	{
#ifdef USE_OPENMP
#pragma omp parallel for
#endif
		//filter body
		for (row_index = 0; row_index < row_end_index; row_index++)
		{
			uint8_t * input_top_ptr = padding_filter_in_align + row_index * row_step;
			uint8_t * input_mid_ptr = padding_filter_in_align + (row_index + 1) * row_step;
			uint8_t * input_bot_ptr = padding_filter_in_align + (row_index + 2) * row_step;
			uint8_t * output_ptr = filter_out + row_index * width * channel_num + filter_offset_flag * channel_num;
			for (col_index = 0; col_index + BATCH_EIGHT <= col_end_index; col_index += BATCH_EIGHT)
			{
				top_data = vld1q_u8(input_top_ptr + col_index * channel_num);
				mid_data = vld1q_u8(input_mid_ptr + col_index * channel_num);
				bot_data = vld1q_u8(input_bot_ptr + col_index * channel_num);

				p0_u8x8 = vget_low_u8(top_data);
				p1_u8x8 = vext_u8(vget_low_u8(top_data), vget_high_u8(top_data), 1);
				p2_u8x8 = vext_u8(vget_low_u8(top_data), vget_high_u8(top_data), 2);
				p3_u8x8 = vget_low_u8(mid_data);
				p4_u8x8 = vext_u8(vget_low_u8(mid_data), vget_high_u8(mid_data), 1);
				p5_u8x8 = vext_u8(vget_low_u8(mid_data), vget_high_u8(mid_data), 2);
				p6_u8x8 = vget_low_u8(bot_data);
				p7_u8x8 = vext_u8(vget_low_u8(bot_data), vget_high_u8(bot_data), 1);
				p8_u8x8 = vext_u8(vget_low_u8(bot_data), vget_high_u8(bot_data), 2);

				sort9_neon(&p0_u8x8, &p1_u8x8, &p2_u8x8, &p3_u8x8, &p4_u8x8, &p5_u8x8, &p6_u8x8, &p7_u8x8, &p8_u8x8);

				vst1_u8(output_ptr, p4_u8x8);
				output_ptr += BATCH_EIGHT;
			}
			input_top_ptr += (col_index * channel_num);
			input_mid_ptr += (col_index * channel_num);
			input_bot_ptr += (col_index * channel_num);
			for (; col_index < col_end_index; col_index++)
			{
				p0_u8 = input_top_ptr[0];
				p1_u8 = input_top_ptr[1];
				p2_u8 = input_top_ptr[2];
				p3_u8 = input_mid_ptr[0];
				p4_u8 = input_mid_ptr[1];
				p5_u8 = input_mid_ptr[2];
				p6_u8 = input_bot_ptr[0];
				p7_u8 = input_bot_ptr[1];
				p8_u8 = input_bot_ptr[2];

				sort9(&p0_u8, &p1_u8, &p2_u8, &p3_u8, &p4_u8, &p5_u8, &p6_u8, &p7_u8, &p8_u8);

				*output_ptr++ = p4_u8;
				input_top_ptr += channel_num;
				input_mid_ptr += channel_num;
				input_bot_ptr += channel_num;
			}
		}
	}
	else
	{
#ifdef USE_OPENMP
#pragma omp parallel for
#endif
		for (row_index = 0; row_index < row_end_index; row_index++)
		{
			uint8_t * input_top_ptr = padding_filter_in_align + row_index * row_step;
			uint8_t * input_mid_ptr = padding_filter_in_align + (row_index + 1) * row_step;
			uint8_t * input_bot_ptr = padding_filter_in_align + (row_index + 2) * row_step;
			uint8_t * output_ptr = filter_out + row_index * width * channel_num + filter_offset_flag * channel_num;

			for (col_index = 0; col_index + BATCH_EIGHT <= col_end_index; col_index += BATCH_EIGHT)
			{
				const uint8x16x3_t top_data = vld3q_u8(input_top_ptr + col_index * channel_num);
				const uint8x16x3_t mid_data = vld3q_u8(input_mid_ptr + col_index * channel_num);
				const uint8x16x3_t bot_data = vld3q_u8(input_bot_ptr + col_index * channel_num);
				uint8x8x3_t out_data;
				//first channel 
				uint8x8_t p0_0 = vget_low_u8(top_data.val[0]);
				uint8x8_t p1_0 = vext_u8(vget_low_u8(top_data.val[0]), vget_high_u8(top_data.val[0]), 1);
				uint8x8_t p2_0 = vext_u8(vget_low_u8(top_data.val[0]), vget_high_u8(top_data.val[0]), 2);
				uint8x8_t p3_0 = vget_low_u8(mid_data.val[0]);
				uint8x8_t p4_0 = vext_u8(vget_low_u8(mid_data.val[0]), vget_high_u8(mid_data.val[0]), 1);
				uint8x8_t p5_0 = vext_u8(vget_low_u8(mid_data.val[0]), vget_high_u8(mid_data.val[0]), 2);
				uint8x8_t p6_0 = vget_low_u8(bot_data.val[0]);
				uint8x8_t p7_0 = vext_u8(vget_low_u8(bot_data.val[0]), vget_high_u8(bot_data.val[0]), 1);
				uint8x8_t p8_0 = vext_u8(vget_low_u8(bot_data.val[0]), vget_high_u8(bot_data.val[0]), 2);

				sort9_neon(&p0_0, &p1_0, &p2_0, &p3_0, &p4_0, &p5_0, &p6_0, &p7_0, &p8_0);
				out_data.val[0] = p4_0;
				///secone channel
				uint8x8_t p0_1 = vget_low_u8(top_data.val[1]);
				uint8x8_t p1_1 = vext_u8(vget_low_u8(top_data.val[1]), vget_high_u8(top_data.val[1]), 1);
				uint8x8_t p2_1 = vext_u8(vget_low_u8(top_data.val[1]), vget_high_u8(top_data.val[1]), 2);
				uint8x8_t p3_1 = vget_low_u8(mid_data.val[1]);
				uint8x8_t p4_1 = vext_u8(vget_low_u8(mid_data.val[1]), vget_high_u8(mid_data.val[1]), 1);
				uint8x8_t p5_1 = vext_u8(vget_low_u8(mid_data.val[1]), vget_high_u8(mid_data.val[1]), 2);
				uint8x8_t p6_1 = vget_low_u8(bot_data.val[1]);
				uint8x8_t p7_1 = vext_u8(vget_low_u8(bot_data.val[1]), vget_high_u8(bot_data.val[1]), 1);
				uint8x8_t p8_1 = vext_u8(vget_low_u8(bot_data.val[1]), vget_high_u8(bot_data.val[1]), 2);

				sort9_neon(&p0_1, &p1_1, &p2_1, &p3_1, &p4_1, &p5_1, &p6_1, &p7_1, &p8_1);
				out_data.val[1] = p4_1;

				///third channel
				uint8x8_t p0_2 = vget_low_u8(top_data.val[2]);
				uint8x8_t p1_2 = vext_u8(vget_low_u8(top_data.val[2]), vget_high_u8(top_data.val[2]), 1);
				uint8x8_t p2_2 = vext_u8(vget_low_u8(top_data.val[2]), vget_high_u8(top_data.val[2]), 2);
				uint8x8_t p3_2 = vget_low_u8(mid_data.val[2]);
				uint8x8_t p4_2 = vext_u8(vget_low_u8(mid_data.val[2]), vget_high_u8(mid_data.val[2]), 1);
				uint8x8_t p5_2 = vext_u8(vget_low_u8(mid_data.val[2]), vget_high_u8(mid_data.val[2]), 2);
				uint8x8_t p6_2 = vget_low_u8(bot_data.val[2]);
				uint8x8_t p7_2 = vext_u8(vget_low_u8(bot_data.val[2]), vget_high_u8(bot_data.val[2]), 1);
				uint8x8_t p8_2 = vext_u8(vget_low_u8(bot_data.val[2]), vget_high_u8(bot_data.val[2]), 2);

				sort9_neon(&p0_2, &p1_2, &p2_2, &p3_2, &p4_2, &p5_2, &p6_2, &p7_2, &p8_2);
				out_data.val[2] = p4_2;

				vst3_u8(output_ptr, out_data);
				output_ptr += (channel_num*BATCH_EIGHT);
			}
			input_top_ptr += col_index * channel_num;
			input_mid_ptr += col_index * channel_num;
			input_bot_ptr += col_index * channel_num;
			for (; col_index < col_end_index; col_index++)
			{
				uint8_t p0_0 = input_top_ptr[0];					
				uint8_t p0_1 = input_top_ptr[1];
				uint8_t p0_2 = input_top_ptr[2];
				uint8_t p1_0 = input_top_ptr[3];
				uint8_t p1_1 = input_top_ptr[4];
				uint8_t p1_2 = input_top_ptr[5];
				uint8_t p2_0 = input_top_ptr[6];
				uint8_t p2_1 = input_top_ptr[7];
				uint8_t p2_2 = input_top_ptr[8];
				uint8_t p3_0 = input_mid_ptr[0];
				uint8_t p3_1 = input_mid_ptr[1];
				uint8_t p3_2 = input_mid_ptr[2];
				uint8_t p4_0 = input_mid_ptr[3];
				uint8_t p4_1 = input_mid_ptr[4];
				uint8_t p4_2 = input_mid_ptr[5];
				uint8_t p5_0 = input_mid_ptr[6];
				uint8_t p5_1 = input_mid_ptr[7];
				uint8_t p5_2 = input_mid_ptr[8];
				uint8_t p6_0 = input_bot_ptr[0];
				uint8_t p6_1 = input_bot_ptr[1];
				uint8_t p6_2 = input_bot_ptr[2];
				uint8_t p7_0 = input_bot_ptr[3];
				uint8_t p7_1 = input_bot_ptr[4];
				uint8_t p7_2 = input_bot_ptr[5];
				uint8_t p8_0 = input_bot_ptr[6];
				uint8_t p8_1 = input_bot_ptr[7];
				uint8_t p8_2 = input_bot_ptr[8];

				sort9(&p0_0, &p1_0, &p2_0, &p3_0, &p4_0, &p5_0, &p6_0, &p7_0, &p8_0);
				sort9(&p0_1, &p1_1, &p2_1, &p3_1, &p4_1, &p5_1, &p6_1, &p7_1, &p8_1);
				sort9(&p0_2, &p1_2, &p2_2, &p3_2, &p4_2, &p5_2, &p6_2, &p7_2, &p8_2);

				*output_ptr++ = p4_0;
				*output_ptr++ = p4_1;
				*output_ptr++ = p4_2;
				input_top_ptr += channel_num;
				input_mid_ptr += channel_num;
				input_bot_ptr += channel_num;
			}
		}
	}
	
	//filter post process, free temp memory
	if (BODER_NON != border_flag)
		align_free(padding_filter_in_align);
	
#ifdef USE_CLOCK
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	printf("median_filter_box_3x3_u8_opt_with_neon_use_softnet funtion time elapse is %f, image size is\
height*width:%dx%d, channel number is %d\n", duration, height, width, channel_num);
#endif

	return 0;
}
/*********************************************************************************************
* para filter_in : input image ,must be uint8_t type
* para height:     input image height
* para width:      input image width
* para channel_num:channel number
* para filter_out: output filtered image,must be uint8_t type
* note:            filter_out size is same as filter_in
*
* para border_flag: BODER_ZERO ---padding zeros
*                   BODER_REPLICATE ---- padding replicate
*                   BODER_NON ---- padding non
*
* BODER_ZERO: filter_in padding with zeros
*           0 0 0 0
*           0 x x x
*           0 x x x
*
* BODER_REPLICATE: filter_in first padding with replicate
*             |
*             v
*           a a c e
*       --> a a c e
*           b b d f
*
* BODER_NON: filter_in padding none
*
* median filter 3x3 cross mask
*               F T F
*               T T T
*               F T F
*
**********************************************************************************************/
int median_filter_cross_3x3_u8_opt_with_neon_use_softnet(uint8_t * filter_in, uint32_t height, uint32_t width,
	uint8_t channel_num, uint8_t * filter_out, BORDER_FLAG border_flag)
{
	uint8_t *padding_filter_in_align = NULL;
	int row_index = 0;
	int col_index = 0;
	int border_size = 1;
	//filter body
	int col_end_index = 0;
	int row_end_index = 0;
	int row_step = 0;
	int filter_offset_flag = 0;
#ifdef USE_CLOCK
	start = clock();
#endif

	if (NULL == filter_in || NULL == filter_out)
		return -1;

	if (height < 3 || width < 3)
		return -1;

	if (channel_num != 1 && channel_num != 3)
		return -1;

	if (border_flag != BODER_NON && border_flag != BODER_REPLICATE && border_flag != BODER_ZERO)
		border_flag = BODER_NON;

	//padding boder
	if (BODER_ZERO == border_flag || BODER_REPLICATE == border_flag)
	{
		int malloc_bytes = ((height + 2 * border_size) * (width + 2 * border_size) * sizeof(uint8_t) + ALIGN_SIXTEEN_BYTES) * channel_num;
		padding_filter_in_align = (uint8_t *)align_malloc(malloc_bytes, ALIGN_SIXTEEN_BYTES);

		padding_boder(filter_in, height, width, channel_num, padding_filter_in_align, border_flag, border_size, sizeof(uint8_t));
	}
	else
	{
		padding_filter_in_align = filter_in;
		padding_boder(filter_in, height, width, channel_num, filter_out, border_flag, border_size, sizeof(uint8_t));
	}
	if (BODER_NON != border_flag)
	{
		row_end_index = height;
		col_end_index = width;
	}
	else
	{
		row_end_index = height - 2;
		col_end_index = width - 2;
	}
	row_step = (width + 2 * border_size * ((BODER_NON != border_flag) ? 1 : 0)) * channel_num;
	filter_offset_flag = ((BODER_NON == border_flag) ? 1 : 0);
	if (channel_num == 1)
	{
#ifdef USE_OPENMP
#pragma omp parallel for
#endif
		//filter body
		for (row_index = 0; row_index < row_end_index; row_index++)
		{
			uint8_t * input_top_ptr = padding_filter_in_align + row_index * row_step + channel_num;
			uint8_t * input_mid_ptr = padding_filter_in_align + (row_index + 1) * row_step;
			uint8_t * input_bot_ptr = padding_filter_in_align + (row_index + 2) * row_step + channel_num;
			uint8_t * output_ptr = filter_out + row_index * width * channel_num + filter_offset_flag * channel_num;
			for (col_index = 0; col_index + BATCH_EIGHT <= col_end_index; col_index += BATCH_EIGHT)
			{
				const uint8x8_t top_data = vld1_u8(input_top_ptr + col_index * channel_num);
				const uint8x16_t mid_data = vld1q_u8(input_mid_ptr + col_index * channel_num);
				const uint8x8_t bot_data = vld1_u8(input_bot_ptr + col_index * channel_num);

				uint8x8_t p0 = top_data;
				uint8x8_t p1 = vget_low_u8(mid_data); 
				uint8x8_t p2 = vext_u8(vget_low_u8(mid_data), vget_high_u8(mid_data), 1);
				uint8x8_t p3 = vext_u8(vget_low_u8(mid_data), vget_high_u8(mid_data), 2);
				uint8x8_t p4 = bot_data;


				sort5_neon(&p0, &p1, &p2, &p3, &p4);

				vst1_u8(output_ptr, p2);
				output_ptr += BATCH_EIGHT;
			}
			input_top_ptr += (col_index * channel_num);
			input_mid_ptr += (col_index * channel_num);
			input_bot_ptr += (col_index * channel_num);
			for (; col_index < col_end_index; col_index++)
			{
				uint8_t p0 = input_top_ptr[0];
				uint8_t p1 = input_mid_ptr[0];
				uint8_t p2 = input_mid_ptr[1];
				uint8_t p3 = input_mid_ptr[2];
				uint8_t p4 = input_bot_ptr[0];

				sort5(&p0, &p1, &p2, &p3, &p4);

				*output_ptr++ = p2;
				input_top_ptr += channel_num;
				input_mid_ptr += channel_num;
				input_bot_ptr += channel_num;
			}
		}
	}
	else
	{
#ifdef USE_OPENMP
#pragma omp parallel for
#endif
		for (row_index = 0; row_index < row_end_index; row_index++)
		{
			uint8_t * input_top_ptr = padding_filter_in_align + row_index * row_step + channel_num;
			uint8_t * input_mid_ptr = padding_filter_in_align + (row_index + 1) * row_step;
			uint8_t * input_bot_ptr = padding_filter_in_align + (row_index + 2) * row_step + channel_num;
			uint8_t * output_ptr = filter_out + row_index * width * channel_num + filter_offset_flag * channel_num;

			for (col_index = 0; col_index + BATCH_EIGHT <= col_end_index; col_index += BATCH_EIGHT)
			{
				const uint8x8x3_t top_data = vld3_u8(input_top_ptr + col_index * channel_num);
				const uint8x16x3_t mid_data = vld3q_u8(input_mid_ptr + col_index * channel_num);
				const uint8x8x3_t bot_data = vld3_u8(input_bot_ptr + col_index * channel_num);
				uint8x8x3_t out_data;
				//first channel 
				uint8x8_t p0_0 = top_data.val[0];
				uint8x8_t p1_0 = vget_low_u8(mid_data.val[0]); 
				uint8x8_t p2_0 = vext_u8(vget_low_u8(mid_data.val[0]), vget_high_u8(mid_data.val[0]), 1);
				uint8x8_t p3_0 = vext_u8(vget_low_u8(mid_data.val[0]), vget_high_u8(mid_data.val[0]), 2);
				uint8x8_t p4_0 = bot_data.val[0];

				sort5_neon(&p0_0, &p1_0, &p2_0, &p3_0, &p4_0);
				out_data.val[0] = p2_0;

				///secone channel
				uint8x8_t p0_1 = top_data.val[1];
				uint8x8_t p1_1 = vget_low_u8(mid_data.val[1]);
				uint8x8_t p2_1 = vext_u8(vget_low_u8(mid_data.val[1]), vget_high_u8(mid_data.val[1]), 1);
				uint8x8_t p3_1 = vext_u8(vget_low_u8(mid_data.val[1]), vget_high_u8(mid_data.val[1]), 2);
				uint8x8_t p4_1 = bot_data.val[1];

				sort5_neon(&p0_1, &p1_1, &p2_1, &p3_1, &p4_1);
				out_data.val[1] = p2_1;

				///third channel
				uint8x8_t p0_2 = top_data.val[2];
				uint8x8_t p1_2 = vget_low_u8(mid_data.val[2]);
				uint8x8_t p2_2 = vext_u8(vget_low_u8(mid_data.val[2]), vget_high_u8(mid_data.val[2]), 1);
				uint8x8_t p3_2 = vext_u8(vget_low_u8(mid_data.val[2]), vget_high_u8(mid_data.val[2]), 2);
				uint8x8_t p4_2 = bot_data.val[2];

				sort5_neon(&p0_2, &p1_2, &p2_2, &p3_2, &p4_2);
				out_data.val[2] = p2_2;

				vst3_u8(output_ptr, out_data);
				output_ptr += (channel_num*BATCH_EIGHT);
			}
			input_top_ptr += col_index * channel_num;
			input_mid_ptr += col_index * channel_num;
			input_bot_ptr += col_index * channel_num;
			for (; col_index < col_end_index; col_index++)
			{
				uint8_t p0_0 = input_top_ptr[0];
				uint8_t p0_1 = input_top_ptr[1];
				uint8_t p0_2 = input_top_ptr[2];
				uint8_t p1_0 = input_mid_ptr[0];
				uint8_t p1_1 = input_mid_ptr[1];
				uint8_t p1_2 = input_mid_ptr[2];
				uint8_t p2_0 = input_mid_ptr[3];
				uint8_t p2_1 = input_mid_ptr[4];
				uint8_t p2_2 = input_mid_ptr[5];
				uint8_t p3_0 = input_mid_ptr[6];
				uint8_t p3_1 = input_mid_ptr[7];
				uint8_t p3_2 = input_mid_ptr[8];
				uint8_t p4_0 = input_bot_ptr[0];
				uint8_t p4_1 = input_bot_ptr[1];
				uint8_t p4_2 = input_bot_ptr[2];
				

				sort5(&p0_0, &p1_0, &p2_0, &p3_0, &p4_0);
				sort5(&p0_1, &p1_1, &p2_1, &p3_1, &p4_1);
				sort5(&p0_2, &p1_2, &p2_2, &p3_2, &p4_2);

				*output_ptr++ = p2_0;
				*output_ptr++ = p2_1;
				*output_ptr++ = p2_2;
				input_top_ptr += channel_num;
				input_mid_ptr += channel_num;
				input_bot_ptr += channel_num;
			}
		}
	}

	//filter post process, free temp memory
	if (BODER_NON != border_flag)
		align_free(padding_filter_in_align);

#ifdef USE_CLOCK
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	printf("median_filter_cross_3x3_u8_opt_with_neon_use_softnet funtion time elapse is %f, image size is\
height*width:%dx%d, channel number is %d\n", duration, height, width, channel_num);
#endif
	return 0;
}

/***************************************************************************************************
* 
* median filter 3x3 box mask 
*               T T T
*               T T T
*               T T T
*
***************************************************************************************************/
int median_filter_box_3x3_u8_non_opt_use_softnet(uint8_t * filter_in, uint32_t height, uint32_t width,
	uint8_t channel_num, uint8_t * filter_out, BORDER_FLAG border_flag)
{
	uint8_t *padding_filter_in_align = NULL;
	int row_index = 0;
	int col_index = 0;
	int border_size = 1;
	int col_end_index = 0;
	int row_end_index = 0;
	int row_step = 0;
	int filter_offset_flag = 0;
#ifdef USE_CLOCK
	start = clock();
#endif

	if (NULL == filter_in || NULL == filter_out)
		return -1;

	if (height < 3 || width < 3)
		return -1;

	if (channel_num != 1 && channel_num != 3)
		return -1;

	//padding boder
	if (BODER_ZERO == border_flag || BODER_REPLICATE == border_flag)
	{
		int malloc_bytes = ((height + 2 * border_size) * (width + 2 * border_size) * sizeof(uint8_t) + ALIGN_SIXTEEN_BYTES) * channel_num;
		padding_filter_in_align = (uint8_t *)align_malloc(malloc_bytes, ALIGN_SIXTEEN_BYTES);

		padding_boder(filter_in, height, width, channel_num, padding_filter_in_align, border_flag, border_size, sizeof(uint8_t));
	}
	else
	{
		padding_filter_in_align = filter_in;
		padding_boder(filter_in, height, width, channel_num, filter_out, border_flag, border_size, sizeof(uint8_t));
		
	}

	if (BODER_NON != border_flag)
	{
		row_end_index = height;
		col_end_index = width;
	}
	else
	{
		row_end_index = height - 2;
		col_end_index = width - 2;
	}
	row_step = (width + 2 * border_size * ((BODER_NON != border_flag) ? 1 : 0)) * channel_num;
	filter_offset_flag = ((BODER_NON == border_flag) ? 1 : 0);
	//filter body

	if (channel_num == 1)
	{
#ifdef USE_OPENMP
#pragma omp parallel for
#endif
		for (row_index = 0; row_index < row_end_index; row_index++)
		{
			uint8_t * input_top_ptr = padding_filter_in_align + row_index * row_step;
			uint8_t * input_mid_ptr = padding_filter_in_align + (row_index + 1) * row_step;
			uint8_t * input_bot_ptr = padding_filter_in_align + (row_index + 2) * row_step;
			uint8_t * output_ptr = filter_out + row_index * width * channel_num + filter_offset_flag * channel_num;

			for (col_index = 0; col_index < col_end_index; col_index++)
			{
				uint8_t p0 = input_top_ptr[0];
				uint8_t p1 = input_top_ptr[1];
				uint8_t p2 = input_top_ptr[2];
				uint8_t p3 = input_mid_ptr[0];
				uint8_t p4 = input_mid_ptr[1];
				uint8_t p5 = input_mid_ptr[2];
				uint8_t p6 = input_bot_ptr[0];
				uint8_t p7 = input_bot_ptr[1];
				uint8_t p8 = input_bot_ptr[2];

				sort9(&p0, &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8);

				*output_ptr++ = p4;
				input_top_ptr++;
				input_mid_ptr++;
				input_bot_ptr++;
			}
		}
	}
	else
	{
#ifdef USE_OPENMP
#pragma omp parallel for
#endif
		for (row_index = 0; row_index < row_end_index; row_index++)
		{
			uint8_t * input_top_ptr = padding_filter_in_align + row_index * row_step;
			uint8_t * input_mid_ptr = padding_filter_in_align + (row_index + 1) * row_step;
			uint8_t * input_bot_ptr = padding_filter_in_align + (row_index + 2) * row_step;
			uint8_t * output_ptr = filter_out + row_index * width * channel_num + filter_offset_flag * channel_num;

			for (col_index = 0; col_index < col_end_index; col_index++)
			{
				uint8_t p0_0 = input_top_ptr[0];
				uint8_t p0_1 = input_top_ptr[1];
				uint8_t p0_2 = input_top_ptr[2];
				uint8_t p1_0 = input_top_ptr[3];
				uint8_t p1_1 = input_top_ptr[4];
				uint8_t p1_2 = input_top_ptr[5];
				uint8_t p2_0 = input_top_ptr[6];
				uint8_t p2_1 = input_top_ptr[7];
				uint8_t p2_2 = input_top_ptr[8];
				uint8_t p3_0 = input_mid_ptr[0];
				uint8_t p3_1 = input_mid_ptr[1];
				uint8_t p3_2 = input_mid_ptr[2];
				uint8_t p4_0 = input_mid_ptr[3];
				uint8_t p4_1 = input_mid_ptr[4];
				uint8_t p4_2 = input_mid_ptr[5];
				uint8_t p5_0 = input_mid_ptr[6];
				uint8_t p5_1 = input_mid_ptr[7];
				uint8_t p5_2 = input_mid_ptr[8];
				uint8_t p6_0 = input_bot_ptr[0];
				uint8_t p6_1 = input_bot_ptr[1];
				uint8_t p6_2 = input_bot_ptr[2];
				uint8_t p7_0 = input_bot_ptr[3];
				uint8_t p7_1 = input_bot_ptr[4];
				uint8_t p7_2 = input_bot_ptr[5];
				uint8_t p8_0 = input_bot_ptr[6];
				uint8_t p8_1 = input_bot_ptr[7];
				uint8_t p8_2 = input_bot_ptr[8];

				sort9(&p0_0, &p1_0, &p2_0, &p3_0, &p4_0, &p5_0, &p6_0, &p7_0, &p8_0);
				sort9(&p0_1, &p1_1, &p2_1, &p3_1, &p4_1, &p5_1, &p6_1, &p7_1, &p8_1);
				sort9(&p0_2, &p1_2, &p2_2, &p3_2, &p4_2, &p5_2, &p6_2, &p7_2, &p8_2);

				*output_ptr++ = p4_0;
				*output_ptr++ = p4_1;
				*output_ptr++ = p4_2;
				input_top_ptr += channel_num;
				input_mid_ptr += channel_num;
				input_bot_ptr += channel_num;
			}
		}
	}
	//filter post process, free temp memory
	if (BODER_NON != border_flag)
		align_free(padding_filter_in_align);

#ifdef USE_CLOCK
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	printf("median_filter_box_3x3_u8_non_opt_use_softnet funtion time elapse is %f, image size is\
height*width:%dx%d, channel number is %d\n", duration, height, width, channel_num);
#endif
	return 0;
}



/***************************************************************************************************
*
* median filter 3x3 cross mask
*               F T F
*               T T T
*               F T F
*
***************************************************************************************************/
int median_filter_cross_3x3_u8_non_opt_use_softnet(uint8_t * filter_in, uint32_t height, uint32_t width,
	uint8_t channel_num, uint8_t * filter_out, BORDER_FLAG border_flag)
{
	uint8_t *padding_filter_in_align = NULL;
	int row_index = 0;
	int col_index = 0;
	int border_size = 1;
	int col_end_index = 0;
	int row_end_index = 0;
	int row_step = 0;
	int filter_offset_flag = 0;
#ifdef USE_OPENMP
	start = clock();
#endif

	if (NULL == filter_in || NULL == filter_out)
		return -1;

	if (height < 3 || width < 3)
		return -1;

	if (channel_num != 1 && channel_num != 3)
		return -1;

	//padding boder
	if (BODER_ZERO == border_flag || BODER_REPLICATE == border_flag)
	{
		int malloc_bytes = ((height + 2 * border_size) * (width + 2 * border_size) * sizeof(uint8_t) + ALIGN_SIXTEEN_BYTES) * channel_num;
		padding_filter_in_align = (uint8_t *)align_malloc(malloc_bytes, ALIGN_SIXTEEN_BYTES);

		padding_boder(filter_in, height, width, channel_num, padding_filter_in_align, border_flag, border_size, sizeof(uint8_t));
	}
	else
	{
		padding_filter_in_align = filter_in;
		padding_boder(filter_in, height, width, channel_num, filter_out, border_flag, border_size, sizeof(uint8_t));

	}

	if (BODER_NON != border_flag)
	{
		row_end_index = height;
		col_end_index = width;
	}
	else
	{
		row_end_index = height - 2;
		col_end_index = width - 2;
	}
	row_step = (width + 2 * border_size * ((BODER_NON != border_flag) ? 1 : 0)) * channel_num;
	filter_offset_flag = ((BODER_NON == border_flag) ? 1 : 0);
	//filter body

	if (channel_num == 1)
	{
#ifdef USE_OPENMP
#pragma omp parallel for
#endif
		for (row_index = 0; row_index < row_end_index; row_index++)
		{
			uint8_t * input_top_ptr = padding_filter_in_align + row_index * row_step + channel_num;
			uint8_t * input_mid_ptr = padding_filter_in_align + (row_index + 1) * row_step;
			uint8_t * input_bot_ptr = padding_filter_in_align + (row_index + 2) * row_step + channel_num;
			uint8_t * output_ptr = filter_out + row_index * width * channel_num + filter_offset_flag * channel_num;

			for (col_index = 0; col_index < col_end_index; col_index++)
			{
				uint8_t p0 = input_top_ptr[0];
				uint8_t p1 = input_mid_ptr[0];
				uint8_t p2 = input_mid_ptr[1];
				uint8_t p3 = input_mid_ptr[2];
				uint8_t p4 = input_bot_ptr[0];
				
				sort5(&p0, &p1, &p2, &p3, &p4);

				*output_ptr++ = p2;
				input_top_ptr++;
				input_mid_ptr++;
				input_bot_ptr++;
			}
		}
	}
	else
	{
#ifdef USE_OPENMP
#pragma omp parallel for
#endif
		for (row_index = 0; row_index < row_end_index; row_index++)
		{
			uint8_t * input_top_ptr = padding_filter_in_align + row_index * row_step + channel_num;
			uint8_t * input_mid_ptr = padding_filter_in_align + (row_index + 1) * row_step;
			uint8_t * input_bot_ptr = padding_filter_in_align + (row_index + 2) * row_step + channel_num;
			uint8_t * output_ptr = filter_out + row_index * width * channel_num + filter_offset_flag * channel_num;

			for (col_index = 0; col_index < col_end_index; col_index++)
			{
				uint8_t p0_0 = input_top_ptr[0];
				uint8_t p0_1 = input_top_ptr[1];
				uint8_t p0_2 = input_top_ptr[2];
				uint8_t p1_0 = input_mid_ptr[0];
				uint8_t p1_1 = input_mid_ptr[1];
				uint8_t p1_2 = input_mid_ptr[2];
				uint8_t p2_0 = input_mid_ptr[3];
				uint8_t p2_1 = input_mid_ptr[4];
				uint8_t p2_2 = input_mid_ptr[5];
				uint8_t p3_0 = input_mid_ptr[6];
				uint8_t p3_1 = input_mid_ptr[7];
				uint8_t p3_2 = input_mid_ptr[8];
				uint8_t p4_0 = input_bot_ptr[0];
				uint8_t p4_1 = input_bot_ptr[1];
				uint8_t p4_2 = input_bot_ptr[2];


				sort5(&p0_0, &p1_0, &p2_0, &p3_0, &p4_0);
				sort5(&p0_1, &p1_1, &p2_1, &p3_1, &p4_1);
				sort5(&p0_2, &p1_2, &p2_2, &p3_2, &p4_2);

				*output_ptr++ = p2_0;
				*output_ptr++ = p2_1;
				*output_ptr++ = p2_2;
				input_top_ptr += channel_num;
				input_mid_ptr += channel_num;
				input_bot_ptr += channel_num;
			}
		}
	}
	//filter post process, free temp memory
	if (BODER_NON != border_flag)
		align_free(padding_filter_in_align);

#ifdef USE_CLOCK
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	printf("median_filter_cross_3x3_u8_non_opt_use_softnet funtion time elapse is %f, image size is\
height*width:%dx%d, channel number is %d\n", duration, height, width, channel_num);
#endif
	return 0;
}



