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

#ifndef padringdb_h
#define padringdb_h

#include "configreader.h"
#include "prlefreader.h"
#include "layout.h"
#include "logging.h"

class PadringDB : public ConfigReader
{
public:

    PadringDB() : m_north(Layout::DIR_HORIZONTAL),
        m_south(Layout::DIR_HORIZONTAL),
        m_east(Layout::DIR_VERTICAL),
        m_west(Layout::DIR_VERTICAL),
        m_grid(1.0) 
    {
        m_south.setEdgePos(0.0);
        m_west.setEdgePos(0.0);
    }

    /** callback for a corner */
    virtual void onCorner(
        const std::string &instance,
        const std::string &location,
        const std::string &cellname) override
    {
        PRLEFReader::LEFCellInfo_t *cell = m_lefreader.getCellByName(cellname);
        if (cell == nullptr)
        {
            doLog(LOG_ERROR,"Cannot find cell %s in the LEF database\n", cellname.c_str());
            return;
        }

        LayoutItem *item_x = new LayoutItem(LayoutItem::TYPE_CORNER);
        item_x->m_instance = instance;
        item_x->m_cellname = cellname;
        item_x->m_location = location;
        item_x->m_size = cell->m_sx;
        item_x->m_lefinfo = cell;

        LayoutItem *item_y = new LayoutItem(LayoutItem::TYPE_CORNER);
        item_y->m_instance = instance;
        item_y->m_cellname = cellname;
        item_y->m_location = location;
        item_y->m_size = cell->m_sy;
        item_y->m_lefinfo = cell;

        // Corner cells should be symmetrical
        // i.e. width = height.
        if (location == "NE")
        {
            // ROT 180
            m_north.setLastCorner(item_x);
            m_east.setLastCorner(item_y);
        }
        else if (location == "NW")
        {
            // ROT 90
            m_north.setFirstCorner(item_y);
            m_west.setLastCorner(item_x);
        }
        else if (location == "SE")
        {
            // ROT 270
            m_south.setLastCorner(item_y);
            m_east.setFirstCorner(item_x);            
        }
        else if (location == "SW")
        {
            // ROT 0
            m_south.setFirstCorner(item_x);
            m_west.setFirstCorner(item_y);
        }
    }

    /** callback for a pad */
    virtual void onPad(
        const std::string &instance,
        const std::string &location,
        const std::string &cellname) override
    {
        PRLEFReader::LEFCellInfo_t *cell = m_lefreader.getCellByName(cellname);
        if (cell == nullptr)
        {
            doLog(LOG_ERROR,"Cannot find cell %s in the LEF database\n", cellname.c_str());
            return;
        }

        LayoutItem *item = new LayoutItem(LayoutItem::TYPE_CELL);
        item->m_instance = instance;
        item->m_cellname = cellname;
        item->m_location = location;
        item->m_size = cell->m_sx;
        item->m_lefinfo = cell;

        // Corner cells should be symmetrical
        // i.e. width = height.
        if (location == "N")
        {
            m_north.addItem(item);
        }
        else if (location == "W")
        {
            m_west.addItem(item);
        }
        else if (location == "S")
        {
            m_south.addItem(item);
        }
        else if (location == "E")
        {
            m_east.addItem(item);
        }
        else
        {
            doLog(LOG_ERROR, "Incorrect location on PAD %s\n", cellname.c_str());
        }

        m_lastLocation = location;
    }

    /** callback for die area in microns */
    virtual void onArea(double x, double y) override
    {
        m_dieWidth  = x;
        m_dieHeight = y;
        
        m_north.setDieSize(x);
        m_south.setDieSize(x);
        m_east.setDieSize(y);
        m_west.setDieSize(y);

        m_north.setEdgePos(y);
        m_east.setEdgePos(x);        
    }

    /** callback for grid spacing in microns */
    virtual void onGrid(double grid) override
    {
        m_grid = grid;
    }

    /** callback for filler cell prefix string */
    virtual void onFiller(const std::string &filler) override
    {
        m_fillerPrefix = filler;
    }

    /** callback for space in microns */
    virtual void onSpace(double space) override
    {
        LayoutItem *item = new LayoutItem(LayoutItem::TYPE_FIXEDSPACE);
        item->m_size = space;

        if (m_lastLocation == "N")
        {
            m_north.addItem(item);
        }
        else if (m_lastLocation == "W")
        {
            m_west.addItem(item);
        }
        else if (m_lastLocation == "S")
        {
            m_south.addItem(item);
        }
        else if (m_lastLocation == "E")
        {
            m_east.addItem(item);
        }
    }

    /** callback for offset in microns */
    virtual void onOffset(double offset)
    {
        //FIXME: offset not supported yet!
    }

    void doLayout()
    {
        m_north.doLayout();
        m_south.doLayout();
        m_west.doLayout();
        m_east.doLayout();
    }

    Layout m_north;
    Layout m_south;
    Layout m_east;
    Layout m_west;

    double m_dieHeight;
    double m_dieWidth;
    double m_grid;

    std::string m_fillerPrefix;
    std::string m_lastLocation;

    PRLEFReader m_lefreader;
};

#endif
