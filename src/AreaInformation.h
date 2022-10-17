#ifndef EDGEFINDER_AREAINFORMATION_H_
#define EDGEFINDER_AREAINFORMATION_H_

#include <cstdint>
#include <unordered_map>
#include <unordered_set>
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
private:
	int m_w;
	int m_h;

	std::vector<int> m_areas;
	std::unordered_map<int, int> m_mergedAreas;
	int m_areaCounter;
	std::vector<int> m_areaMembers;
	std::vector<std::unordered_set<int>> m_areaNeighbours;
};

#endif
