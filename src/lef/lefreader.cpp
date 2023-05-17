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

#include "lefreader.h"

bool LEFReader::isWhitespace(char c) const
{
    return ((c==' ') || (c == '\t'));
}

bool LEFReader::isAlpha(char c) const
{
    if (((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')))
        return true;

    if ((c == '_') || (c == '!'))
        return true;

    return false;
}

bool LEFReader::isDigit(char c) const
{
    return ((c >= '0') && (c <= '9'));
}

bool LEFReader::isAlphaNumeric(char c) const
{
    return (isAlpha(c) || isDigit(c));
}


LEFReader::token_t LEFReader::tokenize(std::string &tokstr)
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
        while(isAlphaNumeric(m_tokchar))
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

void LEFReader::parse(std::istream &lefstream)
{
    m_lineNum = 1;

    if (!lefstream.good())
    {
        error("LEFReader: input stream is faulty\n");
        return;
    }

    m_is = &lefstream;
    
    m_tokchar = m_is->get();

    bool m_inComment = false;
    
    m_curtok = TOK_EOF;
    do
    {
        m_curtok = tokenize(m_tokstr);
        if (!m_inComment)
        {   
            switch(m_curtok)
            {
            case TOK_ERR:
                error("LEF parse error\n");
                break;
            case TOK_HASH:  // line comment
                m_inComment = true;
                break;
            case TOK_IDENT:
                if (m_tokstr == "MACRO")
                {
                    parseMacro();
                }
                else if (m_tokstr == "LAYER")
                {
                    parseLayer();
                }
                else if (m_tokstr == "VIA")
                {
                    parseVia();
                }
                else if (m_tokstr == "VIARULE")
                {
                    parseViaRule();
                }
                else if (m_tokstr == "UNITS")
                {
                    parseUnits();
                }
                else if (m_tokstr == "PROPERTYDEFINITIONS")
                {
                    parsePropertyDefintions();
                }
                break;
            default:
                ;
            }
        }
        else
        {
            if (m_curtok == TOK_EOL)
            {
                m_inComment = false;
            }
        }
    } while(m_curtok != TOK_EOF);

    onEndParse();
}

void LEFReader::error(const std::string &errstr)
{
    std::cerr << "Line " << m_lineNum << " : " << errstr; 
}

bool LEFReader::parseMacro()
{
    std::string name;
    

    // macro name
    m_curtok = tokenize(name);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected a macro name\n");
        return false;
    }

    //std::cout << "MACRO " << name << "\n"; 

    // expect EOL
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected EOL\n");
        return false;
    }

    onMacro(name);

    // wait for 'END macroname'
    bool endFound = false;
    while(true)
    {
        m_curtok = tokenize(m_tokstr);

        if (m_curtok == TOK_IDENT)
        {
            if (m_tokstr == "PIN")
            {
                parsePin();
            }
            else if (m_tokstr == "CLASS")
            {
                parseClass();
            }            
            else if (m_tokstr == "ORIGIN")
            {
                parseOrigin();
            }
            else if (m_tokstr == "FOREIGN")
            {
                parseForeign();
            }
            else if (m_tokstr == "SIZE")
            {
                parseSize();
            }
            else if (m_tokstr == "SYMMETRY")
            {
                parseSymmetry();
            }
            else if (m_tokstr == "SITE")
            {
                parseSite();
            }
            //else if (m_tokstr == "LAYER")
            //{
            //    parseLayer();   // TECH LEF layer, not a port LAYER!
            //}  
        }

        if (endFound)
        {
            if ((m_curtok == TOK_IDENT) && (m_tokstr == name))
            {
                //cout << "END " << name << "\n";
                return true;
            }
        }
        else if ((m_curtok == TOK_IDENT) && (m_tokstr == "END"))
        {
            endFound = true;
        }
        else
        {
            endFound = false;
        }

        if (m_is->eof())
        {
            error("Unexpected end of file\n");
            return false;
        }        
    }
}

bool LEFReader::parsePinName(std::string &outName)
{
    // pin name
    m_curtok = tokenize(outName);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected a pin name\n");
        return false;
    }

    // optionally parse bus index
    m_curtok = tokenize(m_tokstr);
    while(m_curtok == TOK_LBRACKET)
    {
        m_curtok = tokenize(m_tokstr);
        if (m_curtok != TOK_NUMBER)
        {
            error("Expected a number in pin brackets");
            return false;
        }
        auto numstr = m_tokstr;
        
        m_curtok = tokenize(m_tokstr);
        if (m_curtok != TOK_RBRACKET)
        {
            error("Expected closing bracket");
            return false;
        }

        outName.append("[");
        outName.append(numstr);
        outName.append("]");

        m_curtok = tokenize(m_tokstr);
    }
    return true;
}

bool LEFReader::parsePin()
{
    std::string name;
    
    if (!parsePinName(name))
    {
        return false;
    }

    // expect EOL
    if (m_curtok != TOK_EOL)
    {
        error("Expected EOL\n");
        return false;
    }

    std::cout << "  PIN: " << name << "\n";

    onPin(name);

    while(true)
    {
        m_curtok = tokenize(m_tokstr);

        if (m_curtok == TOK_IDENT)
        {
            if (m_tokstr == "DIRECTION")
            {
                parseDirection();
            }
            else if (m_tokstr == "USE")
            {
                parseUse();
            }            
            else if (m_tokstr == "PORT")
            {
                parsePort();
            }
            else if (m_tokstr == "END")
            {
                std::string endName;
                if (!parsePinName(endName))
                {
                    std::stringstream ss;
                    ss << "Expected pin name " << endName << "\n";
                    error(ss.str());
                    return false;
                }

                if (endName != name)
                {
                    std::stringstream ss;
                    ss << "Expected pin name " << endName << "\n";
                    error(ss.str());
                    return false;
                }

                if (m_curtok != TOK_EOL)
                {
                    error("Expected EOL\n");
                    return false;
                }

                return true;
            }           
        }

        if (m_is->eof())
        {
            error("Unexpected end of file\n");
            return false;
        }
    }
}


bool LEFReader::parseClass()
{
    // CLASS name <optional name> ';'

    std::string className;
    

    // read in all the classes
    bool foundOne = false;
    m_curtok = tokenize(m_tokstr);
    while(m_curtok == TOK_IDENT)
    {
        foundOne = true;
        className += m_tokstr;
        className += " ";
        m_curtok = tokenize(m_tokstr);
    }
    if (!foundOne)
    {
        error("Expected at least one class\n");
        return false;
    }
    
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected a semicolon\n");
        return false;
    }

    onClass(className);

    return true;
};

bool LEFReader::parseOrigin()
{
    // ORIGIN <number> <number> ; 

    
    std::string xnum;
    std::string ynum;

    m_curtok = tokenize(xnum);
    if (m_curtok != TOK_NUMBER)
    {
        error("Expected a number\n");
        return false;
    }

    m_curtok = tokenize(ynum);
    if (m_curtok != TOK_NUMBER)
    {
        error("Expected a number\n");
        return false;
    }

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected a semicolon\n");
        return false;
    }

    double xnumd, ynumd;
    try
    {
        xnumd = std::stod(xnum);
        ynumd = std::stod(ynum);
    }
    catch(const std::invalid_argument& ia)
    {
        error(ia.what());
    }

    onOrigin(xnumd, ynumd);

    //std::cout << "  ORIGIN " << xnum << " " << ynum << "\n";

    return true;
};

bool LEFReader::parseSite()
{
    // SITE name ';' 

    std::string siteName;
    

    m_curtok = tokenize(siteName);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected an identifier\n");
        return false;
    }

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected a semicolon\n");
        return false;
    }

    onSite(siteName);

    //std::cout << "  SITE " << siteName << "\n";

    return true;
};

bool LEFReader::parseSize()
{
    // SIZE <number> BY <number> ';' 

    
    std::string xnum;
    std::string ynum;

    m_curtok = tokenize(xnum);
    if (m_curtok != TOK_NUMBER)
    {
        error("Expected a number\n");
        return false;
    }

    m_curtok = tokenize(m_tokstr);
    if ((m_curtok != TOK_IDENT) && (m_tokstr != "BY"))
    {
        error("Expected 'BY'\n");
        return false;
    }

    m_curtok = tokenize(ynum);
    if (m_curtok != TOK_NUMBER)
    {
        error("Expected a number\n");
        return false;
    }

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected a semicolon\n");
        return false;
    }

    double xnumd, ynumd;
    try
    {
        xnumd = std::stod(xnum);
        ynumd = std::stod(ynum);
    }
    catch(const std::invalid_argument& ia)
    {
        error(ia.what());
    }

    onSize(xnumd, ynumd);

    //std::cout << "  SIZE " << xnum << " " << ynum << "\n";

    return true;
};

bool LEFReader::parseSymmetry()
{
    // SYMMETRY (X|Y|R90)+ ';' 

    
    std::string symmetry;

    // read options until we get to the semicolon.
    m_curtok = tokenize(m_tokstr);
    while(m_curtok!= TOK_SEMICOL)
    {
        symmetry += m_tokstr;
        symmetry += " ";
        m_curtok = tokenize(m_tokstr);
    }

    //std::cout << "  SYMMETRY " << symmetry << "\n";

    return true;
};

bool LEFReader::parseForeign()
{
    // FOREIGN <cellname> [<number> <number>] ; 

    std::string cellname;
    std::string xnum;
    std::string ynum;

    m_curtok = tokenize(cellname);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected the cell name\n");
        return false;
    }

    // foreign point is optional

    m_curtok = tokenize(xnum);
    if (m_curtok == TOK_NUMBER)
    {
        m_curtok = tokenize(ynum);
        if (m_curtok != TOK_NUMBER)
        {
            error("Expected a number\n");
            return false;
        }

        m_curtok = tokenize(m_tokstr);
        if (m_curtok != TOK_SEMICOL)
        {
            error("Expected a semicolon\n");
            return false;
        }

        double xnumd, ynumd;
        try
        {
            xnumd = std::stod(xnum);
            ynumd = std::stod(ynum);
        }
        catch(const std::invalid_argument& ia)
        {
            error(ia.what());
        }

        onForeign(cellname, xnumd, ynumd);
        return true;
    }
    else if (m_curtok != TOK_SEMICOL)
    {
        error("Expected a number or semicolon\n");
        return false;
    }    

    onForeign(cellname, 0, 0);

    return true;
};


bool LEFReader::parseDirection()
{
    // DIRECTION OUTPUT/INPUT/INOUT etc.
    std::string direction;

    // read options until we get to the semicolon.
    m_curtok = tokenize(direction);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected direction\n");
        return false;
    }

    m_curtok = tokenize(m_tokstr);
    if ((direction == "OUTPUT") && (m_tokstr == "TRISTATE"))
    {
        // OUTPUT can be followed by TRISTATE
        // FIXME: use enum.
        direction += " TRISTATE";
        m_curtok = tokenize(m_tokstr);
    }

    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected a semicolon\n");
        return false;
    }    

    onPinDirection(direction);

    return true;
};

bool LEFReader::parseUse()
{
    // USE OUTPUT/INPUT/INOUT etc.

    std::string use;

    m_curtok = tokenize(use);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected use\n");
        return false;
    }

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected a semicolon\n");
        return false;
    }    

    onPinUse(use);

    return true;
};

bool LEFReader::parsePort()
{
    std::string name;
    
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected EOL\n");
        return false;
    }

    // expect LAYER
    m_curtok = tokenize(m_tokstr);
    while (m_curtok == TOK_IDENT)
    {
        if (m_tokstr == "LAYER")
        {
            parsePortLayer();
        }
        else if (m_tokstr == "END")
        {
            break;
        }
        else
        {
            // eat until ;
            do
            {
                m_curtok = tokenize(m_tokstr);
            } while(m_curtok != TOK_SEMICOL);

            // eat newline
            m_curtok = tokenize(m_tokstr);
            if (m_curtok != TOK_EOL)
            {
                error("Expected EOL");
                return false;
            }
        }

        if (m_is->eof())
        {
            error("Unexpected end of file\n");
            return false;
        }
    }
    return true;
}

bool LEFReader::parsePortLayer()
{
    // LAYER <name> ';'
    std::string name;

    m_curtok = tokenize(name);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected layer name\n");
        return false;
    }

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected a semicolon\n");
        return false;
    }    

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected an EOL\n");
        return false;
    }    

    //std::cout << "    LAYER " << name << "\n";

    do
    {
        if (!parsePortLayerItem())
        {
            return false;
        }
    } while(m_tokstr != "END");
    
    return true;
}

bool LEFReader::parsePortLayerItem()
{
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected RECT or END\n");
        return false;
    }

    while(1)
    {
        if (m_tokstr == "END")
        {
            return true;
        }
        else if (m_tokstr == "RECT")
        {
            parseRect();
        }
        m_curtok = tokenize(m_tokstr);
    }
}

bool LEFReader::parseRect()
{
    
    std::string coords;

    for(uint32_t i=0; i<4; i++)
    {
        m_curtok = tokenize(m_tokstr);
        if (m_curtok != TOK_NUMBER)
        {
            error("Expected number in RECT\n");
            return false;
        }
        coords += m_tokstr;
        coords += " ";
    }

    // expect ; 
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected a semicolon in RECT\n");
        return false;
    }

    // expect EOL
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected an EOL in RECT\n");
        return false;
    }    

    //std::cout << "      RECT " << coords << "\n";

    return true;
}

bool LEFReader::parseLayer()
{
    m_curtok = tokenize(m_tokstr);
    std::string layerName = m_tokstr;

    if (m_curtok != TOK_IDENT)
    {
        error("Expected a layer name\n");
        return false;
    }

    // expect EOL
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected EOL in LAYER\n");
        return false;
    }

    onLayer(layerName);

    // parse all the layer items
    do
    {
        if (!parseLayerItem())
        {
            return false;
        }
    } while(m_tokstr != "END");

    // expect END <layername> EOL

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected END <layername>\n");
        return false;
    }
    if (m_tokstr != layerName)
    {
        error("Expected END <layername> : name does not match\n");
        return false;
    }
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected EOL\n");
        return false;
    }

    return true;
}

bool LEFReader::parseLayerItem()
{
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected identifier in layer item\n");
        return false;
    }
    if (m_tokstr == "PITCH")
    {
        return parseLayerPitch();   
    }
    else if (m_tokstr == "OFFSET")
    {
        return parseLayerOffset();
    }
    else if (m_tokstr == "TYPE")
    {
        return parseLayerType();
    }    
    else if (m_tokstr == "DIRECTION")
    {
        return parseLayerDirection();
    }
    else if (m_tokstr == "WIDTH")
    {
        return parseLayerWidth();
    }    
    else if (m_tokstr == "MAXWIDTH")
    {
        return parseLayerMaxWidth();
    }        
    else if (m_tokstr == "END")
    {
        return true;
    }
    else
    {
        // eat everything on the line
        while((m_curtok != TOK_EOL) && (m_curtok != TOK_EOF))
        {
            m_curtok = tokenize(m_tokstr);
        }
        if (m_curtok == TOK_EOF)
        {
            error("Unexpected end of file");
            return false;
        }
    }

    return true;
}

bool LEFReader::parseLayerPitch()
{
    std::string pitch;
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_NUMBER)
    {
        error("Expected a number in layer pitch\n");
        return false;    
    }

    pitch = m_tokstr;

    // expect ; 
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected a semicolon in layer pitch\n");
        return false;
    }

    // expect EOL
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected an EOL\n");
        return false;
    }

    double pitchd;
    try
    {
        pitchd = std::stod(pitch);
    }
    catch(const std::invalid_argument& ia)
    {
        error(ia.what());
    }

    onLayerPitch(pitchd);

    return true;    
}

bool LEFReader::parseLayerOffset()
{
    std::string offset;
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_NUMBER)
    {
        error("Expected a number in layer offset\n");
        return false;    
    }

    offset = m_tokstr;

    // expect ; 
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected a semicolon in layer offset\n");
        return false;
    }

    // expect EOL
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected an EOL\n");
        return false;
    }

    double offsetd;
    try
    {
        offsetd = std::stod(offset);
    }
    catch(const std::invalid_argument& ia)
    {
        error(ia.what());
    }

    onLayerOffset(offsetd);

    return true;    
}

bool LEFReader::parseLayerType()
{
    std::string layerType;
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected a string in layer type\n");
        return false;    
    }

    layerType = m_tokstr;

    // expect ; 
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected a semicolon in layer type\n");
        return false;
    }

    // expect EOL
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected an EOL\n");
        return false;
    }

    onLayerType(layerType);

    return true;    
}

bool LEFReader::parseLayerWidth()
{
    std::string width;
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_NUMBER)
    {
        error("Expected a number in layer width\n");
        return false;    
    }

    width = m_tokstr;

    // expect ; 
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected a semicolon in layer width\n");
        return false;
    }

    // expect EOL
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected an EOL\n");
        return false;
    }

    double widthd;
    try
    {
        widthd = std::stod(width);
    }
    catch(const std::invalid_argument& ia)
    {
        error(ia.what());
    }

    onLayerWidth(widthd);

    return true;    
}

bool LEFReader::parseLayerMaxWidth()
{
    std::string maxwidth;
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_NUMBER)
    {
        error("Expected a number in layer max width\n");
        return false;    
    }

    maxwidth = m_tokstr;

    // expect ; 
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected a semicolon in layer max width\n");
        return false;
    }

    // expect EOL
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected an EOL\n");
        return false;
    }

    double maxwidthd;
    try
    {
        maxwidthd = std::stod(maxwidth);
    }
    catch(const std::invalid_argument& ia)
    {
        error(ia.what());
    }

    onLayerMaxWidth(maxwidthd);

    return true;
}

bool LEFReader::parseLayerDirection()
{
    std::string direction;
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_IDENT)
    {
        error("Expected a string in layer direction\n");
        return false;    
    }

    direction = m_tokstr;

    // expect ; 
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_SEMICOL)
    {
        error("Expected a semicolon in layer direction\n");
        return false;
    }

    // expect EOL
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected an EOL\n");
        return false;
    }

    onLayerDirection(direction);

    return true;  
}

bool LEFReader::parseVia()
{
    // VIA <vianame> ...
    // keep on reading tokens until we
    // find END <vianame>

    m_curtok = tokenize(m_tokstr);
    std::string viaName;

    if (m_curtok != TOK_IDENT)
    {
        error("Expected identifier in via name\n");
        return false;
    }

    viaName = m_tokstr;

    // read until we get END <vianame>
    do
    {
        m_curtok = tokenize(m_tokstr);
        while(m_tokstr != "END")
        {
            m_curtok = tokenize(m_tokstr);
        }

        // read via name
        m_curtok = tokenize(m_tokstr);
        if (m_curtok != TOK_IDENT)
        {
            error("Expected via name after END\n");
            return false;
        }
    } while(m_tokstr == viaName);
    
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected EOL after END <vianame>\n");
        return false;
    }

    return true;
}

bool LEFReader::parseViaRule()
{
    // VIARULE <vianame> ...
    // keep on reading tokens until we
    // find END <vianame>

    m_curtok = tokenize(m_tokstr);
    std::string viaRuleName;

    if (m_curtok != TOK_IDENT)
    {
        error("Expected identifier in viarule name\n");
        return false;
    }

    viaRuleName = m_tokstr;

    // read until we get END <vianame>
    do
    {
        m_curtok = tokenize(m_tokstr);
        while(m_tokstr != "END")
        {
            m_curtok = tokenize(m_tokstr);
        }

        // read via name
        m_curtok = tokenize(m_tokstr);
        if (m_curtok != TOK_IDENT)
        {
            error("Expected viarule name after END\n");
            return false;
        }
    } while(m_tokstr == viaRuleName);
    
    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected EOL after END <viarule name>\n");
        return false;
    }

    return true;
}

bool LEFReader::parseUnits()
{
    // UNITS
    //   DATABASE MICRONS ;
    //   ...
    // END UNITS

    // expect EOL after UNITS

    m_curtok = tokenize(m_tokstr);
    if (m_curtok != TOK_EOL)
    {
        error("Expected EOL after UNITS\n");
        return false;
    }

    while(1)
    {
        m_curtok = tokenize(m_tokstr);

        if (m_curtok != TOK_IDENT)
        {
            error("Expected string in units block\n");
            return false;
        }

        if (m_tokstr == "DATABASE")
        {
            m_curtok = tokenize(m_tokstr);
            if ((m_curtok == TOK_IDENT) && (m_tokstr == "MICRONS"))
            {
                m_curtok = tokenize(m_tokstr);
                if (m_curtok == TOK_NUMBER)
                {
                    double micronsd;
                    try
                    {
                        micronsd = std::stod(m_tokstr);
                    }
                    catch(const std::invalid_argument& ia)
                    {
                        error(ia.what());
                        return false;
                    }

                    // expect ; and EOL
                    m_curtok = tokenize(m_tokstr);
                    if (m_curtok != TOK_SEMICOL)
                    {
                        error("Expected ; after DATABASE MICRONS <number>\n");
                        return false;
                    }

                    m_curtok = tokenize(m_tokstr);
                    if (m_curtok != TOK_EOL)
                    {
                        error("Expected EOL after DATABASE MICRONS <number> ;\n");
                        return false;
                    }

                    onDatabaseUnitsMicrons(micronsd);
                }
                else
                {
                    error("Expected a number after DATABASE MICRONS\n");
                    return false;
                }
            }
            else
            {
                error("Expects MICRONS keywords after DATABASE\n");
                return false;
            }
        }
        else if (m_tokstr == "END")
        {
            // check for units
            m_curtok = tokenize(m_tokstr);
            if ((m_curtok == TOK_IDENT) && (m_tokstr == "UNITS"))
            {
                return true;
            }
            else
            {
                // read until EOL
                while ((m_curtok != TOK_EOL) && (m_curtok != TOK_EOF))
                {
                    m_curtok = tokenize(m_tokstr);
                }
            }
        }
        else
        {
            // got something other than DATABASE or END
            // read until EOL
            while ((m_curtok != TOK_EOL) && (m_curtok != TOK_EOF))
            {
                m_curtok = tokenize(m_tokstr);
            }
        }
    }
}

bool LEFReader::parsePropertyDefintions()
{
    // basically, eat everything until
    // we encounter END PROPERTYDEFINTIONS EOL

    while(1)
    {
        m_curtok = tokenize(m_tokstr);
        if ((m_curtok == TOK_IDENT) && (m_tokstr == "END"))
        {
            m_curtok = tokenize(m_tokstr);
            if ((m_curtok == TOK_IDENT) && (m_tokstr == "PROPERTYDEFINITIONS"))
            {
                m_curtok = tokenize(m_tokstr);
                if (m_curtok == TOK_EOL)
                {
                    return true;
                }
                else if (m_curtok == TOK_EOF)
                {
                    error("Unexpected end of liberty file\n");
                    return false;                    
                }
            }
            else if (m_curtok == TOK_EOF)
            {
                error("Unexpected end of liberty file\n");
                return false;
            }
        }
        else if (m_curtok == TOK_EOF)
        {
            error("Unexpected end of liberty file\n");
            return false;
        }
    }
}