// Aqsis
// Copyright c 1997 - 2001, Paul C. Gregory
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



/** \file
		\brief Implements CqPoints using small regular polygon (first try) This is more or less an experimentation with the parser. Later a micropolygon grid will be used to be more efficient to shade/render.
		\author M. Joron (joron@sympatico.ca)
*/
/*    References:
 *          [PIXA89]  Pixar, The RenderMan Interface, Version 3.2, 
 *                    Richmond, CA, September 1989.  
 *
 */

#include	<cmath>
#include	<cfloat>

#include	"points.h"
#include	"imagebuffer.h"
#include	"renderer.h"
#include	"bucketprocessor.h"

namespace Aqsis {

CqObjectPool<CqMovingMicroPolygonKeyPoints>	CqMovingMicroPolygonKeyPoints::m_thePool;
CqObjectPool<CqMicroPolygonPoints>	CqMicroPolygonPoints::m_thePool;
CqObjectPool<CqMicroPolygonMotionPoints>	CqMicroPolygonMotionPoints::m_thePool;

class CqPointsKDTreeData::CqPointsKDTreeDataComparator
{
	public:
		CqPointsKDTreeDataComparator(const CqPoints* pPoints, TqInt dimension)
			: m_P(pPoints->pPoints()->P()->pValue()),
			m_Dim( dimension )
		{ }

		bool operator()(TqInt a, TqInt b)
		{
			return m_P[a][m_Dim] < m_P[b][m_Dim];
		}

	private:
		const CqVector4D* m_P;
		TqInt		m_Dim;
};

void CqPointsKDTreeData::PartitionElements(std::vector<TqInt>& leavesIn,
		TqInt dimension, std::vector<TqInt>& out1, std::vector<TqInt>& out2)
{
	std::vector<TqInt>::iterator medianPos = leavesIn.begin() + leavesIn.size()/2;
	// Partition the values in leavesIn about the median position along the
	// axis specified by dimension.  The nth_element algorithm runs in linear
	// time, so is asymptotically better than doing this using a sort.
	std::nth_element(leavesIn.begin(), medianPos, leavesIn.end(),
					 CqPointsKDTreeDataComparator(m_pPointsSurface, dimension));
	// All the points less than the point at *medianPos are in out1.  The other
	// points go into out2.
	out1.assign(leavesIn.begin(), medianPos);
	out2.assign(medianPos, leavesIn.end());
}

void CqPointsKDTreeData::SetpPoints( const CqPoints* pPoints )
{
	m_pPointsSurface = pPoints;
}


//---------------------------------------------------------------------
/** Constructor.
 */

CqPoints::CqPoints( TqInt nvertices, const boost::shared_ptr<CqPolygonPoints>& pPoints ) : 
		m_pPoints(pPoints),
		m_nVertices(nvertices),
		m_KDTree(&m_KDTreeData),
		m_MaxWidth(0)
{
	//assert( NULL != pPoints );
	assert( nvertices > 0 );

	m_KDTreeData.SetpPoints(this);

	m_widthParamIndex = -1;
	m_constantwidthParamIndex = -1;

	std::vector<CqParameter*>::iterator iUP;
	TqInt index = 0;
	for( iUP = pPoints->aUserParams().begin(); iUP != pPoints->aUserParams().end(); iUP++, index++ )
		if( (*iUP)->strName() == "constantwidth" && (*iUP)->Type() == type_float && (*iUP)->Class() == class_constant )
			m_constantwidthParamIndex = index;
		else if( (*iUP)->strName() == "width" && (*iUP)->Type() == type_float && (*iUP)->Class() == class_varying )
			m_widthParamIndex = index;

	STATS_INC( GPR_points );
}


//---------------------------------------------------------------------
/** Create a clone of this points class
 */

CqSurface* CqPoints::Clone() const 
{
	// Make a 'complete' clone of this primitive, which means cloning the points too.
	CqPolygonPoints* clone_points = static_cast<CqPolygonPoints*>(m_pPoints->Clone());

	CqPoints* clone = new CqPoints(m_nVertices, boost::shared_ptr<CqPolygonPoints>(clone_points));
	CqSurface::CloneData( clone );

//	clone->m_nVertices = m_nVertices;
//	clone->m_pPoints = boost::shared_ptr<CqPolygonPoints>(clone_points);

//	clone->m_KDTreeData.SetpPoints( clone );

//	clone->m_widthParamIndex = m_widthParamIndex;
//	clone->m_constantwidthParamIndex = m_constantwidthParamIndex;
//	clone->m_MaxWidth = m_MaxWidth;

	clone->InitialiseKDTree();
	clone->InitialiseMaxWidth();

	return ( clone );
}




//---------------------------------------------------------------------
/** Dice the quadric into a grid of MPGs for rendering.
 */

CqMicroPolyGridBase* CqPoints::Dice()
{
	assert( pPoints() );

	CqMicroPolyGridPoints* pGrid = new CqMicroPolyGridPoints();
	pGrid->Initialise(nVertices(), 1, shared_from_this());

	TqInt lUses = Uses();

	// Dice the primitive variables.
	if ( USES( lUses, EnvVars_Cs ) && ( pGrid->pVar(EnvVars_Cs) ) )
	{
		if ( pPoints()->bHasVar(EnvVars_Cs) )
			NaturalDice( pPoints()->Cs(), nVertices(), 1, pGrid->pVar(EnvVars_Cs) );
		else if ( NULL != pAttributes() ->GetColorAttribute( "System", "Color" ) )
			pGrid->pVar(EnvVars_Cs) ->SetColor( pAttributes() ->GetColorAttribute( "System", "Color" ) [ 0 ] );
		else
			pGrid->pVar(EnvVars_Cs) ->SetColor( CqColor( 1, 1, 1 ) );
	}

	if ( USES( lUses, EnvVars_Os ) && ( NULL != pGrid->pVar(EnvVars_Os) ) )
	{
		if ( pPoints()->bHasVar(EnvVars_Os) )
			NaturalDice( pPoints()->Os(), nVertices(), 1, pGrid->pVar(EnvVars_Os) );
		else if ( NULL != pAttributes() ->GetColorAttribute( "System", "Opacity" ) )
			pGrid->pVar(EnvVars_Os) ->SetColor( pAttributes() ->GetColorAttribute( "System", "Opacity" ) [ 0 ] );
		else
			pGrid->pVar(EnvVars_Os) ->SetColor( CqColor( 1, 1, 1 ) );
	}

	if ( USES( lUses, EnvVars_s ) && ( NULL != pGrid->pVar(EnvVars_s) ) && pPoints()->bHasVar(EnvVars_s) )
		NaturalDice( pPoints()->s(), nVertices(), 1, pGrid->pVar(EnvVars_s) );

	if ( USES( lUses, EnvVars_t ) && ( NULL != pGrid->pVar(EnvVars_t) ) && pPoints()->bHasVar(EnvVars_t) )
		NaturalDice( pPoints()->t(), nVertices(), 1, pGrid->pVar(EnvVars_t) );

	if ( USES( lUses, EnvVars_u ) && ( NULL != pGrid->pVar(EnvVars_u) ) && pPoints()->bHasVar(EnvVars_u) )
		NaturalDice( pPoints()->u(), nVertices(), 1, pGrid->pVar(EnvVars_u) );

	if ( USES( lUses, EnvVars_v ) && ( NULL != pGrid->pVar(EnvVars_v) ) && pPoints()->bHasVar(EnvVars_v) )
		NaturalDice( pPoints()->v(), nVertices(), 1, pGrid->pVar(EnvVars_v) );


	if ( NULL != pGrid->pVar(EnvVars_P) )
		NaturalDice( pPoints( 0 )->P(), nVertices(), 1, pGrid->pVar(EnvVars_P) );

	// If the shaders need N and they have been explicitly specified, then bilinearly interpolate them.
	if ( USES( lUses, EnvVars_N ) && ( NULL != pGrid->pVar(EnvVars_N) ) && pPoints()->bHasVar(EnvVars_N) )
	{
		NaturalDice( pPoints()->N(), nVertices(), 1, pGrid->pVar(EnvVars_N) );
		pGrid->SetbShadingNormals( true );
	}

	if ( USES( lUses, EnvVars_Ng ) )
	{
		CqVector3D	N(0,0,-1);
		//N = QGetRenderContext() ->matSpaceToSpace( "camera", "object", NULL, pGrid->pTransform() ) * N;
		TqUint u;
		for ( u = 0; u < nVertices(); u++ )
		{
			pGrid->pVar(EnvVars_Ng)->SetNormal( N, u );
		}
		pGrid->SetbGeometricNormals( true );
	}

	// Now we need to dice the user specified parameters as appropriate.
	std::vector<CqParameter*>::iterator iUP;
	std::vector<CqParameter*>::iterator end = pPoints()->aUserParams().end();
	std::vector<boost::shared_ptr<IqShader> > shaders;
	boost::shared_ptr<IqShader> pShader;
	if(NULL != (pShader = pGrid->pAttributes()->pshadSurface(QGetRenderContext()->Time())))
		shaders.push_back(pShader);
	if(NULL != (pShader = pGrid->pAttributes()->pshadDisplacement(QGetRenderContext()->Time())))
		shaders.push_back(pShader);
	if(NULL != (pShader = pGrid->pAttributes()->pshadAtmosphere(QGetRenderContext()->Time())))
		shaders.push_back(pShader);
	
	for ( iUP = pPoints()->aUserParams().begin(); iUP != end ; iUP++ )
	{
		/// \todo: Must transform point/vector/normal/matrix parameter variables from 'object' space to current before setting.
		for( std::vector<boost::shared_ptr<IqShader> >::iterator shader = shaders.begin(), last = shaders.end(); shader != last; ++shader )
		{
			// If the parameter is uniform or constant, use the standard surface/parameter
			// mechanism to copy, as it's behaviour is the same as any other surface type.
			if( (*iUP)->Class() == class_uniform || (*iUP)->Class() == class_constant )
				(*shader)->SetArgument( ( *iUP ), this );
			else
			{
				// For varying, facevarying, vertex or facevertex class parameters,
				// we need to perform the dicing in custom points code, as we have a u dicesize
				// of 1, and a v of <the number of points>, and the standard dicing code on
				// CqParameterTypedVarying<T, I, SLT>::Dice loops over v using <= due to the 
				// grid nature of normal surfaces, i.e. for 'v' micropolygons, there are v+1 vertices.
				IqShaderData* pVar = (*shader)->FindArgument( (*iUP)->strName().c_str() );
				if(NULL != pVar)
					NaturalDice( ( *iUP ), nVertices(), 1, pVar );
			}
		}
	}

	return( pGrid );
}


namespace {

/** \brief Implementation of dicing for points (helper function for CqPoints::NaturalDice)
 *
 * \param pParam - pointer to the parameter to take values from
 * \param paramIdx - indices in the full parameter array (pParam) which
 *                   correspond to the current gprim
 * \param diceSize - number of points in the diced output
 * \param pData - destination for diced shader data.
 */
template <class T, class SLT>
void pointsNaturalDice(CqParameter* pParam, const std::vector<TqInt>& paramIdx,
		TqInt diceSize, IqShaderData* pData)
{
	// Check if the target is a varying variable, if not, this is an error.
	if(pData->Class() != class_varying)
	{
		Aqsis::log() << error << "\"" << "Attempt to assign a varying value to uniform variable \"" <<
			pData->strName() << "\"" << std::endl;
		return;
	}

	CqParameterTyped<T, SLT>* pTParam = static_cast<CqParameterTyped<T, SLT>*>(pParam);
	const T* src = pTParam->pValue();
	for(TqInt j = 0, arraySize = pTParam->Count(); j < arraySize; j++)
	{
		SLT* dest = 0;
		pData->ArrayEntry(j)->GetValuePtr(dest);
		for(TqInt i = 0; i < diceSize; i++ )
			dest[i] = paramToShaderType<SLT,T>( src[arraySize*paramIdx[i] + j] );
	}
}

} // unnamed namespace

void CqPoints::NaturalDice(CqParameter* pParam, TqInt uDiceSize, TqInt vDiceSize,
		IqShaderData* pData )
{
	// \todo: Need to check for 'compatible' types here, not just a match, as
	// it's possible to assign an hpoint to a point for example, need something
	// more comprehensive.
	switch(pParam->Type())
	{
		case type_float:
			pointsNaturalDice<TqFloat, TqFloat>(pParam, m_KDTree.aLeaves(), uDiceSize, pData);
			break;
		case type_integer:
			pointsNaturalDice<TqInt, TqFloat>(pParam, m_KDTree.aLeaves(), uDiceSize, pData);
			break;
		case type_point:
		case type_vector:
		case type_normal:
			pointsNaturalDice<CqVector3D, CqVector3D>(pParam, m_KDTree.aLeaves(), uDiceSize, pData);
			break;
		case type_hpoint:
			pointsNaturalDice<CqVector4D, CqVector3D>(pParam, m_KDTree.aLeaves(), uDiceSize, pData);
			break;
		case type_color:
			pointsNaturalDice<CqColor, CqColor>(pParam, m_KDTree.aLeaves(), uDiceSize, pData);
			break;
		case type_string:
			pointsNaturalDice<CqString, CqString>(pParam, m_KDTree.aLeaves(), uDiceSize, pData);
			break;
		case type_matrix:
			pointsNaturalDice<CqMatrix, CqMatrix>(pParam, m_KDTree.aLeaves(), uDiceSize, pData);
			break;
		default:
			// left blank to avoid compiler warnings about unhandled types
			break;
	}
}

//---------------------------------------------------------------------
/** Determine whether the quadric is suitable for dicing.
 */

bool	CqPoints::Diceable(const CqMatrix& /* matCtoR */)
{
	TqUint gridsize = 256;

	const TqInt* poptGridSize = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "limits", "gridsize" );
	if ( poptGridSize )
		gridsize = (TqUint) poptGridSize[ 0 ];

	if( nVertices() > gridsize )
		return ( false );
	else
		return ( true );
}


//---------------------------------------------------------------------
/** Get the geometric bound of this GPrim in 'current' space.
 */

void	CqPoints::Bound(CqBound* bound) const
{
	CqVector4D* P = m_pPoints->P()->pValue();
	std::vector<TqInt>::const_iterator idx = m_KDTree.aLeaves().begin();
	for(TqInt i = 0; i < m_nVertices; i++)
		bound->Encapsulate( vectorCast<CqVector3D>( P[idx[i]] ) );

	// Expand the bound to take into account the width of the particles.
	bound->vecMax() += CqVector3D( m_MaxWidth, m_MaxWidth, m_MaxWidth );
	bound->vecMin() -= CqVector3D( m_MaxWidth, m_MaxWidth, m_MaxWidth );

	AdjustBoundForTransformationMotion( bound );
}

//---------------------------------------------------------------------
/** Split this GPrim into bicubic patches.
 */

TqInt CqPoints::Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits )
{
 	TqInt median = nVertices()/2;
 	// Split the KDTree and create two new primitives containing the split points set.
 	boost::shared_ptr<CqPoints> pA( new CqPoints( m_nVertices, pPoints() ) );
 	boost::shared_ptr<CqPoints> pB( new CqPoints( m_nVertices, pPoints() ) );
 
 	pA->m_nVertices = median;
 	pB->m_nVertices = nVertices()-median;
 
 	pA->SetSurfaceParameters( *this );
 	pB->SetSurfaceParameters( *this );
 
 	KDTree().Subdivide( pA->KDTree(), pB->KDTree() );
 
	// Initialise the max width of the child surfaces.  Initializing with the
	// current m_MaxWidth is the best strategy when all points in the
	// collection have similar widths.  For widely varying widths this may be
	// an inefficient strategy...
	pA->m_MaxWidth = m_MaxWidth;
	pB->m_MaxWidth = m_MaxWidth;

 	aSplits.push_back( pA );
 	aSplits.push_back( pB );
 
 	return( 2 );
}


//---------------------------------------------------------------------
/** Split the points, taking the split information from the specified donor points surfaces.
 */

TqInt CqPoints::CopySplit( std::vector<boost::shared_ptr<CqSurface> >& aSplits, CqPoints* pFrom1, CqPoints* pFrom2 )
{
	// Split the KDTree and create two new primitives containing the split points set.
 	boost::shared_ptr<CqPoints> pA( new CqPoints( m_nVertices, pPoints() ) );
 	boost::shared_ptr<CqPoints> pB( new CqPoints( m_nVertices, pPoints() ) );
 
 	pA->m_nVertices = pFrom1->m_nVertices;
 	pB->m_nVertices = pFrom2->m_nVertices;
 
 	pA->SetSurfaceParameters( *this );
 	pB->SetSurfaceParameters( *this );
 
 	pA->KDTree() = pFrom1->KDTree();
 	pB->KDTree() = pFrom2->KDTree();
 
 	aSplits.push_back( pA );
 	aSplits.push_back( pB );
 
 	return( 2 );
}


//---------------------------------------------------------------------
/** Initialise the KDTree to contain all the points in the list. Settign the
 * index list to the canonical form.
 */

void CqPoints::InitialiseKDTree()
{
	m_KDTree.aLeaves().reserve( nVertices() );
	for( TqUint i = 0; i < nVertices(); i++ )
		m_KDTree.aLeaves().push_back( i );
}


void CqPoints::InitialiseMaxWidth()
{
	TqInt cu = nVertices();	// Only need cu, as we know cv is 1.

	CqMatrix matObjectToCamera;
	QGetRenderContext() ->matSpaceToSpace( "object", "camera", NULL, pTransform().get(), QGetRenderContext()->Time(), matObjectToCamera );
	const CqParameterTypedConstant<TqFloat, type_float, TqFloat>* pConstantWidthParam = constantwidth( );

	CqVector3D Point0 = matObjectToCamera * CqVector3D(0,0,0);

	TqFloat i_radius = 1.0f;
	if( NULL != pConstantWidthParam )
		i_radius = pConstantWidthParam->pValue( 0 )[ 0 ];
	for ( TqInt iu = 0; iu < cu; iu++ )
	{
		TqFloat radius;
		// Find out if the "width" parameter was specified.
		CqParameterTypedVarying<TqFloat, type_float, TqFloat>* pWidthParam = width( 0 );

		if( NULL != pWidthParam )
			i_radius = pWidthParam->pValue( KDTree().aLeaves()[ iu ] )[ 0 ];

		radius = i_radius;
		// Get point in camera space.
		CqVector3D Point1 = matObjectToCamera * CqVector3D(radius,0,0);
		radius = (Point1-Point0).Magnitude();

		m_MaxWidth = max(m_MaxWidth, radius );
	}
}

//---------------------------------------------------------------------
/** Transform the points by the transformation matrices provided.
 */

void	CqPoints::Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx, TqInt iTime )
{
	CqPolygonPoints* pTimePoints = pPoints( iTime ).get();
	pTimePoints->Transform(matTx, matITTx, matRTx, 0);
}

void CqMicroPolyGridPoints::Split( long xmin, long xmax, long ymin, long ymax )
{
	if ( NULL == pVar(EnvVars_P) )
		return ;

	TqInt cu = uGridRes();	// Only need cu, as we know cv is 1.

	ADDREF( this );

	CqMatrix matCameraToObject0;
	QGetRenderContext() ->matSpaceToSpace( "camera", "object", NULL, pSurface()->pTransform().get(), 0, matCameraToObject0 );
	CqMatrix matCameraToRaster;
	QGetRenderContext() ->matSpaceToSpace( "camera", "raster", NULL, NULL, 0, matCameraToRaster );

	// Get a pointer to the surface, so that we can interrogate the "width" parameters.
	CqPoints* pPoints = static_cast<CqPoints*>( pSurface() );

	const CqParameterTypedConstant<TqFloat, type_float, TqFloat>* pConstantWidthParam = pPoints->constantwidth( );

	CqVector3D* pP;
	pVar(EnvVars_P) ->GetPointPtr( pP );

	// Ugh, this static_cast is pretty awful.  We really should either enhance
	// IqTransform or remove it completely.
	const CqTransform& objTrans = static_cast<const CqTransform&>(*pSurface()->pTransform());
	const CqTransform& camTrans = *QGetRenderContext()->GetCameraTransform();

	if( objTrans.isMoving() || camTrans.isMoving() )
	{
		// Get an array containing all the transformation key times.
		std::vector<TqFloat> keyTimes;
		mergeKeyTimes(keyTimes, objTrans, camTrans);
		TqInt totTimes = keyTimes.size();

		// Get an array of P's for all time positions.
		std::vector<std::vector<CqVector3D> > aaPtimes(totTimes);

		// Array of cached object to camera matrices for each time slot.
		std::vector<CqMatrix>	amatObjectToCameraT(totTimes);
		std::vector<CqMatrix>	amatNObjectToCameraT(totTimes);

		TqInt gsmin1 = GridSize() - 1;

		for( TqInt iTime = 0; iTime < totTimes; iTime++ )
		{
			CqMatrix matCameraToObjectT;
			QGetRenderContext() ->matSpaceToSpace( "camera", "object", NULL, &objTrans, keyTimes[iTime], matCameraToObjectT );
			QGetRenderContext() ->matSpaceToSpace( "object", "camera", NULL, &objTrans,  keyTimes[iTime], amatObjectToCameraT[ iTime ] );
			QGetRenderContext() ->matNSpaceToSpace( "object", "camera", NULL, &objTrans, keyTimes[iTime], amatNObjectToCameraT[ iTime ] );

			aaPtimes[ iTime ].resize( gsmin1 + 1 );

			for (TqInt i = gsmin1; i >= 0; i-- )
			{
				// This makes sure all our points are in object space.
				aaPtimes[ iTime ][ i ] = matCameraToObject0 * pP[ i ];
			}
		}

		for ( TqInt iu = 0; iu < cu; iu++ )
		{
			CqMicroPolygonMotionPoints* pNew = new CqMicroPolygonMotionPoints( this, iu );

			TqFloat radius;
			TqFloat i_radius = 1.0f;
			if( NULL != pConstantWidthParam )
				i_radius = pConstantWidthParam->pValue( 0 )[ 0 ];
			// Find out if the "width" parameter was specified.
			CqParameterTypedVarying<TqFloat, type_float, TqFloat>* pWidthParam = pPoints->width( 0 );

			if( NULL != pWidthParam )
				i_radius = pWidthParam->pValue( pPoints->KDTree().aLeaves()[ iu ] )[ 0 ];

			for( TqInt iTime = 0; iTime < totTimes; iTime++ )
			{
				radius = i_radius;
				// Get point in camera space.
				CqVector3D Point, pt, vecCamP;
				Point = pt = vecCamP = amatObjectToCameraT[ iTime ] * aaPtimes[ iTime ][ iu ];
				// Ensure z is retained in camera space when we convert to raster.
				TqFloat ztemp = Point.z();
				Point = matCameraToRaster * Point;
				Point.z( ztemp );
				pP[ iu ] = Point;

				// first, create a horizontal vector in object space which is
				//  the length of the current width.
				CqVector3D horiz( 1, 0, 0 );
				horiz = amatNObjectToCameraT[ iTime ] * horiz;
				horiz *= radius / horiz.Magnitude();

				// Get the current point in object space.
				CqVector3D pt_delta = pt + horiz;
				pt = amatObjectToCameraT[ iTime ] * pt;
				pt_delta = amatObjectToCameraT[ iTime ] * pt_delta;

				// finally, find the difference between the two points in
				//  the new space - this is the transformed width
				CqVector3D widthVector = pt_delta - pt;
				radius = widthVector.Magnitude();

				CqVector3D vecCamP2 = vecCamP + CqVector3D( radius, 0.0f, 0.0f );
				ztemp = vecCamP2.z();
				CqVector3D vecRasP2 = matCameraToRaster * vecCamP2;
				vecRasP2.z( ztemp );
				TqFloat ras_radius = ( vecRasP2 - Point ).Magnitude();
				radius = ras_radius * 0.5f;

				pNew->AppendKey( Point, radius, keyTimes[iTime] );
			}
			boost::shared_ptr<CqMicroPolygon> pMP( pNew );
			QGetRenderContext()->pImage()->AddMPG( pMP );
		}
	}
	else
	{
		CqMatrix matWorldToObjectT;
		QGetRenderContext() ->matSpaceToSpace( "world", "object", NULL, &objTrans, 0, matWorldToObjectT );
		CqMatrix matObjectToCameraT;
		QGetRenderContext() ->matSpaceToSpace( "object", "camera", NULL, &objTrans, 0, matObjectToCameraT );
		CqMatrix matNObjectToCameraT;
		QGetRenderContext() ->matNSpaceToSpace( "object", "camera", NULL, &objTrans, 0, matNObjectToCameraT );

		for ( TqInt iu = 0; iu < cu; iu++ )
		{
			// Get point in camera space.
			CqVector3D Point, pt, vecCamP;
			Point = pt = vecCamP = pP[ iu ];
			// Ensure z is retained in camera space when we convert to raster.
			TqFloat ztemp = Point.z();
			Point = matCameraToRaster * Point;
			Point.z( ztemp );
			pP[ iu ] = Point;

			TqFloat radius = 1.0f;
			if( NULL != pConstantWidthParam )
				radius = pConstantWidthParam->pValue( 0 )[ 0 ];
			// Find out if the "width" parameter was specified.
			CqParameterTypedVarying<TqFloat, type_float, TqFloat>* pWidthParam = pPoints->width( 0 );

			if( NULL != pWidthParam )
				radius = pWidthParam->pValue( pPoints->KDTree().aLeaves()[ iu ] )[ 0 ];

			// first, create a horizontal vector in camera space which is
			//  the length of the current width in current space
			CqVector3D horiz( 1, 0, 0 );
			horiz = matNObjectToCameraT * horiz;
			horiz *= radius / horiz.Magnitude();

			// Get the current point in object space.
			CqVector3D pt_delta = pt + horiz;
			pt = matObjectToCameraT * pt;
			pt_delta = matObjectToCameraT * pt_delta;

			// finally, find the difference between the two points in
			//  the new space - this is the transformed width
			CqVector3D widthVector = pt_delta - pt;
			radius = widthVector.Magnitude();

			CqVector3D vecCamP2 = vecCamP + CqVector3D( radius, 0.0f, 0.0f );
			ztemp = vecCamP2.z();
			CqVector3D vecRasP2 = matCameraToRaster * vecCamP2;
			vecRasP2.z( ztemp );
			TqFloat ras_radius = ( vecRasP2 - Point ).Magnitude();
			radius = ras_radius * 0.5f;

			CqMicroPolygonPoints* pNew = new CqMicroPolygonPoints(this, iu);
			pNew->Initialise( radius );

			boost::shared_ptr<CqMicroPolygon> pMP( pNew );
			QGetRenderContext()->pImage()->AddMPG( pMP );
		}
	}

	RELEASEREF( this );
}


bool CqMicroPolygonPoints::Sample( CqHitTestCache& cache, SqSampleData const& sample, TqFloat& D, CqVector2D& uv, TqFloat time, bool UsingDof ) const
{
	CqVector2D sampPos = sample.position;
	if(UsingDof)
		sampPos += compMul(sample.dofOffset, cache.cocMult[0]);
	if((vectorCast<CqVector2D>(cache.P[0]) - sampPos).Magnitude2() < m_radius*m_radius)
	{
		D = cache.P[0].z();
		return true;
	}
	return false;
}

void CqMicroPolygonPoints::CacheHitTestValues(CqHitTestCache& cache, bool usingDof) const
{
	pGrid()->pVar(EnvVars_P)->GetPoint(cache.P[0], m_Index);
	if(usingDof)
		cache.cocMult[0] = QGetRenderContext()->GetCircleOfConfusion(cache.P[0].z());
}

void CqMicroPolygonPoints::CacheOutputInterpCoeffs(SqMpgSampleInfo& cache) const
{
	CacheOutputInterpCoeffsConstant(cache);
}

void CqMicroPolygonPoints::InterpolateOutputs(const SqMpgSampleInfo& cache,
		const CqVector2D& pos, CqColor& outCol, CqColor& outOpac) const
{
	outCol = cache.col[0];
	outOpac = cache.opa[0];
}


//---------------------------------------------------------------------
/** Split the micropolygrid into individual MPGs,
 * \param xmin Integer minimum extend of the image part being rendered, takes into account buckets and clipping.
 * \param xmax Integer maximum extend of the image part being rendered, takes into account buckets and clipping.
 * \param ymin Integer minimum extend of the image part being rendered, takes into account buckets and clipping.
 * \param ymax Integer maximum extend of the image part being rendered, takes into account buckets and clipping.
 */

void CqMotionMicroPolyGridPoints::Split( long xmin, long xmax, long ymin, long ymax )
{
	// Get the main object, the one that was shaded.
	CqMicroPolyGrid * pGridA = static_cast<CqMicroPolyGridPoints*>( GetMotionObject( Time( 0 ) ) );
	TqInt iTime;

	assert(NULL != pGridA->pVar(EnvVars_P) );

	ADDREF( pGridA );

	TqInt cu = pGridA->uGridRes();	// Only need cu, as we know cv is 1.

	CqMatrix matCameraToObject0;
	QGetRenderContext() ->matSpaceToSpace( "camera", "object", NULL, pSurface()->pTransform().get(), pSurface()->pTransform()->Time(0), matCameraToObject0 );
	CqMatrix matObjectToCamera;
	QGetRenderContext() ->matSpaceToSpace( "object", "camera", NULL, pSurface()->pTransform().get(), pSurface()->pTransform()->Time(0), matObjectToCamera );
	CqMatrix matCameraToRaster;
	QGetRenderContext() ->matSpaceToSpace( "camera", "raster", NULL, NULL, pSurface()->pTransform()->Time(0), matCameraToRaster );

	CqMatrix matTx;
	QGetRenderContext() ->matSpaceToSpace( "object", "camera", NULL, pSurface()->pTransform().get(), pSurface()->pTransform()->Time(0), matTx );
	CqMatrix matITTx;
	QGetRenderContext() ->matNSpaceToSpace( "object", "camera", NULL, pSurface()->pTransform().get(), pSurface()->pTransform()->Time(0), matITTx );

	CqVector3D vecdefOriginRaster = matCameraToRaster * CqVector3D( 0.0f,0.0f,0.0f );

	TqInt NumTimes = cTimes();

	// Get an array of P's for all time positions.
	std::vector<std::vector<CqVector3D> > aaPtimes;
	aaPtimes.resize( NumTimes );

	// Array of cached object to camera matrices for each time slot.
	std::vector<CqMatrix>	amatObjectToCameraT;
	amatObjectToCameraT.resize( NumTimes );
	std::vector<CqMatrix>	amatNObjectToCameraT;
	amatNObjectToCameraT.resize( NumTimes );

	CqMatrix matObjectToCameraT;
	register TqInt i;
	TqInt gsmin1 = pGridA->pShaderExecEnv()->shadingPointCount() - 1;

	for( iTime = 0; iTime < NumTimes; iTime++ )
	{
		CqMatrix matCameraToObjectT;
		QGetRenderContext() ->matSpaceToSpace( "camera", "object", NULL, pSurface()->pTransform().get(), Time( iTime ), matCameraToObjectT );
		QGetRenderContext() ->matSpaceToSpace( "object", "camera", NULL, pSurface()->pTransform().get(),  Time( iTime ), amatObjectToCameraT[ iTime ] );
		QGetRenderContext() ->matNSpaceToSpace( "object", "camera", NULL, pSurface()->pTransform().get(), Time( iTime ), amatNObjectToCameraT[ iTime ] );

		aaPtimes[ iTime ].resize( gsmin1 + 1 );

		CqMicroPolyGridPoints* pGridT = static_cast<CqMicroPolyGridPoints*>( GetMotionObject( Time( iTime ) ) );

		CqVector3D* pP;
		pGridT->pVar(EnvVars_P) ->GetPointPtr( pP );

		for ( i = gsmin1; i >= 0; i-- )
		{
			// This makes sure all our points are in object space.
			aaPtimes[ iTime ][ i ] = matCameraToObject0 * pP[ i ];
		}
	}

	TqInt iu;
	for ( iu = 0; iu < cu; iu++ )
	{
		CqMicroPolygonMotionPoints* pNew = new CqMicroPolygonMotionPoints( pGridA, iu );

		TqFloat radius;
		TqInt iTime;
		for( iTime = 0; iTime < NumTimes; iTime++ )
		{
			radius = 1.0f;

			CqMicroPolyGridPoints* pGridT = static_cast<CqMicroPolyGridPoints*>( GetMotionObject( Time( iTime ) ) );

			// Get a pointer to the surface, so that we can interrogate the "width" parameters.
			CqPoints* pPoints = static_cast<CqPoints*>( pGridT->pSurface() );

			const CqParameterTypedConstant<TqFloat, type_float, TqFloat>* pConstantWidthParam =
			    pPoints->constantwidth( );

			if( NULL != pConstantWidthParam )
				radius = pConstantWidthParam->pValue( 0 )[ 0 ];

			// Find out if the "width" parameter was specified.
			CqParameterTypedVarying<TqFloat, type_float, TqFloat>* pWidthParam = pPoints->width( 0 );

			if( NULL != pWidthParam )
				radius = pWidthParam->pValue( pPoints->KDTree().aLeaves()[ iu ] )[ 0 ];

			// Get point in camera space.
			CqVector3D Point, pt, vecCamP;
			Point = pt = vecCamP = amatObjectToCameraT[ iTime ] * aaPtimes[ iTime ][ iu ];
			// Ensure z is retained in camera space when we convert to raster.
			TqFloat ztemp = Point.z();
			Point = matCameraToRaster * Point;
			Point.z( ztemp );

			// first, create a horizontal vector in object space which is
			//  the length of the current width.
			CqVector3D horiz( 1, 0, 0 );
			horiz = amatNObjectToCameraT[ iTime ] * horiz;
			horiz *= radius / horiz.Magnitude();

			// Get the current point in object space.
			CqVector3D pt_delta = pt + horiz;
			pt = amatObjectToCameraT[ iTime ] * pt;
			pt_delta = amatObjectToCameraT[ iTime ] * pt_delta;

			// finally, find the difference between the two points in
			//  the new space - this is the transformed width
			CqVector3D widthVector = pt_delta - pt;
			radius = widthVector.Magnitude();

			CqVector3D vecCamP2 = vecCamP + CqVector3D( radius, 0.0f, 0.0f );
			ztemp = vecCamP2.z();
			CqVector3D vecRasP2 = matCameraToRaster * vecCamP2;
			vecRasP2.z( ztemp );
			TqFloat ras_radius = ( vecRasP2 - Point ).Magnitude();
			radius = ras_radius * 0.5f;

			pNew->AppendKey( Point, radius, Time( iTime ) );
		}
		boost::shared_ptr<CqMicroPolygon> pMP( pNew );
		QGetRenderContext()->pImage()->AddMPG( pMP );
	}

	RELEASEREF( pGridA );

	// Delete the donor motion grids, as their work is done.
	/*    for ( iTime = 1; iTime < cTimes(); iTime++ )
	    {
	        CqMicroPolyGrid* pg = static_cast<CqMicroPolyGrid*>( GetMotionObject( Time( iTime ) ) );
	        if ( NULL != pg )
	            RELEASEREF( pg );
	    }
	*/    //		delete( GetMotionObject( Time( iTime ) ) );
}


//---------------------------------------------------------------------
/** Calculate a list of 2D bounds for this micropolygon,
 */

void CqMicroPolygonMotionPoints::BuildBoundList( TqUint timeRanges )
{
	m_BoundList.Clear();

	assert( NULL != m_Keys[0] );

	CqBound start = m_Keys[0]->GetBound();
	TqFloat	startTime = m_Times[ 0 ];
	TqInt cTimes = m_Keys.size();
	for ( TqInt i = 1; i < cTimes; i++ )
	{
		CqBound end = m_Keys[i]->GetBound();
		CqBound mid0( start );
		CqBound mid1;
		TqFloat endTime = m_Times[ i ];
		TqFloat time = startTime;

		TqInt d;
		// arbitary number of divisions, should be related to screen size of blur
		TqInt divisions = 4;
		TqFloat delta = 1.0f / static_cast<TqFloat>( divisions );
		m_BoundList.SetSize( divisions );
		for ( d = 1; d <= divisions; d++ )
		{
			mid1.vecMin() = delta * ( end.vecMin() - start.vecMin() ) + start.vecMin();
			mid1.vecMax() = delta * ( end.vecMax() - start.vecMax() ) + start.vecMax();
			mid0.Encapsulate(&mid1);
			m_BoundList.Set( d-1, mid0, time );
			time = delta * ( endTime - startTime ) + startTime;
			mid0 = mid1;
			delta += delta;
		}
		start = end;
		startTime = endTime;
	}
	m_BoundReady = true;
}


//---------------------------------------------------------------------
/** Sample the specified point against the MPG at the specified time.
 * \param vecSample 2D vector to sample against.
 * \param time Shutter time to sample at.
 * \param D Storage for depth if valid hit.
 * \param uv - Output for parametric coordinates inside the micropolygon.
 * \return Boolean indicating smaple hit.
 */

bool CqMicroPolygonMotionPoints::Sample( CqHitTestCache& hitTestCache, SqSampleData const& sample, TqFloat& D, CqVector2D& uv, TqFloat time, bool UsingDof ) const
{
	TqInt iIndex = 0;
	TqFloat Fraction = 0.0f;
	bool Exact = true;

	if ( time > m_Times.front() )
	{
		if ( time >= m_Times.back() )
			iIndex = m_Times.size() - 1;
		else
		{
			// Find the appropriate time span.
			iIndex = 0;
			while ( time >= m_Times[ iIndex + 1 ] )
				iIndex += 1;
			Fraction = ( time - m_Times[ iIndex ] ) / ( m_Times[ iIndex + 1 ] - m_Times[ iIndex ] );
			Exact = ( m_Times[ iIndex ] == time );
		}
	}

	TqFloat r = 0;
	CqVector3D pos;
	if( Exact )
	{
		CqMovingMicroPolygonKeyPoints* pMP1 = m_Keys[ iIndex ];
		r = pMP1->m_radius;
		pos = pMP1->m_Point0;
	}
	else
	{
		CqMovingMicroPolygonKeyPoints* pMP1 = m_Keys[ iIndex ];
		CqMovingMicroPolygonKeyPoints* pMP2 = m_Keys[ iIndex + 1 ];
		pos = (pMP2->m_Point0 - pMP1->m_Point0) * Fraction + pMP1->m_Point0;
		r = (pMP2->m_radius - pMP1->m_radius) * Fraction + pMP1->m_radius;
	}

	CqVector2D sampPos = sample.position;
	if(UsingDof)
	{
		sampPos += compMul(sample.dofOffset,
						   QGetRenderContext()->GetCircleOfConfusion(pos.z()));
	}
	if( (vectorCast<CqVector2D>(pos) - sampPos).Magnitude2() < r*r )
	{
		D = pos.z();
		return true;
	}
	return false;
}

void CqMicroPolygonMotionPoints::CacheHitTestValues(CqHitTestCache& cache, bool usingDof) const
{ }

void CqMicroPolygonMotionPoints::CacheOutputInterpCoeffs(SqMpgSampleInfo& cache) const
{
	CacheOutputInterpCoeffsConstant(cache);
}

void CqMicroPolygonMotionPoints::InterpolateOutputs(const SqMpgSampleInfo& cache,
		const CqVector2D& pos, CqColor& outCol, CqColor& outOpac) const
{
	outCol = cache.col[0];
	outOpac = cache.opa[0];
}

//---------------------------------------------------------------------
/** Store the vectors of the micropolygon at the specified shutter time.
 * \param vA 3D Vector.
 * \param vB 3D Vector.
 * \param vC 3D Vector.
 * \param vD 3D Vector.
 * \param time Float shutter time that this MPG represents.
 */

void CqMicroPolygonMotionPoints::AppendKey( const CqVector3D& vA, TqFloat radius, TqFloat time )
{
	//	assert( time >= m_Times.back() );

	// Add a new planeset at the specified time.
	CqMovingMicroPolygonKeyPoints* pMP = new CqMovingMicroPolygonKeyPoints( vA, radius );
	m_Times.push_back( time );
	m_Keys.push_back( pMP );
	if ( m_Times.size() == 1 )
		m_Bound = pMP->GetBound();
	else
	{
		CqBound B(pMP->GetBound());
		m_Bound.Encapsulate( &B );
	}
}


} // namespace Aqsis
//---------------------------------------------------------------------



