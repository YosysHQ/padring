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

#ifndef defwriter_h
#define defwriter_h

#include <stdio.h>
#include <stdint.h>
#include <complex>
#include <string>
#include <sstream>

#include "layout.h"

/** a very minimal SVG writer */
class DEFWriter
{
public:
    DEFWriter(std::ostream &os, uint32_t width, uint32_t height);
    virtual ~DEFWriter();

    void writeCell(const LayoutItem *item);

    void setDatabaseUnits(double databaseUnits)
    {
        m_databaseUnits = databaseUnits;
    }

    void setDesignName(const std::string &designName)
    {
        m_designName = designName;
    }

protected:

    /** convert to DEF database units / coordinates.
        this function will issue a warning when
        m_databaseUnits has not been set and set it
        to 1000.
    */
    void toDEFCoordinates(double &x, double &y);

    void writeToFile();

    std::stringstream   m_ss;
    std::string         m_designName;
    std::ostream        &m_def;
    
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_cellCount;
    double   m_databaseUnits;
};

#endif