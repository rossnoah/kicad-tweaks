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

#include <board.h>
#include <footprint.h>
#include <pad.h>
#include <pcb_shape.h>


namespace
{

const int MM = pcbIUScale.mmToIU( 1.0 );


PAD* AddCirclePad( FOOTPRINT& aFootprint, const VECTOR2I& aPosition, PAD_ATTRIB aAttribute = PAD_ATTRIB::SMD )
{
    PAD* pad = new PAD( &aFootprint );
    pad->SetPosition( aPosition );
    pad->SetAttribute( aAttribute );
    pad->SetShape( F_Cu, PAD_SHAPE::CIRCLE );
    pad->SetSize( F_Cu, { MM, MM } );
    pad->SetLayerSet( LSET( { F_Cu } ) );
    aFootprint.Add( pad, ADD_MODE::APPEND );
    return pad;
}


void AddCourtyard( FOOTPRINT& aFootprint, const BOX2I& aRect, PCB_LAYER_ID aLayer = F_CrtYd )
{
    PCB_SHAPE* outline = new PCB_SHAPE( &aFootprint, SHAPE_T::POLY );
    outline->SetLayer( aLayer );
    outline->SetPolyPoints( { aRect.GetOrigin(),
                              { aRect.GetRight(), aRect.GetTop() },
                              aRect.GetEnd(),
                              { aRect.GetLeft(), aRect.GetBottom() } } );
    outline->SetWidth( pcbIUScale.mmToIU( 0.05 ) );
    aFootprint.Add( outline, ADD_MODE::APPEND );
}

} // namespace


BOOST_AUTO_TEST_SUITE( FootprintPlacement )


BOOST_AUTO_TEST_CASE( CourtyardCentreWinsOverOriginAndPads )
{
    BOARD     board;
    FOOTPRINT fp( &board );
    fp.SetPosition( { 0, 0 } );

    AddCirclePad( fp, { 0, 0 } );
    AddCirclePad( fp, { 2 * MM, 0 } );

    // Courtyard deliberately off both the origin and the pad centre.
    AddCourtyard( fp, BOX2I( { -1 * MM, -1 * MM }, { 6 * MM, 2 * MM } ) );

    BOOST_CHECK_EQUAL( fp.GetPlacementCentre(), VECTOR2I( 2 * MM, 0 ) );
    BOOST_CHECK_NE( fp.GetPlacementCentre(), fp.GetPosition() );
}


BOOST_AUTO_TEST_CASE( PadsCentreWhenNoCourtyard )
{
    BOARD     board;
    FOOTPRINT fp( &board );
    fp.SetPosition( { 0, 0 } );

    AddCirclePad( fp, { 0, 0 } );
    AddCirclePad( fp, { 2 * MM, 0 } );

    BOOST_CHECK_EQUAL( fp.GetPlacementCentre(), VECTOR2I( 1 * MM, 0 ) );
}


BOOST_AUTO_TEST_CASE( BoundingBoxCentreWhenNoPads )
{
    BOARD     board;
    FOOTPRINT fp( &board );
    fp.SetPosition( { 0, 0 } );
    fp.Reference().SetVisible( false );
    fp.Value().SetVisible( false );

    PCB_SHAPE* rectangle = new PCB_SHAPE( &fp, SHAPE_T::RECTANGLE );
    rectangle->SetStart( { 1 * MM, 0 } );
    rectangle->SetEnd( { 3 * MM, 1 * MM } );
    rectangle->SetLayer( F_Fab );
    fp.Add( rectangle, ADD_MODE::APPEND );

    BOOST_CHECK_EQUAL( fp.GetPlacementCentre(), fp.GetBoundingBox( false ).Centre() );
    BOOST_CHECK_EQUAL( fp.GetPlacementCentre(), VECTOR2I( 2 * MM, MM / 2 ) );
}


BOOST_AUTO_TEST_CASE( PlacementCentreFollowsMove )
{
    BOARD     board;
    FOOTPRINT fp( &board );
    fp.SetPosition( { 0, 0 } );

    AddCirclePad( fp, { 0, 0 } );
    AddCourtyard( fp, BOX2I( { -1 * MM, -1 * MM }, { 2 * MM, 2 * MM } ) );

    const VECTOR2I before = fp.GetPlacementCentre();
    const VECTOR2I delta( 3 * MM, 7 * MM );

    fp.Move( delta );

    BOOST_CHECK_EQUAL( fp.GetPlacementCentre(), before + delta );
}


BOOST_AUTO_TEST_CASE( FlippedFootprintUsesBackCourtyard )
{
    BOARD     board;
    FOOTPRINT fp( &board );
    fp.SetPosition( { 0, 0 } );

    AddCirclePad( fp, { 0, 0 } );
    AddCourtyard( fp, BOX2I( { -1 * MM, -1 * MM }, { 6 * MM, 2 * MM } ) );

    fp.Flip( { 0, 0 }, FLIP_DIRECTION::TOP_BOTTOM );

    BOOST_REQUIRE( fp.IsFlipped() );
    BOOST_REQUIRE_GT( fp.GetCourtyard( B_CrtYd ).OutlineCount(), 0 );

    // The rectangle mirrors about the X axis; its centre had y = 0 so it stays put.
    BOOST_CHECK_EQUAL( fp.GetPlacementCentre(), VECTOR2I( 2 * MM, 0 ) );
}


BOOST_AUTO_TEST_SUITE_END()
