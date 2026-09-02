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

#ifndef PREVIEW_ITEMS_SNAP_READOUT_H
#define PREVIEW_ITEMS_SNAP_READOUT_H

#include <eda_item.h>
#include <math/vector2d.h>
#include <wx/arrstr.h>

namespace KIGFX
{

/**
 * A short text readout drawn next to the cursor during a drag, saying what the snap system
 * did with the moving item ("centre on 0.5 mm tile", "in step with J1", "free").
 *
 * Drawn with the same two-pass drop-shadow scheme as the drawing assistants so it stays
 * legible over copper.
 */
class SNAP_READOUT : public EDA_ITEM
{
public:
    SNAP_READOUT();

    /// Set the lines to draw at aPosition and make the readout visible.
    void Set( const VECTOR2I& aPosition, const wxArrayString& aLines );

    void Hide() { m_visible = false; }
    bool IsVisible() const { return m_visible; }

    const wxArrayString& Lines() const { return m_lines; }

    const BOX2I ViewBBox() const override;

    std::vector<int> ViewGetLayers() const override;

    void ViewDraw( int aLayer, VIEW* aView ) const override;

    wxString GetClass() const override { return wxT( "SNAP_READOUT" ); }

#if defined( DEBUG )
    void Show( int x, std::ostream& st ) const override {}
#endif

private:
    VECTOR2I      m_position;
    wxArrayString m_lines;
    bool          m_visible;
};

} // namespace KIGFX

#endif // PREVIEW_ITEMS_SNAP_READOUT_H
