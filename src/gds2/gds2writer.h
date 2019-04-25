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

#ifndef gds2writer_h
#define gds2writer_h

#include <stdio.h>
#include <stdint.h>
#include <string>

class GDS2Writer
{
public:
    static GDS2Writer* open(
        const std::string &filename,
        const std::string &designName);

    virtual ~GDS2Writer();

    enum orientation_t
    {
        ROT0,
        ROT90,
        ROT180,
        ROT270,
        FLIPY
    };

    /** Write a structural reference (SREF) to the GDS2
        that places a cell.
    */
    void writeCell(const std::string &cellName, int32_t x, int32_t y, 
        orientation_t orientation = ROT0);

protected:
    void writeHeader();
    void writeEpilog();
    
    void writeUint32(uint32_t v);
    void writeUint16(uint16_t v);
    void writeUint8(uint8_t v);
    void writeInt32(uint32_t v);
    void writeInt16(uint16_t v);
    void writeFloat32(float v);
    void writeFloat64(double v);

    // returns the number of bytes written
    uint32_t writeString(const std::string &str);

    GDS2Writer(FILE *f, const std::string &designName);
    
    FILE        *m_fout;        ///< GDS2 file handle
    uint32_t    m_words;        ///< words written
    std::string m_designName;   ///< set the design name
};

#endif