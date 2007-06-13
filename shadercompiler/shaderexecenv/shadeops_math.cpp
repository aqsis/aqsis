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
		\brief Implements the basic shader operations. (Math related)
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	"aqsis.h"

#include	<math.h>
#include	<map>
#include	<vector>
#include	<string>
#include	<stdio.h>

#include	"shaderexecenv.h"
#include	"shadervm.h"
#include	"irenderer.h"
#include	"version.h"
#include	"logging.h"

START_NAMESPACE(    Aqsis )

// If you want for 32 compiler as fast implementation of sqrt, inversesqrt and abs() just define FASTSQRT below
//#define FASTSQRT 
// Fast inverse square root
#ifdef FASTSQRT
inline	TqFloat isqrtf(float number) 
{
	TqLong		i;
	TqFloat		x2, y;
	const TqFloat threehalfs = 1.5F;

	x2 = number * 0.5F;
	y  = number;
	i  = * ( long * ) &y;
	i  = 0x5f3759df - ( i >> 1 );
	y  = * ( float * ) &i;
	y  = y * ( threehalfs - ( x2 * y * y ) );
	//	y  = y * ( threehalfs - ( x2 * y * y ) );			        // Da second iteration

	return y;
}


// Fast floating point absolute value
inline	TqFloat	absf(float f) 
{
	TqInt tmp = (*(int*)&f) & 0x7FFFFFFF;
	return *(TqFloat*)&tmp;
}

inline TqFloat sqrtf(float f)
{
	return 1.0f/isqrtf(f);
}
#endif

void	CqShaderExecEnv::SO_radians( IqShaderData* degrees, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(degrees)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_degrees( IqShaderData* radians, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(radians)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_sin( IqShaderData* a, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(a)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_asin( IqShaderData* a, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(a)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_cos( IqShaderData* a, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(a)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_acos( IqShaderData* a, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(a)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_tan( IqShaderData* a, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(a)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_atan( IqShaderData* yoverx, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(yoverx)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_atan( IqShaderData* y, IqShaderData* x, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(x)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_pow( IqShaderData* x, IqShaderData* y, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(x)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_exp( IqShaderData* x, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(x)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_sqrt( IqShaderData* x, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(x)->Class()==class_varying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_x;
			(x)->GetFloat(_aq_x,__iGrid);
#ifndef FASTSQRT
			(Result)->SetFloat(static_cast<TqFloat>( sqrt( _aq_x ) ),__iGrid);
#else
			(Result)->SetFloat(static_cast<TqFloat>( sqrtf( _aq_x ) ),__iGrid);
#endif
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_log( IqShaderData* x, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(x)->Class()==class_varying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_x;
			(x)->GetFloat(_aq_x,__iGrid);
			(Result)->SetFloat(static_cast<TqFloat>( ::log( _aq_x ) ),__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_mod( IqShaderData* a, IqShaderData* b, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(a)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// log(x,base)
void	CqShaderExecEnv::SO_log( IqShaderData* x, IqShaderData* base, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(x)->Class()==class_varying;
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
			(Result)->SetFloat(static_cast<TqFloat>( ::log( _aq_x ) / ::log( _aq_base ) ),__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


void	CqShaderExecEnv::SO_abs( IqShaderData* x, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(x)->Class()==class_varying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_x;
			(x)->GetFloat(_aq_x,__iGrid);
#ifndef FASTSQRT
			(Result)->SetFloat(static_cast<TqFloat>( fabs( _aq_x ) ),__iGrid);
#else
			(Result)->SetFloat(absf( _aq_x ),__iGrid);
#endif
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_sign( IqShaderData* x, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(x)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_min( IqShaderData* a, IqShaderData* b, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(a)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_max( IqShaderData* a, IqShaderData* b, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(a)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_pmin( IqShaderData* a, IqShaderData* b, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(a)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_pmax( IqShaderData* a, IqShaderData* b, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(a)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_cmin( IqShaderData* a, IqShaderData* b, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(a)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_cmax( IqShaderData* a, IqShaderData* b, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(a)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_clamp( IqShaderData* a, IqShaderData* _min, IqShaderData* _max, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(a)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_pclamp( IqShaderData* a, IqShaderData* _min, IqShaderData* _max, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(a)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_cclamp( IqShaderData* a, IqShaderData* _min, IqShaderData* _max, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(a)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_floor( IqShaderData* x, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(x)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_ceil( IqShaderData* x, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(x)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_round( IqShaderData* x, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;
	TqFloat res;

	__fVarying=(x)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_length( IqShaderData* V, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(V)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_distance( IqShaderData* P1, IqShaderData* P2, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(P1)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


void	CqShaderExecEnv::SO_inversesqrt( IqShaderData* x, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(x)->Class()==class_varying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_x;
			(x)->GetFloat(_aq_x,__iGrid);
#ifndef FASTSQRT
			(Result)->SetFloat(1.0f / static_cast<TqFloat>( sqrt( _aq_x ) ),__iGrid);
#else
			(Result)->SetFloat(isqrtf( _aq_x ) ,__iGrid);
#endif
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}



END_NAMESPACE(    Aqsis )
//---------------------------------------------------------------------
