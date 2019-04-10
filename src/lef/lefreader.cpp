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
    std::string tokstr;
    m_tokchar = m_is->get();

    bool m_inComment = false;
    
    LEFReader::token_t tok = TOK_EOF;
    do
    {
        tok = tokenize(tokstr);
        if (!m_inComment)
        {   
            switch(tok)
            {
            case TOK_ERR:
                error("LEF parse error\n");
                break;
            case TOK_HASH:  // line comment
                m_inComment = true;
                break;
            case TOK_IDENT:
                if (tokstr == "MACRO")
                {
                    parseMacro();
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
}

void LEFReader::error(const std::string &errstr)
{
    std::cerr << "Line " << m_lineNum << " : " << errstr; 
}

bool LEFReader::parseMacro()
{
    std::string name;
    std::string tokstr;

    // macro name
    LEFReader::token_t tok = tokenize(name);
    if (tok != TOK_IDENT)
    {
        error("Expected a macro name\n");
        return false;
    }

    //std::cout << "MACRO " << name << "\n"; 

    // expect EOL
    tok = tokenize(tokstr);
    if (tok != TOK_EOL)
    {
        error("Expected EOL\n");
        return false;
    }

    onMacro(name);

    // wait for 'END macroname'
    bool endFound = false;
    while(true)
    {
        tok = tokenize(tokstr);

        if (tok == TOK_IDENT)
        {
            if (tokstr == "PIN")
            {
                parsePin();
            }
            else if (tokstr == "CLASS")
            {
                parseClass();
            }            
            else if (tokstr == "ORIGIN")
            {
                parseOrigin();
            }
            else if (tokstr == "FOREIGN")
            {
                parseForeign();
            }
            else if (tokstr == "SIZE")
            {
                parseSize();
            }
            else if (tokstr == "SYMMETRY")
            {
                parseSymmetry();
            }
            else if (tokstr == "SITE")
            {
                parseSite();
            }            
        }

        if (endFound)
        {
            if ((tok == TOK_IDENT) && (tokstr == name))
            {
                //cout << "END " << name << "\n";
                return true;
            }
        }
        else if ((tok == TOK_IDENT) && (tokstr == "END"))
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

bool LEFReader::parsePin()
{
    std::string name;
    std::string tokstr;

    // pin name
    LEFReader::token_t tok = tokenize(name);
    if (tok != TOK_IDENT)
    {
        error("Expected a pin name\n");
        return false;
    }

    //std::cout << "  PIN " << name << "\n"; 

    // expect EOL
    tok = tokenize(tokstr);
    if (tok != TOK_EOL)
    {
        error("Expected EOL\n");
        return false;
    }

    onPin(name);

    // wait for 'END macroname'
    bool endFound = false;
    while(true)
    {
        tok = tokenize(tokstr);

        if (tok == TOK_IDENT)
        {
            if (tokstr == "DIRECTION")
            {
                parseDirection();
            }
            else if (tokstr == "USE")
            {
                parseUse();
            }            
            else if (tokstr == "PORT")
            {
                parsePort();
            }           
        }

        if (endFound)
        {
            if ((tok == TOK_IDENT) && (tokstr == name))
            {
                //std::cout << "  END " << name << "\n";
                return true;
            }
        }
        else if ((tok == TOK_IDENT) && (tokstr == "END"))
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


bool LEFReader::parseClass()
{
    // CLASS name <optional name> ';'

    std::string className;
    std::string tokstr;

    // read in all the classes
    bool foundOne = false;
    LEFReader::token_t tok = tokenize(tokstr);
    while(tok == TOK_IDENT)
    {
        foundOne = true;
        className += tokstr;
        className += " ";
        tok = tokenize(tokstr);
    }
    if (!foundOne)
    {
        error("Expected at least one class\n");
        return false;
    }
    
    if (tok != TOK_SEMICOL)
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

    std::string tokstr;
    std::string xnum;
    std::string ynum;

    LEFReader::token_t tok = tokenize(xnum);
    if (tok != TOK_NUMBER)
    {
        error("Expected a number\n");
        return false;
    }

    tok = tokenize(ynum);
    if (tok != TOK_NUMBER)
    {
        error("Expected a number\n");
        return false;
    }

    tok = tokenize(tokstr);
    if (tok != TOK_SEMICOL)
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
    std::string tokstr;

    LEFReader::token_t tok = tokenize(siteName);
    if (tok != TOK_IDENT)
    {
        error("Expected an identifier\n");
        return false;
    }

    tok = tokenize(tokstr);
    if (tok != TOK_SEMICOL)
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

    std::string tokstr;
    std::string xnum;
    std::string ynum;

    LEFReader::token_t tok = tokenize(xnum);
    if (tok != TOK_NUMBER)
    {
        error("Expected a number\n");
        return false;
    }

    tok = tokenize(tokstr);
    if ((tok != TOK_IDENT) && (tokstr != "BY"))
    {
        error("Expected 'BY'\n");
        return false;
    }

    tok = tokenize(ynum);
    if (tok != TOK_NUMBER)
    {
        error("Expected a number\n");
        return false;
    }

    tok = tokenize(tokstr);
    if (tok != TOK_SEMICOL)
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

    std::string tokstr;
    std::string symmetry;

    // read options until we get to the semicolon.
    LEFReader::token_t tok = tokenize(tokstr);
    while(tok != TOK_SEMICOL)
    {
        symmetry += tokstr;
        symmetry += " ";
        tok = tokenize(tokstr);
    }

    //std::cout << "  SYMMETRY " << symmetry << "\n";

    return true;
};

bool LEFReader::parseForeign()
{
    // FOREIGN <cellname> <number> <number> ; 

    std::string tokstr;
    std::string cellname;
    std::string xnum;
    std::string ynum;

    LEFReader::token_t tok = tokenize(cellname);
    if (tok != TOK_IDENT)
    {
        error("Expected the cell name\n");
        return false;
    }

    tok = tokenize(xnum);
    if (tok != TOK_NUMBER)
    {
        error("Expected a number\n");
        return false;
    }

    tok = tokenize(ynum);
    if (tok != TOK_NUMBER)
    {
        error("Expected a number\n");
        return false;
    }

    tok = tokenize(tokstr);
    if (tok != TOK_SEMICOL)
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
};


bool LEFReader::parseDirection()
{
    // DIRECTION OUTPUT/INPUT/INOUT etc.

    std::string tokstr;
    std::string direction;

    // read options until we get to the semicolon.
    LEFReader::token_t tok = tokenize(direction);
    if (tok != TOK_IDENT)
    {
        error("Expected direction\n");
        return false;
    }

    tok = tokenize(tokstr);
    if ((direction == "OUTPUT") && (tokstr == "TRISTATE"))
    {
        // OUTPUT can be followed by TRISTATE
        // FIXME: use enum.
        direction += " TRISTATE";
        tok = tokenize(tokstr);
    }

    if (tok != TOK_SEMICOL)
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

    std::string tokstr;
    std::string use;

    LEFReader::token_t tok = tokenize(use);
    if (tok != TOK_IDENT)
    {
        error("Expected use\n");
        return false;
    }

    tok = tokenize(tokstr);
    if (tok != TOK_SEMICOL)
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
    std::string tokstr;

    LEFReader::token_t tok = tokenize(tokstr);
    if (tok != TOK_EOL)
    {
        error("Expected EOL\n");
        return false;
    }

    // expect LAYER
    tok = tokenize(tokstr);
    while (tok == TOK_IDENT)
    {
        if (tokstr == "LAYER")
        {
            parseLayer(tokstr);
        }
        else if (tokstr == "END")
        {
            break;
        }
        else
        {
            // eat until ;
            do
            {
                tok = tokenize(tokstr);
            } while(tok != TOK_SEMICOL);

            // eat newline
            tok = tokenize(tokstr);
            if (tok != TOK_EOL)
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

bool LEFReader::parseLayer(std::string &tokstr)
{
    // LAYER <name> ';'
    std::string name;

    LEFReader::token_t tok = tokenize(name);
    if (tok != TOK_IDENT)
    {
        error("Expected layer name\n");
        return false;
    }

    tok = tokenize(tokstr);
    if (tok != TOK_SEMICOL)
    {
        error("Expected a semicolon\n");
        return false;
    }    

    tok = tokenize(tokstr);
    if (tok != TOK_EOL)
    {
        error("Expected an EOL\n");
        return false;
    }    

    //std::cout << "    LAYER " << name << "\n";

    do
    {
        parseLayerItem(tokstr);
    } while(tokstr != "END");
    
    return true;
}

bool LEFReader::parseLayerItem(std::string &tokstr)
{
    LEFReader::token_t tok = tokenize(tokstr);
    if (tok != TOK_IDENT)
    {
        error("Expected RECT or END\n");
    }

    while(1)
    {
        if (tokstr == "END")
        {
            return true;
        }
        else if (tokstr == "RECT")
        {
            parseRect();
        }
        tok = tokenize(tokstr);
    }
}

bool LEFReader::parseRect()
{
    std::string tokstr;
    std::string coords;

    for(uint32_t i=0; i<4; i++)
    {
        LEFReader::token_t tok = tokenize(tokstr);
        if (tok != TOK_NUMBER)
        {
            error("Expected number in RECT\n");
            return false;
        }
        coords += tokstr;
        coords += " ";
    }

    // expect ; 
    LEFReader::token_t tok = tokenize(tokstr);
    if (tok != TOK_SEMICOL)
    {
        error("Expected a semicolon in RECT\n");
        return false;
    }

    // expect EOL
    tok = tokenize(tokstr);
    if (tok != TOK_EOL)
    {
        error("Expected an EOL in RECT\n");
        return false;
    }    

    //std::cout << "      RECT " << coords << "\n";

    return true;
}
