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

#ifndef config_reader_h
#define config_reader_h

#include<list>
#include<vector>
#include<array>
#include<string>
#include<iostream>

#include "linereader.h"

/** reads a IO configuration file

    Example file:

    DESIGN PADRING
    AREA 1000 1000
    GRID 1
    CORNER CORNER1 NW CORNERESDP
    CORNER CORNER2 SW CORNERESDP
    CORNER CORNER3 SE CORNERESDP
    CORNER CORNER4 NE CORNERESDP
    PAD IO1 N BBC16F
    PAD IO2 N BBC16F
    PAD IO3 N BBC16F
    PAD IO4 N BBC16F
    SPACE 50            # fixed space between cells
    PAD IO5 N BBC16F
    PAD IO6 N BBC16F
    OFFSET 900          # absolute offset
    PAD IO7 N BBC16F 
    PAD IO8 N BBC16F

*/
class ConfigReader
{
public:
    ConfigReader() : m_padCount(0) {}
    
    virtual ~ConfigReader() {}

    enum token_t
    {
        TOK_EOF,
        TOK_IDENT,
        TOK_STRING,
        TOK_NUMBER,
        TOK_MINUS,
        TOK_LBRACKET,
        TOK_RBRACKET,
        TOK_LPAREN,
        TOK_RPAREN,
        TOK_HASH,
        TOK_SEMICOL,
        TOK_EOL,
        TOK_ERR
    };

    bool parse(std::istream &configfile);

    /** callback for a corner */
    virtual void onCorner(
        const std::string &instance,
        const std::string &location,
        const std::string &cellname)
    {
        std::cout << "CORNER " << instance << " " << location << " " << cellname << "\n";
    }

    /** callback for a pad */
    virtual void onPad(
        const std::string &instance,
        const std::string &location,
        const std::string &cellname)
    {
        std::cout << "PAD " << instance << " " << location << " " << cellname << "\n";
    }

    /** callback for die area in microns */
    virtual void onArea(double x, double y) 
    {
        std::cout << "Area " << x << " " << y << "\n";
    }

    /** callback for grid spacing in microns */
    virtual void onGrid(double grid)
    {
        std::cout << "Grid " << grid << "\n";
    }

    /** callback for grid spacing in microns */
    virtual void onFiller(const std::string &fillerName)
    {
        std::cout << "Filler prefix:" << fillerName << "\n";
    }

    /** callback for space in microns */
    virtual void onSpace(double space)
    {
        std::cout << "Space " << space << "\n";
    }

    /** callback for offset in microns */
    virtual void onOffset(double offset)
    {
        std::cout << "Offset " << offset << "\n";
    }

    /** callback for design name */
    virtual void onDesignName(const std::string &designName)
    {
        std::cout << "Design name " << designName << "\n";
    }

    /** return the number of pad cells (excluding corners) */
    uint32_t getPadCellCount() const
    {
        return m_padCount;
    }

protected:
    bool isWhitespace(char c) const;
    bool isAlpha(char c) const;
    bool isDigit(char c) const;
    bool isAlphaNumeric(char c) const;
    bool isSpecialIdentChar(char c) const;

    bool inArray(const std::string &value, const std::array<std::string, 4> &array);

    bool parsePad();
    bool parseCorner();
    bool parseArea();
    bool parseGrid();
    bool parseSpace();
    bool parseOffset();
    bool parseFiller();
    bool parseDesignName();

    token_t      tokenize(std::string &tokstr);
    char         m_tokchar;

    void error(const std::string &errstr);

    std::istream *m_is;
    uint32_t      m_lineNum;
    uint32_t      m_padCount;   ///< number of pad cells excluding corners
};


#endif


