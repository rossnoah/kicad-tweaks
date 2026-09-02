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

#include <footprint_pitch.h>

#include <algorithm>
#include <cmath>
#include <map>

#include <footprint.h>
#include <math/util.h>
#include <pad.h>


std::optional<int> DominantSpacing( std::vector<int> aCoordinates, int aTolerance, size_t aMinDistinct )
{
    if( aCoordinates.size() < 2 )
        return std::nullopt;

    aTolerance = std::max( 1, aTolerance );
    std::sort( aCoordinates.begin(), aCoordinates.end() );

    // Merge coordinates within tolerance into one, keeping the mean of the run.
    std::vector<double> distinct;
    double              runSum = aCoordinates.front();
    int                 runCount = 1;
    int                 runStart = aCoordinates.front();

    for( size_t i = 1; i < aCoordinates.size(); ++i )
    {
        if( aCoordinates[i] - runStart <= aTolerance )
        {
            runSum += aCoordinates[i];
            ++runCount;
        }
        else
        {
            distinct.push_back( runSum / runCount );
            runSum = aCoordinates[i];
            runCount = 1;
            runStart = aCoordinates[i];
        }
    }

    distinct.push_back( runSum / runCount );

    if( distinct.size() < std::max<size_t>( 2, aMinDistinct ) )
        return std::nullopt;

    // Histogram consecutive gaps in tolerance-sized buckets.
    std::map<long, std::pair<int, double>> buckets; // bucket -> (count, sum)
    const size_t                           gapCount = distinct.size() - 1;

    for( size_t i = 1; i < distinct.size(); ++i )
    {
        const double gap = distinct[i] - distinct[i - 1];
        const long   bucket = std::lround( gap / aTolerance );
        auto&        entry = buckets[bucket];
        entry.first += 1;
        entry.second += gap;
    }

    const auto dominant = std::max_element( buckets.begin(), buckets.end(),
                                            []( const auto& aLeft, const auto& aRight )
                                            {
                                                return aLeft.second.first < aRight.second.first;
                                            } );

    if( dominant == buckets.end() || static_cast<size_t>( dominant->second.first ) * 2 < gapCount )
        return std::nullopt;

    const int pitch = KiROUND( dominant->second.second / dominant->second.first );

    return pitch > 0 ? std::optional<int>( pitch ) : std::nullopt;
}


FOOTPRINT_PITCH DetectFootprintPitch( const FOOTPRINT& aFootprint, int aTolerance, size_t aMinDistinct )
{
    FOOTPRINT_PITCH  result;
    std::vector<int> xs;
    std::vector<int> ys;

    for( const PAD* pad : aFootprint.Pads() )
    {
        if( pad->GetAttribute() == PAD_ATTRIB::NPTH || pad->IsAperturePad() )
            continue;

        const VECTOR2I centre = pad->GetPosition();
        result.centres.push_back( centre );
        xs.push_back( centre.x );
        ys.push_back( centre.y );
    }

    result.x = DominantSpacing( std::move( xs ), aTolerance, aMinDistinct );
    result.y = DominantSpacing( std::move( ys ), aTolerance, aMinDistinct );

    return result;
}


bool SamePitch( int aFirst, int aSecond, int aTolerance )
{
    return std::abs( aFirst - aSecond ) <= aTolerance;
}
