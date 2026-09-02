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

#ifndef FOOTPRINT_PITCH_H
#define FOOTPRINT_PITCH_H

#include <optional>
#include <vector>

#include <base_units.h>
#include <math/vector2d.h>

class FOOTPRINT;


/**
 * The dominant pad spacing of a footprint along each axis, if it has one.
 *
 * A 0.1" header has x = 2.54 mm; a 2x5 header has both; a QFN has both at its pin pitch; an
 * irregular connector or a part rotated off 90 degrees has neither.
 */
struct FOOTPRINT_PITCH
{
    std::optional<int>    x;       ///< dominant nearest-neighbour spacing of pad columns (IU)
    std::optional<int>    y;       ///< dominant nearest-neighbour spacing of pad rows (IU)
    std::vector<VECTOR2I> centres; ///< world-space centres of the pads that took part

    bool Any() const { return x.has_value() || y.has_value(); }
};


/**
 * The dominant spacing of a set of coordinates.
 *
 * Coordinates within aTolerance of each other are merged, consecutive gaps are histogrammed
 * in buckets of aTolerance, and the biggest bucket is returned when it covers at least half
 * of the gaps.
 *
 * @param aCoordinates the coordinates (consumed; order does not matter)
 * @param aTolerance how far apart two coordinates can be and still count as one
 * @param aMinDistinct the minimum number of distinct coordinates for a spacing to exist
 * @return the spacing, or nothing when the coordinates have no dominant rhythm
 */
std::optional<int> DominantSpacing( std::vector<int> aCoordinates, int aTolerance, size_t aMinDistinct = 2 );


/**
 * Detect the pad pitch of a footprint from its copper pads (NPTH and aperture pads are
 * ignored).
 */
FOOTPRINT_PITCH DetectFootprintPitch( const FOOTPRINT& aFootprint,
                                      int aTolerance = pcbIUScale.mmToIU( 0.01 ), size_t aMinDistinct = 2 );


/// True when two pitches describe the same lattice.
bool SamePitch( int aFirst, int aSecond, int aTolerance = pcbIUScale.mmToIU( 0.01 ) );

#endif // FOOTPRINT_PITCH_H
