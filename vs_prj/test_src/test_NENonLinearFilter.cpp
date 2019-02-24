#include "test_common.h"
#include "test_NENonLinearFilter.h"
#include "NENonLinearFilter.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if (defined(_WIN32) || defined(_WIN64))

#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

#endif


#define IMAGE_HEIGHT_NORMAL 480
#define IMAGE_WIDTH_NORMAL 640
#define IMAGE_CHANNEL_ONE 1
#define IMAGE_CHANNEL_THREE 3

#define IMAGE_HEIGHT_UNUSUAL 480
#define IMAGE_WIDTH_UNUSUAL 640


int test_NENonLinearFilter_median_filter_box_u8_3x3(uint8_t *filter_in, int height, int width, int ch_num)
{
	int ret = 0;
	BORDER_FLAG border_flag = BODER_NON;
	int image_size = height * width;
	uint8_t *non_opt_filter_out = new uint8_t[image_size * ch_num];
	uint8_t *opt_filter_out = new uint8_t[image_size * ch_num];
	memset(non_opt_filter_out, 0, image_size * ch_num);
	memset(opt_filter_out, 0, image_size * ch_num);

	//1  test border non
	//origin algorithm border flag is non
	ret = median_filter_box_3x3_u8_non_opt_use_softnet(filter_in, height, width, ch_num, non_opt_filter_out, border_flag);
	if (ret == -1)
		return -1;
	show_image(non_opt_filter_out, height, width, ch_num, sizeof(unsigned char), "non_opt_filter_out_border_non");

	//opt algorithm border flag is non
	ret = median_filter_box_3x3_u8_opt_with_neon_use_softnet(filter_in, height, width, ch_num, opt_filter_out, border_flag);
	if (ret == -1)
		return -1;
	show_image(opt_filter_out, height, width, ch_num, sizeof(unsigned char), "opt_filter_out_border_non");
	//compare origin algorithm and opt algorithm result
	compare_opt_and_non_opt_result(opt_filter_out, non_opt_filter_out, sizeof(unsigned char), image_size, ch_num);

	memset(non_opt_filter_out, 0, image_size * ch_num);
	memset(opt_filter_out, 0, image_size * ch_num);
	//2  test border replicate
	//origin algorithm border flag is replicate
	border_flag = BODER_REPLICATE;
	ret = median_filter_box_3x3_u8_non_opt_use_softnet(filter_in, height, width, ch_num, non_opt_filter_out, border_flag);
	if (ret == -1)
		return -1;
	show_image(non_opt_filter_out, height, width, ch_num, sizeof(unsigned char), "non_opt_filter_out_border_replicate");

	//opt algorithm border flag is replicate
	ret = median_filter_box_3x3_u8_opt_with_neon_use_softnet(filter_in, height, width, ch_num, opt_filter_out, border_flag);
	if (ret == -1)
		return -1;
	show_image(opt_filter_out, height, width, ch_num, sizeof(unsigned char), "opt_filter_out_border_replicate");

	//compare origin algorithm and opt algorithm result
	compare_opt_and_non_opt_result(opt_filter_out, non_opt_filter_out, sizeof(unsigned char), image_size, ch_num);


	memset(non_opt_filter_out, 0, image_size * ch_num);
	memset(opt_filter_out, 0, image_size * ch_num);
	//3  test border zero
	border_flag = BODER_ZERO;
	//origin algorithm border flag is replicate
	ret = median_filter_box_3x3_u8_non_opt_use_softnet(filter_in, height, width, ch_num, non_opt_filter_out, border_flag);
	if (ret == -1)
		return -1;
	show_image(non_opt_filter_out, height, width, ch_num, sizeof(unsigned char), "non_opt_filter_out_border_zero");
	//opt algorithm border flag is replicate
	ret = median_filter_box_3x3_u8_opt_with_neon_use_softnet(filter_in, height, width, ch_num, opt_filter_out, border_flag);
	if (ret == -1)
		return -1;
	show_image(opt_filter_out, height, width, ch_num, sizeof(unsigned char), "opt_filter_out_border_zero");
	//compare origin algorithm and opt algorithm result
	compare_opt_and_non_opt_result(opt_filter_out, non_opt_filter_out, sizeof(unsigned char), image_size, ch_num);

	delete[] non_opt_filter_out;
	delete[] opt_filter_out;
	return ret;
}


int test_NENonLinearFilter_median_filter_cross_u8_3x3(uint8_t *filter_in, int height, int width, int ch_num)
{
	int ret = 0;
	BORDER_FLAG border_flag = BODER_NON;
	int image_size = height * width;
	uint8_t *non_opt_filter_out = new uint8_t[image_size * ch_num];
	uint8_t *opt_filter_out = new uint8_t[image_size * ch_num];
	memset(non_opt_filter_out, 0, image_size * ch_num);
	memset(opt_filter_out, 0, image_size * ch_num);

	//1  test border non
	//origin algorithm border flag is non
	ret = median_filter_cross_3x3_u8_non_opt_use_softnet(filter_in, height, width, ch_num, non_opt_filter_out, border_flag);
	if (ret == -1)
		return -1;
	show_image(non_opt_filter_out, height, width, ch_num, sizeof(unsigned char), "non_opt_filter_out_border_non");

	//opt algorithm border flag is non
	ret = median_filter_cross_3x3_u8_opt_with_neon_use_softnet(filter_in, height, width, ch_num, opt_filter_out, border_flag);
	if (ret == -1)
		return -1;
	show_image(opt_filter_out, height, width, ch_num, sizeof(unsigned char), "opt_filter_out_border_non");
	//compare origin algorithm and opt algorithm result
	compare_opt_and_non_opt_result(opt_filter_out, non_opt_filter_out, sizeof(unsigned char), image_size, ch_num);

	memset(non_opt_filter_out, 0, image_size * ch_num);
	memset(opt_filter_out, 0, image_size * ch_num);
	//2  test border replicate
	//origin algorithm border flag is replicate
	border_flag = BODER_REPLICATE;
	ret = median_filter_cross_3x3_u8_non_opt_use_softnet(filter_in, height, width, ch_num, non_opt_filter_out, border_flag);
	if (ret == -1)
		return -1;
	show_image(non_opt_filter_out, height, width, ch_num, sizeof(unsigned char), "non_opt_filter_out_border_replicate");

	//opt algorithm border flag is replicate
	ret = median_filter_cross_3x3_u8_opt_with_neon_use_softnet(filter_in, height, width, ch_num, opt_filter_out, border_flag);
	if (ret == -1)
		return -1;
	show_image(opt_filter_out, height, width, ch_num, sizeof(unsigned char), "opt_filter_out_border_replicate");

	//compare origin algorithm and opt algorithm result
	compare_opt_and_non_opt_result(opt_filter_out, non_opt_filter_out, sizeof(unsigned char), image_size, ch_num);


	memset(non_opt_filter_out, 0, image_size * ch_num);
	memset(opt_filter_out, 0, image_size * ch_num);
	//3  test border zero
	border_flag = BODER_ZERO;
	//origin algorithm border flag is replicate
	ret = median_filter_cross_3x3_u8_non_opt_use_softnet(filter_in, height, width, ch_num, non_opt_filter_out, border_flag);
	if (ret == -1)
		return -1;
	show_image(non_opt_filter_out, height, width, ch_num, sizeof(unsigned char), "non_opt_filter_out_border_zero");
	//opt algorithm border flag is replicate
	ret = median_filter_cross_3x3_u8_opt_with_neon_use_softnet(filter_in, height, width, ch_num, opt_filter_out, border_flag);
	if (ret == -1)
		return -1;
	show_image(opt_filter_out, height, width, ch_num, sizeof(unsigned char), "opt_filter_out_border_zero");
	//compare origin algorithm and opt algorithm result
	compare_opt_and_non_opt_result(opt_filter_out, non_opt_filter_out, sizeof(unsigned char), image_size, ch_num);

	delete[] non_opt_filter_out;
	delete[] opt_filter_out;
	return ret;
}

int test_NENonLinearFilter_median_filter_disk_u8_3x3(uint8_t *filter_in, int height, int width, int ch_num)
{
	int ret = 0;
	BORDER_FLAG border_flag = BODER_NON;
	int image_size = height * width;
	uint8_t *non_opt_filter_out = new uint8_t[image_size * ch_num];
	uint8_t *opt_filter_out = new uint8_t[image_size * ch_num];
	memset(non_opt_filter_out, 0, image_size * ch_num);
	memset(opt_filter_out, 0, image_size * ch_num);

	//1  test border non
	//origin algorithm border flag is non
	ret = median_filter_disk_3x3_u8_non_opt_use_softnet(filter_in, height, width, ch_num, non_opt_filter_out, border_flag);
	if (ret == -1)
		return -1;
	show_image(non_opt_filter_out, height, width, ch_num, sizeof(unsigned char), "non_opt_filter_out_border_non");

	//opt algorithm border flag is non
	ret = median_filter_disk_3x3_u8_opt_with_neon_use_softnet(filter_in, height, width, ch_num, opt_filter_out, border_flag);
	if (ret == -1)
		return -1;
	show_image(opt_filter_out, height, width, ch_num, sizeof(unsigned char), "opt_filter_out_border_non");
	//compare origin algorithm and opt algorithm result
	compare_opt_and_non_opt_result(opt_filter_out, non_opt_filter_out, sizeof(unsigned char), image_size, ch_num);

	memset(non_opt_filter_out, 0, image_size * ch_num);
	memset(opt_filter_out, 0, image_size * ch_num);
	//2  test border replicate
	//origin algorithm border flag is replicate
	border_flag = BODER_REPLICATE;
	ret = median_filter_disk_3x3_u8_non_opt_use_softnet(filter_in, height, width, ch_num, non_opt_filter_out, border_flag);
	if (ret == -1)
		return -1;
	show_image(non_opt_filter_out, height, width, ch_num, sizeof(unsigned char), "non_opt_filter_out_border_replicate");

	//opt algorithm border flag is replicate
	ret = median_filter_disk_3x3_u8_opt_with_neon_use_softnet(filter_in, height, width, ch_num, opt_filter_out, border_flag);
	if (ret == -1)
		return -1;
	show_image(opt_filter_out, height, width, ch_num, sizeof(unsigned char), "opt_filter_out_border_replicate");

	//compare origin algorithm and opt algorithm result
	compare_opt_and_non_opt_result(opt_filter_out, non_opt_filter_out, sizeof(unsigned char), image_size, ch_num);


	memset(non_opt_filter_out, 0, image_size * ch_num);
	memset(opt_filter_out, 0, image_size * ch_num);
	//3  test border zero
	border_flag = BODER_ZERO;
	//origin algorithm border flag is replicate
	ret = median_filter_disk_3x3_u8_non_opt_use_softnet(filter_in, height, width, ch_num, non_opt_filter_out, border_flag);
	if (ret == -1)
		return -1;
	show_image(non_opt_filter_out, height, width, ch_num, sizeof(unsigned char), "non_opt_filter_out_border_zero");
	//opt algorithm border flag is replicate
	ret = median_filter_disk_3x3_u8_opt_with_neon_use_softnet(filter_in, height, width, ch_num, opt_filter_out, border_flag);
	if (ret == -1)
		return -1;
	show_image(opt_filter_out, height, width, ch_num, sizeof(unsigned char), "opt_filter_out_border_zero");
	//compare origin algorithm and opt algorithm result
	compare_opt_and_non_opt_result(opt_filter_out, non_opt_filter_out, sizeof(unsigned char), image_size, ch_num);

	delete[] non_opt_filter_out;
	delete[] opt_filter_out;
	return ret;
}

#if (defined(_WIN32) || defined(_WIN64))
int test_NENonLinearFilter_vs_entry()
{
	uint8_t * filter_in = NULL;
	int img_width = 0;
	int img_height = 0;
	int img_ch = 0;

	{

		//test uint8_t gray image median filter with 3x3 box mask
		Mat img_origin_gray = imread("../../../test_data/nonlinear_filter/test1.bmp", 0);
		img_width = img_origin_gray.cols;
		img_height = img_origin_gray.rows;
		filter_in = img_origin_gray.data;
		img_ch = img_origin_gray.channels();
		test_NENonLinearFilter_median_filter_box_u8_3x3(filter_in, img_height, img_width, img_ch);

		//test uint8_t rgb image median filter with 3x3 box mask
		Mat img_origin_rgb = imread("../../../test_data/nonlinear_filter/test1.png");
		img_width = img_origin_rgb.cols;
		img_height = img_origin_rgb.rows;
		filter_in = img_origin_rgb.data;
		img_ch = img_origin_rgb.channels();
		test_NENonLinearFilter_median_filter_box_u8_3x3(filter_in, img_height, img_width, img_ch);
	}


	{
		//test uint8_t gray image median filter with 3x3 cross mask
		Mat img_origin_gray = imread("../../../test_data/nonlinear_filter/test1.bmp", 0);
		img_width = img_origin_gray.cols;
		img_height = img_origin_gray.rows;
		filter_in = img_origin_gray.data;
		img_ch = img_origin_gray.channels();
		test_NENonLinearFilter_median_filter_cross_u8_3x3(filter_in, img_height, img_width, img_ch);

		//test uint8_t rgb image median filter with 3x3 cross mask
		Mat img_origin_rgb = imread("../../../test_data/nonlinear_filter/test1.png");
		img_width = img_origin_rgb.cols;
		img_height = img_origin_rgb.rows;
		filter_in = img_origin_rgb.data;
		img_ch = img_origin_rgb.channels();
		test_NENonLinearFilter_median_filter_cross_u8_3x3(filter_in, img_height, img_width, img_ch);
	}



	{
		//test uint8_t gray image median filter with 3x3 disk mask
		Mat img_origin_gray = imread("../../../test_data/nonlinear_filter/test1.bmp", 0);
		img_width = img_origin_gray.cols;
		img_height = img_origin_gray.rows;
		filter_in = img_origin_gray.data;
		img_ch = img_origin_gray.channels();
		test_NENonLinearFilter_median_filter_disk_u8_3x3(filter_in, img_height, img_width, img_ch);

		//test uint8_t rgb image median filter with 3x3 disk mask
		Mat img_origin_rgb = imread("../../../test_data/nonlinear_filter/test1.png");
		img_width = img_origin_rgb.cols;
		img_height = img_origin_rgb.rows;
		filter_in = img_origin_rgb.data;
		img_ch = img_origin_rgb.channels();
		test_NENonLinearFilter_median_filter_disk_u8_3x3(filter_in, img_height, img_width, img_ch);
	}

	return 0;
}


#else
int test_NENonLinearFilter_ds_entry()
{
	uint8_t * filter_in = NULL;
	int img_width = 0;
	int img_height = 0;
	int img_ch = 0;
	FILE *f_img = NULL;
	uint8_t *img_data = NULL;
	{
		//test uint8_t gray image median filter with 3x3 box mask
		const char * image_filename = "rgbx1x480x640.txt";
		f_img = fopen(image_filename, "rb");
		if(NULL == f_img)
			printf("open image file rgbx1x480x640.txt fail!\n");

		img_data = new uint8_t[IMAGE_HEIGHT_NORMAL * IMAGE_WIDTH_NORMAL * IMAGE_CHANNEL_ONE];
		if(NULL == img_data)
		{
			printf("malloc memory for image fail\n");
			fclose(f_img);
		}

		fread(img_data, sizeof(uint8_t), IMAGE_HEIGHT_NORMAL * IMAGE_WIDTH_NORMAL *IMAGE_CHANNEL_ONE, f_img);
		fclose(f_img);

		img_width = IMAGE_WIDTH_NORMAL;
		img_height = IMAGE_HEIGHT_NORMAL;
		filter_in = img_data;
		img_ch = IMAGE_CHANNEL_ONE;
		test_NENonLinearFilter_median_filter_box_u8_3x3(filter_in, img_height, img_width, img_ch);

#if 0
		//test uint8_t rgb image median filter with 3x3 box mask
		img_width = IMAGE_WIDTH_NORMAL;
		img_height = IMAGE_HEIGHT_NORMAL;
		filter_in = img_data;
		img_ch = IMAGE_CHANNEL_ONE;
		test_NENonLinearFilter_median_filter_box_u8_3x3(filter_in, img_height, img_width, img_ch);
#endif
	}



	return 0;
}
#endif
