// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
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
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	"aqsis.h"
#include	"polygon.h"
#include	"patch.h"
#include	"imagebuffer.h"

START_NAMESPACE( Aqsis )


//---------------------------------------------------------------------
/** Return the boundary extents in camera space of the polygon
 */

CqBound CqPolygonBase::Bound() const
{
	CqVector3D	vecA( FLT_MAX, FLT_MAX, FLT_MAX );
	CqVector3D	vecB( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	TqInt i;
	for ( i = 0; i < NumVertices(); i++ )
	{
		CqVector3D	vecV = PolyP( i );
		if ( vecV.x() < vecA.x() ) vecA.x( vecV.x() );
		if ( vecV.y() < vecA.y() ) vecA.y( vecV.y() );
		if ( vecV.x() > vecB.x() ) vecB.x( vecV.x() );
		if ( vecV.y() > vecB.y() ) vecB.y( vecV.y() );
		if ( vecV.z() < vecA.z() ) vecA.z( vecV.z() );
		if ( vecV.z() > vecB.z() ) vecB.z( vecV.z() );
	}
	CqBound	B;
	B.vecMin() = vecA;
	B.vecMax() = vecB;
	return ( B );
}


//---------------------------------------------------------------------
/** Determine whether the polygon can be diced or not.
 */

TqBool CqPolygonBase::Diceable()
{
	// Get the bound in camera space.
	/*	CqBound B=Bound();
		// Convert it to screen space
		B.Transform(QGetRenderContext()->matSpaceToSpace("camera","raster"));
	 
		// Get the appropriate shading rate.
		float ShadingRate=pAttributes()->fEffectiveShadingRate();
	//	if(QGetRenderContext()->Mode()==RenderMode_Shadows)
	//	{
	//		const TqFloat* pattrShadowShadingRate=pAttributes()->GetFloatAttribute("render","shadow_shadingrate");
	//		if(pattrShadowShadingRate!=0)
	//			ShadingRate=pattrShadowShadingRate[0];
	//	}	
	 
		ShadingRate=static_cast<float>(sqrt(ShadingRate));
	 
		// Calculate the screen area to decide whether to dice or not.
		TqFloat sx=(B.vecMax().x()-B.vecMin().x())/ShadingRate;
		TqFloat sy=(B.vecMax().y()-B.vecMin().y())/ShadingRate;
		TqFloat area=sx*sy;
	 
		if(area>256)	return(TqFalse);
		else			return(TqTrue);*/ 
	return ( TqFalse );
}


//---------------------------------------------------------------------
/** Dice the polygon into a grid of MPGs for rendering.
 */

CqMicroPolyGridBase* CqPolygonBase::Dice()
{
	return ( 0 );
	/*	TqInt	iA,iB,iC,iD;
		TqFloat sA,tA,sB,tB,sC,tC,sD,tD;
		TqFloat uA,vA,uB,vB,uC,vC,uD,vD;
	 
		const CqMatrix& matObjToRaster=QGetRenderContext()->matSpaceToSpace("camera","raster");
	 
		TqBool	bhasN=bHasN();
		TqBool	bhass=bHass();
		TqBool	bhast=bHast();
		TqBool	bhasu=bHasu();
		TqBool	bhasv=bHasv();
		TqBool	bhasCs=bHasCs();
		TqBool	bhasOs=bHasOs();
	 
		TqInt iUses=PolyUses();
	 
		// Get the appropriate shading rate.
		float ShadingRate=pAttributes()->fEffectiveShadingRate();
	//	if(QGetRenderContext()->Mode()==RenderMode_Shadows)
	//	{
	//		const TqFloat* pattrShadowShadingRate=pAttributes()->GetFloatAttribute("render","shadow_shadingrate");
	//		if(pattrShadowShadingRate!=0)
	//			ShadingRate=pattrShadowShadingRate[0];
	//	}	
	 
		ShadingRate=static_cast<float>(sqrt(ShadingRate));
	 
		// Get the first two vertices
		iA=0;
		iB=1;
	 
		TqInt i=2;
		while(i<NumVertices())
		{
			iC=iD=i;
			i++;
			if(i<NumVertices())
			{
				iD=i;
				i++;
			}
			// Now we have the four indices, do a basic bilinear interpolation of the values.
			// First of all get the screen space coordinates to caluclate an approximate u,v dicing level,
			CqVector3D PA=PolyP(iA);
			CqVector3D PB=PolyP(iB);
			CqVector3D PC=PolyP(iC);
			CqVector3D PD=PolyP(iD);
	 
			if(bhass)
			{
				sA=Polys(iA);
				sB=Polys(iB);
				sC=Polys(iC);
				sD=Polys(iD);
			}
	 
			if(bhast)
			{
				tA=Polyt(iA);
				tB=Polyt(iB);
				tC=Polyt(iC);
				tD=Polyt(iD);
			}
	 
			if(bhasu)
			{
				uA=Polyu(iA);
				uB=Polyu(iB);
				uC=Polyu(iC);
				uD=Polyu(iD);
			}
	 
			if(bhasv)
			{
				vA=Polyv(iA);
				vB=Polyv(iB);
				vC=Polyv(iC);
				vD=Polyv(iD);
			}
	 
			PA=matObjToRaster*PA;
			PB=matObjToRaster*PB;
			PC=matObjToRaster*PC;
			PD=matObjToRaster*PD;
	 
			TqFloat ab=CqVector3D(PB-PA).Magnitude();
			TqFloat bc=CqVector3D(PC-PB).Magnitude();
			TqFloat cd=CqVector3D(PD-PC).Magnitude();
			TqFloat da=CqVector3D(PA-PD).Magnitude();
			TqFloat uLen=MAX(ab,cd);
			TqFloat vLen=MAX(ab,cd);
	 
			// Make sure to take into account the appropriate shading rate when calculating the dice size.
			uLen=uLen/ShadingRate;
			vLen=vLen/ShadingRate;
	 
			TqInt ucount=static_cast<TqInt>(MAX(1,uLen));
			TqInt vcount=static_cast<TqInt>(MAX(1,vLen));
	 
			TqFloat diu=1.0f/ucount;
			TqFloat div=1.0f/vcount;
			TqInt iu;
			TqInt iv;
	 
			CqMicroPolyGrid* pGrid=new CqMicroPolyGrid(ucount, vcount, &Surface());
	 
			if(bhasN)	pGrid->SetbShadingNormals(TqTrue);
	 
			// Need to copy a single Cq & Os if used, but not defined.
			if(USES(iUses,EnvVars_Cs) && !bhasCs)	pGrid->Cs().SetValue(pAttributes()->colColor());
			if(USES(iUses,EnvVars_Os) && !bhasOs)	pGrid->Os().SetValue(pAttributes()->colOpacity());
	 
			pGrid->Reset();
			for(iv=0; iv<=vcount; iv++)
			{
				for(iu=0; iu<=ucount; iu++)
				{
					pGrid->P()=BilinearEvaluate<CqVector4D>(PolyP(iA),PolyP(iB),PolyP(iD),PolyP(iC),iu*diu,iv*div);
					if(bhasN && USES(iUses,EnvVars_N))	pGrid->N()=BilinearEvaluate<CqVector3D>(PolyN(iA),PolyN(iB),PolyN(iD),PolyN(iC),iu*diu,iv*div);
					if(USES(iUses,EnvVars_s))
						pGrid->s()=BilinearEvaluate<TqFloat>(sA,sB,sD,sC,iu*diu,iv*div);
	 
					if(USES(iUses,EnvVars_t))
						pGrid->t()=BilinearEvaluate<TqFloat>(tA,tB,tD,tC,iu*diu,iv*div);
	 
					if(USES(iUses,EnvVars_u))
						pGrid->u()=BilinearEvaluate<TqFloat>(uA,uB,uD,uC,iu*diu,iv*div);
					
					if(USES(iUses,EnvVars_v))
						pGrid->v()=BilinearEvaluate<TqFloat>(vA,vB,vD,vC,iu*diu,iv*div);
	 
					if(bhasCs && USES(iUses,EnvVars_Cs))	pGrid->Cs()=BilinearEvaluate<CqColor>(PolyCs(iA),PolyCs(iB),PolyCs(iD),PolyCs(iC),iu*diu,iv*div);
					if(bhasOs && USES(iUses,EnvVars_Os))	pGrid->Os()=BilinearEvaluate<CqColor>(PolyOs(iA),PolyOs(iB),PolyOs(iD),PolyOs(iC),iu*diu,iv*div);
					pGrid->Advance();
				}
			}
			pGrid->Split(QGetRenderContext()->pImage());
		}*/
}


//---------------------------------------------------------------------
/** Split the polygon into bilinear patches.
 */

TqInt CqPolygonBase::Split( std::vector<CqBasicSurface*>& aSplits )
{
	CqVector3D	vecA, vecB, vecC, vecD;
	CqVector3D	vecNA, vecNB, vecNC, vecND;
	TqInt indexA, indexB, indexC, indexD;

	CqColor colSys = pAttributes() ->GetColorAttribute("System", "Color")[0];
	CqColor opaSys = pAttributes() ->GetColorAttribute("System", "Opacity")[0];

	TqBool	bhasN = bHasN();
	TqBool	bhass = bHass();
	TqBool	bhast = bHast();
	TqBool	bhasu = bHasu();
	TqBool	bhasv = bHasv();
	TqBool	bhasCs = bHasCs();
	TqBool	bhasOs = bHasOs();

	TqInt iUses = PolyUses();

	// We need to take into account Orientation here, even though most other
	// primitives leave it up to the CalcNormals function on the MPGrid, because we
	// are forcing N to be setup here, so clockwise nature is important.
	TqInt O = pAttributes() ->GetIntegerAttribute("System", "Orientation")[0];

	indexA = 0;
	indexB = 1;

	// Start by splitting the polygon into 4 point patches.
	vecA = PolyP( indexA );
	vecB = PolyP( indexB );

	// Get the normals, or calculate the facet normal if not specified.
	if ( bhasN )
	{
		vecNA = PolyN( indexA );
		vecNB = PolyN( indexB );
	}
	else
	{
		// Find two suitable vectors, and produce a geometric normal to use.
		TqInt i = 1;
		CqVector3D	vecN0, vecN1;
		while ( i < NumVertices() )
		{
			vecN0 = static_cast<CqVector3D>( PolyP( i ) ) - vecA;
			if ( vecN0.Magnitude() > FLT_EPSILON )
				break;
			i++;
		}
		i++;
		while ( i < NumVertices() )
		{
			vecN1 = static_cast<CqVector3D>( PolyP( i ) ) - vecA;
			if ( vecN1.Magnitude() > FLT_EPSILON && vecN1 != vecN0 )
				break;
			i++;
		}
		vecNA = vecN0 % vecN1;
		vecNA = (O==OrientationLH)? vecNA : -vecNA;
		vecNA.Unit();
		vecNB = vecNC = vecND = vecNA;
	}


	TqInt cNew = 0;
	TqInt i;
	for ( i = 2; i < NumVertices(); i += 2 )
	{
		indexC = indexD = i;
		vecC = vecD = PolyP( indexC );
		if ( NumVertices() > i + 1 ) 
		{
			indexD = i + 1;
			vecD = PolyP( indexD );
		}

		if ( bhasN )
		{
			vecNC = vecND = PolyN( indexC );
			if ( NumVertices() > i + 1 ) 
				vecND = PolyN( indexD );
		}
		else
			vecNC = vecND = vecNA;

		// Create bilinear patches
		CqSurfacePatchBilinear* pNew = new CqSurfacePatchBilinear();
		pNew->AddRef();
		pNew->SetSurfaceParameters( Surface() );

		pNew->P().SetSize( 4 ); 
		pNew->N().SetSize( 4 );
		pNew->P() [ 0 ] = vecA; 
		pNew->P() [ 1 ] = vecB; 
		pNew->P() [ 2 ] = vecD;	
		pNew->P() [ 3 ] = vecC;
		pNew->N() [ 0 ] = vecNA; 
		pNew->N() [ 1 ] = vecNB; 
		pNew->N() [ 2 ] = vecND; 
		pNew->N() [ 3 ] = vecNC;

		TqInt iUPA = PolyIndex(indexA);
		TqInt iUPB = PolyIndex(indexB);
		TqInt iUPC = PolyIndex(indexC);
		TqInt iUPD = PolyIndex(indexD);

		// Copy any user specified primitive variables.
		std::vector<CqParameter*>::iterator iUP;
		for( iUP = Surface().aUserParams().begin(); iUP != Surface().aUserParams().end(); iUP++ )
		{
			CqParameter* pNewUP = (*iUP)->Clone();
			pNewUP->Clear();
			pNewUP->SetSize(4);

			pNewUP->SetValue( (*iUP), 0, iUPA );
			pNewUP->SetValue( (*iUP), 1, iUPB );
			pNewUP->SetValue( (*iUP), 2, iUPD );
			pNewUP->SetValue( (*iUP), 3, iUPC );

			pNew->aUserParams().push_back(pNewUP);
		}

		aSplits.push_back( pNew );
		cNew++;

		// Move onto the next quad
		indexB = indexD;
		vecB = vecD;
		vecNB = vecND;
	}
	return ( cNew );
}


//---------------------------------------------------------------------
/** Default constructor.
 */

CqSurfacePolygon::CqSurfacePolygon( TqInt cVertices ) : CqSurface(),
		m_cVertices( cVertices )
{}


//---------------------------------------------------------------------
/** Copy constructor.
 */

CqSurfacePolygon::CqSurfacePolygon( const CqSurfacePolygon& From )
{
	*this = From;
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqSurfacePolygon::~CqSurfacePolygon()
{}


//---------------------------------------------------------------------
/** Check if a polygon is degenerate, i.e. all points collapse to the same or almost the same place.
 */

TqBool CqSurfacePolygon::CheckDegenerate() const
{
	// Check if all points are within a minute distance of each other.
	TqBool	fDegen = TqTrue;
	TqInt i;
	for ( i = 1; i < NumVertices(); i++ )
	{
		if ( ( PolyP( i ) - PolyP( i - 1 ) ).Magnitude() > FLT_EPSILON )
			fDegen = TqFalse;
	}
	return ( fDegen );
}

//---------------------------------------------------------------------
/** Assignment operator.
 */

CqSurfacePolygon& CqSurfacePolygon::operator=( const CqSurfacePolygon& From )
{
	CqSurface::operator=( From );
	return ( *this );
}



//---------------------------------------------------------------------
/** Transform the polygon by the specified matrix.
 */

void	CqSurfacePolygon::Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx )
{
	// Tansform the points by the specified matrix.
	TqInt i;
	for ( i = P().Size() - 1; i >= 0; i-- )
		P() [ i ] = matTx * P() [ i ];

	// Tansform the normals by the specified matrix.
	for ( i = N().Size() - 1; i >= 0; i-- )
		N() [ i ] = matITTx * N() [ i ];
}


//---------------------------------------------------------------------
/** Transfer x,y points to s,t and u,v if required.
 */

void CqSurfacePolygon::TransferDefaultSurfaceParameters()
{
	// If the shader needs s/t or u/v, and s/t is not specified, then at this point store the object space x,y coordinates.
	TqInt i;
	TqInt iUses = PolyUses();
	if ( USES( iUses, EnvVars_s ) && !bHass() )
	{
		AddPrimitiveVariable(  new CqParameterTypedVarying<TqFloat, type_float, TqFloat>("s") );
		s()->SetSize( NumVertices() );
		for ( i = 0; i < NumVertices(); i++ )
			(*s())[ i ] = P() [ i ].x();
	}

	if ( USES( iUses, EnvVars_t ) && !bHast() )
	{
		AddPrimitiveVariable(  new CqParameterTypedVarying<TqFloat, type_float, TqFloat>("t") );
		t()->SetSize( NumVertices() );
		for ( i = 0; i < NumVertices(); i++ )
			(*t())[ i ] = P() [ i ].y();
	}

	if ( USES( iUses, EnvVars_u ) )
	{
		AddPrimitiveVariable(  new CqParameterTypedVarying<TqFloat, type_float, TqFloat>("u") );
		u()->SetSize( NumVertices() );
		for ( i = 0; i < NumVertices(); i++ )
			(*u())[ i ] = P() [ i ].x();
	}

	if ( USES( iUses, EnvVars_v ) )
	{
		AddPrimitiveVariable(  new CqParameterTypedVarying<TqFloat, type_float, TqFloat>("v") );
		v()->SetSize( NumVertices() );
		for ( i = 0; i < NumVertices(); i++ )
			(*v())[ i ] = P() [ i ].y();
	}
}


//---------------------------------------------------------------------
/** Copy constructor.
 */

CqSurfacePointsPolygon::CqSurfacePointsPolygon( const CqSurfacePointsPolygon& From ) : m_pPoints( 0 )
{
	*this = From;
}


//---------------------------------------------------------------------
/** Assignment operator.
 */

CqSurfacePointsPolygon& CqSurfacePointsPolygon::operator=( const CqSurfacePointsPolygon& From )
{
	TqInt i;
	m_aIndices.resize( From.m_aIndices.size() );
	for ( i = From.m_aIndices.size() - 1; i >= 0; i-- )
		m_aIndices[ i ] = From.m_aIndices[ i ];

	// Store the old points array pointer, as we must reference first then
	// unreference to avoid accidental deletion if they are the same and we are the
	// last reference.
	CqPolygonPoints*	pOldPoints = m_pPoints;
	m_pPoints = From.m_pPoints;
	m_pPoints->AddRef();
	if ( pOldPoints ) pOldPoints->Release();

	return ( *this );
}




//---------------------------------------------------------------------
/** Transform the points by the specified matrix.
 */

void	CqPolygonPoints::Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx )
{
	if ( m_Transformed ) return ;

	// Tansform the points by the specified matrix.
	TqInt i;
	for ( i = P().Size() - 1; i >= 0; i-- )
		P() [ i ] = matTx * P() [ i ];

	// Tansform the normals by the specified matrix.
	for ( i = N().Size() - 1; i >= 0; i-- )
		N() [ i ] = matITTx * N() [ i ];

	m_Transformed = TqTrue;
}


//---------------------------------------------------------------------
/** Transfer x,y points to s,t and u,v if required.
 */

void CqPolygonPoints::TransferDefaultSurfaceParameters()
{
	// If the shader needs s/t or u/v, and s/t is not specified, then at this point store the object space x,y coordinates.
	TqInt i;
	TqInt iUses = Uses();
	if ( USES( iUses, EnvVars_s ) && !bHass() )
	{
		AddPrimitiveVariable(  new CqParameterTypedVarying<TqFloat, type_float, TqFloat>("s") );
		s()->SetSize( NumVertices() );
		for ( i = 0; i < NumVertices(); i++ )
			(*s())[ i ] = P() [ i ].x();
	}

	if ( USES( iUses, EnvVars_t ) && !bHast() )
	{
		AddPrimitiveVariable(  new CqParameterTypedVarying<TqFloat, type_float, TqFloat>("t") );
		t()->SetSize( NumVertices() );
		for ( i = 0; i < NumVertices(); i++ )
			(*t())[ i ] = P() [ i ].y();
	}

	if ( USES( iUses, EnvVars_u ) )
	{
		AddPrimitiveVariable(  new CqParameterTypedVarying<TqFloat, type_float, TqFloat>("u") );
		u()->SetSize( NumVertices() );
		for ( i = 0; i < NumVertices(); i++ )
			(*u())[ i ] = P() [ i ].x();
	}

	if ( USES( iUses, EnvVars_v ) )
	{
		AddPrimitiveVariable(  new CqParameterTypedVarying<TqFloat, type_float, TqFloat>("v") );
		v()->SetSize( NumVertices() );
		for ( i = 0; i < NumVertices(); i++ )
			(*v())[ i ] = P() [ i ].y();
	}
}



//---------------------------------------------------------------------
/** Get the bound of this GPrim.
 */

CqBound	CqMotionSurfacePointsPolygon::Bound() const
{
	CqMotionSurfacePointsPolygon * pthis = const_cast<CqMotionSurfacePointsPolygon*>( this );
	CqBound B( FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX );
	TqInt i;
	for ( i = 0; i < cTimes(); i++ )
	{
		pthis->m_CurrTimeIndex = i;
		B = B.Combine( CqPolygonBase::Bound() );
	}
	return ( B );
}


//---------------------------------------------------------------------
/** Dice this GPrim.
 */

CqMicroPolyGridBase* CqMotionSurfacePointsPolygon::Dice()
{
	CqMotionMicroPolyGrid * pGrid = new CqMotionMicroPolyGrid;
	TqInt i;
	for ( i = 0; i < cTimes(); i++ )
	{
		m_CurrTimeIndex = i;
		CqMicroPolyGridBase* pGrid2 = CqPolygonBase::Dice();
		pGrid->AddTimeSlot( Time( i ), pGrid2 );
	}
	return ( pGrid );
}


//---------------------------------------------------------------------
/** Split this GPrim into smaller GPrims.
 */

TqInt CqMotionSurfacePointsPolygon::Split( std::vector<CqBasicSurface*>& aSplits )
{
	std::vector<std::vector<CqBasicSurface*> > aaMotionSplits;
	aaMotionSplits.resize( cTimes() );
	TqInt cSplits = 0;
	TqInt i;
	for ( i = 0; i < cTimes(); i++ )
	{
		m_CurrTimeIndex = i;
		cSplits = CqPolygonBase::Split( aaMotionSplits[ i ] );
	}

	// Now build motion surfaces from the splits and pass them back.
	for ( i = 0; i < cSplits; i++ )
	{
		CqMotionSurface<CqBasicSurface*>* pNewMotion = new CqMotionSurface<CqBasicSurface*>( 0 );
		pNewMotion->AddRef();
		pNewMotion->m_fDiceable = TqTrue;
		pNewMotion->m_EyeSplitCount = m_EyeSplitCount;
		TqInt j;
		for ( j = 0; j < cTimes(); j++ )
			pNewMotion->AddTimeSlot( Time( j ), aaMotionSplits[ j ][ i ] );
		aSplits.push_back( pNewMotion );
	}
	return ( cSplits );
}


//---------------------------------------------------------------------
/** Determine whether this GPrim is diceable or not.
 * Only checks the GPrim at shutter time 0.
 */

TqBool CqMotionSurfacePointsPolygon::Diceable()
{
	TqBool f = GetMotionObject( Time( 0 ) ) ->Diceable();
	return ( f );
}


END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------
