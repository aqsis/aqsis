// Aqsis
// Copyright c 1997 - 2001, Paul C. Gregory
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
		\brief Implements CqPoints using small regular polygon (first try) This is more or less an experimentation with the parser. Later a micropolygon grid will be used to be more efficient to shade/render.
		\author M. Joron (joron@sympatico.ca)
*/ 
/*    References:
 *          [PIXA89]  Pixar, The RenderMan Interface, Version 3.2, 
 *                    Richmond, CA, September 1989.  
 *
 */

#include	<math.h>

#include	"aqsis.h"
#include	"points.h"
#include	"micropolygon.h"
#include	"imagebuffer.h"
#include	"polygon.h"

#include	"ri.h"

START_NAMESPACE( Aqsis )

#define NBR_SEGMENTS 6

//---------------------------------------------------------------------
/** Constructor.
 */

CqPoints::CqPoints(TqInt n, TqFloat *origins , TqFloat *sizes, TqFloat constantwidth )
{
	TqInt i,j;
	TqFloat angle = 0.0;
	
    m_pPolygons.resize(n);
	
	
	
	for (i=0; i<n;i++) 
	{
        CqSurfacePolygon *pSurface = new CqSurfacePolygon(NBR_SEGMENTS);
	
		pSurface->AddRef();
	

		angle = 0.0;
		pSurface->SetDefaultPrimitiveVariables();
		pSurface->SetSurfaceParameters( *this );
		pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<CqColor, type_color, CqColor>("Cs") );
		pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<CqColor, type_color, CqColor>("Os") );
		pSurface->AddPrimitiveVariable( new CqParameterTypedVertex<CqVector4D, type_hpoint, CqVector3D>("P",0) );
		pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>("s") );
		pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>("t") );
	    pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>("v") );
	    pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>("u") );
		
	
		pSurface->Cs()->SetSize( NBR_SEGMENTS );
		pSurface->Os()->SetSize( NBR_SEGMENTS );
		pSurface->P()->SetSize( NBR_SEGMENTS );
		pSurface->s()->SetSize( NBR_SEGMENTS );
		pSurface->t()->SetSize( NBR_SEGMENTS );
		pSurface->u()->SetSize( NBR_SEGMENTS );
		pSurface->v()->SetSize( NBR_SEGMENTS );
		
		


		for (j=0; j<NBR_SEGMENTS; j++)
		{
			TqFloat co,si;
			co = 0.5 * cos(angle);
			si = 0.5 * sin(angle);

			if (sizes) {
				co *= sizes[i];
				si *= sizes[i];
			} else {
				co *= constantwidth;
				si *= constantwidth;
			}
	
			(*pSurface->s())[j] = 0.5 * cos(angle) + 0.5;
			(*pSurface->t())[j] = 0.5 * sin(angle) + 0.5;
			(*pSurface->P())[j] = CqVector3D(
					co + origins[3*i], 
					si + origins[3*i+1], 
					origins[3*i+2]);
			angle += 2.0 * RI_PI/(float) NBR_SEGMENTS;
			
		}
		for (j=0; j<NBR_SEGMENTS; j++)
		{
			(*pSurface->Cs())[ j ] = m_pAttributes->GetColorAttribute("System", "Color")[0]; //m_pAttributes->colColor();
			(*pSurface->Os())[ j ] = m_pAttributes->GetColorAttribute("System", "Opacity")[0];//m_pAttributes->colOpacity();
		}
		
        m_pPolygons[i]  = pSurface;
	}
	return ;
}


//---------------------------------------------------------------------
/** Assignment operator.
 */

CqPoints&	CqPoints::operator=( const CqPoints& From )
{
	CqPoints::operator=( From );
	m_pPolygons = From.m_pPolygons;
	m_n = From.m_n;
	m_matTx = From.m_matTx;
	m_matITTx = From.m_matITTx;

	return ( *this );
}

//---------------------------------------------------------------------
/** Transform the quadric primitive by the specified matrix.
 */

void	CqPoints::Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx )
{
	m_matTx *= matTx;
	m_matITTx *= matITTx;
}


//---------------------------------------------------------------------
/** Dice the quadric into a grid of MPGs for rendering.
 */

CqMicroPolyGridBase* CqPoints::Dice()
{
	

	return ( NULL );

}


//---------------------------------------------------------------------
/** Determine whether the quadric is suitable for dicing.
 */

TqBool	CqPoints::Diceable()
{
	return ( TqFalse);
}

//---------------------------------------------------------------------
/** Get the geometric bound of this GPrim.
 */

CqBound	CqPoints::Bound() const
{
	CqVector3D vecMin( -1, -1, -1);
	CqVector3D vecMax( 1, 1, 1);


	CqBound	B( vecMin, vecMax );
	B.Transform( m_matTx );
	return ( B );
}

//---------------------------------------------------------------------
/** Split this GPrim into bicubic patches.
 */

TqInt CqPoints::Split( std::vector<CqBasicSurface*>& aSplits )
{


	return 0;
}


END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------


