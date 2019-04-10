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

#ifndef fillerhandler_h
#define fillerhandler_h

#include <string>
#include <list>

class FillerHandler
{
public:
    FillerHandler() : m_sorted(false) {}

    /** add a filler cell to the list of cells */
    void addFillerCell(const std::string &cellName, double width)
    {
        m_sorted = false;
        m_fillerCells.push_back(std::make_pair(width, cellName));
    }
    
    /** get largest filler cell the is smaller or equal to 
     *  the given width and return it's width and name.
     * 
     *  if no filler cell is found, -1 is returned.
     **/
    double getFillerCell(double width, std::string &outCellName)
    {
        if (!m_sorted)
        {
            m_sorted = true;
            m_fillerCells.sort(cellCompare);
        }
        
        for(auto cell : m_fillerCells)
        {
            if (cell.first <= width)
            {
                outCellName = cell.second;
                return cell.first;
            }
        }
        return -1.0;    // not found
    }

    /** return the number of filler cells available */
    size_t getCellCount() const
    {
        return m_fillerCells.size();
    }

    /** Get the smallest filler cell as a hint for the 
        actual grid spacing of IO cells.

        returns -1.0 on error.
    */
    double getSmallestWidth()
    {
        if (!m_sorted)
        {
            m_sorted = true;
            m_fillerCells.sort(cellCompare);
        }

        if (!m_fillerCells.empty())
            return m_fillerCells.back().first;        
        
        return -1.0;
    }

protected:

    /** pair: filler cell width & filler cell name. */
    typedef std::pair<double, std::string> fillerInfo_t;

    static bool cellCompare(const fillerInfo_t &c1, const fillerInfo_t c2)
    {
        return c1.first > c2.first;
    }

    bool m_sorted;  ///< whether the filler cell list has been sorted (largest first).

    std::list<fillerInfo_t> m_fillerCells;
};

#endif