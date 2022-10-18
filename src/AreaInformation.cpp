#include "AreaInformation.h"

#include <array>
#include <iostream>
#include <list>
#include <set>
#include <stack>

AreaInformation::AreaInformation(int width, int height) : m_w(width), m_h(height), m_areas(width* height, -1), m_mergedAreas(), m_areaCounter(0) {
	//
}

int AreaInformation::getArea(int x, int y) const {
	int const area = m_areas.at(posToVec(x, y, m_w));
	if (area < 0) {
		std::cerr << "Internal Error: Area was not set yet!" << std::endl;
		throw;
	}
	return resolveArea(area);
}

void AreaInformation::setArea(int x, int y, int area) {
	if (m_mergedAreas.contains(area)) {
		std::cerr << "Internal Error: Used area was not unmerged!" << std::endl;
		throw;
	}
	m_areas[posToVec(x, y, m_w)] = area;

	// Check surrounding neighbours
	bool const isTopDifferent = (y > 0) && (getArea(x, y - 1) != area);
	if (isTopDifferent) {
		m_areaNeighbours[area].insert(getArea(x, y - 1));
	}
	bool const isLeftDifferent = (x > 0) && (getArea(x - 1, y) != area);
	if (isLeftDifferent) {
		m_areaNeighbours[area].insert(getArea(x - 1, y));
	}

	m_areaMembers[area]++;
}

int AreaInformation::resolveArea(int area) const {
	while (m_mergedAreas.contains(area)) {
		area = m_mergedAreas.at(area);
	}
	return area;
}

int AreaInformation::mergeAreas(int topArea, int leftArea) {
	m_mergedAreas.insert(std::make_pair(topArea, leftArea));
	for (auto it = m_mergedAreas.begin(); it != m_mergedAreas.end(); ++it) {
		if (it->second == topArea) {
			it->second = leftArea;
		}
	}
	m_areaMembers[leftArea] += m_areaMembers[topArea];
	m_areaNeighbours[leftArea].insert(m_areaNeighbours[topArea].cbegin(), m_areaNeighbours[topArea].cend());
	return leftArea;
}

int AreaInformation::addArea() {
	int const newArea = m_areaCounter;
	++m_areaCounter;
	m_areaMembers.push_back(0);
	if (m_areaMembers.size() != m_areaCounter) {
		std::cerr << "Internal Error: Area Counter and membership vector out of sync!" << std::endl;
		throw;
	}
	m_areaNeighbours.push_back({});
	if (m_areaNeighbours.size() != m_areaCounter) {
		std::cerr << "Internal Error: Area Counter and neighbour vector out of sync!" << std::endl;
		throw;
	}
	return newArea;
}

int AreaInformation::getAreaCount() const {
	return m_areaCounter;
}

int AreaInformation::getAreaMemberCount(int area) const {
	return m_areaMembers.at(area);
}

int AreaInformation::getLargestNeighbourArea(int area) const {
	int largestNeighbourAreaId = -1;
	int largestNeighbourSize = -1;
	for (auto it = m_areaNeighbours[area].cbegin(); it != m_areaNeighbours[area].cend(); ++it) {
		int const neighbourSize = getAreaMemberCount(*it);
		if (neighbourSize > largestNeighbourSize) {
			largestNeighbourSize = neighbourSize;
			largestNeighbourAreaId = *it;
		}
	}
	return largestNeighbourAreaId;
}

AreaInformation AreaInformation::packAreas() const {
	AreaInformation result(m_w, m_h);		
	std::unordered_map<int, int> indexMap;

	for (int w = 0; w < m_w; ++w) {
		for (int h = 0; h < m_h; ++h) {
			int const area = m_areas.at(posToVec(w, h, m_w));
			if (area < 0) {
				std::cerr << "Internal Error: Area was not set yet!" << std::endl;
				throw;
			}
			int const resolvedArea = resolveArea(area);
				
			if (!indexMap.contains(resolvedArea)) {
				indexMap.insert(std::make_pair(resolvedArea, result.addArea()));
			}
			result.setArea(w, h, indexMap.at(resolvedArea));
		}
	}
	return result;
}

#define LINESEARCH_NEIGHBOUR_COUNT 8u
class LineSearchStackFrame {
public:
	LineSearchStackFrame(IPoint const& point) : m_index(0), m_neighbours({
			std::make_pair(point.first - 1, point.second),
			std::make_pair(point.first, point.second - 1),
			std::make_pair(point.first + 1, point.second),
			std::make_pair(point.first, point.second + 1),
			std::make_pair(point.first - 1, point.second - 1),
			std::make_pair(point.first + 1, point.second - 1),
			std::make_pair(point.first + 1, point.second + 1),
			std::make_pair(point.first - 1, point.second + 1)
		}), m_points({ point }) {
		//
	}

	// For the root frame, no points, only a list
	LineSearchStackFrame() : m_index(getNeighbourCount() + 5), m_neighbours(), m_points() {}

	bool hasNextNeighbour() const {
		return m_index < getNeighbourCount();
	}

	IPoint const& getNextNeighbour() {
		return m_neighbours[m_index++];
	}

	std::list<IPoint> const& getPointList() const {
		return m_points;
	}

	std::list<IPoint>& getPointList() {
		return m_points;
	}

	static constexpr inline std::uint8_t getNeighbourCount() {
		return LINESEARCH_NEIGHBOUR_COUNT;
	}
private:
	std::uint8_t m_index;
	std::array<IPoint, LINESEARCH_NEIGHBOUR_COUNT> m_neighbours;
	std::list<IPoint> m_points;
};

std::list<IPoint> AreaInformation::addPointIterative(IPoint const& start, std::set<IPoint>& points, std::size_t& maxStackSize) const {
	std::stack<LineSearchStackFrame> stack;
	stack.push(LineSearchStackFrame(start));
	points.erase(start);

	while (!stack.empty()) {
		if (stack.size() > maxStackSize) {
			maxStackSize = stack.size();
		}

		auto& frame = stack.top();
		if (frame.hasNextNeighbour()) {
			IPoint const& nextNeighbour = frame.getNextNeighbour();
			auto it = points.find(nextNeighbour);
			if (it != points.end()) {
				points.erase(it);

				// Push a new List frame
				stack.push(LineSearchStackFrame(nextNeighbour));
			}
		} else {
			// My frame is now on top, merge with parent and pop
			std::list<IPoint> const nextPoints = stack.top().getPointList();
			stack.pop();
			if (stack.empty()) {
				// Root Frame
				return nextPoints;
			}

			std::list<IPoint>& ourPoints = stack.top().getPointList();
			double const dist_A = pointDistance(*ourPoints.rbegin(), *nextPoints.cbegin());	/* Our end, their front */
			double const dist_B = pointDistance(*ourPoints.rbegin(), *nextPoints.rbegin());	/* Our end, their end */
			double const dist_C = pointDistance(*ourPoints.cbegin(), *nextPoints.cbegin());	/* Our front, their front */
			double const dist_D = pointDistance(*ourPoints.cbegin(), *nextPoints.rbegin());	/* Our front, their end */
			if ((dist_A <= dist_B) && (dist_A <= dist_C) && (dist_A <= dist_D)) {
				ourPoints.insert(ourPoints.end(), nextPoints.cbegin(), nextPoints.cend());
			} else if ((dist_B <= dist_A) && (dist_B <= dist_C) && (dist_B <= dist_D)) {
				ourPoints.insert(ourPoints.end(), nextPoints.rbegin(), nextPoints.rend());
			} else if ((dist_C <= dist_A) && (dist_C <= dist_B) && (dist_C <= dist_D)) {
				ourPoints.insert(ourPoints.begin(), nextPoints.rbegin(), nextPoints.rend());
			} else {
				ourPoints.insert(ourPoints.begin(), nextPoints.cbegin(), nextPoints.cend());
			}
		}
	}

	std::cerr << "Internal Error: Control flow reached the end of the stack without returning, this should never happen!" << std::endl;
	throw;
}

std::vector<std::vector<std::vector<Point>>> AreaInformation::getListOfLinesPerArea() const {
	std::vector<std::vector<std::vector<Point>>> result;

	std::vector<std::set<IPoint>> boundingsPoints;
	boundingsPoints.resize(getAreaCount());
	for (int w = 0; w < m_w; ++w) {
		for (int h = 0; h < m_h; ++h) {
			int const myArea = getArea(w, h);
			if ((w > 0) && (getArea(w - 1, h) != myArea)) {
				boundingsPoints[myArea].insert(std::make_pair(w, h));
				boundingsPoints[getArea(w - 1, h)].insert(std::make_pair(w - 1, h));
			}
			if ((h > 0) && (getArea(w, h - 1) != myArea)) {
				boundingsPoints[myArea].insert(std::make_pair(w, h));
				boundingsPoints[getArea(w, h - 1)].insert(std::make_pair(w, h - 1));
			}
		}
	}

	std::size_t maxStackSize = 0;
	for (int i = 0; i < boundingsPoints.size(); ++i) {
		std::set<IPoint> points = boundingsPoints.at(i);
		std::vector<std::vector<Point>> orderedPointsList;
		while (points.size() > 0) {
			// Pick a point, form a line.
			auto const point = *points.cbegin();
			auto const line = addPointIterative(point, points, maxStackSize);

			// Convert types
			std::vector<Point> convertedLine;
			convertedLine.reserve(line.size());
			for (auto it = line.cbegin(); it != line.cend(); ++it) {
				convertedLine.push_back(std::make_pair(static_cast<PointType>(it->first), static_cast<PointType>(it->second)));
			}

			orderedPointsList.push_back(convertedLine);
		}

		result.push_back(orderedPointsList);
	}
	std::cout << "Maximum stack depth was " << maxStackSize << "." << std::endl;

	return result;
}
