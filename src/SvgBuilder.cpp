#include "SvgBuilder.h"

QString SvgBuilder::buildSvgFromLines(std::vector<std::vector<Point>> const& lines) {
	QString result;
	result.reserve(16 * 1024 * 1024); // 16 MB
    result.append(R"F00BAR(<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<!-- Created with Inkscape (http://www.inkscape.org/) -->

<svg
   width="297mm"
   height="210mm"
   viewBox="0 0 297 210"
   version="1.1"
   id="svg5"
   inkscape:version="1.2.1 (9c6d41e410, 2022-07-14)"
   sodipodi:docname="test.svg"
   xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
   xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
   xmlns="http://www.w3.org/2000/svg"
   xmlns:svg="http://www.w3.org/2000/svg">
  <sodipodi:namedview
     id="namedview7"
     pagecolor="#ffffff"
     bordercolor="#666666"
     borderopacity="1.0"
     inkscape:showpageshadow="2"
     inkscape:pageopacity="0.0"
     inkscape:pagecheckerboard="0"
     inkscape:deskcolor="#d1d1d1"
     inkscape:document-units="mm"
     showgrid="false"
     inkscape:zoom="0.914906"
     inkscape:cx="396.76207"
     inkscape:cy="557.98082"
     inkscape:window-width="3840"
     inkscape:window-height="2066"
     inkscape:window-x="-11"
     inkscape:window-y="-11"
     inkscape:window-maximized="1"
     inkscape:current-layer="layer1" />
)F00BAR");
    for (auto it = lines.cbegin(); it != lines.cend(); ++it) {
        auto const& line = *it;
        appendLine(result, line);
    }
    result.append(R"F00BAR(  </g>
</svg>
)F00BAR");
    return result;
}

void SvgBuilder::appendLine(QString& out, std::vector<Point> const& line) {
    out.append(R"F00BAR(    <path
       style="fill:none;stroke:#ff0000;stroke-width:0.26458333;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1;stroke-dasharray:none;vector-effect:non-scaling-stroke;-inkscape-stroke:hairline"
       d="M)F00BAR");
    for (auto it = line.cbegin(); it != line.cend(); ++it) {
        Point const p = scalePoint (*it);
        out.append(' ');
        out.append(QString::number(p.first, 'f'));
        out.append(',');
        out.append(QString::number(p.second, 'f'));
    }
    out.append(QString(R"F00BAR("
       id="path%1" />
)F00BAR").arg(m_pathIdCounter));
    ++m_pathIdCounter;
}