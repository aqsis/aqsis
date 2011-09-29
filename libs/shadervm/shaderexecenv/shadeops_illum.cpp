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
		\brief Implements the basic shader operations. (Lights related)
		\author Paul C. Gregory (pgregory@aqsis.org)
*/


#include	<string>
#include	<stdio.h>

#include	<aqsis/math/math.h>
#include	"shaderexecenv.h"
#include	<aqsis/core/ilightsource.h>

#include	"../../pointrender/microbuffer.h"

namespace Aqsis {

//----------------------------------------------------------------------
// init_illuminance()
// NOTE: There is duplication here between SO_init_illuminance and 
// SO_advance_illuminance. This is required to ensure that the 
// first light is not skipped.
bool CqShaderExecEnv::SO_init_illuminance()
{
	// Check if lighting is turned off.
	if(getRenderContext())
	{
		const TqInt* enableLightingOpt = getRenderContext()->GetIntegerOption("EnableShaders", "lighting");
		if(NULL != enableLightingOpt && enableLightingOpt[0] == 0)
			return(false);
	}

	m_li = 0;
	while ( m_li < m_pAttributes ->cLights() &&
	        m_pAttributes ->pLight( m_li ) ->pShader() ->fAmbient() )
	{
		m_li++;
	}
	if ( m_li < m_pAttributes ->cLights() )
		return ( true );
	else
		return ( false );
}


//----------------------------------------------------------------------
// advance_illuminance()
bool CqShaderExecEnv::SO_advance_illuminance()
{
	// Check if lighting is turned off, should never need this check as SO_init_illuminance will catch first.
	if(getRenderContext())
	{
		const TqInt* enableLightingOpt = getRenderContext()->GetIntegerOption("EnableShaders", "lighting");
		if(NULL != enableLightingOpt && enableLightingOpt[0] == 0)
			return(false);
	}

	m_li++;
	while ( m_li < m_pAttributes ->cLights() &&
	        m_pAttributes ->pLight( m_li ) ->pShader() ->fAmbient() )
	{
		m_li++;
	}
	if ( m_li < m_pAttributes ->cLights() )
		return ( true );
	else
		return ( false );
}


//----------------------------------------------------------------------
// init_gather()
void CqShaderExecEnv::SO_init_gather(IqShaderData* samples, IqShader* pShader)
{
	bool __fVarying;
	TqUint __iGrid = 0;

	__fVarying=(samples)->Class()==class_varying;

	TqFloat _aq_samples;
	(samples)->GetFloat(_aq_samples,__iGrid);

	// Check if lighting is turned off.
	if(getRenderContext())
	{
		const TqInt* enableLightingOpt = getRenderContext()->GetIntegerOption("EnableShaders", "lighting");
		if(NULL != enableLightingOpt && enableLightingOpt[0] == 0)
			return;
	}

	m_gatherSample = static_cast<TqUint> (_aq_samples);
}


//----------------------------------------------------------------------
// advance_illuminance()
bool CqShaderExecEnv::SO_advance_gather()
{
	// Check if lighting is turned off, should never need this check as SO_init_illuminance will catch first.
	if(getRenderContext())
	{
		const TqInt* enableLightingOpt = getRenderContext()->GetIntegerOption("EnableShaders", "lighting");
		if(NULL != enableLightingOpt && enableLightingOpt[0] == 0)
			return(false);
	}

	return((--m_gatherSample) > 0);
}


void CqShaderExecEnv::ValidateIlluminanceCache( IqShaderData* pP, IqShaderData* pN, IqShader* pShader )
{
	// If this is the first call to illuminance this time round, call all lights and setup the Cl and L caches.
	if ( !m_IlluminanceCacheValid )
	{
		// Check if lighting is turned off.
		if(getRenderContext())
		{
			const TqInt* enableLightingOpt = getRenderContext()->GetIntegerOption("EnableShaders", "lighting");
			if(NULL != enableLightingOpt && enableLightingOpt[0] == 0)
			{
				m_IlluminanceCacheValid = true;
				return;
			}
		}

		IqShaderData* Ns = (pN != NULL )? pN : N();
		IqShaderData* Ps = (pP != NULL )? pP : P();
		TqUint li = 0;
		while ( li < m_pAttributes ->cLights() )
		{
			IqLightsource * lp = m_pAttributes ->pLight( li );
			// Initialise the lightsource
			lp->Initialise( uGridRes(), vGridRes(), microPolygonCount(), shadingPointCount(), m_hasValidDerivatives );
			m_Illuminate = 0;
			// Evaluate the lightsource
			lp->Evaluate( Ps, Ns, m_pCurrentSurface );
			li++;
		}
		m_IlluminanceCacheValid = true;
	}
}

//----------------------------------------------------------------------
// reflect(I,N)
void CqShaderExecEnv::SO_reflect( IqShaderData* I, IqShaderData* N, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(I)->Class()==class_varying;
	__fVarying=(N)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_I;
			(I)->GetVector(_aq_I,__iGrid);
			CqVector3D _aq_N;
			(N)->GetNormal(_aq_N,__iGrid);
			TqFloat idn = 2.0f * ( _aq_I * _aq_N );
			CqVector3D res = _aq_I - ( idn * _aq_N );
			(Result)->SetVector(res,__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


//----------------------------------------------------------------------
// reftact(I,N,eta)
void CqShaderExecEnv::SO_refract( IqShaderData* I, IqShaderData* N, IqShaderData* eta, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(I)->Class()==class_varying;
	__fVarying=(N)->Class()==class_varying||__fVarying;
	__fVarying=(eta)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_I;
			(I)->GetVector(_aq_I,__iGrid);
			CqVector3D _aq_N;
			(N)->GetNormal(_aq_N,__iGrid);
			TqFloat _aq_eta;
			(eta)->GetFloat(_aq_eta,__iGrid);
			TqFloat IdotN = _aq_I * _aq_N;
			TqFloat feta = _aq_eta;
			TqFloat k = 1 - feta * feta * ( 1 - IdotN * IdotN );
			(Result)->SetVector(( k < 0.0f ) ? CqVector3D( 0, 0, 0 ) : CqVector3D( feta * _aq_I - ( feta * IdotN + sqrt( k ) ) * _aq_N ),__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


//----------------------------------------------------------------------
// fresnel(I,N,eta,Kr,Kt)

void CqShaderExecEnv::SO_fresnel( IqShaderData* I, IqShaderData* N, IqShaderData* eta, IqShaderData* Kr, IqShaderData* Kt, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(I)->Class()==class_varying;
	__fVarying=(N)->Class()==class_varying||__fVarying;
	__fVarying=(eta)->Class()==class_varying||__fVarying;
	__fVarying=(Kr)->Class()==class_varying||__fVarying;
	__fVarying=(Kt)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_I;
			(I)->GetVector(_aq_I,__iGrid);
			CqVector3D _aq_N;
			(N)->GetNormal(_aq_N,__iGrid);
			TqFloat _aq_eta;
			(eta)->GetFloat(_aq_eta,__iGrid);
			TqFloat _aq_Kr;
			(Kr)->GetFloat(_aq_Kr,__iGrid);
			TqFloat _aq_Kt;
			(Kt)->GetFloat(_aq_Kt,__iGrid);
			TqFloat cos_theta = -_aq_I * _aq_N;
			TqFloat fuvA = ((1.0f / _aq_eta)*(1.0f / _aq_eta)) - ( 1.0f - ((cos_theta)*(cos_theta)) );
			TqFloat fuvB = fabs( fuvA );
			TqFloat fu2 = ( fuvA + fuvB ) / 2;
			TqFloat fv2 = ( -fuvA + fuvB ) / 2;
			TqFloat fv2sqrt = ( fv2 == 0.0f ) ? 0.0f : sqrt( fabs( fv2 ) );
			TqFloat fu2sqrt = ( fu2 == 0.0f ) ? 0.0f : sqrt( fabs( fu2 ) );
			TqFloat fperp2 = ( ((cos_theta - fu2sqrt)*(cos_theta - fu2sqrt)) + fv2 ) / ( ((cos_theta + fu2sqrt)*(cos_theta + fu2sqrt)) + fv2 );
			TqFloat feta = _aq_eta;
			TqFloat fpara2 = ( ((((1.0f / feta)*(1.0f / feta)) * cos_theta - fu2sqrt)*(((1.0f / feta)*(1.0f / feta)) * cos_theta - fu2sqrt)) + ((-fv2sqrt)*(-fv2sqrt)) ) /
			                 ( ((((1.0f / feta)*(1.0f / feta)) * cos_theta + fu2sqrt)*(((1.0f / feta)*(1.0f / feta)) * cos_theta + fu2sqrt)) + ((fv2sqrt)*(fv2sqrt)) );

			TqFloat __Kr = 0.5f * ( fperp2 + fpara2 );
			(Kr)->SetFloat(__Kr,__iGrid);
			(Kt)->SetFloat(1.0f - __Kr,__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// fresnel(I,N,eta,Kr,Kt,R,T)
void CqShaderExecEnv::SO_fresnel( IqShaderData* I, IqShaderData* N, IqShaderData* eta, IqShaderData* Kr, IqShaderData* Kt, IqShaderData* R, IqShaderData* T, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(I)->Class()==class_varying;
	__fVarying=(N)->Class()==class_varying||__fVarying;
	__fVarying=(eta)->Class()==class_varying||__fVarying;
	__fVarying=(Kr)->Class()==class_varying||__fVarying;
	__fVarying=(Kt)->Class()==class_varying||__fVarying;
	__fVarying=(R)->Class()==class_varying||__fVarying;
	__fVarying=(T)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_I;
			(I)->GetVector(_aq_I,__iGrid);
			CqVector3D _aq_N;
			(N)->GetNormal(_aq_N,__iGrid);
			TqFloat _aq_eta;
			(eta)->GetFloat(_aq_eta,__iGrid);
			TqFloat _aq_Kr;
			(Kr)->GetFloat(_aq_Kr,__iGrid);
			TqFloat _aq_Kt;
			(Kt)->GetFloat(_aq_Kt,__iGrid);
			CqVector3D _aq_R;
			(R)->GetVector(_aq_R,__iGrid);
			CqVector3D _aq_T;
			(T)->GetVector(_aq_T,__iGrid);
			TqFloat cos_theta = -_aq_I * _aq_N;
			TqFloat fuvA = ((1.0f / _aq_eta)*(1.0f / _aq_eta)) - ( 1.0f - ((cos_theta)*(cos_theta)) );
			TqFloat fuvB = fabs( fuvA );
			TqFloat fu2 = ( fuvA + fuvB ) / 2;
			TqFloat fv2 = ( -fuvA + fuvB ) / 2;
			TqFloat feta = _aq_eta;
			TqFloat fv2sqrt = ( fv2 == 0.0f ) ? 0.0f : sqrt( fabs( fv2 ) );
			TqFloat fu2sqrt = ( fu2 == 0.0f ) ? 0.0f : sqrt( fabs( fu2 ) );
			TqFloat fperp2 = ( ((cos_theta - fu2sqrt)*(cos_theta - fu2sqrt)) + fv2 ) / ( ((cos_theta + fu2sqrt)*(cos_theta + fu2sqrt)) + fv2 );
			TqFloat fpara2 = ( ((((1.0f / feta)*(1.0f / feta)) * cos_theta - fu2sqrt)*(((1.0f / feta)*(1.0f / feta)) * cos_theta - fu2sqrt)) + ((-fv2sqrt)*(-fv2sqrt)) ) /
			                 ( ((((1.0f / feta)*(1.0f / feta)) * cos_theta + fu2sqrt)*(((1.0f / feta)*(1.0f / feta)) * cos_theta + fu2sqrt)) + ((fv2sqrt)*(fv2sqrt)) );
			TqFloat __Kr = 0.5f * ( fperp2 + fpara2 );
			(Kr)->SetFloat(__Kr,__iGrid);
			(Kt)->SetFloat(1.0f - __Kr,__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);

	SO_reflect( I, N, R );
	SO_refract( I, N, eta, T );
}


//----------------------------------------------------------------------
// depth(P)
void CqShaderExecEnv::SO_depth( IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	if (!getRenderContext() )
		return ;

	__fVarying=(p)->Class()==class_varying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();

	TqFloat ClippingNear = getRenderContext() ->GetFloatOption( "System", "Clipping" ) [ 0 ] ;
	TqFloat ClippingFar = getRenderContext() ->GetFloatOption( "System", "Clipping" ) [ 1 ] ;
	TqFloat DeltaClipping = ClippingFar - ClippingNear;

	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_p;
			(p)->GetPoint(_aq_p,__iGrid);
			TqFloat d = _aq_p.z();
			d = ( d - ClippingNear)/DeltaClipping;
			(Result)->SetFloat(d,__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


//----------------------------------------------------------------------
// ambient()

void CqShaderExecEnv::SO_ambient( IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	// Check if lighting is turned off.
	if(getRenderContext())
	{
		const TqInt* enableLightingOpt = getRenderContext()->GetIntegerOption("EnableShaders", "lighting");
		if(NULL != enableLightingOpt && enableLightingOpt[0] == 0)
			return;
	}

	// Use the lightsource stack on the current surface
	if ( m_pAttributes != 0 )
	{
		// If this is the first call to illuminance this time round, call all lights and setup the Cl and L caches.
		if ( !m_IlluminanceCacheValid )
		{
			ValidateIlluminanceCache( NULL, NULL, pShader );
		}

		Result->SetColor( gColBlack );

		for ( TqUint light_index = 0; light_index < m_pAttributes ->cLights(); light_index++ )
		{
			__fVarying = true;

			IqLightsource* lp = m_pAttributes ->pLight( light_index );
			if ( lp->pShader() ->fAmbient() )
			{
				__iGrid = 0;
				const CqBitVector& RS = RunningState();
				do
				{
					if(!__fVarying || RS.Value( __iGrid ) )
					{
						// Now Combine the color of all ambient lightsources.
						CqColor _aq_Result;
						(Result)->GetColor(_aq_Result,__iGrid);
						CqColor colCl;
						if ( NULL != lp->Cl() )
							lp->Cl() ->GetColor( colCl, __iGrid );
						(Result)->SetColor(_aq_Result + colCl,__iGrid);

					}
				}
				while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
			}
		}
	}
}


//----------------------------------------------------------------------
// diffuse(N)
void CqShaderExecEnv::SO_diffuse( IqShaderData* N, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	// If the illuminance cache is already OK, then we don't need to bother filling in the illuminance parameters.
	if ( !m_IlluminanceCacheValid )
	{
		ValidateIlluminanceCache( NULL, N, pShader );
	}

	IqShaderData* pDefAngle = pShader->CreateTemporaryStorage( type_float, class_uniform );
	if ( NULL == pDefAngle )
		return ;

	pDefAngle->SetFloat( M_PI_2 );

	Result->SetColor( gColBlack );

	__fVarying = true;
	IqShaderData* __nondiffuse = NULL;
	__nondiffuse = pShader->CreateTemporaryStorage( type_float, class_varying );

	// SO_init_illuminance returns TRUE if there are any non ambient ligthsources available.
	if ( SO_init_illuminance() )
	{
		boost::shared_ptr<IqShader> pLightsource;
		do
		{
			// Get the "__nondiffuse" setting from the current lightsource, if specified.
			TqFloat	__nondiffuse_val;
			if ( m_li < m_pAttributes ->cLights() )
				pLightsource = m_pAttributes ->pLight( m_li ) ->pShader();
			if ( pLightsource )
			{
				pLightsource->GetVariableValue( "__nondiffuse", __nondiffuse );
				/// \note: This is OK here, outside the BEGIN_VARYING_SECTION as, varying in terms of lightsources
				/// is not valid.
				if( NULL != __nondiffuse )
				{
					__nondiffuse->GetFloat( __nondiffuse_val, 0 );
					if( __nondiffuse_val != 0.0f )
						continue;
				}
			}

			// SO_illuminance sets the current state to whether the lightsource illuminates the points or not.
			SO_illuminance( NULL, NULL, N, pDefAngle, NULL );

			PushState();
			GetCurrentState();

			__iGrid = 0;
			const CqBitVector& RS = RunningState();
			do
			{
				if(!__fVarying || RS.Value( __iGrid ) )
				{

					// Get the light vector and color from the lightsource.
					CqVector3D Ln;
					L() ->GetVector( Ln, __iGrid );
					Ln.Unit();

					// Combine the light color into the result
					CqColor _aq_Result;
					(Result)->GetColor(_aq_Result,__iGrid);
					CqVector3D _aq_N;
					(N)->GetNormal(_aq_N,__iGrid);
					CqColor colCl;
					Cl() ->GetColor( colCl, __iGrid );
					(Result)->SetColor(_aq_Result + colCl * ( Ln * _aq_N ),__iGrid);

				}
			}
			while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
			PopState();
			// SO_advance_illuminance returns TRUE if there are any more non ambient lightsources.
		}
		while ( SO_advance_illuminance() );
	}
	pShader->DeleteTemporaryStorage( __nondiffuse );
	pShader->DeleteTemporaryStorage( pDefAngle );
}


//----------------------------------------------------------------------
// specular(N,V,roughness)
void CqShaderExecEnv::SO_specular( IqShaderData* N, IqShaderData* V, IqShaderData* roughness, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	// If the illuminance cache is already OK, then we don't need to bother filling in the illuminance parameters.
	if ( !m_IlluminanceCacheValid )
	{
		ValidateIlluminanceCache( NULL, N, pShader );
	}

	IqShaderData* pDefAngle = pShader->CreateTemporaryStorage( type_float, class_uniform );
	if ( NULL == pDefAngle )
		return ;

	pDefAngle->SetFloat( M_PI_2 );

	Result->SetColor( gColBlack );
	__fVarying = true;

	IqShaderData* __nonspecular = NULL;
	__nonspecular = pShader->CreateTemporaryStorage( type_float, class_varying );

	// SO_init_illuminance returns TRUE if there are any non ambient ligthsources available.
	if ( SO_init_illuminance() )
	{
		boost::shared_ptr<IqShader> pLightsource;
		do
		{
			// Get the "__nonspecular" setting from the current lightsource, if specified.
			TqFloat	__nonspecular_val;
			if ( m_li < m_pAttributes ->cLights() )
				pLightsource = m_pAttributes ->pLight( m_li ) ->pShader();
			if ( pLightsource )
			{
				pLightsource->GetVariableValue( "__nonspecular", __nonspecular );
				/// \note: This is OK here, outside the BEGIN_VARYING_SECTION as, varying in terms of lightsources
				/// is not valid.
				if( NULL != __nonspecular )
				{
					__nonspecular->GetFloat( __nonspecular_val, 0 );
					if( __nonspecular_val != 0.0f )
						continue;
				}
			}

			// SO_illuminance sets the current state to whether the lightsource illuminates the points or not.
			SO_illuminance( NULL, NULL, N, pDefAngle, NULL );

			PushState();
			GetCurrentState();
			__iGrid = 0;
			const CqBitVector& RS = RunningState();
			do
			{
				if(!__fVarying || RS.Value( __iGrid ) )
				{

					CqVector3D _aq_V;
					(V)->GetVector(_aq_V,__iGrid);
					// Get the ligth vector and color from the lightsource
					CqVector3D Ln;
					L() ->GetVector( Ln, __iGrid );
					Ln.Unit();
					CqVector3D	H = Ln + _aq_V;
					H.Unit();

					// Combine the color into the result.
					/// \note The (roughness/8) term emulates the BMRT behaviour for prmanspecular.
					CqColor _aq_Result;
					(Result)->GetColor(_aq_Result,__iGrid);
					CqVector3D _aq_N;
					(N)->GetNormal(_aq_N,__iGrid);
					TqFloat _aq_roughness;
					(roughness)->GetFloat(_aq_roughness,__iGrid);
					CqColor colCl;
					Cl() ->GetColor( colCl, __iGrid );
					(Result)->SetColor(_aq_Result + colCl * pow( max( 0.0f, _aq_N * H ), 1.0f / ( _aq_roughness / 8.0f ) ),__iGrid);

				}
			}
			while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
			PopState();
			// SO_advance_illuminance returns TRUE if there are any more non ambient lightsources.
		}
		while ( SO_advance_illuminance() );
	}
	pShader->DeleteTemporaryStorage( __nonspecular );
	pShader->DeleteTemporaryStorage( pDefAngle );
}


//----------------------------------------------------------------------
// phong(N,V,size)
void CqShaderExecEnv::SO_phong( IqShaderData* N, IqShaderData* V, IqShaderData* size, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	IqShaderData * pnV = pShader ->CreateTemporaryStorage( type_vector, class_varying );
	IqShaderData* pnN = pShader ->CreateTemporaryStorage( type_normal, class_varying );
	IqShaderData* pR = pShader ->CreateTemporaryStorage( type_vector, class_varying );

	/// note: Not happy about this, the shader should take care of this at construction time,
	/// but at the moment, it can't guarantee the validity of the m_u/vGridRes data members.
	pnV->Initialise( shadingPointCount() );
	pnN->Initialise( shadingPointCount() );
	pR->Initialise( shadingPointCount() );

	SO_normalize( V, pnV );
	SO_normalize( N, pnN );

	__fVarying = true;
	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D vecnV;
			pnV->GetVector( vecnV, __iGrid );
			pnV->SetVector( -vecnV, __iGrid );
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);

	SO_reflect( pnV, pnN, pR );

	pShader->DeleteTemporaryStorage( pnV );
	pShader->DeleteTemporaryStorage( pnN );

	// If the illuminance cache is already OK, then we don't need to bother filling in the illuminance parameters.
	if ( !m_IlluminanceCacheValid )
	{
		ValidateIlluminanceCache( NULL, N, pShader );
	}

	IqShaderData* pDefAngle = pShader->CreateTemporaryStorage( type_float, class_uniform );
	if ( NULL == pDefAngle )
		return ;

	pDefAngle->SetFloat( M_PI_2 );

	// Initialise the return value
	Result->SetColor( gColBlack );

	// SO_init_illuminance returns TRUE if there are any non ambient ligthsources available.
	if ( SO_init_illuminance() )
	{
		do
		{
			// SO_illuminance sets the current state to whether the lightsource illuminates the points or not.
			SO_illuminance( NULL, NULL, N, pDefAngle, NULL );

			PushState();
			GetCurrentState();

			__iGrid = 0;
			const CqBitVector& RS = RunningState();
			do
			{
				if(!__fVarying || RS.Value( __iGrid ) )
				{

					// Get the light vector and color from the light source.
					CqVector3D Ln;
					L() ->GetVector( Ln, __iGrid );
					Ln.Unit();

					// Now combine the color into the result.
					CqColor _aq_Result;
					(Result)->GetColor(_aq_Result,__iGrid);
					CqVector3D vecR;
					pR->GetVector( vecR, __iGrid );
					TqFloat _aq_size;
					(size)->GetFloat(_aq_size,__iGrid);
					CqColor colCl;
					Cl() ->GetColor( colCl, __iGrid );
					(Result)->SetColor(_aq_Result + colCl * pow( max( 0.0f, vecR * Ln ), _aq_size ),__iGrid);

				}
			}
			while( ( ++__iGrid < shadingPointCount() ) && __fVarying);

			PopState();
			// SO_advance_illuminance returns TRUE if there are any more non ambient lightsources.
		}
		while ( SO_advance_illuminance() );
	}
	pShader->DeleteTemporaryStorage( pDefAngle );
	pShader->DeleteTemporaryStorage( pR );
}


//----------------------------------------------------------------------
// trace(P,R)
void CqShaderExecEnv::SO_trace( IqShaderData* P, IqShaderData* R, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(P)->Class()==class_varying;
	__fVarying=(R)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			(Result)->SetColor(CqColor( 0, 0, 0 ),__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


//----------------------------------------------------------------------
// illuminance(P,nsamples)
void CqShaderExecEnv::SO_illuminance( IqShaderData* Category, IqShaderData* P, IqShaderData* Axis, IqShaderData* Angle, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__iGrid = 0;
	CqString cat( "" );
	if ( NULL != Category )
		Category->GetString( cat );


	__fVarying = true;

	// Fill in the lightsource information, and transfer the results to the shader variables,
	if ( m_pAttributes != 0 )
	{
		IqLightsource * lp = m_pAttributes ->pLight( m_li );

		if ( NULL != Axis )
			__fVarying=(Axis)->Class()==class_varying||__fVarying;
		if ( NULL != Angle )
			__fVarying=(Angle)->Class()==class_varying||__fVarying;

		bool exec = true;

		if( cat.size() )
		{

			bool exclude = false;
			CqString lightcategories;
			CqString catname;


			if( cat.find( "-" ) == 0 )
			{
				exclude = true;
				catname = cat.substr( 1, cat.size() );
			}
			else
			{
				catname = cat;
			}

			IqShaderData* pcats = lp->pShader()->FindArgument("__category");
			if( pcats )
			{
				pcats->GetString( lightcategories );

				exec = false;
				// While no matching category has been found...
				CqString::size_type tokenpos = 0, tokenend;
				while( 1 )
				{
					tokenend = lightcategories.find(',', tokenpos);
					CqString token = lightcategories.substr( tokenpos, tokenend );
					if( catname.compare( token ) == 0 )
					{
						if( !exclude )
						{
							exec = true;
							break;
						}
					}
					if( tokenend == std::string::npos )
						break;
					else
						tokenpos = tokenend+1;
				}
			}
		}

		if( exec )
		{
			__iGrid = 0;
			const CqBitVector& RS = RunningState();
			do
			{
				if(!__fVarying || RS.Value( __iGrid ) )
				{

					CqVector3D Ln;
					lp->L() ->GetVector( Ln, __iGrid );
					Ln = -Ln;

					// Store them locally on the surface.
					L() ->SetVector( Ln, __iGrid );
					CqColor colCl;
					lp->Cl() ->GetColor( colCl, __iGrid );
					Cl() ->SetColor( colCl, __iGrid );

					// Check if its within the cone.
					Ln.Unit();
					CqVector3D vecAxis( 0, 1, 0 );
					if ( NULL != Axis )
						Axis->GetVector( vecAxis, __iGrid );
					TqFloat fAngle = M_PI;
					if ( NULL != Angle )
						Angle->GetFloat( fAngle, __iGrid );

					TqFloat cosangle = Ln * vecAxis;
					cosangle = clamp(cosangle, -1.0f, 1.0f);
					if ( acos( cosangle ) > fAngle )
						m_CurrentState.SetValue( __iGrid, false );
					else
						m_CurrentState.SetValue( __iGrid, true );
				}
			}
			while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
		}
	}
}


void	CqShaderExecEnv::SO_illuminance( IqShaderData* Category, IqShaderData* P, IqShader* pShader )
{
	SO_illuminance( Category, P, NULL, NULL );
}


//----------------------------------------------------------------------
// illuminate(P)
void CqShaderExecEnv::SO_illuminate( IqShaderData* P, IqShaderData* Axis, IqShaderData* Angle, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	bool res = true;
	if ( m_Illuminate > 0 )
		res = false;

	__fVarying = true;
	if ( res )
	{
		__iGrid = 0;
		const CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				// Get the point being lit and set the ligth vector.
				CqVector3D _aq_P;
				(P)->GetPoint(_aq_P,__iGrid);
				CqVector3D vecPs;
				Ps() ->GetPoint( vecPs, __iGrid );
				L() ->SetVector( vecPs - _aq_P, __iGrid );

				// Check if its within the cone.
				CqVector3D Ln;
				L() ->GetVector( Ln, __iGrid );
				Ln.Unit();

				CqVector3D vecAxis( 0.0f, 1.0f, 0.0f );
				if ( NULL != Axis )
					Axis->GetVector( vecAxis, __iGrid );
				TqFloat fAngle = M_PI;
				if ( NULL != Angle )
					Angle->GetFloat( fAngle, __iGrid );
				TqFloat cosangle = Ln * vecAxis;
				cosangle = clamp(cosangle, -1.0f, 1.0f);
				if ( acos( cosangle ) > fAngle )
				{
					// Make sure we set the light color to zero in the areas that won't be lit.
					Cl() ->SetColor( CqColor( 0, 0, 0 ), __iGrid );
					m_CurrentState.SetValue( __iGrid, false );
				}
				else
					m_CurrentState.SetValue( __iGrid, true );
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}

	m_Illuminate++;
}


void	CqShaderExecEnv::SO_illuminate( IqShaderData* P, IqShader* pShader )
{
	SO_illuminate( P, NULL, NULL, pShader );
}


//----------------------------------------------------------------------
// solar()
void CqShaderExecEnv::SO_solar( IqShaderData* Axis, IqShaderData* Angle, IqShader* pShader )
{
	// TODO: Check light cone, and exclude points outside.
	bool __fVarying;
	TqUint __iGrid;

	bool res = true;
	if ( m_Illuminate > 0 )
		res = false;

	__fVarying = true;
	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			if ( res )
			{
				CqVector3D vecAxis;
				Ns()->GetNormal(vecAxis,__iGrid);
				vecAxis = -vecAxis;
				if ( NULL != Axis )
					Axis->GetVector( vecAxis, __iGrid );
				L() ->SetVector( vecAxis, __iGrid );
				m_CurrentState.SetValue( __iGrid, true );
			}
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);

	m_Illuminate++;
}


void	CqShaderExecEnv::SO_solar( IqShader* pShader )
{
	SO_solar( NULL, NULL, pShader );
}

//----------------------------------------------------------------------
// gather(category,P,N,angle,nsamples)
void CqShaderExecEnv::SO_gather( IqShaderData* category, IqShaderData* P, IqShaderData* N, IqShaderData* angle, IqShaderData* samples, IqShader* pShader, int cParams, IqShaderData** apParams)
{
	bool __fVarying;
	TqUint __iGrid;

	__iGrid = 0;
	__fVarying = true;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			m_CurrentState.SetValue( __iGrid, false );
		}
		else
			m_CurrentState.SetValue( __iGrid, false );
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// incident

void CqShaderExecEnv::SO_incident( IqShaderData* name, IqShaderData* pV, IqShaderData* Result, IqShader* pShader )
{
	TqUint __iGrid;

	__iGrid = 0;
	Result->SetValue( 0.0f, 0 );

}


//----------------------------------------------------------------------
// opposite

void CqShaderExecEnv::SO_opposite( IqShaderData* name, IqShaderData* pV, IqShaderData* Result, IqShader* pShader )
{
	TqUint __iGrid;

	__iGrid = 0;
	Result->SetValue( 0.0f, 0 );

}

//----------------------------------------------------------------------
// specularbrdf(L,N,V,rough)
void CqShaderExecEnv::SO_specularbrdf( IqShaderData* L, IqShaderData* N, IqShaderData* V, IqShaderData* rough, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(L)->Class()==class_varying;
	__fVarying=(N)->Class()==class_varying||__fVarying;
	__fVarying=(V)->Class()==class_varying||__fVarying;
	__fVarying=(rough)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_L;
			(L)->GetVector(_aq_L,__iGrid);
			CqVector3D _aq_V;
			(V)->GetVector(_aq_V,__iGrid);
			_aq_L.Unit();

			CqVector3D	H = _aq_L + _aq_V;
			H.Unit();
			/// \note The (roughness/8) term emulates the BMRT behaviour for prmanspecular.
			CqVector3D _aq_N;
			(N)->GetNormal(_aq_N,__iGrid);
			TqFloat _aq_rough;
			(rough)->GetFloat(_aq_rough,__iGrid);
			CqColor colCl;
			Cl() ->GetColor( colCl, __iGrid );
			(Result)->SetColor(colCl * pow( max( 0.0f, _aq_N * H ), 1.0f / ( _aq_rough / 8.0f ) ),__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// calculatenormal(P)
void CqShaderExecEnv::SO_calculatenormal( IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	// Find out if the orientation is inverted.
	bool CSO = pTransform()->GetHandedness(getRenderContext()->Time());
	bool O = false;
	if( pAttributes() )
		O = pAttributes() ->GetIntegerAttribute( "System", "Orientation" ) [ 0 ] != 0;
	TqFloat neg = 1;
	if ( !( (O && CSO) || (!O && !CSO) ) )
		neg = -1;

	__fVarying=(p)->Class()==class_varying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D N = diffU<CqVector3D>(p, __iGrid)
				% diffV<CqVector3D>(p, __iGrid);
			N.Unit();
			N *= neg;
			(Result)->SetNormal(N,__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


// FIXME: It's pretty ugly to have a global cache here!
//
// Missing cache features:
// * Ri search paths
static PointOctreeCache g_pointOctreeCache;

void clearPointCloudCache()
{
	g_pointOctreeCache.clear();
}


namespace {
/// Store zeros in shader variable, depending on integrator type:
/// OcclusionIntegrator has result type float, whereas RadiosityIntegrator has
/// result type Color.
template<typename T>
void storeZeroResult(IqShaderData* result, int igrid)
{
	result->SetFloat(0.0f,igrid);
}
template<>
void storeZeroResult<RadiosityIntegrator>(IqShaderData* result, int igrid)
{
	result->SetColor(CqColor(0.0f),igrid);
}
}


template<typename IntegratorT>
void CqShaderExecEnv::pointCloudIntegrate(IqShaderData* P, IqShaderData* N,
										  IqShaderData* result, int cParams,
										  IqShaderData** apParams,
										  IqShader* pShader)
{
	if(!getRenderContext())
		return;

	// Extract options
	CqString paramName;
	const PointOctree* pointTree = 0;
	int faceRes = 10;
	float maxSolidAngle = 0.03;
	float coneAngle = M_PI_2;
	float bias = 0;
	CqString coordSystem = "world";
	IqShaderData* occlusionResult = 0;
	for(int i = 0; i < cParams; i+=2)
	{
		apParams[i]->GetString(paramName, 0);
		IqShaderData* paramValue = apParams[i+1];
		if(paramName == "coneangle")
		{
			if(paramValue->Type() == type_float)
				paramValue->GetFloat(coneAngle);
		}
		else if(paramName == "filename")
		{
			if(paramValue->Type() == type_string)
			{
				CqString fileName;
				paramValue->GetString(fileName, 0);
				pointTree = g_pointOctreeCache.find(fileName);
			}
		}
		else if(paramName == "maxsolidangle")
		{
			if(paramValue->Type() == type_float)
				paramValue->GetFloat(maxSolidAngle);
		}
		else if(paramName == "bias")
		{
			if(paramValue->Type() == type_float)
				paramValue->GetFloat(bias);
		}
		else if(paramName == "microbufres")
		{
			if(paramValue->Type() == type_float)
			{
				float res = 10;
				paramValue->GetFloat(res);
				faceRes = std::max(1, static_cast<int>(res));
			}
		}
		else if(paramName == "coordsystem")
		{
			if(paramValue->Type() == type_string)
				paramValue->GetString(coordSystem);
		}
		else if(paramName == "occlusion")
		{
			if(paramValue->Type() == type_float)
				occlusionResult = paramValue;
		}
		// Interesting arguments which could be implemented:
		//   "hitsides"    - sidedness culling: "front", "back", "both"
		//   "falloff", "falloffmode" - falloff of occlusion with distance
		//   ... more!
		//
		// Other arguments we may not bother with:
		//   "pointbased"  - we don't support any other method...
	}

	// Compute transform from current to appropriate space.
	CqMatrix positionTrans;
	getRenderContext()->matSpaceToSpace("current", coordSystem.c_str(),
										pShader->getTransform(),
										pTransform().get(), 0, positionTrans);
	CqMatrix normalTrans = normalTransform(positionTrans);

	// TODO: interpolation.  3delight uses Attribute "irradiance"
	// "shadingrate" to control interpolation; PRMan uses the "maxvariation"
	// parameter.

	// Number of vertices in u-direction of grid
	int uSize = m_uGridRes+1;

	bool varying = result->Class() == class_varying;
	const CqBitVector& RS = RunningState();
	if(pointTree)
	{
		int npoints = varying ? shadingPointCount() : 1;
#pragma omp parallel
		{
		// Compute occlusion for each point
		IntegratorT integrator(faceRes);
#pragma omp for
		for(int igrid = 0; igrid < npoints; ++igrid)
		{
			if(!varying || RS.Value(igrid))
			{
				CqVector3D Pval;
				// TODO: What about RiPoints?  They're not a 2D grid!
				int v = igrid/uSize;
				int u = igrid - v*uSize;
				float uinterp = 0;
				float vinterp = 0;
				// Microgrids sometimes meet each other at an acute angle.
				// Computing occlusion at the vertices where the grids meet is
				// then rather difficult because an occluding disk passes
				// exactly through the point to be occluded.  This usually
				// results in obvious light leakage from the other side of the
				// surface.
				//
				// To avoid this problem, we modify the position of any
				// vertices at the edges of grids by moving them inward
				// slightly.
				//
				// TODO: Make adjustable?
				const float edgeShrink = 0.2f;
				if(u == 0)
					uinterp = edgeShrink;
				else if(u == m_uGridRes)
				{
					uinterp = 1 - edgeShrink;
					--u;
				}
				if(v == 0)
					vinterp = edgeShrink;
				else if(v == m_vGridRes)
				{
					vinterp = 1 - edgeShrink;
					--v;
				}
				if(uinterp != 0 || vinterp != 0)
				{
					CqVector3D _P1; CqVector3D _P2;
					CqVector3D _P3; CqVector3D _P4;
					int uSize = m_uGridRes + 1;
					P->GetPoint(_P1, v*uSize + u);
					P->GetPoint(_P2, v*uSize + u+1);
					P->GetPoint(_P3, (v+1)*uSize + u);
					P->GetPoint(_P4, (v+1)*uSize + u+1);
					Pval = (1-vinterp)*(1-uinterp) * _P1 +
						   (1-vinterp)*uinterp     * _P2 +
						   vinterp*(1-uinterp)     * _P3 +
						   vinterp*uinterp         * _P4;
				}
				else
					P->GetVector(Pval, igrid);
				CqVector3D Nval;   N->GetVector(Nval, igrid);
				Pval = positionTrans * Pval;
				Nval = normalTrans * Nval;
				V3f Pval2(Pval.x(), Pval.y(), Pval.z());
				V3f Nval2(Nval.x(), Nval.y(), Nval.z());
				// TODO: It may make more sense to scale bias by the current
				// micropolygon radius - that way we avoid problems with an
				// absolute length scale.
				if(bias != 0)
					Pval2 += Nval2*bias;
				integrator.clear();
				microRasterize(integrator, Pval2, Nval2, coneAngle,
							   maxSolidAngle, *pointTree);
				storeIntegratedResult(integrator, Nval2, coneAngle, result,
									  occlusionResult, igrid);
			}
		}
		}
	}
	else
	{
		// Couldn't find point cloud, set result to zero.
		TqUint igrid = 0;
		do
		{
			if(!varying || RS.Value(igrid))
			{
				storeZeroResult<IntegratorT>(result, igrid);
			}
		}
		while( ( ++igrid < shadingPointCount() ) && varying);
	}
}


//----------------------------------------------------------------------
static void storeIntegratedResult(const OcclusionIntegrator& integrator,
								  const V3f& N, float coneAngle,
								  IqShaderData* result,
								  IqShaderData* /*occlusionResult*/, int igrid)
{
	result->SetFloat(integrator.occlusion(N, coneAngle), igrid);
}

// occlusion(P,N,samples)
void CqShaderExecEnv::SO_occlusion_rt( IqShaderData* P, IqShaderData* N, IqShaderData* samples, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	pointCloudIntegrate<OcclusionIntegrator>(P, N, Result, cParams, apParams,
											 pShader);
}


//----------------------------------------------------------------------
// indirectdiffuse(P, N, samples, ...)
static void storeIntegratedResult(const RadiosityIntegrator& integrator,
								  const V3f& N, float coneAngle,
								  IqShaderData* result,
								  IqShaderData* occlusionResult, int igrid)
{
	float occ = 0;
	C3f col = integrator.radiosity(N, coneAngle, &occ);
	result->SetColor(CqColor(col.x, col.y, col.z), igrid);
	if(occlusionResult)
		occlusionResult->SetFloat(occ, igrid);
}

void CqShaderExecEnv::SO_indirectdiffuse(IqShaderData* P, IqShaderData* N,
										 IqShaderData* samples,
										 IqShaderData* Result,
										 IqShader* pShader, int cParams,
										 IqShaderData** apParams)
{
	pointCloudIntegrate<RadiosityIntegrator>(P, N, Result, cParams, apParams,
											 pShader);
}


//----------------------------------------------------------------------
// rayinfo
//

void CqShaderExecEnv::SO_rayinfo( IqShaderData* dataname, IqShaderData* pV, IqShaderData* Result, IqShader* pShader )
{
	TqUint __iGrid;

	if ( !getRenderContext() )
		return ;

	TqFloat Ret = 0.0f;

	__iGrid = 0;

	(Result)->SetFloat(Ret,__iGrid);
}

} // namespace Aqsis
//---------------------------------------------------------------------
