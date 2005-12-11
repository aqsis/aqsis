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
		\brief Implements the basic shader operations.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	"aqsis.h"

#include	<math.h>
#include	<map>
#include	<vector>
#include	<string>
#include	<stdio.h>

#if defined(REGEXP)
#include        <regex.h>
#endif

#include	"shaderexecenv.h"
#include	"spline.h"
#include	"shadervm.h"
#include	"irenderer.h"
#include	"itexturemap.h"
#include	"ilightsource.h"
#include	"version.h"

START_NAMESPACE(    Aqsis )

IqRenderer* QGetRenderContextI();

static TqFloat match(const char *string, const char *pattern);

//----------------------------------------------------------------------
// SO_sprintf
// Helper function to process a string inserting variable, used in printf and format.

static	CqString	SO_sprintf( const char* str, int cParams, IqShaderData** apParams, int varyingindex )
{
	CqString strRes( "" );
	CqString strTrans = str;
	strTrans = strTrans.TranslateEscapes();

	TqUint i = 0;
	TqUint ivar = 0;
	while ( i < strTrans.size() )
	{
		switch ( strTrans[ i ] )
		{
				case '%':   	// Insert variable.
				{
					i++;
					switch ( strTrans[ i ] )
					{
							case 'f':
							{
								TqFloat f;
								apParams[ ivar++ ] ->GetFloat( f, varyingindex );
								CqString strVal;
								strVal.Format( "%f", f );
								strRes += strVal;
							}
							break;

							case 'p':
							{
								CqVector3D vec;
								apParams[ ivar++ ] ->GetPoint( vec, varyingindex );
								CqString strVal;
								strVal.Format( "%f,%f,%f", vec.x(), vec.y(), vec.z() );
								strRes += strVal;
							}
							break;

							case 'c':
							{
								CqColor col;
								apParams[ ivar++ ] ->GetColor( col, varyingindex );
								CqString strVal;
								strVal.Format( "%f,%f,%f", col.fRed(), col.fGreen(), col.fBlue() );
								strRes += strVal;
							}
							break;

							case 'm':
							{
								CqMatrix mat;
								apParams[ ivar++ ] ->GetMatrix( mat, varyingindex );
								CqString strVal;
								strVal.Format( "%f,%f,%f,%f\n%f,%f,%f,%f\n%f,%f,%f,%f\n%f,%f,%f,%f",
								               mat.Element( 0, 0 ), mat.Element( 0, 1 ), mat.Element( 0, 2 ), mat.Element( 0, 3 ),
								               mat.Element( 1, 0 ), mat.Element( 1, 1 ), mat.Element( 1, 2 ), mat.Element( 1, 3 ),
								               mat.Element( 2, 0 ), mat.Element( 2, 1 ), mat.Element( 2, 2 ), mat.Element( 2, 3 ),
								               mat.Element( 3, 0 ), mat.Element( 3, 1 ), mat.Element( 3, 2 ), mat.Element( 3, 3 ) );
								strRes += strVal;
							}
							break;

							case 's':
							{
								CqString stra;
								apParams[ ivar++ ] ->GetString( stra, varyingindex );
								strRes += stra;
							}
							break;

							default:
							{
								strRes += strTrans[ i ];
							}
							break;
					}
					i++;
				}
				break;

				default:
				{
					strRes += strTrans[ i ];
					i++;
				}
				break;
		}
	}
	return ( strRes );
}

//----------------------------------------------------------------------
// init_illuminance()
TqBool CqShaderExecEnv::SO_init_illuminance()
{
	m_li = -1;
	return ( SO_advance_illuminance() );
}


//----------------------------------------------------------------------
// advance_illuminance()
TqBool CqShaderExecEnv::SO_advance_illuminance()
{
	m_li++;
	while ( m_li < m_pAttributes ->cLights() &&
	        m_pAttributes ->pLight( m_li ) ->pShader() ->fAmbient() )
	{
		m_li++;
	}
	if ( m_li < m_pAttributes ->cLights() )
		return ( TqTrue );
	else
		return ( TqFalse );
}


void CqShaderExecEnv::ValidateIlluminanceCache( IqShaderData* pP, IqShaderData* pN, IqShader* pShader )
{
	// If this is the first call to illuminance this time round, call all lights and setup the Cl and L caches.
	if ( !m_IlluminanceCacheValid )
	{
		IqShaderData* Ns = (pN != NULL )? pN : N();
		IqShaderData* Ps = (pP != NULL )? pP : P();
		TqUint li = 0;
		while ( li < m_pAttributes ->cLights() )
		{
			IqLightsource * lp = m_pAttributes ->pLight( li );
			// Initialise the lightsource
			lp->Initialise( uGridRes(), vGridRes() );
			m_Illuminate = 0;
			// Evaluate the lightsource
			lp->Evaluate( Ps, Ns, m_pCurrentSurface );
			li++;
		}
		m_IlluminanceCacheValid = TqTrue;
	}
}


void	CqShaderExecEnv::SO_radians( IqShaderData* degrees, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(degrees)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_degrees;
			(degrees)->GetFloat(_aq_degrees,__iGrid);
			(Result)->SetFloat(RAD( _aq_degrees ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_degrees( IqShaderData* radians, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(radians)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_radians;
			(radians)->GetFloat(_aq_radians,__iGrid);
			(Result)->SetFloat(DEG( _aq_radians ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_sin( IqShaderData* a, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(a)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_a;
			(a)->GetFloat(_aq_a,__iGrid);
			(Result)->SetFloat(static_cast<TqFloat>( sin( _aq_a ) ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_asin( IqShaderData* a, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(a)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_a;
			(a)->GetFloat(_aq_a,__iGrid);
			(Result)->SetFloat(static_cast<TqFloat>( asin( _aq_a ) ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_cos( IqShaderData* a, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(a)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_a;
			(a)->GetFloat(_aq_a,__iGrid);
			(Result)->SetFloat(static_cast<TqFloat>( cos( _aq_a ) ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_acos( IqShaderData* a, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(a)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_a;
			(a)->GetFloat(_aq_a,__iGrid);
			(Result)->SetFloat(static_cast<TqFloat>( acos( _aq_a ) ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_tan( IqShaderData* a, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(a)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_a;
			(a)->GetFloat(_aq_a,__iGrid);
			(Result)->SetFloat(static_cast<TqFloat>( tan( _aq_a ) ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_atan( IqShaderData* yoverx, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(yoverx)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_yoverx;
			(yoverx)->GetFloat(_aq_yoverx,__iGrid);
			(Result)->SetFloat(static_cast<TqFloat>( atan( _aq_yoverx ) ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_atan( IqShaderData* y, IqShaderData* x, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(x)->Class()==class_varying||__fVarying;
	__fVarying=(y)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_x;
			(x)->GetFloat(_aq_x,__iGrid);
			TqFloat _aq_y;
			(y)->GetFloat(_aq_y,__iGrid);
			(Result)->SetFloat(static_cast<TqFloat>( atan2( _aq_y, _aq_x ) ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_pow( IqShaderData* x, IqShaderData* y, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(x)->Class()==class_varying||__fVarying;
	__fVarying=(y)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_x;
			(x)->GetFloat(_aq_x,__iGrid);
			TqFloat _aq_y;
			(y)->GetFloat(_aq_y,__iGrid);
			TqFloat yy = _aq_y;
			TqFloat xx = _aq_x;
			if ( xx < 0.0f )
				yy = FLOOR( yy );
			(Result)->SetFloat(static_cast<TqFloat>( pow( xx, yy ) ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_exp( IqShaderData* x, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(x)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_x;
			(x)->GetFloat(_aq_x,__iGrid);
			(Result)->SetFloat(static_cast<TqFloat>( exp( _aq_x ) ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_sqrt( IqShaderData* x, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(x)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_x;
			(x)->GetFloat(_aq_x,__iGrid);
			(Result)->SetFloat(static_cast<TqFloat>( sqrt( _aq_x ) ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_log( IqShaderData* x, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(x)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_x;
			(x)->GetFloat(_aq_x,__iGrid);
			(Result)->SetFloat(static_cast<TqFloat>( log( _aq_x ) ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_mod( IqShaderData* a, IqShaderData* b, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(a)->Class()==class_varying||__fVarying;
	__fVarying=(b)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_a;
			(a)->GetFloat(_aq_a,__iGrid);
			TqFloat _aq_b;
			(b)->GetFloat(_aq_b,__iGrid);
			TqInt n = static_cast<TqInt>( _aq_a / _aq_b );
			TqFloat a2 = _aq_a - n * _aq_b;
			if ( a2 < 0.0f )
				a2 += _aq_b;
			(Result)->SetFloat(a2,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// log(x,base)
void	CqShaderExecEnv::SO_log( IqShaderData* x, IqShaderData* base, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(x)->Class()==class_varying||__fVarying;
	__fVarying=(base)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_x;
			(x)->GetFloat(_aq_x,__iGrid);
			TqFloat _aq_base;
			(base)->GetFloat(_aq_base,__iGrid);
			(Result)->SetFloat(static_cast<TqFloat>( log( _aq_x ) / log( _aq_base ) ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


void	CqShaderExecEnv::SO_abs( IqShaderData* x, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(x)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_x;
			(x)->GetFloat(_aq_x,__iGrid);
			(Result)->SetFloat(static_cast<TqFloat>( fabs( _aq_x ) ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_sign( IqShaderData* x, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(x)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_x;
			(x)->GetFloat(_aq_x,__iGrid);
			(Result)->SetFloat(( _aq_x < 0.0f ) ? -1.0f : 1.0f,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_min( IqShaderData* a, IqShaderData* b, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(a)->Class()==class_varying||__fVarying;
	__fVarying=(b)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_a;
			(a)->GetFloat(_aq_a,__iGrid);
			TqFloat _aq_b;
			(b)->GetFloat(_aq_b,__iGrid);
			TqFloat fRes = MIN( _aq_a, _aq_b );
			while ( cParams-- > 0 )
			{
				TqFloat fn;
				apParams[ cParams ] ->GetFloat( fn, __iGrid );
				fRes = MIN( fRes, fn );
			}
			(Result)->SetFloat(fRes,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_max( IqShaderData* a, IqShaderData* b, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(a)->Class()==class_varying||__fVarying;
	__fVarying=(b)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_a;
			(a)->GetFloat(_aq_a,__iGrid);
			TqFloat _aq_b;
			(b)->GetFloat(_aq_b,__iGrid);
			TqFloat fRes = MAX( _aq_a, _aq_b );
			while ( cParams-- > 0 )
			{
				TqFloat fn;
				apParams[ cParams ] ->GetFloat( fn, __iGrid );
				fRes = MAX( fRes, fn );
			}
			(Result)->SetFloat(fRes,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_pmin( IqShaderData* a, IqShaderData* b, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(a)->Class()==class_varying||__fVarying;
	__fVarying=(b)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_a;
			(a)->GetPoint(_aq_a,__iGrid);
			CqVector3D _aq_b;
			(b)->GetPoint(_aq_b,__iGrid);
			CqVector3D res = VMIN( _aq_a, _aq_b );
			while ( cParams-- > 0 )
			{
				CqVector3D pn;
				apParams[ cParams ] ->GetPoint( pn, __iGrid );
				res = VMIN( res, pn );
			}
			(Result)->SetPoint(res,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_pmax( IqShaderData* a, IqShaderData* b, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(a)->Class()==class_varying||__fVarying;
	__fVarying=(b)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_a;
			(a)->GetPoint(_aq_a,__iGrid);
			CqVector3D _aq_b;
			(b)->GetPoint(_aq_b,__iGrid);
			CqVector3D res = VMAX( _aq_a, _aq_b );
			while ( cParams-- > 0 )
			{
				CqVector3D pn;
				apParams[ cParams ] ->GetPoint( pn, __iGrid );
				res = VMAX( res, pn );
			}
			(Result)->SetPoint(res,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_cmin( IqShaderData* a, IqShaderData* b, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(a)->Class()==class_varying||__fVarying;
	__fVarying=(b)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqColor _aq_a;
			(a)->GetColor(_aq_a,__iGrid);
			CqColor _aq_b;
			(b)->GetColor(_aq_b,__iGrid);
			CqColor res = CMIN( _aq_a, _aq_b );
			while ( cParams-- > 0 )
			{
				CqColor cn;
				apParams[ cParams ] ->GetColor( cn, __iGrid );
				res = CMIN( res, cn );
			}
			(Result)->SetColor(res,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_cmax( IqShaderData* a, IqShaderData* b, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(a)->Class()==class_varying||__fVarying;
	__fVarying=(b)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqColor _aq_a;
			(a)->GetColor(_aq_a,__iGrid);
			CqColor _aq_b;
			(b)->GetColor(_aq_b,__iGrid);
			CqColor res = CMAX( _aq_a, _aq_b );
			while ( cParams-- > 0 )
			{
				CqColor cn;
				apParams[ cParams ] ->GetColor( cn, __iGrid );
				res = CMAX( res, cn );
			}
			(Result)->SetColor(res,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_clamp( IqShaderData* a, IqShaderData* _min, IqShaderData* _max, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(a)->Class()==class_varying||__fVarying;
	__fVarying=(_min)->Class()==class_varying||__fVarying;
	__fVarying=(_max)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_a;
			(a)->GetFloat(_aq_a,__iGrid);
			TqFloat _aq__min;
			(_min)->GetFloat(_aq__min,__iGrid);
			TqFloat _aq__max;
			(_max)->GetFloat(_aq__max,__iGrid);
			(Result)->SetFloat(CLAMP( _aq_a, _aq__min, _aq__max ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_pclamp( IqShaderData* a, IqShaderData* _min, IqShaderData* _max, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(a)->Class()==class_varying||__fVarying;
	__fVarying=(_min)->Class()==class_varying||__fVarying;
	__fVarying=(_max)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_a;
			(a)->GetPoint(_aq_a,__iGrid);
			CqVector3D _aq__min;
			(_min)->GetPoint(_aq__min,__iGrid);
			CqVector3D _aq__max;
			(_max)->GetPoint(_aq__max,__iGrid);
			(Result)->SetPoint(VCLAMP( _aq_a, _aq__min, _aq__max ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_cclamp( IqShaderData* a, IqShaderData* _min, IqShaderData* _max, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(a)->Class()==class_varying||__fVarying;
	__fVarying=(_min)->Class()==class_varying||__fVarying;
	__fVarying=(_max)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqColor _aq_a;
			(a)->GetColor(_aq_a,__iGrid);
			CqColor _aq__min;
			(_min)->GetColor(_aq__min,__iGrid);
			CqColor _aq__max;
			(_max)->GetColor(_aq__max,__iGrid);
			(Result)->SetColor(CCLAMP( _aq_a, _aq__min, _aq__max ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_floor( IqShaderData* x, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(x)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_x;
			(x)->GetFloat(_aq_x,__iGrid);
			(Result)->SetFloat(static_cast<TqFloat>( FLOOR( _aq_x ) ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_ceil( IqShaderData* x, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(x)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_x;
			(x)->GetFloat(_aq_x,__iGrid);
			(Result)->SetFloat(static_cast<TqFloat>( CEIL( _aq_x ) ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_round( IqShaderData* x, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;
	TqFloat res;

	__fVarying=(x)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_x;
			(x)->GetFloat(_aq_x,__iGrid);
			if ( _aq_x >= 0.0f )
				res = static_cast<TqFloat>( static_cast<TqInt>( _aq_x + 0.5f ) );
			else
				res = static_cast<TqFloat>( static_cast<TqInt>( ( CEIL( _aq_x - 0.5f ) ) ) );
			(Result)->SetFloat(res,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_step( IqShaderData* _min, IqShaderData* value, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(_min)->Class()==class_varying||__fVarying;
	__fVarying=(value)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq__min;
			(_min)->GetFloat(_aq__min,__iGrid);
			TqFloat _aq_value;
			(value)->GetFloat(_aq_value,__iGrid);
			(Result)->SetFloat(( _aq_value < _aq__min ) ? 0.0f : 1.0f,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// smoothstep(_min,_max,value)
void	CqShaderExecEnv::SO_smoothstep( IqShaderData* _min, IqShaderData* _max, IqShaderData* value, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(value)->Class()==class_varying||__fVarying;
	__fVarying=(_min)->Class()==class_varying||__fVarying;
	__fVarying=(_max)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq__min;
			(_min)->GetFloat(_aq__min,__iGrid);
			TqFloat _aq__max;
			(_max)->GetFloat(_aq__max,__iGrid);
			TqFloat _aq_value;
			(value)->GetFloat(_aq_value,__iGrid);
			if ( _aq_value < _aq__min )
				(Result)->SetFloat(0.0f,__iGrid);
			else if ( _aq_value >= _aq__max )
				(Result)->SetFloat(1.0f,__iGrid);
			else
			{
				TqFloat v = ( _aq_value - _aq__min ) / ( _aq__max - _aq__min );
				(Result)->SetFloat(v * v * ( 3.0f - 2.0f * v ),__iGrid);
			}
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
void	CqShaderExecEnv::SO_fspline( IqShaderData* value, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	CqSplineCubic spline( cParams );
	spline.SetBasis("catmull-rom");

	__fVarying=(value)->Class()==class_varying||__fVarying;
	TqInt v;
	for ( v = 0; v < cParams; v++ )
	{
		__fVarying=(( apParams[ v ] ))->Class()==class_varying||__fVarying;
	}
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_value;
			(value)->GetFloat(_aq_value,__iGrid);
			if ( _aq_value >= 1.0f )
			{
				TqFloat fl;
				apParams[ cParams - 2 ] ->GetFloat( fl, __iGrid );
				(Result)->SetFloat(fl,__iGrid);
			}
			else if ( _aq_value <= 0.0f )
			{
				TqFloat ff;
				apParams[ 1 ] ->GetFloat( ff, __iGrid );
				(Result)->SetFloat(ff,__iGrid);
			}
			else
			{
				TqInt j;
				for ( j = 0; j < cParams; j++ )
				{
					TqFloat fn;
					apParams[ j ] ->GetFloat( fn, __iGrid );
					spline[ j ] = CqVector4D( fn, 0.0f, 0.0f, 1.0f );
				}

				CqVector4D	res = spline.Evaluate( _aq_value );
				(Result)->SetFloat(res.x(),__iGrid);
			}
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
void	CqShaderExecEnv::SO_cspline( IqShaderData* value, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	CqSplineCubic spline( cParams );

	__fVarying=(value)->Class()==class_varying||__fVarying;
	TqInt v;
	for ( v = 0; v < cParams; v++ )
	{
		__fVarying=(( apParams[ v ] ))->Class()==class_varying||__fVarying;
	}
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_value;
			(value)->GetFloat(_aq_value,__iGrid);
			if ( _aq_value >= 1.0f )
			{
				CqColor cl;
				apParams[ cParams - 2 ] ->GetColor( cl, __iGrid );
				(Result)->SetColor(cl,__iGrid);
			}
			else if ( _aq_value <= 0.0f )
			{
				CqColor cf;
				apParams[ 1 ] ->GetColor( cf, __iGrid );
				(Result)->SetColor(cf,__iGrid);
			}
			else
			{
				TqInt j;
				for ( j = 0; j < cParams; j++ )
				{
					CqColor cn;
					apParams[ j ] ->GetColor( cn, __iGrid );
					spline[ j ] = CqVector4D( cn.fRed(), cn.fGreen(), cn.fBlue(), 1.0f );
				}

				CqVector4D	res = spline.Evaluate( _aq_value );
				(Result)->SetColor(CqColor( res.x(), res.y(), res.z() ),__iGrid);
			}
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
void	CqShaderExecEnv::SO_pspline( IqShaderData* value, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	CqSplineCubic spline( cParams );

	__fVarying=(value)->Class()==class_varying||__fVarying;
	TqInt v;
	for ( v = 0; v < cParams; v++ )
	{
		__fVarying=(( apParams[ v ] ))->Class()==class_varying||__fVarying;
	}
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_value;
			(value)->GetFloat(_aq_value,__iGrid);
			if ( _aq_value >= 1.0f )
			{
				CqVector3D pl;
				apParams[ cParams - 2 ] ->GetPoint( pl, __iGrid );
				(Result)->SetPoint(pl,__iGrid);
			}
			else if ( _aq_value <= 0.0f )
			{
				CqVector3D pf;
				apParams[ 1 ] ->GetPoint( pf, __iGrid );
				(Result)->SetPoint(pf,__iGrid);
			}
			else
			{
				TqInt j;
				for ( j = 0; j < cParams; j++ )
				{
					CqVector3D pn;
					apParams[ j ] ->GetPoint( pn, __iGrid );
					spline[ j ] = pn;
				}

				CqVector3D	res = spline.Evaluate( _aq_value );
				(Result)->SetPoint(res,__iGrid);
			}
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
void	CqShaderExecEnv::SO_sfspline( IqShaderData* basis, IqShaderData* value, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	CqSplineCubic spline( cParams );

	__fVarying=(value)->Class()==class_varying||__fVarying;
	TqInt v;
	for ( v = 0; v < cParams; v++ )
	{
		__fVarying=(( apParams[ v ] ))->Class()==class_varying||__fVarying;
	}
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqString _aq_basis;
	(basis)->GetString(_aq_basis,__iGrid);
	spline.SetBasis( _aq_basis );


	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_value;
			(value)->GetFloat(_aq_value,__iGrid);
			if ( _aq_value >= 1.0f )
			{
				TqFloat fl;
				apParams[ cParams - 2 ] ->GetFloat( fl, __iGrid );
				(Result)->SetFloat(fl,__iGrid);
			}
			else if ( _aq_value <= 0.0f )
			{
				TqFloat ff;
				apParams[ 1 ] ->GetFloat( ff, __iGrid );
				(Result)->SetFloat(ff,__iGrid);
			}
			else
			{
				TqInt j;
				for ( j = 0; j < cParams; j++ )
				{
					TqFloat fn;
					apParams[ j ] ->GetFloat( fn, __iGrid );
					spline[ j ] = CqVector4D( fn, 0.0f, 0.0f, 1.0f );
				}

				CqVector4D	res = spline.Evaluate( _aq_value );
				(Result)->SetFloat(res.x(),__iGrid);
			}
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
void	CqShaderExecEnv::SO_scspline( IqShaderData* basis, IqShaderData* value, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	CqSplineCubic spline( cParams );

	__fVarying=(value)->Class()==class_varying||__fVarying;
	TqInt v;
	for ( v = 0; v < cParams; v++ )
	{
		__fVarying=(( apParams[ v ] ))->Class()==class_varying||__fVarying;
	}
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	TqFloat _aq_basis;
	(basis)->GetFloat(_aq_basis,__iGrid);
	spline.SetBasis( _aq_basis );


	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_value;
			(value)->GetFloat(_aq_value,__iGrid);
			if ( _aq_value >= 1.0f )
			{
				CqColor cl;
				apParams[ cParams - 2 ] ->GetColor( cl, __iGrid );
				(Result)->SetColor(cl,__iGrid);
			}
			else if ( _aq_value <= 0.0f )
			{
				CqColor cf;
				apParams[ 1 ] ->GetColor( cf, __iGrid );
				(Result)->SetColor(cf,__iGrid);
			}
			else
			{
				TqInt j;
				for ( j = 0; j < cParams; j++ )
				{
					CqColor cn;
					apParams[ j ] ->GetColor( cn, __iGrid );
					spline[ j ] = CqVector4D( cn.fRed(), cn.fGreen(), cn.fBlue(), 1.0f );
				}

				CqVector4D	res = spline.Evaluate( _aq_value );
				(Result)->SetColor(CqColor( res.x(), res.y(), res.z() ),__iGrid);
			}
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
void	CqShaderExecEnv::SO_spspline( IqShaderData* basis, IqShaderData* value, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	CqSplineCubic spline( cParams );

	__fVarying=(value)->Class()==class_varying||__fVarying;
	TqInt v;
	for ( v = 0; v < cParams; v++ )
	{
		__fVarying=(( apParams[ v ] ))->Class()==class_varying||__fVarying;
	}
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqString _aq_basis;
	(basis)->GetString(_aq_basis,__iGrid);
	spline.SetBasis( _aq_basis );


	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_value;
			(value)->GetFloat(_aq_value,__iGrid);
			if ( _aq_value >= 1.0f )
			{
				CqVector3D pl;
				apParams[ cParams - 2 ] ->GetPoint( pl, __iGrid );
				(Result)->SetPoint(pl,__iGrid);
			}
			else if ( _aq_value <= 0.0f )
			{
				CqVector3D pf;
				apParams[ 1 ] ->GetPoint( pf, __iGrid );
				(Result)->SetPoint(pf,__iGrid);
			}
			else
			{
				TqInt j;
				for ( j = 0; j < cParams; j++ )
				{
					CqVector3D pn;
					apParams[ j ] ->GetPoint( pn, __iGrid );
					spline[ j ] = pn;
				}

				CqVector3D	res = spline.Evaluate( _aq_value );
				(Result)->SetPoint(res,__iGrid);
			}
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


void	CqShaderExecEnv::SO_fDu( IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	TqFloat Deffloat = 0.0f;
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			(Result)->SetFloat(SO_DuType<TqFloat>( p, __iGrid, this, Deffloat ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


void	CqShaderExecEnv::SO_fDv( IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	TqFloat Deffloat = 0.0f;
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			(Result)->SetFloat(SO_DvType<TqFloat>( p, __iGrid, this, Deffloat ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


void	CqShaderExecEnv::SO_fDeriv( IqShaderData* p, IqShaderData* den, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(den)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			(Result)->SetFloat(SO_DerivType<TqFloat>( p, den, __iGrid, this ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


void	CqShaderExecEnv::SO_cDu( IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	CqColor Defcol( 0.0f, 0.0f, 0.0f );
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			(Result)->SetColor(SO_DuType<CqColor>( p, __iGrid, this, Defcol ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


void	CqShaderExecEnv::SO_cDv( IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	CqColor Defcol( 0.0f, 0.0f, 0.0f );
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			(Result)->SetColor(SO_DvType<CqColor>( p, __iGrid, this, Defcol ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


void	CqShaderExecEnv::SO_cDeriv( IqShaderData* p, IqShaderData* den, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(den)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			(Result)->SetColor(SO_DerivType<CqColor>( p, den, __iGrid, this ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


void	CqShaderExecEnv::SO_pDu( IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	CqVector3D Defvec( 0.0f, 0.0f, 0.0f );
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			(Result)->SetPoint(SO_DuType<CqVector3D>( p, __iGrid, this, Defvec ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


void	CqShaderExecEnv::SO_pDv( IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	CqVector3D Defvec( 0.0f, 0.0f, 0.0f );
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			(Result)->SetPoint(SO_DvType<CqVector3D>( p, __iGrid, this, Defvec ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


void	CqShaderExecEnv::SO_pDeriv( IqShaderData* p, IqShaderData* den, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(den)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			(Result)->SetPoint(SO_DerivType<CqVector3D>( p, den, __iGrid, this ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


void	CqShaderExecEnv::SO_frandom( IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			(Result)->SetFloat(m_random.RandomFloat(),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_crandom( IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat a, b, c;
			a = m_random.RandomFloat();
			b = m_random.RandomFloat();
			c = m_random.RandomFloat();

			(Result)->SetColor(CqColor(a,b,c),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_prandom( IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat a, b, c;
			a = m_random.RandomFloat();
			b = m_random.RandomFloat();
			c = m_random.RandomFloat();

			(Result)->SetColor(CqVector3D(a,b,c),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// noise(v)
void	CqShaderExecEnv::SO_fnoise1( IqShaderData* v, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			(Result)->SetFloat( m_noise.FGNoise1( _aq_v ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// noise(u,v)
void CqShaderExecEnv::SO_fnoise2( IqShaderData* u, IqShaderData* v, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(u)->Class()==class_varying||__fVarying;
	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_u;
			(u)->GetFloat(_aq_u,__iGrid);
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			(Result)->SetFloat( m_noise.FGNoise2( _aq_u, _aq_v ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// noise(p)
void CqShaderExecEnv::SO_fnoise3( IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_p;
			(p)->GetPoint(_aq_p,__iGrid);
			(Result)->SetFloat( m_noise.FGNoise3( _aq_p ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// noise(p,t)
void CqShaderExecEnv::SO_fnoise4( IqShaderData* p, IqShaderData* t, IqShaderData* Result, IqShader* pShader )
{
	// TODO: Do proper 4D noise.
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(t)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_p;
			(p)->GetPoint(_aq_p,__iGrid);
			TqFloat _aq_t;
			(t)->GetFloat(_aq_t,__iGrid);
			(Result)->SetFloat( m_noise.FGNoise4( _aq_p, _aq_t ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// noise(v)
void	CqShaderExecEnv::SO_cnoise1( IqShaderData* v, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			(Result)->SetColor( m_noise.CGNoise1( _aq_v ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// noise(u,v)
void CqShaderExecEnv::SO_cnoise2( IqShaderData* u, IqShaderData* v, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(u)->Class()==class_varying||__fVarying;
	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_u;
			(u)->GetFloat(_aq_u,__iGrid);
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			(Result)->SetColor( m_noise.CGNoise2( _aq_u, _aq_v ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// noise(p)
void CqShaderExecEnv::SO_cnoise3( IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_p;
			(p)->GetPoint(_aq_p,__iGrid);
			(Result)->SetColor( m_noise.CGNoise3( _aq_p ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// noise(p,t)
void CqShaderExecEnv::SO_cnoise4( IqShaderData* p, IqShaderData* t, IqShaderData* Result, IqShader* pShader )
{
	// TODO: Do proper 4D noise.
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(t)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_p;
			(p)->GetPoint(_aq_p,__iGrid);
			TqFloat _aq_t;
			(t)->GetFloat(_aq_t,__iGrid);
			(Result)->SetColor( m_noise.CGNoise4( _aq_p, _aq_t ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// noise(v)
void CqShaderExecEnv::SO_pnoise1( IqShaderData* v, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			(Result)->SetPoint( m_noise.PGNoise1( _aq_v ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// noise(u,v)
void CqShaderExecEnv::SO_pnoise2( IqShaderData* u, IqShaderData* v, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(u)->Class()==class_varying||__fVarying;
	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_u;
			(u)->GetFloat(_aq_u,__iGrid);
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			(Result)->SetPoint( m_noise.PGNoise2( _aq_u, _aq_v ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// noise(p)
void CqShaderExecEnv::SO_pnoise3( IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_p;
			(p)->GetPoint(_aq_p,__iGrid);
			(Result)->SetPoint( m_noise.PGNoise3( _aq_p ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// noise(p,t)
void CqShaderExecEnv::SO_pnoise4( IqShaderData* p, IqShaderData* t, IqShaderData* Result, IqShader* pShader )
{
	// TODO: Do proper 4D noise.
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(t)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_p;
			(p)->GetPoint(_aq_p,__iGrid);
			TqFloat _aq_t;
			(t)->GetFloat(_aq_t,__iGrid);
			(Result)->SetPoint( m_noise.PGNoise4( _aq_p, _aq_t ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// setcomp(c,__iGrid,v)
void	CqShaderExecEnv::SO_setcomp( IqShaderData* p, IqShaderData* index, IqShaderData* v, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(index)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqColor _aq_p;
			(p)->GetColor(_aq_p,__iGrid);
			TqFloat _aq_index;
			(index)->GetFloat(_aq_index,__iGrid);
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			_aq_p [ static_cast<int>( _aq_index ) ] = _aq_v;
			(p)->SetColor(_aq_p,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// setxcomp(p,v)
void	CqShaderExecEnv::SO_setxcomp( IqShaderData* p, IqShaderData* v, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(v)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_p;
			(p)->GetPoint(_aq_p,__iGrid);
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			_aq_p.x( _aq_v );
			(p)->SetPoint(_aq_p,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// setycomp(p,v)
void	CqShaderExecEnv::SO_setycomp( IqShaderData* p, IqShaderData* v, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(v)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_p;
			(p)->GetPoint(_aq_p,__iGrid);
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			_aq_p.y( _aq_v );
			(p)->SetPoint(_aq_p,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// setzcomp(p,v)
void	CqShaderExecEnv::SO_setzcomp( IqShaderData* p, IqShaderData* v, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(v)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_p;
			(p)->GetPoint(_aq_p,__iGrid);
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			_aq_p.z( _aq_v );
			(p)->SetPoint(_aq_p,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}



void	CqShaderExecEnv::SO_length( IqShaderData* V, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(V)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_V;
			(V)->GetVector(_aq_V,__iGrid);
			(Result)->SetFloat(_aq_V.Magnitude(),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_distance( IqShaderData* P1, IqShaderData* P2, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(P1)->Class()==class_varying||__fVarying;
	__fVarying=(P2)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_P1;
			(P1)->GetPoint(_aq_P1,__iGrid);
			CqVector3D _aq_P2;
			(P2)->GetPoint(_aq_P2,__iGrid);
			(Result)->SetFloat(( _aq_P1 - _aq_P2 ).Magnitude(),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// area(P)
void CqShaderExecEnv::SO_area( IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	CqVector3D Defvec( 0.0f, 0.0f, 0.0f );
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	CqVector3D	vecR;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			if ( m_pAttributes )
			{
				TqFloat fdu, fdv;
				du() ->GetFloat( fdu, __iGrid );
				dv() ->GetFloat( fdv, __iGrid );
				vecR = ( SO_DuType<CqVector3D>( p, __iGrid, this, Defvec ) * fdu ) %
				       ( SO_DvType<CqVector3D>( p, __iGrid, this, Defvec ) * fdv );
				(Result)->SetFloat(vecR.Magnitude(),__iGrid);
			}

		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


void	CqShaderExecEnv::SO_normalize( IqShaderData* V, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(V)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	CqVector3D _old(1,0,0);
	CqVector3D _unit(1,0,0);
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_V;
			(V)->GetVector(_aq_V,__iGrid);
			// Trade a small comparaison instead of
			// blindly call Unit(). Big improvement
			// for polygon or relative flat patch/microgrid.
			if (_old != _aq_V)
			{
				_unit = _aq_V;
				_unit.Unit();
				_old = _aq_V;
			}
			(Result)->SetVector(_unit,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// faceforward(N,I)
void CqShaderExecEnv::SO_faceforward( IqShaderData* N, IqShaderData* I, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(N)->Class()==class_varying||__fVarying;
	__fVarying=(I)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_N;
			(N)->GetNormal(_aq_N,__iGrid);
			CqVector3D _aq_I;
			(I)->GetVector(_aq_I,__iGrid);
			CqVector3D Nref;
			Ng() ->GetNormal( Nref, __iGrid );
			TqFloat s = ( ( ( -_aq_I ) * Nref ) < 0.0f ) ? -1.0f : 1.0f;
			(Result)->SetNormal(_aq_N * s,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// faceforward(N,I,Nref)
void CqShaderExecEnv::SO_faceforward2( IqShaderData* N, IqShaderData* I, IqShaderData* Nref, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(N)->Class()==class_varying||__fVarying;
	__fVarying=(I)->Class()==class_varying||__fVarying;
	__fVarying=(Nref)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_N;
			(N)->GetNormal(_aq_N,__iGrid);
			CqVector3D _aq_I;
			(I)->GetVector(_aq_I,__iGrid);
			CqVector3D _aq_Nref;
			(Nref)->GetNormal(_aq_Nref,__iGrid);
			TqFloat s = ( ( ( -_aq_I ) * _aq_Nref ) < 0.0f ) ? -1.0f : 1.0f;
			(Result)->SetNormal(_aq_N * s,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// reflect(I,N)
void CqShaderExecEnv::SO_reflect( IqShaderData* I, IqShaderData* N, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(I)->Class()==class_varying||__fVarying;
	__fVarying=(N)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// reftact(I,N,eta)
void CqShaderExecEnv::SO_refract( IqShaderData* I, IqShaderData* N, IqShaderData* eta, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(I)->Class()==class_varying||__fVarying;
	__fVarying=(N)->Class()==class_varying||__fVarying;
	__fVarying=(eta)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// fresnel(I,N,eta,Kr,Kt)

void CqShaderExecEnv::SO_fresnel( IqShaderData* I, IqShaderData* N, IqShaderData* eta, IqShaderData* Kr, IqShaderData* Kt, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(I)->Class()==class_varying||__fVarying;
	__fVarying=(N)->Class()==class_varying||__fVarying;
	__fVarying=(eta)->Class()==class_varying||__fVarying;
	__fVarying=(Kr)->Class()==class_varying||__fVarying;
	__fVarying=(Kt)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// fresnel(I,N,eta,Kr,Kt,R,T)
void CqShaderExecEnv::SO_fresnel( IqShaderData* I, IqShaderData* N, IqShaderData* eta, IqShaderData* Kr, IqShaderData* Kt, IqShaderData* R, IqShaderData* T, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(I)->Class()==class_varying||__fVarying;
	__fVarying=(N)->Class()==class_varying||__fVarying;
	__fVarying=(eta)->Class()==class_varying||__fVarying;
	__fVarying=(Kr)->Class()==class_varying||__fVarying;
	__fVarying=(Kt)->Class()==class_varying||__fVarying;
	__fVarying=(R)->Class()==class_varying||__fVarying;
	__fVarying=(T)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < GridSize() ) && __fVarying);

	SO_reflect( I, N, R );
	SO_refract( I, N, eta, T );
}


//----------------------------------------------------------------------
// transform(s,s,P)
void CqShaderExecEnv::SO_transform( IqShaderData* fromspace, IqShaderData* tospace, IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	assert( pShader != 0 );

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	if ( NULL != QGetRenderContextI() )
	{
		__iGrid = 0;
		CqString _aq_fromspace;
		(fromspace)->GetString(_aq_fromspace,__iGrid);
		CqString _aq_tospace;
		(tospace)->GetString(_aq_tospace,__iGrid);
		const CqMatrix& mat = QGetRenderContextI() ->matSpaceToSpace( _aq_fromspace.c_str(), _aq_tospace.c_str(), pShader->matCurrent(), matObjectToWorld(), QGetRenderContextI()->Time() );


		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqVector3D _aq_p;
				(p)->GetPoint(_aq_p,__iGrid);
				(Result)->SetPoint(mat * _aq_p,__iGrid);
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqVector3D _aq_p;
				(p)->GetPoint(_aq_p,__iGrid);
				(Result)->SetPoint(_aq_p,__iGrid);
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
}


//----------------------------------------------------------------------
// transform(s,P)
void CqShaderExecEnv::SO_transform( IqShaderData* tospace, IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	assert( pShader != 0 );

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	if ( NULL != QGetRenderContextI() )
	{
		__iGrid = 0;
		CqString _aq_tospace;
		(tospace)->GetString(_aq_tospace,__iGrid);
		const CqMatrix& mat = QGetRenderContextI() ->matSpaceToSpace( "current", _aq_tospace.c_str(), pShader->matCurrent(), matObjectToWorld(), QGetRenderContextI()->Time() );


		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqVector3D _aq_p;
				(p)->GetPoint(_aq_p,__iGrid);
				(Result)->SetPoint(mat * _aq_p,__iGrid);
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqVector3D _aq_p;
				(p)->GetPoint(_aq_p,__iGrid);
				(Result)->SetPoint(_aq_p,__iGrid);
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
}


//----------------------------------------------------------------------
// transform(m,P)
void CqShaderExecEnv::SO_transformm( IqShaderData* tospace, IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	assert( pShader != 0 );

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqMatrix _aq_tospace;
			(tospace)->GetMatrix(_aq_tospace,__iGrid);
			CqVector3D _aq_p;
			(p)->GetPoint(_aq_p,__iGrid);
			(Result)->SetPoint(_aq_tospace * _aq_p,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// vtransform(s,s,P)
void CqShaderExecEnv::SO_vtransform( IqShaderData* fromspace, IqShaderData* tospace, IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	assert( pShader != 0 );

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	if ( NULL != QGetRenderContextI() )
	{
		__iGrid = 0;
		CqString _aq_fromspace;
		(fromspace)->GetString(_aq_fromspace,__iGrid);
		CqString _aq_tospace;
		(tospace)->GetString(_aq_tospace,__iGrid);
		const CqMatrix& mat = QGetRenderContextI() ->matVSpaceToSpace( _aq_fromspace.c_str(), _aq_tospace.c_str(), pShader->matCurrent(), matObjectToWorld(), QGetRenderContextI()->Time() );


		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqVector3D _aq_p;
				(p)->GetVector(_aq_p,__iGrid);
				(Result)->SetVector(mat * _aq_p,__iGrid);
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqVector3D _aq_p;
				(p)->GetVector(_aq_p,__iGrid);
				(Result)->SetVector(_aq_p,__iGrid);
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
}


//----------------------------------------------------------------------
// vtransform(s,P)
void CqShaderExecEnv::SO_vtransform( IqShaderData* tospace, IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	assert( pShader != 0 );

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	if ( NULL != QGetRenderContextI() )
	{
		__iGrid = 0;
		CqString _aq_tospace;
		(tospace)->GetString(_aq_tospace,__iGrid);
		const CqMatrix& mat = QGetRenderContextI() ->matVSpaceToSpace( "current", _aq_tospace.c_str(), pShader->matCurrent(), matObjectToWorld(), QGetRenderContextI()->Time() );


		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqVector3D _aq_p;
				(p)->GetVector(_aq_p,__iGrid);
				(Result)->SetVector(mat * _aq_p,__iGrid);
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqVector3D _aq_p;
				(p)->GetVector(_aq_p,__iGrid);
				(Result)->SetVector(_aq_p,__iGrid);
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
}


//----------------------------------------------------------------------
// vtransform(m,P)
void CqShaderExecEnv::SO_vtransformm( IqShaderData* tospace, IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	assert( pShader != 0 );

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqMatrix _aq_tospace;
			(tospace)->GetMatrix(_aq_tospace,__iGrid);
			CqVector3D _aq_p;
			(p)->GetVector(_aq_p,__iGrid);
			(Result)->SetVector(_aq_tospace * _aq_p,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// ntransform(s,s,P)
void CqShaderExecEnv::SO_ntransform( IqShaderData* fromspace, IqShaderData* tospace, IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	assert( pShader != 0 );

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	if ( NULL != QGetRenderContextI() )
	{
		__iGrid = 0;
		CqString _aq_fromspace;
		(fromspace)->GetString(_aq_fromspace,__iGrid);
		CqString _aq_tospace;
		(tospace)->GetString(_aq_tospace,__iGrid);
		const CqMatrix& mat = QGetRenderContextI() ->matNSpaceToSpace( _aq_fromspace.c_str(), _aq_tospace.c_str(), pShader->matCurrent(), matObjectToWorld(), QGetRenderContextI()->Time() );
		__iGrid = 0;

		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqVector3D _aq_p;
				(p)->GetNormal(_aq_p,__iGrid);
				(Result)->SetNormal(mat * _aq_p,__iGrid);
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqVector3D _aq_p;
				(p)->GetNormal(_aq_p,__iGrid);
				(Result)->SetNormal(_aq_p,__iGrid);
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
}


//----------------------------------------------------------------------
// ntransform(s,P)
void CqShaderExecEnv::SO_ntransform( IqShaderData* tospace, IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	assert( pShader != 0 );

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	if ( NULL != QGetRenderContextI() )
	{
		__iGrid = 0;
		CqString _aq_tospace;
		(tospace)->GetString(_aq_tospace,__iGrid);
		const CqMatrix& mat = QGetRenderContextI() ->matNSpaceToSpace( "current", _aq_tospace.c_str(), pShader->matCurrent(), matObjectToWorld(), QGetRenderContextI()->Time() );
		__iGrid = 0;

		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqVector3D _aq_p;
				(p)->GetNormal(_aq_p,__iGrid);
				(Result)->SetNormal(mat * _aq_p,__iGrid);
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqVector3D _aq_p;
				(p)->GetNormal(_aq_p,__iGrid);
				(Result)->SetNormal(_aq_p,__iGrid);
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
}


//----------------------------------------------------------------------
// ntransform(m,P)
void CqShaderExecEnv::SO_ntransformm( IqShaderData* tospace, IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	assert( pShader != 0 );

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqMatrix _aq_tospace;
			(tospace)->GetMatrix(_aq_tospace,__iGrid);
			CqVector3D _aq_p;
			(p)->GetNormal(_aq_p,__iGrid);
			(Result)->SetNormal(_aq_tospace * _aq_p,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// depth(P)
void CqShaderExecEnv::SO_depth( IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	if ( NULL == QGetRenderContextI() )
		return ;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_p;
			(p)->GetPoint(_aq_p,__iGrid);
			TqFloat d = _aq_p.z();
			d = ( d - QGetRenderContextI() ->GetFloatOption( "System", "Clipping" ) [ 0 ] ) /
			    ( QGetRenderContextI() ->GetFloatOption( "System", "Clipping" ) [ 1 ] - QGetRenderContextI() ->GetFloatOption( "System", "Clipping" ) [ 0 ] );
			(Result)->SetFloat(d,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// calculatenormal(P)
void CqShaderExecEnv::SO_calculatenormal( IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	CqVector3D Defvec( 0.0f, 0.0f, 0.0f );
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	// Find out if the orientation is inverted.
	TqBool CSO = pTransform()->GetHandedness(QGetRenderContextI()->Time());
	TqBool O = TqFalse;
	if( pAttributes() )
		O = pAttributes() ->GetIntegerAttribute( "System", "Orientation" ) [ 0 ] != 0;
	TqFloat neg = 1;
	if ( O != CSO )
		neg = -1;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			//CqVector3D	dPdu = SO_DuType<CqVector3D>( p, __iGrid, this, Defvec );
			//CqVector3D	dPdv = SO_DvType<CqVector3D>( p, __iGrid, this, Defvec );
			//CqVector3D	N = dPdu % dPdv;

			CqVector3D Ret, Ret2;
			TqInt uRes = uGridRes();
			TqInt GridX = __iGrid % ( uRes + 1 );

			CqVector3D v1, v2;
			if ( GridX < uRes )
			{
				p->GetValue( v1, __iGrid + 1 );
				p->GetValue( v2, __iGrid );
				Ret = ( v1 - v2 );
			}
			else
			{
				p->GetValue( v1, __iGrid );
				p->GetValue( v2, __iGrid - 1 );
				Ret = ( v1 - v2 );
			}
			TqInt vRes = vGridRes();
			TqInt GridY = ( __iGrid / ( uRes + 1 ) );

			if ( GridY < vRes )
			{
				p->GetValue( v1, __iGrid + uRes + 1 );
				p->GetValue( v2, __iGrid );
				Ret2 = ( v1 - v2 );
			}
			else
			{
				p->GetValue( v1, __iGrid );
				p->GetValue( v2, __iGrid - ( uRes + 1 ) );
				Ret2 = ( v1 - v2 );
			}

			CqVector3D N = Ret % Ret2;
			N.Unit();
			N *= neg;
			(Result)->SetNormal(N,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void CqShaderExecEnv::SO_cmix( IqShaderData* color0, IqShaderData* color1, IqShaderData* value, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(color0)->Class()==class_varying||__fVarying;
	__fVarying=(color1)->Class()==class_varying||__fVarying;
	__fVarying=(value)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqColor _aq_color0;
			(color0)->GetColor(_aq_color0,__iGrid);
			CqColor _aq_color1;
			(color1)->GetColor(_aq_color1,__iGrid);
			TqFloat _aq_value;
			(value)->GetFloat(_aq_value,__iGrid);
			CqColor c( ( 1.0f - _aq_value ) * _aq_color0 + _aq_value * _aq_color1 );
			(Result)->SetColor(c,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_fmix( IqShaderData* f0, IqShaderData* f1, IqShaderData* value, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(f0)->Class()==class_varying||__fVarying;
	__fVarying=(f1)->Class()==class_varying||__fVarying;
	__fVarying=(value)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_f0;
			(f0)->GetFloat(_aq_f0,__iGrid);
			TqFloat _aq_f1;
			(f1)->GetFloat(_aq_f1,__iGrid);
			TqFloat _aq_value;
			(value)->GetFloat(_aq_value,__iGrid);
			TqFloat f( ( 1.0f - _aq_value ) * _aq_f0 + _aq_value * _aq_f1 );
			(Result)->SetFloat(f,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_pmix( IqShaderData* p0, IqShaderData* p1, IqShaderData* value, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(p0)->Class()==class_varying||__fVarying;
	__fVarying=(p1)->Class()==class_varying||__fVarying;
	__fVarying=(value)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_p0;
			(p0)->GetPoint(_aq_p0,__iGrid);
			CqVector3D _aq_p1;
			(p1)->GetPoint(_aq_p1,__iGrid);
			TqFloat _aq_value;
			(value)->GetFloat(_aq_value,__iGrid);
			CqVector3D p( ( 1.0f - _aq_value ) * _aq_p0 + _aq_value * _aq_p1 );
			(Result)->SetPoint(p,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_vmix( IqShaderData* v0, IqShaderData* v1, IqShaderData* value, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(v0)->Class()==class_varying||__fVarying;
	__fVarying=(v1)->Class()==class_varying||__fVarying;
	__fVarying=(value)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_v0;
			(v0)->GetVector(_aq_v0,__iGrid);
			CqVector3D _aq_v1;
			(v1)->GetVector(_aq_v1,__iGrid);
			TqFloat _aq_value;
			(value)->GetFloat(_aq_value,__iGrid);
			CqVector3D v( ( 1.0f - _aq_value ) * _aq_v0 + _aq_value * _aq_v1 );
			(Result)->SetVector(v,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void	CqShaderExecEnv::SO_nmix( IqShaderData* n0, IqShaderData* n1, IqShaderData* value, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(n0)->Class()==class_varying||__fVarying;
	__fVarying=(n1)->Class()==class_varying||__fVarying;
	__fVarying=(value)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_n0;
			(n0)->GetNormal(_aq_n0,__iGrid);
			CqVector3D _aq_n1;
			(n1)->GetNormal(_aq_n1,__iGrid);
			TqFloat _aq_value;
			(value)->GetFloat(_aq_value,__iGrid);
			CqVector3D n( ( 1.0f - _aq_value ) * _aq_n0 + _aq_value * _aq_n1 );
			(Result)->SetNormal(n,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// texture(S)
void CqShaderExecEnv::SO_ftexture1( IqShaderData* name, IqShaderData* channel, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqFloat Deffloat = 0.0f;
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	if ( NULL == QGetRenderContextI() )
		return ;

	std::map<std::string, IqShaderData*> paramMap;
	GetTexParams(cParams, apParams, paramMap);

	TqFloat fill = 0.0f;
	if ( paramMap.find( "fill" ) != paramMap.end() )
		paramMap[ "fill" ] ->GetFloat( fill );

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	TqFloat _aq_channel;
	(channel)->GetFloat(_aq_channel,__iGrid);
	IqTextureMap* pTMap = QGetRenderContextI() ->GetTextureMap( _aq_name );
	TqFloat fdu = 0.0f, fdv = 0.0f;
	if ( m_pAttributes )
	{
		du() ->GetFloat( fdu );
		dv() ->GetFloat( fdv );
	}



	__fVarying = TqTrue;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		std::valarray<TqFloat> val;
		pTMap->PrepareSampleOptions( paramMap );

		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				TqFloat swidth = 0.0f, twidth = 0.0f;
				if ( fdu != 0.0f && fdv != 0.0f )
				{
					TqFloat dsdu = SO_DuType<TqFloat>( s(), __iGrid, this, Deffloat );
					swidth = fabs( dsdu * fdu );
					TqFloat dtdu = SO_DuType<TqFloat>( t(), __iGrid, this, Deffloat );
					twidth = fabs( dtdu * fdu );

					TqFloat dsdv = SO_DvType<TqFloat>( s(), __iGrid, this, Deffloat );
					swidth += fabs( dsdv * fdv );
					TqFloat dtdv = SO_DvType<TqFloat>( t(), __iGrid, this, Deffloat );
					twidth += fabs( dtdv * fdv );
				}
				else
				{
					swidth = 1.0 / pTMap->XRes();
					twidth = 1.0 / pTMap->YRes();
				}

				// Sample the texture.
				TqFloat fs, ft;
				s() ->GetFloat( fs, __iGrid );
				t() ->GetFloat( ft, __iGrid );
				pTMap->SampleMap( fs, ft, swidth, twidth, val );

				// Grab the appropriate channel.
				TqFloat fchan = _aq_channel;
				if ( fchan >= val.size() )
					(Result)->SetFloat(fill,__iGrid);
				else
					(Result)->SetFloat(val[ static_cast<unsigned int>( fchan ) ],__iGrid);
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				(Result)->SetFloat(0.0f,__iGrid);
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
}

//----------------------------------------------------------------------
// texture(S,F,F)
void CqShaderExecEnv::SO_ftexture2( IqShaderData* name, IqShaderData* channel, IqShaderData* s, IqShaderData* t, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqFloat Deffloat = 0.0f;
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	if ( NULL == QGetRenderContextI() )
		return ;

	std::map<std::string, IqShaderData*> paramMap;
	GetTexParams(cParams, apParams, paramMap);

	TqFloat fill = 0.0f;
	if ( paramMap.find( "fill" ) != paramMap.end() )
		paramMap[ "fill" ] ->GetFloat( fill );

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	TqFloat _aq_channel;
	(channel)->GetFloat(_aq_channel,__iGrid);
	IqTextureMap* pTMap = QGetRenderContextI() ->GetTextureMap( _aq_name );
	TqFloat fdu = 0.0f, fdv = 0.0f;
	if ( m_pAttributes )
	{
		du() ->GetFloat( fdu );
		dv() ->GetFloat( fdv );
	}



	__fVarying = TqTrue;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		std::valarray<TqFloat> val;
		pTMap->PrepareSampleOptions( paramMap );

		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				TqFloat swidth = 0.0f, twidth = 0.0f;
				if ( fdu != 0.0f && fdv != 0.0f )
				{
					TqFloat dsdu = SO_DuType<TqFloat>( s, __iGrid, this, Deffloat );
					swidth = fabs( dsdu * fdu );
					TqFloat dtdu = SO_DuType<TqFloat>( t, __iGrid, this, Deffloat );
					twidth = fabs( dtdu * fdu );

					TqFloat dsdv = SO_DvType<TqFloat>( s, __iGrid, this, Deffloat );
					swidth += fabs( dsdv * fdv );
					TqFloat dtdv = SO_DvType<TqFloat>( t, __iGrid, this, Deffloat );
					twidth += fabs( dtdv * fdv );
				}
				else
				{
					swidth = 1.0 / pTMap->XRes();
					twidth = 1.0 / pTMap->YRes();
				}

				// Sample the texture.
				TqFloat _aq_s;
				(s)->GetFloat(_aq_s,__iGrid);
				TqFloat _aq_t;
				(t)->GetFloat(_aq_t,__iGrid);
				pTMap->SampleMap( _aq_s, _aq_t, swidth, twidth, val );

				// Grab the appropriate channel.
				TqFloat fchan = _aq_channel;
				if ( fchan >= val.size() )
					(Result)->SetFloat(fill,__iGrid);
				else
					(Result)->SetFloat(val[ static_cast<unsigned int>( fchan ) ],__iGrid);
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				(Result)->SetFloat(0.0f,__iGrid);	// Default, completely lit
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
}

//----------------------------------------------------------------------
// texture(S,F,F,F,F,F,F,F,F)
void CqShaderExecEnv::SO_ftexture3( IqShaderData* name, IqShaderData* channel, IqShaderData* s1, IqShaderData* t1, IqShaderData* s2, IqShaderData* t2, IqShaderData* s3, IqShaderData* t3, IqShaderData* s4, IqShaderData* t4, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	if ( NULL == QGetRenderContextI() )
		return ;

	std::map<std::string, IqShaderData*> paramMap;
	GetTexParams(cParams, apParams, paramMap);

	TqFloat fill = 0.0f;
	if ( paramMap.find( "fill" ) != paramMap.end() )
		paramMap[ "fill" ] ->GetFloat( fill );

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	TqFloat _aq_channel;
	(channel)->GetFloat(_aq_channel,__iGrid);
	IqTextureMap* pTMap = QGetRenderContextI() ->GetTextureMap( _aq_name );



	__fVarying = TqTrue;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		std::valarray<TqFloat> val;
		pTMap->PrepareSampleOptions( paramMap );

		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{

				// Sample the texture.
				TqFloat _aq_s1;
				(s1)->GetFloat(_aq_s1,__iGrid);
				TqFloat _aq_t1;
				(t1)->GetFloat(_aq_t1,__iGrid);
				TqFloat _aq_s2;
				(s2)->GetFloat(_aq_s2,__iGrid);
				TqFloat _aq_t2;
				(t2)->GetFloat(_aq_t2,__iGrid);
				TqFloat _aq_s3;
				(s3)->GetFloat(_aq_s3,__iGrid);
				TqFloat _aq_t3;
				(t3)->GetFloat(_aq_t3,__iGrid);
				TqFloat _aq_s4;
				(s4)->GetFloat(_aq_s4,__iGrid);
				TqFloat _aq_t4;
				(t4)->GetFloat(_aq_t4,__iGrid);
				pTMap->SampleMap( _aq_s1, _aq_t1, _aq_s2, _aq_t2, _aq_s3, _aq_t3, _aq_s4, _aq_t4, val );

				// Grab the appropriate channel.
				TqFloat fchan = _aq_channel;
				if ( fchan >= val.size() )
					(Result)->SetFloat(fill,__iGrid);
				else
					(Result)->SetFloat(val[ static_cast<unsigned int>( fchan ) ],__iGrid);
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				(Result)->SetFloat(0.0f,__iGrid);	// Default, completely lit
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
}

//----------------------------------------------------------------------
// texture(S)
void CqShaderExecEnv::SO_ctexture1( IqShaderData* name, IqShaderData* channel, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqFloat Deffloat = 0.0f;
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	if ( NULL == QGetRenderContextI() )
		return ;

	std::map<std::string, IqShaderData*> paramMap;
	GetTexParams(cParams, apParams, paramMap);

	TqFloat fill = 0.0f;
	if ( paramMap.find( "fill" ) != paramMap.end() )
		paramMap[ "fill" ] ->GetFloat( fill );

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	TqFloat _aq_channel;
	(channel)->GetFloat(_aq_channel,__iGrid);
	IqTextureMap* pTMap = QGetRenderContextI() ->GetTextureMap( _aq_name );
	TqFloat fdu = 0.0f, fdv = 0.0f;
	if ( m_pAttributes )
	{
		du() ->GetFloat( fdu );
		dv() ->GetFloat( fdv );
	}



	__fVarying = TqTrue;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		std::valarray<TqFloat> val;
		pTMap->PrepareSampleOptions( paramMap );

		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				TqFloat swidth = 0.0f, twidth = 0.0f;
				if ( fdu != 0.0f && fdv != 0.0f )
				{
					TqFloat dsdu = SO_DuType<TqFloat>( s(), __iGrid, this, Deffloat );
					swidth = fabs( dsdu * fdu );
					TqFloat dsdv = SO_DvType<TqFloat>( s(), __iGrid, this, Deffloat );
					swidth += fabs( dsdv * fdv );

					TqFloat dtdu = SO_DuType<TqFloat>( t(), __iGrid, this, Deffloat );
					twidth = fabs( dtdu * fdu );
					TqFloat dtdv = SO_DvType<TqFloat>( t(), __iGrid, this, Deffloat );
					twidth += fabs( dtdv * fdv );
				}
				else
				{
					swidth = 1.0 / pTMap->XRes();
					twidth = 1.0 / pTMap->YRes();
				}

				// Sample the texture.
				TqFloat fs, ft;
				s() ->GetFloat( fs, __iGrid );
				t() ->GetFloat( ft, __iGrid );
				pTMap->SampleMap( fs, ft, swidth, twidth, val );

				// Grab the appropriate channel.
				TqFloat fchan = _aq_channel;
				CqColor colResult;
				colResult.SetfRed( (fchan >= val.size())? fill : val[ static_cast<unsigned int>( fchan ) ] );
				colResult.SetfGreen( ((fchan + 1) >= val.size())? fill : val[ static_cast<unsigned int>( fchan+1 ) ] );
				colResult.SetfBlue( ((fchan + 2) >= val.size())? fill : val[ static_cast<unsigned int>( fchan+2 ) ] );

				(Result)->SetColor(colResult,__iGrid);
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				(Result)->SetColor(CqColor( 0, 0, 0 ),__iGrid);	// Default, no color
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
}

//----------------------------------------------------------------------
// texture(S,F,F)
void CqShaderExecEnv::SO_ctexture2( IqShaderData* name, IqShaderData* channel, IqShaderData* s, IqShaderData* t, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqFloat Deffloat = 0.0f;
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	if ( NULL == QGetRenderContextI() )
		return ;

	std::map<std::string, IqShaderData*> paramMap;
	GetTexParams(cParams, apParams, paramMap);

	TqFloat fill = 0.0f;
	if ( paramMap.find( "fill" ) != paramMap.end() )
		paramMap[ "fill" ] ->GetFloat( fill );

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	TqFloat _aq_channel;
	(channel)->GetFloat(_aq_channel,__iGrid);
	IqTextureMap* pTMap = QGetRenderContextI() ->GetTextureMap( _aq_name );
	TqFloat fdu = 0.0f, fdv = 0.0f;
	if ( m_pAttributes )
	{
		du() ->GetFloat( fdu );
		dv() ->GetFloat( fdv );
	}



	__fVarying = TqTrue;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		std::valarray<TqFloat> val;
		pTMap->PrepareSampleOptions( paramMap );

		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				TqFloat swidth = 0.0f, twidth = 0.0f;
				if ( fdu != 0.0f && fdv != 0.0f )
				{
					TqFloat dsdu = SO_DuType<TqFloat>( s, __iGrid, this, Deffloat );
					swidth = fabs( dsdu * fdu );
					TqFloat dsdv = SO_DvType<TqFloat>( s, __iGrid, this, Deffloat );
					swidth += fabs( dsdv * fdv );

					TqFloat dtdu = SO_DuType<TqFloat>( t, __iGrid, this, Deffloat );
					twidth = fabs( dtdu * fdu );
					TqFloat dtdv = SO_DvType<TqFloat>( t, __iGrid, this, Deffloat );
					twidth += fabs( dtdv * fdv );
				}
				else
				{
					swidth = 1.0 / pTMap->XRes();
					twidth = 1.0 / pTMap->YRes();
				}

				// Sample the texture.
				TqFloat _aq_s;
				(s)->GetFloat(_aq_s,__iGrid);
				TqFloat _aq_t;
				(t)->GetFloat(_aq_t,__iGrid);
				pTMap->SampleMap( _aq_s, _aq_t, swidth, twidth, val );

				// Grab the appropriate channel.
				TqFloat fchan = _aq_channel;
				CqColor colResult;
				colResult.SetfRed( (fchan >= val.size())? fill : val[ static_cast<unsigned int>( fchan ) ] );
				colResult.SetfGreen( ((fchan + 1) >= val.size())? fill : val[ static_cast<unsigned int>( fchan+1 ) ] );
				colResult.SetfBlue( ((fchan + 2) >= val.size())? fill : val[ static_cast<unsigned int>( fchan+2 ) ] );

				(Result)->SetColor(colResult,__iGrid);
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				(Result)->SetColor(CqColor( 0, 0, 0 ),__iGrid);	// Default, completely lit
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
}

//----------------------------------------------------------------------
// texture(S,F,F,F,F,F,F,F,F)
void CqShaderExecEnv::SO_ctexture3( IqShaderData* name, IqShaderData* channel, IqShaderData* s1, IqShaderData* t1, IqShaderData* s2, IqShaderData* t2, IqShaderData* s3, IqShaderData* t3, IqShaderData* s4, IqShaderData* t4, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	if ( NULL == QGetRenderContextI() )
		return ;

	std::map<std::string, IqShaderData*> paramMap;
	GetTexParams(cParams, apParams, paramMap);

	TqFloat fill = 0.0f;
	if ( paramMap.find( "fill" ) != paramMap.end() )
		paramMap[ "fill" ] ->GetFloat( fill );

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	TqFloat _aq_channel;
	(channel)->GetFloat(_aq_channel,__iGrid);
	IqTextureMap* pTMap = QGetRenderContextI() ->GetTextureMap( _aq_name );



	__fVarying = TqTrue;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		std::valarray<TqFloat> val;
		pTMap->PrepareSampleOptions( paramMap );

		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				// Sample the texture.
				TqFloat _aq_s1;
				(s1)->GetFloat(_aq_s1,__iGrid);
				TqFloat _aq_t1;
				(t1)->GetFloat(_aq_t1,__iGrid);
				TqFloat _aq_s2;
				(s2)->GetFloat(_aq_s2,__iGrid);
				TqFloat _aq_t2;
				(t2)->GetFloat(_aq_t2,__iGrid);
				TqFloat _aq_s3;
				(s3)->GetFloat(_aq_s3,__iGrid);
				TqFloat _aq_t3;
				(t3)->GetFloat(_aq_t3,__iGrid);
				TqFloat _aq_s4;
				(s4)->GetFloat(_aq_s4,__iGrid);
				TqFloat _aq_t4;
				(t4)->GetFloat(_aq_t4,__iGrid);
				pTMap->SampleMap( _aq_s1, _aq_t1, _aq_s2, _aq_t2, _aq_s3, _aq_t3, _aq_s4, _aq_t4, val );

				// Grab the appropriate channel.
				TqFloat fchan = _aq_channel;
				CqColor colResult;
				colResult.SetfRed( (fchan >= val.size())? fill : val[ static_cast<unsigned int>( fchan ) ] );
				colResult.SetfGreen( ((fchan + 1) >= val.size())? fill : val[ static_cast<unsigned int>( fchan+1 ) ] );
				colResult.SetfBlue( ((fchan + 2) >= val.size())? fill : val[ static_cast<unsigned int>( fchan+2 ) ] );

				(Result)->SetColor(colResult,__iGrid);
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				(Result)->SetColor(CqColor( 0, 0, 0 ),__iGrid);	// Default, completely lit
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
}


//----------------------------------------------------------------------
// environment(S,P)
void CqShaderExecEnv::SO_fenvironment2( IqShaderData* name, IqShaderData* channel, IqShaderData* R, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	CqVector3D Defvec( 0.0f, 0.0f, 0.0f );
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	if ( NULL == QGetRenderContextI() )
		return ;

	std::map<std::string, IqShaderData*> paramMap;
	GetTexParams(cParams, apParams, paramMap);

	TqFloat fill = 0.0f;
	if ( paramMap.find( "fill" ) != paramMap.end() )
		paramMap[ "fill" ] ->GetFloat( fill );

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	TqFloat _aq_channel;
	(channel)->GetFloat(_aq_channel,__iGrid);
	IqTextureMap* pTMap = QGetRenderContextI() ->GetEnvironmentMap( _aq_name );

	// Try with LatLong map file
	if ( pTMap == 0 )
	{
		pTMap = QGetRenderContextI() ->GetLatLongMap( _aq_name );
	}
	TqFloat fdu = 0.0f, fdv = 0.0f;
	if ( m_pAttributes )
	{
		du() ->GetFloat( fdu );
		dv() ->GetFloat( fdv );
	}


	__fVarying = TqTrue;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		pTMap->PrepareSampleOptions( paramMap );
		std::valarray<TqFloat> val;

		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqVector3D swidth = 0.0f, twidth = 0.0f;
				if ( fdu != 0.0f )
				{
					CqVector3D dRdu = SO_DuType<CqVector3D>( R, __iGrid, this, Defvec );
					swidth = dRdu * fdu;
				}
				if ( fdv != 0.0f )
				{
					CqVector3D dRdv = SO_DvType<CqVector3D>( R, __iGrid, this, Defvec );
					twidth = dRdv * fdv;
				}
				else
				{
					swidth = CqVector3D( 1.0 / pTMap->XRes() );
					twidth = CqVector3D( 1.0 / pTMap->YRes() );
				}

				// Sample the texture.
				CqVector3D _aq_R;
				(R)->GetVector(_aq_R,__iGrid);
				pTMap->SampleMap( _aq_R, swidth, twidth, val );

				// Grab the appropriate channel.
				TqFloat fchan = _aq_channel;
				if ( fchan >= val.size() )
					(Result)->SetFloat(fill,__iGrid);
				else
					(Result)->SetFloat(val[ static_cast<unsigned int>( fchan ) ],__iGrid);
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				(Result)->SetFloat(0.0f,__iGrid);	// Default, completely lit
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
}

//----------------------------------------------------------------------
// environment(S,P,P,P,P)
void CqShaderExecEnv::SO_fenvironment3( IqShaderData* name, IqShaderData* channel, IqShaderData* R1, IqShaderData* R2, IqShaderData* R3, IqShaderData* R4, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	if ( NULL == QGetRenderContextI() )
		return ;

	std::map<std::string, IqShaderData*> paramMap;
	GetTexParams(cParams, apParams, paramMap);

	TqFloat fill = 0.0f;
	if ( paramMap.find( "fill" ) != paramMap.end() )
		paramMap[ "fill" ] ->GetFloat( fill );

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	TqFloat _aq_channel;
	(channel)->GetFloat(_aq_channel,__iGrid);
	IqTextureMap* pTMap = QGetRenderContextI() ->GetEnvironmentMap( _aq_name );
	// Try with LatLong map file
	if ( pTMap == 0 )
	{
		pTMap = QGetRenderContextI() ->GetLatLongMap( _aq_name );
	}


	__fVarying = TqTrue;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		std::valarray<TqFloat> val;
		pTMap->PrepareSampleOptions( paramMap );

		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				// Sample the texture.
				CqVector3D _aq_R1;
				(R1)->GetVector(_aq_R1,__iGrid);
				CqVector3D _aq_R2;
				(R2)->GetVector(_aq_R2,__iGrid);
				CqVector3D _aq_R3;
				(R3)->GetVector(_aq_R3,__iGrid);
				CqVector3D _aq_R4;
				(R4)->GetVector(_aq_R4,__iGrid);
				pTMap->SampleMap( _aq_R1, _aq_R2, _aq_R3, _aq_R4, val );

				// Grab the appropriate channel.
				TqFloat fchan = _aq_channel;
				if ( fchan >= val.size() )
					(Result)->SetFloat(fill,__iGrid);
				else
					(Result)->SetFloat(val[ static_cast<unsigned int>( fchan ) ],__iGrid);
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				(Result)->SetFloat(0.0f,__iGrid);	// Default, completely lit
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
}


//----------------------------------------------------------------------
// environment(S,P)
void CqShaderExecEnv::SO_cenvironment2( IqShaderData* name, IqShaderData* channel, IqShaderData* R, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	CqVector3D Defvec( 0.0f, 0.0f, 0.0f );
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	if ( NULL == QGetRenderContextI() )
		return ;

	std::map<std::string, IqShaderData*> paramMap;
	GetTexParams(cParams, apParams, paramMap);

	TqFloat fill = 0.0f;
	if ( paramMap.find( "fill" ) != paramMap.end() )
		paramMap[ "fill" ] ->GetFloat( fill );

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	TqFloat _aq_channel;
	(channel)->GetFloat(_aq_channel,__iGrid);
	IqTextureMap* pTMap = QGetRenderContextI() ->GetEnvironmentMap( _aq_name );
	// Try with LatLong map file
	if ( pTMap == 0 )
	{
		pTMap = QGetRenderContextI() ->GetLatLongMap( _aq_name );
	}
	TqFloat fdu = 0.0f, fdv = 0.0f;
	if ( m_pAttributes )
	{
		du() ->GetFloat( fdu );
		dv() ->GetFloat( fdv );
	}


	__fVarying = TqTrue;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		std::valarray<TqFloat> val;
		pTMap->PrepareSampleOptions( paramMap );

		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqVector3D swidth = 0.0f, twidth = 0.0f;
				if ( fdu != 0.0f )
				{
					CqVector3D dRdu = SO_DuType<CqVector3D>( R, __iGrid, this, Defvec );
					swidth = dRdu * fdu;
				}
				if ( fdv != 0.0f )
				{
					CqVector3D dRdv = SO_DvType<CqVector3D>( R, __iGrid, this, Defvec );
					twidth = dRdv * fdv;
				}
				else
				{
					swidth = CqVector3D( 1.0 / pTMap->XRes() );
					twidth = CqVector3D( 1.0 / pTMap->YRes() );
				}

				// Sample the texture.
				CqVector3D _aq_R;
				(R)->GetVector(_aq_R,__iGrid);
				pTMap->SampleMap( _aq_R, swidth, twidth, val );


				// Grab the appropriate channel.
				TqFloat fchan = _aq_channel;
				if ( fchan + 2 >= val.size() )
					(Result)->SetColor(CqColor( fill, fill, fill ),__iGrid);
				else
					(Result)->SetColor(CqColor( val[ static_cast<unsigned int>( fchan ) ], val[ static_cast<unsigned int>( fchan ) + 1 ], val[ static_cast<unsigned int>( fchan ) + 2 ] ),__iGrid);
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				(Result)->SetColor(CqColor( 0.0f, 0.0f, 0.0f ),__iGrid);	// Default, completely lit
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
}

//----------------------------------------------------------------------
// environment(S,P,P,P,P)
void CqShaderExecEnv::SO_cenvironment3( IqShaderData* name, IqShaderData* channel, IqShaderData* R1, IqShaderData* R2, IqShaderData* R3, IqShaderData* R4, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	if ( NULL == QGetRenderContextI() )
		return ;

	std::map<std::string, IqShaderData*> paramMap;
	GetTexParams(cParams, apParams, paramMap);

	TqFloat fill = 0.0f;
	if ( paramMap.find( "fill" ) != paramMap.end() )
		paramMap[ "fill" ] ->GetFloat( fill );

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	TqFloat _aq_channel;
	(channel)->GetFloat(_aq_channel,__iGrid);
	IqTextureMap* pTMap = QGetRenderContextI() ->GetEnvironmentMap( _aq_name );
	// Try with LatLong map file
	if ( pTMap == 0 )
	{
		pTMap = QGetRenderContextI() ->GetLatLongMap( _aq_name );
	}
	__iGrid = 0;

	__fVarying = TqTrue;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		std::valarray<TqFloat> val;
		pTMap->PrepareSampleOptions( paramMap );

		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				// Sample the texture.
				CqVector3D _aq_R1;
				(R1)->GetVector(_aq_R1,__iGrid);
				CqVector3D _aq_R2;
				(R2)->GetVector(_aq_R2,__iGrid);
				CqVector3D _aq_R3;
				(R3)->GetVector(_aq_R3,__iGrid);
				CqVector3D _aq_R4;
				(R4)->GetVector(_aq_R4,__iGrid);
				pTMap->SampleMap( _aq_R1, _aq_R2, _aq_R3, _aq_R4, val );

				// Grab the appropriate channel.
				TqFloat fchan = _aq_channel;
				if ( fchan + 2 >= val.size() )
					(Result)->SetColor(CqColor( fill, fill, fill ),__iGrid);
				else
					(Result)->SetColor(CqColor( val[ static_cast<unsigned int>( fchan ) ], val[ static_cast<unsigned int>( fchan ) + 1 ], val[ static_cast<unsigned int>( fchan ) + 2 ] ),__iGrid);
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				(Result)->SetColor(CqColor( 0.0f, 0.0f, 0.0f ),__iGrid);	// Default, completely lit
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
}

//----------------------------------------------------------------------
// bump(S)
void CqShaderExecEnv::SO_bump1( IqShaderData* name, IqShaderData* channel, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying = TqTrue;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			(Result)->SetPoint(CqVector3D( 0, 0, 0 ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// bump(S,F,F)
void CqShaderExecEnv::SO_bump2( IqShaderData* name, IqShaderData* channel, IqShaderData* s, IqShaderData* t, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying = TqTrue;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			(Result)->SetPoint(CqVector3D( 0, 0, 0 ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// bump(S,F,F,F,F,F,F,F,F)
void CqShaderExecEnv::SO_bump3( IqShaderData* name, IqShaderData* channel, IqShaderData* s1, IqShaderData* t1, IqShaderData* s2, IqShaderData* t2, IqShaderData* s3, IqShaderData* t3, IqShaderData* s4, IqShaderData* t4, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying = TqTrue;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			(Result)->SetPoint(CqVector3D( 0, 0, 0 ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// shadow(S,P)
void CqShaderExecEnv::SO_shadow( IqShaderData* name, IqShaderData* channel, IqShaderData* P, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	if ( NULL == QGetRenderContextI() )
		return ;

	std::map<std::string, IqShaderData*> paramMap;
	GetTexParams(cParams, apParams, paramMap);

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	TqFloat _aq_channel;
	(channel)->GetFloat(_aq_channel,__iGrid);
	IqTextureMap* pMap = QGetRenderContextI() ->GetShadowMap( _aq_name );


	__fVarying = TqTrue;
	if ( pMap != 0 && pMap->IsValid() )
	{
		std::valarray<TqFloat> fv;
		pMap->PrepareSampleOptions( paramMap );

		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{

			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqVector3D swidth = 0.0f, twidth = 0.0f;

				CqVector3D _aq_P;
				(P)->GetPoint(_aq_P,__iGrid);

				pMap->SampleMap( _aq_P, swidth, twidth, fv, 0 );
				(Result)->SetFloat(fv[ 0 ],__iGrid);
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				(Result)->SetFloat(0.0f,__iGrid);	// Default, completely lit
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
}

//----------------------------------------------------------------------
// shadow(S,P,P,P,P)

void CqShaderExecEnv::SO_shadow1( IqShaderData* name, IqShaderData* channel, IqShaderData* P1, IqShaderData* P2, IqShaderData* P3, IqShaderData* P4, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	if ( NULL == QGetRenderContextI() )
		return ;

	std::map<std::string, IqShaderData*> paramMap;
	GetTexParams(cParams, apParams, paramMap);

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	TqFloat _aq_channel;
	(channel)->GetFloat(_aq_channel,__iGrid);
	IqTextureMap* pMap = QGetRenderContextI() ->GetShadowMap( _aq_name );


	__fVarying = TqTrue;
	if ( pMap != 0 && pMap->IsValid() )
	{
		std::valarray<TqFloat> fv;
		pMap->PrepareSampleOptions( paramMap );

		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqVector3D _aq_P1;
				(P1)->GetPoint(_aq_P1,__iGrid);
				CqVector3D _aq_P2;
				(P2)->GetPoint(_aq_P2,__iGrid);
				CqVector3D _aq_P3;
				(P3)->GetPoint(_aq_P3,__iGrid);
				CqVector3D _aq_P4;
				(P4)->GetPoint(_aq_P4,__iGrid);
				pMap->SampleMap( _aq_P1, _aq_P2, _aq_P3, _aq_P4, fv, 0 );
				(Result)->SetFloat(fv[ 0 ],__iGrid);
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				(Result)->SetFloat(0.0f,__iGrid);	// Default, completely lit
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
}


//----------------------------------------------------------------------
// ambient()

void CqShaderExecEnv::SO_ambient( IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

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
			__fVarying = TqTrue;

			IqLightsource* lp = m_pAttributes ->pLight( light_index );
			if ( lp->pShader() ->fAmbient() )
			{
				__iGrid = 0;
				CqBitVector& RS = RunningState();
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
				while( ( ++__iGrid < GridSize() ) && __fVarying);
			}
		}
	}
}


//----------------------------------------------------------------------
// diffuse(N)
void CqShaderExecEnv::SO_diffuse( IqShaderData* N, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	// If the illuminance cache is already OK, then we don't need to bother filling in the illuminance parameters.
	if ( !m_IlluminanceCacheValid )
	{
		ValidateIlluminanceCache( NULL, N, pShader );
	}

	IqShaderData* pDefAngle = pShader->CreateTemporaryStorage( type_float, class_uniform );
	if ( NULL == pDefAngle )
		return ;

	pDefAngle->SetFloat( PIO2 );

	Result->SetColor( gColBlack );

	__fVarying = TqTrue;
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
			CqBitVector& RS = RunningState();
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
			while( ( ++__iGrid < GridSize() ) && __fVarying);
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
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	// If the illuminance cache is already OK, then we don't need to bother filling in the illuminance parameters.
	if ( !m_IlluminanceCacheValid )
	{
		ValidateIlluminanceCache( NULL, N, pShader );
	}

	IqShaderData* pDefAngle = pShader->CreateTemporaryStorage( type_float, class_uniform );
	if ( NULL == pDefAngle )
		return ;

	pDefAngle->SetFloat( PIO2 );

	Result->SetColor( gColBlack );
	__fVarying = TqTrue;

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
			CqBitVector& RS = RunningState();
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
					(Result)->SetColor(_aq_Result + colCl * pow( MAX( 0.0f, _aq_N * H ), 1.0f / ( _aq_roughness / 8.0f ) ),__iGrid);

				}
			}
			while( ( ++__iGrid < GridSize() ) && __fVarying);
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
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	IqShaderData * pnV = pShader ->CreateTemporaryStorage( type_vector, class_varying );
	IqShaderData* pnN = pShader ->CreateTemporaryStorage( type_normal, class_varying );
	IqShaderData* pR = pShader ->CreateTemporaryStorage( type_vector, class_varying );

	/// note: Not happy about this, the shader should take care of this at construction time,
	/// but at the moment, it can't guarantee the validity of the m_u/vGridRes data members.
	pnV->Initialise( uGridRes(), vGridRes() );
	pnN->Initialise( uGridRes(), vGridRes() );
	pR->Initialise( uGridRes(), vGridRes() );

	SO_normalize( V, pnV );
	SO_normalize( N, pnN );

	__fVarying = TqTrue;
	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D vecnV;
			pnV->GetVector( vecnV, __iGrid );
			pnV->SetVector( -vecnV, __iGrid );
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);

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

	pDefAngle->SetFloat( PIO2 );

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
			CqBitVector& RS = RunningState();
			do
			{
				if(!__fVarying || RS.Value( __iGrid ) )
				{

					// Get the light vector and color from the loght source.
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
					(Result)->SetColor(_aq_Result + colCl * pow( MAX( 0.0f, vecR * Ln ), _aq_size ),__iGrid);

				}
			}
			while( ( ++__iGrid < GridSize() ) && __fVarying);

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
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(P)->Class()==class_varying||__fVarying;
	__fVarying=(R)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			(Result)->SetColor(CqColor( 0, 0, 0 ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// illuminance(P,nsamples)
void CqShaderExecEnv::SO_illuminance( IqShaderData* Category, IqShaderData* P, IqShaderData* Axis, IqShaderData* Angle, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__iGrid = 0;
	CqString cat( "" );
	if ( NULL != Category )
		Category->GetString( cat );


	__fVarying = TqTrue;

	// Fill in the lightsource information, and transfer the results to the shader variables,
	if ( m_pAttributes != 0 )
	{
		IqLightsource * lp = m_pAttributes ->pLight( m_li );

		if ( NULL != Axis )
			__fVarying=(Axis)->Class()==class_varying||__fVarying;
		if ( NULL != Angle )
			__fVarying=(Angle)->Class()==class_varying||__fVarying;

		TqBool exec = TqTrue;

		if( cat.size() )
		{

			TqBool exclude = TqFalse;
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

				exec = TqFalse;
				// While no matching category has been found...
				TqInt tokenpos = 0, tokenend;
				while( 1 )
				{
					tokenend = lightcategories.find(',', tokenpos);
					CqString token = lightcategories.substr( tokenpos, tokenend );
					if( catname.compare( token ) == 0 )
					{
						if( !exclude )
						{
							exec = TqTrue;
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
			CqBitVector& RS = RunningState();
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
					TqFloat fAngle = PI;
					if ( NULL != Angle )
						Angle->GetFloat( fAngle, __iGrid );

					TqFloat cosangle = Ln * vecAxis;
					cosangle = CLAMP( cosangle, -1, 1 );
					if ( acos( cosangle ) > fAngle )
						m_CurrentState.SetValue( __iGrid, TqFalse );
					else
						m_CurrentState.SetValue( __iGrid, TqTrue );
				}
			}
			while( ( ++__iGrid < GridSize() ) && __fVarying);
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
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	TqBool res = TqTrue;
	if ( m_Illuminate > 0 )
		res = TqFalse;

	__fVarying = TqTrue;
	if ( res )
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
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
				TqFloat fAngle = PI;
				if ( NULL != Angle )
					Angle->GetFloat( fAngle, __iGrid );
				TqFloat cosangle = Ln * vecAxis;
				cosangle = CLAMP( cosangle, -1, 1 );
				if ( acos( cosangle ) > fAngle )
				{
					// Make sure we set the light color to zero in the areas that won't be lit.
					Cl() ->SetColor( CqColor( 0, 0, 0 ), __iGrid );
					m_CurrentState.SetValue( __iGrid, TqFalse );
				}
				else
					m_CurrentState.SetValue( __iGrid, TqTrue );
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
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
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	TqBool res = TqTrue;
	if ( m_Illuminate > 0 )
		res = TqFalse;

	__fVarying = TqTrue;
	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			if ( res )
			{
				CqVector3D vecAxis( 0.0f, 1.0f, 0.0f );
				if ( NULL != Axis )
					Axis->GetVector( vecAxis, __iGrid );
				L() ->SetVector( vecAxis, __iGrid );
				m_CurrentState.SetValue( __iGrid, TqTrue );
			}
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);

	m_Illuminate++;
}


void	CqShaderExecEnv::SO_solar( IqShader* pShader )
{
	SO_solar( NULL, NULL, pShader );
}


//----------------------------------------------------------------------
// printf(s,...)

void	CqShaderExecEnv::SO_printf( IqShaderData* str, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	if ( NULL == QGetRenderContextI() )
		return ;

	__fVarying=(str)->Class()==class_varying||__fVarying;
	TqInt ii;
	for ( ii = 0; ii < cParams; ii++ )
	{
		__fVarying=(apParams[ ii ])->Class()==class_varying||__fVarying;
	}

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqString _aq_str;
			(str)->GetString(_aq_str,__iGrid);
			CqString strA = SO_sprintf( _aq_str.c_str(), cParams, apParams, __iGrid );
			QGetRenderContextI() ->PrintString( strA.c_str() );
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// format(s,...)

void	CqShaderExecEnv::SO_format( IqShaderData* str, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(str)->Class()==class_varying||__fVarying;
	int ii;
	for ( ii = 0; ii < cParams; ii++ )
	{
		__fVarying=(apParams[ ii ])->Class()==class_varying||__fVarying;
	}
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqString _aq_str;
			(str)->GetString(_aq_str,__iGrid);
			CqString strA = SO_sprintf( _aq_str.c_str(), cParams, apParams, __iGrid );
			(Result)->SetString(strA,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// concat(s,s,...)

void	CqShaderExecEnv::SO_concat( IqShaderData* stra, IqShaderData* strb, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(stra)->Class()==class_varying||__fVarying;
	__fVarying=(strb)->Class()==class_varying||__fVarying;
	int ii;
	for ( ii = 0; ii < cParams; ii++ )
	{
		__fVarying=(apParams[ ii ])->Class()==class_varying||__fVarying;
	}
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqString _aq_stra;
			(stra)->GetString(_aq_stra,__iGrid);
			CqString strRes = _aq_stra;
			CqString _aq_strb;
			(strb)->GetString(_aq_strb,__iGrid);
			strRes += _aq_strb;
			for ( ii = 0; ii < cParams; ii++ )
			{
				CqString sn;
				apParams[ ii ] ->GetString( sn, __iGrid );
				strRes += sn;
			}
			(Result)->SetString(strRes,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// noise(v)
void CqShaderExecEnv::SO_fcellnoise1( IqShaderData* v, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			(Result)->SetFloat(m_cellnoise.FCellNoise1( _aq_v ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void CqShaderExecEnv::SO_ccellnoise1( IqShaderData* v, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			(Result)->SetColor(CqColor( m_cellnoise.PCellNoise1( _aq_v ) ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

void CqShaderExecEnv::SO_pcellnoise1( IqShaderData* v, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			(Result)->SetPoint(m_cellnoise.PCellNoise1( _aq_v ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// noise(u,v)
void CqShaderExecEnv::SO_fcellnoise2( IqShaderData* u, IqShaderData* v, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(u)->Class()==class_varying||__fVarying;
	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_u;
			(u)->GetFloat(_aq_u,__iGrid);
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			(Result)->SetFloat(m_cellnoise.FCellNoise2( _aq_u, _aq_v ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}
void CqShaderExecEnv::SO_ccellnoise2( IqShaderData* u, IqShaderData* v, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(u)->Class()==class_varying||__fVarying;
	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_u;
			(u)->GetFloat(_aq_u,__iGrid);
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			(Result)->SetColor(CqColor( m_cellnoise.PCellNoise2( _aq_u, _aq_v ) ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}
void CqShaderExecEnv::SO_pcellnoise2( IqShaderData* u, IqShaderData* v, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(u)->Class()==class_varying||__fVarying;
	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_u;
			(u)->GetFloat(_aq_u,__iGrid);
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			(Result)->SetPoint(m_cellnoise.PCellNoise2( _aq_u, _aq_v ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// noise(p)
void CqShaderExecEnv::SO_fcellnoise3( IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_p;
			(p)->GetPoint(_aq_p,__iGrid);
			(Result)->SetFloat(m_cellnoise.FCellNoise3( _aq_p ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}
void CqShaderExecEnv::SO_ccellnoise3( IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_p;
			(p)->GetPoint(_aq_p,__iGrid);
			(Result)->SetColor(CqColor( m_cellnoise.PCellNoise3( _aq_p ) ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}
void CqShaderExecEnv::SO_pcellnoise3( IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_p;
			(p)->GetPoint(_aq_p,__iGrid);
			(Result)->SetPoint(m_cellnoise.PCellNoise3( _aq_p ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// noise(p,f)
void CqShaderExecEnv::SO_fcellnoise4( IqShaderData* p, IqShaderData* v, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_p;
			(p)->GetPoint(_aq_p,__iGrid);
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			(Result)->SetFloat(m_cellnoise.FCellNoise4( _aq_p, _aq_v ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}
void CqShaderExecEnv::SO_ccellnoise4( IqShaderData* p, IqShaderData* v, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_p;
			(p)->GetPoint(_aq_p,__iGrid);
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			(Result)->SetColor(CqColor( m_cellnoise.PCellNoise4( _aq_p, _aq_v ) ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}
void CqShaderExecEnv::SO_pcellnoise4( IqShaderData* p, IqShaderData* v, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_p;
			(p)->GetPoint(_aq_p,__iGrid);
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			(Result)->SetPoint(m_cellnoise.PCellNoise4( _aq_p, _aq_v ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}



//----------------------------------------------------------------------
// atmosphere
//

void CqShaderExecEnv::SO_atmosphere( IqShaderData* name, IqShaderData* pV, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	boost::shared_ptr<IqShader> pAtmosphere;

	if ( NULL != m_pAttributes && (m_pAttributes ->pshadAtmosphere(QGetRenderContextI()->Time())) )
		pAtmosphere = m_pAttributes ->pshadAtmosphere(QGetRenderContextI()->Time());

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	if ( pAtmosphere )
		Result->SetValue( pAtmosphere->GetVariableValue( _aq_name.c_str(), pV ) ? 1.0f : 0.0f, 0 );
	else
		Result->SetValue( 0.0f, 0 );

}


//----------------------------------------------------------------------
// displacement
//

void CqShaderExecEnv::SO_displacement( IqShaderData* name, IqShaderData* pV, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	boost::shared_ptr<IqShader> pDisplacement;

	if ( NULL != m_pAttributes && (m_pAttributes ->pshadDisplacement(QGetRenderContextI()->Time())) )
		pDisplacement = m_pAttributes ->pshadDisplacement(QGetRenderContextI()->Time());

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	if ( pDisplacement )
		Result->SetValue( pDisplacement->GetVariableValue( _aq_name.c_str(), pV ) ? 1.0f : 0.0f, 0 );
	else
		Result->SetValue( 0.0f, 0 );

}


//----------------------------------------------------------------------
// lightsource
//

void CqShaderExecEnv::SO_lightsource( IqShaderData* name, IqShaderData* pV, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	// This should only be called within an Illuminance construct, so m_li should be valid.
	boost::shared_ptr<IqShader> pLightsource;

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	if ( m_li < m_pAttributes ->cLights() )
		pLightsource = m_pAttributes ->pLight( m_li ) ->pShader();
	if ( pLightsource )
		Result->SetValue( pLightsource->GetVariableValue( _aq_name.c_str(), pV ) ? 1.0f : 0.0f, 0 );
	else
		Result->SetValue( 0.0f, 0 );

}


//----------------------------------------------------------------------
// surface
//

void CqShaderExecEnv::SO_surface( IqShaderData* name, IqShaderData* pV, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	boost::shared_ptr<IqShader> pSurface;

	if ( GetCurrentSurface() &&
	        NULL != GetCurrentSurface()->pAttributes() &&
	        GetCurrentSurface()->pAttributes() ->pshadSurface(QGetRenderContextI()->Time()) )
		pSurface = GetCurrentSurface()->pAttributes() ->pshadSurface(QGetRenderContextI()->Time());

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	if ( pSurface )
		Result->SetValue( pSurface->GetVariableValue( _aq_name.c_str(), pV ) ? 1.0f : 0.0f, 0 );
	else
		Result->SetValue( 0.0f, 0 );

}


//----------------------------------------------------------------------
// attribute
//

void CqShaderExecEnv::SO_attribute( IqShaderData* name, IqShaderData* pV, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	//Find out if it is a specific attribute request
	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	TqFloat Ret = 0.0f;

	if ( _aq_name.compare( "ShadingRate" ) == 0 )
	{
		if ( pV->Type() == type_float )
		{
			pV->SetFloat( m_pAttributes ->GetFloatAttribute( "System", "ShadingRate" ) [ 0 ] );
			Ret = 1.0f;
		}
	}
	else if ( _aq_name.compare( "Sides" ) == 0 )
	{
		if ( pV->Type() == type_float )
		{
			pV->SetFloat( m_pAttributes ->GetIntegerAttribute( "System", "Sides" ) [ 0 ] );
			Ret = 1.0f;
		}
	}
	else if ( _aq_name.compare( "Matte" ) == 0 )
	{
		if ( pV->Type() == type_float )
		{
			pV->SetFloat( m_pAttributes ->GetIntegerAttribute( "System", "Matte" ) [ 0 ] );
			Ret = 1.0f;
		}
	}
	else
	{
		int iColon = _aq_name.find_first_of( ':' );
		if ( iColon >= 0 )
		{
			CqString strParam = _aq_name.substr( iColon + 1, _aq_name.size() - iColon - 1 );
			_aq_name = _aq_name.substr( 0, iColon );
			//const CqParameter* pParam = m_pAttributes ->pParameter( STRING( name ).c_str(), strParam.c_str() );

			Ret = 1.0f;
			if ( NULL != pAttributes() ->GetFloatAttribute( _aq_name.c_str(), strParam.c_str() ) )
				pV->SetFloat( pAttributes() ->GetFloatAttribute( _aq_name.c_str(), strParam.c_str() ) [ 0 ] );
			else if ( NULL != pAttributes() ->GetIntegerAttribute( _aq_name.c_str(), strParam.c_str() ) )
				pV->SetFloat( pAttributes() ->GetIntegerAttribute( _aq_name.c_str(), strParam.c_str() ) [ 0 ] );
			else if ( NULL != pAttributes() ->GetStringAttribute( _aq_name.c_str(), strParam.c_str() ) )
				pV->SetString( pAttributes() ->GetStringAttribute( _aq_name.c_str(), strParam.c_str() ) [ 0 ] );
			else if ( NULL != pAttributes() ->GetPointAttribute( _aq_name.c_str(), strParam.c_str() ) )
				pV->SetPoint( pAttributes() ->GetPointAttribute( _aq_name.c_str(), strParam.c_str() ) [ 0 ] );
			else if ( NULL != pAttributes() ->GetVectorAttribute( _aq_name.c_str(), strParam.c_str() ) )
				pV->SetVector( pAttributes() ->GetVectorAttribute( _aq_name.c_str(), strParam.c_str() ) [ 0 ] );
			else if ( NULL != pAttributes() ->GetNormalAttribute( _aq_name.c_str(), strParam.c_str() ) )
				pV->SetNormal( pAttributes() ->GetNormalAttribute( _aq_name.c_str(), strParam.c_str() ) [ 0 ] );
			else if ( NULL != pAttributes() ->GetColorAttribute( _aq_name.c_str(), strParam.c_str() ) )
				pV->SetColor( pAttributes() ->GetColorAttribute( _aq_name.c_str(), strParam.c_str() ) [ 0 ] );
			else if ( NULL != pAttributes() ->GetMatrixAttribute( _aq_name.c_str(), strParam.c_str() ) )
				pV->SetMatrix( pAttributes() ->GetMatrixAttribute( _aq_name.c_str(), strParam.c_str() ) [ 0 ] );
			else
				Ret = 0.0f;
		}
	}
	Result->SetValue( Ret, 0 );

}


//----------------------------------------------------------------------
// option
//

void CqShaderExecEnv::SO_option( IqShaderData* name, IqShaderData* pV, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	if ( NULL == QGetRenderContextI() )
		return ;

	__iGrid = 0;
	//Find out if it is a specific option request
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	TqFloat Ret = 0.0f;

	if ( _aq_name.compare( "Format" ) == 0 )
	{
		if ( pV->Type() == type_float &&
		        pV->ArrayLength() > 0 )
		{
			if ( pV->ArrayLength() >= 3 )
			{
				pV->ArrayEntry( 0 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetIntegerOption( "System", "Resolution" ) [ 0 ] ) );
				pV->ArrayEntry( 1 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetIntegerOption( "System", "Resolution" ) [ 1 ] ) );
				pV->ArrayEntry( 2 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "PixelAspectRatio" ) [ 2 ] ) );
				Ret = 1.0f;
			}
		}
	}
	else if ( _aq_name.compare( "CropWindow" ) == 0 )
	{
		if ( pV->Type() == type_float &&
		        pV->ArrayLength() > 0 )
		{
			if ( pV->ArrayLength() >= 4 )
			{
				pV->ArrayEntry( 0 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "CropWindow" ) [ 0 ] ) );
				pV->ArrayEntry( 1 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "CropWindow" ) [ 1 ] ) );
				pV->ArrayEntry( 2 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "CropWindow" ) [ 2 ] ) );
				pV->ArrayEntry( 3 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "CropWindow" ) [ 3 ] ) );
				Ret = 1.0f;
			}
		}
	}
	else if ( _aq_name.compare( "FrameAspectRatio" ) == 0 )
	{
		if ( pV->Type() == type_float )
		{
			pV->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "FrameAspectRatio" ) [ 0 ] ) );
			Ret = 1.0f;
		}
	}
	else if ( _aq_name.compare( "DepthOfField" ) == 0 )
	{
		if ( pV->Type() == type_float &&
		        pV->ArrayLength() > 0 )
		{
			if ( pV->ArrayLength() >= 3 )
			{
				pV->ArrayEntry( 0 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "DepthOfField" ) [ 0 ] ) );
				pV->ArrayEntry( 1 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "DepthOfField" ) [ 1 ] ) );
				pV->ArrayEntry( 2 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "DepthOfField" ) [ 2 ] ) );
				Ret = 1.0f;
			}
		}
	}
	else if ( _aq_name.compare( "Shutter" ) == 0 )
	{
		if ( pV->Type() == type_float &&
		        pV->ArrayLength() > 0 )
		{
			if ( pV->ArrayLength() >= 2 )
			{
				pV->ArrayEntry( 0 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "Shutter" ) [ 0 ] ) );
				pV->ArrayEntry( 1 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "Shutter" ) [ 1 ] ) );
				Ret = 1.0f;
			}
		}
	}
	else if ( _aq_name.compare( "Clipping" ) == 0 )
	{
		if ( pV->Type() == type_float &&
		        pV->ArrayLength() > 0 )
		{
			if ( pV->ArrayLength() >= 2 )
			{
				pV->ArrayEntry( 0 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "Clipping" ) [ 0 ] ) );
				pV->ArrayEntry( 1 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "Clipping" ) [ 1 ] ) );
				Ret = 1.0f;
			}
		}
	}
	else
	{
		CqString strName = _aq_name.c_str();
		int iColon = strName.find_first_of( ':' );
		if ( iColon >= 0 )
		{
			CqString strParam = strName.substr( iColon + 1, strName.size() - iColon - 1 );
			strName = strName.substr( 0, iColon );
			//const CqParameter* pParam = m_pAttributes ->pParameter( strName.c_str(), strParam.c_str() );

			Ret = 1.0f;

			if ( NULL != QGetRenderContextI() ->GetStringOption( strName.c_str(), strParam.c_str() ) )
				pV->SetString( QGetRenderContextI() ->GetStringOption( strName.c_str(), strParam.c_str() ) [ 0 ] );
			else if ( NULL != QGetRenderContextI() ->GetIntegerOption( strName.c_str(), strParam.c_str() ) )
				pV->SetFloat( QGetRenderContextI() ->GetIntegerOption( strName.c_str(), strParam.c_str() ) [ 0 ] );

			else if ( NULL != QGetRenderContextI() ->GetPointOption( strName.c_str(), strParam.c_str() ) )
				pV->SetPoint( QGetRenderContextI() ->GetPointOption( strName.c_str(), strParam.c_str() ) [ 0 ] );

			else if ( NULL != QGetRenderContextI() ->GetColorOption( strName.c_str(), strParam.c_str() ) )
				pV->SetColor( QGetRenderContextI() ->GetColorOption( strName.c_str(), strParam.c_str() ) [ 0 ] );
			else if ( NULL != QGetRenderContextI() ->GetFloatOption( strName.c_str(), strParam.c_str() ) )
				pV->SetFloat( QGetRenderContextI() ->GetFloatOption( strName.c_str(), strParam.c_str() ) [ 0 ] );
			/* did not deal with Vector, Normal and Matrix yet */
			else
				Ret = 0.0f;
		}
	}

	Result->SetValue( Ret, 0 );

}


//----------------------------------------------------------------------
// rendererinfo
//

void CqShaderExecEnv::SO_rendererinfo( IqShaderData* name, IqShaderData* pV, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	TqFloat Ret = 0.0f;

	if ( _aq_name.compare( "renderer" ) == 0 )
	{
		if ( pV->Type() == type_string )
		{
			pV->SetString( STRNAME );
			Ret = 1.0f;
		}
	}
	else if ( _aq_name.compare( "version" ) == 0 )
	{
		if ( pV->Type() == type_float &&
		        pV->ArrayLength() > 0 )
		{
			if ( pV->ArrayLength() >= 4 )
			{
				pV->ArrayEntry( 0 ) ->SetFloat( static_cast<TqFloat>( VERMAJOR ) );
				pV->ArrayEntry( 1 ) ->SetFloat( static_cast<TqFloat>( VERMINOR ) );
				pV->ArrayEntry( 2 ) ->SetFloat( static_cast<TqFloat>( BUILD ) );
				pV->ArrayEntry( 3 ) ->SetFloat( 0.0f );
				Ret = 1.0f;
			}
		}
	}
	else if ( _aq_name.compare( "versionstring" ) == 0 )
	{
		if ( pV->Type() == type_string )
		{
			pV->SetString( VERSION_STR );
			Ret = 1.0f;
		}
	}
	Result->SetValue( Ret, 0 );

}


//----------------------------------------------------------------------
// incident

void CqShaderExecEnv::SO_incident( IqShaderData* name, IqShaderData* pV, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__iGrid = 0;
	Result->SetValue( 0.0f, 0 );

}


//----------------------------------------------------------------------
// opposite

void CqShaderExecEnv::SO_opposite( IqShaderData* name, IqShaderData* pV, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__iGrid = 0;
	Result->SetValue( 0.0f, 0 );

}


//----------------------------------------------------------------------
// ctransform(s,s,c)
void CqShaderExecEnv::SO_ctransform( IqShaderData* fromspace, IqShaderData* tospace, IqShaderData* c, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(c)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqString strfromspace( "rgb" );
	if ( NULL != fromspace )
		fromspace->GetString( strfromspace );
	CqString _aq_tospace;
	(tospace)->GetString(_aq_tospace,__iGrid);


	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqColor _aq_c;
			(c)->GetColor(_aq_c,__iGrid);
			CqColor res( _aq_c );
			if ( strfromspace.compare( "hsv" ) == 0 )
				res = _aq_c.hsvtorgb();
			else if ( strfromspace.compare( "hsl" ) == 0 )
				res = _aq_c.hsltorgb();
			else if ( strfromspace.compare( "XYZ" ) == 0 )
				res = _aq_c.XYZtorgb();
			else if ( strfromspace.compare( "xyY" ) == 0 )
				res = _aq_c.xyYtorgb();
			else if ( strfromspace.compare( "YIQ" ) == 0 )
				res = _aq_c.YIQtorgb();

			if ( _aq_tospace.compare( "hsv" ) == 0 )
				res = _aq_c.rgbtohsv();
			else if ( _aq_tospace.compare( "hsl" ) == 0 )
				res = _aq_c.rgbtohsl();
			else if ( _aq_tospace.compare( "XYZ" ) == 0 )
				res = _aq_c.rgbtoXYZ();
			else if ( _aq_tospace.compare( "xyY" ) == 0 )
				res = _aq_c.rgbtoxyY();
			else if ( _aq_tospace.compare( "YIQ" ) == 0 )
				res = _aq_c.rgbtoYIQ();

			(Result)->SetColor(res,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// ctransform(s,c)
void CqShaderExecEnv::SO_ctransform( IqShaderData* tospace, IqShaderData* c, IqShaderData* Result, IqShader* pShader )
{
	assert( pShader != 0 );
	SO_ctransform( NULL, tospace, c, Result, pShader );
}


//----------------------------------------------------------------------
// ctransform(s,c)
void CqShaderExecEnv::SO_ptlined( IqShaderData* P0, IqShaderData* P1, IqShaderData* Q, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(P0)->Class()==class_varying||__fVarying;
	__fVarying=(P1)->Class()==class_varying||__fVarying;
	__fVarying=(Q)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_P0;
			(P0)->GetPoint(_aq_P0,__iGrid);
			CqVector3D _aq_P1;
			(P1)->GetPoint(_aq_P1,__iGrid);
			CqVector3D _aq_Q;
			(Q)->GetPoint(_aq_Q,__iGrid);
			CqVector3D kDiff = _aq_Q - _aq_P0;
			CqVector3D vecDir = _aq_P1 - _aq_P0;
			TqFloat fT = kDiff * vecDir;

			if ( fT <= 0.0f )
				fT = 0.0f;
			else
			{
				TqFloat fSqrLen = vecDir.Magnitude2();
				if ( fT >= fSqrLen )
				{
					fT = 1.0f;
					kDiff -= vecDir;
				}
				else
				{
					fT /= fSqrLen;
					kDiff -= fT * vecDir;
				}
			}
			(Result)->SetFloat(kDiff.Magnitude(),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


void	CqShaderExecEnv::SO_inversesqrt( IqShaderData* x, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(x)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_x;
			(x)->GetFloat(_aq_x,__iGrid);
			(Result)->SetFloat(1.0f / static_cast<TqFloat>( sqrt( _aq_x ) ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


void	CqShaderExecEnv::SO_match( IqShaderData* a, IqShaderData* b, IqShaderData* Result, IqShader* pShader )
{
	// TODO: Do this properly.
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__iGrid = 0;
	float r = 0.0f;
	CqString _aq_a;
	(a)->GetString(_aq_a,__iGrid);
	CqString _aq_b;
	(b)->GetString(_aq_b,__iGrid);
	if ( _aq_a.size() == 0 )
		r = 0.0f;
	else if ( _aq_b.size() == 0 )
		r = 0.0f;
	else
	{
		// Check the simple case first where both strings are identical
		TqUlong hasha = CqString::hash(_aq_a.c_str());
		TqUlong hashb = CqString::hash(_aq_b.c_str());

		if (hasha == hashb)
		{
			r = 1.0f;
		}
		else
		{
			/*
			* Match string b into a
			*/
			r = match(_aq_a.c_str(), _aq_b.c_str());
		}


	}

	(Result)->SetFloat(r,__iGrid);

}


//----------------------------------------------------------------------
// pnoise(u,period)
void CqShaderExecEnv::SO_fpnoise1( IqShaderData* v, IqShaderData* period, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(period)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			TqFloat _aq_period;
			(period)->GetFloat(_aq_period,__iGrid);
			(Result)->SetFloat( m_noise.FGPNoise1( _aq_v, _aq_period ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// pnoise(u,v,uperiod,vperiod)
void CqShaderExecEnv::SO_fpnoise2( IqShaderData* u, IqShaderData* v, IqShaderData* uperiod, IqShaderData* vperiod, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(u)->Class()==class_varying||__fVarying;
	__fVarying=(uperiod)->Class()==class_varying||__fVarying;
	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(vperiod)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_u;
			(u)->GetFloat(_aq_u,__iGrid);
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			TqFloat _aq_uperiod;
			(uperiod)->GetFloat(_aq_uperiod,__iGrid);
			TqFloat _aq_vperiod;
			(vperiod)->GetFloat(_aq_vperiod,__iGrid);

			(Result)->SetFloat( m_noise.FGPNoise2( _aq_u, _aq_v, _aq_uperiod, _aq_vperiod ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// pnoise(p,pperiod)
void CqShaderExecEnv::SO_fpnoise3( IqShaderData* p, IqShaderData* pperiod, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(pperiod)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_p;
			(p)->GetPoint(_aq_p,__iGrid);
			CqVector3D _aq_pperiod;
			(pperiod)->GetPoint(_aq_pperiod,__iGrid);

			(Result)->SetFloat( m_noise.FGPNoise3( _aq_p, _aq_pperiod ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// pnoise(p,t,pperiod,tperiod)
void CqShaderExecEnv::SO_fpnoise4( IqShaderData* p, IqShaderData* t, IqShaderData* pperiod, IqShaderData* tperiod, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(pperiod)->Class()==class_varying||__fVarying;
	__fVarying=(t)->Class()==class_varying||__fVarying;
	__fVarying=(tperiod)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_p;
			(p)->GetPoint(_aq_p,__iGrid);
			TqFloat _aq_t;
			(t)->GetFloat(_aq_t,__iGrid);
			CqVector3D _aq_pperiod;
			(pperiod)->GetPoint(_aq_pperiod,__iGrid);
			TqFloat _aq_tperiod;
			(tperiod)->GetFloat(_aq_tperiod,__iGrid);

			(Result)->SetFloat( m_noise.FGPNoise4( _aq_p, _aq_t, _aq_pperiod, _aq_tperiod ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// pnoise(u,period)
void CqShaderExecEnv::SO_cpnoise1( IqShaderData* v, IqShaderData* period, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(period)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			TqFloat _aq_period;
			(period)->GetFloat(_aq_period,__iGrid);
			(Result)->SetColor( m_noise.CGPNoise1( _aq_v, _aq_period ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// pnoise(u,v,uperiod,vperiod)
void CqShaderExecEnv::SO_cpnoise2( IqShaderData* u, IqShaderData* v, IqShaderData* uperiod, IqShaderData* vperiod, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(u)->Class()==class_varying||__fVarying;
	__fVarying=(uperiod)->Class()==class_varying||__fVarying;
	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(vperiod)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_u;
			(u)->GetFloat(_aq_u,__iGrid);
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			TqFloat _aq_uperiod;
			(uperiod)->GetFloat(_aq_uperiod,__iGrid);
			TqFloat _aq_vperiod;
			(vperiod)->GetFloat(_aq_vperiod,__iGrid);

			(Result)->SetColor( m_noise.CGPNoise2( _aq_u, _aq_v, _aq_uperiod, _aq_vperiod ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// pnoise(p,pperiod)
void CqShaderExecEnv::SO_cpnoise3( IqShaderData* p, IqShaderData* pperiod, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(pperiod)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_p;
			(p)->GetPoint(_aq_p,__iGrid);
			CqVector3D _aq_pperiod;
			(pperiod)->GetPoint(_aq_pperiod,__iGrid);

			(Result)->SetColor( m_noise.CGPNoise3( _aq_p, _aq_pperiod ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// pnoise(p,t,pperiod,tperiod)
void CqShaderExecEnv::SO_cpnoise4( IqShaderData* p, IqShaderData* t, IqShaderData* pperiod, IqShaderData* tperiod, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(pperiod)->Class()==class_varying||__fVarying;
	__fVarying=(t)->Class()==class_varying||__fVarying;
	__fVarying=(tperiod)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_p;
			(p)->GetPoint(_aq_p,__iGrid);
			TqFloat _aq_t;
			(t)->GetFloat(_aq_t,__iGrid);
			CqVector3D _aq_pperiod;
			(pperiod)->GetPoint(_aq_pperiod,__iGrid);
			TqFloat _aq_tperiod;
			(tperiod)->GetFloat(_aq_tperiod,__iGrid);

			(Result)->SetColor( m_noise.CGPNoise4( _aq_p, _aq_t, _aq_pperiod, _aq_tperiod ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// pnoise(u,period)
void CqShaderExecEnv::SO_ppnoise1( IqShaderData* v, IqShaderData* period, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(period)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			TqFloat _aq_period;
			(period)->GetFloat(_aq_period,__iGrid);
			(Result)->SetPoint( m_noise.PGPNoise1( _aq_v, _aq_period ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// pnoise(u,v,uperiod,vperiod)
void CqShaderExecEnv::SO_ppnoise2( IqShaderData* u, IqShaderData* v, IqShaderData* uperiod, IqShaderData* vperiod, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(u)->Class()==class_varying||__fVarying;
	__fVarying=(uperiod)->Class()==class_varying||__fVarying;
	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(vperiod)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_u;
			(u)->GetFloat(_aq_u,__iGrid);
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			TqFloat _aq_uperiod;
			(uperiod)->GetFloat(_aq_uperiod,__iGrid);
			TqFloat _aq_vperiod;
			(vperiod)->GetFloat(_aq_vperiod,__iGrid);

			(Result)->SetPoint( m_noise.PGPNoise2( _aq_u, _aq_v, _aq_uperiod, _aq_vperiod ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// pnoise(p,pperiod)
void CqShaderExecEnv::SO_ppnoise3( IqShaderData* p, IqShaderData* pperiod, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(pperiod)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_p;
			(p)->GetPoint(_aq_p,__iGrid);
			CqVector3D _aq_pperiod;
			(pperiod)->GetPoint(_aq_pperiod,__iGrid);

			(Result)->SetPoint( m_noise.PGPNoise3( _aq_p, _aq_pperiod ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// pnoise(p,t,pperiod,tperiod)
void CqShaderExecEnv::SO_ppnoise4( IqShaderData* p, IqShaderData* t, IqShaderData* pperiod, IqShaderData* tperiod, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(p)->Class()==class_varying||__fVarying;
	__fVarying=(pperiod)->Class()==class_varying||__fVarying;
	__fVarying=(t)->Class()==class_varying||__fVarying;
	__fVarying=(tperiod)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_p;
			(p)->GetPoint(_aq_p,__iGrid);
			TqFloat _aq_t;
			(t)->GetFloat(_aq_t,__iGrid);
			CqVector3D _aq_pperiod;
			(pperiod)->GetPoint(_aq_pperiod,__iGrid);
			TqFloat _aq_tperiod;
			(tperiod)->GetFloat(_aq_tperiod,__iGrid);

			(Result)->SetPoint( m_noise.PGPNoise4( _aq_p, _aq_t, _aq_pperiod, _aq_tperiod ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// rotate(Q,angle,P0,P1)
void CqShaderExecEnv::SO_rotate( IqShaderData* Q, IqShaderData* angle, IqShaderData* P0, IqShaderData* P1, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(Q)->Class()==class_varying||__fVarying;
	__fVarying=(angle)->Class()==class_varying||__fVarying;
	__fVarying=(P0)->Class()==class_varying||__fVarying;
	__fVarying=(P1)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_angle;
			(angle)->GetFloat(_aq_angle,__iGrid);
			CqVector3D _aq_Q;
			(Q)->GetVector(_aq_Q,__iGrid);
			CqVector3D _aq_P0;
			(P0)->GetPoint(_aq_P0,__iGrid);
			CqVector3D _aq_P1;
			(P1)->GetPoint(_aq_P1,__iGrid);
			CqMatrix matR( _aq_angle, _aq_P1 - _aq_P0 );

			CqVector3D	Res( _aq_Q );
			Res = matR * Res;

			(Result)->SetPoint(Res,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// filterstep(edge,s1)
void CqShaderExecEnv::SO_filterstep( IqShaderData* edge, IqShaderData* s1, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqFloat Deffloat = 0.0f;
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	float _pswidth=1.0f,_ptwidth=1.0f;
	GetFilterParams(cParams, apParams, _pswidth,_ptwidth);

	__fVarying=(edge)->Class()==class_varying||__fVarying;
	__fVarying=(s1)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	TqFloat fdu, fdv;
	du() ->GetFloat( fdu );
	dv() ->GetFloat( fdv );


	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_s1;
			(s1)->GetFloat(_aq_s1,__iGrid);
			TqFloat _aq_edge;
			(edge)->GetFloat(_aq_edge,__iGrid);
			TqFloat dsdu = SO_DuType<TqFloat>( s1, __iGrid, this, Deffloat );
			TqFloat dsdv = SO_DvType<TqFloat>( s1, __iGrid, this, Deffloat );

			TqFloat uwidth = fabs( dsdu * fdu );
			TqFloat vwidth = fabs( dsdv * fdv );

			TqFloat w = uwidth + vwidth;
			w *= _pswidth;

			(Result)->SetFloat(CLAMP( ( _aq_s1 + w / 2.0f - _aq_edge ) / w, 0, 1 ),__iGrid);

			//	TqFloat res  = RiCatmullRomFilter( FLOAT( s1 ) - FLOAT( edge ), 0, w, 0);
			//	SETFLOAT( Result, res );

			//	std::cout << res << std::endl;
			//	TqFloat res = 1.0f - CLAMP( ( FLOAT( s1 ) + w / 2.0f - FLOAT( edge ) ) / w, 0, 1 );
			//	if( res > 0.0f )
			//		std::cout << "Aqsis angle/dangle: " << FLOAT(s1) << ", edge: " << FLOAT(edge) << ", dsdu: " << dsdu << ", dsdv: " << dsdv << ", w: " << w << ", res: " << res << std::endl;
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// filterstep(edge,s1,s2)
void CqShaderExecEnv::SO_filterstep2( IqShaderData* edge, IqShaderData* s1, IqShaderData* s2, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	float _pswidth=1.0f,_ptwidth=1.0f;
	GetFilterParams(cParams, apParams, _pswidth,_ptwidth);

	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(edge)->Class()==class_varying||__fVarying;
	__fVarying=(s1)->Class()==class_varying||__fVarying;
	__fVarying=(s2)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_edge;
			(edge)->GetFloat(_aq_edge,__iGrid);
			TqFloat _aq_s1;
			(s1)->GetFloat(_aq_s1,__iGrid);
			TqFloat _aq_s2;
			(s2)->GetFloat(_aq_s2,__iGrid);
			TqFloat w = _aq_s2 - _aq_s1;
			w *= _pswidth;
			(Result)->SetFloat(CLAMP( ( _aq_s1 + w / 2.0f - _aq_edge ) / w, 0, 1 ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// specularbrdf(L,N,V,rough)
void CqShaderExecEnv::SO_specularbrdf( IqShaderData* L, IqShaderData* N, IqShaderData* V, IqShaderData* rough, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(L)->Class()==class_varying||__fVarying;
	__fVarying=(N)->Class()==class_varying||__fVarying;
	__fVarying=(V)->Class()==class_varying||__fVarying;
	__fVarying=(rough)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
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
			(Result)->SetColor(colCl * pow( MAX( 0.0f, _aq_N * H ), 1.0f / ( _aq_rough / 8.0f ) ),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// mtransform(s,s,M)
void CqShaderExecEnv::SO_mtransform( IqShaderData* fromspace, IqShaderData* tospace, IqShaderData* m, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	assert( pShader != 0 );

	__fVarying=(m)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	if ( NULL != QGetRenderContextI() )
	{
		__iGrid = 0;
		CqString _aq_fromspace;
		(fromspace)->GetString(_aq_fromspace,__iGrid);
		CqString _aq_tospace;
		(tospace)->GetString(_aq_tospace,__iGrid);
		const CqMatrix& mat = QGetRenderContextI() ->matNSpaceToSpace( _aq_fromspace.c_str(), _aq_tospace.c_str(), pShader->matCurrent(), matObjectToWorld(), QGetRenderContextI()->Time() );
		__iGrid = 0;

		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqMatrix _aq_m;
				(m)->GetMatrix(_aq_m,__iGrid);
				(Result)->SetMatrix(mat * _aq_m,__iGrid);
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqMatrix _aq_m;
				(m)->GetMatrix(_aq_m,__iGrid);
				(Result)->SetMatrix(_aq_m,__iGrid);
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
}


//----------------------------------------------------------------------
// mtransform(s,M)
void CqShaderExecEnv::SO_mtransform( IqShaderData* tospace, IqShaderData* m, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	assert( pShader != 0 );

	__fVarying=(m)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	if ( NULL != QGetRenderContextI() )
	{
		__iGrid = 0;
		CqString _aq_tospace;
		(tospace)->GetString(_aq_tospace,__iGrid);
		const CqMatrix& mat = QGetRenderContextI() ->matNSpaceToSpace( "current", _aq_tospace.c_str(), pShader->matCurrent(), matObjectToWorld(), QGetRenderContextI()->Time() );
		__iGrid = 0;

		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqMatrix _aq_m;
				(m)->GetMatrix(_aq_m,__iGrid);
				(Result)->SetMatrix(mat * _aq_m,__iGrid);
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqMatrix _aq_m;
				(m)->GetMatrix(_aq_m,__iGrid);
				(Result)->SetMatrix(_aq_m,__iGrid);
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
}



//----------------------------------------------------------------------
// determinant(m)
void CqShaderExecEnv::SO_determinant( IqShaderData* M, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(M)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqMatrix _aq_M;
			(M)->GetMatrix(_aq_M,__iGrid);
			(Result)->SetFloat(_aq_M.Determinant(),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// translate(m,v)
void CqShaderExecEnv::SO_mtranslate( IqShaderData* M, IqShaderData* V, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(M)->Class()==class_varying||__fVarying;
	__fVarying=(V)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqMatrix _aq_M;
			(M)->GetMatrix(_aq_M,__iGrid);
			CqVector3D _aq_V;
			(V)->GetVector(_aq_V,__iGrid);
			_aq_M.Translate( _aq_V );
			(Result)->SetMatrix(_aq_M,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// rotate(m,v)
void CqShaderExecEnv::SO_mrotate( IqShaderData* M, IqShaderData* angle, IqShaderData* axis, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(M)->Class()==class_varying||__fVarying;
	__fVarying=(angle)->Class()==class_varying||__fVarying;
	__fVarying=(axis)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqMatrix _aq_M;
			(M)->GetMatrix(_aq_M,__iGrid);
			TqFloat _aq_angle;
			(angle)->GetFloat(_aq_angle,__iGrid);
			CqVector3D _aq_axis;
			(axis)->GetVector(_aq_axis,__iGrid);
			_aq_M.Rotate( _aq_angle, _aq_axis );
			(Result)->SetMatrix(_aq_M,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}

//----------------------------------------------------------------------
// scale(m,p)
void CqShaderExecEnv::SO_mscale( IqShaderData* M, IqShaderData* S, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(M)->Class()==class_varying||__fVarying;
	__fVarying=(S)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_S;
			(S)->GetPoint(_aq_S,__iGrid);
			CqMatrix _aq_M;
			(M)->GetMatrix(_aq_M,__iGrid);
			_aq_M.Scale( _aq_S.x(), _aq_S.y(), _aq_S.z() );
			(Result)->SetMatrix(_aq_M,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// setmcomp(p,v)
void	CqShaderExecEnv::SO_setmcomp( IqShaderData* M, IqShaderData* r, IqShaderData* c, IqShaderData* v, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(M)->Class()==class_varying||__fVarying;
	__fVarying=(r)->Class()==class_varying||__fVarying;
	__fVarying=(c)->Class()==class_varying||__fVarying;
	__fVarying=(v)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqMatrix _aq_M;
			(M)->GetMatrix(_aq_M,__iGrid);
			TqFloat _aq_r;
			(r)->GetFloat(_aq_r,__iGrid);
			TqFloat _aq_c;
			(c)->GetFloat(_aq_c,__iGrid);
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			_aq_M [ static_cast<TqInt>( _aq_r ) ][ static_cast<TqInt>( _aq_c ) ] = _aq_v;
			_aq_M.SetfIdentity( TqFalse );
			M->SetValue( _aq_M, __iGrid );
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
void	CqShaderExecEnv::SO_fsplinea( IqShaderData* value, IqShaderData* a, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	assert( a->ArrayLength() > 0 );
	assert( a->Type() == type_float );

	TqInt	cParams = a->ArrayLength();
	CqSplineCubic spline( cParams );

	__fVarying=(value)->Class()==class_varying||__fVarying;
	__fVarying=(a)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_value;
			(value)->GetFloat(_aq_value,__iGrid);

			TqFloat fTemp;
			if ( _aq_value >= 1.0f )
			{
				a->ArrayEntry( cParams - 2 ) ->GetFloat( fTemp, __iGrid );
				Result->SetFloat( fTemp, __iGrid );
			}
			else if ( _aq_value <= 0.0f )
			{
				a->ArrayEntry( 1 ) ->GetFloat( fTemp, __iGrid );
				Result->SetFloat( fTemp, __iGrid );
			}
			else
			{
				TqInt j;
				for ( j = 0; j < cParams; j++ )
				{
					a->ArrayEntry( j ) ->GetFloat( fTemp, __iGrid );
					spline[ j ] = CqVector4D( fTemp, 0.0f, 0.0f, 1.0f );
				}

				CqVector4D	res = spline.Evaluate( _aq_value );
				(Result)->SetFloat(res.x(),__iGrid);
			}
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
void	CqShaderExecEnv::SO_csplinea( IqShaderData* value, IqShaderData* a, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	assert( a->ArrayLength() > 0 );
	assert( a->Type() == type_color );

	TqInt	cParams = a->ArrayLength();
	CqSplineCubic spline( cParams );
	CqColor colTemp;

	__fVarying=(value)->Class()==class_varying||__fVarying;
	__fVarying=(a)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_value;
			(value)->GetFloat(_aq_value,__iGrid);

			CqColor cTemp;
			if ( _aq_value >= 1.0f )
			{
				a->ArrayEntry( cParams - 2 ) ->GetColor( colTemp, __iGrid );
				Result->SetColor( colTemp, __iGrid );
			}
			else if ( _aq_value <= 0.0f )
			{
				a->ArrayEntry( 1 ) ->GetColor( colTemp, __iGrid );
				Result->SetColor( colTemp, __iGrid );
			}
			else
			{
				TqInt j;
				for ( j = 0; j < cParams; j++ )
				{
					a->ArrayEntry( j ) ->GetColor( colTemp, __iGrid );
					spline[ j ] = CqVector4D( colTemp.fRed(), colTemp.fGreen(), colTemp.fBlue(), 1.0f );
				}

				CqVector4D	res = spline.Evaluate( _aq_value );
				(Result)->SetColor(CqColor( res.x(), res.y(), res.z() ),__iGrid);
			}
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
void	CqShaderExecEnv::SO_psplinea( IqShaderData* value, IqShaderData* a, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	assert( a->ArrayLength() > 0 );
	assert( a->Type() == type_point );

	TqInt	cParams = a->ArrayLength();
	CqSplineCubic spline( cParams );
	CqVector3D vecTemp;

	__fVarying=(value)->Class()==class_varying||__fVarying;
	__fVarying=(a)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_value;
			(value)->GetFloat(_aq_value,__iGrid);

			CqVector3D vecTemp;
			if ( _aq_value >= 1.0f )
			{
				a->ArrayEntry( cParams - 2 ) ->GetPoint( vecTemp, __iGrid );
				Result->SetPoint( vecTemp, __iGrid );
			}
			else if ( _aq_value <= 0.0f )
			{
				a->ArrayEntry( 1 ) ->GetPoint( vecTemp, __iGrid );
				Result->SetPoint( vecTemp, __iGrid );
			}
			else
			{
				TqInt j;
				for ( j = 0; j < cParams; j++ )
				{
					a->ArrayEntry( j ) ->GetPoint( vecTemp, __iGrid );
					spline[ j ] = vecTemp;
				}

				CqVector3D	res = spline.Evaluate( _aq_value );
				(Result)->SetPoint(res,__iGrid);
			}
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
void	CqShaderExecEnv::SO_sfsplinea( IqShaderData* basis, IqShaderData* value, IqShaderData* a, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	assert( a->ArrayLength() > 0 );
	assert( a->Type() == type_float );

	TqInt	cParams = a->ArrayLength();
	CqSplineCubic spline( cParams );

	__fVarying=(value)->Class()==class_varying||__fVarying;
	__fVarying=(a)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqString _aq_basis;
	(basis)->GetString(_aq_basis,__iGrid);
	spline.SetBasis( _aq_basis );


	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_value;
			(value)->GetFloat(_aq_value,__iGrid);

			TqFloat fTemp;
			if ( _aq_value >= 1.0f )
			{
				a->ArrayEntry( cParams - 2 ) ->GetFloat( fTemp, __iGrid );
				Result->SetFloat( fTemp, __iGrid );
			}
			else if ( _aq_value <= 0.0f )
			{
				a->ArrayEntry( 1 ) ->GetFloat( fTemp, __iGrid );
				Result->SetFloat( fTemp, __iGrid );
			}
			else
			{
				TqInt j;
				for ( j = 0; j < cParams; j++ )
				{
					a->ArrayEntry( j ) ->GetFloat( fTemp, __iGrid );
					spline[ j ] = CqVector4D( fTemp, 0.0f, 0.0f, 1.0f );
				}

				CqVector4D	res = spline.Evaluate( _aq_value );
				(Result)->SetFloat(res.x(),__iGrid);
			}
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
void	CqShaderExecEnv::SO_scsplinea( IqShaderData* basis, IqShaderData* value, IqShaderData* a, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	assert( a->ArrayLength() > 0 );
	assert( a->Type() == type_color );

	TqInt	cParams = a->ArrayLength();
	CqSplineCubic spline( cParams );
	CqColor colTemp;

	__fVarying=(value)->Class()==class_varying||__fVarying;
	__fVarying=(a)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqString _aq_basis;
	(basis)->GetString(_aq_basis,__iGrid);
	spline.SetBasis( _aq_basis );


	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_value;
			(value)->GetFloat(_aq_value,__iGrid);

			CqColor colTemp;
			if ( _aq_value >= 1.0f )
			{
				a->ArrayEntry( cParams - 2 ) ->GetColor( colTemp, __iGrid );
				Result->SetColor( colTemp, __iGrid );
			}
			else if ( _aq_value <= 0.0f )
			{
				a->ArrayEntry( 1 ) ->GetColor( colTemp, __iGrid );
				Result->SetColor( colTemp, __iGrid );
			}
			else
			{
				TqInt j;
				for ( j = 0; j < cParams; j++ )
				{
					a->ArrayEntry( j ) ->GetColor( colTemp, __iGrid );
					spline[ j ] = CqVector4D( colTemp.fRed(), colTemp.fGreen(), colTemp.fBlue(), 1.0f );
				}

				CqVector4D	res = spline.Evaluate( _aq_value );
				(Result)->SetColor(CqColor( res.x(), res.y(), res.z() ),__iGrid);
			}
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
void	CqShaderExecEnv::SO_spsplinea( IqShaderData* basis, IqShaderData* value, IqShaderData* a, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	assert( a->ArrayLength() > 0 );
	assert( a->Type() == type_point );

	TqInt	cParams = a->ArrayLength();
	CqSplineCubic spline( cParams );
	CqVector3D vecTemp;

	__fVarying=(value)->Class()==class_varying||__fVarying;
	__fVarying=(a)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqString _aq_basis;
	(basis)->GetString(_aq_basis,__iGrid);
	spline.SetBasis( _aq_basis );


	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_value;
			(value)->GetFloat(_aq_value,__iGrid);

			CqVector3D vecTemp;
			if ( _aq_value >= 1.0f )
			{
				a->ArrayEntry( cParams - 2 ) ->GetPoint( vecTemp, __iGrid );
				Result->SetPoint( vecTemp, __iGrid );
			}
			else if ( _aq_value <= 0.0f )
			{
				a->ArrayEntry( 1 ) ->GetPoint( vecTemp, __iGrid );
				Result->SetPoint( vecTemp, __iGrid );
			}
			else
			{
				TqInt j;
				for ( j = 0; j < cParams; j++ )
				{
					a->ArrayEntry( j ) ->GetPoint( vecTemp, __iGrid );
					spline[ j ] = vecTemp;
				}

				CqVector3D	res = spline.Evaluate( _aq_value );
				(Result)->SetPoint(res,__iGrid);
			}
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// shadername()
void	CqShaderExecEnv::SO_shadername( IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			(Result)->SetString(pShader->strName(),__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// shadername(s)
void	CqShaderExecEnv::SO_shadername2( IqShaderData* shader, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	CqString strName( "" );
	CqString strShader;
	boost::shared_ptr<IqShader> pSurface;
	boost::shared_ptr<IqShader> pDisplacement;
	boost::shared_ptr<IqShader> pAtmosphere;
	if( m_pAttributes )
	{
		pSurface = m_pAttributes ->pshadSurface(QGetRenderContextI()->Time());
		pDisplacement = m_pAttributes ->pshadDisplacement(QGetRenderContextI()->Time());
		pAtmosphere = m_pAttributes ->pshadAtmosphere(QGetRenderContextI()->Time());
	}

	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			strName = "";
			CqString _aq_shader;
			(shader)->GetString(_aq_shader,__iGrid);
			if ( _aq_shader.compare( "surface" ) == 0 && pSurface )
				strName = pSurface->strName();
			else if ( _aq_shader.compare( "displacement" ) == 0 && pDisplacement )
				strName = pDisplacement->strName();
			else if ( _aq_shader.compare( "atmosphere" ) == 0 && pAtmosphere )
				strName = pAtmosphere->strName();
			(Result)->SetString(strName,__iGrid);
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
}


//----------------------------------------------------------------------
// textureinfo
// support resolution, type, channels, projectionmatrix(*) and viewingmatrix(*)
// User has to provide an array of TqFloat (2) for resolution
//                     an string for type
//                     an integer for channels
//                     an array of floats (16) for both projectionmatrix and viewingmatrix
//                     (*) the name must be a shadow map
//

void CqShaderExecEnv::SO_textureinfo( IqShaderData* name, IqShaderData* dataname, IqShaderData* pV, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	if ( NULL == QGetRenderContextI() )
		return ;

	TqFloat Ret = 0.0f;
	IqTextureMap* pMap = NULL;
	IqTextureMap *pSMap = NULL;
	IqTextureMap *pLMap = NULL;
	IqTextureMap *pEMap = NULL;
	IqTextureMap *pTMap = NULL;

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	CqString _aq_dataname;
	(dataname)->GetString(_aq_dataname,__iGrid);

	if ( !pMap && strstr( _aq_name.c_str(), ".tif" ) )
	{
		pTMap = QGetRenderContextI() ->GetTextureMap( _aq_name );
		if ( pTMap && ( pTMap->Type() == MapType_Texture ) )
		{
			pMap = pTMap;
		}
		else if ( pTMap )
			delete pTMap;
	}
	if ( !pMap )
	{
		pSMap = QGetRenderContextI() ->GetShadowMap( _aq_name );
		if ( pSMap && ( pSMap->Type() == MapType_Shadow ) )
		{
			pMap = pSMap;
		}
		else if ( pSMap )
			delete pSMap;
	}

	if ( !pMap )
	{
		pEMap = QGetRenderContextI() ->GetEnvironmentMap( _aq_name );
		if ( pEMap && ( pEMap->Type() == MapType_Environment ) )
		{
			pMap = pEMap;
		}
		else if ( pEMap )
			delete pEMap;
	}

	if ( !pMap )
	{
		pTMap = QGetRenderContextI() ->GetTextureMap( _aq_name );
		if ( pTMap && ( pTMap->Type() == MapType_Texture ) )
		{
			pMap = pTMap;
		}
		else if ( pTMap )
			delete pTMap;
	}


	if ( pMap == 0 )
	{
		(Result)->SetFloat(Ret,__iGrid);
		return ;
	}

	if ( _aq_dataname.compare( "exists" ) == 0 )
	{
		if ( pV->Type() == type_float )
		{
			pV->SetFloat( 1.0f );
			Ret = 1.0f;
		}
	}

	if ( _aq_dataname.compare( "resolution" ) == 0 )
	{
		if ( pV->Type() == type_float &&
		        pV->ArrayLength() > 0 )
		{

			if ( pV->ArrayLength() == 2 )
			{
				pV->ArrayEntry( 0 ) ->SetFloat( static_cast<TqFloat>( pMap->XRes() ) );
				pV->ArrayEntry( 1 ) ->SetFloat( static_cast<TqFloat>( pMap->YRes() ) );
				Ret = 1.0f;

			}
		}
	}
	if ( _aq_dataname.compare( "type" ) == 0 )
	{
		if ( pV->Type() == type_string )
		{
			if ( pMap->Type() == MapType_Texture )
			{
				pV->SetString( "texture" );
				Ret = 1.0f;

			}
			if ( pMap->Type() == MapType_Bump )
			{
				pV->SetString( "bump" );
				Ret = 1.0f;

			}

			if ( pMap->Type() == MapType_Shadow )
			{
				pV->SetString( "shadow" );
				Ret = 1.0f;

			}
			if ( pMap->Type() == MapType_Environment )
			{
				pV->SetString( "environment" );
				Ret = 1.0f;

			}
			if ( pMap->Type() == MapType_LatLong )
			{
				// both latlong/cube respond the same way according to BMRT
				// It makes sense since both use environment() shader fct.
				pV->SetString( "environment" );
				Ret = 1.0f;

			}


		}
	}

	if ( _aq_dataname.compare( "channels" ) == 0 )
	{
		if ( pV->Type() == type_float )
		{
			pV->SetFloat( static_cast<TqFloat>( pMap->SamplesPerPixel() ) );
			Ret = 1.0f;
		}

	}

	if ( _aq_dataname.compare( "viewingmatrix" ) == 0 )
	{
		if ( ( ( pV->Type() == type_float ) && ( pV->ArrayLength() == 16 ) ) ||
		        ( pV->Type() == type_matrix ) )
		{
			if ( pSMap )   // && pSMap->Type() == MapType_Shadow)
			{

				CqMatrix m = pSMap->GetMatrix( 0 );  /* WorldToCamera */
				if ( pV->ArrayLength() == 16 )
				{

					pV->ArrayEntry( 0 ) ->SetFloat( static_cast<TqFloat>( m[ 0 ][ 0 ] ) );
					pV->ArrayEntry( 1 ) ->SetFloat( static_cast<TqFloat>( m[ 0 ][ 1 ] ) );
					pV->ArrayEntry( 2 ) ->SetFloat( static_cast<TqFloat>( m[ 0 ][ 2 ] ) );
					pV->ArrayEntry( 3 ) ->SetFloat( static_cast<TqFloat>( m[ 0 ][ 3 ] ) );
					pV->ArrayEntry( 4 ) ->SetFloat( static_cast<TqFloat>( m[ 1 ][ 0 ] ) );
					pV->ArrayEntry( 5 ) ->SetFloat( static_cast<TqFloat>( m[ 1 ][ 1 ] ) );
					pV->ArrayEntry( 6 ) ->SetFloat( static_cast<TqFloat>( m[ 1 ][ 2 ] ) );
					pV->ArrayEntry( 7 ) ->SetFloat( static_cast<TqFloat>( m[ 1 ][ 3 ] ) );
					pV->ArrayEntry( 8 ) ->SetFloat( static_cast<TqFloat>( m[ 2 ][ 0 ] ) );
					pV->ArrayEntry( 9 ) ->SetFloat( static_cast<TqFloat>( m[ 2 ][ 1 ] ) );
					pV->ArrayEntry( 10 ) ->SetFloat( static_cast<TqFloat>( m[ 2 ][ 2 ] ) );
					pV->ArrayEntry( 11 ) ->SetFloat( static_cast<TqFloat>( m[ 2 ][ 3 ] ) );
					pV->ArrayEntry( 12 ) ->SetFloat( static_cast<TqFloat>( m[ 3 ][ 0 ] ) );
					pV->ArrayEntry( 13 ) ->SetFloat( static_cast<TqFloat>( m[ 3 ][ 1 ] ) );
					pV->ArrayEntry( 14 ) ->SetFloat( static_cast<TqFloat>( m[ 3 ][ 2 ] ) );
					pV->ArrayEntry( 15 ) ->SetFloat( static_cast<TqFloat>( m[ 3 ][ 3 ] ) );

				}
				else
				{
					pV->SetMatrix( m, 0 );
				}
				Ret = 1.0f;

			}

		}
	}

	if ( _aq_dataname.compare( "projectionmatrix" ) == 0 )
	{
		if ( ( ( pV->Type() == type_float ) && ( pV->ArrayLength() == 16 ) ) ||
		        ( pV->Type() == type_matrix ) )
		{
			if ( pSMap )    // && pSMap->Type() == MapType_Shadow)
			{

				CqMatrix m = pSMap->GetMatrix( 1 ); /* WorldToScreen */

				if ( pV->ArrayLength() == 16 )
				{
					pV->ArrayEntry( 0 ) ->SetFloat( static_cast<TqFloat>( m[ 0 ][ 0 ] ) );
					pV->ArrayEntry( 1 ) ->SetFloat( static_cast<TqFloat>( m[ 0 ][ 1 ] ) );
					pV->ArrayEntry( 2 ) ->SetFloat( static_cast<TqFloat>( m[ 0 ][ 2 ] ) );
					pV->ArrayEntry( 3 ) ->SetFloat( static_cast<TqFloat>( m[ 0 ][ 3 ] ) );
					pV->ArrayEntry( 4 ) ->SetFloat( static_cast<TqFloat>( m[ 1 ][ 0 ] ) );
					pV->ArrayEntry( 5 ) ->SetFloat( static_cast<TqFloat>( m[ 1 ][ 1 ] ) );
					pV->ArrayEntry( 6 ) ->SetFloat( static_cast<TqFloat>( m[ 1 ][ 2 ] ) );
					pV->ArrayEntry( 7 ) ->SetFloat( static_cast<TqFloat>( m[ 1 ][ 3 ] ) );
					pV->ArrayEntry( 8 ) ->SetFloat( static_cast<TqFloat>( m[ 2 ][ 0 ] ) );
					pV->ArrayEntry( 9 ) ->SetFloat( static_cast<TqFloat>( m[ 2 ][ 1 ] ) );
					pV->ArrayEntry( 10 ) ->SetFloat( static_cast<TqFloat>( m[ 2 ][ 2 ] ) );
					pV->ArrayEntry( 11 ) ->SetFloat( static_cast<TqFloat>( m[ 2 ][ 3 ] ) );
					pV->ArrayEntry( 12 ) ->SetFloat( static_cast<TqFloat>( m[ 3 ][ 0 ] ) );
					pV->ArrayEntry( 13 ) ->SetFloat( static_cast<TqFloat>( m[ 3 ][ 1 ] ) );
					pV->ArrayEntry( 14 ) ->SetFloat( static_cast<TqFloat>( m[ 3 ][ 2 ] ) );
					pV->ArrayEntry( 15 ) ->SetFloat( static_cast<TqFloat>( m[ 3 ][ 3 ] ) );


				}
				else
				{
					pV->SetMatrix( m, 0 );

				}
				Ret = 1.0f;
			}

		}
	}

	delete pMap;

	(Result)->SetFloat(Ret,__iGrid);

}

// SIGGRAPH 2002; Larry G. Bake functions

const int batchsize = 10240; // elements to buffer before writing
// Make sure we're thread-safe on those file writes

class BakingChannel
{
		// A "BakingChannel" is the buffer for a single baking output file.
		// We buffer up samples until "batchsize" has been accepted, then
		// write them all at once. This keeps us from constantly accessing
		// the disk. Note that we are careful to use a mutex to keep
		// simultaneous multithreaded writes from clobbering each other.
	public:
		// Constructors
		BakingChannel ( void ) : buffered( 0 ), data( NULL ), filename( NULL )
		{ }
		BakingChannel ( const char *_filename, int _elsize )
		{
			init ( _filename, _elsize );
		}
		// Initialize - allocate memory, etc.
		void init ( const char *_filename, int _elsize )
		{
			elsize = _elsize + 2;
			buffered = 0;
			data = new float [ elsize * batchsize ];
			filename = strdup ( _filename );
		}
		// Destructor: write buffered output, close file, deallocate
		~BakingChannel ()
		{
			writedata();
			free ( filename );
			delete [] data;
		}
		// Add one more data item
		void moredata ( float s, float t, float *newdata )
		{
			if ( buffered >= batchsize )
				writedata();
			float *f = data + elsize * buffered;
			f[ 0 ] = s;
			f[ 1 ] = t;
			for ( int j = 2; j < elsize; ++j )
				f[ j ] = newdata[ j - 2 ];
			++buffered;
		}
	private:
		int elsize; // element size (e.g., 3 for colors)
		int buffered; // how many elements are currently buffered
		float *data; // pointer to the allocated buffer (new'ed)
		char *filename; // pointer to filename (strdup'ed)
		// Write any buffered data to the file
		void writedata ( void )
		{

			if ( buffered > 0 && filename != NULL )
			{
				FILE * file = fopen ( filename, "a" );
				float *f = data;
				for ( int i = 0; i < buffered; ++i, f += elsize )
				{
					for ( int j = 0; j < elsize; ++j )
						fprintf ( file, "%g ", f[ j ] );
					fprintf ( file, "\n" );
				}
				fclose ( file );
			}

			buffered = 0;
		}
};

typedef std::map<std::string, BakingChannel> BakingData;


extern "C" BakingData *bake_init()
{
	BakingData * bd = new BakingData;

	return bd;
}
extern "C" void bake_done( BakingData *bd )
{
	delete bd; // Will destroy bd, and in turn all its BakingChannel's
}
// Workhorse routine -- look up the channel name, add a new BakingChannel
// if it doesn't exist, add one point's data to the channel.
extern "C" void bake ( BakingData *bd, const std::string &name,
	                       float s, float t, int elsize, float *data )
{
	BakingData::iterator found = bd->find ( name );

	if ( found == bd->end() )
	{
		// This named map doesn't yet exist
		( *bd ) [ name ] = BakingChannel();
		found = bd->find ( name );
		BakingChannel &bc = ( found->second );
		bc.init ( name.c_str(), elsize );
		bc.moredata ( s, t, data );
	}
	else
	{
		BakingChannel &bc = ( found->second );
		bc.moredata ( s, t, data );
	}
}

extern "C" int bake_f( BakingData *bd, char *name, float s, float t, float f )
{
	float * bakedata = ( float * ) & f;

	bake ( bd, name, s, t, 1, bakedata );
	return 0;
}
// for baking a triple -- just call bake with appropriate args
extern "C" int bake_3( BakingData *bd, char *name, float s, float t, float *bakedata )
{
	bake ( bd, name, s, t, 3, bakedata );
	return 0;
}



void CqShaderExecEnv::SO_bake_f( IqShaderData* name, IqShaderData* s, IqShaderData* t, IqShaderData* f, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(f)->Class()==class_varying||__fVarying;
	__fVarying=(s)->Class()==class_varying||__fVarying;
	__fVarying=(t)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	BakingData *bd = bake_init(  /*STRING( name).c_str() */ );


	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_s;
			(s)->GetFloat(_aq_s,__iGrid);
			TqFloat _aq_t;
			(t)->GetFloat(_aq_t,__iGrid);
			TqFloat _aq_f;
			(f)->GetFloat(_aq_f,__iGrid);
			bake_f( bd, ( char * ) _aq_name.c_str(), _aq_s, _aq_t, _aq_f );
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);

	__iGrid = 0;
	bake_done( bd );

}

void CqShaderExecEnv::SO_bake_3c( IqShaderData* name, IqShaderData* s, IqShaderData* t, IqShaderData* f, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(f)->Class()==class_varying||__fVarying;
	__fVarying=(s)->Class()==class_varying||__fVarying;
	__fVarying=(t)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	TqFloat rgb[ 3 ];

	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	BakingData *bd = bake_init(  /*(char *) STRING( name ).c_str()*/ );

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_s;
			(s)->GetFloat(_aq_s,__iGrid);
			TqFloat _aq_t;
			(t)->GetFloat(_aq_t,__iGrid);
			CqColor _aq_f;
			(f)->GetColor(_aq_f,__iGrid);
			_aq_f.GetColorRGB( &rgb[ 0 ], &rgb[ 1 ], &rgb[ 2 ] );
			bake_3( bd, ( char * ) _aq_name.c_str(), _aq_s, _aq_t, rgb );
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);
	__iGrid = 0;
	bake_done( bd );

}

void CqShaderExecEnv::SO_bake_3n( IqShaderData* name, IqShaderData* s, IqShaderData* t, IqShaderData* f, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(f)->Class()==class_varying||__fVarying;
	__fVarying=(s)->Class()==class_varying||__fVarying;
	__fVarying=(t)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	BakingData *bd = bake_init(  /*(char *) STRING( name ).c_str() */ );


	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_s;
			(s)->GetFloat(_aq_s,__iGrid);
			TqFloat _aq_t;
			(t)->GetFloat(_aq_t,__iGrid);
			CqVector3D _aq_f;
			(f)->GetNormal(_aq_f,__iGrid);
			TqFloat rgb[ 3 ];
			rgb[ 0 ] = _aq_f [ 0 ];
			rgb[ 1 ] = _aq_f [ 1 ];
			rgb[ 2 ] = _aq_f [ 2 ];
			bake_3( bd, ( char * ) _aq_name.c_str(), _aq_s, _aq_t, rgb );
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);

	__iGrid = 0;
	bake_done( bd );

}

void CqShaderExecEnv::SO_bake_3p( IqShaderData* name, IqShaderData* s, IqShaderData* t, IqShaderData* f, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(f)->Class()==class_varying||__fVarying;
	__fVarying=(s)->Class()==class_varying||__fVarying;
	__fVarying=(t)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	BakingData *bd = bake_init(  /*(char *) STRING( name ).c_str()  */ );


	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_s;
			(s)->GetFloat(_aq_s,__iGrid);
			TqFloat _aq_t;
			(t)->GetFloat(_aq_t,__iGrid);
			CqVector3D _aq_f;
			(f)->GetPoint(_aq_f,__iGrid);
			TqFloat rgb[ 3 ];
			rgb[ 0 ] = _aq_f [ 0 ];
			rgb[ 1 ] = _aq_f [ 1 ];
			rgb[ 2 ] = _aq_f [ 2 ];
			bake_3( bd, ( char * ) _aq_name.c_str(), _aq_s, _aq_t, rgb );
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);

	__iGrid = 0;
	bake_done( bd );

}

void CqShaderExecEnv::SO_bake_3v( IqShaderData* name, IqShaderData* s, IqShaderData* t, IqShaderData* f, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;
	__fVarying=(f)->Class()==class_varying||__fVarying;
	__fVarying=(s)->Class()==class_varying||__fVarying;
	__fVarying=(t)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	BakingData *bd = bake_init(  /*(char *) STRING( name ).c_str()  */ );


	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_s;
			(s)->GetFloat(_aq_s,__iGrid);
			TqFloat _aq_t;
			(t)->GetFloat(_aq_t,__iGrid);
			CqVector3D _aq_f;
			(f)->GetVector(_aq_f,__iGrid);
			TqFloat rgb[ 3 ];
			rgb[ 0 ] = _aq_f [ 0 ];
			rgb[ 1 ] = _aq_f [ 1 ];
			rgb[ 2 ] = _aq_f [ 2 ];
			bake_3( bd, ( char * ) _aq_name.c_str(), _aq_s, _aq_t, rgb );
		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);

	__iGrid = 0;
	bake_done( bd );

}


// We manually decalr th
void CqShaderExecEnv::SO_external( DSOMethod method, void *initData, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;

	__fVarying=(Result)->Class()==class_varying||__fVarying;
	int p;
	for ( p = 0;p < cParams;p++ )
	{
		__fVarying=(apParams[ p ])->Class()==class_varying||__fVarying;
	};

	int dso_argc = cParams + 1; // dso_argv[0] is used for the return value
	void **dso_argv = new void * [ dso_argc ] ;

	// create storage for the returned value
	switch ( Result->Type() )
	{

			case type_float:
			dso_argv[ 0 ] = ( void* ) new TqFloat;
			break;
			case type_point:
			case type_color:
			case type_triple:
			case type_vector:
			case type_normal:
			case type_hpoint:
			dso_argv[ 0 ] = ( void* ) new TqFloat[ 3 ];
			break;
			case type_string:
			dso_argv[ 0 ] = ( void* ) new STRING_DESC;
			( ( STRING_DESC* ) dso_argv[ 0 ] ) ->s = NULL;
			( ( STRING_DESC* ) dso_argv[ 0 ] ) ->bufflen = 0;
			break;
			case type_matrix:
			case type_sixteentuple:
			dso_argv[ 0 ] = ( void* ) new TqFloat[ 16 ];
			break;
			default:
			// Unhandled TYpe
			break;
	};

	// Allocate space for the arguments
	for ( p = 1;p <= cParams;p++ )
	{

		switch ( apParams[ p - 1 ] ->Type() )
		{
				case type_float:
				dso_argv[ p ] = ( void* ) new TqFloat;
				break;
				case type_hpoint:
				case type_point:
				case type_triple:  // This seems reasonable
				case type_vector:
				case type_normal:
				case type_color:
				dso_argv[ p ] = ( void* ) new TqFloat[ 3 ];
				break;
				case type_string:
				dso_argv[ p ] = ( void* ) new STRING_DESC;
				( ( STRING_DESC* ) dso_argv[ p ] ) ->s = NULL;
				( ( STRING_DESC* ) dso_argv[ p ] ) ->bufflen = 0;
				break;
				case type_matrix:
				case type_sixteentuple:
				dso_argv[ 0 ] = ( void* ) new TqFloat[ 16 ];
				break;
				default:
				// Unhandled TYpe
				break;
		};
	};


	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{

			// Convert the arguments to the required format for the DSO
			for ( p = 1;p <= cParams;p++ )
			{

				switch ( apParams[ p - 1 ] ->Type() )
				{
						case type_float:
						apParams[ p - 1 ] ->GetFloat( *( ( float* ) dso_argv[ p ] ), __iGrid );
						break;
						case type_hpoint:
						case type_point:
						{
							CqVector3D v;
							apParams[ p - 1 ] ->GetPoint( v, __iGrid );
							( ( float* ) dso_argv[ p ] ) [ 0 ] = v[ 0 ];
							( ( float* ) dso_argv[ p ] ) [ 1 ] = v[ 1 ];
							( ( float* ) dso_argv[ p ] ) [ 2 ] = v[ 2 ];
						}
						;
						break;
						case type_triple:  // This seems reasonable
						case type_vector:
						{
							CqVector3D v;
							apParams[ p - 1 ] ->GetVector( v, __iGrid );
							( ( float* ) dso_argv[ p ] ) [ 0 ] = v[ 0 ];
							( ( float* ) dso_argv[ p ] ) [ 1 ] = v[ 1 ];
							( ( float* ) dso_argv[ p ] ) [ 2 ] = v[ 2 ];
						}
						;
						break;
						case type_normal:
						{
							CqVector3D v;
							apParams[ p - 1 ] ->GetNormal( v, __iGrid );
							( ( float* ) dso_argv[ p ] ) [ 0 ] = v[ 0 ];
							( ( float* ) dso_argv[ p ] ) [ 1 ] = v[ 1 ];
							( ( float* ) dso_argv[ p ] ) [ 2 ] = v[ 2 ];
						}
						;
						break;
						case type_color:
						{
							CqColor c;
							apParams[ p - 1 ] ->GetColor( c, __iGrid );
							( ( float* ) dso_argv[ p ] ) [ 0 ] = c[ 0 ];
							( ( float* ) dso_argv[ p ] ) [ 1 ] = c[ 1 ];
							( ( float* ) dso_argv[ p ] ) [ 2 ] = c[ 2 ];
						}
						;
						break;
						case type_string:
						{
							CqString s;
							apParams[ p - 1 ] ->GetString( s, __iGrid );
							char *ps = new char[ s.size() + 1 ];
							strncpy ( ps, s.c_str(), s.size() + 1 );
							( ( STRING_DESC* ) dso_argv[ p ] ) ->s = ps;
							( ( STRING_DESC* ) dso_argv[ p ] ) ->bufflen = s.size() + 1;
						}
						;
						break;
						case type_matrix:
						case type_sixteentuple:
						{
							CqMatrix m;
							int r, c;
							apParams[ p - 1 ] ->GetMatrix( m, __iGrid );
							for ( r = 0; r < 4; r++ )
								for ( c = 0; c < 4; c++ )
									( ( TqFloat* ) dso_argv[ p ] ) [ ( r * 4 ) + c ] = m[ r ][ c ];
						}
						;
						break;
						default:
						// Unhandled TYpe
						break;
				};
			};

			// Atlast, we call the shadeop method, looks rather dull after all this effort.
			method( initData, dso_argc, dso_argv );

			// Pass the returned value back to aqsis
			switch ( Result->Type() )
			{

					case type_float:
					{
						TqFloat val = *( ( float* ) ( dso_argv[ 0 ] ) );
						Result->SetFloat( val, __iGrid );
					}
					;
					break;
					case type_hpoint:
					case type_point:
					{
						CqVector3D v;
						v[ 0 ] = ( ( float* ) dso_argv[ 0 ] ) [ 0 ];
						v[ 1 ] = ( ( float* ) dso_argv[ 0 ] ) [ 1 ];
						v[ 2 ] = ( ( float* ) dso_argv[ 0 ] ) [ 2 ];
						Result->SetPoint( v, __iGrid );
					}
					;
					break;
					case type_triple:  // This seems reasonable
					case type_vector:
					{
						CqVector3D v;
						v[ 0 ] = ( ( float* ) dso_argv[ 0 ] ) [ 0 ];
						v[ 1 ] = ( ( float* ) dso_argv[ 0 ] ) [ 1 ];
						v[ 2 ] = ( ( float* ) dso_argv[ 0 ] ) [ 2 ];
						Result->SetVector( v, __iGrid );
					}
					;
					break;
					case type_normal:
					{
						CqVector3D v;
						v[ 0 ] = ( ( float* ) dso_argv[ 0 ] ) [ 0 ];
						v[ 1 ] = ( ( float* ) dso_argv[ 0 ] ) [ 1 ];
						v[ 2 ] = ( ( float* ) dso_argv[ 0 ] ) [ 2 ];
						Result->SetNormal( v, __iGrid );
					}
					;
					break;
					case type_color:
					{
						CqColor c;
						c[ 0 ] = ( ( float* ) dso_argv[ 0 ] ) [ 0 ];
						c[ 1 ] = ( ( float* ) dso_argv[ 0 ] ) [ 1 ];
						c[ 2 ] = ( ( float* ) dso_argv[ 0 ] ) [ 2 ];
						Result->SetColor( c, __iGrid );
					}
					;
					break;
					case type_string:
					{
						CqString s( ( ( STRING_DESC* ) dso_argv[ 0 ] ) ->s );
						Result->SetString( s, __iGrid );
					}
					;
					break;
					case type_matrix:
					case type_sixteentuple:
					{
						CqMatrix m( ( float* ) dso_argv[ 0 ] );
						Result->SetMatrix( m, __iGrid );
					}
					;
					break;
					default:
					// Unhandled TYpe
					std::cout << "Unsupported type" << std::endl;
					break;
			};


			// Set the values that were altered by the Shadeop
			for ( p = 1;p <= cParams;p++ )
			{
				switch ( apParams[ p - 1 ] ->Type() )
				{
						case type_float:
						{
							TqFloat val = *( ( float* ) dso_argv[ p ] ) ;
							apParams[ p - 1 ] ->SetFloat( val, __iGrid );
						}
						;
						break;
						case type_hpoint:
						case type_point:
						{
							CqVector3D v;
							v[ 0 ] = ( ( float* ) dso_argv[ p ] ) [ 0 ];
							v[ 1 ] = ( ( float* ) dso_argv[ p ] ) [ 1 ];
							v[ 2 ] = ( ( float* ) dso_argv[ p ] ) [ 2 ];
							apParams[ p - 1 ] ->SetPoint( v, __iGrid );
						}
						;
						break;
						case type_triple:  // This seems reasonable
						case type_vector:
						{
							CqVector3D v;
							v[ 0 ] = ( ( float* ) dso_argv[ p ] ) [ 0 ];
							v[ 1 ] = ( ( float* ) dso_argv[ p ] ) [ 1 ];
							v[ 2 ] = ( ( float* ) dso_argv[ p ] ) [ 2 ];
							apParams[ p - 1 ] ->SetVector( v, __iGrid );
						}
						;
						break;
						case type_normal:
						{
							CqVector3D v;
							v[ 0 ] = ( ( float* ) dso_argv[ p ] ) [ 0 ];
							v[ 1 ] = ( ( float* ) dso_argv[ p ] ) [ 1 ];
							v[ 2 ] = ( ( float* ) dso_argv[ p ] ) [ 2 ];
							apParams[ p - 1 ] ->SetNormal( v, __iGrid );
						}
						;
						break;
						case type_color:
						{
							CqColor c;
							c[ 0 ] = ( ( float* ) dso_argv[ p ] ) [ 0 ];
							c[ 1 ] = ( ( float* ) dso_argv[ p ] ) [ 1 ];
							c[ 2 ] = ( ( float* ) dso_argv[ p ] ) [ 2 ];
							apParams[ p - 1 ] ->SetColor( c, __iGrid );
						}
						;
						break;
						case type_string:
						{
							CqString s( ( ( STRING_DESC* ) dso_argv[ p ] ) ->s );
							apParams[ p - 1 ] ->SetString( s, __iGrid );
						}
						;
						break;
						case type_matrix:
						case type_sixteentuple:
						{
							CqMatrix m( ( float* ) dso_argv[ p ] );
							apParams[ p - 1 ] ->SetMatrix( m, __iGrid );
						}
						;
						break;
						default:
						// Unhandled TYpe
						break;
				};
			};

		}
	}
	while( ( ++__iGrid < GridSize() ) && __fVarying);

	// Free up the storage allocated for the return type
	switch ( Result->Type() )
	{

			case type_float:
			delete ( float* ) dso_argv[ 0 ];
			break;
			case type_point:
			case type_triple:  // This seems reasonable
			case type_vector:
			case type_normal:
			case type_color:
			case type_hpoint:
			delete[] ( float* ) dso_argv[ 0 ];
			break;
			case type_string:  // Need to look into these
			delete ( STRING_DESC* ) dso_argv[ 0 ];
			break;
			case type_matrix:
			case type_sixteentuple:
			delete[] ( float* ) dso_argv[ 0 ];
			break;
			default:
			// Unhandled TYpe
			break;
	};

	// Free up the storage allocated for the args
	for ( p = 1;p <= cParams;p++ )
	{
		switch ( apParams[ p - 1 ] ->Type() )
		{
				case type_float:
				delete ( float* ) dso_argv[ p ];
				break;
				case type_point:
				case type_triple:
				case type_vector:
				case type_normal:
				case type_color:
				case type_hpoint:
				delete[] ( float* ) dso_argv[ p ];
				break;
				case type_string:
				delete ( STRING_DESC* ) dso_argv[ p ];
				break;
				case type_matrix:
				case type_sixteentuple:
				delete[] ( float* ) dso_argv[ p ];
				break;
				default:
				// Unhandled TYpe
				break;
		};
	};

	delete dso_argv;
}

//----------------------------------------------------------------------
// occlusion(occlmap,P,N,samples)
void CqShaderExecEnv::SO_occlusion( IqShaderData* occlmap, IqShaderData* channel, IqShaderData* P, IqShaderData* N, IqShaderData* samples, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqBool __fVarying=TqFalse;
	TqInt __iGrid;
	static CqMatrix *allmatrices= NULL;
	static TqUlong  allmatrix = 0;

	if ( NULL == QGetRenderContextI() )
		return ;

	std::map<std::string, IqShaderData*> paramMap;
	GetTexParams(cParams, apParams, paramMap);

	__iGrid = 0;
	CqString _aq_occlmap;
	(occlmap)->GetString(_aq_occlmap,__iGrid);
	CqVector3D _aq_N;
	(N)->GetNormal(_aq_N,__iGrid);
	TqFloat _aq_samples;
	(samples)->GetFloat(_aq_samples,__iGrid);
	IqTextureMap* pMap = QGetRenderContextI() ->GetShadowMap( _aq_occlmap );


	CqVector3D L(0,0,-1);

	__fVarying = TqTrue;
	if ( pMap != 0 && pMap->IsValid() )
	{
		std::valarray<TqFloat> fv;
		pMap->PrepareSampleOptions( paramMap );

		__iGrid = 0;
		CqBitVector& RS = RunningState();
		TqInt nPages = pMap->NumPages() - 1;
		if (CqString::hash(_aq_occlmap.c_str()) != allmatrix)
		{
			if (allmatrices)
				delete allmatrices;
			allmatrix = CqString::hash(_aq_occlmap.c_str());
			allmatrices = new CqMatrix[nPages+1];

			TqInt j = nPages;
			for( ; j >= 0; j-- )
			{
				allmatrices[j] = pMap->GetMatrix(2, j);
			}
		}
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				// Storage for the final combined occlusion value.
				TqFloat occlsum = 0.0f;
				TqFloat dotsum = 0.0f;

				CqVector3D swidth = 0.0f, twidth = 0.0f;

				CqVector3D _aq_N;
				CqVector3D _aq_P;
				(N)->GetNormal(_aq_N,__iGrid);
				(P)->GetPoint(_aq_P,__iGrid);
				TqInt i = nPages;
				for( ; i >= 0; i-- )
				{
					// Check if the lightsource is behind the sample.
					CqVector3D Nl = allmatrices[i] * _aq_N;
					TqFloat cosangle = Nl * L;
					if( cosangle < 0.0f )
						continue;
					pMap->SampleMap( _aq_P, swidth, twidth, fv, i );
					occlsum += cosangle * fv[0];
					dotsum += cosangle;
				}
				occlsum /= dotsum;
				(Result)->SetFloat(occlsum,__iGrid);
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				(Result)->SetFloat(0.0f,__iGrid);	// Default, completely lit
			}
		}
		while( ( ++__iGrid < GridSize() ) && __fVarying);
	}
}

static TqFloat match(const char *string, const char *pattern)
{
#if defined(REGEXP)
	int status;
	regex_t re;
	if (regcomp(&re, pattern, REG_EXTENDED|REG_NOSUB) != 0)
	{
		return(0.0f);      /* report error */
	}
	status = regexec(&re, string, (size_t) 0, NULL, 0);
	regfree(&re);

	if (status != 0)
	{
		return(0.0f);      /* report error */
	}
	return(1.0f);
#else

	return (TqFloat) (strstr(string, pattern) != 0);
#endif
}

END_NAMESPACE(    Aqsis )
//---------------------------------------------------------------------
