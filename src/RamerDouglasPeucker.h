#ifndef EDGEFINDER_RAMERDOUGLASPEUCKER_H_
#define EDGEFINDER_RAMERDOUGLASPEUCKER_H_

#include <cstdint>
#include <vector>

#include "Point.h"

void RamerDouglasPeucker(std::vector<Point> const& pointList, double epsilon, std::vector<Point>& out);

#endif
