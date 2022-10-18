#ifndef EDGEFINDER_AREAINFORMATION_H_
#define EDGEFINDER_AREAINFORMATION_H_

#include <cstdint>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <vector>

#include "Point.h"

typedef std::pair<int, int> IPoint;

class AreaInformation {
public:
	AreaInformation(int width, int height);

	int getArea(int x, int y) const;

	void setArea(int x, int y, int area);

	int resolveArea(int area) const;

	int mergeAreas(int topArea, int leftArea);

	int addArea();

	int getAreaCount() const;

	int getAreaMemberCount(int area) const;

	int getLargestNeighbourArea(int area) const;

	AreaInformation packAreas() const;

	std::vector<std::vector<std::vector<Point>>> getListOfLinesPerArea() const;

	static inline std::size_t posToVec(int x, int y, int width) {
		return y * width + x;
	}

	static inline double pointDistance(IPoint const& a, IPoint const& b) {
		double const d_x = a.first - b.first;
		double const d_y = a.second - b.second;
		return std::sqrt(d_x * d_x + d_y * d_y);
	}
private:
	int m_w;
	int m_h;

	std::vector<int> m_areas;
	std::unordered_map<int, int> m_mergedAreas;
	int m_areaCounter;
	std::vector<int> m_areaMembers;
	std::vector<std::unordered_set<int>> m_areaNeighbours;

	std::list<IPoint> addPointIterative(IPoint const& start, std::set<IPoint>& points, std::size_t& maxStackSize) const;
};

#endif
