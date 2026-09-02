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

#include <qa_utils/wx_utils/unit_test_utils.h>

#include <cmath>

#include <geometry/grid_geometry.h>
#include <trigo.h>


namespace
{

GRID_GEOMETRY MakeCartesian( const VECTOR2D& aOrigin, double aPitch, double aOrientation = 0.0 )
{
    GRID_GEOMETRY grid;
    grid.kind = GRID_GEOMETRY::KIND::CARTESIAN;
    grid.origin = aOrigin;
    grid.pitch = { aPitch, aPitch };
    grid.orientation = aOrientation;
    grid.extent = { 1000.0, 1000.0 };
    return grid;
}

} // namespace


BOOST_AUTO_TEST_SUITE( GridGeometry )


BOOST_AUTO_TEST_CASE( CellCentreIsHalfPitchOffGridPoints )
{
    const GRID_GEOMETRY grid = MakeCartesian( { 0, 0 }, 100 );

    BOOST_CHECK_EQUAL( grid.SnapToCellCentre( { 120, 30 } ), VECTOR2D( 150, 50 ) );
    BOOST_CHECK_EQUAL( grid.SnapToCellCentre( { -20, -70 } ), VECTOR2D( -50, -50 ) );

    // A grid point is a cell corner: it lands in the cell above/right of it.
    BOOST_CHECK_EQUAL( grid.SnapToCellCentre( { 200, 300 } ), VECTOR2D( 250, 350 ) );
}


BOOST_AUTO_TEST_CASE( CellCentreHonoursOrigin )
{
    const GRID_GEOMETRY grid = MakeCartesian( { 5, 5 }, 100 );

    BOOST_CHECK_EQUAL( grid.SnapToCellCentre( { 120, 30 } ), VECTOR2D( 155, 55 ) );
}


BOOST_AUTO_TEST_CASE( CellCentreFollowsRotatedGrid )
{
    const double        orientation = 30.0 * M_PI / 180.0;
    const GRID_GEOMETRY grid = MakeCartesian( { 40, -20 }, 100, orientation );
    const VECTOR2D      point( 333, 121 );
    const VECTOR2D      snapped = grid.SnapToCellCentre( point );

    // Back in the grid's own frame the result must sit at odd multiples of half a pitch.
    const VECTOR2D local = GetRotated( snapped - grid.origin, EDA_ANGLE( -orientation, RADIANS_T ) );

    auto halfPitchResidual =
            []( double aCoord )
            {
                return std::fmod( std::fabs( aCoord ) + 50.0, 100.0 );
            };

    BOOST_CHECK_SMALL( halfPitchResidual( local.x ), 1e-6 );
    BOOST_CHECK_SMALL( halfPitchResidual( local.y ), 1e-6 );

    // And it is the cell the point is actually in: never more than a diagonal half-cell away.
    BOOST_CHECK_LT( ( snapped - point ).EuclideanNorm(), 50.0 * std::sqrt( 2.0 ) + 1e-6 );
}


BOOST_AUTO_TEST_CASE( PolarGridFallsBackToSnap )
{
    GRID_GEOMETRY grid;
    grid.kind = GRID_GEOMETRY::KIND::POLAR;
    grid.origin = { 0, 0 };
    grid.pitch = { 100, M_PI / 4 };
    grid.extent = { 1000, 2 * M_PI };

    const VECTOR2D point( 210, 190 );

    BOOST_CHECK_EQUAL( grid.SnapToCellCentre( point ), grid.Snap( point ) );
}


BOOST_AUTO_TEST_CASE( DegeneratePitchReturnsPoint )
{
    GRID_GEOMETRY grid = MakeCartesian( { 0, 0 }, 0 );
    const VECTOR2D point( 210, 190 );

    BOOST_CHECK_EQUAL( grid.SnapToCellCentre( point ), point );
}


BOOST_AUTO_TEST_SUITE_END()
