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

#ifndef linereader_h
#define linereader_h

#include <iterator>
#include <iostream>
#include <list>
#include <string>
#include <string_view>

/** Takes a line and produces a list of std::string_view objects,
 *  one for each whitespace-separated chunk.
 **/
class TextChunkifier
{
public:
    TextChunkifier(const std::string &separators) : m_separators(separators) {}

    /** Submit a string and generate a list of std::string_view objects/chunks.
     *  Use iterators to access the chunks **/
    void submitString(const std::string &line)
    {
        uint32_t state = 0;
        int32_t  idx = 0;
        int32_t  startIdx = -1; // -1 signals no start found
        
        m_chunks.clear();

        while(idx < line.size())
        {
            char c = line[idx];
            switch(state)
            {
            case 0: // idle state
                if (m_separators.find(c) != std::string::npos)
                {
                    startIdx = -1;  // no start found
                    continue;
                }
                else
                {
                    // non whitespace char ->
                    // the start of a new chunk
                    startIdx = idx;
                    state = 1;
                }
                break;
            case 1: // inside a chunk
                if (m_separators.find(c) != std::string::npos)
                {               
                    // chunk ended, so we emit it!
                    std::string_view chunk{line.c_str() + startIdx, static_cast<uint32_t>(idx - startIdx)};
                    m_chunks.push_back(chunk);
                    startIdx = -1;  // no start found
                    state = 0;
                }         
                break;    
            default:
                state = 0;   
            }
            idx++;
        }

        // check if there was a chunk at the end of the line.
        // if there is one, emit it!
        if (startIdx != -1)
        {
            std::string_view chunk{line.c_str() + startIdx, static_cast<uint32_t>(idx - startIdx)};
            m_chunks.push_back(chunk);            
        }
    }

    /** get the first chunk as a std::string_view object */
    std::string_view getFirstChunk() const
    {
        if (m_chunks.size() < 1)
        { 
            return std::string_view();
        }
        else
        {
            return m_chunks.front();
        }
    }

    typedef std::list<std::string_view> itertype;

    inline itertype::iterator begin() noexcept { return m_chunks.begin(); }
    inline itertype::const_iterator cbegin() const noexcept { return m_chunks.cbegin(); }
    inline itertype::iterator end() noexcept { return m_chunks.end(); }
    inline itertype::const_iterator cend() const noexcept { return m_chunks.cend(); }

protected:
    std::string m_separators;
    std::list<std::string_view > m_chunks;
};


/** a stream based line reader with accept function to allow
    lexing/parsing. */
class LineReader
{
public:
    LineReader(std::istream &is) 
        : m_is(is)
    {
        m_lineNum = 0;
        nextLine();
    }

    /** accept the current line and advance to 
        the next */
    void accept()
    {
        nextLine();
    }

    /** true if the end of file/stream is encountered */ 
    bool eof() const
    {
        return m_eof;
    }

    /** return the current line number */
    uint32_t getLineNumber() const
    {
        return m_lineNum;
    }

    /** return the current line as string_view */
    const std::string& getLine() const
    {
        return m_line;
    }

protected:

    /** read the next line and update the line number 
        and eof boolean.
    */
    void nextLine()
    {
        if (!std::getline(m_is, m_line))
        {
            m_eof = true;
        }
        else
        {
            m_lineNum++;
            m_eof = false;
        }
    }

    std::string     m_line;
    std::istream    &m_is;

    bool            m_eof;
    uint32_t        m_lineNum;
};


/** a stream based line reader with accept function to allow
    lexing/parsing. It will split the string into chunks based
    on a set of separators 
*/
class ChunkyLineReader
{
public:
    ChunkyLineReader(std::istream &is, const std::string separators = " \t") 
        : m_is(is), m_chunkifier(separators)
    {
        m_lineNum = 0;
        nextLine();
    }

    /** accept the current line and advance to 
        the next */
    void accept()
    {
        nextLine();
    }

    /** true if the end of file/stream is encountered */ 
    bool eof() const
    {
        return m_eof;
    }

    /** return the current line number */
    uint32_t getLineNumber() const
    {
        return m_lineNum;
    }

    /** get this first chunk in the list */
    std::string_view getFirstChunk() const
    {
        return m_chunkifier.getFirstChunk();
    }

    typedef std::list<std::string_view> itertype;

    /** iterator to access the string_view chunks */
    inline itertype::iterator begin() noexcept { return m_chunkifier.begin(); }

    /** iterator to access the string_view chunks */
    inline itertype::const_iterator cbegin() const noexcept { return m_chunkifier.cbegin(); }

    /** iterator to access the string_view chunks */
    inline itertype::iterator end() noexcept { return m_chunkifier.end(); }

    /** iterator to access the string_view chunks */
    inline itertype::const_iterator cend() const noexcept { return m_chunkifier.cend(); }

protected:

    /** read the next line and update the line number 
        and eof boolean.
    */
    void nextLine()
    {
        if (!std::getline(m_is, m_line))
        {
            m_eof = true;
        }
        else
        {
            m_lineNum++;
            m_eof = false;
            m_chunkifier.submitString(m_line);
        }
    }

    TextChunkifier  m_chunkifier;
    std::string     m_line;

    std::istream    &m_is;

    bool            m_eof;
    uint32_t        m_lineNum;
};

#endif