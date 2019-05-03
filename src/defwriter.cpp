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
        m_ss << "E ;\n";
    }
    else if (item->m_location == "SE")
    {
        // South East orientation, rotation = 90 degrees
        //x += item->m_lefinfo->m_sy;
        toDEFCoordinates(x,y);
        m_ss << "    + PLACED ( " << x << " " << y << " ) ";
        m_ss << "W ;\n";
    }
    else if (item->m_location == "NE")
    {
        y -= item->m_lefinfo->m_sy;
        toDEFCoordinates(x,y);
        m_ss << "    + PLACED ( " << x << " " << y << " ) ";
        m_ss << "S ;\n";
    }
    else if (item->m_location == "SW")
    {
        toDEFCoordinates(x,y);
        m_ss << "    + PLACED ( " << x << " " << y << " ) ";
        m_ss << "N ;\n";
    }
    else if (item->m_location == "E")
    {
        x -= item->m_lefinfo->m_sy;
        toDEFCoordinates(x,y);
        m_ss << "    + PLACED ( " << x << " " << y << " ) ";

        if (!item->m_flipped) 
        {
            m_ss << " W" << " ;\n";
        }
        else
        {
            m_ss << " FE" << " ;\n";
        }
    }
    else if (item->m_location == "N")
    {
        y -= item->m_lefinfo->m_sy;
        toDEFCoordinates(x,y);
        m_ss << "    + PLACED ( " << x << " " << y << " ) ";
        if (!item->m_flipped) 
        {
            m_ss << " S" << " ;\n";
        }
        else
        {
            m_ss << " FS" << " ;\n";
        }
    }   
    else if (item->m_location == "S")
    {
        //y -= item->m_lefinfo->m_sy;
        toDEFCoordinates(x,y);
        m_ss << "    + PLACED ( " << x << " " << y << " ) ";
        if (!item->m_flipped)
        {
            m_ss << " N" << " ;\n";
        }
        else
        {
            m_ss << " FN" << " ;\n";
        }
    }        
    else
    {
        toDEFCoordinates(x,y);
        m_ss << "    + PLACED ( " << x << " " << y << " ) ";
        if (!item->m_flipped) 
        {
            m_ss << " E" << " ;\n";
        }
        else
        {
            m_ss << " W" << " ;\n";
        }
    }
}
