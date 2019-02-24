#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#if (defined _WIN32) || (defined _WIN64)

//#define SHOW_IMAGE

#endif
void compare_opt_and_non_opt_result(void *opt_result, void *non_opt_result, int step, int size, int ch_num);
void show_image(void *image_data, int height,int width,int channel_num, int step, const char * window_name);

#endif //TEST_COMMON_H
