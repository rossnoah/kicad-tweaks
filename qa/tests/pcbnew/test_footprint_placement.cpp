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
#include <footprint_pitch.h>


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
    PCB_SHAPE* outline = new PCB_SHAPE( &aFootprint, SHAPE_T::RECTANGLE );
    outline->SetLayer( aLayer );
    outline->SetStart( aRect.GetOrigin() );
    outline->SetEnd( aRect.GetEnd() );
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

    // The text-free bounding box also takes in the origin marker, so only the identity with
    // the bounding box is asserted, not a hand-computed centre.
    BOOST_CHECK_EQUAL( fp.GetPlacementCentre(), fp.GetBoundingBox( false ).Centre() );
    BOOST_CHECK_NE( fp.GetPlacementCentre(), fp.GetPosition() );
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
    fp.BuildCourtyardCaches();

    BOOST_REQUIRE( fp.IsFlipped() );
    BOOST_REQUIRE_GT( fp.GetCourtyard( B_CrtYd ).OutlineCount(), 0 );

    // The rectangle mirrors about the X axis; its centre had y = 0 so it stays put.
    BOOST_CHECK_EQUAL( fp.GetPlacementCentre(), VECTOR2I( 2 * MM, 0 ) );
}


BOOST_AUTO_TEST_CASE( DominantSpacingOfRegularRow )
{
    const int inch = pcbIUScale.mmToIU( 2.54 );
    const int tol = pcbIUScale.mmToIU( 0.01 );

    std::optional<int> pitch = DominantSpacing( { 0, inch, 2 * inch, 3 * inch }, tol );
    BOOST_REQUIRE( pitch.has_value() );
    BOOST_CHECK_EQUAL( *pitch, inch );

    // Order does not matter, and near-duplicates collapse into one column.
    pitch = DominantSpacing( { 3 * inch, 0, inch + 3, 2 * inch, inch - 2 }, tol );
    BOOST_REQUIRE( pitch.has_value() );
    BOOST_CHECK_EQUAL( *pitch, inch );

    // Two pads still have a spacing.
    pitch = DominantSpacing( { 0, 2 * MM }, tol );
    BOOST_REQUIRE( pitch.has_value() );
    BOOST_CHECK_EQUAL( *pitch, 2 * MM );
}


BOOST_AUTO_TEST_CASE( DominantSpacingRejectsIrregularRows )
{
    const int tol = pcbIUScale.mmToIU( 0.01 );

    BOOST_CHECK( !DominantSpacing( { 0 }, tol ).has_value() );
    BOOST_CHECK( !DominantSpacing( { 0, 3 }, tol ).has_value() );
    BOOST_CHECK( !DominantSpacing( { 0, MM, 3 * MM, 7 * MM, 15 * MM }, tol ).has_value() );
    BOOST_CHECK( !DominantSpacing( { 0, 2 * MM, 4 * MM }, tol, 4 ).has_value() );
}


BOOST_AUTO_TEST_CASE( DetectFootprintPitchOnBothAxes )
{
    const int inch = pcbIUScale.mmToIU( 2.54 );
    BOARD     board;
    FOOTPRINT fp( &board );
    fp.SetPosition( { 0, 0 } );

    for( int row = 0; row < 2; ++row )
    {
        for( int col = 0; col < 5; ++col )
            AddCirclePad( fp, { col * inch, row * inch } );
    }

    // A mounting hole must not disturb the rhythm.
    AddCirclePad( fp, { 7 * MM, 9 * MM }, PAD_ATTRIB::NPTH );

    FOOTPRINT_PITCH pitch = DetectFootprintPitch( fp );
    BOOST_REQUIRE( pitch.x.has_value() );
    BOOST_REQUIRE( pitch.y.has_value() );
    BOOST_CHECK_EQUAL( *pitch.x, inch );
    BOOST_CHECK_EQUAL( *pitch.y, inch );
    BOOST_CHECK_EQUAL( pitch.centres.size(), 10 );
    BOOST_CHECK( SamePitch( *pitch.x, inch ) );
    BOOST_CHECK( !SamePitch( *pitch.x, MM ) );
}


BOOST_AUTO_TEST_CASE( DetectFootprintPitchFollowsRotation )
{
    const int inch = pcbIUScale.mmToIU( 2.54 );
    BOARD     board;
    FOOTPRINT fp( &board );
    fp.SetPosition( { 0, 0 } );

    for( int col = 0; col < 4; ++col )
        AddCirclePad( fp, { col * inch, 0 } );

    FOOTPRINT_PITCH pitch = DetectFootprintPitch( fp );
    BOOST_REQUIRE( pitch.x.has_value() );
    BOOST_CHECK( !pitch.y.has_value() ); // a single row has no vertical rhythm

    // A quarter turn swaps the axes: the row becomes a column.
    fp.Rotate( { 0, 0 }, EDA_ANGLE( 90.0, DEGREES_T ) );

    pitch = DetectFootprintPitch( fp );
    BOOST_CHECK( !pitch.x.has_value() );
    BOOST_REQUIRE( pitch.y.has_value() );
    BOOST_CHECK_EQUAL( *pitch.y, inch );
}


BOOST_AUTO_TEST_SUITE_END()
