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


#include <stdio.h>

#include <aqsis/math/math.h>
#include "shaderexecenv.h"
#include <aqsis/util/logging.h>

namespace Aqsis {

namespace {

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

inline void domainError(const char* funcName, IqShaderData* var, TqFloat varValue)
{
	std::ostream& out = Aqsis::log();
	out << warning << "domain error: " << funcName << "(";
	if(var->strName() != "")
		out << var->strName() << "=";
	out << varValue << ") is undefined, result has been set to zero\n";
}

inline void domainError(const char* funcName, IqShaderData* var1, IqShaderData* var2,
		TqFloat var1Value, TqFloat var2Value)
{
	std::ostream& out = Aqsis::log();
	out << warning << "domain error: " << funcName << "(";
	if(var1->strName() != "")
		out << var1->strName() << "=";
	out << var1Value << ", ";
	if(var2->strName() != "")
		out << var2->strName() << "=";
	out << var2Value;
	out << ") is undefined, result has been set to zero\n";
}

} // unnamed namespace

void	CqShaderExecEnv::SO_radians( IqShaderData* degrees, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(degrees)->Class()==class_varying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_degrees;
			(degrees)->GetFloat(_aq_degrees,__iGrid);
			(Result)->SetFloat(degToRad( _aq_degrees ),__iGrid);
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
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_radians;
			(radians)->GetFloat(_aq_radians,__iGrid);
			(Result)->SetFloat(radToDeg( _aq_radians ),__iGrid);
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
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_a;
			(a)->GetFloat(_aq_a,__iGrid);
			(Result)->SetFloat(std::sin(_aq_a), __iGrid);
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
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat aVal;
			(a)->GetFloat(aVal,__iGrid);
			TqFloat res = 0;
			if(aVal < -1 || aVal > 1)
				domainError("asin", a, aVal);
			else
				res = std::asin(aVal);
			(Result)->SetFloat(res, __iGrid);
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
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_a;
			(a)->GetFloat(_aq_a,__iGrid);
			(Result)->SetFloat(std::cos(_aq_a), __iGrid);
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
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat aVal;
			(a)->GetFloat(aVal,__iGrid);
			TqFloat res = 0;
			if(aVal < -1 || aVal > 1)
				domainError("acos", a, aVal);
			else
				res = std::acos(aVal);
			(Result)->SetFloat(res, __iGrid);
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
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat aVal;
			(a)->GetFloat(aVal,__iGrid);
			(Result)->SetFloat(std::tan(aVal), __iGrid);
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
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_yoverx;
			(yoverx)->GetFloat(_aq_yoverx,__iGrid);
			(Result)->SetFloat(std::atan(_aq_yoverx), __iGrid);
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
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_x;
			(x)->GetFloat(_aq_x,__iGrid);
			TqFloat _aq_y;
			(y)->GetFloat(_aq_y,__iGrid);
			(Result)->SetFloat(std::atan2(_aq_y, _aq_x), __iGrid);
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
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat xVal;
			(x)->GetFloat(xVal,__iGrid);
			TqFloat yVal;
			(y)->GetFloat(yVal,__iGrid);
			TqFloat res = 0;
			if(xVal < 0)
			{
				TqInt yInt = lfloor(yVal);
				if(yInt != yVal)
				{
					res = 0;
					domainError("pow", x, y, xVal, yVal);
				}
				else
				{
					res = std::pow(xVal, yInt);
				}
			}
			else
			{
				res = std::pow(xVal, yVal);
			}
			(Result)->SetFloat(res, __iGrid);
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
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_x;
			(x)->GetFloat(_aq_x,__iGrid);
			(Result)->SetFloat(std::exp(_aq_x), __iGrid);
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
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat xVal;
			(x)->GetFloat(xVal,__iGrid);
			TqFloat res = 0;
			if(xVal < 0)
			{
				domainError("sqrt", x, xVal);
			}
			else
			{
#ifndef FASTSQRT
				res = std::sqrt(xVal);
#else
				res = sqrtf(xVal);
#endif
			}
			(Result)->SetFloat(res, __iGrid);
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
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat xVal;
			(x)->GetFloat(xVal,__iGrid);
			TqFloat res = 0;
			if(xVal <= 0)
				domainError("log", x, xVal);
			else
				res = std::log(xVal);
			(Result)->SetFloat(res, __iGrid);
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
	const CqBitVector& RS = RunningState();
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
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat xVal;
			(x)->GetFloat(xVal,__iGrid);
			TqFloat baseVal;
			(base)->GetFloat(baseVal,__iGrid);
			TqFloat res = 0;
			if(xVal <= 0 || baseVal <= 0)
				domainError("log", x, base, xVal, baseVal);
			else
				res = std::log(xVal)/std::log(baseVal);
			(Result)->SetFloat(res, __iGrid);
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
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_x;
			(x)->GetFloat(_aq_x,__iGrid);
#ifndef FASTSQRT
			(Result)->SetFloat(std::fabs(_aq_x), __iGrid);
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
	const CqBitVector& RS = RunningState();
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
			for(TqInt i = 0; i < cParams; ++i)
			{
				TqFloat fn;
				apParams[ i ] ->GetFloat( fn, __iGrid );
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
			for(TqInt i = 0; i < cParams; ++i)
			{
				TqFloat fn;
				apParams[ i ] ->GetFloat( fn, __iGrid );
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
			for(TqInt i = 0; i < cParams; ++i)
			{
				CqVector3D pn;
				apParams[ i ] ->GetPoint( pn, __iGrid );
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
			for(TqInt i = 0; i < cParams; ++i)
			{
				CqVector3D pn;
				apParams[ i ] ->GetPoint( pn, __iGrid );
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
			for(TqInt i = 0; i < cParams; ++i)
			{
				CqColor cn;
				apParams[ i ] ->GetColor( cn, __iGrid );
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
			for(TqInt i = 0; i < cParams; ++i)
			{
				CqColor cn;
				apParams[ i ] ->GetColor( cn, __iGrid );
				res = Aqsis::max( res, cn );
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
	const CqBitVector& RS = RunningState();
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
			(Result)->SetFloat(clamp( _aq_a, _aq__min, _aq__max ),__iGrid);
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
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(x)->Class()==class_varying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_x;
			(x)->GetFloat(_aq_x,__iGrid);
			(Result)->SetFloat(std::floor(_aq_x), __iGrid);
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
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_x;
			(x)->GetFloat(_aq_x,__iGrid);
			(Result)->SetFloat(std::ceil(_aq_x), __iGrid);
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
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_x;
			(x)->GetFloat(_aq_x,__iGrid);
			res = round(_aq_x);
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
	const CqBitVector& RS = RunningState();
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
	const CqBitVector& RS = RunningState();
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
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat xVal;
			(x)->GetFloat(xVal,__iGrid);
			TqFloat res = 0;
			if(xVal <= 0)
			{
				domainError("inversesqrt", x, xVal);
			}
			else
			{
#ifndef FASTSQRT
				res = 1/std::sqrt(xVal);
#else
				res = isqrtf(xVal);
#endif
			}
			(Result)->SetFloat(res, __iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}



} // namespace Aqsis
//---------------------------------------------------------------------
