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

#include "gds2writer.h"

GDS2Writer* GDS2Writer::open(const std::string &filename)
{
    FILE *f = fopen(filename.c_str(), "wb");
    if (f == nullptr)
    {
        return nullptr;
    }

    return new GDS2Writer(f);
}

GDS2Writer::GDS2Writer(FILE *f) : m_fout(f)
{   
    writeHeader(); 
}

GDS2Writer::~GDS2Writer()
{
    writeEpilog();
    fclose(m_fout);
}

inline void endian_swap(uint16_t &x)
{
    x = (x>>8) | 
        (x<<8);
}

inline void endian_swap(int16_t &x)
{
    uint16_t y = static_cast<uint16_t>(x);
    x = static_cast<uint16_t>((y>>8) | (y<<8));
}

inline void endian_swap(uint32_t &x)
{
    x = (x>>24) | 
        ((x<<8) & 0x00FF0000) |
        ((x>>8) & 0x0000FF00) |
        (x<<24);
}

inline void endian_swap(int32_t &x)
{
    uint32_t y = static_cast<uint16_t>(x);
    x = static_cast<uint32_t>((y>>24) | 
        ((y<<8) & 0x00FF0000) |
        ((y>>8) & 0x0000FF00) |
        (y<<24));
}

inline void endian_swap(uint64_t &x)
{
    x = (x>>56) | 
        ((x<<40) & 0x00FF000000000000) |
        ((x<<24) & 0x0000FF0000000000) |
        ((x<<8)  & 0x000000FF00000000) |
        ((x>>8)  & 0x00000000FF000000) |
        ((x>>24) & 0x0000000000FF0000) |
        ((x>>40) & 0x000000000000FF00) |
        (x<<56);
}

void GDS2Writer::writeUint32(uint32_t v)
{
    endian_swap(v);
    fwrite(&v, sizeof(v), 1, m_fout);
}

void GDS2Writer::writeUint16(uint16_t v)
{
    endian_swap(v);
    fwrite(&v, sizeof(v), 1, m_fout);
}

void GDS2Writer::writeUint8(uint8_t v)
{
    fwrite(&v, sizeof(v), 1, m_fout);
}

void GDS2Writer::writeInt32(uint32_t v)
{
    endian_swap(v);
    fwrite(&v, sizeof(v), 1, m_fout);
}

void GDS2Writer::writeInt16(uint16_t v)
{
    endian_swap(v);
    fwrite(&v, sizeof(v), 1, m_fout);
}

void GDS2Writer::writeFloat32(float v)
{
    uint32_t *ptr = reinterpret_cast<uint32_t*>(&v);
    endian_swap(*ptr);
    fwrite(ptr, sizeof(v), 1, m_fout);
}

void GDS2Writer::writeFloat64(double v)
{
    uint64_t *ptr = reinterpret_cast<uint64_t*>(&v);
    endian_swap(*ptr);
    fwrite(ptr, sizeof(v), 1, m_fout);
}

uint32_t GDS2Writer::writeString(const std::string &str)
{
    uint32_t bytes = str.size();
    for(auto c : str)
    {
        fputc(c, m_fout);
    }
    if ((str.size() % 2) == 1)
    {
        fputc(0, m_fout);
        bytes++;
    }
    return bytes;
}

void GDS2Writer::writeHeader()
{
    // HEADER record
    writeUint16(0x0006);    // Len = 6 bytes
    writeUint16(0x0002);    // HEADER id
    writeUint16(0x0003);    // version 3?

    // BGNLIB
    writeUint16(0x001C);    // Len
    writeUint16(0x0102);    // BGNLIB id
    writeUint16(0x0000);    // year (last modified)
    writeUint16(0x0000);    // month
    writeUint16(0x0000);    // day
    writeUint16(0x0000);    // hour
    writeUint16(0x0000);    // minute
    writeUint16(0x0000);    // second
    writeUint16(0x0000);    // year (last accessed)
    writeUint16(0x0000);    // month
    writeUint16(0x0000);    // day
    writeUint16(0x0000);    // hour
    writeUint16(0x0000);    // minute
    writeUint16(0x0000);    // second    

    // LIBNAME
    writeUint16(0x0012);    // Len 18
    writeUint16(0x0206);    // LIBNAME id
    writeUint16(0x4141);
    writeUint16(0x4141);
    writeUint16(0x4141);
    writeUint16(0x4141);
    writeUint16(0x4141);
    writeUint16(0x4141);
    writeUint16(0x4141);

    // UNITS
    writeUint16(0x014);     // two 8-byte real
    writeUint16(0x0305);    // UNITS id
    writeUint32(0x3E418937);
    writeUint32(0x4BC6A7EF);    
    writeUint32(0x3944B82F);
    writeUint32(0xA09B5A54);    
    
    // BGNSTR
    writeUint16(0x001C);    // Len
    writeUint16(0x0502);    // BGNSTR id
    writeUint16(0x0000);    // year (last modified)
    writeUint16(0x0000);    // month
    writeUint16(0x0000);    // day
    writeUint16(0x0000);    // hour
    writeUint16(0x0000);    // minute
    writeUint16(0x0000);    // second
    writeUint16(0x0000);    // year (last accessed)
    writeUint16(0x0000);    // month
    writeUint16(0x0000);    // day
    writeUint16(0x0000);    // hour
    writeUint16(0x0000);    // minute
    writeUint16(0x0000);    // second

    // STRNAME
    writeUint16(0x000C);    // Len
    writeUint16(0x0606);    // STRNAME id
    writeString("EXAMPLE");
}

void GDS2Writer::writeEpilog()
{
    // ENDSTR
    writeUint16(0x0004);    // Len = 4
    writeUint16(0x0700);    // ENDSTR id

    // ENDLIB
    writeUint16(0x0004);    // Len = 4
    writeUint16(0x0400);    // ENDLIB id
}

void GDS2Writer::writeCell(const std::string &cellName, int32_t x, int32_t y, orientation_t orientation)
{
    // SREF
    writeUint16(0x0004);    // Len
    writeUint16(0x0A00);    // SREF id

    // SNAME
    uint32_t bytes = cellName.size() + (cellName.size() % 2);
    writeUint16(bytes+4);   // Len
    writeUint16(0x1206);    // SNAME
    writeString(cellName);

    // STRANS
    //writeUint16(6);
    //writeUint16(0x1A01);    // STRANS id
    //writeInt32(x);
    //writeInt32(y);

/*

    7-bit exponent = + 64
    mantissa*16^exponent

    mantissa is always >=1/16 and <1

    exp = floor(log(v)/log(16))
    mant = 1e-3 / 16^exp

    90 -> exp = 2, mantissa = 5A000000
    180 -> exp = 2, mantissa = B4000000
    270 -> exp = 3, mantissa = 10E00000
*/

    // check for FLIP
    if (orientation == FLIPY)
    {
        writeUint16(0x0006);
        writeUint16(0x1A01);    // write STRANS
        writeUint16(0x0001);
    }
    else
    {
        writeUint16(0x0006);
        writeUint16(0x1A01);    // write STRANS
        writeUint16(0x0000);        
    }

    // ANGLE
    if (orientation != ROT0)
    {
        writeUint16(4+8);
        writeUint16(0x1C05);    // ANGLE id
        switch(orientation)
        {
        case ROT90:
            writeUint8(2+64);       // exponent
            writeUint32(0x5A000000);// mantissa
            writeUint8(0);
            writeUint8(0);
            writeUint8(0);    
            break;
        case ROT180:
            writeUint8(2+64);       // exponent
            writeUint32(0xB4000000);// mantissa
            writeUint8(0);
            writeUint8(0);
            writeUint8(0);    
            break;
        case ROT270:
            writeUint8(3+64);       // exponent
            writeUint32(0x10E00000);// mantissa
            writeUint8(0);
            writeUint8(0);
            writeUint8(0);    
            break;
        default:
            writeUint8(64);         // exponent
            writeUint32(0x00000000);// mantissa
            writeUint8(0);
            writeUint8(0);
            writeUint8(0);          
        }
    }

    // XY
    writeUint16(4+8);
    writeUint16(0x1003);    // XY id
    writeInt32(x);
    writeInt32(y);

    // ENDEL
    writeUint16(4);         // Len
    writeUint16(0x1100);    // ENDEL id
}
