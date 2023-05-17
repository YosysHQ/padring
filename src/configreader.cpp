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
#include <algorithm>
#include "logging.h"
#include "configreader.h"

bool ConfigReader::isWhitespace(char c) const
{
    return ((c==' ') || (c == '\t'));
}

bool ConfigReader::isAlpha(char c) const
{
    if (((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')))
        return true;

    if ((c == '_') || (c == '!'))
        return true;

    return false;
}

bool ConfigReader::isDigit(char c) const
{
    return ((c >= '0') && (c <= '9'));
}

bool ConfigReader::isAlphaNumeric(char c) const
{
    return (isAlpha(c) || isDigit(c));
}

bool ConfigReader::isSpecialIdentChar(char c) const
{
    if ((c == '[') || (c == ']') || 
        (c == '<') || (c == '>') || 
        (c == '/') || (c == '\\') ||
        (c == '.'))
    {
        return true;
    }
    return false;
}

ConfigReader::token_t ConfigReader::tokenize(std::string &tokstr)
{
    tokstr.clear();

    while(isWhitespace(m_tokchar) && !m_is->eof())
    {
        m_tokchar = m_is->get();
    }

    if (m_is->eof())
    {
        return TOK_EOF;
    }

    if ((m_tokchar==10) || (m_tokchar==13))
    {
        m_tokchar = m_is->get();
        m_lineNum++;
        return TOK_EOL;
    }

    if (m_tokchar=='#')
    {
        m_tokchar = m_is->get();
        return TOK_HASH; 
    }

    if (m_tokchar==';')
    {
        m_tokchar = m_is->get();
        return TOK_SEMICOL; 
    }

    if (m_tokchar=='(')
    {
        m_tokchar = m_is->get();
        return TOK_LPAREN;
    }

    if (m_tokchar==')')
    {
        m_tokchar = m_is->get();
        return TOK_RPAREN;
    }

    if (m_tokchar=='[')
    {
        m_tokchar = m_is->get();
        return TOK_LBRACKET;
    }

    if (m_tokchar==']')
    {
        m_tokchar = m_is->get();
        return TOK_RBRACKET;
    }

    if (m_tokchar=='-')
    {
        // could be the start of a number
        tokstr = m_tokchar;
        m_tokchar = m_is->get();
        if (isDigit(m_tokchar))
        {
            // it is indeed a number!
            while(isDigit(m_tokchar) || (m_tokchar == '.') || (m_tokchar == 'e'))
            {
                tokstr += m_tokchar;
                m_tokchar = m_is->get();
            }
            return TOK_NUMBER;            
        }
        return TOK_MINUS;
    }

    if (isAlpha(m_tokchar))
    {
        tokstr = m_tokchar;
        m_tokchar = m_is->get();
        while(isAlphaNumeric(m_tokchar) || isSpecialIdentChar(m_tokchar))
        {            
            tokstr += m_tokchar;
            m_tokchar = m_is->get();
        }
        return TOK_IDENT;
    }

    if (m_tokchar=='"')
    {
        m_tokchar = m_is->get();
        while((m_tokchar != '"') && (m_tokchar != 10) && (m_tokchar != 13))
        {
            tokstr += m_tokchar;
            m_tokchar = m_is->get();
        }

        // skip closing quotes
        if (m_tokchar == '"')
        {
            m_tokchar = m_is->get();
        }

        // error on newline
        if ((m_tokchar == 10) || (m_tokchar == 13))
        {
            // TODO: error, string cannot continue after newline!
        }
        return TOK_STRING;
    }

    if (isDigit(m_tokchar))
    {
        tokstr = m_tokchar;
        m_tokchar = m_is->get();
        while(isDigit(m_tokchar) || (m_tokchar == '.') || (m_tokchar == 'e'))
        {
            tokstr += m_tokchar;
            m_tokchar = m_is->get();
        }
        return TOK_NUMBER;
    }

    m_tokchar = m_is->get();
    return TOK_ERR;
}

bool ConfigReader::parse(std::istream &configstream)
{
    m_lineNum = 1;

    if (!configstream.good())
    {
        doLog(LOG_ERROR,"ConfigReader: input stream is not open\n");
        return false;
    }

    m_is = &configstream;
    std::string tokstr;
    m_tokchar = m_is->get();

    bool m_inComment = false;
    
    ConfigReader::token_t tok = TOK_EOF;
    do
    {
        tok = tokenize(tokstr);
        if (!m_inComment)
        {   
            switch(tok)
            {
            case TOK_ERR:
                error("Config parse error\n");
                break;
            case TOK_HASH:  // line comment
                m_inComment = true;
                break;
            case TOK_IDENT:
                if (tokstr == "CORNER")
                {
                    if (!parseCorner()) return false;
                }
                else if (tokstr == "AREA")
                {
                    if (!parseArea()) return false;
                }
                else if (tokstr == "PAD")
                {
                    if (!parsePad()) return false;
                }
                else if (tokstr == "GRID")
                {
                    if (!parseGrid()) return false;
                }
                else if (tokstr == "SPACE")
                {
                    if (!parseSpace()) return false;
                }
                else if (tokstr == "FILLER")
                {
                    if (!parseFiller()) return false;
                }                
                else if (tokstr == "OFFSET")
                {
                    if (!parseOffset()) return false;
                }
                else if (tokstr == "DESIGN")
                {
                    if (!parseDesignName()) return false;
                }                
                else
                {
                    std::stringstream ss;
                    ss << "unrecognized item " << tokstr << "\n";
                    error(ss.str());
                }
                break;
            default:
                ;
            }
        }
        else
        {
            if (tok == TOK_EOL)
            {
                m_inComment = false;
            }
        }
    } while(tok != TOK_EOF);

    return true;
}

void ConfigReader::error(const std::string &errstr)
{
    std::stringstream ss;
    ss << "Line " << m_lineNum << " : " << errstr; 
    doLog(LOG_ERROR, ss.str());
}


bool ConfigReader::parsePad()
{
    // PAD: instance location cellname
    std::string tokstr;
    std::string instance;
    std::string location;
    std::string cellname;
    bool flipped = false;

    // instance name
    ConfigReader::token_t tok = tokenize(instance);
    if (tok != TOK_IDENT)
    {
        error("Expected an instance name\n");
        return false;
    }

    // location name
    tok = tokenize(location);
    if (tok != TOK_IDENT)
    {
        error("Expected a location\n");
        return false;
    }

    // PADs can only be on North, South, East or West
    std::array<std::string, 4> items = {"N","E","S","W"};
    if (!inArray(location, items))
    {
        error("Expected a pad location to be one of N/E/S/W\n");
        return false;
    }

    // parse optional 'FLIP' argument for flipped cells
    tok = tokenize(cellname);
    if ((tok == TOK_IDENT) && (cellname == "FLIP"))
    {
        flipped = true;
        tok = tokenize(cellname);
    }

    // cell name    
    if (tok != TOK_IDENT)
    {
        error("Expected a cell name\n");
        return false;
    }

    // expect semicol
    tok = tokenize(tokstr);
    if (tok != TOK_SEMICOL)
    {
        error("Expected ;\n");
        return false;
    }

    m_padCount++;
    onPad(instance,location,cellname,flipped);

    return true;
}

bool ConfigReader::inArray(const std::string &value, const std::array<std::string, 4> &array)
{
    return std::find(array.begin(), array.end(), value) != array.end();
}

bool ConfigReader::parseCorner()
{
    // CORNER: instance location cellname
    std::string tokstr;
    std::string instance;
    std::string location;
    std::string cellname;

    // instance name
    ConfigReader::token_t tok = tokenize(instance);
    if (tok != TOK_IDENT)
    {
        error("Expected an instance name\n");
        return false;
    }

    // location name
    tok = tokenize(location);
    if (tok != TOK_IDENT)
    {
        error("Expected a location\n");
        return false;
    }

    // corners can only be on NorthWest, SouthWest, SouthEast or NorthEast
    std::array<std::string, 4> items = {"NW","SW","SE","NE"};
    if (!inArray(location, items))
    {
        error("Expected a corner location to be one of NW/SW/SE/NE\n");
        return false;
    }

    // cell name
    tok = tokenize(cellname);
    if (tok != TOK_IDENT)
    {
        error("Expected a cell name\n");
        return false;
    }

    // expect semicol
    tok = tokenize(tokstr);
    if (tok != TOK_SEMICOL)
    {
        error("Expected ;\n");
        return false;
    }

    onCorner(instance,location,cellname);
    return true;
}

bool ConfigReader::parseArea()
{
    // AREA: x y 
    std::string tokstr;
    std::string w,h;

    // width
    ConfigReader::token_t tok = tokenize(w);
    if (tok != TOK_NUMBER)
    {
        error("Expected a number for area width\n");
        return false;
    }

    // height
    tok = tokenize(h);
    if (tok != TOK_NUMBER)
    {
        error("Expected a number for area height\n");
        return false;
    }

    // expect semicol
    tok = tokenize(tokstr);
    if (tok != TOK_SEMICOL)
    {
        error("Expected ;\n");
        return false;
    }

    double wd, hd;
    try
    {
        wd = std::stod(w);
        hd = std::stod(h);
    }
    catch(const std::invalid_argument& ia)
    {
        error(ia.what());
        return false;
    }

    onArea(wd,hd);
    return true;
}

bool ConfigReader::parseGrid()
{
    // GRID: g 
    std::string tokstr;
    std::string g;

    // grid
    ConfigReader::token_t tok = tokenize(g);
    if (tok != TOK_NUMBER)
    {
        error("Expected a number for grid spacing\n");
        return false;
    }

    // expect semicol
    tok = tokenize(tokstr);
    if (tok != TOK_SEMICOL)
    {
        error("Expected ;\n");
        return false;
    }

    double gd;
    try
    {
        gd = std::stod(g);
    }
    catch(const std::invalid_argument& ia)
    {
        error(ia.what());
    }

    onGrid(gd);
    return true;
}

bool ConfigReader::parseSpace()
{
    // SPACE: g 
    std::string tokstr;
    std::string g;

    // space
    ConfigReader::token_t tok = tokenize(g);
    if (tok != TOK_NUMBER)
    {
        error("Expected a number for space\n");
        return false;
    }

    // expect semicol
    tok = tokenize(tokstr);
    if (tok != TOK_SEMICOL)
    {
        error("Expected ;\n");
        return false;
    }

    double gd;
    try
    {
        gd = std::stod(g);
    }
    catch(const std::invalid_argument& ia)
    {
        error(ia.what());
    }

    onSpace(gd);
    return true;
}


bool ConfigReader::parseOffset()
{
    // OFFSET: g 
    std::string tokstr;
    std::string g;

    // offset
    ConfigReader::token_t tok = tokenize(g);
    if (tok != TOK_NUMBER)
    {
        error("Expected a number for offset\n");
        return false;
    }

    // expect semicol
    tok = tokenize(tokstr);
    if (tok != TOK_SEMICOL)
    {
        error("Expected ;\n");
        return false;
    }

    double gd;
    try
    {
        gd = std::stod(g);
    }
    catch(const std::invalid_argument& ia)
    {
        error(ia.what());
    }

    onOffset(gd);
    return true;
}

bool ConfigReader::parseFiller()
{
    // FILLER: fillername
    std::string tokstr;
    std::string fillerName;

    // fillername
    ConfigReader::token_t tok = tokenize(fillerName);
    if (tok != TOK_IDENT)
    {
        error("Expected a filler cell prefix\n");
        return false;
    }

    // expect semicol
    tok = tokenize(tokstr);
    if (tok != TOK_SEMICOL)
    {
        error("Expected ;\n");
        return false;
    }

    onFiller(fillerName);
    return true;
}

bool ConfigReader::parseDesignName()
{
    // DESIGN: designname
    std::string tokstr;
    std::string designName;

    // designname
    ConfigReader::token_t tok = tokenize(designName);
    if (tok != TOK_IDENT)
    {
        error("Expected a design name\n");
        return false;
    }

    // expect semicol
    tok = tokenize(tokstr);
    if (tok != TOK_SEMICOL)
    {
        error("Expected ;\n");
        return false;
    }

    onDesignName(designName);
    return true;
}
