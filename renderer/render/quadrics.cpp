
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


/** \file
		\brief Implements the standard RenderMan quadric primitive classes.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	<math.h>

#include	"aqsis.h"
#include	"quadrics.h"
#include	"micropolygon.h"
#include	"imagebuffer.h"
#include	"nurbs.h"
#include	"aqsismath.h"

#include	"ri.h"

START_NAMESPACE( Aqsis )

static bool IntersectLine( CqVector3D& P1, CqVector3D& T1, CqVector3D& P2, CqVector3D& T2, CqVector3D& P );
static void ProjectToLine( const CqVector3D& S, const CqVector3D& Trj, const CqVector3D& pnt, CqVector3D& p );

#define TOOLARGEQUADS 10000
static TqUlong RIH_P = CqString::hash("P");

CqQuadric::CqQuadric()
{
	m_uDiceSize = m_vDiceSize = 0;

	STATS_INC( GPR_quad );

}


//---------------------------------------------------------------------
/** Clone the data on this quadric to the (possibly derived) class
 *  passed in.
 */

void CqQuadric::CloneData( CqQuadric* clone ) const
{
	CqSurface::CloneData( clone );
	clone->m_matTx = m_matTx;
	clone->m_matITTx = m_matITTx;
	clone->m_uDiceSize = m_uDiceSize;
	clone->m_vDiceSize = m_vDiceSize;
}


//---------------------------------------------------------------------
/** Transform the quadric primitive by the specified matrix.
 */

void	CqQuadric::Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx, TqInt iTime )
{
	m_matTx.PreMultiply(matTx);
	m_matITTx.PreMultiply(matITTx);
}


//---------------------------------------------------------------------
/** Dice the quadric filling in as much information on the grid as possible.
 */

TqInt CqQuadric::DiceAll( CqMicroPolyGrid* pGrid )
{
	TqInt lUses = Uses();
	TqInt lDone = 0;

	CqVector3D	P, N;
	int v, u;

	CqParameterTyped<TqFloat, TqFloat>* ps = s();
	CqParameterTyped<TqFloat, TqFloat>* pt = t();
	CqParameterTyped<TqFloat, TqFloat>* pu = this->u();
	CqParameterTyped<TqFloat, TqFloat>* pv = this->v();
	CqParameterTyped<TqFloat, TqFloat>* pst = static_cast<CqParameterTyped<TqFloat, TqFloat>*>(FindUserParam( "st" ));

	TqFloat s0,s1,s2,s3;
	if( USES( lUses, EnvVars_s ) && NULL != pGrid->pVar(EnvVars_s) && bHasVar(EnvVars_s) )
	{
		if( pst )
		{
			s0 = pst->pValue( 0 )[0];
			s1 = pst->pValue( 1 )[0];
			s2 = pst->pValue( 2 )[0];
			s3 = pst->pValue( 3 )[0];
		}
		else if( ps )
		{
			s0 = ps->pValue( 0 )[0];
			s1 = ps->pValue( 1 )[0];
			s2 = ps->pValue( 2 )[0];
			s3 = ps->pValue( 3 )[0];
		}

		DONE( lDone, EnvVars_s );
	}

	TqFloat t0,t1,t2,t3;
	if( USES( lUses, EnvVars_t ) && NULL != pGrid->pVar(EnvVars_t) && bHasVar(EnvVars_t) )
	{
		if( pst )
		{
			t0 = pst->pValue( 0 )[1];
			t1 = pst->pValue( 1 )[1];
			t2 = pst->pValue( 2 )[1];
			t3 = pst->pValue( 3 )[1];
		}
		else if( pt )
		{
			t0 = pt->pValue( 0 )[0];
			t1 = pt->pValue( 1 )[0];
			t2 = pt->pValue( 2 )[0];
			t3 = pt->pValue( 3 )[0];
		}

		DONE( lDone, EnvVars_t );
	}

	TqFloat u0,u1,u2,u3;
	if( USES( lUses, EnvVars_u ) && NULL != pGrid->pVar(EnvVars_u) && bHasVar(EnvVars_u) )
	{
		u0 = pu->pValue( 0 )[0];
		u1 = pu->pValue( 1 )[0];
		u2 = pu->pValue( 2 )[0];
		u3 = pu->pValue( 3 )[0];

		DONE( lDone, EnvVars_u );
	}

	TqFloat v0,v1,v2,v3;
	if( USES( lUses, EnvVars_v ) && NULL != pGrid->pVar(EnvVars_v) && bHasVar(EnvVars_v) )
	{
		v0 = pv->pValue( 0 )[0];
		v1 = pv->pValue( 1 )[0];
		v2 = pv->pValue( 2 )[0];
		v3 = pv->pValue( 3 )[0];

		DONE( lDone, EnvVars_v );
	}

	/*    if( USES( lUses, EnvVars_P ) && NULL != pGrid->pVar(EnvVars_P) )
	        DONE( lDone, EnvVars_P );
	    if( USES( lUses, EnvVars_Ng ) && NULL != pGrid->pVar(EnvVars_Ng) )
	    {
	        DONE( lDone, EnvVars_Ng );
	        pGrid->SetbGeometricNormals( true );
	    }
	*/
	TqFloat du = 1.0 / uDiceSize();
	TqFloat dv = 1.0 / vDiceSize();
	for ( v = 0; v <= vDiceSize(); v++ )
	{
		TqFloat vf = v * dv;
		for ( u = 0; u <= uDiceSize(); u++ )
		{
			TqFloat uf = u * du;
			TqInt igrid = ( v * ( uDiceSize() + 1 ) ) + u;
			if( USES( lUses, EnvVars_P ) && NULL != pGrid->pVar(EnvVars_P) )
			{
				if( USES( lUses, EnvVars_Ng ) && NULL != pGrid->pVar(EnvVars_Ng) )
				{
					P = DicePoint( u, v, N );
					pGrid->pVar(EnvVars_P)->SetPoint( m_matTx * P, igrid );
					pGrid->pVar(EnvVars_Ng)->SetNormal( m_matITTx * N, igrid );

				}
				else
				{
					P = DicePoint( u, v );
					pGrid->pVar(EnvVars_P)->SetPoint( m_matTx * P, igrid );
				}
			}
			if( USES( lUses, EnvVars_s ) && NULL != pGrid->pVar(EnvVars_s) && bHasVar(EnvVars_s) )
			{
				TqFloat _s = BilinearEvaluate( s0, s1, s2, s3, uf, vf );
				pGrid->pVar(EnvVars_s)->SetFloat( _s, igrid );
			}
			if( USES( lUses, EnvVars_t ) && NULL != pGrid->pVar(EnvVars_t) && bHasVar(EnvVars_t) )
			{
				TqFloat _t = BilinearEvaluate( t0, t1, t2, t3, uf, vf );
				pGrid->pVar(EnvVars_t)->SetFloat( _t, igrid );
			}
			if( USES( lUses, EnvVars_u ) && NULL != pGrid->pVar(EnvVars_u) && bHasVar(EnvVars_u) )
			{
				TqFloat _u = BilinearEvaluate( u0, u1, u2, u3, uf, vf );
				pGrid->pVar(EnvVars_u)->SetFloat( _u, igrid );
			}
			if( USES( lUses, EnvVars_v ) && NULL != pGrid->pVar(EnvVars_v) && bHasVar(EnvVars_v) )
			{
				TqFloat _v = BilinearEvaluate( v0, v1, v2, v3, uf, vf );
				pGrid->pVar(EnvVars_v)->SetFloat( _v, igrid );
			}
		}
	}
	return( lDone );
}

//---------------------------------------------------------------------
/** Dice the quadric into a grid of MPGs for rendering.
 */

void CqQuadric::NaturalDice( CqParameter* pParameter, TqInt uDiceSize, TqInt vDiceSize, IqShaderData* pData )
{
	// Special case for "P", else normal bilinear dice for all others.

	TqUlong hash = CqString::hash(pData->strName().c_str());
	if ( hash == RIH_P )
	{
		CqVector3D	P;
		int v, u;
		for ( v = 0; v <= vDiceSize; v++ )
		{
			for ( u = 0; u <= uDiceSize; u++ )
			{
				TqInt igrid = ( v * ( uDiceSize + 1 ) ) + u;
				P = DicePoint( u, v );
				pData->SetPoint( m_matTx * P, igrid );
			}
		}
	}
	else
		CqSurface::NaturalDice( pParameter, uDiceSize, vDiceSize, pData );
}

//---------------------------------------------------------------------
/** Generate and store the geometric normals for this quadric.
 */

void CqQuadric::GenerateGeometricNormals( TqInt uDiceSize, TqInt vDiceSize, IqShaderData* pNormals )
{
	int v, u;
	CqVector3D	N;
	for ( v = 0; v <= vDiceSize; v++ )
	{
		for ( u = 0; u <= uDiceSize; u++ )
		{
			TqInt igrid = ( v * ( uDiceSize + 1 ) ) + u;
			DicePoint( u, v, N );
			bool CSO = pTransform()->GetHandedness(pTransform()->Time(0));
			bool O = pAttributes() ->GetIntegerAttribute( "System", "Orientation" ) [ 0 ] != 0;
			N = ( (O && CSO) || (!O && !CSO) ) ? N : -N;
			pNormals->SetNormal( m_matITTx * N, igrid );
		}
	}
}


//---------------------------------------------------------------------
/** Determine whether the quadric is suitable for dicing.
 */


bool	CqQuadric::Diceable()
{
	// If the cull check showed that the primitive cannot be diced due to crossing the e and hither planes,
	// then we can return immediately.
	if ( !m_fDiceable )
		return ( false );

	TqUlong toomuch = EstimateGridSize();

	m_SplitDir = ( m_uDiceSize > m_vDiceSize ) ? SplitDir_U : SplitDir_V;

	TqFloat gs = 16.0f;
	const TqFloat* poptGridSize = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "SqrtGridSize" );
	if( NULL != poptGridSize )
		gs = poptGridSize[0];

	if (toomuch > TOOLARGEQUADS)
		return false;

	if ( m_uDiceSize > gs)
		return false;
	if ( m_vDiceSize > gs)
		return false;


	return ( true );

}


//---------------------------------------------------------------------
/** Estimate the size of the micropolygrid required to dice this GPrim to a suitable shading rate.
 */

TqUlong CqQuadric::EstimateGridSize()
{
	TqFloat maxusize, maxvsize;
	maxusize = maxvsize = 0;

	CqMatrix matTx = QGetRenderContext() ->matSpaceToSpace( "camera", "raster", NULL, NULL, QGetRenderContext()->Time() ) * m_matTx;

	m_uDiceSize = m_vDiceSize = ESTIMATEGRIDSIZE;

	TqFloat udist, vdist;
	CqVector3D p, pum1, pvm1[ ESTIMATEGRIDSIZE ];

	int v, u;
	for ( v = 0; v <= ESTIMATEGRIDSIZE; v++ )
	{
		for ( u = 0; u <= ESTIMATEGRIDSIZE; u++ )
		{
			p = DicePoint( u, v );
			p = matTx * p;
			// If we are on row two or above, calculate the mp size.
			if ( v >= 1 && u >= 1 )
			{
				udist = ( p.x() - pum1.x() ) * ( p.x() - pum1.x() ) +
				        ( p.y() - pum1.y() ) * ( p.y() - pum1.y() );
				vdist = ( pvm1[ u - 1 ].x() - pum1.x() ) * ( pvm1[ u - 1 ].x() - pum1.x() ) +
				        ( pvm1[ u - 1 ].y() - pum1.y() ) * ( pvm1[ u - 1 ].y() - pum1.y() );

				maxusize = MAX( maxusize, udist );
				maxvsize = MAX( maxvsize, vdist );
			}
			if ( u >= 1 )
				pvm1[ u - 1 ] = pum1;
			pum1 = p;
		}
	}
	maxusize = sqrt( maxusize );
	maxvsize = sqrt( maxvsize );

	TqFloat ShadingRate = pAttributes() ->GetFloatAttribute( "System", "ShadingRateSqrt" ) [ 0 ];

	m_uDiceSize = CEIL( ESTIMATEGRIDSIZE * maxusize / ( ShadingRate ) );
	m_vDiceSize = CEIL( ESTIMATEGRIDSIZE * maxvsize / ( ShadingRate ) );

	// Ensure power of 2 to avoid cracking
	const TqInt *binary = pAttributes() ->GetIntegerAttribute( "dice", "binary" );
	if ( binary && *binary)
	{
		m_uDiceSize = ceilPow2( m_uDiceSize );
		m_vDiceSize = ceilPow2( m_vDiceSize );
	}

	return  (TqUlong) m_uDiceSize * m_vDiceSize;
}


//---------------------------------------------------------------------
/** Constructor.
 */

CqSphere::CqSphere( TqFloat radius, TqFloat zmin, TqFloat zmax, TqFloat thetamin, TqFloat thetamax ) :
		m_Radius( radius ),
		m_ThetaMin( thetamin ),
		m_ThetaMax( thetamax )
{
	// Sanity check the values, while ensuring we keep the same signs.
	TqFloat frad = fabs(m_Radius);
	if( fabs(zmin) > frad )
		zmin = frad*(zmin<0)?-1:1;
	if( fabs(zmin) > frad )
		zmin = frad*(zmin<0)?-1:1;

	m_PhiMin = asin( zmin / m_Radius );
	m_PhiMax = asin( zmax / m_Radius );
}


//---------------------------------------------------------------------
/** Create a clone of this sphere
 */

CqSurface*	CqSphere::Clone( ) const
{
	CqSphere* clone = new CqSphere();
	CqQuadric::CloneData( clone );
	clone->m_Radius = m_Radius;
	clone->m_PhiMin = m_PhiMin;
	clone->m_PhiMax = m_PhiMax;
	clone->m_ThetaMin = m_ThetaMin;
	clone->m_ThetaMax = m_ThetaMax;

	return ( clone );
}

//---------------------------------------------------------------------
/** Get the geometric bound of this GPrim.
 */

CqBound	CqSphere::Bound() const
{
	std::vector<CqVector3D> curve;
	CqVector3D vA( 0, 0, 0 ), vB( 1, 0, 0 ), vC( 0, 0, 1 );
	Circle( vA, vB, vC, m_Radius, m_PhiMin, m_PhiMax, curve );

	CqMatrix matRot( RAD ( m_ThetaMin ), vC );
	for ( std::vector<CqVector3D>::iterator i = curve.begin(); i != curve.end(); i++ )
		*i = matRot * ( *i );

	CqBound	B( RevolveForBound( curve, vA, vC, RAD( m_ThetaMax - m_ThetaMin ) ) );
	B.Transform( m_matTx );

	return ( AdjustBoundForTransformationMotion( B ) );
}


//---------------------------------------------------------------------
/** Split this GPrim into a NURBS surface. Temp implementation, should split into smalled quadrics.
 */

TqInt CqSphere::PreSubdivide( std::vector<boost::shared_ptr<CqSurface> >& aSplits, bool u )
{
	TqFloat phicent = ( m_PhiMin + m_PhiMax ) * 0.5;
	TqFloat arccent = ( m_ThetaMin + m_ThetaMax ) * 0.5;

	boost::shared_ptr<CqSphere> pNew1( new CqSphere() );
	boost::shared_ptr<CqSphere> pNew2( new CqSphere() );
	pNew1->m_matTx =pNew2->m_matTx = m_matTx;
	pNew1->m_matITTx = pNew2->m_matITTx = m_matITTx;
	pNew1->m_fDiceable = pNew2->m_fDiceable = m_fDiceable;
	pNew1->m_Radius = m_Radius;
	pNew2->m_Radius = m_Radius;
	pNew1->m_fDiceable = pNew2->m_fDiceable = m_fDiceable;
	pNew1->m_EyeSplitCount = pNew2->m_EyeSplitCount = m_EyeSplitCount;
	pNew1->SetSurfaceParameters( *this );
	pNew2->SetSurfaceParameters( *this );
	pNew1->m_fDiscard = pNew2->m_fDiscard = m_fDiscard;

	if ( u )
	{
		pNew1->m_ThetaMax = arccent;
		pNew2->m_ThetaMin = arccent;
		pNew1->m_ThetaMin = m_ThetaMin;
		pNew2->m_ThetaMax = m_ThetaMax;
		pNew1->m_PhiMin = pNew2->m_PhiMin = m_PhiMin;
		pNew1->m_PhiMax = pNew2->m_PhiMax = m_PhiMax;
	}
	else
	{
		pNew1->m_PhiMax = phicent;
		pNew2->m_PhiMin = phicent;
		pNew1->m_PhiMin = m_PhiMin;
		pNew2->m_PhiMax = m_PhiMax;
		pNew1->m_ThetaMin = pNew2->m_ThetaMin = m_ThetaMin;
		pNew1->m_ThetaMax = pNew2->m_ThetaMax = m_ThetaMax;
	}

	aSplits.push_back( pNew1 );
	aSplits.push_back( pNew2 );

	return ( 2 );
}



//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 */

CqVector3D CqSphere::DicePoint( TqInt u, TqInt v )
{
	TqFloat phi = m_PhiMin + ( ( TqFloat ) v * ( m_PhiMax - m_PhiMin ) ) / m_vDiceSize;

	TqFloat cosphi = cos( phi );
	TqFloat theta = RAD( m_ThetaMin + ( ( TqFloat ) u * ( m_ThetaMax - m_ThetaMin ) ) / m_uDiceSize );

	return ( CqVector3D( ( m_Radius * cos( theta ) * cosphi ), ( m_Radius * sin( theta ) * cosphi ), ( m_Radius * sin( phi ) ) ) );
}


//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 * \param Normal Storage for the surface normal at that point.
 */

CqVector3D CqSphere::DicePoint( TqInt u, TqInt v, CqVector3D& Normal )
{
	CqVector3D	p( DicePoint( u, v ) );
	Normal = p;
	Normal.Unit();
	return ( p );
}


//---------------------------------------------------------------------
/** Constructor.
 */

CqCone::CqCone( TqFloat height, TqFloat radius, TqFloat thetamin, TqFloat thetamax, TqFloat vmin, TqFloat vmax ) :
		m_Height( height ),
		m_Radius( radius ),
		m_vMin( vmin ),
		m_vMax( vmax ),
		m_ThetaMin( thetamin ),
		m_ThetaMax( thetamax )
{}


//---------------------------------------------------------------------
/** Create a clone of this cone
 */

CqSurface*	CqCone::Clone( ) const
{
	CqCone* clone = new CqCone();
	CqQuadric::CloneData( clone );
	clone->m_Height = m_Height;
	clone->m_Radius = m_Radius;
	clone->m_vMin = m_vMin;
	clone->m_vMax = m_vMax;
	clone->m_ThetaMin = m_ThetaMin;
	clone->m_ThetaMax = m_ThetaMax;

	return ( clone );
}



//---------------------------------------------------------------------
/** Get the geometric bound of this GPrim.
 */

CqBound	CqCone::Bound() const
{
	std::vector<CqVector3D> curve;
	TqFloat zmin = m_vMin * m_Height;
	TqFloat zmax = m_vMax * m_Height;
	CqVector3D vA( m_Radius, 0, zmin ), vB( 0, 0, zmax ), vC( 0, 0, 0 ), vD( 0, 0, 1 );
	curve.push_back( vA );
	curve.push_back( vB );
	CqMatrix matRot( RAD ( m_ThetaMin ), vD );
	for ( std::vector<CqVector3D>::iterator i = curve.begin(); i != curve.end(); i++ )
		*i = matRot * ( *i );
	CqBound	B( RevolveForBound( curve, vC, vD, RAD( m_ThetaMax - m_ThetaMin ) ) );
	B.Transform( m_matTx );

	return ( AdjustBoundForTransformationMotion( B ) );
}


//---------------------------------------------------------------------
/** Split this GPrim into a NURBS surface. Temp implementation, should split into smalled quadrics.
 */

TqInt CqCone::PreSubdivide( std::vector<boost::shared_ptr<CqSurface> >& aSplits, bool u )
{
	TqFloat vcent = ( m_vMin + m_vMax ) * 0.5;
	TqFloat arccent = ( m_ThetaMin + m_ThetaMax ) * 0.5;
	//TqFloat rcent=m_RMax*sqrt(zcent/m_ZMax);

	boost::shared_ptr<CqCone> pNew1( new CqCone() );
	boost::shared_ptr<CqCone> pNew2( new CqCone() );
	pNew1->m_matTx =pNew2->m_matTx = m_matTx;
	pNew1->m_matITTx = pNew2->m_matITTx = m_matITTx;
	pNew1->m_fDiceable = pNew2->m_fDiceable = m_fDiceable;
	pNew1->m_Height = pNew2->m_Height = m_Height;
	pNew1->m_Radius = pNew2->m_Radius = m_Radius;
	pNew1->m_EyeSplitCount = pNew2->m_EyeSplitCount = m_EyeSplitCount;
	pNew1->SetSurfaceParameters( *this );
	pNew2->SetSurfaceParameters( *this );

	if ( u )
	{
		pNew1->m_ThetaMax = arccent;
		pNew2->m_ThetaMin = arccent;
		pNew1->m_ThetaMin = m_ThetaMin;
		pNew2->m_ThetaMax = m_ThetaMax;
		pNew1->m_vMin = pNew2->m_vMin = m_vMin;
		pNew1->m_vMax = pNew2->m_vMax = m_vMax;
	}
	else
	{
		pNew1->m_vMax = vcent;
		pNew2->m_vMin = vcent;
		pNew1->m_vMin = m_vMin;
		pNew2->m_vMax = m_vMax;
		pNew1->m_ThetaMin = pNew2->m_ThetaMin = m_ThetaMin;
		pNew1->m_ThetaMax = pNew2->m_ThetaMax = m_ThetaMax;
	}

	aSplits.push_back( pNew1 );
	aSplits.push_back( pNew2 );

	return ( 2 );
}



//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 */

CqVector3D CqCone::DicePoint( TqInt u, TqInt v )
{
	TqFloat theta = RAD( m_ThetaMin + ( ( TqFloat ) u * ( m_ThetaMax - m_ThetaMin ) ) / m_uDiceSize );

	TqFloat zmin = m_vMin * m_Height;
	TqFloat zmax = m_vMax * m_Height;
	TqFloat z = zmin + ( ( TqFloat ) v * ( zmax - zmin ) ) / m_vDiceSize;
	TqFloat vv = m_vMin + ( ( TqFloat ) v * ( m_vMax - m_vMin ) ) / m_vDiceSize;
	TqFloat r = m_Radius * ( 1.0 - vv );

	return ( CqVector3D( r * cos( theta ), r * sin( theta ), z ) );
}


//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 * \param Normal Storage for the surface normal at that point.
 */

CqVector3D CqCone::DicePoint( TqInt u, TqInt v, CqVector3D& Normal )
{
	TqFloat theta = RAD( m_ThetaMin + ( ( TqFloat ) u * ( m_ThetaMax - m_ThetaMin ) ) / m_uDiceSize );

	TqFloat zmin = m_vMin * m_Height;
	TqFloat zmax = m_vMax * m_Height;
	TqFloat z = zmin + ( ( TqFloat ) v * ( zmax - zmin ) ) / m_vDiceSize;
	TqFloat vv = m_vMin + ( ( TqFloat ) v * ( m_vMax - m_vMin ) ) / m_vDiceSize;
	TqFloat r = m_Radius * ( 1.0 - vv );

	TqFloat cos_theta = cos( theta );
	TqFloat sin_theta = sin( theta );

	TqFloat coneLength = sqrt( m_Height * m_Height + m_Radius * m_Radius );
	TqFloat xN = m_Height / coneLength;
	Normal.x( xN * cos_theta );
	Normal.y( xN * sin_theta );
	Normal.z( m_Radius / coneLength );

	return ( CqVector3D( r * cos_theta, r * sin_theta, z ) );
}


//---------------------------------------------------------------------
/** Constructor.
 */

CqCylinder::CqCylinder( TqFloat radius, TqFloat zmin, TqFloat zmax, TqFloat thetamin, TqFloat thetamax ) :
		m_Radius( radius ),
		m_ZMin( zmin ),
		m_ZMax( zmax ),
		m_ThetaMin( thetamin ),
		m_ThetaMax( thetamax )
{}


//---------------------------------------------------------------------
/** Create a clone of this cylinder.
 */

CqSurface*	CqCylinder::Clone() const
{
	CqCylinder* clone = new CqCylinder();
	CqQuadric::CloneData( clone );
	clone->m_Radius = m_Radius;
	clone->m_ZMin = m_ZMin;
	clone->m_ZMax = m_ZMax;
	clone->m_ThetaMin = m_ThetaMin;
	clone->m_ThetaMax = m_ThetaMax;

	return ( clone );
}



//---------------------------------------------------------------------
/** Get the geometric bound of this GPrim.
 */

CqBound	CqCylinder::Bound() const
{
	std::vector<CqVector3D> curve;
	CqVector3D vA( m_Radius, 0, m_ZMin ), vB( m_Radius, 0, m_ZMax ), vC( 0, 0, 0 ), vD( 0, 0, 1 );
	curve.push_back( vA );
	curve.push_back( vB );
	CqMatrix matRot( RAD ( m_ThetaMin ), vD );
	for ( std::vector<CqVector3D>::iterator i = curve.begin(); i != curve.end(); i++ )
		*i = matRot * ( *i );
	CqBound	B( RevolveForBound( curve, vC, vD, RAD( m_ThetaMax - m_ThetaMin ) ) );
	B.Transform( m_matTx );

	return ( AdjustBoundForTransformationMotion( B ) );
}


//---------------------------------------------------------------------
/** Split this GPrim into a NURBS surface. Temp implementation, should split into smalled quadrics.
 */

TqInt CqCylinder::PreSubdivide( std::vector<boost::shared_ptr<CqSurface> >& aSplits, bool u )
{
	TqFloat zcent = ( m_ZMin + m_ZMax ) * 0.5;
	TqFloat arccent = ( m_ThetaMin + m_ThetaMax ) * 0.5;

	boost::shared_ptr<CqCylinder> pNew1( new CqCylinder() );
	boost::shared_ptr<CqCylinder> pNew2( new CqCylinder() );
	pNew1->m_matTx =pNew2->m_matTx = m_matTx;
	pNew1->m_matITTx = pNew2->m_matITTx = m_matITTx;
	pNew1->m_fDiceable = pNew2->m_fDiceable = m_fDiceable;
	pNew1->m_Radius = pNew2->m_Radius = m_Radius;
	pNew1->m_EyeSplitCount = pNew2->m_EyeSplitCount = m_EyeSplitCount;
	pNew1->SetSurfaceParameters( *this );
	pNew2->SetSurfaceParameters( *this );

	if ( u )
	{
		pNew1->m_ThetaMax = arccent;
		pNew2->m_ThetaMin = arccent;
		pNew1->m_ThetaMin = m_ThetaMin;
		pNew2->m_ThetaMax = m_ThetaMax;
		pNew1->m_ZMin = pNew2->m_ZMin = m_ZMin;
		pNew1->m_ZMax = pNew2->m_ZMax = m_ZMax;
	}
	else
	{
		pNew1->m_ZMax = zcent;
		pNew2->m_ZMin = zcent;
		pNew1->m_ZMin = m_ZMin;
		pNew2->m_ZMax = m_ZMax;
		pNew1->m_ThetaMin = pNew2->m_ThetaMin = m_ThetaMin;
		pNew1->m_ThetaMax = pNew2->m_ThetaMax = m_ThetaMax;
	}

	aSplits.push_back( pNew1 );
	aSplits.push_back( pNew2 );

	return ( 2 );
}



//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 */

CqVector3D CqCylinder::DicePoint( TqInt u, TqInt v )
{
	TqFloat theta = RAD( m_ThetaMin + ( ( m_ThetaMax - m_ThetaMin ) * ( TqFloat ) u ) / m_uDiceSize );

	TqFloat vz = m_ZMin + ( ( TqFloat ) v * ( m_ZMax - m_ZMin ) ) / m_vDiceSize;
	return ( CqVector3D( m_Radius * cos( theta ), m_Radius * sin( theta ), vz ) );
}


//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 * \param Normal Storage for the surface normal at that point.
 */

CqVector3D CqCylinder::DicePoint( TqInt u, TqInt v, CqVector3D& Normal )
{
	CqVector3D	p( DicePoint( u, v ) );
	Normal = p;
	Normal.z( 0 );
	Normal.Unit();

	return ( p );
}


//---------------------------------------------------------------------
/** Constructor.
 */

CqHyperboloid::CqHyperboloid( )
{
	m_Point1 = CqVector3D( 0.0f, 0.0f, 0.0f );
	m_Point2 = CqVector3D( 0.0f, 0.0f, 1.0f );
	m_ThetaMin = 0.0f;
	m_ThetaMax = 1.0f;
}

//---------------------------------------------------------------------
/** Constructor.
 */

CqHyperboloid::CqHyperboloid( CqVector3D& point1, CqVector3D& point2, TqFloat thetamin, TqFloat thetamax ) :
		m_Point1( point1 ),
		m_Point2( point2 ),
		m_ThetaMin( thetamin ),
		m_ThetaMax( thetamax )
{}


//---------------------------------------------------------------------
/** Clone a copy of this hyperboloid.
 */

CqSurface*	CqHyperboloid::Clone() const
{
	CqHyperboloid* clone = new CqHyperboloid();
	CqQuadric::CloneData( clone );
	clone->m_Point1 = m_Point1;
	clone->m_Point2 = m_Point2;
	clone->m_ThetaMin = m_ThetaMin;
	clone->m_ThetaMax = m_ThetaMax;

	return ( clone );
}


//---------------------------------------------------------------------
/** Get the geometric bound of this GPrim.
 */

CqBound	CqHyperboloid::Bound() const
{
	std::vector<CqVector3D> curve;
	curve.push_back( m_Point1 );
	curve.push_back( m_Point2 );
	CqVector3D vA( 0, 0, 0 ), vB( 0, 0, 1 );
	CqMatrix matRot( RAD ( m_ThetaMin ), vB );
	for ( std::vector<CqVector3D>::iterator i = curve.begin(); i != curve.end(); i++ )
		*i = matRot * ( *i );
	CqBound	B( RevolveForBound( curve, vA, vB, RAD( m_ThetaMax - m_ThetaMin ) ) );
	B.Transform( m_matTx );

	return ( AdjustBoundForTransformationMotion( B ) );
}


//---------------------------------------------------------------------
/** Split this GPrim into a NURBS surface. Temp implementation, should split into smalled quadrics.
 */

TqInt CqHyperboloid::PreSubdivide( std::vector<boost::shared_ptr<CqSurface> >& aSplits, bool u )
{
	TqFloat arccent = ( m_ThetaMin + m_ThetaMax ) * 0.5;
	CqVector3D midpoint = ( m_Point1 + m_Point2 ) / 2.0;

	boost::shared_ptr<CqHyperboloid> pNew1( new CqHyperboloid() );
	boost::shared_ptr<CqHyperboloid> pNew2( new CqHyperboloid() );
	pNew1->m_matTx =pNew2->m_matTx = m_matTx;
	pNew1->m_matITTx = pNew2->m_matITTx = m_matITTx;
	pNew1->m_fDiceable = pNew2->m_fDiceable = m_fDiceable;
	pNew1->m_EyeSplitCount = pNew2->m_EyeSplitCount = m_EyeSplitCount;
	pNew1->SetSurfaceParameters( *this );
	pNew2->SetSurfaceParameters( *this );

	if ( u )
	{
		pNew1->m_ThetaMax = arccent;
		pNew2->m_ThetaMin = arccent;
		pNew1->m_ThetaMin = m_ThetaMin;
		pNew2->m_ThetaMax = m_ThetaMax;
		pNew1->m_Point1 = pNew2->m_Point1 = m_Point1;
		pNew1->m_Point2 = pNew2->m_Point2 = m_Point2;
	}
	else
	{
		pNew1->m_Point2 = midpoint;
		pNew2->m_Point1 = midpoint;
		pNew1->m_Point1 = m_Point1;
		pNew2->m_Point2 = m_Point2;
		pNew1->m_ThetaMin = pNew2->m_ThetaMin = m_ThetaMin;
		pNew1->m_ThetaMax = pNew2->m_ThetaMax = m_ThetaMax;
	}

	aSplits.push_back( pNew1 );
	aSplits.push_back( pNew2 );

	return ( 2 );
}



//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 */

CqVector3D CqHyperboloid::DicePoint( TqInt u, TqInt v )
{
	TqFloat theta = RAD( m_ThetaMin + ( ( TqFloat ) u * ( m_ThetaMax - m_ThetaMin ) ) / m_uDiceSize );

	CqVector3D p;
	TqFloat vv = static_cast<TqFloat>( v ) / m_vDiceSize;
	p = m_Point1 * ( 1.0 - vv ) + m_Point2 * vv;

	return ( CqVector3D( p.x() * cos( theta ) - p.y() * sin( theta ), p.x() * sin( theta ) + p.y() * cos( theta ), p.z() ) );
}


//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 * \param Normal Storage for the surface normal at that point.
 */

CqVector3D CqHyperboloid::DicePoint( TqInt u, TqInt v, CqVector3D& Normal )
{

	TqFloat theta = RAD( m_ThetaMin + ( ( TqFloat ) u * ( m_ThetaMax - m_ThetaMin ) ) / m_uDiceSize );
	TqFloat sin_theta = sin( theta );
	TqFloat cos_theta = cos( theta );

	CqVector3D p;
	TqFloat vv = static_cast<TqFloat>( v ) / m_vDiceSize;
	p = m_Point1 * ( 1.0 - vv ) + m_Point2 * vv;

	// Calculate the normal vector - this is a bit tortuous, and uses the general
	//  formula for the normal to a surface that is specified by two parametric
	//  parameters.

	// Calculate a vector, a, of derivatives of coordinates w.r.t. u
	TqFloat dxdu = -p.x() * m_ThetaMax * sin_theta - p.y() * m_ThetaMax * cos_theta;
	TqFloat dydu =  p.x() * m_ThetaMax * cos_theta - p.y() * m_ThetaMax * sin_theta;
	TqFloat dzdu = 0.0;
	CqVector3D a(dxdu, dydu, dzdu);

	// Calculate a vector, b, of derivatives of coordinates w.r.t. v
	CqVector3D p2p1 = m_Point2 - m_Point1;
	TqFloat dxdv = p2p1.x() * cos_theta  -  p2p1.y() * sin_theta;
	TqFloat dydv = p2p1.x() * sin_theta  +  p2p1.y() * cos_theta;
	TqFloat dzdv = p2p1.z();
	CqVector3D b(dxdv, dydv, dzdv);

	// The normal vector points in the direction of: a x b
	Normal = a % b;
	Normal.Unit();

	// Return the point on the surface.
	return ( CqVector3D( p.x() * cos_theta - p.y() * sin_theta, p.x() * sin_theta + p.y() * cos_theta, p.z() ) );
}


//---------------------------------------------------------------------
/** Constructor.
 */

CqParaboloid::CqParaboloid( TqFloat rmax, TqFloat zmin, TqFloat zmax, TqFloat thetamin, TqFloat thetamax ) :
		m_RMax( rmax ),
		m_ZMin( zmin ),
		m_ZMax( zmax ),
		m_ThetaMin( thetamin ),
		m_ThetaMax( thetamax )
{}


//---------------------------------------------------------------------
/** Create a clone of this paraboloid.
 */

CqSurface*	CqParaboloid::Clone() const
{
	CqParaboloid* clone = new CqParaboloid();
	CqQuadric::CloneData( clone );
	clone->m_RMax = m_RMax;
	clone->m_ZMin = m_ZMin;
	clone->m_ZMax = m_ZMax;
	clone->m_ThetaMin = m_ThetaMin;
	clone->m_ThetaMax = m_ThetaMax;

	return ( clone );
}



//---------------------------------------------------------------------
/** Get the geometric bound of this GPrim.
 */

CqBound	CqParaboloid::Bound() const
{
	/*	TqFloat xminang,yminang,xmaxang,ymaxang;
		xminang=yminang=MIN(m_ThetaMin,m_ThetaMax);
		xmaxang=ymaxang=MAX(m_ThetaMin,m_ThetaMax);
	 
	 
		// If start and end in same segement, just use the points.
		if(static_cast<TqInt>(m_ThetaMin/90)!=static_cast<TqInt>(m_ThetaMax/90))
		{
			if(yminang<90 && ymaxang>90)	yminang=90;
			if(yminang<270 && ymaxang>270)	ymaxang=270;
			if(xminang<180 && xmaxang>180)	xmaxang=180;
		}*/

	TqFloat x1 = m_RMax * cos( RAD( 0 ) );
	TqFloat x2 = m_RMax * cos( RAD( 180 ) );
	TqFloat y1 = m_RMax * sin( RAD( 90 ) );
	TqFloat y2 = m_RMax * sin( RAD( 270 ) );

	CqVector3D vecMin( MIN( x1, x2 ), MIN( y1, y2 ), MIN( m_ZMin, m_ZMax ) );
	CqVector3D vecMax( MAX( x1, x2 ), MAX( y1, y2 ), MAX( m_ZMin, m_ZMax ) );

	CqBound	B( vecMin, vecMax );
	B.Transform( m_matTx );

	return ( AdjustBoundForTransformationMotion( B ) );
}


//---------------------------------------------------------------------
/** Split this GPrim into smaller quadrics.
 */

TqInt CqParaboloid::PreSubdivide( std::vector<boost::shared_ptr<CqSurface> >& aSplits, bool u )
{
	TqFloat zcent = ( m_ZMin + m_ZMax ) * 0.5;
	TqFloat arccent = ( m_ThetaMin + m_ThetaMax ) * 0.5;
	TqFloat rcent = m_RMax * sqrt( zcent / m_ZMax );

	boost::shared_ptr<CqParaboloid> pNew1( new CqParaboloid() );
	boost::shared_ptr<CqParaboloid> pNew2( new CqParaboloid() );
	pNew1->m_matTx =pNew2->m_matTx = m_matTx;
	pNew1->m_matITTx = pNew2->m_matITTx = m_matITTx;
	pNew1->m_fDiceable = pNew2->m_fDiceable = m_fDiceable;
	pNew1->m_EyeSplitCount = pNew2->m_EyeSplitCount = m_EyeSplitCount;
	pNew1->SetSurfaceParameters( *this );
	pNew2->SetSurfaceParameters( *this );

	if ( u )
	{
		pNew1->m_ThetaMax = arccent;
		pNew2->m_ThetaMin = arccent;
		pNew1->m_ThetaMin = m_ThetaMin;
		pNew2->m_ThetaMax = m_ThetaMax;
		pNew1->m_RMax = pNew2->m_RMax = m_RMax;
		pNew1->m_ZMin = pNew2->m_ZMin = m_ZMin;
		pNew1->m_ZMax = pNew2->m_ZMax = m_ZMax;
	}
	else
	{
		pNew1->m_ZMax = zcent;
		pNew1->m_RMax = rcent;
		pNew2->m_ZMin = zcent;
		pNew1->m_ZMin = m_ZMin;
		pNew2->m_ZMax = m_ZMax;
		pNew2->m_RMax = m_RMax;
		pNew1->m_ThetaMin = pNew2->m_ThetaMin = m_ThetaMin;
		pNew1->m_ThetaMax = pNew2->m_ThetaMax = m_ThetaMax;
	}

	aSplits.push_back( pNew1 );
	aSplits.push_back( pNew2 );

	return ( 2 );
}



//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 */

CqVector3D CqParaboloid::DicePoint( TqInt u, TqInt v )
{
	TqFloat theta = RAD( m_ThetaMin + ( ( m_ThetaMax - m_ThetaMin ) * ( TqFloat ) u ) / m_uDiceSize );

	TqFloat z = m_ZMin + ( ( TqFloat ) v * ( m_ZMax - m_ZMin ) ) / m_vDiceSize;
	TqFloat r = m_RMax * sqrt( z / m_ZMax );
	return ( CqVector3D( r * cos( theta ), r * sin( theta ), z ) );
}


//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 * \param Normal Storage for the surface normal at that point.
 */

CqVector3D CqParaboloid::DicePoint( TqInt u, TqInt v, CqVector3D& Normal )
{
	TqFloat theta = RAD( m_ThetaMin + ( ( m_ThetaMax - m_ThetaMin ) * ( TqFloat ) u ) / m_uDiceSize );
	TqFloat sin_theta = sin( theta );
	TqFloat cos_theta = cos( theta );

	TqFloat z = m_ZMin + ( ( TqFloat ) v * ( m_ZMax - m_ZMin ) ) / m_vDiceSize;
	TqFloat r = m_RMax * sqrt( z / m_ZMax );

	TqFloat dzdr = r * 2.0 * m_ZMax / ( m_RMax * m_RMax );
	TqFloat normalAngle = PI / 2.0 - atan( dzdr );
	Normal.x( cos_theta * cos( normalAngle ) );
	Normal.y( sin_theta * cos( normalAngle ) );
	Normal.z( -sin( normalAngle ) );

	return ( CqVector3D( r * cos_theta, r * sin_theta, z ) );
}


//---------------------------------------------------------------------
/** Constructor.
 */

CqTorus::CqTorus( TqFloat majorradius, TqFloat minorradius, TqFloat phimin, TqFloat phimax, TqFloat thetamin, TqFloat thetamax ) :
		m_MajorRadius( majorradius ),
		m_MinorRadius( minorradius ),
		m_PhiMin( phimin ),
		m_PhiMax( phimax ),
		m_ThetaMin( thetamin ),
		m_ThetaMax( thetamax )
{}


//---------------------------------------------------------------------
/** Create a clone copy of this torus.
 */

CqSurface*	CqTorus::Clone() const
{
	CqTorus* clone = new CqTorus();
	CqQuadric::CloneData( clone );
	clone->m_MajorRadius = m_MajorRadius;
	clone->m_MinorRadius = m_MinorRadius;
	clone->m_PhiMax = m_PhiMax;
	clone->m_PhiMin = m_PhiMin;
	clone->m_ThetaMin = m_ThetaMin;
	clone->m_ThetaMax = m_ThetaMax;

	return ( clone );
}



//---------------------------------------------------------------------
/** Get the geometric bound of this GPrim.
 */

CqBound	CqTorus::Bound() const
{
	std::vector<CqVector3D> curve;
	CqVector3D vA( m_MajorRadius, 0, 0 ), vB( 1, 0, 0 ), vC( 0, 0, 1 ), vD( 0, 0, 0 );
	Circle( vA, vB, vC, m_MinorRadius, RAD( m_PhiMin ), RAD( m_PhiMax ), curve );
	CqMatrix matRot( RAD ( m_ThetaMin ), vC );
	for ( std::vector<CqVector3D>::iterator i = curve.begin(); i != curve.end(); i++ )
		*i = matRot * ( *i );
	CqBound	B( RevolveForBound( curve, vD, vC, RAD( m_ThetaMax - m_ThetaMin ) ) );
	B.Transform( m_matTx );

	return ( AdjustBoundForTransformationMotion( B ) );
}


//---------------------------------------------------------------------
/** Split this GPrim into a NURBS surface. Temp implementation, should split into smalled quadrics.
 */

TqInt CqTorus::PreSubdivide( std::vector<boost::shared_ptr<CqSurface> >& aSplits, bool u )
{
	TqFloat zcent = ( m_PhiMax + m_PhiMin ) * 0.5;
	TqFloat arccent = ( m_ThetaMin + m_ThetaMax ) * 0.5;

	boost::shared_ptr<CqTorus> pNew1( new CqTorus() );
	boost::shared_ptr<CqTorus> pNew2( new CqTorus() );
	pNew1->m_matTx =pNew2->m_matTx = m_matTx;
	pNew1->m_matITTx = pNew2->m_matITTx = m_matITTx;
	pNew1->m_fDiceable = pNew2->m_fDiceable = m_fDiceable;
	pNew1->m_MajorRadius = pNew2->m_MajorRadius = m_MajorRadius;
	pNew1->m_MinorRadius = pNew2->m_MinorRadius = m_MinorRadius;
	pNew1->m_EyeSplitCount = pNew2->m_EyeSplitCount = m_EyeSplitCount;
	pNew1->SetSurfaceParameters( *this );
	pNew2->SetSurfaceParameters( *this );

	if ( u )
	{
		pNew1->m_ThetaMax = arccent;
		pNew2->m_ThetaMin = arccent;
		pNew1->m_ThetaMin = m_ThetaMin;
		pNew2->m_ThetaMax = m_ThetaMax;
		pNew1->m_PhiMax = pNew2->m_PhiMax = m_PhiMax;
		pNew1->m_PhiMin = pNew2->m_PhiMin = m_PhiMin;
	}
	else
	{
		pNew1->m_PhiMax = zcent;
		pNew2->m_PhiMin = zcent;
		pNew1->m_PhiMin = m_PhiMin;
		pNew2->m_PhiMax = m_PhiMax;
		pNew1->m_ThetaMin = pNew2->m_ThetaMin = m_ThetaMin;
		pNew1->m_ThetaMax = pNew2->m_ThetaMax = m_ThetaMax;
	}

	aSplits.push_back( pNew1 );
	aSplits.push_back( pNew2 );

	return ( 2 );
}



//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 */

CqVector3D CqTorus::DicePoint( TqInt u, TqInt v )
{
	TqFloat theta = RAD( m_ThetaMin + ( ( TqFloat ) u * ( m_ThetaMax - m_ThetaMin ) ) / m_uDiceSize );
	TqFloat phi = RAD( m_PhiMin + ( ( TqFloat ) v * ( m_PhiMax - m_PhiMin ) ) / m_vDiceSize );

	TqFloat r = m_MinorRadius * cos( phi );
	TqFloat z = m_MinorRadius * sin( phi );
	return ( CqVector3D( ( m_MajorRadius + r ) * cos( theta ), ( m_MajorRadius + r ) * sin( theta ), z ) );
}


//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 * \param Normal Storage for the surface normal at that point.
 */

CqVector3D CqTorus::DicePoint( TqInt u, TqInt v, CqVector3D& Normal )
{
	TqFloat theta = RAD( m_ThetaMin + ( ( TqFloat ) u * ( m_ThetaMax - m_ThetaMin ) ) / m_uDiceSize );
	TqFloat phi = RAD( m_PhiMin + ( ( TqFloat ) v * ( m_PhiMax - m_PhiMin ) ) / m_vDiceSize );

	TqFloat r = m_MinorRadius * cos( phi );
	TqFloat z = m_MinorRadius * sin( phi );

	Normal.x( cos( phi ) * cos( theta ) );
	Normal.y( cos( phi ) * sin( theta ) );
	Normal.z( sin( phi ) );

	return ( CqVector3D( ( m_MajorRadius + r ) * cos( theta ), ( m_MajorRadius + r ) * sin( theta ), z ) );
}


//---------------------------------------------------------------------
/** Constructor.
 */

CqDisk::CqDisk( TqFloat height, TqFloat minorradius, TqFloat majorradius, TqFloat thetamin, TqFloat thetamax ) :
		m_Height( height ),
		m_MajorRadius( majorradius ),
		m_MinorRadius( minorradius ),
		m_ThetaMin( thetamin ),
		m_ThetaMax( thetamax )
{}


//---------------------------------------------------------------------
/** Create a clone of this disk.
 */

CqSurface*	CqDisk::Clone() const
{
	CqDisk* clone = new CqDisk();
	CqQuadric::CloneData( clone );
	clone->m_Height = m_Height;
	clone->m_MajorRadius = m_MajorRadius;
	clone->m_MinorRadius = m_MinorRadius;
	clone->m_ThetaMin = m_ThetaMin;
	clone->m_ThetaMax = m_ThetaMax;

	return ( clone );
}



//---------------------------------------------------------------------
/** Get the geometric bound of this GPrim.
 */

CqBound	CqDisk::Bound() const
{
	std::vector<CqVector3D> curve;
	CqVector3D vA( m_MajorRadius, 0, m_Height ), vB( m_MinorRadius, 0, m_Height ), vC( 0, 0, 0 ), vD( 0, 0, 1 );
	curve.push_back( vA );
	curve.push_back( vB );
	CqMatrix matRot( RAD ( m_ThetaMin ), vD );
	for ( std::vector<CqVector3D>::iterator i = curve.begin(); i != curve.end(); i++ )
		*i = matRot * ( *i );
	CqBound	B( RevolveForBound( curve, vC, vD, RAD( m_ThetaMax - m_ThetaMin ) ) );
	B.Transform( m_matTx );

	return ( AdjustBoundForTransformationMotion( B ) );
}


//---------------------------------------------------------------------
/** Split this GPrim into a NURBS surface. Temp implementation, should split into smalled quadrics.
 */

TqInt CqDisk::PreSubdivide( std::vector<boost::shared_ptr<CqSurface> >& aSplits, bool u )
{
	TqFloat zcent = ( m_MajorRadius + m_MinorRadius ) * 0.5;
	TqFloat arccent = ( m_ThetaMin + m_ThetaMax ) * 0.5;

	boost::shared_ptr<CqDisk> pNew1( new CqDisk() );
	boost::shared_ptr<CqDisk> pNew2( new CqDisk() );
	pNew1->m_matTx =pNew2->m_matTx = m_matTx;
	pNew1->m_matITTx = pNew2->m_matITTx = m_matITTx;
	pNew1->m_fDiceable = pNew2->m_fDiceable = m_fDiceable;
	pNew1->m_Height = pNew2->m_Height = m_Height;
	pNew1->m_EyeSplitCount = pNew2->m_EyeSplitCount = m_EyeSplitCount;
	pNew1->SetSurfaceParameters( *this );
	pNew2->SetSurfaceParameters( *this );

	if ( u )
	{
		pNew1->m_ThetaMax = arccent;
		pNew2->m_ThetaMin = arccent;
		pNew1->m_ThetaMin = m_ThetaMin;
		pNew2->m_ThetaMax = m_ThetaMax;
		pNew1->m_MajorRadius = pNew2->m_MajorRadius = m_MajorRadius;
		pNew1->m_MinorRadius = pNew2->m_MinorRadius = m_MinorRadius;
	}
	else
	{
		pNew1->m_MinorRadius = zcent;
		pNew2->m_MajorRadius = zcent;
		pNew1->m_MajorRadius = m_MajorRadius;
		pNew2->m_MinorRadius = m_MinorRadius;
		pNew1->m_ThetaMin = pNew2->m_ThetaMin = m_ThetaMin;
		pNew1->m_ThetaMax = pNew2->m_ThetaMax = m_ThetaMax;
	}

	aSplits.push_back( pNew1 );
	aSplits.push_back( pNew2 );

	return ( 2 );
}



//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 */

CqVector3D CqDisk::DicePoint( TqInt u, TqInt v )
{
	TqFloat theta = RAD( m_ThetaMin + ( ( TqFloat ) u * ( m_ThetaMax - m_ThetaMin ) ) / m_uDiceSize );
	TqFloat vv = m_MajorRadius - ( ( TqFloat ) v * ( m_MajorRadius - m_MinorRadius ) ) / m_vDiceSize;
	return ( CqVector3D( vv * cos( theta ), vv * sin( theta ), m_Height ) );
}


//---------------------------------------------------------------------
/** Get a point on the surface indexed by the surface paramters passed.
 * \param u Float surface paramter in u.
 * \param v Float surface paramter in v.
 * \param Normal Storage for the surface normal at that point.
 */

CqVector3D CqDisk::DicePoint( TqInt u, TqInt v, CqVector3D& Normal )
{
	Normal = CqVector3D( 0, 0, m_ThetaMax > 0 ? 1 : -1 );
	return ( DicePoint( u, v ) );
}


//------------------------------------------------------------------------------
/**
 *	Create the points which make up a NURBS circle control hull, for use during boundary
 *  generation.
 *
 *	\param	O	Origin of the circle.
 *	\param	X	X axis of the plane to generate the circle in.
 *	\param	Y	Y axis of the plane to generate the circle in.
 *	\param	r	Radius of the circle.
 *	\param	as	Start angle of the circle.
 *	\param	ae	End angle of the circle.
 *	\param	points	Storage for the points of the circle.
 */

void CqQuadric::Circle( const CqVector3D& O, const CqVector3D& X, const CqVector3D& Y, TqFloat r, TqFloat as, TqFloat ae, std::vector<CqVector3D>& points ) const
{
	TqFloat theta, angle, dtheta;
	TqUint narcs;

	while ( ae < as )
		ae += 2 * RI_PI;

	theta = ae - as;
	/*	if ( theta <= RI_PIO2 )
			narcs = 1;
		else
		{
			if ( theta <= RI_PI )
				narcs = 2;
			else
			{
				if ( theta <= 1.5 * RI_PI )
					narcs = 3;
				else*/
	narcs = 4;
	/*		}
		}*/
	dtheta = theta / static_cast<TqFloat>( narcs );
	TqUint n = 2 * narcs + 1;				// n control points ;

	CqVector3D P0, T0, P2, T2, P1;
	P0 = O + r * cos( as ) * X + r * sin( as ) * Y;
	T0 = -sin( as ) * X + cos( as ) * Y;		// initialize start values

	points.resize( n );

	points[ 0 ] = P0;
	TqUint index = 0;
	angle = as;

	TqUint i;
	for ( i = 1; i <= narcs; i++ )
	{
		angle += dtheta;
		P2 = O + r * cos( angle ) * X + r * sin( angle ) * Y;
		points[ index + 2 ] = P2;
		T2 = -sin( angle ) * X + cos( angle ) * Y;
		IntersectLine( P0, T0, P2, T2, P1 );
		points[ index + 1 ] = P1;
		index += 2;
		if ( i < narcs )
		{
			P0 = P2;
			T0 = T2;
		}
	}
}



CqBound CqQuadric::RevolveForBound( const std::vector<CqVector3D>& profile, const CqVector3D& S, const CqVector3D& Tvec, TqFloat theta ) const
{
	CqBound bound( FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX );

	TqFloat angle, dtheta;
	TqUint narcs;
	TqUint i, j;

	if ( fabs( theta ) > 2.0 * RI_PI )
	{
		if ( theta < 0 )
			theta = -( 2.0 * RI_PI );
		else
			theta = 2.0 * RI_PI;
	}

	/*	if ( fabs( theta ) <= RI_PIO2 )
			narcs = 1;
		else
		{
			if ( fabs( theta ) <= RI_PI )
				narcs = 2;
			else
			{
				if ( fabs( theta ) <= 1.5 * RI_PI )
					narcs = 3;
				else*/
	narcs = 4;
	/*		}
		}*/
	dtheta = theta / static_cast<TqFloat>( narcs );

	std::vector<TqFloat> cosines( narcs + 1 );
	std::vector<TqFloat> sines( narcs + 1 );

	angle = 0.0;
	for ( i = 1; i <= narcs; i++ )
	{
		angle = dtheta * static_cast<TqFloat>( i );
		cosines[ i ] = cos( angle );
		sines[ i ] = sin( angle );
	}

	CqVector3D P0, T0, P2, T2, P1;
	CqVector3D vecTemp;

	for ( j = 0; j < profile.size(); j++ )
	{
		CqVector3D O;
		CqVector3D pj( profile[ j ] );

		ProjectToLine( S, Tvec, pj, O );
		CqVector3D X, Y;
		X = pj - O;

		TqFloat r = X.Magnitude();

		if ( r < 1e-7 )
		{
			bound.Encapsulate( O );
			continue;
		}

		X.Unit();
		Y = Tvec % X;
		Y.Unit();

		P0 = profile[ j ];
		bound.Encapsulate( P0 );

		T0 = Y;
		for ( i = 1; i <= narcs; ++i )
		{
			angle = dtheta * static_cast<TqFloat>( i );
			P2 = O + r * cosines[ i ] * X + r * sines[ i ] * Y;
			bound.Encapsulate( P2 );
			T2 = -sines[ i ] * X + cosines[ i ] * Y;
			IntersectLine( P0, T0, P2, T2, P1 );
			bound.Encapsulate( P1 );
			if ( i < narcs )
			{
				P0 = P2;
				T0 = T2;
			}
		}
	}
	return ( bound );
}


//---------------------------------------------------------------------
/** Find the point at which two infinite lines intersect.
 * The algorithm generates a plane from one of the lines and finds the 
 * intersection point between this plane and the other line.
 * \return false if they are parallel, true if they intersect.
 */

bool IntersectLine( CqVector3D& P1, CqVector3D& T1, CqVector3D& P2, CqVector3D& T2, CqVector3D& P )
{
	CqVector3D	v, px;

	px = T1 % ( P1 - T2 );
	v = px % T1;

	TqFloat	t = ( P1 - P2 ) * v;
	TqFloat vw = v * T2;
	if ( ( vw * vw ) < 1.0e-07 )
		return ( false );
	t /= vw;
	P = P2 + ( ( ( P1 - P2 ) * v ) / vw ) * T2 ;
	return ( true );
}


//---------------------------------------------------------------------
/** Project a point onto a line, returns the projection point in p.
 */

void ProjectToLine( const CqVector3D& S, const CqVector3D& Trj, const CqVector3D& pnt, CqVector3D& p )
{
	CqVector3D a = pnt - S;
	TqFloat fraction, denom;
	denom = Trj.Magnitude2();
	fraction = ( denom == 0.0 ) ? 0.0 : ( Trj * a ) / denom;
	p = fraction * Trj;
	p += S;
}

END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------
