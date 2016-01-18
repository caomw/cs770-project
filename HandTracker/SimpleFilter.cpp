#include "SimpleFilter.h"
#include <math.h>

float* SimpleFilter::MedianFilter(float* list, int windowSize) {
	float* retList = new float[windowSize-2];
	for (int i = 0; i < windowSize - 2; i++) {
		retList[i] = median3(list[i], list[i + 1], list[i + 2]);
	}

	return retList;
}

float SimpleFilter::Average(float* list, int windowSize) {
	float total = 0;
	for (int i = 0; i < windowSize; i++) {
		total += list[i];
	}
	return (float)total / float(windowSize);
}

float SimpleFilter::WindowFilter(float* list, int windowSize) {
	float av = Average(MedianFilter(list, windowSize), 3);
	return av;
}