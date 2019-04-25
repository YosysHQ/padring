/*
    PADRING -- a padring generator for ASICs.

    Copyright (c) 2019, Niels Moseley <niels@symbioticeda.com>

    Permission to use, copy, modify, and/or distribute this software for any
    purpose with or without fee is hereby granted, provided that the above
    copyright notice and this permission notice appear in all copies.

    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
    
*/

#include <sstream>
#include <fstream>
#include <iomanip>
#include <complex>
#include <math.h>
#include <assert.h>
#include "logging.h"
#include "defwriter.h"

DEFWriter::DEFWriter(std::ostream &os, uint32_t width, uint32_t height)
    : m_def(os),
      m_width(width),
      m_height(height),
      m_cellCount(0),
      m_databaseUnits(0.0)
{
    // make sure the stringstream doesn't use
    // exponential notation with doubles!
    m_ss << std::setprecision(std::numeric_limits<double>::digits10);
}

DEFWriter::~DEFWriter()
{
    m_def.flush();    
    writeToFile();
}

void DEFWriter::writeToFile()
{
    assert(!m_designName.empty());

    m_def << "DESIGN " << m_designName << " ;\n";
    m_def << "UNITS DISTANCE MICRONS " << m_databaseUnits << " ; \n";
    m_def << "COMPONENTS " << m_cellCount << " ;\n";

    m_def << m_ss.str();

    m_def << "END COMPONENTS\n";
    m_def << "END DESIGN\n";
}


void DEFWriter::toDEFCoordinates(double &x, double &y)
{
    //return std::complex<double>(p.real(), m_height - p.imag());
    //FIXME: use database units defined in LEF file!

    if (m_databaseUnits < 1e-12)
    {
        doLog(LOG_WARN, "DEF database units not set! does your imported LEF file specify it?\n");
        doLog(LOG_WARN, "  Assuming the value is 100.0\n");
        m_databaseUnits = 100.0;
    }

    x *= m_databaseUnits;
    y *= m_databaseUnits;
}

void DEFWriter::writeCell(const LayoutItem *item)
{
    if (item == nullptr)
    {
        return;
    }

    double rot = 0.0;
    double x = item->m_x;
    double y = item->m_y;

    m_cellCount++;
    
    if (item->m_ltype == LayoutItem::TYPE_FILLER)
    {
        m_ss << "  - FILLER_" << m_cellCount << " " << item->m_cellname << "\n";
    }
    else
    {
        m_ss << "  - " << item->m_instance << " " << item->m_cellname << "\n";
    }
    // do corners
    if (item->m_location == "NW")
    {
        y -= item->m_lefinfo->m_sy;
        toDEFCoordinates(x,y);
        m_ss << "    + PLACED ( " << x << " " << y << " ) ";
        m_ss << "N ;\n";
    }
    else if (item->m_location == "SE")
    {
        // South East orientation, rotation = 90 degrees
        //x += item->m_lefinfo->m_sy;
        toDEFCoordinates(x,y);
        m_ss << "    + PLACED ( " << x << " " << y << " ) ";
        m_ss << "E ;\n";
    }
    else if (item->m_location == "NE")
    {
        y -= item->m_lefinfo->m_sy;
        toDEFCoordinates(x,y);
        m_ss << "    + PLACED ( " << x << " " << y << " ) ";
        m_ss << "E ;\n";
    }
    else if (item->m_location == "SW")
    {
        toDEFCoordinates(x,y);
        m_ss << "    + PLACED ( " << x << " " << y << " ) ";
        m_ss << "S ;\n";
    }
    else if (item->m_location == "E")
    {
        x -= item->m_lefinfo->m_sy;
        toDEFCoordinates(x,y);
        m_ss << "    + PLACED ( " << x << " " << y << " ) ";
        m_ss << " W" << " ;\n";
    }
    else if (item->m_location == "N")
    {
        y -= item->m_lefinfo->m_sy;
        toDEFCoordinates(x,y);
        m_ss << "    + PLACED ( " << x << " " << y << " ) ";
        m_ss << " S" << " ;\n";
    }   
    else if (item->m_location == "S")
    {
        //y -= item->m_lefinfo->m_sy;
        toDEFCoordinates(x,y);
        m_ss << "    + PLACED ( " << x << " " << y << " ) ";
        m_ss << " N" << " ;\n";
    }        
    else
    {
        toDEFCoordinates(x,y);
        m_ss << "    + PLACED ( " << x << " " << y << " ) ";
        m_ss << " E" << " ;\n";
    }
    

#if 0
    // regular cells have N,S,E,W,
    // corner cells have NE,NW,SE,SW
    if (item->m_location == "N")
    {
        // North orientation, rotation = 180 degrees
        x += item->m_lefinfo->m_sx;
        rot = 180.0;
    }
    else if (item->m_location == "S")
    {
        // South orientation, rotation = 0 degrees
        //y += item->m_lefinfo->m_sy;
    }
    else if (item->m_location == "E")
    {
        // East orientation
        rot = 90.0;
    }
    else if (item->m_location == "W")
    {
        // West 
        y += item->m_lefinfo->m_sx;
        rot = 270.0;
    }

    // do corners
    if (item->m_location == "NW")
    {
        // North West orientation, rotation = 270 degrees
        rot = 270.0;
    }
    else if (item->m_location == "SE")
    {
        // South East orientation, rotation = 90 degrees
        x += item->m_lefinfo->m_sy;
        rot = 90.0;
    }
    else if (item->m_location == "NE")
    {
        x += item->m_lefinfo->m_sx;
        rot = 180.0;
    }

    std::complex<double> ll = {0.0,0.0};
    std::complex<double> ul = {0.0,item->m_lefinfo->m_sy};
    std::complex<double> ur = {item->m_lefinfo->m_sx,item->m_lefinfo->m_sy};
    std::complex<double> lr = {item->m_lefinfo->m_sx,0.0};

    std::complex<double> rr = {cos(3.1415927*rot/180.0), sin(3.1415927*rot/180.0)};

    ll *= rr;
    ul *= rr;
    ur *= rr;
    lr *= rr;

    ll += std::complex<double>(x,y);
    ul += std::complex<double>(x,y);
    ur += std::complex<double>(x,y);
    lr += std::complex<double>(x,y);

    ll = toSVGCoordinates(ll);
    ul = toSVGCoordinates(ul);
    ur = toSVGCoordinates(ur);
    lr = toSVGCoordinates(lr);

    m_svg << "<polyline points=\"";
    m_svg << ll.real() << " " << ll.imag() << " ";
    m_svg << ul.real() << " " << ul.imag() << " ";
    m_svg << ur.real() << " " << ur.imag() << " ";
    m_svg << lr.real() << " " << lr.imag() << " ";
    m_svg << ll.real() << " " << ll.imag() << "\" ";

    //
    // colour palette
    //
    //  #BFE1F3 light blue
    //  #179AA9 blue
    //  #AAD355 green
    //  #F9C908 yellow
    //  #F25844 red
    // 

    //m_svg << "<rect x=\"" << x << "\" y=\"" << m_height-y << "\" ";
    //m_svg << "width=\"" << sx << "\" height=\"" << sy << "\" ";
    if (item->m_ltype == LayoutItem::TYPE_FILLER)
    {
        m_svg << "style=\"fill:#BFE1F3;stroke:#179AA9;stroke-width:0.25\" />\n";
    }
    else
    {
        m_svg << "style=\"fill:#FAAD355;stroke:#F25844;stroke-width:0.25\" />\n";
    }

    if (item->m_ltype == LayoutItem::TYPE_CORNER)
    {
        m_svg << "<circle cx=\"" << ll.real() << "\" cy=\"" << ll.imag() <<  "\" r=\"" << 5.0 << "\" style=\"fill:#000000\" />\n";
    }

    std::complex<double> center = (ll + ur) / 2.0;
    if (item->m_ltype != LayoutItem::TYPE_FILLER)
    {
        m_svg << "<text text-anchor=\"middle\" x=\"" << center.real() << "\" y=\"" << center.imag() << "\" class=\"small\">" << item->m_cellname << "</text>\n";
    }
#endif
}
