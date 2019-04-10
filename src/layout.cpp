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

#include <cmath>
#include <iostream>
#include "logging.h"
#include "layout.h"


Layout::Layout(direction_t dir) : m_dir(dir), m_edgePos(0.0), m_insertFlexSpacer(true)
{
    m_firstCorner = nullptr;
    m_lastCorner  = nullptr;
}

Layout::~Layout()
{
    for(auto c : m_items)
    {
        delete c;
    }
}

double Layout::getMinSize() const
{
    double total = 0.0;
    for(auto item : m_items)
    {
        if (item->m_size >= 0)
        {
            total += item->m_size;
        }
    }

    if (m_firstCorner != nullptr)
    {
        total += m_firstCorner->m_size;
    }

    if (m_lastCorner != nullptr)
    {
        total += m_lastCorner->m_size;
    }

    return total;
}

void Layout::prepareForLayout()
{
    // if there are no items on this edge,
    // add filler cells.
    if (m_items.size() == 0)
    {
        auto filler = new LayoutItem(LayoutItem::LayoutItemType::TYPE_FLEXSPACE);
        m_items.push_back(filler);
        return;
    }

    m_insertFlexSpacer = false;
    for(auto item : m_items)
    {
        setItemPos(item, -1);
        setItemEdgePos(item);
    }

    // check if last item is a CELL
    // if so, insert a FLEXSPACER
    if (m_items.back()->m_ltype == LayoutItem::TYPE_CELL)
    {
        LayoutItem *item = new LayoutItem(LayoutItem::TYPE_FLEXSPACE);
        item->m_size = -1;
        setItemPos(item, -1);
        setItemEdgePos(item);
        m_items.push_back(item);
    }
}

bool Layout::doLayout()
{
    prepareForLayout();

    // get the minimum width of cells
    double minx = getMinSize();

    if (minx > m_dieSize)
    {
        doLog(LOG_ERROR,"Layout items are larger than the available die size\n");
        doLog(LOG_ERROR,"  size = %f  items = %f\n", m_dieSize, minx);
        return false;
    }

    // count the number of FLEXSPACE items
    uint32_t flexSpaceItems = 0;
    for(auto item : m_items)
    {
        if (item->m_ltype == LayoutItem::TYPE_FLEXSPACE)
        {
            flexSpaceItems++;
        }
    }

    double meanFlexSpaceSize = (m_dieSize - minx) / static_cast<double>(flexSpaceItems);
    double pos = 0;

    // position the first corner
    if (m_firstCorner != nullptr)
    {
        pos += m_firstCorner->m_size;
        setItemPos(m_firstCorner, 0.0);
        setItemEdgePos(m_firstCorner);
    }

    // FIXME: make grid configurable! 
    double grid = 1.0;
    double newPos;
    double error = 0.0;
    for(auto item : m_items)
    {
        setItemPos(item, pos);

        // advance the position depending on the type of
        // item
        switch(item->m_ltype)
        {
        case LayoutItem::TYPE_FLEXSPACE:
            newPos = pos + meanFlexSpaceSize + error;
            newPos = std::floor(newPos / grid) * grid;  // round new position to grid
            item->m_size = newPos - pos;                // set size of FLEXSPACE
            pos = newPos;
            error += (meanFlexSpaceSize - item->m_size);
            break;
        case LayoutItem::TYPE_CELL:
            pos += item->m_size;
            break;
        case LayoutItem::TYPE_CORNER:
            pos += item->m_size;
            break;
        case LayoutItem::TYPE_FIXEDSPACE:
            pos += item->m_size;
            break;
        }
    }

    // position the last corner
    if (m_lastCorner != nullptr)
    {
        setItemPos(m_lastCorner, m_dieSize - m_lastCorner->m_size);
        setItemEdgePos(m_lastCorner);
    }

    return true;
}

void Layout::dump()
{
    if (m_firstCorner != nullptr)
    {
        auto c = m_firstCorner;
        std::cout << c->m_instance << " : " << c->m_cellname << " " << getItemPos(c) << "\n";
    }

    for(auto c : m_items)
    {
        if ((c->m_ltype == LayoutItem::TYPE_CELL) || (c->m_ltype == LayoutItem::TYPE_CORNER))
        {
            std::cout << c->m_instance << " : " << c->m_cellname << " " << getItemPos(c) << "\n";
        }
    }

    if (m_lastCorner != nullptr)
    {
        auto c = m_lastCorner;
        std::cout << c->m_instance << " : " << c->m_cellname << " " << getItemPos(c) << "\n";
    }

}
