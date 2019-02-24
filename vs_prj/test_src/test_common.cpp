#include "test_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>

#if (defined _WIN32) || (defined _WIN64)

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

#endif
void compare_opt_and_non_opt_result(void *opt_result, void *non_opt_result, int step, int size, int ch_num)
{
	if (opt_result == NULL || non_opt_result == NULL)
		printf("%s input parameter opt_result or non_opt_result is NULL ptr\n", __func__);
	
	if (size <= 0 || (step != 1 && step != 2 && step != 4))
		printf("%s input parameter size or step is error\n", __func__);

	int index = 0;
	int errOneCount = 0;
	int errTwoCount = 0;
	int errFourCount = 0;
	int errEightCount = 0;
	int errSixteenCount = 0;
	if (step == 1)
	{
		for (index = 0; index < size * ch_num; index++)
		{
			int opt_data = *(unsigned char *)opt_result;
			int non_opt_data = *(unsigned char *)non_opt_result;
			if (abs(opt_data - non_opt_data) >= 1)
			{
				errOneCount++;
			}
			if (abs(opt_data - non_opt_data) >= 2)
			{
				errTwoCount++;
			}
			if (abs(opt_data - non_opt_data) >= 4)
			{
				errFourCount++;
			}
			if (abs(opt_data - non_opt_data) >= 8)
			{
				errEightCount++;
			}
			if (abs(opt_data - non_opt_data) >= 16)
			{
				errSixteenCount++;
			}
			opt_result = (void *)((unsigned char *)opt_result + 1);
			non_opt_result = (void *)((unsigned char *)non_opt_result + 1);
		}
	}
	else if (step == 2)
	{
		for (index = 0; index < size * ch_num; index++)
		{
			int opt_data = *(unsigned short *)opt_result;
			int non_opt_data = *(unsigned short *)non_opt_result;
			if (abs(opt_data - non_opt_data) >= 1)
			{
				errOneCount++;
			}
			if (abs(opt_data - non_opt_data) >= 2)
			{
				errTwoCount++;
			}
			if (abs(opt_data - non_opt_data) >= 4)
			{
				errFourCount++;
			}
			if (abs(opt_data - non_opt_data) >= 8)
			{
				errEightCount++;
			}
			if (abs(opt_data - non_opt_data) >= 16)
			{
				errSixteenCount++;
			}
			opt_result = (void *)((unsigned short *)opt_result + 1);
			non_opt_result = (void *)((unsigned short *)non_opt_result + 1);
		}
	}
	else
	{
		for (index = 0; index < size * ch_num; index++)
		{
			int opt_data = *(unsigned int *)opt_result;
			int non_opt_data = *(unsigned int *)non_opt_result;
			if (abs(opt_data - non_opt_data) >= 1)
			{
				errOneCount++;
			}
			if (abs(opt_data - non_opt_data) >= 2)
			{
				errTwoCount++;
			}
			if (abs(opt_data - non_opt_data) >= 4)
			{
				errFourCount++;
			}
			if (abs(opt_data - non_opt_data) >= 8)
			{
				errEightCount++;
			}
			if (abs(opt_data - non_opt_data) >= 16)
			{
				errSixteenCount++;
			}
			opt_result = (void *)((unsigned int *)opt_result + 1);
			non_opt_result = (void *)((unsigned int *)non_opt_result + 1);
		}
	}

	printf("errOneCount is %d\n", errOneCount);
	printf("errTwoCount is %d\n", errTwoCount);
	printf("errFourCount is %d\n", errFourCount);
	printf("errEightCount is %d\n", errEightCount);
	printf("errSixteenCount is %d\n", errSixteenCount);
}


void show_image(void *image_data, int height, int width, int channel_num, int pixel_size, const char * window_name)
{
#ifdef SHOW_IMAGE

	if (image_data == NULL || window_name == NULL)
	{
		printf("%s input parameter image_data or window_name is NULL ptr\n", __func__);
		return;
	}
		

	if (height <= 0 || width <= 0 || (pixel_size != 1 && pixel_size != 2 && pixel_size != 4))
	{
		printf("%s input parameter height width or step is error\n", __func__);
		return;
	}

	if (pixel_size == 1)
	{
		if (channel_num == 1)
		{
			Mat image(height, width, CV_8UC1, image_data);
			imshow(window_name, image);
			waitKey(0);
			cvDestroyWindow(window_name);
		}
		else
		{
			Mat image(height, width, CV_8UC3, image_data);
			imshow(window_name, image);
			waitKey(0);
			cvDestroyWindow(window_name);
		}
	}
	else if (pixel_size == 2)
	{
		if (channel_num == 1)
		{
			Mat image(height, width, CV_16UC1, image_data);
			imshow(window_name, image);
			waitKey(0);
			cvDestroyWindow(window_name);
		}
		else
		{
			Mat image(height, width, CV_16UC3, image_data);
			imshow(window_name, image);
			waitKey(0);
			cvDestroyWindow(window_name);
		}
	}
	else
	{
		if (channel_num == 1)
		{
			Mat image(height, width, CV_32SC1, image_data);
			imshow(window_name, image);
			waitKey(0);
			cvDestroyWindow(window_name);
		}
		else
		{
			Mat image(height, width, CV_32SC3, image_data);
			imshow(window_name, image);
			waitKey(0);
			cvDestroyWindow(window_name);
		}

	}
#endif
}
