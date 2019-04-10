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

#ifndef prlefreader_h
#define prlefreader_h

#include <string>
#include <unordered_map>

#include "lef/lefreader.h"

/** LEF Reader + cell database */
class PRLEFReader : public LEFReader
{
public:
    PRLEFReader();

    /** callback for each LEF macro */
    virtual void onMacro(const std::string &macroName) override;

    /** callback for CLASS within a macro */
    virtual void onClass(const std::string &className) override;

    /** callback for FOREIGN within a macro */
    virtual void onForeign(const std::string &foreignName, double x, double y) override;

    /** callback for SIZE within a macro */
    virtual void onSize(double sx, double sy) override;

    /** callback for SYMMETRY within a macro */
    virtual void onSymmetry(const std::string &symmetry) override;

#if 0
    /** callback for ORIGIN within a macro */
    virtual void onOrigin(double x, double y) {}

    /** callback for SITE within a macro */
    virtual void onSite(const std::string &site) {}

    /** callback for PIN within a macro */
    virtual void onPin(const std::string &pinName) {}

    /** callback for PIN direction */
    virtual void onPinDirection(const std::string &direction) {}

    /** callback for PIN use */
    virtual void onPinUse(const std::string &use) {}
#endif


    void doIntegrityChecks();

    class LEFCellInfo_t
    {
    public:
        LEFCellInfo_t() : m_sx(0.0), m_sy(0.0) {}

        std::string     m_name;     ///< LEF cell name
        std::string     m_foreign;  ///< foreign name
        double          m_sx;       ///< size in microns
        double          m_sy;       ///< size in microns
        std::string     m_symmetry; ///< symmetry string taken from LEF.
        bool            m_isFiller;        
    };

    LEFCellInfo_t *getCellByName(const std::string &name) const;
    LEFCellInfo_t *m_parseCell;   ///< current cell being parsed
    
    std::unordered_map<std::string, LEFCellInfo_t*> m_cells;
};

#endif