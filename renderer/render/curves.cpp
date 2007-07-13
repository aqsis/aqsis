// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

/**
        \file
        \brief Implements the classes and support structures for 
                handling RenderMan Curves primitives.
        \author Jonathan Merritt (j.merritt@pgrad.unimelb.edu.au)
*/

#include "aqsis.h"
#include "vector3d.h"
#include "curves.h"
START_NAMESPACE( Aqsis )


static const TqUlong hwidth = CqString::hash("width");
static const TqUlong hcwidth = CqString::hash("constantwidth");

/**
 * CqCurve constructor.
 */
CqCurve::CqCurve() : CqSurface()
{
	m_widthParamIndex = -1;
	m_constantwidthParamIndex = -1;
	m_splitDecision = Split_Undecided;

	STATS_INC( GPR_crv );
}



/**
 * CqCurve copy constructor.
 */
/* CqCurve::CqCurve( const CqCurve &from ) : CqSurface()
 * {
 * 	( *this ) = from;
 * 
 * 	STATS_INC( GPR_crv );
 * }
 */



/**
 * CqCurve destructor.
 */
CqCurve::~CqCurve()
{ }


/**
 * Adds a primitive variable to the list of user parameters.  This method
 * caches the indexes of the "width" and "constantwidth" parameters within
 * the array of user parameters for later access.
 *
 * @param pParam        Pointer to the parameter to add.
 */
void CqCurve::AddPrimitiveVariable( CqParameter* pParam )
{

	// add the primitive variable using the superclass method
	CqSurface::AddPrimitiveVariable( pParam );

	// trap the indexes of "width" and "constantwidth" parameters
	if ( pParam->hash() == hwidth )
	{
		assert( m_widthParamIndex == -1 );
		m_widthParamIndex = m_aUserParams.size() - 1;
	}
	else if ( pParam->hash() == hcwidth )
	{
		assert( m_constantwidthParamIndex == -1 );
		m_constantwidthParamIndex = m_aUserParams.size() - 1;
	}

}



/**
 * Calculates bounds for a CqCurve.
 *
 * NOTE: This method makes the same assumptions as 
 * CqSurfacePatchBicubic::Bound() does about the convex-hull property of the
 * curve.  This is fine most of the time, but the user can specify basis
 * matrices like Catmull-Rom, which are non-convex.
 *
 * FIXME: Make sure that all hulls which reach this method are convex!
 *
 * @return CqBound object containing the bounds.
 */
CqBound CqCurve::Bound() const
{

	// Get the boundary in camera space.
	CqVector3D vecA( FLT_MAX, FLT_MAX, FLT_MAX );
	CqVector3D vecB( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	TqFloat maxCameraSpaceWidth = 0;
	TqUint nWidthParams = cVarying();
	for ( TqUint i = 0; i < ( *P() ).Size(); i++ )
	{
		// expand the boundary if necessary to accomodate the
		//  current vertex
		CqVector3D vecV = P()->pValue( i )[0];
		if ( vecV.x() < vecA.x() )
			vecA.x( vecV.x() );
		if ( vecV.y() < vecA.y() )
			vecA.y( vecV.y() );
		if ( vecV.x() > vecB.x() )
			vecB.x( vecV.x() );
		if ( vecV.y() > vecB.y() )
			vecB.y( vecV.y() );
		if ( vecV.z() < vecA.z() )
			vecA.z( vecV.z() );
		if ( vecV.z() > vecB.z() )
			vecB.z( vecV.z() );

		// increase the maximum camera space width of the curve if
		//  necessary
		if ( i < nWidthParams )
		{
			TqFloat camSpaceWidth = width()->pValue( i )[0];
			if ( camSpaceWidth > maxCameraSpaceWidth )
			{
				maxCameraSpaceWidth = camSpaceWidth;
			}
		}

	}

	// increase the size of the boundary by half the width of the
	//  curve in camera space
	vecA -= ( maxCameraSpaceWidth / 2.0 );
	vecB += ( maxCameraSpaceWidth / 2.0 );

	// return the boundary
	CqBound	B;
	B.vecMin() = vecA;
	B.vecMax() = vecB;
	return ( AdjustBoundForTransformationMotion( B ) );
}



/**
 * CqCurve CloneData function
 *
 */
void CqCurve::CloneData(CqCurve* clone) const
{
	CqSurface::CloneData(clone);
	clone->m_widthParamIndex = m_widthParamIndex;
	clone->m_constantwidthParamIndex = m_constantwidthParamIndex;
}



/**
 * Returns the approximate "length" of an edge of a grid in raster space.
 *
 * @return Approximate grid length.
 */
TqFloat CqCurve::GetGridLength() const
{

	// we want to find the number of micropolygons per grid - the default
	//  is 256 (16x16 micropolygon grid).
	TqFloat micropolysPerGrid = 256;
	const TqInt* poptGridSize =
	    QGetRenderContext() ->poptCurrent()->GetIntegerOption(
	        "limits", "gridsize"
	    );
	if ( poptGridSize != NULL )
	{
		micropolysPerGrid =
		    static_cast<TqFloat>( poptGridSize[ 0 ] ) *
		    static_cast<TqFloat>( poptGridSize[ 1 ] );
	}

	// find the shading rate
	TqFloat ShadingRate = pAttributes() ->GetFloatAttribute(
	                          "System", "ShadingRate"
	                      ) [ 0 ];

	// we assume that the grids are square and take the square root to find
	//  the number of micropolygons along one side
	TqFloat mpgsAlongSide = sqrt( micropolysPerGrid );

	// now, the number of pixels (raster space length) taken up by one
	//  micropolygon is given by 1 / shading rate.  So, to find the length
	//  in raster space of the edge of the micropolygon grid, we divide its
	//  length (in micropolygons) by the shading rate
	return mpgsAlongSide / ShadingRate;

}



/**
 * Populates the "width" parameter if it is not already present (ie supplied
 * by the user).  The "width" is populated either by the value of
 * "constantwidth", or by the default object-space width 1.0.
 */
void CqCurve::PopulateWidth()
{

	// if the width parameter has been supplied by the user then bail
	//  immediately
	if ( width() != NULL )
		return ;

	// otherwise, find the value to fill the width array with; default
	//  value is 1.0 which can be overridden by the "constantwidth"
	//  parameter
	TqFloat widthvalue = 1.0;
	if ( constantwidth() != NULL )
	{
		widthvalue = *( constantwidth() ->pValue() );
	}

	// create and fill in the width array
	CqParameterTypedVarying<TqFloat, type_float, TqFloat>* widthP =
	    new CqParameterTypedVarying<TqFloat, type_float, TqFloat>(
	        "width"
	    );
	widthP->SetSize( cVarying() );
	for ( TqUint i = 0; i < cVarying(); i++ )
	{
		widthP->pValue( i )[0] = widthvalue;
	}

	// add the width array to the curve as a primitive variable
	AddPrimitiveVariable( widthP );
}


bool CqCurve::GetNormal( TqInt index, CqVector3D& normal ) const
{
	if ( N() != NULL )
	{
		normal = N()->pValue( index )[0];
		return true;
	}
	else
	{
		bool CSO = pTransform()->GetHandedness(pTransform()->Time(0));
		bool O = pAttributes() ->GetIntegerAttribute( "System", "Orientation" ) [ 0 ] != 0;
		if ( (O && CSO) || (!O && !CSO) )
			normal = CqVector3D(0, 0, -1);
		else
			normal = CqVector3D(0, 0, 1);
		return false;
	}
}

/**
 * Sets the default primitive variables.
 *
 * @param bUseDef_st
 */
void CqCurve::SetDefaultPrimitiveVariables( bool bUseDef_st )
{
	// we don't want any default primitive variables.

	// s,t are set to u,v for curves
}


/**
 * CqCurvesGroup constructor.
 */
CqCurvesGroup::CqCurvesGroup() : CqCurve(), m_ncurves( 0 ), m_periodic( false ),
		m_nTotalVerts( 0 )
{ }



/**
 * CqCurvesGroup copy constructor.
 */
/* CqCurvesGroup::CqCurvesGroup( const CqCurvesGroup& from ) : CqCurve()
 * {
 * 	( *this ) = from;
 * }
 */



/**
 * CqCurvesGroup destructor.
 */
CqCurvesGroup::~CqCurvesGroup()
{ }



/**
 * Clone the data from this curve group onto the one specified.
 *
 */
void CqCurvesGroup::CloneData( CqCurvesGroup* clone ) const
{
	CqCurve::CloneData(clone);

	// copy members
	clone->m_ncurves = m_ncurves;
	clone->m_periodic = m_periodic;
	clone->m_nvertices = m_nvertices;
	clone->m_nTotalVerts = m_nTotalVerts;
}


END_NAMESPACE( Aqsis )
