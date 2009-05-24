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
		\brief Implements the basic shader operations.(Matrix, transform related)
		\author Paul C. Gregory (pgregory@aqsis.org)
*/


#include	<stdio.h>

#include	"shaderexecenv.h"

namespace Aqsis {


//----------------------------------------------------------------------
// transform(s,s,P)
void CqShaderExecEnv::SO_transform( IqShaderData* fromspace, IqShaderData* tospace, IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	assert( pShader != 0 );

	__fVarying=(p)->Class()==class_varying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	if ( getRenderContext() )
	{
		__iGrid = 0;
		CqString _aq_fromspace;
		(fromspace)->GetString(_aq_fromspace,__iGrid);
		CqString _aq_tospace;
		(tospace)->GetString(_aq_tospace,__iGrid);
		CqMatrix mat; 
		getRenderContext() ->matSpaceToSpace( _aq_fromspace.c_str(), _aq_tospace.c_str(), pShader->getTransform(), pTransform().get(), getRenderContext()->Time(), mat );


		__iGrid = 0;
		const CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqVector3D _aq_p;
				(p)->GetPoint(_aq_p,__iGrid);
				(Result)->SetPoint(mat * _aq_p,__iGrid);
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		const CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqVector3D _aq_p;
				(p)->GetPoint(_aq_p,__iGrid);
				(Result)->SetPoint(_aq_p,__iGrid);
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
}


//----------------------------------------------------------------------
// transform(s,P)
void CqShaderExecEnv::SO_transform( IqShaderData* tospace, IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	assert( pShader != 0 );

	__fVarying=(p)->Class()==class_varying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	if ( getRenderContext() )
	{
		__iGrid = 0;
		CqString _aq_tospace;
		(tospace)->GetString(_aq_tospace,__iGrid);
		CqMatrix mat;
		getRenderContext() ->matSpaceToSpace( "current", _aq_tospace.c_str(), pShader->getTransform(), pTransform().get(), getRenderContext()->Time(), mat );


		__iGrid = 0;
		const CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqVector3D _aq_p;
				(p)->GetPoint(_aq_p,__iGrid);
				(Result)->SetPoint(mat * _aq_p,__iGrid);
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		const CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqVector3D _aq_p;
				(p)->GetPoint(_aq_p,__iGrid);
				(Result)->SetPoint(_aq_p,__iGrid);
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
}


//----------------------------------------------------------------------
// transform(m,P)
void CqShaderExecEnv::SO_transformm( IqShaderData* tospace, IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	assert( pShader != 0 );

	__fVarying=(p)->Class()==class_varying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


//----------------------------------------------------------------------
// vtransform(s,s,P)
void CqShaderExecEnv::SO_vtransform( IqShaderData* fromspace, IqShaderData* tospace, IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	assert( pShader != 0 );

	__fVarying=(p)->Class()==class_varying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	if ( getRenderContext() )
	{
		__iGrid = 0;
		CqString _aq_fromspace;
		(fromspace)->GetString(_aq_fromspace,__iGrid);
		CqString _aq_tospace;
		(tospace)->GetString(_aq_tospace,__iGrid);
		CqMatrix mat;
		getRenderContext() ->matVSpaceToSpace( _aq_fromspace.c_str(), _aq_tospace.c_str(), pShader->getTransform(), pTransform().get(), getRenderContext()->Time(), mat );


		__iGrid = 0;
		const CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqVector3D _aq_p;
				(p)->GetVector(_aq_p,__iGrid);
				(Result)->SetVector(mat * _aq_p,__iGrid);
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		const CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqVector3D _aq_p;
				(p)->GetVector(_aq_p,__iGrid);
				(Result)->SetVector(_aq_p,__iGrid);
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
}


//----------------------------------------------------------------------
// vtransform(s,P)
void CqShaderExecEnv::SO_vtransform( IqShaderData* tospace, IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	assert( pShader != 0 );

	__fVarying=(p)->Class()==class_varying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	if ( getRenderContext() )
	{
		__iGrid = 0;
		CqString _aq_tospace;
		(tospace)->GetString(_aq_tospace,__iGrid);
		CqMatrix mat;
		getRenderContext() ->matVSpaceToSpace( "current", _aq_tospace.c_str(), pShader->getTransform(), pTransform().get(), getRenderContext()->Time(), mat );


		__iGrid = 0;
		const CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqVector3D _aq_p;
				(p)->GetVector(_aq_p,__iGrid);
				(Result)->SetVector(mat * _aq_p,__iGrid);
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		const CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqVector3D _aq_p;
				(p)->GetVector(_aq_p,__iGrid);
				(Result)->SetVector(_aq_p,__iGrid);
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
}


//----------------------------------------------------------------------
// vtransform(m,P)
void CqShaderExecEnv::SO_vtransformm( IqShaderData* tospace, IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	assert( pShader != 0 );

	__fVarying=(p)->Class()==class_varying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


//----------------------------------------------------------------------
// ntransform(s,s,P)
void CqShaderExecEnv::SO_ntransform( IqShaderData* fromspace, IqShaderData* tospace, IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	assert( pShader != 0 );

	__fVarying=(p)->Class()==class_varying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	if ( getRenderContext() )
	{
		__iGrid = 0;
		CqString _aq_fromspace;
		(fromspace)->GetString(_aq_fromspace,__iGrid);
		CqString _aq_tospace;
		(tospace)->GetString(_aq_tospace,__iGrid);
		CqMatrix mat;
		getRenderContext() ->matNSpaceToSpace( _aq_fromspace.c_str(), _aq_tospace.c_str(), pShader->getTransform(), pTransform().get(), getRenderContext()->Time(), mat );
		__iGrid = 0;

		__iGrid = 0;
		const CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqVector3D _aq_p;
				(p)->GetNormal(_aq_p,__iGrid);
				(Result)->SetNormal(mat * _aq_p,__iGrid);
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		const CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqVector3D _aq_p;
				(p)->GetNormal(_aq_p,__iGrid);
				(Result)->SetNormal(_aq_p,__iGrid);
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
}


//----------------------------------------------------------------------
// ntransform(s,P)
void CqShaderExecEnv::SO_ntransform( IqShaderData* tospace, IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	assert( pShader != 0 );

	__fVarying=(p)->Class()==class_varying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	if ( getRenderContext() )
	{
		__iGrid = 0;
		CqString _aq_tospace;
		(tospace)->GetString(_aq_tospace,__iGrid);
		CqMatrix mat;
		getRenderContext() ->matNSpaceToSpace( "current", _aq_tospace.c_str(), pShader->getTransform(), pTransform().get(), getRenderContext()->Time(), mat );
		__iGrid = 0;

		__iGrid = 0;
		const CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqVector3D _aq_p;
				(p)->GetNormal(_aq_p,__iGrid);
				(Result)->SetNormal(mat * _aq_p,__iGrid);
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		const CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqVector3D _aq_p;
				(p)->GetNormal(_aq_p,__iGrid);
				(Result)->SetNormal(_aq_p,__iGrid);
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
}


//----------------------------------------------------------------------
// ntransform(m,P)
void CqShaderExecEnv::SO_ntransformm( IqShaderData* tospace, IqShaderData* p, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	assert( pShader != 0 );

	__fVarying=(p)->Class()==class_varying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void CqShaderExecEnv::SO_cmix( IqShaderData* color0, IqShaderData* color1, IqShaderData* value, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(color0)->Class()==class_varying;
	__fVarying=(color1)->Class()==class_varying||__fVarying;
	__fVarying=(value)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void CqShaderExecEnv::SO_cmixc( IqShaderData* color0, IqShaderData* color1, IqShaderData* value, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(color0)->Class()==class_varying;
	__fVarying=(color1)->Class()==class_varying||__fVarying;
	__fVarying=(value)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqColor _aq_color0;
			(color0)->GetColor(_aq_color0,__iGrid);
			CqColor _aq_color1;
			(color1)->GetColor(_aq_color1,__iGrid);
			CqColor _aq_value;
			(value)->GetColor(_aq_value,__iGrid);
			TqFloat c1 = ( 1.0f - _aq_value[0] ) * _aq_color0[0] + _aq_value[0] * _aq_color1[0] ;
			TqFloat c2 = ( 1.0f - _aq_value[1] ) * _aq_color0[1] + _aq_value[1] * _aq_color1[1] ;
			TqFloat c3 = ( 1.0f - _aq_value[2] ) * _aq_color0[2] + _aq_value[2] * _aq_color1[2] ;
			(Result)->SetColor(CqColor(c1,c2,c3),__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_fmix( IqShaderData* f0, IqShaderData* f1, IqShaderData* value, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(f0)->Class()==class_varying;
	__fVarying=(f1)->Class()==class_varying||__fVarying;
	__fVarying=(value)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void    CqShaderExecEnv::SO_pmix( IqShaderData* p0, IqShaderData* p1, IqShaderData* value, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(p0)->Class()==class_varying;
	__fVarying=(p1)->Class()==class_varying||__fVarying;
	__fVarying=(value)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_pmixc( IqShaderData* p0, IqShaderData* p1, IqShaderData* value, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(p0)->Class()==class_varying;
	__fVarying=(p1)->Class()==class_varying||__fVarying;
	__fVarying=(value)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_p0;
			(p0)->GetPoint(_aq_p0,__iGrid);
			CqVector3D _aq_p1;
			(p1)->GetPoint(_aq_p1,__iGrid);
			CqColor _aq_value;
			(value)->GetColor(_aq_value,__iGrid);
			TqFloat p1 = ( 1.0f - _aq_value[0] ) * _aq_p0[0] + _aq_value[0] * _aq_p1[0] ;
			TqFloat p2 = ( 1.0f - _aq_value[1] ) * _aq_p0[1] + _aq_value[1] * _aq_p1[1] ;
			TqFloat p3 = ( 1.0f - _aq_value[2] ) * _aq_p0[2] + _aq_value[2] * _aq_p1[2] ;
			(Result)->SetPoint(CqVector3D(p1,p2,p3),__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void    CqShaderExecEnv::SO_vmix( IqShaderData* v0, IqShaderData* v1, IqShaderData* value, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(v0)->Class()==class_varying;
	__fVarying=(v1)->Class()==class_varying||__fVarying;
	__fVarying=(value)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_vmixc( IqShaderData* v0, IqShaderData* v1, IqShaderData* value, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(v0)->Class()==class_varying;
	__fVarying=(v1)->Class()==class_varying||__fVarying;
	__fVarying=(value)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_v0;
			(v0)->GetVector(_aq_v0,__iGrid);
			CqVector3D _aq_v1;
			(v1)->GetVector(_aq_v1,__iGrid);
			CqColor _aq_value;
			(value)->GetColor(_aq_value,__iGrid);
                        TqFloat v1 = ( 1.0f - _aq_value[0] ) * _aq_v0[0] + _aq_value[0] * _aq_v1[0] ;
                        TqFloat v2 = ( 1.0f - _aq_value[1] ) * _aq_v0[1] + _aq_value[1] * _aq_v1[1] ;
                        TqFloat v3 = ( 1.0f - _aq_value[2] ) * _aq_v0[2] + _aq_value[2] * _aq_v1[2] ;
                        (Result)->SetVector(CqVector3D(v1,v2,v3),__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_nmix( IqShaderData* n0, IqShaderData* n1, IqShaderData* value, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(n0)->Class()==class_varying;
	__fVarying=(n1)->Class()==class_varying||__fVarying;
	__fVarying=(value)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

void	CqShaderExecEnv::SO_nmixc( IqShaderData* n0, IqShaderData* n1, IqShaderData* value, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(n0)->Class()==class_varying;
	__fVarying=(n1)->Class()==class_varying||__fVarying;
	__fVarying=(value)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqVector3D _aq_n0;
			(n0)->GetNormal(_aq_n0,__iGrid);
			CqVector3D _aq_n1;
			(n1)->GetNormal(_aq_n1,__iGrid);
                        CqColor _aq_value;
                        (value)->GetColor(_aq_value,__iGrid);
                        TqFloat n1 = ( 1.0f - _aq_value[0] ) * _aq_n0[0] + _aq_value[0] * _aq_n1[0] ;
                        TqFloat n2 = ( 1.0f - _aq_value[1] ) * _aq_n0[1] + _aq_value[1] * _aq_n1[1] ;
                        TqFloat n3 = ( 1.0f - _aq_value[2] ) * _aq_n0[2] + _aq_value[2] * _aq_n1[2] ;
                        (Result)->SetNormal(CqVector3D(n1,n2,n3),__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


//----------------------------------------------------------------------
// ctransform(s,s,c)
void CqShaderExecEnv::SO_ctransform( IqShaderData* fromspace, IqShaderData* tospace, IqShaderData* c, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(c)->Class()==class_varying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqString fromSpaceName( "rgb" );
	if ( NULL != fromspace )
		fromspace->GetString( fromSpaceName );
	CqString toSpaceName;
	(tospace)->GetString(toSpaceName,__iGrid);

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqColor col;
			(c)->GetColor(col,__iGrid);
			if      (fromSpaceName == "hsv")  col = hsvtorgb(col);
			else if (fromSpaceName == "hsl")  col = hsltorgb(col);
			else if (fromSpaceName == "XYZ")  col = XYZtorgb(col);
			else if (fromSpaceName == "xyY")  col = xyYtorgb(col);
			else if (fromSpaceName == "YIQ")  col = YIQtorgb(col);

			if      (toSpaceName == "hsv")   col = rgbtohsv(col);
			else if (toSpaceName == "hsl")   col = rgbtohsl(col);
			else if (toSpaceName == "XYZ")   col = rgbtoXYZ(col);
			else if (toSpaceName == "xyY")   col = rgbtoxyY(col);
			else if (toSpaceName == "YIQ")   col = rgbtoYIQ(col);

			(Result)->SetColor(col,__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
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
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(P0)->Class()==class_varying;
	__fVarying=(P1)->Class()==class_varying||__fVarying;
	__fVarying=(Q)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// mtransform(s,s,M)
void CqShaderExecEnv::SO_mtransform( IqShaderData* fromspace, IqShaderData* tospace, IqShaderData* m, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	assert( pShader != 0 );

	__fVarying=(m)->Class()==class_varying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	if ( getRenderContext() )
	{
		__iGrid = 0;
		CqString _aq_fromspace;
		(fromspace)->GetString(_aq_fromspace,__iGrid);
		CqString _aq_tospace;
		(tospace)->GetString(_aq_tospace,__iGrid);
		CqMatrix mat;
		getRenderContext() ->matNSpaceToSpace( _aq_fromspace.c_str(), _aq_tospace.c_str(), pShader->getTransform(), pTransform().get(), getRenderContext()->Time(), mat );
		__iGrid = 0;

		__iGrid = 0;
		const CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqMatrix _aq_m;
				(m)->GetMatrix(_aq_m,__iGrid);
				(Result)->SetMatrix(mat * _aq_m,__iGrid);
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		const CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqMatrix _aq_m;
				(m)->GetMatrix(_aq_m,__iGrid);
				(Result)->SetMatrix(_aq_m,__iGrid);
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
}


//----------------------------------------------------------------------
// mtransform(s,M)
void CqShaderExecEnv::SO_mtransform( IqShaderData* tospace, IqShaderData* m, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	assert( pShader != 0 );

	__fVarying=(m)->Class()==class_varying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	if ( getRenderContext() )
	{
		__iGrid = 0;
		CqString _aq_tospace;
		(tospace)->GetString(_aq_tospace,__iGrid);
		CqMatrix mat;
		getRenderContext() ->matNSpaceToSpace( "current", _aq_tospace.c_str(), pShader->getTransform(), pTransform().get(), getRenderContext()->Time(), mat );
		__iGrid = 0;

		__iGrid = 0;
		const CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqMatrix _aq_m;
				(m)->GetMatrix(_aq_m,__iGrid);
				(Result)->SetMatrix(mat * _aq_m,__iGrid);
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		const CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqMatrix _aq_m;
				(m)->GetMatrix(_aq_m,__iGrid);
				(Result)->SetMatrix(_aq_m,__iGrid);
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
}



//----------------------------------------------------------------------
// determinant(m)
void CqShaderExecEnv::SO_determinant( IqShaderData* M, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(M)->Class()==class_varying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqMatrix _aq_M;
			(M)->GetMatrix(_aq_M,__iGrid);
			(Result)->SetFloat(_aq_M.Determinant(),__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


//----------------------------------------------------------------------
// translate(m,v)
void CqShaderExecEnv::SO_mtranslate( IqShaderData* M, IqShaderData* V, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(M)->Class()==class_varying;
	__fVarying=(V)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// rotate(m,v)
void CqShaderExecEnv::SO_mrotate( IqShaderData* M, IqShaderData* angle, IqShaderData* axis, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(M)->Class()==class_varying;
	__fVarying=(angle)->Class()==class_varying||__fVarying;
	__fVarying=(axis)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// scale(m,p)
void CqShaderExecEnv::SO_mscale( IqShaderData* M, IqShaderData* S, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(M)->Class()==class_varying;
	__fVarying=(S)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


//----------------------------------------------------------------------
// setmcomp(p,v)
void	CqShaderExecEnv::SO_setmcomp( IqShaderData* M, IqShaderData* r, IqShaderData* c, IqShaderData* v, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(M)->Class()==class_varying;
	__fVarying=(r)->Class()==class_varying||__fVarying;
	__fVarying=(c)->Class()==class_varying||__fVarying;
	__fVarying=(v)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
			_aq_M.SetfIdentity( false );
			M->SetValue( _aq_M, __iGrid );
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


//----------------------------------------------------------------------
// rotate(Q,angle,P0,P1)
void CqShaderExecEnv::SO_rotate( IqShaderData* Q, IqShaderData* angle, IqShaderData* P0, IqShaderData* P1, IqShaderData* Result, IqShader* pShader )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(Q)->Class()==class_varying;
	__fVarying=(angle)->Class()==class_varying||__fVarying;
	__fVarying=(P0)->Class()==class_varying||__fVarying;
	__fVarying=(P1)->Class()==class_varying||__fVarying;
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

} // namespace Aqsis
//---------------------------------------------------------------------
