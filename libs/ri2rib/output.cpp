// Aqsis
// Copyright  1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

/** \file
 *  \brief Libri2rib output base class implementation.
 *  \author Lionel J. Lacour (intuition01@online.fr)
 */

#if _MSC_VER
#pragma warning(disable : 4786)
#endif

#include "output.h"

#include "stddef.h"
#include <cstring>

#include "error.h"
#include "context.h"
#include <aqsis/util/exception.h>

namespace libri2rib {

// Bring in some names from the Aqsis namespace.
//
// \todo: Consider replacing namespace libri2rib with Aqsis or remove this using.
using namespace Aqsis;

#define PR(x,y)  printRequest(x,y)
#ifdef	PI
#undef	PI
#endif
#define PI(x)  printInteger(x)
#define PF(x)  printFloat(x)
#define PS(x)  printString(x)
#define S   printSpace()
#define EOL printEOL()



CqOutput::CqOutput( const char *name, int fdesc,
                    SqOptions::EqCompression comp)
		: m_Dictionary(true),
		m_ColorNComps( 3 ),
		m_ObjectHandle( 1 ),
		m_LightHandle( 1 )
{
	switch ( comp )
	{
			case SqOptions::Compression_None:
			out = new CqStreamFDesc();
			break;
			case SqOptions::Compression_Gzip:
			out = new CqStreamGzip();
			break;
	}

	if ( name != RI_NULL )
	{
		out->openFile( name );
	}
	else
	{
		out->openFile( fdesc );
	}

	SqSteps a = {RI_BEZIERSTEP, RI_BEZIERSTEP};
	m_Steps.push( a );
}

CqOutput::~CqOutput()
{
	out->closeFile();
	delete out;
}





// *********************************************************************
// ******* ******* ******* STEPS STACK FUNCTIONS ******* ******* *******
// *********************************************************************
void CqOutput::push()
{
	m_Steps.push( m_Steps.top() );
}
void CqOutput::pop()
{
	if ( m_Steps.size() == 0 )
	{
		return ;
	}
	m_Steps.pop();
}






// **************************************************************************
// ******* ******* ******** CONTEXT NESTING FUNCTIONS ******* ******* *******
// **************************************************************************
const char* const CqOutput::m_blockNames[] =
    {
        "RiBegin/RiEnd", "Frame", "World", "Attribute", "Transform",
        "Solid", "Object", "Motion", "Resource"
    };
const RtInt CqOutput::m_blockErrors[] =
    {
        RIE_NESTING, RIE_NESTING, RIE_NESTING, RIE_NESTING, RIE_NESTING,
        RIE_BADSOLID, RIE_NESTING, RIE_BADMOTION, RIE_NESTING
    };
const CqOutput::EqFunctions CqOutput::m_blockFunctions[] =
    {
		LAST_Function, LAST_Function,  // < There's no function type for RiBegin/End
		FrameBegin, FrameEnd, WorldBegin, WorldEnd, AttributeBegin, AttributeEnd,
		TransformBegin, TransformEnd, SolidBegin, SolidEnd, ObjectBegin, ObjectEnd,
		MotionBegin, MotionEnd, ResourceBegin, ResourceEnd
    };


/**
 * Begins a block nesting.
 */
void CqOutput::beginNesting(EqBlocks type)
{
	// we cannot have more than one Ri, Frame or World block, so check that
	//  these are not present already
	if ((type == B_Ri) || (type == B_Frame) || (type == B_World))
	{
		if (nestingContains(type))
		{
			throw CqError(RIE_NESTING, RIE_SEVERE,
			              "Attempt to open another ",
			              m_blockNames[type],
			              " block when one is already open.", false);
		}
	}

	if(type != B_Ri)
	{
		std::string requestName = std::string(m_blockNames[type]) + "Begin";
		printRequest(requestName.c_str(), m_blockFunctions[type*2]);
	}

	// everything else is fine, so open the block
	m_nesting.push_back(type);
}



/**
 * Ends a block nesting.
 *
 * A block nesting can only be ended in a valid way if the previous entry in
 * the block stack is of the same type.  So, for example, the following is
 * valid:
 *      RiTransformBegin();
 *              RiAttributeBegin();
 *              RiAttributeEnd();       // Note: block is Attribute
 *      RiTransformEnd();
 * However, the following nesting is NOT valid:
 *      RiTransformBegin();
 *              RiAttributeBegin();
 *      RiTransformEnd();               // Note: block is *not* Transform
 *              RiAttributeEnd();
 */
void CqOutput::endNesting(EqBlocks type)
{
	// check for an empty block stack.  this is not valid, since we
	//  can't close a block that hasn't been opened.
	if (m_nesting.empty())
	{
		// invalid
		throw CqError(m_blockErrors[type], RIE_SEVERE,
		              "Cannot close block of type ", m_blockNames[type],
		              " when no blocks have yet been opened.", false);
	}

	// check that the block we're closing is of the correct type
	EqBlocks curBlock = m_nesting.back();
	if (curBlock != type)
	{
		// invalid - we raise the error based upon the last entry on
		//  the stack
		std::stringstream strBuf;
		strBuf << "Bad nesting: Attempting to close block of type "
		<< m_blockNames[type]
		<< " within a "
		<< m_blockNames[curBlock]
		<< " block." << std::ends;
		throw CqError(m_blockErrors[curBlock], RIE_SEVERE,
		              &strBuf.str()[0], false);
	}

	m_nesting.pop_back();

	if(type != B_Ri)
	{
		std::string requestName = std::string(m_blockNames[type]) + "End";
		printRequest(requestName.c_str(), m_blockFunctions[type*2+1]);
	}
}


/**
 * Checks whether or not the current nesting contains a particular block type.
 *
 * @param type  Type to check for.
 *
 * @return      true if the nesting does contain the given block type
 */
bool CqOutput::nestingContains(EqBlocks type) const
{
	std::vector<EqBlocks>::const_iterator nesti = m_nesting.begin();
	for ( ; nesti != m_nesting.end(); ++nesti)
	{
		if (*nesti == type)
			return true;
	}
	return false;
}


// **************************************************************
// ******* ******* ******* PRINTING TOOLS ******* ******* *******
// **************************************************************
void CqOutput::printPL( RtInt n, RtToken tokens[], RtPointer parms[],
                        RtInt vertex, RtInt varying, RtInt uniform,
						RtInt facevarying, RtInt facevertex)
{
	for (TqInt i = 0; i < n ; i++ )
	{
		CqPrimvarToken tok;
		try
		{
			tok = m_Dictionary.parseAndLookup(tokens[i]);
		}
		catch(XqValidation& e)
		{
			// More ugly error reportage.
			CqError(RIE_BADTOKEN, RIE_ERROR, e.what(), true).manage();
			continue;
		}

		printToken( tokens[ i ] );
		S;

		TqInt storageCount = tok.storageCount(SqInterpClassCounts(uniform, varying,
					vertex, facevarying, facevertex), m_ColorNComps);
		switch ( tok.storageType() )
		{
			case type_float:
				printArray( storageCount, static_cast<RtFloat*>(parms[i]) );
				break;
			case type_string:
				{
					char** cp = static_cast<char **> ( parms[ i ] );
					print( "[" );
					S;
					for (TqInt j = 0; j < storageCount; j++ )
					{
						printCharP( cp[ j ] );
						S;
					}
					print( "]" );
				}
				break;
			case type_integer:
				printArray( storageCount, static_cast<RtInt*>(parms[i]) );
				break;
			default:
				{
					CqError(RIE_BADTOKEN, RIE_ERROR, "Unrecognised token ", tok.name(),
						"", true).manage();
				}
				break;
		}
		S;
	}
	EOL;
}

std::string CqOutput::getFilterFuncName( RtFilterFunc filterfunc, const char *name ) const
{
	if ( filterfunc == RiBoxFilter )
		return "box";
	else if ( filterfunc == RiMitchellFilter )
		return "mitchell";
	else if ( filterfunc == RiTriangleFilter )
		return "triangle";
	else if ( filterfunc == RiCatmullRomFilter )
		return "catmull-rom";
	else if ( filterfunc == RiSincFilter )
		return "sinc";
	else if ( filterfunc == RiGaussianFilter )
		return "gaussian";
	else if ( filterfunc == RiDiskFilter )
		return "disk";
	else if ( filterfunc == RiBesselFilter )
		return "bessel";
	else
	{
		throw CqError( RIE_CONSISTENCY, RIE_WARNING,
		               "Unknown RiFilterFunc. ",
		               name, " function skipped.", true );
		return "";
	}
}




// *****************************************************************
// ******* ******* ******* OUTPUT FUNCTIONS  ******* ******* *******
// *****************************************************************
RtToken CqOutput::RiDeclare( const char *name, const char *declaration )
{
	m_Dictionary.insert(CqPrimvarToken(declaration, name));

	PR( "Declare", Declare );
	S;
	printCharP( name );
	S;
	printCharP( declaration );
	EOL;

	return const_cast<RtToken> ( name );
}

// *****************************************************************
// ******* ******* *******   NESTED BLOCKS   ******* ******* *******
// *****************************************************************

RtVoid CqOutput::RiBegin( RtToken name )
{
	beginNesting(B_Ri);
	printHeader();
}

RtVoid CqOutput::RiEnd( )
{
	endNesting(B_Ri);
	out->flushFile();
}

RtVoid CqOutput::RiFrameBegin( RtInt frame )
{
	beginNesting(B_Frame);
	S;
	PI( frame );
	EOL;
	push();
}

RtVoid CqOutput::RiFrameEnd( )
{
	endNesting(B_Frame);
	EOL;
	pop();
}

RtVoid CqOutput::RiWorldBegin( )
{
	beginNesting(B_World);
	EOL;
	push();
}

RtVoid CqOutput::RiWorldEnd( )
{
	endNesting(B_World);
	EOL;
	pop();
}

RtObjectHandle CqOutput::RiObjectBegin( )
{
	beginNesting(B_Object);
	S;
	PI( ( RtInt ) m_ObjectHandle );
	EOL;

	push();
	return reinterpret_cast<RtObjectHandle>(static_cast<ptrdiff_t>(m_ObjectHandle++));
}

RtVoid CqOutput::RiObjectEnd( )
{
	endNesting(B_Object);
	EOL;
	pop();
}

RtVoid CqOutput::RiObjectInstance( RtObjectHandle handle )
{
	PR( "ObjectInstance", ObjectInstance );
	S;
	PI( static_cast<RtInt>(reinterpret_cast<ptrdiff_t>(handle)) );
	EOL;
}

RtVoid CqOutput::RiAttributeBegin( )
{
	beginNesting(B_Attribute);
	EOL;
	push();
}

RtVoid CqOutput::RiAttributeEnd( )
{
	endNesting(B_Attribute);
	EOL;
	pop();
}

RtVoid CqOutput::RiTransformBegin( )
{
	beginNesting(B_Transform);
	EOL;
}

RtVoid CqOutput::RiTransformEnd( )
{
	endNesting(B_Transform);
	EOL;
}

RtVoid CqOutput::RiSolidBegin( RtToken operation )
{
	beginNesting(B_Solid);
	S;
	printToken( operation );
	EOL;

	push();
}

RtVoid CqOutput::RiSolidEnd( )
{
	endNesting(B_Solid);
	EOL;
	pop();
}

RtVoid CqOutput::RiMotionBeginV( RtInt n, RtFloat times[] )
{
	beginNesting(B_Motion);
	S;
	printArray( n, times );
	EOL;
}

RtVoid CqOutput::RiMotionEnd( )
{
	endNesting(B_Motion);
	EOL;
}

RtVoid CqOutput::RiResourceBegin( )
{
	beginNesting(B_Resource);
	EOL;
	push();
}

RtVoid CqOutput::RiResourceEnd( )
{
	endNesting(B_Resource);
	EOL;
	pop();
}



// **************************************************************
// ******* ******* ******* CAMERA OPTIONS ******* ******* *******
// **************************************************************
RtVoid CqOutput::RiFormat ( RtInt xres, RtInt yres, RtFloat aspect )
{
	PR( "Format", Format );
	S;
	PI( xres );
	S;
	PI( yres );
	S;
	PF( aspect );
	EOL;
}

RtVoid CqOutput::RiFrameAspectRatio ( RtFloat aspect )
{
	PR( "FrameAspectRatio", FrameAspectRatio );
	S;
	PF( aspect );
	EOL;
}

RtVoid CqOutput::RiScreenWindow ( RtFloat left, RtFloat right, RtFloat bottom, RtFloat top )
{
	PR( "ScreenWindow", ScreenWindow );
	S;
	PF( left );
	S;
	PF( right );
	S;
	PF( bottom );
	S;
	PF( top );
	EOL;
}

RtVoid CqOutput::RiCropWindow ( RtFloat xmin, RtFloat xmax, RtFloat ymin, RtFloat ymax )
{
	PR( "CropWindow", CropWindow );
	S;
	PF( xmin );
	S;
	PF( xmax );
	S;
	PF( ymin );
	S;
	PF( ymax );
	EOL;
}

RtVoid CqOutput::RiProjectionV ( const char *name, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	PR( "Projection", Projection );
	S;
	if(name == NULL)
		printCharP("");
	else
		printCharP( name );
	S;
	printPL( n, tokens, parms );
}

RtVoid CqOutput::RiClipping( RtFloat hither, RtFloat yon )
{
	PR( "Clipping", Clipping );
	S;
	PF( hither );
	S;
	PF( yon );
	EOL;
}

RtVoid CqOutput::RiClippingPlane( RtFloat x, RtFloat y, RtFloat z,
                                  RtFloat nx, RtFloat ny, RtFloat nz )
{
	PR( "ClippingPlane", ClippingPlane );
	S;
	PF( x );
	S;
	PF( y );
	S;
	PF( z );
	S;
	PF( nx );
	S;
	PF( ny );
	S;
	PF( nz );
	EOL;
}

RtVoid CqOutput::RiDepthOfField ( RtFloat fstop, RtFloat focallength, RtFloat focaldistance )
{
	PR( "DepthOfField", DepthOfField );
	S;
	PF( fstop );
	S;
	PF( focallength );
	S;
	PF( focaldistance );
	EOL;
}

RtVoid CqOutput::RiShutter( RtFloat min, RtFloat max )
{
	PR( "Shutter", Shutter );
	S;
	PF( min );
	S;
	PF( max );
	EOL;
}




// ***************************************************************
// ******* ******* ******* DISPLAY OPTIONS ******* ******* *******
// ***************************************************************
RtVoid CqOutput::RiPixelVariance( RtFloat variation )
{
	PR( "PixelVariance", PixelVariance );
	S;
	PF( variation );
	EOL;
}

RtVoid CqOutput::RiPixelSamples( RtFloat xsamples, RtFloat ysamples )
{
	PR( "PixelSamples", PixelSamples );
	S;
	PF( xsamples );
	S;
	PF( ysamples );
	EOL;
}

RtVoid CqOutput::RiPixelFilter( RtFilterFunc filterfunc, RtFloat xwidth, RtFloat ywidth )
{
	std::string st = getFilterFuncName( filterfunc, "PixelFilter" );
	PR( "PixelFilter", PixelFilter );
	S;
	PS( st );
	S;
	PF( xwidth );
	S;
	PF( ywidth );
	EOL;
}

RtVoid CqOutput::RiExposure( RtFloat gain, RtFloat gamma )
{
	PR( "Exposure", Exposure );
	S;
	PF( gain );
	S;
	PF( gamma );
	EOL;
}

RtVoid CqOutput::RiImagerV( const char *name, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	PR( "Imager", Imager );
	S;
	printCharP( name );
	S;
	printPL( n, tokens, parms );
}

RtVoid CqOutput::RiQuantize( RtToken type, RtInt one, RtInt min, RtInt max, RtFloat ampl )
{
	PR( "Quantize", Quantize );
	S;
	printToken( type );
	S;
	PI( one );
	S;
	PI( min );
	S;
	PI( max );
	S;
	PF( ampl );
	EOL;
}

RtVoid CqOutput::RiDisplayV( const char *name, RtToken type, RtToken mode,
                             RtInt n, RtToken tokens[], RtPointer parms[] )
{
	PR( "Display", Display );
	S;
	printCharP( name );
	S;
	printToken( type );
	S;
	printToken( mode );
	S;
	printPL( n, tokens, parms );
}




// ******************************************************************
// ******* ******* ******* ADDITIONAL OPTIONS ******* ******* *******
// ******************************************************************
RtVoid CqOutput::RiHiderV( const char *type, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	PR( "Hider", Hider );
	S;
	printCharP( type );
	S;
	printPL( n, tokens, parms );
}

RtVoid CqOutput::RiColorSamples( RtInt n, RtFloat nRGB[], RtFloat RGBn[] )
{
	PR( "ColorSamples", ColorSamples );
	S;
	printArray( n * 3, nRGB );
	S;
	printArray( n * 3, RGBn );
	EOL;

	m_ColorNComps = n;
}

RtVoid CqOutput::RiRelativeDetail( RtFloat relativedetail )
{
	PR( "RelativeDetail", RelativeDetail );
	S;
	PF( relativedetail );
	EOL;
}

RtVoid CqOutput::RiOptionV( const char *name, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	PR( "Option", Option );
	S;
	printCharP( name );
	S;
	printPL( n, tokens, parms );
}




// ******************************************************************
// ******* ******* ******* SHADING ATTRIBUTES ******* ******* *******
// ******************************************************************
RtVoid CqOutput::RiColor( RtColor color )
{
	PR( "Color", Color );
	S;
	printArray( m_ColorNComps, color );
	EOL;
}

RtVoid CqOutput::RiOpacity( RtColor color )
{
	PR( "Opacity", Opacity );
	S;
	printArray( m_ColorNComps, color );
	EOL;
}

RtVoid CqOutput::RiTextureCoordinates( RtFloat s1, RtFloat t1, RtFloat s2, RtFloat t2,
                                       RtFloat s3, RtFloat t3, RtFloat s4, RtFloat t4 )
{
	PR( "TextureCoordinates", TextureCoordinates );
	S;
	PF( s1 );
	S;
	PF( t1 );
	S;
	PF( s2 );
	S;
	PF( t2 );
	S;
	PF( s3 );
	S;
	PF( t3 );
	S;
	PF( s4 );
	S;
	PF( t4 );
	EOL;
}

RtLightHandle CqOutput::RiLightSourceV( const char *name, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	PR( "LightSource", LightSource );
	S;
	printCharP( name );
	S;
	PI( ( RtInt ) m_LightHandle );
	S;
	printPL( n, tokens, parms );

	return reinterpret_cast<RtLightHandle>(static_cast<ptrdiff_t>(m_LightHandle++));
}

RtLightHandle CqOutput::RiAreaLightSourceV( const char *name,
        RtInt n, RtToken tokens[], RtPointer parms[] )
{
	PR( "AreaLightSource", AreaLightSource );
	S;
	printCharP( name );
	S;
	PI( ( RtInt ) m_LightHandle );
	S;
	printPL( n, tokens, parms );

	return reinterpret_cast<RtLightHandle>(static_cast<ptrdiff_t>(m_LightHandle++));
}

RtVoid CqOutput::RiIlluminate( RtLightHandle light, RtBoolean onoff )
{
	PR( "Illuminate", Illuminate );
	S;
	PI( static_cast<RtInt>(reinterpret_cast<ptrdiff_t>(light)) );
	S;

	if ( onoff == RI_TRUE )
		print( "1" );
	else
		print( "0" );
	EOL;
}

RtVoid CqOutput::RiSurfaceV( const char *name, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	PR( "Surface", Surface );
	S;
	printCharP( name );
	S;
	printPL( n, tokens, parms );
}

RtVoid CqOutput::RiAtmosphereV( const char *name, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	PR( "Atmosphere", Atmosphere );
	S;
	printCharP( name );
	S;
	printPL( n, tokens, parms );
}

RtVoid CqOutput::RiInteriorV( const char *name, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	PR( "Interior", Interior );
	S;
	printCharP( name );
	S;
	printPL( n, tokens, parms );
}

RtVoid CqOutput::RiExteriorV( const char *name, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	PR( "Exterior", Exterior );
	S;
	printCharP( name );
	S;
	printPL( n, tokens, parms );
}

RtVoid CqOutput::RiShadingRate( RtFloat size )
{
	PR( "ShadingRate", ShadingRate );
	S;
	PF( size );
	EOL;
}

RtVoid CqOutput::RiShadingInterpolation( RtToken type )
{
	PR( "ShadingInterpolation", ShadingInterpolation );
	S;
	printToken( type );
	EOL;
}

RtVoid CqOutput::RiMatte( RtBoolean onoff )
{
	PR( "Matte", Matte );
	S;

	if ( onoff == RI_TRUE )
		print( "1" );
	else
		print( "0" );
	EOL;
}




// *******************************************************************
// ******* ******* ******* GEOMETRY ATTRIBUTES ******* ******* *******
// *******************************************************************
RtVoid CqOutput::RiBound( RtBound b )
{
	PR( "Bound", Bound );
	S;
	printArray( 6, b );
	EOL;
}

RtVoid CqOutput::RiDetail( RtBound d )
{
	PR( "Detail", Detail );
	S;
	printArray( 6, d );
	EOL;
}

RtVoid CqOutput::RiDetailRange( RtFloat minvis, RtFloat lowtran, RtFloat uptran, RtFloat maxvis )
{
	PR( "DetailRange", DetailRange );
	S;
	PF( minvis );
	S;
	PF( lowtran );
	S;
	PF( uptran );
	S;
	PF( maxvis );
	EOL;
}

RtVoid CqOutput::RiGeometricApproximation( RtToken type, RtFloat value )
{
	PR( "GeometricApproximation", GeometricApproximation );
	S;
	printToken( type );
	S;
	PF( value );
	EOL;
}

namespace {

const char* getBasisName(RtBasis basis)
{
	if(basis == RiBezierBasis)
		return "bezier";
	if(basis == RiBSplineBasis)
		return "b-spline";
	if(basis == RiCatmullRomBasis)
		return "catmull-rom";
	if(basis == RiHermiteBasis)
		return "hermite";
	if(basis == RiPowerBasis)
		return "power";
	return 0;
}

} // unnamed namespace

RtVoid CqOutput::RiBasis( RtBasis ubasis, RtInt ustep, RtBasis vbasis, RtInt vstep )
{
	RtInt i;
	RtFloat m[ 16 ];

	PR( "Basis", Basis );
	S;
	if(const char* basisName = getBasisName(ubasis))
	{
		PS(basisName);
	}
	else
	{
		for ( i = 0; i < 16; i++ )
			m[ i ] = ubasis[ i / 4 ][ i % 4 ];
		printArray( 16, &( m[ 0 ] ) );
	}
	S;
	PI( ustep );
	S;

	if(const char* basisName = getBasisName(vbasis))
	{
		PS(basisName);
	}
	else
	{
		for ( i = 0; i < 16; i++ )
			m[ i ] = vbasis[ i / 4 ][ i % 4 ];
		printArray( 16, &( m[ 0 ] ) );
	}
	S;
	PI( vstep );
	EOL;

	m_Steps.top().uStep = ustep;
	m_Steps.top().vStep = vstep;
}

RtVoid CqOutput::RiTrimCurve( RtInt nloops, RtInt ncurves[], RtInt order[],
                              RtFloat knot[], RtFloat min[], RtFloat max[], RtInt n[],
                              RtFloat u[], RtFloat v[], RtFloat w[] )
{
	RtInt i;
	RtInt ttlc = 0;
	for ( i = 0; i < nloops; i++ )
		ttlc += ncurves[ i ];

	RtInt nbcoords = 0;
	RtInt knotsize = 0;
	for ( i = 0; i < ttlc; i++ )
	{
		nbcoords += n[ i ];
		knotsize += order[ i ] + n[ i ];
	}

	PR( "TrimCurve", TrimCurve );
	S;
	printArray( nloops, ncurves );
	S;
	printArray( ttlc, order );
	S;
	printArray( knotsize, knot );
	S;
	printArray( ttlc, min );
	S;
	printArray( ttlc, max );
	S;
	printArray( ttlc, n );
	S;
	printArray( nbcoords, u );
	S;
	printArray( nbcoords, v );
	S;
	printArray( nbcoords, w );
	EOL;
}

RtVoid CqOutput::RiOrientation( RtToken o )
{
	PR( "Orientation", Orientation );
	S;
	printToken( o );
	EOL;
}

RtVoid CqOutput::RiReverseOrientation( )
{
	PR( "ReverseOrientation", ReverseOrientation );
	EOL;
}

RtVoid CqOutput::RiSides( RtInt sides )
{
	PR( "Sides", Sides );
	S;
	PI( sides );
	EOL;
}

RtVoid CqOutput::RiDisplacementV( const char *name, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	PR( "Displacement", Displacement );
	S;
	printCharP( name );
	S;
	printPL( n, tokens, parms );
}




// ***************************************************************
// ******* ******* ******* TRANSFORMATIONS ******* ******* *******
// ***************************************************************
RtVoid CqOutput::RiIdentity( )
{
	PR( "Identity", Identity );
	EOL;
}

RtVoid CqOutput::RiTransform( RtMatrix transform )
{
	RtFloat m[ 16 ];
	for ( RtInt i = 0; i < 16; i++ )
	{
		m[ i ] = transform[ i / 4 ][ i % 4 ];
	}

	PR( "Transform", Transform );
	S;
	printArray( 16, &( m[ 0 ] ) );
	EOL;
}

RtVoid CqOutput::RiConcatTransform( RtMatrix transform )
{
	RtFloat m[ 16 ];
	for ( RtInt i = 0; i < 16; i++ )
	{
		m[ i ] = transform[ i / 4 ][ i % 4 ];
	}

	PR( "ConcatTransform", ConcatTransform );
	S;
	printArray( 16, &( m[ 0 ] ) );
	EOL;
}

RtVoid CqOutput::RiPerspective( RtFloat fov )
{
	PR( "Perspective", Perspective );
	S;
	PF( fov );
	EOL;
}

RtVoid CqOutput::RiTranslate( RtFloat dx, RtFloat dy, RtFloat dz )
{
	PR( "Translate", Translate );
	S;
	PF( dx );
	S;
	PF( dy );
	S;
	PF( dz );
	EOL;
}

RtVoid CqOutput::RiRotate( RtFloat angle, RtFloat dx, RtFloat dy, RtFloat dz )
{
	PR( "Rotate", Rotate );
	S;
	PF( angle );
	S;
	PF( dx );
	S;
	PF( dy );
	S;
	PF( dz );
	EOL;
}

RtVoid CqOutput::RiResourceV ( RtToken handle, RtToken type, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	PR( "Resource", Projection );
	S;
	printToken( handle );
	S;
	printToken( type );
	S;
	printPL( n, tokens, parms );
}

RtVoid CqOutput::RiScale( RtFloat sx, RtFloat sy, RtFloat sz )
{
	PR( "Scale", Scale );
	S;
	PF( sx );
	S;
	PF( sy );
	S;
	PF( sz );
	EOL;
}

RtVoid CqOutput::RiSkew( RtFloat angle, RtFloat dx1, RtFloat dy1, RtFloat dz1,
                         RtFloat dx2, RtFloat dy2, RtFloat dz2 )
{
	PR( "Skew", Skew );
	S;
	PF( angle );
	S;
	PF( dx1 );
	S;
	PF( dy1 );
	S;
	PF( dz1 );
	S;
	PF( dx2 );
	S;
	PF( dy2 );
	S;
	PF( dz2 );
	EOL;
}

RtVoid CqOutput::RiDeformationV( const char *name, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	PR( "Deformation", Deformation );
	S;
	printCharP( name );
	S;
	printPL( n, tokens, parms );
}

RtVoid CqOutput::RiCoordinateSystem( RtToken space )
{
	PR( "CoordinateSystem", CoordinateSystem );
	S;
	printToken( space );
	EOL;
}

RtVoid CqOutput::RiCoordSysTransform( RtToken space )
{
	PR( "CoordSysTransform", CoordSysTransform );
	S;
	printToken( space );
	EOL;
}

RtVoid CqOutput::RiAttributeV( const char *name, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	PR( "Attribute", Attribute );
	S;
	printCharP( name );
	S;
	printPL( n, tokens, parms );
}




// **********************************************************
// ******* ******* ******* PRIMITIVES ******* ******* *******
// **********************************************************
RtVoid CqOutput::RiPolygonV( RtInt nverts, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	PR( "Polygon", Polygon );
	S;
	printPL( n, tokens, parms, nverts, nverts, 1, nverts, nverts );
}

RtVoid CqOutput::RiGeneralPolygonV( RtInt nloops, RtInt nverts[],
                                    RtInt n, RtToken tokens[], RtPointer parms[] )
{
	RtInt nbpts = 0;
	for ( RtInt i = 0; i < nloops; i++ )
	{
		nbpts += nverts[ i ];
	}

	PR( "GeneralPolygon", GeneralPolygon );
	S;
	printArray( nloops, nverts );
	S;
	printPL( n, tokens, parms, nbpts, nbpts, 1, nbpts, nbpts );
}

RtVoid CqOutput::RiPointsPolygonsV( RtInt npolys, RtInt nverts[], RtInt verts[],
                                    RtInt n, RtToken tokens[], RtPointer parms[] )
{
	PR( "PointsPolygons", PointsPolygons );
	S;
	printArray( npolys, nverts );
	S;

	RtInt i;
	RtInt nbpts = 0;
	for ( i = 0; i < npolys; i++ )
	{
		nbpts += nverts[ i ];
	}
	printArray( nbpts, verts );
	S;

	RtInt psize = 0;
	for ( i = 0; i < nbpts; i++ )
	{
		if ( psize < verts[ i ] )
			psize = verts[ i ];
	}
	printPL( n, tokens, parms, psize + 1, psize + 1, npolys, nbpts, nbpts );
}

RtVoid CqOutput::RiPointsGeneralPolygonsV( RtInt npolys, RtInt nloops[], RtInt nverts[],
        RtInt verts[],
        RtInt n, RtToken tokens[], RtPointer parms[] )
{
	PR( "PointsGeneralPolygons", PointsGeneralPolygons );
	S;
	printArray( npolys, nloops );
	S;

	RtInt i;
	RtInt nbvert = 0;
	for ( i = 0; i < npolys; i++ )
	{
		nbvert += nloops[ i ];
	}
	printArray( nbvert, nverts );
	S;

	RtInt nv = 0;
	for ( i = 0; i < nbvert; i++ )
	{
		nv += nverts[ i ];
	}
	printArray( nv, verts );
	S;

	RtInt psize = 0;
	for ( i = 0; i < nv; i++ )
	{
		if ( psize < verts[ i ] )
			psize = verts[ i ];
	}
	printPL( n, tokens, parms, psize + 1, psize + 1, npolys, nv, nv );
}

RtVoid CqOutput::RiPatchV( RtToken type, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	RtInt nb;
	if ( type == RI_BILINEAR || (strcmp(type, RI_BILINEAR)==0) )
		nb = 4;
	else if ( type == RI_BICUBIC || (strcmp(type, RI_BICUBIC)==0) )
		nb = 16;
	else
	{
		throw CqError( RIE_BADTOKEN, RIE_ERROR,
		               "Unknown RiPatch type: ", type,
		               "  RiPatch() instruction skipped", true );
	}

	PR( "Patch", Patch );
	S;
	printToken( type );
	S;
	printPL( n, tokens, parms, nb, 4 );
}

RtVoid CqOutput::RiPatchMeshV( RtToken type, RtInt nu, RtToken uwrap,
                               RtInt nv, RtToken vwrap, RtInt n, RtToken tokens[],
                               RtPointer parms[] )
{
	RtInt nuptch, nvptch;
	RtInt varyingDataLen = 1;
	/// \todo Review: Consider removing redundent direct pointer comparisons to RI_BILINEAR.  Surely this isn't a worthwhile optimisation?
	if ( type == RI_BILINEAR || (strcmp(type, RI_BILINEAR)==0) )
	{
		/// \todo Review: Factor out checks for RI_PERIODIC into a small helper function.
		if ( uwrap == RI_PERIODIC || (strcmp(uwrap, RI_PERIODIC) ==0) )
		{
			nuptch = nu;
			varyingDataLen *= nuptch;
		}
		else if ( uwrap == RI_NONPERIODIC || (strcmp(uwrap, RI_NONPERIODIC)==0) )
		{
			nuptch = nu - 1;
			varyingDataLen *= nuptch + 1;
		}
		else
		{
			throw CqError( RIE_BADTOKEN, RIE_ERROR,
			               "Unknown RiPatchMesh uwrap token:", uwrap,
			               "  RiPatchMesh instruction skipped", true );
		}
		if ( vwrap == RI_PERIODIC || (strcmp(vwrap, RI_PERIODIC)==0) )
		{
			nvptch = nv;
			varyingDataLen *= nvptch;
		}
		else if ( vwrap == RI_NONPERIODIC || (strcmp(vwrap, RI_NONPERIODIC)==0) )
		{
			nvptch = nv - 1;
			varyingDataLen *= nvptch + 1;
		}
		else
		{
			throw CqError( RIE_BADTOKEN, RIE_ERROR,
			               "Unknown RiPatchMesh vwrap token:", vwrap,
			               "  RiPatchMesh instruction skipped", true );
		}
	}
	else if ( type == RI_BICUBIC || (strcmp(type, RI_BICUBIC)==0) )
	{
		RtInt nustep = m_Steps.top().uStep;
		RtInt nvstep = m_Steps.top().vStep;

		if ( uwrap == RI_PERIODIC || (strcmp(uwrap, RI_PERIODIC) ==0) )
		{
			nuptch = nu / nustep;
			varyingDataLen *= nuptch;
		}
		else if ( uwrap == RI_NONPERIODIC || (strcmp(uwrap, RI_NONPERIODIC)==0) )
		{
			nuptch = ( nu - 4 ) / nustep + 1;
			varyingDataLen *= nuptch + 1;
		}
		else
		{
			throw CqError( RIE_BADTOKEN, RIE_ERROR,
			               "Unknown RiPatchMesh uwrap token:", uwrap,
			               "  RiPatchMesh instruction skipped", true );
		}
		if ( vwrap == RI_PERIODIC || (strcmp(vwrap, RI_PERIODIC)==0) )
		{
			nvptch = nv / nvstep;
			varyingDataLen *= nvptch;
		}
		else if ( vwrap == RI_NONPERIODIC || (strcmp(vwrap, RI_NONPERIODIC)==0) )
		{
			nvptch = ( nv - 4 ) / nvstep + 1;
			varyingDataLen *= nvptch + 1;
		}
		else
		{
			throw CqError( RIE_BADTOKEN, RIE_ERROR,
			               "Unknown RiPatchMesh vwrap token:", vwrap,
			               "  RiPatchMesh instruction skipped", true );
		}
	}
	else
	{
		throw CqError( RIE_BADTOKEN, RIE_ERROR,
		               "Unknown RiPatchMesh type:", type,
		               "  RiPatchMesh instruction skipped", true );
	}

	PR( "PatchMesh", PatchMesh );
	S;
	printToken( type );
	S;
	PI( nu );
	S;
	printToken( uwrap );
	S;
	PI( nv );
	S;
	printToken( vwrap );
	S;
	printPL( n, tokens, parms, nu * nv, varyingDataLen, nuptch * nvptch );
}

RtVoid CqOutput::RiNuPatchV( RtInt nu, RtInt uorder, RtFloat uknot[],
                             RtFloat umin, RtFloat umax,
                             RtInt nv, RtInt vorder, RtFloat vknot[],
                             RtFloat vmin, RtFloat vmax,
                             RtInt n, RtToken tokens[], RtPointer parms[] )
{
	PR( "NuPatch", NuPatch );
	S;
	PI( nu );
	S;
	PI( uorder );
	S;
	printArray( nu + uorder, uknot );
	S;
	PF( umin );
	S;
	PF( umax );
	S;

	PI( nv );
	S;
	PI( vorder );
	S;
	printArray( nv + vorder, vknot );
	S;
	PF( vmin );
	S;
	PF( vmax );
	S;
	printPL( n, tokens, parms, nu * nv, ( 2 + nu - uorder ) * ( 2 + nv - vorder ), ( 1 + nu - uorder ) * ( 1 + nv - vorder ) );
}

RtVoid CqOutput::RiSphereV( RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat tmax,
                            RtInt n, RtToken tokens[], RtPointer parms[] )
{
	PR( "Sphere", Sphere );
	S;
	PF( radius );
	S;
	PF( zmin );
	S;
	PF( zmax );
	S;
	PF( tmax );
	S;
	printPL( n, tokens, parms, 4, 4 );
}

RtVoid CqOutput::RiConeV( RtFloat height, RtFloat radius, RtFloat tmax,
                          RtInt n, RtToken tokens[], RtPointer parms[] )
{
	PR( "Cone", Cone );
	S;
	PF( height );
	S;
	PF( radius );
	S;
	PF( tmax );
	S;
	printPL( n, tokens, parms, 4, 4 );
}

RtVoid CqOutput::RiCylinderV( RtFloat radius, RtFloat zmin, RtFloat zmax, RtFloat tmax,
                              RtInt n, RtToken tokens[], RtPointer parms[] )
{
	PR( "Cylinder", Cylinder );
	S;
	PF( radius );
	S;
	PF( zmin );
	S;
	PF( zmax );
	S;
	PF( tmax );
	S;
	printPL( n, tokens, parms, 4, 4 );
}

RtVoid CqOutput::RiHyperboloidV( RtPoint point1, RtPoint point2, RtFloat tmax,
                                 RtInt n, RtToken tokens[], RtPointer parms[] )
{
	PR( "Hyperboloid", Hyperboloid );
	S;
	PF( point1[0] );
	S;
	PF( point1[1] );
	S;
	PF( point1[2] );
	S;
	PF( point2[0] );
	S;
	PF( point2[1] );
	S;
	PF( point2[2] );
	S;
	PF( tmax );
	S;
	printPL( n, tokens, parms, 4, 4 );
}

RtVoid CqOutput::RiParaboloidV( RtFloat rmax, RtFloat zmin, RtFloat zmax, RtFloat tmax,
                                RtInt n, RtToken tokens[], RtPointer parms[] )
{
	PR( "Paraboloid", Paraboloid );
	S;
	PF( rmax );
	S;
	PF( zmin );
	S;
	PF( zmax );
	S;
	PF( tmax );
	S;
	printPL( n, tokens, parms, 4, 4 );
}

RtVoid CqOutput::RiDiskV( RtFloat height, RtFloat radius, RtFloat tmax,
                          RtInt n, RtToken tokens[], RtPointer parms[] )
{
	PR( "Disk", Disk );
	S;
	PF( height );
	S;
	PF( radius );
	S;
	PF( tmax );
	S;
	printPL( n, tokens, parms, 4, 4 );
}

RtVoid CqOutput::RiTorusV( RtFloat majrad, RtFloat minrad, RtFloat phimin, RtFloat phimax,
                           RtFloat tmax, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	PR( "Torus", Torus );
	S;
	PF( majrad );
	S;
	PF( minrad );
	S;
	PF( phimin );
	S;
	PF( phimax );
	S;
	PF( tmax );
	S;
	printPL( n, tokens, parms, 4, 4 );
}

RtVoid CqOutput::RiBlobbyV( RtInt nleaf, RtInt ncode, RtInt code[],
                            RtInt nflt, RtFloat flt[],
                            RtInt nstr, RtToken str[],
                            RtInt n, RtToken tokens[], RtPointer parms[] )
{
	PR( "Blobby", Blobby );
	std::ostringstream tmp;
	tmp << nleaf;
	std::string tmp2 (tmp.str());
	S;
	print(tmp2.c_str());
	S;
	printArray( ncode, code );
	S;
	printArray( nflt, flt );
	S;
	print( "[" );
	S;
	for ( RtInt i = 0; i < nstr; i++ )
	{
		printToken( str[ i ] );
		S;
	}
	print( "]" );
	S;
	printPL( n, tokens, parms, nleaf, nleaf );
}

RtVoid CqOutput::RiPointsV( RtInt npoints,
                            RtInt n, RtToken tokens[], RtPointer parms[] )
{
	PR( "Points", Points );
	S;
	printPL( n, tokens, parms, npoints, npoints );
}

RtVoid CqOutput::RiCurvesV( RtToken type, RtInt ncurves,
                            RtInt nvertices[], RtToken wrap,
                            RtInt n, RtToken tokens[], RtPointer parms[] )
{
	RtInt i;
	RtInt vval = 0;
	if ( type == RI_LINEAR || (strcmp(type, RI_LINEAR)==0) )
	{
		if ( wrap == RI_PERIODIC || (strcmp(wrap, RI_PERIODIC)==0) )
		{
			for ( i = 0; i < ncurves; i++ )
			{
				vval += nvertices[ i ];
			}
		}
		else if ( wrap == RI_NONPERIODIC || (strcmp(wrap, RI_NONPERIODIC)==0) )
		{
			for ( i = 0; i < ncurves; i++ )
			{
				vval += nvertices[ i ];
			}
		}
		else
		{
			throw CqError( RIE_BADTOKEN, RIE_ERROR,
			               "Unknown RiCurves wrap token:", wrap,
			               "  RiCurves instruction skipped", true );
		}
	}
	else if ( type == RI_CUBIC || (strcmp(type, RI_CUBIC)==0) )
	{
		if ( wrap == RI_PERIODIC || (strcmp(wrap, RI_PERIODIC)==0) )
		{
			for ( i = 0; i < ncurves; i++ )
			{
				vval += ( nvertices[ i ] - 4 ) / m_Steps.top().vStep;
			}
		}
		else if ( wrap == RI_NONPERIODIC || (strcmp(wrap, RI_NONPERIODIC)==0) )
		{
			for ( i = 0; i < ncurves; i++ )
			{
				vval += 2 + ( nvertices[ i ] - 4 ) / m_Steps.top().vStep;
			}
		}
		else
		{
			throw CqError( RIE_BADTOKEN, RIE_ERROR,
			               "Unknown RiCurves wrap token:", wrap,
			               "  RiCurves instruction skipped", true );
		}
	}
	else
	{
		throw CqError( RIE_BADTOKEN, RIE_ERROR,
		               "Unknown RiCurves type:", type,
		               "  RiCurves instruction skipped", true );
	}

	PR( "Curves", Curves );
	S;
	printToken( type );
	S;
	printArray( ncurves, nvertices );
	S;
	printToken( wrap );
	S;

	RtInt nbpts = 0;
	for ( i = 0; i < ncurves; i++ )
	{
		nbpts += nvertices[ i ];
	}
	printPL( n, tokens, parms, nbpts, vval, ncurves );
}

RtVoid CqOutput::RiSubdivisionMeshV( RtToken mask, RtInt nf, RtInt nverts[],
                                     RtInt verts[],
                                     RtInt ntags, RtToken tags[], RtInt numargs[],
                                     RtInt intargs[], RtFloat floatargs[],
                                     RtInt n, RtToken tokens[], RtPointer parms[] )
{
	PR( "SubdivisionMesh", SubdivisionMesh );
	S;
	printToken( mask );
	S;
	printArray( nf, nverts );
	S;

	RtInt i;
	RtInt vsize = 0;
	for ( i = 0; i < nf; i++ )
	{
		vsize += nverts[ i ];
	}
	printArray( vsize, verts );
	S;

	printArray( ntags, tags );
	S;
	printArray( ntags * 2, numargs );
	S;

	RtInt isize = 0, fsize = 0;
	for ( i = 0; i < ntags*2; i++ )
	{
		if ( i % 2 == 0 )
			isize += numargs[ i ];
		else
			fsize += numargs[ i ];
	}
	printArray( isize, intargs );
	S;
	printArray( fsize, floatargs );
	S;

	RtInt psize = 0;
	for ( i = 0; i < vsize; i++ )
	{
		if ( psize < verts[ i ] )
			psize = verts[ i ];
	}
	printPL( n, tokens, parms, psize + 1, psize + 1, nf, vsize, vsize );
}

RtVoid CqOutput::RiProcedural( RtPointer data, RtBound bound,
                               RtVoid ( *subdivfunc ) ( RtPointer, RtFloat ),
                               RtVoid ( *freefunc ) ( RtPointer ) )
{
	std::string sf;
	RtInt a;
	if ( subdivfunc == RiProcDelayedReadArchive )
	{
		sf = "DelayedReadArchive";
		a = 1;
	}
	else if ( subdivfunc == RiProcRunProgram )
	{
		sf = "RunProgram";
		a = 2;
	}
	else if ( subdivfunc == RiProcDynamicLoad )
	{
		sf = "DynamicLoad";
		a = 3;
	}
	else
	{
		throw CqError( RIE_SYNTAX, RIE_ERROR, "Unknown procedural function.", true );
	}

	PR( "Procedural", Procedural );
	S;

	RtInt i;
	switch ( a )
	{
			case 1:
			PS( sf );
			S;
			print( "[" );
			S;
			printCharP( ( ( RtString * ) data ) [ 0 ] );
			S;
			print( "]" );
			S;

			print( "[" );
			S;
			for ( i = 0; i < 6 ; i++ )
			{
				PF( bound[ i ] );
				S;
			}
			print( "]" );
			EOL;
			break;
			case 2:
			case 3:
			PS( sf );
			S;
			print( "[" );
			S;
			printCharP( ( ( RtString * ) data ) [ 0 ] );
			S;
			printCharP( ( ( RtString * ) data ) [ 1 ] );
			S;
			print( "]" );
			S;

			print( "[" );
			S;
			for ( i = 0; i < 6 ; i++ )
			{
				PF( bound[ i ] );
				S;
			}
			print( "]" );
			EOL;
			break;
	}
}

RtVoid CqOutput::RiGeometryV( RtToken type, RtInt n, RtToken tokens[], RtPointer parms[] )
{
	PR( "Geometry", Geometry );
	S;
	printToken( type );
	S;
	printPL( n, tokens, parms );
}




// *******************************************************
// ******* ******* ******* TEXTURE ******* ******* *******
// *******************************************************
RtVoid CqOutput::RiMakeTextureV( const char *pic, const char *tex, RtToken swrap, RtToken twrap,
                                 RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth,
                                 RtInt n, RtToken tokens[], RtPointer parms[] )
{
	std::string ff = getFilterFuncName( filterfunc, "MakeTexture" );

	PR( "MakeTexture", MakeTexture );
	S;
	printCharP( pic );
	S;
	printCharP( tex );
	S;
	printToken( swrap );
	S;
	printToken( twrap );
	S;
	PS( ff );
	S;
	PF( swidth );
	S;
	PF( twidth );
	S;
	printPL( n, tokens, parms );
}

RtVoid CqOutput::RiMakeBumpV( const char *pic, const char *tex, RtToken swrap, RtToken twrap,
                              RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth,
                              RtInt n, RtToken tokens[], RtPointer parms[] )
{
	std::string ff = getFilterFuncName( filterfunc, "MakeBump" );

	PR( "MakeBump", MakeBump );
	S;
	printCharP( pic );
	S;
	printCharP( tex );
	S;
	printToken( swrap );
	S;
	printToken( twrap );
	S;
	PS( ff );
	S;
	PF( swidth );
	S;
	PF( twidth );
	S;
	printPL( n, tokens, parms );
}

RtVoid CqOutput::RiMakeLatLongEnvironmentV( const char *pic, const char *tex, RtFilterFunc filterfunc,
        RtFloat swidth, RtFloat twidth,
        RtInt n, RtToken tokens[], RtPointer parms[] )
{
	std::string ff = getFilterFuncName( filterfunc, "MakeLatLongEnvironment" );

	PR( "MakeLatLongEnvironment", MakeLatLongEnvironment );
	S;
	printCharP( pic );
	S;
	printCharP( tex );
	S;
	PS( ff );
	S;
	PF( swidth );
	S;
	PF( twidth );
	S;
	printPL( n, tokens, parms );
}

RtVoid CqOutput::RiMakeCubeFaceEnvironmentV( const char *px, const char *nx, const char *py, const char *ny,
        const char *pz, const char *nz, const char *tex, RtFloat fov,
        RtFilterFunc filterfunc, RtFloat swidth,
        RtFloat twidth,
        RtInt n, RtToken tokens[], RtPointer parms[] )
{
	std::string ff = getFilterFuncName( filterfunc, "MakeCubeFaceEnvironment" );

	PR( "MakeCubeFaceEnvironment", MakeCubeFaceEnvironment );
	S;
	printCharP( px );
	S;
	printCharP( nx );
	S;
	printCharP( py );
	S;
	printCharP( ny );
	S;
	printCharP( pz );
	S;
	printCharP( nz );
	S;
	printCharP( tex );
	S;
	PF( fov );
	S;
	PS( ff );
	S;
	PF( swidth );
	S;
	PF( twidth );
	S;
	printPL( n, tokens, parms );
}

RtVoid CqOutput::RiMakeShadowV( const char *pic, const char *tex,
                                RtInt n, RtToken tokens[], RtPointer parms[] )
{
	PR( "MakeShadow", MakeShadow );
	S;
	printCharP( pic );
	S;
	printCharP( tex );
	S;
	printPL( n, tokens, parms );
}


RtVoid	CqOutput::RiMakeOcclusionV( RtInt npics, RtString picfiles[], RtString shadowfile, RtInt count, RtToken tokens[], RtPointer values[] )
{
	PR( "MakeOcclusion", MakeOcclusion );
	S;
	printArray(npics, picfiles); 
	S;
	printCharP( shadowfile );
	S;
	printPL(count, tokens, values);
}


// *******************************************************
// ******* ******* ******* ARCHIVE ******* ******* *******
// *******************************************************
RtVoid CqOutput::RiArchiveRecord( RtToken type, std::string txt )
{
	std::string tmp;

	if ( type == RI_COMMENT || (strcmp(type, RI_COMMENT)==0) )
		tmp = "#";
	else if ( type == RI_STRUCTURE || (strcmp(type, RI_STRUCTURE)==0) )
		tmp = "##";
	else if ( type == RI_VERBATIM || (strcmp(type, RI_VERBATIM)==0) )
	{
		print( txt.c_str() );
		return ;
	}
	else
	{
		throw CqError( RIE_BADTOKEN, RIE_ERROR,
		               "Unknown ArchiveRecord type: ", type, "", true );
	}
	// EOL must be forced for binary
	printIndentation();
	print( ( tmp + txt + "\n" ).c_str() );
}

RtVoid CqOutput::RiReadArchiveV( RtToken name, RtArchiveCallback callback,
                                 RtInt n, RtToken tokens[], RtPointer parms[] )
{
	PR( "ReadArchive", ReadArchive );
	S;
	printToken( name );
	EOL;
}


RtVoid CqOutput::RiIfBegin( RtString condition )
{
	PR( "IfBegin", IfBegin );
	S;
	printToken( condition );
	EOL;
}
RtVoid CqOutput::RiElseIf( RtString condition )
{
	PR( "ElseIf", ElseIf );
	S;
	printToken( condition );
	EOL;
}
RtVoid CqOutput::RiIfEnd( )
{
	PR( "IfEnd", IfEnd );
	S;
	EOL;
}
RtVoid CqOutput::RiElse( )
{
	PR( "Else", Else );
	S;
	EOL;
}

// *************************************************************
// ******* ******* ******* ERROR HANDLER ******* ******* *******
// *************************************************************
RtVoid CqOutput::RiErrorHandler( RtErrorFunc handler )
{
	std::string ch;
	if ( handler == RiErrorIgnore )
		ch = "ignore";
	else if ( handler == RiErrorPrint )
		ch = "print";
	else if ( handler == RiErrorAbort )
		ch = "abort";
	else
	{
		throw CqError( RIE_CONSISTENCY, RIE_ERROR, "Unknown Error handler", true );
	}
	PR( "ErrorHandler", ErrorHandler );
	S;
	PS( ch );
	EOL;
}


// *************************************************************
// ******* ******* ****** LAYERED SHADERS ****** ******* *******
// *************************************************************
RtVoid CqOutput::RiShaderLayerV( RtToken type, RtToken name, RtToken layername, RtInt count, RtToken tokens[], RtPointer values[] )
{
	PR( "ShaderLayer", ShaderLayer );
	S;
	printToken( type );
	S;
	printToken( name );
	S;
	printToken( layername );
	S;
	printPL( count, tokens, values );
}


RtVoid CqOutput::RiConnectShaderLayers( RtToken type, RtToken layer1, RtToken variable1, RtToken layer2, RtToken variable2 )
{
	PR( "ConnectShaderLayers", ConnectShaderLayers);
	S;
	printToken( type );
	S;
	printToken( layer1 );
	S;
	printToken( variable1 );
	S;
	printToken( layer2 );
	S;
	printToken( variable2 );
}

} // namespace libri2rib
