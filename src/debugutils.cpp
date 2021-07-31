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

#include "debugutils.h"
#include <sstream>

void DebugUtils::dumpToConsole(const PRLEFReader::LEFCellInfo_t *cell)
{
    std::stringstream ss;
    ss << "\n";

    if (cell != nullptr)
    {
        ss << "Name:    " << cell->m_name.c_str() << "\n";
        ss << "Foreign  " << cell->m_foreign.c_str() << "\n";
        ss << "Width    " << cell->m_sx << "\n";
        ss << "Height   " << cell->m_sy << "\n";
        ss << "Type     " << (cell->m_isFiller ? "FILLER" : "REGULAR") << "\n";
        ss << "Symmetry " << cell->m_symmetry.c_str() << "\n";
    }
    else
    {
        ss << "Error: cell is a nullptr!\n";
    }
    doLog(LOG_INFO, ss.str());
}

