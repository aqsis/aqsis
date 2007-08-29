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
		\brief Implements the basic shader operations.(Antialias related)
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	"aqsis.h"

#include	<math.h>
#include	<map>
#include	<vector>
#include	<string>
#include	<stdio.h>

#include	"shaderexecenv.h"
#include	"spline.h"
#include	"shadervm.h"
#include	"irenderer.h"
#include	"version.h"
#include	"logging.h"

START_NAMESPACE( Aqsis )


void	CqShaderExecEnv::SO_step( IqShaderData* _min, IqShaderData* value, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(_min)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


//----------------------------------------------------------------------
// smoothstep(_min,_max,value)
void	CqShaderExecEnv::SO_smoothstep( IqShaderData* _min, IqShaderData* _max, IqShaderData* value, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(value)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
void	CqShaderExecEnv::SO_fspline( IqShaderData* value, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	CqCubicSpline<TqFloat> spline( SplineBasis_CatmullRom, cParams );

	__fVarying=(value)->Class()==class_varying;
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
					spline.pushBack( fn );
				}

				(Result)->SetFloat( spline.evaluate( _aq_value ), __iGrid );
				spline.clear();
			}
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
void	CqShaderExecEnv::SO_cspline( IqShaderData* value, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	CqCubicSpline<CqColor> spline( SplineBasis_CatmullRom, cParams );

	__fVarying=(value)->Class()==class_varying;
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
			(value)->GetFloat( _aq_value, __iGrid );
			if ( _aq_value >= 1.0f )
			{
				CqColor cl;
				apParams[ cParams - 2 ] ->GetColor( cl, __iGrid );
				(Result)->SetColor( cl, __iGrid );
			}
			else if ( _aq_value <= 0.0f )
			{
				CqColor cf;
				apParams[ 1 ] ->GetColor( cf, __iGrid );
				(Result)->SetColor( cf, __iGrid );
			}
			else
			{
				TqInt j;
				for ( j = 0; j < cParams; j++ )
				{
					CqColor cn;
					apParams[ j ] ->GetColor( cn, __iGrid );
					spline.pushBack( cn );
				}

				(Result)->SetColor( spline.evaluate( _aq_value ), __iGrid);
				spline.clear();
			}
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
void	CqShaderExecEnv::SO_pspline( IqShaderData* value, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	CqCubicSpline<CqVector3D> spline( SplineBasis_CatmullRom, cParams );

	__fVarying=(value)->Class()==class_varying;
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
					spline.pushBack( pn );
				}
				
				(Result)->SetPoint( spline.evaluate( _aq_value ), __iGrid );
				spline.clear();
			}
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
void	CqShaderExecEnv::SO_sfspline( IqShaderData* basis, IqShaderData* value, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(value)->Class()==class_varying;
	TqInt v;
	for ( v = 0; v < cParams; v++ )
	{
		__fVarying=(( apParams[ v ] ))->Class()==class_varying||__fVarying;
	}
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqString _aq_basis;
	(basis)->GetString( _aq_basis, __iGrid );
	CqCubicSpline<TqFloat> spline( _aq_basis, cParams );

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
					spline.pushBack( fn );
				}

				(Result)->SetFloat( spline.evaluate( _aq_value ), __iGrid );
				spline.clear();
			}
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
void	CqShaderExecEnv::SO_scspline( IqShaderData* basis, IqShaderData* value, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(value)->Class()==class_varying;
	TqInt v;
	for ( v = 0; v < cParams; v++ )
	{
		__fVarying=(( apParams[ v ] ))->Class()==class_varying||__fVarying;
	}
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqString _aq_basis;
	(basis)->GetString(_aq_basis,__iGrid);
	CqCubicSpline<CqColor> spline( _aq_basis, cParams );

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
					spline.pushBack( cn );
				}

				(Result)->SetColor( spline.evaluate( _aq_value ), __iGrid );
				spline.clear();
			}
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
void	CqShaderExecEnv::SO_spspline( IqShaderData* basis, IqShaderData* value, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(value)->Class()==class_varying;
	TqInt v;
	for ( v = 0; v < cParams; v++ )
	{
		__fVarying=(( apParams[ v ] ))->Class()==class_varying||__fVarying;
	}
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqString _aq_basis;
	(basis)->GetString(_aq_basis,__iGrid);
	CqCubicSpline<CqVector3D> spline( _aq_basis, cParams );

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
					spline.pushBack( pn );
				}

				(Result)->SetPoint( spline.evaluate( _aq_value ), __iGrid );
				spline.clear();
			}
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


void	CqShaderExecEnv::SO_fDu( IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	TqFloat Deffloat = 0.0f;
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(p)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


void	CqShaderExecEnv::SO_fDv( IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	TqFloat Deffloat = 0.0f;
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(p)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


void	CqShaderExecEnv::SO_fDeriv( IqShaderData* p, IqShaderData* den, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(p)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


void	CqShaderExecEnv::SO_cDu( IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	CqColor Defcol( 0.0f, 0.0f, 0.0f );
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(p)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


void	CqShaderExecEnv::SO_cDv( IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	CqColor Defcol( 0.0f, 0.0f, 0.0f );
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(p)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


void	CqShaderExecEnv::SO_cDeriv( IqShaderData* p, IqShaderData* den, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(p)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


void	CqShaderExecEnv::SO_pDu( IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	CqVector3D Defvec( 0.0f, 0.0f, 0.0f );
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(p)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


void	CqShaderExecEnv::SO_pDv( IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	CqVector3D Defvec( 0.0f, 0.0f, 0.0f );
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(p)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


void	CqShaderExecEnv::SO_pDeriv( IqShaderData* p, IqShaderData* den, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(p)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// area(P)
void CqShaderExecEnv::SO_area( IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	CqVector3D Defvec( 0.0f, 0.0f, 0.0f );
	bool __fVarying;
	TqUint __iGrid;

	CqVector3D	vecR;

	__fVarying=(p)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


void	CqShaderExecEnv::SO_normalize( IqShaderData* V, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(V)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


//----------------------------------------------------------------------
// faceforward(N,I)
void CqShaderExecEnv::SO_faceforward( IqShaderData* N, IqShaderData* I, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(N)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


//----------------------------------------------------------------------
// faceforward(N,I,Nref)
void CqShaderExecEnv::SO_faceforward2( IqShaderData* N, IqShaderData* I, IqShaderData* Nref, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(N)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
void    CqShaderExecEnv::SO_fsplinea( IqShaderData* value, IqShaderData* a, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	bool __fVaryingA=false;
	TqUint __iGrid;

	assert( a->ArrayLength() > 0 );
	assert( a->Type() == type_float );
	
	TqInt    cParams = a->ArrayLength();
	CqCubicSpline<TqFloat> spline( SplineBasis_CatmullRom, cParams );

	__fVarying=(value)->Class()==class_varying;
	__fVaryingA=(a)->Class()==class_varying;
	__fVarying = __fVaryingA || __fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;

	if (!__fVaryingA)
	{
		TqInt j;
		TqFloat fTemp;
		for ( j = 0; j < cParams; j++ )
		{
			a->ArrayEntry( j ) ->GetFloat( fTemp, __iGrid );
			spline.pushBack( fTemp );
		}
	}

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
				spline.clear();
				TqInt j;
				for ( j = 0; __fVaryingA && j < cParams; j++ )
				{
					a->ArrayEntry( j ) ->GetFloat( fTemp, __iGrid );
					spline.pushBack( fTemp);
				}

				(Result)->SetFloat( spline.evaluate( _aq_value ), __iGrid );
				spline.clear();
			}
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
void    CqShaderExecEnv::SO_csplinea( IqShaderData* value, IqShaderData* a, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	bool __fVaryingA=false;
	TqUint __iGrid;

	assert( a->ArrayLength() > 0 );
	assert( a->Type() == type_color );
	
	TqInt	cParams = a->ArrayLength();
	CqCubicSpline<CqColor> spline( SplineBasis_CatmullRom, cParams );
	CqColor colTemp;

	__fVarying=(value)->Class()==class_varying;
	__fVaryingA=(a)->Class()==class_varying;
	__fVarying = __fVaryingA || __fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;

	if (!__fVaryingA)
	{
		TqInt j;
		for ( j = 0; j < cParams; j++ )
		{
			a->ArrayEntry( j ) ->GetColor( colTemp, __iGrid );
			spline.pushBack( colTemp );
		}
	}

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
				spline.clear();
				TqInt j;
				for ( j = 0; __fVaryingA && j < cParams; j++ )
				{
					a->ArrayEntry( j ) ->GetColor( colTemp, __iGrid );
					spline.pushBack( colTemp );
				}

				(Result)->SetColor( spline.evaluate( _aq_value ), __iGrid );
				spline.clear();
			}
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
void    CqShaderExecEnv::SO_psplinea( IqShaderData* value, IqShaderData* a, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	bool __fVaryingA=false;
	TqUint __iGrid;

	assert( a->ArrayLength() > 0 );
	assert( a->Type() == type_point );
	
	TqInt    cParams = a->ArrayLength();
	CqCubicSpline<CqVector3D> spline( SplineBasis_CatmullRom, cParams );
	CqVector3D vecTemp;

	__fVarying=(value)->Class()==class_varying;
	__fVaryingA=(a)->Class()==class_varying;
	__fVarying = __fVaryingA || __fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;
 
	__iGrid = 0;

	if (!__fVaryingA)
	{
		TqInt j;
		for ( j = 0; j < cParams; j++ )
		{
			a->ArrayEntry( j ) ->GetPoint( vecTemp, __iGrid );
			spline.pushBack( vecTemp );
		}
	}

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
				spline.clear();
				TqInt j;
				for ( j = 0; __fVaryingA && j < cParams; j++ )
				{
					a->ArrayEntry( j ) ->GetPoint( vecTemp, __iGrid );
					spline.pushBack( vecTemp );
				}

				(Result)->SetPoint( spline.evaluate( _aq_value ), __iGrid );
				spline.clear();
			}
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
void    CqShaderExecEnv::SO_sfsplinea( IqShaderData* basis, IqShaderData* value, IqShaderData* a, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	bool __fVaryingA=false;
	TqUint __iGrid;

	assert( a->ArrayLength() > 0 );
	assert( a->Type() == type_float );

	TqInt    cParams = a->ArrayLength();

	__fVarying=(value)->Class()==class_varying;
	__fVaryingA=(a)->Class()==class_varying;
	__fVarying = __fVaryingA || __fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	
	CqString _aq_basis;
	(basis)->GetString( _aq_basis, __iGrid );
	CqCubicSpline<TqFloat> spline( _aq_basis, cParams );

	__iGrid = 0;

	if (!__fVaryingA)
	{
		TqInt j;
		TqFloat fTemp;
		for ( j = 0; j < cParams; j++ )
		{
			a->ArrayEntry( j ) ->GetFloat( fTemp, __iGrid );
			spline.pushBack( fTemp );
		}
	}

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
				spline.clear();
				TqInt j;
				for ( j = 0; __fVaryingA && j < cParams; j++ )
				{
					a->ArrayEntry( j ) ->GetFloat( fTemp, __iGrid );
					spline.pushBack( fTemp );
				}

				(Result)->SetFloat( spline.evaluate( _aq_value ), __iGrid );
				spline.clear();
			}
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
void    CqShaderExecEnv::SO_scsplinea( IqShaderData* basis, IqShaderData* value, IqShaderData* a, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	bool __fVaryingA=false;
	TqUint __iGrid;

	assert( a->ArrayLength() > 0 );
	assert( a->Type() == type_color );

	TqInt    cParams = a->ArrayLength();
	CqColor colTemp;

	__fVarying=(value)->Class()==class_varying;
	__fVaryingA=(a)->Class()==class_varying;
	__fVarying = __fVaryingA || __fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqString _aq_basis;
	(basis)->GetString(_aq_basis,__iGrid);
	CqCubicSpline<CqColor> spline( _aq_basis, cParams );

	__iGrid = 0;

	if (!__fVaryingA)
	{
		TqInt j;
		for ( j = 0; j < cParams; j++ )
		{
			a->ArrayEntry( j ) ->GetColor( colTemp, __iGrid );
			spline.pushBack( colTemp );
		}
	}

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
				spline.clear();
				TqInt j;
				for ( j = 0; __fVaryingA && j < cParams; j++ )
				{
					a->ArrayEntry( j ) ->GetColor( colTemp, __iGrid );
					spline.pushBack( colTemp );
				}

				(Result)->SetColor( spline.evaluate( _aq_value ), __iGrid );
				spline.clear();
			}
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
void    CqShaderExecEnv::SO_spsplinea( IqShaderData* basis, IqShaderData* value, IqShaderData* a, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	bool __fVaryingA=false;
	TqUint __iGrid;

	assert( a->ArrayLength() > 0 );
	assert( a->Type() == type_point );

	TqInt    cParams = a->ArrayLength();
	CqVector3D vecTemp;

	__fVarying=(value)->Class()==class_varying;
	__fVaryingA=(a)->Class()==class_varying;
	__fVarying = __fVaryingA || __fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqString _aq_basis;
	(basis)->GetString(_aq_basis,__iGrid);
	CqCubicSpline<CqVector3D> spline( _aq_basis, cParams );

	__iGrid = 0;
	if (!__fVaryingA)
	{
		TqInt j;
		for ( j = 0; j < cParams; j++ )
		{
			a->ArrayEntry( j ) ->GetPoint( vecTemp, __iGrid );
			spline.pushBack( vecTemp );
		}
	}

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
				spline.clear();
				TqInt j;
				for ( j = 0; __fVaryingA && j < cParams; j++ )
				{
					a->ArrayEntry( j ) ->GetPoint( vecTemp, __iGrid );
					spline.pushBack( vecTemp );
				}

				(Result)->SetPoint( spline.evaluate( _aq_value ), __iGrid );
				spline.clear();
			}
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// filterstep(edge,s1)
void CqShaderExecEnv::SO_filterstep( IqShaderData* edge, IqShaderData* s1, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqFloat Deffloat = 0.0f;
	bool __fVarying;
	TqUint __iGrid;

	float _pswidth=1.0f,_ptwidth=1.0f;
	GetFilterParams(cParams, apParams, _pswidth,_ptwidth);

	__fVarying=(edge)->Class()==class_varying;
	__fVarying=(s1)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	TqFloat fdu, fdv;
	du() ->GetFloat( fdu );
	dv() ->GetFloat( fdv );

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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// filterstep(edge,s1,s2)
void CqShaderExecEnv::SO_filterstep2( IqShaderData* edge, IqShaderData* s1, IqShaderData* s2, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	float _pswidth=1.0f,_ptwidth=1.0f;
	GetFilterParams(cParams, apParams, _pswidth,_ptwidth);

	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(edge)->Class()==class_varying;
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}
END_NAMESPACE(    Aqsis )
//---------------------------------------------------------------------
