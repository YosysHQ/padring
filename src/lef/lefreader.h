/*
    SCPLACER -- a standard cell placer for ASICs.

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

#ifndef lef_reader_h
#define lef_reader_h

#include<list>
#include<vector>
#include<string>
#include<iostream>
#include<regex>

#include "../linereader.h"

/** reads a blif stream and generates callbacks for every relevant
    item, such as .input .output etc.
*/
class LEFReader
{
public:
    LEFReader() {}
    
    virtual ~LEFReader() {}

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

    void parse(std::istream &leffile);

    /** callback for each LEF macro */
    virtual void onMacro(const std::string &macroName) {}

    /** callback for CLASS within a macro */
    virtual void onClass(const std::string &className) {}

    /** callback for ORIGIN within a macro */
    virtual void onOrigin(double x, double y) {}

    /** callback for ORIGIN within a macro */
    virtual void onForeign(const std::string &foreignName, double x, double y) {}

    /** callback for SIZE within a macro */
    virtual void onSize(double sx, double sy) {}

    /** callback for SYMMETRY within a macro */
    virtual void onSymmetry(const std::string &symmetry) {}

    /** callback for SITE within a macro */
    virtual void onSite(const std::string &site) {}

    /** callback for PIN within a macro */
    virtual void onPin(const std::string &pinName) {}

    /** callback for PIN direction */
    virtual void onPinDirection(const std::string &direction) {}

    /** callback for PIN use */
    virtual void onPinUse(const std::string &use) {}

    /** callback when done parsing */
    virtual void onEndParse() {}

    /** callback for layer */
    virtual void onLayer(const std::string &layerName) {}

    /** callback for layer type */
    virtual void onLayerType(const std::string &layerType) {}

    /** callback for layer pitch */
    virtual void onLayerPitch(double pitch) {}

    /** callback for layer offset */    
    virtual void onLayerOffset(double offset) {}

    /** callback for layer routing direction */
    virtual void onLayerDirection(const std::string &direction) {}

    /** callback for layer trace width */
    virtual void onLayerWidth(double width) {}

    /** callback for layer trace max width */    
    virtual void onLayerMaxWidth(double maxWidth) {}

    /** callback for units database microns */
    virtual void onDatabaseUnitsMicrons(double unitsPerMicron) {}

protected:
    bool isWhitespace(char c) const;
    bool isAlpha(char c) const;
    bool isDigit(char c) const;
    bool isAlphaNumeric(char c) const;

    bool parseMacro();
    bool parseClass();
    bool parseOrigin();
    bool parseForeign();
    bool parseSize();
    bool parseSymmetry();
    bool parseSite();
    bool parsePin();
    bool parsePinName(std::string &outName);
    bool parseDirection();
    bool parseUse();
    
    bool parsePort();
    bool parsePortLayer();
    bool parsePortLayerItem();
    bool parseRect();
    
    bool parseLayer();
    bool parseLayerItem();
    bool parseLayerType();
    bool parseLayerPitch();
    bool parseLayerWidth();
    bool parseLayerMaxWidth();
    bool parseLayerDirection();
    bool parseLayerOffset();

    bool parseVia();
    bool parseViaRule();

    bool parseUnits();

    bool parsePropertyDefintions();

    token_t tokenize(std::string &tokstr);
    char         m_tokchar;

    LEFReader::token_t m_curtok;
    std::string        m_tokstr;

    void error(const std::string &errstr);

    std::istream *m_is;
    uint32_t      m_lineNum;
};


#endif


