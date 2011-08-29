// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
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

#include <aqsis/aqsis.h>
#include <aqsis/math/vector3d.h>
#include "curves.h"
namespace Aqsis {


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
void CqCurve::Bound(CqBound* bound) const
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
		CqVector3D vecV = vectorCast<CqVector3D>(P()->pValue( i )[0]);
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

	bound->vecMin() = vecA;
	bound->vecMax() = vecB;
	AdjustBoundForTransformationMotion( bound );
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
		micropolysPerGrid = poptGridSize[0];

	// Assuming the grid is square, the side length of the grid in raster space
	// is the following.
	// TODO: this assumption may be pretty bad for RiCurves!
	return sqrt(micropolysPerGrid * AdjustedShadingRate());
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
	for (TqInt i = 0, numVarying = cVarying(); i < numVarying; i++ )
	{
		widthP->pValue( i )[0] = widthvalue;
	}

	// add the width array to the curve as a primitive variable
	AddPrimitiveVariable( widthP );
}


bool CqCurve::Diceable(const CqMatrix& matCtoR)
{
	// OK, here the CqCubicCurveSegment line has two options:
	//  1. split into two more lines
	//  2. turn into a bilinear patch for rendering
	// We don't want to go turning into a patch unless absolutely
	// necessary, since patches cost more.  We only want to become a patch
	// if the current curve is "best handled" as a patch.  For now, I'm
	// choosing to define that the curve is best handled as a patch under
	// one or more of the following two conditions:
	//  1. If the maximum width is a significant fraction of the length of
	//      the line (width greater than 0.75 x length; ignoring normals).
	//  2. If the length of the line (ignoring the width; cos' it's
	//      covered by point 1) is such that it's likely a bilinear
	//      patch would be diced immediately if we created one (so that
	//      patches don't have to get split!).
	//  3. If the curve crosses the eye plane (m_fDiceable == false).

	// find the length of the CqLinearCurveSegment line in raster space
	if( m_splitDecision == Split_Undecided )
	{
		// AGG - 31/07/04
		// well, if we follow the above statagy we end up splitting into
		// far too many grids (with roughly 1 mpg per grid). so after
		// profiling a few scenes, the fastest method seems to be just
		// to convert to a patch immediatly.
		// we really need a native dice for curves but until that time
		// i reckon this is best.
		//m_splitDecision = Split_Patch;

		// control hull
		CqVector3D hull[2] = {
			vectorCast<CqVector3D>(matCtoR * P()->pValue(0)[0]),
			vectorCast<CqVector3D>(matCtoR * P()->pValue(1)[0])
		};
		CqVector3D lengthVector = hull[ 1 ] - hull[ 0 ];
		TqFloat lengthraster = lengthVector.Magnitude();

		// find the approximate "length" of a diced patch in raster space
		TqFloat gridlength = GetGridLength();

		// decide whether to split into more curve segments or a patch
		if(( lengthraster < gridlength ) || ( !m_fDiceable ))
		{
			// split into a patch
			m_splitDecision = Split_Patch;
		}
		else
		{
			// split into smaller curves
			m_splitDecision = Split_Curve;
		}
	}

	return false;
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

/** Transform a curve group using the specified matrices.
 *
 * All parameters are transformed by the base class transformation function,
 * except for the curve widths, which needs special consideration.  Widths
 * are scaled by the average of the amount which the curve is squashed
 * perpendicular to the viewing direction.  This isn't the most accurate
 * algorithm, but it's relatively easy to implement (see notes below).
 *
 * The algorithm used assumes that the transformation is not a projective
 * transformation.  Any affine transformation should be fine.
 *
 * \param matTx         Reference to the transformation matrix.
 * \param matITTx       Reference to the inverse transpose of the 
 *                        transformation matrix, used to transform normals.
 * \param matRTx        Reference to the rotation only transformation matrix, 
 *                        used to transform vectors.
 * \param iTime			The frame time at which to apply the transformation.
 */
void CqCurvesGroup::Transform(
    const CqMatrix& matTx,
    const CqMatrix& matITTx,
    const CqMatrix& matRTx,
    TqInt iTime
)
{
	// make sure the "width" parameter is present
	PopulateWidth();

	/// \todo The algorithm for transforming curve widths is dubious!
	//
	// The previous algorithm used a very complicated method which boiled down
	// to scaling the width parameter of curves by the amount
	//
	//   1/(matITTx*CqVector3D(1,0,0)).Magnitude()
	//
	// If the transformation is an anisotropic scaling transformation (eg,
	// squashing more in the x-direction than the y-direction), this is rather
	// inadequte: it results in different widths depending on the direction
	// of the scaling.
	//
	// A slightly better algorithm is to use the average of the scaling amount
	// in the x and y directions as done below.  (Chosen since x and y should
	// be the directions perpendicular to the viewing direction.)
	//
	// However, this is far from ideal; ideally the curve would look like it
	// has been squashed preferentially in one direction if that is the case.
	// We could achieve this by calculating the width vector which is
	// proportional to the tangent crossed with the normal, as is currently
	// done by CqCubicCurveSegment::SplitToPatch.
	//
	// The width vector would then be transformed as a normal, and the
	// resulting length used as the scaling factor, independently for each
	// width on the curve.  This would be more correct, but is much more
	// complicated to implement correctly!
	TqFloat widthScale = 2/((matITTx*CqVector3D(1,0,0)).Magnitude()
			+ (matITTx*CqVector3D(0,1,0)).Magnitude());
	CqParameterTypedVarying<TqFloat, type_float, TqFloat>* w = width();
	for(TqInt i = 0, end = w->Size(); i < end; ++i)
		w->pValue(i)[0] *= widthScale;

	// For everything else (apart from the width), just call through to the
	// underlying transformation function.
	CqCurve::Transform( matTx, matITTx, matRTx, iTime );
}


} // namespace Aqsis
