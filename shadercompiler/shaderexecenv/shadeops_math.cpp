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
		\brief Implements the basic shader operations. (Math related)
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include "shaderexecenv.h"

#include "aqsismath.h"

namespace Aqsis {

namespace {

// Start of the complimentary math functions 

TqDouble deg(TqDouble f)
{
	return radToDeg(f);
}

TqDouble rad(TqDouble f)
{
	return degToRad(f);
}

TqDouble mod(TqDouble a, TqDouble b)
{
	TqInt n = static_cast<TqInt>( a / b );
	TqDouble a2 = a - n * b;
	if ( a2 < 0.0 )
		a2 += b;
	return a2;
}

TqDouble logBase(TqDouble x, TqDouble base)
{
	return  ( std::log(x) / std::log(base));
}

TqDouble sign(TqDouble f)
{
	return ( f < 0.0 ) ? -1.0 : 1.0;
}

TqDouble isqrt(TqDouble f) 
{
	return 1.0/sqrt(f);
}

TqDouble length(CqVector3D point)
{
	return point.Magnitude();
}

TqDouble distance(CqVector3D P1, CqVector3D P2)
{
	CqVector3D result(P1 - P2);
	return result.Magnitude();
}

TqDouble dpow(TqDouble x, TqDouble y)
{
	if ( x < 0.0 )
		y = std::floor( y );
	return std::pow(x,y);
}

// End of the complimentary math functions 
} // unnamed namespace


void	CqShaderExecEnv::SO_radians( IqShaderData* degrees, IqShaderData* Result, IqShader* pShader )
{
	MathOneParam(rad, degrees , Result);
}

void	CqShaderExecEnv::SO_degrees( IqShaderData* radians, IqShaderData* Result, IqShader* pShader )
{
	MathOneParam(deg, radians , Result);
}

void	CqShaderExecEnv::SO_sin( IqShaderData* a, IqShaderData* Result, IqShader* pShader )
{
	MathOneParam(std::sin, a , Result);
}

void	CqShaderExecEnv::SO_asin( IqShaderData* a, IqShaderData* Result, IqShader* pShader )
{
	MathOneParam(std::asin, a , Result);
}

void	CqShaderExecEnv::SO_cos( IqShaderData* a, IqShaderData* Result, IqShader* pShader )
{
	MathOneParam(std::cos, a , Result);
}

void	CqShaderExecEnv::SO_acos( IqShaderData* a, IqShaderData* Result, IqShader* pShader )
{
	MathOneParam(std::acos, a , Result);
}

void	CqShaderExecEnv::SO_tan( IqShaderData* a, IqShaderData* Result, IqShader* pShader ) {
	MathOneParam(std::tan, a , Result);
}

void	CqShaderExecEnv::SO_atan( IqShaderData* yoverx, IqShaderData* Result, IqShader* pShader )
{
	MathOneParam(std::atan, yoverx , Result);
}

void	CqShaderExecEnv::SO_atan( IqShaderData* y, IqShaderData* x, IqShaderData* Result, IqShader* pShader )
{
	MathTwoParams(std::atan2, y, x, Result);
}

void	CqShaderExecEnv::SO_pow( IqShaderData* x, IqShaderData* y, IqShaderData* Result, IqShader* pShader )
{
	MathTwoParams(dpow, x , y, Result);
}

void	CqShaderExecEnv::SO_exp( IqShaderData* x, IqShaderData* Result, IqShader* pShader )
{
	MathOneParam(std::exp, x , Result);
}

void	CqShaderExecEnv::SO_sqrt( IqShaderData* x, IqShaderData* Result, IqShader* pShader )
{
	MathOneParam(std::sqrt, x , Result);
}

void	CqShaderExecEnv::SO_log( IqShaderData* x, IqShaderData* Result, IqShader* pShader )
{
	MathOneParam(std::log, x , Result);
}

void	CqShaderExecEnv::SO_mod( IqShaderData* a, IqShaderData* b, IqShaderData* Result, IqShader* pShader )
{
	MathTwoParams(mod, a, b , Result);
}

//----------------------------------------------------------------------
// log(x,base)
void	CqShaderExecEnv::SO_log( IqShaderData* x, IqShaderData* base, IqShaderData* Result, IqShader* pShader )
{
	MathTwoParams(logBase, x, base, Result);
}


void	CqShaderExecEnv::SO_abs( IqShaderData* x, IqShaderData* Result, IqShader* pShader )
{
	MathOneParam(std::fabs, x , Result);
}

void	CqShaderExecEnv::SO_sign( IqShaderData* x, IqShaderData* Result, IqShader* pShader )
{
	MathOneParam(sign, x , Result);
}

void	CqShaderExecEnv::SO_min( IqShaderData* a, IqShaderData* b, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(a)->Class()==class_varying;
	__fVarying=(b)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_a;
			(a)->GetFloat(_aq_a,__iGrid);
			TqFloat _aq_b;
			(b)->GetFloat(_aq_b,__iGrid);
			TqFloat fRes = min( _aq_a, _aq_b );
			while ( cParams-- > 0 )
			{
				TqFloat fn;
				apParams[ cParams ] ->GetFloat( fn, __iGrid );
				fRes = Aqsis::min( fRes, fn );
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
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_a;
			(a)->GetFloat(_aq_a,__iGrid);
			TqFloat _aq_b;
			(b)->GetFloat(_aq_b,__iGrid);
			TqFloat fRes = max( _aq_a, _aq_b );
			while ( cParams-- > 0 )
			{
				TqFloat fn;
				apParams[ cParams ] ->GetFloat( fn, __iGrid );
				fRes = Aqsis::max( fRes, fn );
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
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_a;
			(a)->GetPoint(_aq_a,__iGrid);
			CqVector3D _aq_b;
			(b)->GetPoint(_aq_b,__iGrid);
			CqVector3D res = min( _aq_a, _aq_b );
			while ( cParams-- > 0 )
			{
				CqVector3D pn;
				apParams[ cParams ] ->GetPoint( pn, __iGrid );
				res = Aqsis::min( res, pn );
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
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_a;
			(a)->GetPoint(_aq_a,__iGrid);
			CqVector3D _aq_b;
			(b)->GetPoint(_aq_b,__iGrid);
			CqVector3D res = max( _aq_a, _aq_b );
			while ( cParams-- > 0 )
			{
				CqVector3D pn;
				apParams[ cParams ] ->GetPoint( pn, __iGrid );
				res = Aqsis::max( res, pn );
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
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqColor _aq_a;
			(a)->GetColor(_aq_a,__iGrid);
			CqColor _aq_b;
			(b)->GetColor(_aq_b,__iGrid);
			CqColor res = min( _aq_a, _aq_b );
			while ( cParams-- > 0 )
			{
				CqColor cn;
				apParams[ cParams ] ->GetColor( cn, __iGrid );
				res = Aqsis::min( res, cn );
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
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqColor _aq_a;
			(a)->GetColor(_aq_a,__iGrid);
			CqColor _aq_b;
			(b)->GetColor(_aq_b,__iGrid);
			CqColor res = max( _aq_a, _aq_b );
			while ( cParams-- > 0 )
			{
				CqColor cn;
				apParams[ cParams ] ->GetColor( cn, __iGrid );
				res = Aqsis::max( res, cn );
			}
			(Result)->SetColor(res,__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_clamp( IqShaderData* a, IqShaderData* _min, IqShaderData* _max, IqShaderData* Result, IqShader* pShader )
{
	MathThreeParams(clamp, a, _min, _max, Result);
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
	const CqBitVector& RS = RunningState();
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
			(Result)->SetPoint(clamp( _aq_a, _aq__min, _aq__max ),__iGrid);
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
	const CqBitVector& RS = RunningState();
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
			(Result)->SetColor(clamp( _aq_a, _aq__min, _aq__max ),__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_floor( IqShaderData* x, IqShaderData* Result, IqShader* pShader )
{
	MathOneParam(std::floor, x , Result);
}

void	CqShaderExecEnv::SO_ceil( IqShaderData* x, IqShaderData* Result, IqShader* pShader )
{
	MathOneParam(std::ceil, x , Result);
}

void	CqShaderExecEnv::SO_round( IqShaderData* x, IqShaderData* Result, IqShader* pShader )
{
	MathOneParam(round, x , Result);
}

void	CqShaderExecEnv::SO_inversesqrt( IqShaderData* x, IqShaderData* Result, IqShader* pShader )
{
	MathOneParam(isqrt, x , Result);
}

void	CqShaderExecEnv::SO_length( IqShaderData* V, IqShaderData* Result, IqShader* pShader )
{
	MathOneParamVector(length, V , Result);
}

void	CqShaderExecEnv::SO_distance( IqShaderData* P1, IqShaderData* P2, IqShaderData* Result, IqShader* pShader )
{
	MathTwoParamsVector(distance, P1, P2, Result);
}


} // namespace Aqsis
//---------------------------------------------------------------------
