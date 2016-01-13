#pragma once

#include <iostream>
#include <vector>

class SimpleFilter {
public:
	static float* MedianFilter(float* list, int windowSize);
	static float* LowPassFilter(float* list, int windowSize);
	static float Average(float* list, int windowSize);
	static float WindowFilter(float* list, int windowSize);
	static float median3(float a, float b, float c) {
		if (a < b && a > c || a > b && a < c) return a;
		if (b < a && b > c || b > a && b < c) return b;
		if (c < a && c > b || c > a && c < b) return c;
	}
};
