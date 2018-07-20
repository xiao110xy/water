#pragma once

void xyexp(const float *_x, float *y, int n);
void xyfastAtan2(const float *Y, const float *X, float *angle, int len, bool angleInDegrees = true);
void xymagnitude(const float* x, const float* y, float* mag, int len);
