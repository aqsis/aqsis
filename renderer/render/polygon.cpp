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


/** \file
		\brief Implements the classes and support structures for handling polygons.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	"polygon.h"
#include	"patch.h"

namespace Aqsis {


//---------------------------------------------------------------------
/** Return the boundary extents in camera space of the polygon
 */

void CqPolygonBase::Bound(CqBound* bound) const
{
	CqVector3D	vecA( FLT_MAX, FLT_MAX, FLT_MAX );
	CqVector3D	vecB( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	TqInt i, n;
	n = NumVertices();
	for ( i = 0; i < n; i++ )
	{
		CqVector3D	vecV = PolyP( i );
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
	}
	bound->vecMin() = vecA;
	bound->vecMax() = vecB;
}


//---------------------------------------------------------------------
/** Split the polygon into bilinear patches.
 *  This split has a special way of dealing with triangles. To avoid the problem of grids with degenerate micro polygons
 *  ( micro polygons with two points the same. ) we treat a triangle as a parallelogram, by extending the fourth point out.
 *  Then everything on the left side of the vector between points 1 and 2 ( on the same side as point 0 ) is rendered, 
 *  everything on the right (the same side as point 3) is not.
 */

TqInt CqPolygonBase::Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits )
{
	CqVector3D	vecN(0,0,0);
	TqInt indexA, indexB, indexC, indexD;

	// We need to take into account Orientation here, even though most other
	// primitives leave it up to the CalcNormals function on the MPGrid, because we
	// are forcing N to be setup here, so clockwise nature is important.
	bool CSO = pTransform()->GetHandedness(pTransform()->Time(0));
	bool O = pAttributes() ->GetIntegerAttribute( "System", "Orientation" ) [ 0 ] != 0;

	indexA = 0;
	indexB = 1;

	TqInt iUses = PolyUses();
	TqInt n = NumVertices();

	// Get the normals, or calculate the facet normal if not specified.
	if ( !bHasVar(EnvVars_N) )
	{
		CqVector3D vecA = PolyP( indexA );
		// Find two suitable vectors, and produce a geometric normal to use.
		TqInt i = 1;
		CqVector3D	vecN0, vecN1;

		while ( i < n )
		{
			vecN0 = PolyP(i) - vecA;
			if ( vecN0.Magnitude() > FLT_EPSILON )
				break;
			i++;
		}
		i++;
		while ( i < n )
		{
			vecN1 = PolyP(i) - vecA;
			if ( vecN1.Magnitude() > FLT_EPSILON && vecN1 != vecN0 )
				break;
			i++;
		}
		vecN = vecN0 % vecN1;
		vecN = ( (O && CSO) || (!O && !CSO) ) ? vecN : -vecN;
		vecN.Unit();
	}

	// Start by splitting the polygon into 4 point patches.
	// Get the normals, or calculate the facet normal if not specified.

	TqInt cNew = 0;
	TqInt i;
	for ( i = 2; i < n; i += 2 )
	{
		indexC = indexD = i;
		if ( n > i + 1 )
			indexD = i + 1;

		// Create bilinear patches
		boost::shared_ptr<CqSurfacePatchBilinear> pNew( new CqSurfacePatchBilinear() );
		//ADDREF( pNew );
		pNew->SetSurfaceParameters( Surface() );

		/* Comment this out for now as it breaks Bug948827 in a strange way, needs more investigation */
		/*        if ( indexC == indexD )
				{
					// Calculate which point in the triangle produces an angle with it's neighbours that is closest to 90 degrees.
					// Placing the phantom point opposite this point will ensure the best orientation of the final grids, reducing
					// shading artefacts.
					CqVector3D pointA = PolyP(indexA);
					CqVector3D pointB = PolyP(indexB);
					CqVector3D pointC = PolyP(indexC);
					TqFloat aA = 1.5707f - acosf((pointB - pointA).Unit()*(pointC - pointA).Unit());
					TqFloat aB = 1.5707f - acosf((pointA - pointB).Unit()*(pointC - pointB).Unit());
					TqFloat aC = 1.5707f - acosf((pointA - pointC).Unit()*(pointB - pointC).Unit());
					TqInt cycle = 0;
					if( aB < aA && aB < aC )		cycle = 1;
					else if( aC < aA && aC < aB)	cycle = 2;
					for(; cycle>0; --cycle)
					{
						TqInt temp = indexA;
						indexA = indexB;
						indexB = indexC;
						indexC = indexD = temp;
					}
				}
		*/
		TqInt iUPA, iUPB, iUPC, iUPD;
		TqInt iUPAf, iUPBf, iUPCf, iUPDf;
		// Get the indices for varying variables.
		iUPA = PolyIndex( indexA );
		iUPB = PolyIndex( indexB );
		iUPC = PolyIndex( indexC );
		iUPD = PolyIndex( indexD );

		// Get the indices for facevarying variables.
		iUPAf = FaceVaryingIndex( indexA );
		iUPBf = FaceVaryingIndex( indexB );
		iUPCf = FaceVaryingIndex( indexC );
		iUPDf = FaceVaryingIndex( indexD );

		// Copy any user specified primitive variables.
		std::vector<CqParameter*>::iterator iUP;
		std::vector<CqParameter*>::iterator end = Surface().aUserParams().end();
		for ( iUP = Surface().aUserParams().begin(); iUP != end; iUP++ )
		{
			CqParameter* pNewUP = ( *iUP ) ->CloneType( ( *iUP ) ->strName().c_str(), ( *iUP ) ->Count() );

			if ( pNewUP->Class() == class_varying || pNewUP->Class() == class_vertex )
			{
				pNewUP->SetSize( pNew->cVarying() );
				pNewUP->SetValue( ( *iUP ), 0, iUPA );
				pNewUP->SetValue( ( *iUP ), 1, iUPB );
				pNewUP->SetValue( ( *iUP ), 2, iUPD );
				pNewUP->SetValue( ( *iUP ), 3, iUPC );
				if ( indexC == indexD )
					CreatePhantomData( pNewUP );
			}
			else if ( pNewUP->Class() == class_uniform )
			{
				pNewUP->SetSize( pNew->cUniform() );
				pNewUP->SetValue( ( *iUP ), 0, MeshIndex() );
			}
			else if ( pNewUP->Class() == class_constant )
			{
				pNewUP->SetSize( 1 );
				pNewUP->SetValue( ( *iUP ), 0, 0 );
			}
			else if ( pNewUP->Class() == class_facevarying || pNewUP->Class() == class_facevertex )
			{
				pNewUP->SetSize( pNew->cVarying() );
				pNewUP->SetValue( ( *iUP ), 0, iUPAf );
				pNewUP->SetValue( ( *iUP ), 1, iUPBf );
				pNewUP->SetValue( ( *iUP ), 2, iUPDf );
				pNewUP->SetValue( ( *iUP ), 3, iUPCf );
				if ( indexC == indexD )
					CreatePhantomData( pNewUP );
			}

			pNew->AddPrimitiveVariable( pNewUP );
		}

		// If this is a triangle, then mark the patch as a special case. See function header comment for more details.
		if ( indexC == indexD )
			pNew->SetfHasPhantomFourthVertex( true );

		// If there are no smooth normals specified, then fill in the facet normal at each vertex.
		if ( !bHasVar(EnvVars_N) && USES( iUses, EnvVars_N ) )
		{
			CqParameterTypedVarying<CqVector3D, type_normal, CqVector3D>* pNewUP = new CqParameterTypedVarying<CqVector3D, type_normal, CqVector3D>( "N", 1 );
			pNewUP->SetSize( pNew->cVarying() );

			pNewUP->pValue() [ 0 ] = vecN;
			pNewUP->pValue() [ 1 ] = vecN;
			pNewUP->pValue() [ 2 ] = vecN;
			pNewUP->pValue() [ 3 ] = vecN;

			pNew->AddPrimitiveVariable( pNewUP );
		}

		// If the shader needs s/t or u/v, and s/t is not specified, then at this point store the object space x,y coordinates.
		if ( USES( iUses, EnvVars_s ) || USES( iUses, EnvVars_t ) || USES( iUses, EnvVars_u ) || USES( iUses, EnvVars_v ) )
		{
			CqVector3D PA, PB, PC, PD;
			CqMatrix matCurrentToWorld;
			QGetRenderContext() ->matSpaceToSpace( "current", "object", NULL, Surface().pTransform().get(), Surface().pTransform() ->Time(0), matCurrentToWorld );
			PA = matCurrentToWorld * vectorCast<CqVector3D>(pNew->P() ->pValue() [ 0 ]);
			PB = matCurrentToWorld * vectorCast<CqVector3D>(pNew->P() ->pValue() [ 1 ]);
			PC = matCurrentToWorld * vectorCast<CqVector3D>(pNew->P() ->pValue() [ 3 ]);
			PD = matCurrentToWorld * vectorCast<CqVector3D>(pNew->P() ->pValue() [ 2 ]);

			if ( USES( iUses, EnvVars_s ) && !bHasVar(EnvVars_s) )
			{
				CqParameterTypedVarying<TqFloat, type_float, TqFloat>* pNewUP = new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "s" );
				pNewUP->SetSize( pNew->cVarying() );

				pNewUP->pValue() [ 0 ] = PA.x();
				pNewUP->pValue() [ 1 ] = PB.x();
				pNewUP->pValue() [ 2 ] = PD.x();
				pNewUP->pValue() [ 3 ] = PC.x();

				pNew->AddPrimitiveVariable( pNewUP );
			}

			if ( USES( iUses, EnvVars_t ) && !bHasVar(EnvVars_t) )
			{
				CqParameterTypedVarying<TqFloat, type_float, TqFloat>* pNewUP = new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "t" );
				pNewUP->SetSize( pNew->cVarying() );

				pNewUP->pValue() [ 0 ] = PA.y();
				pNewUP->pValue() [ 1 ] = PB.y();
				pNewUP->pValue() [ 2 ] = PD.y();
				pNewUP->pValue() [ 3 ] = PC.y();

				pNew->AddPrimitiveVariable( pNewUP );
			}

			if ( USES( iUses, EnvVars_u ) && !bHasVar(EnvVars_u) )
			{
				CqParameterTypedVarying<TqFloat, type_float, TqFloat>* pNewUP = new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "u" );
				pNewUP->SetSize( pNew->cVarying() );

				// We choose to violate the RI standard here - the RISpec
				// requires the the surface coordinate u takes the value of the
				// x-coordinates of the corners of the patch in the same way
				// that the texture coordinate "t" does.
				//
				// This really screws up derivative calcuations in the shading
				// language since following the RISpec implies that u isn't
				// aligned with the grid.  Worse, du can turn out to be
				// technically equal to 0, which prevents the expression
				//
				//   dPu = Du(P)*du;
				//
				// from making sense.  This is really bad from the point of
				// view of anyone trying to use something like dPu to antialias
				// thier shaders.
				pNewUP->pValue() [ 0 ] = 0;
				pNewUP->pValue() [ 1 ] = 1;
				pNewUP->pValue() [ 2 ] = 0;
				pNewUP->pValue() [ 3 ] = 1;

				pNew->AddPrimitiveVariable( pNewUP );
			}

			if ( USES( iUses, EnvVars_v ) && !bHasVar(EnvVars_v) )
			{
				CqParameterTypedVarying<TqFloat, type_float, TqFloat>* pNewUP = new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "v" );
				pNewUP->SetSize( pNew->cVarying() );

				// We choose to violate the RI standard here - see the comments
				// above about u.
				pNewUP->pValue() [ 0 ] = 0;
				pNewUP->pValue() [ 1 ] = 0;
				pNewUP->pValue() [ 2 ] = 1;
				pNewUP->pValue() [ 3 ] = 1;

				pNew->AddPrimitiveVariable( pNewUP );
			}

		}

		aSplits.push_back( pNew );
		cNew++;

		// Move onto the next quad
		indexB = indexD;
	}
	return ( cNew );
}


//---------------------------------------------------------------------
/** Generate phanton data to 'stretch' the triangle patch into a parallelogram.
 */

void CqPolygonBase::CreatePhantomData( CqParameter* pParam )
{
	assert( pParam->Class() == class_varying || 
			pParam->Class() == class_vertex || 
			pParam->Class() == class_facevarying ||
			pParam->Class() == class_facevertex );

	TqInt iArrayCount = 1;
	TqInt iArray;
	if( pParam->Count() > 0)
		iArrayCount = pParam->Count();

	switch ( pParam->Type() )
	{
			case type_point:
			case type_vector:
			case type_normal:
			{
				CqParameterTyped<CqVector3D, CqVector3D>* pTParam = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( pParam );
				for( iArray = 0; iArray < iArrayCount; iArray++ )
					pTParam->pValue( 3 ) [ iArray ] = ( pTParam->pValue( 1 ) [ iArray ] - pTParam->pValue( 0 ) [ iArray ] ) + pTParam->pValue( 2 ) [ iArray ];
				break;
			}

			case type_hpoint:
			{
				CqParameterTyped<CqVector4D, CqVector3D>* pTParam = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( pParam );
				for( iArray = 0; iArray < iArrayCount; iArray++ )
					pTParam->pValue( 3 ) [ iArray ] = ( pTParam->pValue( 1 ) [ iArray ] - pTParam->pValue( 0 ) [ iArray ] ) + pTParam->pValue( 2 ) [ iArray ];
				break;
			}

			case type_float:
			{
				CqParameterTyped<TqFloat, TqFloat>* pTParam = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( pParam );
				for( iArray = 0; iArray < iArrayCount; iArray++ )
					pTParam->pValue( 3 ) [ iArray ] = ( pTParam->pValue( 1 ) [ iArray ] - pTParam->pValue( 0 ) [ iArray ] ) + pTParam->pValue( 2 ) [ iArray ];
				break;
			}

			case type_integer:
			{
				CqParameterTyped<TqInt, TqFloat>* pTParam = static_cast<CqParameterTyped<TqInt, TqFloat>*>( pParam );
				for( iArray = 0; iArray < iArrayCount; iArray++ )
					pTParam->pValue( 3 ) [ iArray ] = ( pTParam->pValue( 1 ) [ iArray ] - pTParam->pValue( 0 ) [ iArray ] ) + pTParam->pValue( 2 ) [ iArray ];
				break;
			}

			case type_color:
			{
				CqParameterTyped<CqColor, CqColor>* pTParam = static_cast<CqParameterTyped<CqColor, CqColor>*>( pParam );
				for( iArray = 0; iArray < iArrayCount; iArray++ )
					pTParam->pValue( 3 ) [ iArray ] = ( pTParam->pValue( 1 ) [ iArray ] - pTParam->pValue( 0 ) [ iArray ] ) + pTParam->pValue( 2 ) [ iArray ];
				break;
			}

			case type_matrix:
			{
				CqParameterTyped<CqMatrix, CqMatrix>* pTParam = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( pParam );
				for( iArray = 0; iArray < iArrayCount; iArray++ )
					pTParam->pValue( 3 ) [ iArray ] = ( pTParam->pValue( 1 ) [ iArray ] - pTParam->pValue( 0 ) [ iArray ] ) + pTParam->pValue( 2 ) [ iArray ];
				break;
			}

			case type_string:
			{
				CqParameterTyped<CqString, CqString>* pTParam = static_cast<CqParameterTyped<CqString, CqString>*>( pParam );
				for( iArray = 0; iArray < iArrayCount; iArray++ )
					pTParam->pValue( 3 ) [ iArray ] = ( pTParam->pValue( 1 ) [ iArray ] - pTParam->pValue( 0 ) [ iArray ] ) + pTParam->pValue( 2 ) [ iArray ];
				break;
			}

			default:
			{
				// left blank to avoid compiler warnings about unhandled types
				break;
			}
	}
}

//---------------------------------------------------------------------
/** Default constructor.
 */

CqSurfacePolygon::CqSurfacePolygon( TqInt cVertices ) : CqSurface(),
		m_cVertices( cVertices )
{}


//---------------------------------------------------------------------
/** Destructor.
 */

CqSurfacePolygon::~CqSurfacePolygon()
{}


//---------------------------------------------------------------------
/** Check if a polygon is degenerate, i.e. all points collapse to the same or almost the same place.
 */

bool CqSurfacePolygon::CheckDegenerate() const
{
	// Check if all points are within a minute distance of each other.
	bool	fDegen = true;
	TqInt i, n;
	n = NumVertices();
	for ( i = 1; i < n; i++ )
	{
		if ( ( PolyP( i ) - PolyP( i - 1 ) ).Magnitude() > FLT_EPSILON )
		{
			fDegen = false;
			break;
		}
	}
	return ( fDegen );
}

//---------------------------------------------------------------------
/** Create a copy of this polygon primitive.
 */

CqSurface* CqSurfacePolygon::Clone() const
{
	CqSurfacePolygon* clone = new CqSurfacePolygon();
	CqSurface::CloneData(clone);
	clone->m_cVertices = m_cVertices;
	return ( clone );
}


//---------------------------------------------------------------------
/** Copy constructor.
 */

/* CqSurfacePointsPolygon::CqSurfacePointsPolygon( const CqSurfacePointsPolygon& From )
 * {
 * 	*this = From;
 * }
 */


//---------------------------------------------------------------------
/** Assignment operator.
 */

/* CqSurfacePointsPolygon& CqSurfacePointsPolygon::operator=( const CqSurfacePointsPolygon& From )
 * {
 * 	TqInt i;
 * 	m_aIndices.resize( From.m_aIndices.size() );
 * 	for ( i = From.m_aIndices.size() - 1; i >= 0; i-- )
 * 		m_aIndices[ i ] = From.m_aIndices[ i ];
 * 
 * 	// Store the old points array pointer, as we must reference first then
 * 	// unreference to avoid accidental deletion if they are the same and we are the
 * 	// last reference.
 * 	m_pPoints = From.m_pPoints;
 * 	m_Index = From.m_Index;
 * 	m_FaceVaryingIndex = From.m_FaceVaryingIndex;
 * 
 * 	return ( *this );
 * }
 */




//---------------------------------------------------------------------
/** Transform the points by the specified matrix.
 */

void	CqPolygonPoints::Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx, TqInt iTime )
{
//	if ( m_Transformed )
//		return ;

	CqSurface::Transform( matTx, matITTx, matRTx, iTime );

//	m_Transformed = true;
}


CqSurface* CqPolygonPoints::Clone() const
{
	CqPolygonPoints* clone = new CqPolygonPoints();
	CqSurface::CloneData(clone);
	clone->m_cVertices = m_cVertices;
	clone->m_Transformed = m_Transformed;
	clone->m_cFaces = m_cFaces;
	clone->m_sumnVerts = m_sumnVerts;
	return(clone);
}


void	CqSurfacePointsPolygons::Bound(CqBound* bound) const
{
	if( m_pPoints && m_pPoints->P() )
	{
		TqInt PointIndex;
		for( PointIndex = m_pPoints->P()->Size()-1; PointIndex >= 0; PointIndex-- )
			bound->Encapsulate( vectorCast<CqVector3D>(m_pPoints->P()->pValue()[PointIndex]) );
	}
	AdjustBoundForTransformationMotion( bound );
}

TqInt CqSurfacePointsPolygons::Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits )
{
	TqInt	CreatedPolys = 0;
	TqInt	iP = 0, poly;
	for ( poly = 0; poly < m_NumPolys; poly++ )
	{
		// Create a surface polygon
		boost::shared_ptr<CqSurfacePointsPolygon> pSurface( new CqSurfacePointsPolygon( m_pPoints, poly, iP ) );
		RtBoolean fValid = RI_TRUE;

		pSurface->aIndices().resize( m_PointCounts[ poly ] );
		TqUint i;
		for ( i = 0; i < (TqUint)m_PointCounts[ poly ]; i++ )          	// Fill in the points
		{
			if ( (TqUint) m_PointIndices[ iP ] >= m_pPoints->P()->Size() )
			{
				fValid = RI_FALSE;
				//CqAttributeError( 1, Severity_Normal, "Invalid PointsPolygon index", pSurface->pAttributes() );
				CqString objname( "unnamed" );
				const CqString* pattrName = pSurface->pAttributes()->GetStringAttribute( "identifier", "name" );
				if ( pattrName != 0 )
					objname = pattrName[ 0 ];
				Aqsis::log() << warning << "Invalid PointsPolygon index in object \"" << objname.c_str() << "\"" << std::endl;

				break;
			}
			pSurface->aIndices() [ i ] = m_PointIndices[ iP ];
			iP++;
		}
		if ( fValid )
		{
			aSplits.push_back( pSurface );
			CreatedPolys++;
		}
	}
	return( CreatedPolys );
}


CqSurface* CqSurfacePointsPolygons::Clone() const
{
	// Make a 'complete' clone of this primitive, which means cloning the points too.
	CqPolygonPoints* clone_points = static_cast<CqPolygonPoints*>(m_pPoints->Clone());
	CqSurfacePointsPolygons* clone = new CqSurfacePointsPolygons();
	CqSurface::CloneData(clone);
	clone->m_NumPolys = m_NumPolys;
	clone->m_PointCounts = m_PointCounts;
	clone->m_PointIndices = m_PointIndices;
	clone->m_pPoints = boost::shared_ptr<CqPolygonPoints>(clone_points);
	return(clone);
}

} // namespace Aqsis
//---------------------------------------------------------------------
