#include "common.h"


void *align_malloc(size_t size, int aligned)
{
	// aligned is a power of 2
	if ((aligned & (aligned - 1)) != 0)
	{
		return NULL;
	}

	//分配内存空间
	void *data = malloc(sizeof(void *) + aligned + size);


	//地址对齐
	void **temp = (void **)data + 1;
	void **alignedData = (void **)(((size_t)temp + aligned - 1) & (-aligned));


	//保存原始内存地址
	alignedData[-1] = data;

	return alignedData;
}


void align_free(void *data)
{
	if (data)
	{
		free(((void **)data)[-1]);
	}
}