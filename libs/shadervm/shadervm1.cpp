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
		\brief Implements functions for the shader virtual machine.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include "shadervm.h"

#include <iostream>

#include <aqsis/util/logging.h>
#include "shadeopmacros.h"

namespace Aqsis {

DECLARE_SHADERSTACK_TEMPS

void CqShaderVM::SO_nop()
{}

void CqShaderVM::SO_dup()
{
	Dup();
}

void CqShaderVM::SO_drop()
{
	Drop();
}

void CqShaderVM::SO_debug_break()
{}

void CqShaderVM::SO_pushif()
{
	CONSTFUNC;
	RESULT(type_float, class_uniform);
	TqFloat f = ReadNext().m_FloatVal;
	// We need to perform this operation regardless of whether any SIMD
	// elements are running, since it's used to push the parameter count of
	// varargs functions...
	pResult->SetFloat(f);
	Push( pResult );
}

void CqShaderVM::SO_puship()
{
	CONSTFUNC;
	TqFloat f = ReadNext().m_FloatVal;
	TqFloat f2 = ReadNext().m_FloatVal;
	TqFloat f3 = ReadNext().m_FloatVal;
	RESULT(type_point, class_uniform);
	if(m_pEnv->IsRunning())
	{
		CqVector3D v(f,f2,f3);
		pResult->SetValue(v);
	}
	Push( pResult );
}

void CqShaderVM::SO_pushis()
{
	CONSTFUNC;
	RESULT(type_string, class_uniform);
	CqString * ps = ReadNext().m_pString;
	if(m_pEnv->IsRunning())
		pResult->SetValue( *ps );
	Push( pResult );
}

void CqShaderVM::SO_pushv()
{
	PushV( GetVar( ReadNext().m_iVariable ) );
}

void CqShaderVM::SO_ipushv()
{
	AUTOFUNC;
	POPV( A );	// Index
	IqShaderData* pVar = GetVar( ReadNext().m_iVariable );
	// If either the value or the index is varying, so must the result be.
	RESULT(pVar->Type(), (pVar->Size()>1 || A->Size()>1)?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
	{
		TqInt ext = m_pEnv->shadingPointCount();
		bool fVarying = ext > 1;
		TqInt arrayLen = pVar->ArrayLength();
		const CqBitVector& RS = m_pEnv->RunningState();
		for(TqInt i = 0; i < ext; i++)
		{
			if(!fVarying || RS.Value(i))
			{
				TqFloat aVal;
				A->GetFloat(aVal, i);
				TqInt index = lfloor(aVal);
				if(index >= 0 && index < arrayLen)
				{
					pResult->SetValueFromVariable(pVar->ArrayEntry(index), i);
				}
				else
				{
					Aqsis::log() << error
						<< "indexing array out of bounds: " << pVar->strName()
						<< "[" << A->strName() << "=" << index << "]\n";
				}
			}
		}
	}
	Push( pResult );
	RELEASE( A );
}

void CqShaderVM::SO_pop()
{
	AUTOFUNC;
	TqInt iVar = ReadNext().m_iVariable;
	IqShaderData* pV = GetVar( iVar );
	POPV( Val );
	if(m_pEnv->IsRunning())
	{
		TqUint ext = max( m_pEnv->shadingPointCount(), pV->Size() );
		bool fVarying = ext > 1;
		TqUint i;
		const CqBitVector& RS = m_pEnv->RunningState();
		for ( i = 0; i < ext; i++ )
		{
			if(!fVarying || RS.Value( i ))
				pV->SetValueFromVariable( Val, i );
		}
	}
	RELEASE( Val );
}

void CqShaderVM::SO_ipop()
{
	AUTOFUNC;
	IqShaderData* pVar = GetVar(ReadNext().m_iVariable);
	POPV( A );
	POPV( Val );
	if(m_pEnv->IsRunning())
	{
		TqInt ext = max( m_pEnv->shadingPointCount(), pVar->Size() );
		bool fVarying = ext > 1;
		TqInt arrayLen = pVar->ArrayLength();
		const CqBitVector& RS = m_pEnv->RunningState();
		for(TqInt i = 0; i < ext; i++)
		{
			if ( !fVarying || RS.Value( i ) )
			{
				TqFloat fIndex;
				A->GetFloat(fIndex, i);
				TqInt index = lfloor(fIndex);
				if(index >= 0 && index < arrayLen)
				{
					pVar->ArrayEntry(index)->SetValueFromVariable(Val, i);
				}
				else
				{
					Aqsis::log() << error
						<< "indexing array out of bounds: " << pVar->strName()
						<< "[" << A->strName() << "=" << index << "]\n";
				}
			}
		}
	}
	RELEASE( Val );
	RELEASE( A );
}

void CqShaderVM::SO_mergef()
{
	// Get the current state from the current stack entry
	AUTOFUNC;
	POPV( F );	// False statement
	POPV( T );	// True statement
	POPV( A );	// Relational result
	RESULT(type_float, class_varying);
	if(m_pEnv->IsRunning())
	{
		TqInt i;
		TqInt ext = m_pEnv->shadingPointCount();
		for ( i = 0; i < ext; i++ )
		{
			bool _aq_A;
			TqFloat _aq_T, _aq_F;
			A->GetBool( _aq_A, i );
			T->GetFloat( _aq_T, i );
			F->GetFloat( _aq_F, i );
			if ( _aq_A )
				pResult->SetValue( _aq_T, i );
			else
				pResult->SetValue( _aq_F, i );
		}
	}
	Push( pResult );
	RELEASE( A );
	RELEASE( T );
	RELEASE( F );
}

void CqShaderVM::SO_merges()
{
	// Get the current state from the current stack entry
	AUTOFUNC;
	POPV( F );	// False statement
	POPV( T );	// True statement
	POPV( A );	// Relational result
	RESULT(type_string, class_varying);
	if(m_pEnv->IsRunning())
	{
		TqInt i;
		TqInt ext = m_pEnv->shadingPointCount();
		for ( i = 0; i < ext; i++ )
		{
			bool _aq_A;
			CqString _aq_T, _aq_F;
			A->GetBool( _aq_A, i );
			T->GetString( _aq_T, i );
			F->GetString( _aq_F, i );
			if ( _aq_A )
				pResult->SetValue( _aq_T, i );
			else
				pResult->SetValue( _aq_F, i );
		}
	}
	Push( pResult );
	RELEASE( A );
	RELEASE( T );
	RELEASE( F );
}

void CqShaderVM::SO_mergep()
{
	// Get the current state from the current stack entry
	AUTOFUNC;
	POPV( F );	// False statement
	POPV( T );	// True statement
	POPV( A );	// Relational result
	RESULT(type_point, class_varying);
	if(m_pEnv->IsRunning())
	{
		TqInt i;
		TqInt ext = m_pEnv->shadingPointCount();
		for ( i = 0; i < ext; i++ )
		{
			bool _aq_A;
			CqVector3D _aq_T, _aq_F;
			A->GetBool( _aq_A, i );
			T->GetPoint( _aq_T, i );
			F->GetPoint( _aq_F, i );
			if ( _aq_A )
				pResult->SetValue( _aq_T, i );
			else
				pResult->SetValue( _aq_F, i );
		}
	}
	Push( pResult );
	RELEASE( A );
	RELEASE( T );
	RELEASE( F );
}

void CqShaderVM::SO_mergec()
{
	// Get the current state from the current stack entry
	AUTOFUNC;
	POPV( F );	// False statement
	POPV( T );	// True statement
	POPV( A );	// Relational result
	RESULT(type_color, class_varying);
	if(m_pEnv->IsRunning())
	{
		TqInt i;
		TqInt ext = m_pEnv->shadingPointCount();
		for ( i = 0; i < ext; i++ )
		{
			bool _aq_A;
			CqColor _aq_T, _aq_F;
			A->GetBool( _aq_A, i );
			T->GetColor( _aq_T, i );
			F->GetColor( _aq_F, i );
			if ( _aq_A )
				pResult->SetValue( _aq_T, i );
			else
				pResult->SetValue( _aq_F, i );
		}
	}
	Push( pResult );
	RELEASE( A );
	RELEASE( T );
	RELEASE( F );
}

void CqShaderVM::SO_setfc()
{
	AUTOFUNC;
	POPV( A );
	RESULT(type_color, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpCAST_FC( A, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( A );
}

void CqShaderVM::SO_setfp()
{
	AUTOFUNC;
	POPV( A );
	RESULT(type_point, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpCAST_FP( A, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( A );
}


void CqShaderVM::SO_setfm()
{
	AUTOFUNC;
	POPV( A );
	RESULT(type_matrix, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpCAST_FM( A, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( A );
}

void CqShaderVM::SO_settc()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	POPV( C );
	RESULT(type_color, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpTRIPLE_C( pResult, A, B, C, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( C );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_settp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	POPV( C );
	RESULT(type_point, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpTRIPLE_P( pResult, A, B, C, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( C );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_setpc()
{
	AUTOFUNC;
	POPV( A );
	RESULT(type_color, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpCAST_PC( A, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( A );
}

void CqShaderVM::SO_setcp()
{
	AUTOFUNC;
	POPV( A );
	RESULT(type_point, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpCAST_CP( A, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( A );
}

void CqShaderVM::SO_setwm()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	POPV( C );
	POPV( D );
	POPV( E );
	POPV( F );
	POPV( G );
	POPV( H );
	POPV( I );
	POPV( J );
	POPV( K );
	POPV( L );
	POPV( M );
	POPV( N );
	POPV( O );
	POPV( P );
	RESULT(type_matrix, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpHEXTUPLE_M( pResult, P, O, N, M, L, K, J, I, H, G, F, E, D, C, B, A, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( P );
	RELEASE( O );
	RELEASE( N );
	RELEASE( M );
	RELEASE( L );
	RELEASE( K );
	RELEASE( J );
	RELEASE( I );
	RELEASE( H );
	RELEASE( G );
	RELEASE( F );
	RELEASE( E );
	RELEASE( D );
	RELEASE( C );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_RS_PUSH()
{
	m_pEnv->PushState();
}

void CqShaderVM::SO_RS_POP()
{
	m_pEnv->PopState();
}

void CqShaderVM::SO_RS_GET()
{
	m_pEnv->GetCurrentState();
}

void CqShaderVM::SO_RS_INVERSE()
{
	m_pEnv->InvertRunningState();
}

void CqShaderVM::SO_S_CLEAR()
{
	m_pEnv->ClearCurrentState();
}

void CqShaderVM::SO_S_GET()
{
	// Get the current state from the current stack entry
	AUTOFUNC;
	POPV( A );
	if(m_pEnv->IsRunning())
	{
		TqInt i;
		const CqBitVector& RS = m_pEnv->RunningState();
		TqInt ext = m_pEnv->shadingPointCount();
		for ( i = 0; i < ext; i++ )
		{
			if ( RS.Value( i ) )
			{
				bool _aq_A;
				A->GetBool( _aq_A, i );
				m_pEnv->CurrentState().SetValue( i, _aq_A );
			}
		}
	}
	RELEASE( A );
}

void CqShaderVM::SO_RS_JZ()
{
	SqLabel lab = ReadNext().m_Label;
	if ( !m_pEnv->IsRunning() )
	{
		m_PO = lab.m_Offset;
		m_PC = lab.m_pAddress;
	}
}

void CqShaderVM::SO_RS_BREAK()
{
	TqInt breakDepth = ReadNext().m_intVal;
	m_pEnv->RunningStatesBreak(breakDepth);
}

void CqShaderVM::SO_S_JZ()
{
	SqLabel lab = ReadNext().m_Label;
	if ( m_pEnv->CurrentState().Count() == 0 )
	{
		m_PO = lab.m_Offset;
		m_PC = lab.m_pAddress;
	}
}

void CqShaderVM::SO_jnz()
{
	SqLabel lab = ReadNext().m_Label;
	AUTOFUNC;
	SqStackEntry stack = POP;
	IqShaderData* f = stack.m_Data;
	TqUint __iGrid = 0;
	const CqBitVector& RS = m_pEnv->RunningState();
	do
	{
		if ( !__fVarying || RS.Value( __iGrid ) )
		{
			bool _f;
			f->GetBool( _f, __iGrid );
			if ( !_f )
			{
				Release(stack);
				return ;
			}
		}
	}
	while ( ++__iGrid < m_pEnv->shadingPointCount() );
	m_PO = lab.m_Offset;
	m_PC = lab.m_pAddress;
	Release(stack);
}

void CqShaderVM::SO_jz()
{
	SqLabel lab = ReadNext().m_Label;
	AUTOFUNC;
	SqStackEntry stack = POP;
	IqShaderData* f = stack.m_Data;
	TqUint __iGrid = 0;
	const CqBitVector& RS = m_pEnv->RunningState();
	do
	{
		if ( !__fVarying || RS.Value( __iGrid ) )
		{
			bool _f;
			f->GetBool( _f, __iGrid );
			if ( _f )
			{
				Release(stack);
				return ;
			}
		}
	}
	while ( ++__iGrid < m_pEnv->shadingPointCount() );
	m_PO = lab.m_Offset;
	m_PC = lab.m_pAddress;
	Release(stack);
}

void CqShaderVM::SO_jmp()
{
	SqLabel lab = ReadNext().m_Label;
	m_PO = lab.m_Offset;
	m_PC = lab.m_pAddress;
}

void CqShaderVM::SO_lsff()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpLSS_FF( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_lspp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpLSS_PP( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_lscc()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpLSS_CC( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_gtff()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpGRT_FF( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_gtpp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpGRT_PP( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_gtcc()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpGRT_CC( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_geff()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpGE_FF( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_gepp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpGE_PP( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_gecc()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpGE_CC( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_leff()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpLE_FF( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_lepp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpLE_PP( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_lecc()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpLE_CC( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_eqff()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpEQ_FF( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_eqpp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpEQ_PP( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_eqcc()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpEQ_CC( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_eqss()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpEQ_SS( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_neff()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpNE_FF( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_nepp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpNE_PP( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_necc()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpNE_CC( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_ness()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpNE_SS( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_mulff()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpMUL_FF( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_divff()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpDIV_FF( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_addff()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpADD_FF( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_subff()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpSUB_FF( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_negf()
{
	AUTOFUNC;
	POPV( A );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpNEG_F( A, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( A );
}

void CqShaderVM::SO_mulpp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_point, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpMULV( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_divpp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_point, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpDIV_PP( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_addpp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_point, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpADD_PP( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_subpp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_point, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpSUB_PP( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_crspp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_point, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpCRS_PP( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_dotpp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpDOT_PP( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_negp()
{
	AUTOFUNC;
	POPV( A );
	RESULT(type_point, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpNEG_P( A, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( A );
}

void CqShaderVM::SO_mulcc()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_color, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpMUL_CC( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_divcc()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_color, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpDIV_CC( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_addcc()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_color, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpADD_CC( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_subcc()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_color, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpSUB_CC( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_dotcc()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_float, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpDOT_CC( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_negc()
{
	AUTOFUNC;
	POPV( A );
	RESULT(type_color, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpNEG_C( A, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( A );
}

void CqShaderVM::SO_mulfp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_point, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpMUL_FP( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_divfp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_point, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpDIV_FP( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_addfp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_point, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpADD_FP( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_subfp()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_point, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpSUB_FP( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_mulfc()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_color, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpMUL_FC( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_divfc()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_color, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpDIV_FC( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_addfc()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_color, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpADD_FC( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_subfc()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_color, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpSUB_FC( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_mulmm()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_matrix, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpMUL_MM( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}

void CqShaderVM::SO_divmm()
{
	AUTOFUNC;
	POPV( A );
	POPV( B );
	RESULT(type_matrix, __fVarying?class_varying:class_uniform);
	if(m_pEnv->IsRunning())
		OpDIVMM( A, B, pResult, m_pEnv->RunningState() );
	Push( pResult );
	RELEASE( B );
	RELEASE( A );
}


void CqShaderVM::SO_radians()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_radians );
}

void CqShaderVM::SO_degrees()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_degrees );
}

void CqShaderVM::SO_sin()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_sin );
}

void CqShaderVM::SO_asin()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_asin );
}

void CqShaderVM::SO_cos()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_cos );
}

void CqShaderVM::SO_acos()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_acos );
}

void CqShaderVM::SO_tan()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_tan );
}

void CqShaderVM::SO_atan()
{
	AUTOFUNC;
	FUNC1( type_float, m_pEnv->SO_atan );
}

void CqShaderVM::SO_atan2()
{
	AUTOFUNC;
	FUNC2( type_float, m_pEnv->SO_atan );
}

void CqShaderVM::SO_pow()
{
	AUTOFUNC;
	FUNC2( type_float, m_pEnv->SO_pow );
}

} // namespace Aqsis
//---------------------------------------------------------------------
