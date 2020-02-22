#ifndef __HILBERT_H__
#define __HILBERT_H__

#include <vector>

typedef std::pair<int, int> pt_t;
typedef std::vector<pt_t> curve_t;

void gilbert2d(int width, int height, curve_t &curve);

#endif