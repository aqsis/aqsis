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
		\brief Implements the basic shader operations. (Color, vector componets related)
		\author Paul C. Gregory (pgregory@aqsis.org)
*/


#include	<stdio.h>

#include	"shaderexecenv.h"

namespace Aqsis {


//----------------------------------------------------------------------
// setcomp(c,__iGrid,v)
void	CqShaderExecEnv::SO_setcomp( IqShaderData* p, IqShaderData* index, IqShaderData* v, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(p)->Class()==class_varying;
	__fVarying=(v)->Class()==class_varying||__fVarying;
	__fVarying=(index)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// setxcomp(p,v)
void	CqShaderExecEnv::SO_setxcomp( IqShaderData* p, IqShaderData* v, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(p)->Class()==class_varying;
	__fVarying=(v)->Class()==class_varying||__fVarying;

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
			_aq_p.x( _aq_v );
			(p)->SetPoint(_aq_p,__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// setycomp(p,v)
void	CqShaderExecEnv::SO_setycomp( IqShaderData* p, IqShaderData* v, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(p)->Class()==class_varying;
	__fVarying=(v)->Class()==class_varying||__fVarying;

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
			_aq_p.y( _aq_v );
			(p)->SetPoint(_aq_p,__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// setzcomp(p,v)
void	CqShaderExecEnv::SO_setzcomp( IqShaderData* p, IqShaderData* v, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(p)->Class()==class_varying;
	__fVarying=(v)->Class()==class_varying||__fVarying;

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
			_aq_p.z( _aq_v );
			(p)->SetPoint(_aq_p,__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}



} // namespace Aqsis
//---------------------------------------------------------------------
