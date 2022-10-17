#include <QCoreApplication>
#include <QApplication>
#include <QCommandLineParser>
#include <QFile>
#include <QImage>
#include <QTransform>
#include <QTextStream>

#include <chrono>
#include <cmath>
#include <cstdint>
#include <format>
#include <iostream>
#include <unordered_set>

#include "AreaInformation.h"
#include "RamerDouglasPeucker.h"
#include "SvgBuilder.h"

std::vector<QColor> makeColors(int areaCount) {
	std::vector<QColor> result;
	result.reserve(areaCount);

	int r = 0;
	int g = 0;
	int b = 0;
	int const increase = 252 / std::ceil(areaCount / 3.0);
	for (int i = 0; i < areaCount; ++i) {
		result.push_back(QColor(r, g, b));
		if (i % 3 == 0) {
			r += increase;
		} else if (i % 3 == 1) {
			g += increase;
		} else {
			b += increase;
		}
	}

	return result;
}

inline std::size_t posToVec(int x, int y, int width) {
	return y * width + x;
}

void detectAreas(QImage const& image, int const colourThreshold, int const areaSizeThreshold, double const epsilon) {
	int const width = image.width();
	int const height = image.height();
	std::vector<bool> imageBw(width * height, false);
	for (int w = 0; w < width; ++w) {
		for (int h = 0; h < height; ++h) {
			QRgb const rgb = image.pixel(w, h);
			imageBw[posToVec(w, h, width)] = (qRed(rgb) > colourThreshold && qGreen(rgb) > colourThreshold && qBlue(rgb) > colourThreshold);
		}
	}

	QImage bwImage(width, height, QImage::Format_RGB32);
	for (int w = 0; w < width; ++w) {
		for (int h = 0; h < height; ++h) {
			bwImage.setPixelColor(w, h, imageBw[posToVec(w, h, width)] ? Qt::GlobalColor::white : Qt::GlobalColor::black);
		}
	}
	bwImage.save("imageBw.png");

	AreaInformation areaInformation(width, height);
	for (int w = 0; w < width; ++w) {
		for (int h = 0; h < height; ++h) {
			// Always look left and up
			bool const hasLeftArea = (w > 0) && (imageBw[posToVec(w - 1, h, width)] == imageBw[posToVec(w, h, width)]);
			bool const hasTopArea = (h > 0) && (imageBw[posToVec(w, h - 1, width)] == imageBw[posToVec(w, h, width)]);
			if (hasLeftArea && hasTopArea) {
				int const topArea = areaInformation.getArea(w, h - 1);
				int const leftArea = areaInformation.getArea(w - 1, h);
				if (topArea == leftArea) {
					areaInformation.setArea(w, h, leftArea);
				} else {
					// Merge
					areaInformation.setArea(w, h, areaInformation.mergeAreas(topArea, leftArea));
				}
			} else if (hasLeftArea) {
				int const leftArea = areaInformation.getArea(w - 1, h);
				areaInformation.setArea(w, h, leftArea);
			} else if (hasTopArea) {
				int const topArea = areaInformation.getArea(w, h - 1);
				areaInformation.setArea(w, h, topArea);
			} else {
				areaInformation.setArea(w, h, areaInformation.addArea());
			}
		}
	}

	// How many areas for real?
	AreaInformation repackedAreas = areaInformation.packAreas();
	std::cout << "Used " << areaInformation.getAreaCount() << " areas, merged to a final amount of " << repackedAreas.getAreaCount() << " areas." << std::endl;
	
	bool hadChange = false;
	do {
		// Now, merge all area < X
		hadChange = false;
		for (int i = 0; i < repackedAreas.getAreaCount(); ++i) {
			if (repackedAreas.getAreaMemberCount(i) < areaSizeThreshold) {
				int const newArea = repackedAreas.getLargestNeighbourArea(i);
				if (newArea == -1) {
					std::cerr << "Internal Error: Area has no neighbours?!" << std::endl;
					throw;
				}
				repackedAreas.mergeAreas(i, newArea);
				hadChange = true;
			}
		}
		repackedAreas = repackedAreas.packAreas();
	} while (hadChange);
	std::cout << "Merging small areas brings us to a final amount of " << repackedAreas.getAreaCount() << " areas." << std::endl;

	for (int i = 0; i < repackedAreas.getAreaCount(); ++i) {
		std::cout << "\tArea " << i << " has " << repackedAreas.getAreaMemberCount(i) << " members." << std::endl;
	}

	std::vector<QColor> const colours = makeColors(repackedAreas.getAreaCount());
	std::cout << "Prepared " << colours.size() << " colours." << std::endl;

	QImage areaImage(width, height, QImage::Format_RGB32);
	for (int w = 0; w < width; ++w) {
		for (int h = 0; h < height; ++h) {
			int const resolvedArea = repackedAreas.getArea(w, h);
			areaImage.setPixelColor(w, h, colours[resolvedArea]);
		}
	}
	areaImage.save("imageArea.png");

	auto const listOfLinesPerArea = repackedAreas.getListOfLinesPerArea();
	
	std::vector<std::vector<Point>> outLines;
	std::size_t pointCountBeforeRdp = 0;
	std::size_t pointCountAfterRdp = 0;
	std::size_t linesRemovedFromDeduplication = 0;
	std::size_t maxPointCountPerLineBefore = 0;
	std::size_t maxPointCountPerLineAfter = 0;

	// We will save a single point of each line, and if another line comes closer than a small epsilon, we will consider them identical and remove one.
	std::vector<Point> deduplicationMarkerPoints;
	double const deduplicationEpsilon = 2.0;

	for (int i = 0; i < repackedAreas.getAreaCount(); ++i) {
		auto const& lines = listOfLinesPerArea.at(i);
		for (std::size_t j = 0; j < lines.size(); ++j) {
			auto const& line = lines.at(j);

			bool isDuplicate = false;
			if (deduplicationMarkerPoints.size() > 0) {
				for (std::size_t k = 0; (k < line.size()) && (!isDuplicate); ++k) {
					auto const& p = line.at(k);
					for (auto it = deduplicationMarkerPoints.cbegin(); it != deduplicationMarkerPoints.cend(); ++it) {
						if (AreaInformation::pointDistance(p, *it) < deduplicationEpsilon) {
							isDuplicate = true;
							break;
						}
					}
				}
			}
			
			if (isDuplicate) {
				++linesRemovedFromDeduplication;
				continue;
			}
			deduplicationMarkerPoints.push_back(*line.cbegin());

			pointCountBeforeRdp += line.size();
			if (line.size() > maxPointCountPerLineBefore) {
				maxPointCountPerLineBefore = line.size();
			}
			if (line.size() > 2) {
				std::vector<Point> out;
				RamerDouglasPeucker(line, epsilon, out);
				pointCountAfterRdp += out.size();
				if (out.size() > maxPointCountPerLineAfter) {
					maxPointCountPerLineAfter = out.size();
				}
				outLines.push_back(std::move(out));
			} else {
				pointCountAfterRdp += line.size();
				outLines.push_back(line);
			}
		}
	}
	std::cout << "We got " << outLines.size() << " lines (removed from deduplication: " << linesRemovedFromDeduplication << ") with " << pointCountBeforeRdp << " points (longest: " << maxPointCountPerLineBefore << " points, average: " << (pointCountBeforeRdp / outLines.size()) << " points)." << std::endl;
	std::cout << "After applying RDP, we have " << outLines.size() << " lines with " << pointCountAfterRdp << " points (longest: " << maxPointCountPerLineAfter << ", average: " << (pointCountAfterRdp / outLines.size()) << ")." << std::endl;

	double const targetW = 297.0;
	double const targetH = 210.0;
	SvgBuilder svgBuilder(width, height, targetW, targetH);

	QFile svgFile("image.svg");
	if (!svgFile.open(QFile::WriteOnly)) {
		std::cerr << "Failed to open SVG output!" << std::endl;
		throw;
	}
	svgFile.write(svgBuilder.buildSvgFromLines(outLines).toUtf8());
	svgFile.close();
	std::cout << "Wrote SVG file to disk." << std::endl;
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);

	QApplication::setApplicationName("EdgeFinder");
	QApplication::setApplicationVersion("1.0.0");

	QCommandLineParser parser;
	parser.setApplicationDescription("EdgeFinder");
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addPositionalArgument("image", QApplication::translate("main", "Path to input image"));
	parser.addOption(QCommandLineOption("epsilon", "Epsilon for the Ramer-Douglas-Peucker algoritm", "epsilon", "0.01"));
	parser.addOption(QCommandLineOption("areaSizeThreshold", "Threshold for small area deletion", "areaSizeThreshold", "500"));
	parser.addOption(QCommandLineOption("colourThreshold", "Threshold for  deciding between black and white", "colourThreshold", "64"));

	// Process the actual command line arguments given by the user
	parser.process(app);

	QStringList args = parser.positionalArguments();
	if (args.size() != 1) {
		std::cerr << "Missing command line arguments, quiting..." << std::endl;
		return 2;
	}

	QString const epsilonString = parser.value("epsilon");
	bool ok = false;
	double const epsilon = epsilonString.toDouble(&ok);
	if (!ok) {
		std::cerr << "Epsilon for RDP algorithmus could not be parsed: '" << epsilonString.toStdString() << "'" << std::endl;
		return -1;
	}
	std::cout << "Using epsilon = " << epsilon << " for the RDP algorithmus." << std::endl;

	QString const areaSizeThresholdString = parser.value("areaSizeThreshold");
	ok = false;
	int const areaSizeThreshold = areaSizeThresholdString.toInt(&ok);
	if (!ok) {
		std::cerr << "Threshold for small area deletion could not be parsed: '" << areaSizeThresholdString.toStdString() << "'" << std::endl;
		return -1;
	}
	std::cout << "Using threshold = " << areaSizeThreshold << " for small area deletion." << std::endl;

	QString const colourThresholdString = parser.value("colourThreshold");
	ok = false;
	int const colourThreshold = colourThresholdString.toInt(&ok);
	if (!ok) {
		std::cerr << "Threshold for black/white decision could not be parsed: '" << colourThresholdString.toStdString() << "'" << std::endl;
		return -1;
	}
	std::cout << "Using threshold = " << colourThreshold << " for black/white decision (every RGB component > threshold => white)." << std::endl;

	if (!QFile::exists(args[0])) {
		std::cerr << "Input image '" << args[0].toStdString() << "' does not exist!" << std::endl;
		return -1;
	}
	QImage image(args[0]);
	std::cout << "Input image has dimensions " << image.width() << " x " << image.height() << "." << std::endl;

	detectAreas(image, colourThreshold, areaSizeThreshold, epsilon);

	std::cout << "Bye bye!" << std::endl;
	return 0;
}

#ifdef _MSC_VER
int __stdcall WinMain(struct HINSTANCE__* hInstance, struct HINSTANCE__* hPrevInstance, char* lpszCmdLine, int nCmdShow) {
	return main(__argc, __argv);
}

#endif
