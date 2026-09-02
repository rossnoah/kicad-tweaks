/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright The KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <preview_items/snap_readout.h>

#include <layer_ids.h>
#include <preview_items/preview_utils.h>
#include <view/view.h>

using namespace KIGFX;


SNAP_READOUT::SNAP_READOUT() :
        EDA_ITEM( NOT_USED ),
        m_visible( false )
{
}


void SNAP_READOUT::Set( const VECTOR2I& aPosition, const wxArrayString& aLines )
{
    m_position = aPosition;
    m_lines = aLines;
    m_visible = !aLines.empty();
}


const BOX2I SNAP_READOUT::ViewBBox() const
{
    BOX2I tmp;
    tmp.SetMaximum();
    return tmp;
}


std::vector<int> SNAP_READOUT::ViewGetLayers() const
{
    return { LAYER_SELECT_OVERLAY, LAYER_GP_OVERLAY };
}


void SNAP_READOUT::ViewDraw( int aLayer, VIEW* aView ) const
{
    if( !m_visible || m_lines.empty() )
        return;

    // Right of and above the cursor, clear of the snap indicator icon that sits below-right.
    const VECTOR2D quadrant( -1, 1 );

    PREVIEW::DrawTextNextToCursor( aView, m_position, quadrant, m_lines, aLayer == LAYER_SELECT_OVERLAY );
}
