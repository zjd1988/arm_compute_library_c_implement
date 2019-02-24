#ifndef COMMON_H
#define COMMON_H
#include <stdio.h>
#include <stdlib.h>
#ifdef __cplusplus
extern "C" {
#endif
//#define USE_OPENMP
#define USE_CLOCK

#ifdef USE_OPENMP
#include <omp.h>
#endif

#ifdef USE_CLOCK
#include <time.h>

clock_t start, finish;
double duration;
#endif

#define ALIGN_FOUR_BYTES 4
#define ALIGN_EIGHT_BYTES 8
#define ALIGN_SIXTEEN_BYTES 16

void *align_malloc(size_t size, int aligned);
void align_free(void *data);


#ifdef __cplusplus
}
#endif

#endif //COMMON_H
