#ifndef NE_NON_LINEAR_FILTER_H
#define NE_NON_LINEAR_FILTER_H
#ifdef __cplusplus
extern "C" {
#endif

#if (defined _WIN64) || (defined _WIN32)
#include "NEON_2_SSE.h"
#else
#include <arm_neon.h>
#endif

typedef enum BORDER_FLAG
{
	BODER_ZERO = 0,
	BODER_REPLICATE = 1,
	BODER_NON = 2
}BORDER_FLAG;

typedef int(*median_filter_3x3_func)(uint8_t * filter_in, uint32_t height, uint32_t width,
	uint8_t channel_num, uint8_t * filter_out, BORDER_FLAG border_flag);

int median_filter_box_3x3_u8_opt_with_neon_use_softnet(uint8_t * filter_in, uint32_t height, uint32_t width,
	uint8_t channel_num, uint8_t * filter_out, BORDER_FLAG border_flag);

int median_filter_box_3x3_u8_non_opt_use_softnet(uint8_t * filter_in, uint32_t height, uint32_t width,
	uint8_t channel_num, uint8_t * filter_out, BORDER_FLAG border_flag);

int median_filter_cross_3x3_u8_opt_with_neon_use_softnet(uint8_t * filter_in, uint32_t height, uint32_t width,
	uint8_t channel_num, uint8_t * filter_out, BORDER_FLAG border_flag);

int median_filter_cross_3x3_u8_non_opt_use_softnet(uint8_t * filter_in, uint32_t height, uint32_t width,
	uint8_t channel_num, uint8_t * filter_out, BORDER_FLAG border_flag);

extern median_filter_3x3_func median_filter_disk_3x3_u8_non_opt_use_softnet;

extern median_filter_3x3_func median_filter_disk_3x3_u8_opt_with_neon_use_softnet;


#ifdef __cplusplus
}
#endif
#endif //NE_NON_LINEAR_FILTER_H

