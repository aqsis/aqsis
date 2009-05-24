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
		\brief Implements the basic shader operations.(Noise, random related)
		\author Paul C. Gregory (pgregory@aqsis.org)
*/


#include	<stdio.h>

#include	"shaderexecenv.h"
#include	<aqsis/math/vectorcast.h>

namespace Aqsis {


void	CqShaderExecEnv::SO_frandom( IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(Result)->Class()==class_varying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			(Result)->SetFloat(m_random.RandomFloat(),__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_crandom( IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(Result)->Class()==class_varying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_prandom( IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(Result)->Class()==class_varying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat a, b, c;
			a = m_random.RandomFloat();
			b = m_random.RandomFloat();
			c = m_random.RandomFloat();

			(Result)->SetPoint(CqVector3D(a,b,c),__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


//----------------------------------------------------------------------
// noise(v)
void	CqShaderExecEnv::SO_fnoise1( IqShaderData* v, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(v)->Class()==class_varying;
	__fVarying=(Result)->Class()==class_varying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			(Result)->SetFloat( m_noise.FGNoise1( _aq_v ),__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// noise(u,v)
void CqShaderExecEnv::SO_fnoise2( IqShaderData* u, IqShaderData* v, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(u)->Class()==class_varying;
	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// noise(p)
void CqShaderExecEnv::SO_fnoise3( IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(p)->Class()==class_varying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_p;
			(p)->GetPoint(_aq_p,__iGrid);
			(Result)->SetFloat( m_noise.FGNoise3( _aq_p ),__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// noise(p,t)
void CqShaderExecEnv::SO_fnoise4( IqShaderData* p, IqShaderData* t, IqShaderData* Result, IqShader* pShader )
{
	// TODO: Do proper 4D noise.
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(p)->Class()==class_varying;
	__fVarying=(t)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// noise(v)
void	CqShaderExecEnv::SO_cnoise1( IqShaderData* v, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(v)->Class()==class_varying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			(Result)->SetColor( m_noise.CGNoise1( _aq_v ),__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// noise(u,v)
void CqShaderExecEnv::SO_cnoise2( IqShaderData* u, IqShaderData* v, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(u)->Class()==class_varying;
	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// noise(p)
void CqShaderExecEnv::SO_cnoise3( IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(p)->Class()==class_varying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_p;
			(p)->GetPoint(_aq_p,__iGrid);
			(Result)->SetColor( m_noise.CGNoise3( _aq_p ),__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// noise(p,t)
void CqShaderExecEnv::SO_cnoise4( IqShaderData* p, IqShaderData* t, IqShaderData* Result, IqShader* pShader )
{
	// TODO: Do proper 4D noise.
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(p)->Class()==class_varying;
	__fVarying=(t)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// noise(v)
void CqShaderExecEnv::SO_pnoise1( IqShaderData* v, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(v)->Class()==class_varying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			(Result)->SetPoint( m_noise.PGNoise1( _aq_v ),__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// noise(u,v)
void CqShaderExecEnv::SO_pnoise2( IqShaderData* u, IqShaderData* v, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(u)->Class()==class_varying;
	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// noise(p)
void CqShaderExecEnv::SO_pnoise3( IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(p)->Class()==class_varying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_p;
			(p)->GetPoint(_aq_p,__iGrid);
			(Result)->SetPoint( m_noise.PGNoise3( _aq_p ),__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// noise(p,t)
void CqShaderExecEnv::SO_pnoise4( IqShaderData* p, IqShaderData* t, IqShaderData* Result, IqShader* pShader )
{
	// TODO: Do proper 4D noise.
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(p)->Class()==class_varying;
	__fVarying=(t)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


//----------------------------------------------------------------------
// noise(v)
void CqShaderExecEnv::SO_fcellnoise1( IqShaderData* v, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(v)->Class()==class_varying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			(Result)->SetFloat(m_cellnoise.FCellNoise1( _aq_v ),__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void CqShaderExecEnv::SO_ccellnoise1( IqShaderData* v, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(v)->Class()==class_varying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			(Result)->SetColor(vectorCast<CqColor>( m_cellnoise.PCellNoise1(_aq_v) ), __iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void CqShaderExecEnv::SO_pcellnoise1( IqShaderData* v, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(v)->Class()==class_varying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			(Result)->SetPoint(m_cellnoise.PCellNoise1( _aq_v ),__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// noise(u,v)
void CqShaderExecEnv::SO_fcellnoise2( IqShaderData* u, IqShaderData* v, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(u)->Class()==class_varying;
	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}
void CqShaderExecEnv::SO_ccellnoise2( IqShaderData* u, IqShaderData* v, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(u)->Class()==class_varying;
	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_u;
			(u)->GetFloat(_aq_u,__iGrid);
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			(Result)->SetColor(vectorCast<CqColor>( m_cellnoise.PCellNoise2(_aq_u, _aq_v) ), __iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}
void CqShaderExecEnv::SO_pcellnoise2( IqShaderData* u, IqShaderData* v, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(u)->Class()==class_varying;
	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// noise(p)
void CqShaderExecEnv::SO_fcellnoise3( IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(p)->Class()==class_varying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_p;
			(p)->GetPoint(_aq_p,__iGrid);
			(Result)->SetFloat(m_cellnoise.FCellNoise3( _aq_p ),__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}
void CqShaderExecEnv::SO_ccellnoise3( IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(p)->Class()==class_varying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_p;
			(p)->GetPoint(_aq_p,__iGrid);
			(Result)->SetColor(vectorCast<CqColor>( m_cellnoise.PCellNoise3(_aq_p) ),__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}
void CqShaderExecEnv::SO_pcellnoise3( IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(p)->Class()==class_varying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_p;
			(p)->GetPoint(_aq_p,__iGrid);
			(Result)->SetPoint(m_cellnoise.PCellNoise3( _aq_p ),__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// noise(p,f)
void CqShaderExecEnv::SO_fcellnoise4( IqShaderData* p, IqShaderData* v, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(p)->Class()==class_varying;
	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}
void CqShaderExecEnv::SO_ccellnoise4( IqShaderData* p, IqShaderData* v, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(p)->Class()==class_varying;
	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_p;
			(p)->GetPoint(_aq_p,__iGrid);
			TqFloat _aq_v;
			(v)->GetFloat(_aq_v,__iGrid);
			(Result)->SetColor(vectorCast<CqColor>( m_cellnoise.PCellNoise4(_aq_p, _aq_v) ),__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}
void CqShaderExecEnv::SO_pcellnoise4( IqShaderData* p, IqShaderData* v, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(p)->Class()==class_varying;
	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


//----------------------------------------------------------------------
// pnoise(u,period)
void CqShaderExecEnv::SO_fpnoise1( IqShaderData* v, IqShaderData* period, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(v)->Class()==class_varying;
	__fVarying=(period)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// pnoise(u,v,uperiod,vperiod)
void CqShaderExecEnv::SO_fpnoise2( IqShaderData* u, IqShaderData* v, IqShaderData* uperiod, IqShaderData* vperiod, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(u)->Class()==class_varying;
	__fVarying=(uperiod)->Class()==class_varying||__fVarying;
	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(vperiod)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// pnoise(p,pperiod)
void CqShaderExecEnv::SO_fpnoise3( IqShaderData* p, IqShaderData* pperiod, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(p)->Class()==class_varying;
	__fVarying=(pperiod)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// pnoise(p,t,pperiod,tperiod)
void CqShaderExecEnv::SO_fpnoise4( IqShaderData* p, IqShaderData* t, IqShaderData* pperiod, IqShaderData* tperiod, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(p)->Class()==class_varying;
	__fVarying=(pperiod)->Class()==class_varying||__fVarying;
	__fVarying=(t)->Class()==class_varying||__fVarying;
	__fVarying=(tperiod)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// pnoise(u,period)
void CqShaderExecEnv::SO_cpnoise1( IqShaderData* v, IqShaderData* period, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(v)->Class()==class_varying;
	__fVarying=(period)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// pnoise(u,v,uperiod,vperiod)
void CqShaderExecEnv::SO_cpnoise2( IqShaderData* u, IqShaderData* v, IqShaderData* uperiod, IqShaderData* vperiod, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(u)->Class()==class_varying;
	__fVarying=(uperiod)->Class()==class_varying||__fVarying;
	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(vperiod)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// pnoise(p,pperiod)
void CqShaderExecEnv::SO_cpnoise3( IqShaderData* p, IqShaderData* pperiod, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(p)->Class()==class_varying;
	__fVarying=(pperiod)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// pnoise(p,t,pperiod,tperiod)
void CqShaderExecEnv::SO_cpnoise4( IqShaderData* p, IqShaderData* t, IqShaderData* pperiod, IqShaderData* tperiod, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(p)->Class()==class_varying;
	__fVarying=(pperiod)->Class()==class_varying||__fVarying;
	__fVarying=(t)->Class()==class_varying||__fVarying;
	__fVarying=(tperiod)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// pnoise(u,period)
void CqShaderExecEnv::SO_ppnoise1( IqShaderData* v, IqShaderData* period, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(v)->Class()==class_varying;
	__fVarying=(period)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// pnoise(u,v,uperiod,vperiod)
void CqShaderExecEnv::SO_ppnoise2( IqShaderData* u, IqShaderData* v, IqShaderData* uperiod, IqShaderData* vperiod, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(u)->Class()==class_varying;
	__fVarying=(uperiod)->Class()==class_varying||__fVarying;
	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(vperiod)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// pnoise(p,pperiod)
void CqShaderExecEnv::SO_ppnoise3( IqShaderData* p, IqShaderData* pperiod, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(p)->Class()==class_varying;
	__fVarying=(pperiod)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// pnoise(p,t,pperiod,tperiod)
void CqShaderExecEnv::SO_ppnoise4( IqShaderData* p, IqShaderData* t, IqShaderData* pperiod, IqShaderData* tperiod, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(p)->Class()==class_varying;
	__fVarying=(pperiod)->Class()==class_varying||__fVarying;
	__fVarying=(t)->Class()==class_varying||__fVarying;
	__fVarying=(tperiod)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}





} // namespace Aqsis
//---------------------------------------------------------------------
