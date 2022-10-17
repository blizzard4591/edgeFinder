#ifndef EDGEFINDER_SVGBUILDER_H_
#define EDGEFINDER_SVGBUILDER_H_

#include <cstdint>
#include <iostream>
#include <vector>

#include <QString>

#include "Point.h"

class SvgBuilder {
public:
	SvgBuilder(int w, int h, double tW, double tH) : m_imageWidth(w), m_imageHeight(h), m_targetWidth(tW), m_targetHeight(tH), m_pathIdCounter(176) {
		//
	}

	QString buildSvgFromLines(std::vector<std::vector<Point>> const& lines);
private:
	int const m_imageWidth;
	int const m_imageHeight;
	double const m_targetWidth;
	double const m_targetHeight;
	std::size_t m_pathIdCounter;

	inline Point scalePoint(Point const& p) const {
		//std::cout << "x = " << p.first << " -> " << ((p.first / m_imageWidth) * m_targetWidth) << ", y = " << p.second << " -> " << ((p.second / m_imageHeight) * m_targetHeight) << std::endl;
		return std::make_pair((p.first / m_imageWidth) * m_targetWidth, (p.second / m_imageHeight) * m_targetHeight);
	}
	void appendLine(QString& out, std::vector<Point> const& line);
};


#endif
