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

#ifndef svgwriter_h
#define svgwriter_h

#include <stdio.h>
#include <stdint.h>
#include <complex>
#include <string>

#include "layout.h"

/** a very minimal SVG writer */
class SVGWriter
{
public:
    SVGWriter(std::ostream &os, uint32_t width, uint32_t height);
    virtual ~SVGWriter();

    void writeCell(const LayoutItem *item);

protected:
    std::complex<double> toSVGCoordinates(std::complex<double> &p) const;

    void writeHeader();
    void writeFooter();

    std::ostream &m_svg;
    uint32_t m_width;
    uint32_t m_height;
};

#endif