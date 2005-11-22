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
		\brief Implements functions for the shader virtual machine.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	"aqsis.h"

#include	<iostream>

#include	<ctype.h>

#include	"shadervm.h"
#include	"logging.h"

START_NAMESPACE( Aqsis )

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
    AUTOFUNC;
    RESULT(type_float, class_uniform);
    pResult->SetFloat( ReadNext().m_FloatVal );
    Push( pResult );
}

void CqShaderVM::SO_puship()
{
    AUTOFUNC;
    TqFloat f = ReadNext().m_FloatVal;
    TqFloat f2 = ReadNext().m_FloatVal;
    TqFloat f3 = ReadNext().m_FloatVal;
    RESULT(type_point, class_uniform);
    CqVector3D v(f,f2,f3);
    pResult->SetValue(v);
    Push( pResult );
}

void CqShaderVM::SO_pushis()
{
    AUTOFUNC;
    RESULT(type_string, class_uniform);
    CqString * ps = ReadNext().m_pString;
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
    if ( pVar->ArrayLength() == 0 )
    {
        // Report error.
        std::cerr << critical << "Attempt to index a non array variable" << std::endl;
        return ;
    }
	//If either the value or the index is varying, so must the result be.
	RESULT(pVar->Type(), (pVar->Size()>1 || A->Size()>1)?class_varying:class_uniform);
    TqInt ext = m_pEnv->GridSize();
    TqBool fVarying = ext > 1;
    TqInt i;
    CqBitVector& RS = m_pEnv->RunningState();
    for ( i = 0; i < ext; i++ )
    {
        if(!fVarying || RS.Value( i ))
		{
			TqFloat _aq_A;
			A->GetFloat( _aq_A, i );
			pResult->SetValueFromVariable( pVar->ArrayEntry( static_cast<unsigned int>( _aq_A ) ), i );
		}
    }
    Push( pResult );
    RELEASE( A );
}

void CqShaderVM::SO_pop()
{
    AUTOFUNC;
    IqShaderData* pV = GetVar( ReadNext().m_iVariable );
    POPV( Val );
    TqUint ext = MAX( m_pEnv->GridSize(), pV->Size() );
    TqBool fVarying = ext > 1;
    TqInt i;
    CqBitVector& RS = m_pEnv->RunningState();
    for ( i = 0; i < ext; i++ )
    {
        if(!fVarying || RS.Value( i ))
            pV->SetValueFromVariable( Val, i );
    }
    RELEASE( Val );
}

void CqShaderVM::SO_ipop()
{
    AUTOFUNC;
    UsProgramElement& el = ReadNext();
    IqShaderData* pV = GetVar( el.m_iVariable );
    CqShaderVariableArray* pVA = static_cast<CqShaderVariableArray*>( pV );
    if ( pV->ArrayLength() == 0 )
    {
        // Report error.
        std::cerr << critical << "Attempt to index a non array variable" << std::endl;
        return ;
    }
    POPV( A );
    POPV( Val );
    //TqInt ext=__fVarying?m_pEnv->GridSize():1;
    TqUint ext = MAX( m_pEnv->GridSize(), pV->Size() );
    TqBool fVarying = ext > 1;
    TqInt i;
    CqBitVector& RS = m_pEnv->RunningState();
    for ( i = 0; i < ext; i++ )
    {
        if ( !fVarying || RS.Value( i ) )
        {
            TqFloat fIndex;
            A->GetFloat( fIndex, i );
            TqInt index = static_cast<unsigned int>( fIndex );
            ( *pVA ) [ index ] ->SetValueFromVariable( Val, i );
        }
    }
    RELEASE( Val );
    RELEASE( A );
}

void CqShaderVM::SO_mergef()
{
    // Get the current state from the current stack entry
    AUTOFUNC;
    POPV( A );	// Relational result
    POPV( F );	// False statement
    POPV( T );	// True statement
    RESULT(type_float, class_varying);
    TqInt i;
    TqInt ext = m_pEnv->GridSize();
    for ( i = 0; i < ext; i++ )
    {
        TqBool _aq_A;
        TqFloat _aq_T, _aq_F;
        A->GetBool( _aq_A, i );
        T->GetFloat( _aq_T, i );
        F->GetFloat( _aq_F, i );
        if ( _aq_A ) pResult->SetValue( _aq_T, i );
        else	pResult->SetValue( _aq_F, i );
    }
    Push( pResult );
    RELEASE( T );
    RELEASE( F );
    RELEASE( A );
}

void CqShaderVM::SO_merges()
{
    // Get the current state from the current stack entry
    AUTOFUNC;
    POPV( A );	// Relational result
    POPV( F );	// False statement
    POPV( T );	// True statement
    RESULT(type_string, class_varying);
    TqInt i;
    TqInt ext = m_pEnv->GridSize();
    for ( i = 0; i < ext; i++ )
    {
        TqBool _aq_A;
        CqString _aq_T, _aq_F;
        A->GetBool( _aq_A, i );
        T->GetString( _aq_T, i );
        F->GetString( _aq_F, i );
        if ( _aq_A ) pResult->SetValue( _aq_T, i );
        else	pResult->SetValue( _aq_F, i );
    }
    Push( pResult );
    RELEASE( T );
    RELEASE( F );
    RELEASE( A );
}

void CqShaderVM::SO_mergep()
{
    // Get the current state from the current stack entry
    AUTOFUNC;
    POPV( A );	// Relational result
    POPV( F );	// False statement
    POPV( T );	// True statement
    RESULT(type_point, class_varying);
    TqInt i;
    TqInt ext = m_pEnv->GridSize();
    for ( i = 0; i < ext; i++ )
    {
        TqBool _aq_A;
        CqVector3D _aq_T, _aq_F;
        A->GetBool( _aq_A, i );
        T->GetPoint( _aq_T, i );
        F->GetPoint( _aq_F, i );
        if ( _aq_A ) pResult->SetValue( _aq_T, i );
        else	pResult->SetValue( _aq_F, i );
    }
    Push( pResult );
    RELEASE( T );
    RELEASE( F );
    RELEASE( A );
}

void CqShaderVM::SO_mergec()
{
    // Get the current state from the current stack entry
    AUTOFUNC;
    POPV( A );	// Relational result
    POPV( F );	// False statement
    POPV( T );	// True statement
    RESULT(type_color, class_varying);
    TqInt i;
    TqInt ext = m_pEnv->GridSize();
    for ( i = 0; i < ext; i++ )
    {
        TqBool _aq_A;
        CqColor _aq_T, _aq_F;
        A->GetBool( _aq_A, i );
        T->GetColor( _aq_T, i );
        F->GetColor( _aq_F, i );
        if ( _aq_A ) pResult->SetValue( _aq_T, i );
        else	pResult->SetValue( _aq_F, i );
    }
    Push( pResult );
    RELEASE( T );
    RELEASE( F );
    RELEASE( A );
}

void CqShaderVM::SO_setfc()
{
    AUTOFUNC;
    POPV( A );
    RESULT(type_color, __fVarying?class_varying:class_uniform);
    OpCAST_FC( A, pResult, m_pEnv->RunningState() );
    Push( pResult );
    RELEASE( A );
}

void CqShaderVM::SO_setfp()
{
    AUTOFUNC;
    POPV( A );
    RESULT(type_point, __fVarying?class_varying:class_uniform);
    OpCAST_FP( A, pResult, m_pEnv->RunningState() );
    Push( pResult );
    RELEASE( A );
}


void CqShaderVM::SO_setfm()
{
    AUTOFUNC;
    POPV( A );
    RESULT(type_matrix, __fVarying?class_varying:class_uniform);
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
    OpCAST_PC( A, pResult, m_pEnv->RunningState() );
    Push( pResult );
    RELEASE( A );
}

void CqShaderVM::SO_setcp()
{
    AUTOFUNC;
    POPV( A );
    RESULT(type_point, __fVarying?class_varying:class_uniform);
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
    TqInt i;
    CqBitVector& RS = m_pEnv->RunningState();
    TqInt ext = m_pEnv->GridSize();
    for ( i = 0; i < ext; i++ )
    {
        if ( RS.Value( i ) )
        {
            TqBool _aq_A;
            A->GetBool( _aq_A, i );
            m_pEnv->CurrentState().SetValue( i, _aq_A );
        }
    }
    RELEASE( A );
}

void CqShaderVM::SO_RS_JZ()
{
    SqLabel lab = ReadNext().m_Label;
    if ( m_pEnv->RunningState().Count() == 0 )
    {
        m_PO = lab.m_Offset;
        m_PC = lab.m_pAddress;
    }
}

void CqShaderVM::SO_RS_JNZ()
{
    SqLabel lab = ReadNext().m_Label;
    if ( m_pEnv->RunningState().Count() == m_pEnv->RunningState().Size() )
    {
        m_PO = lab.m_Offset;
        m_PC = lab.m_pAddress;
    }
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

void CqShaderVM::SO_S_JNZ()
{
    SqLabel lab = ReadNext().m_Label;
    if ( m_pEnv->CurrentState().Count() == m_pEnv->RunningState().Size() )
    {
        m_PO = lab.m_Offset;
        m_PC = lab.m_pAddress;
    }
}

void CqShaderVM::SO_jnz()
{
    SqLabel lab = ReadNext().m_Label;
    AUTOFUNC;
    IqShaderData* f = POP.m_Data;
    TqInt __iGrid = 0;
    do
    {
        if ( !__fVarying || m_pEnv->RunningState().Value( __iGrid ) )
        {
            TqBool _f;
            f->GetBool( _f, __iGrid );
            if ( !_f ) return ;
        }
    }
    while ( ++__iGrid < m_pEnv->GridSize() );
    m_PO = lab.m_Offset;
    m_PC = lab.m_pAddress;
}

void CqShaderVM::SO_jz()
{
    SqLabel lab = ReadNext().m_Label;
    AUTOFUNC;
    IqShaderData* f = POP.m_Data;
    TqInt __iGrid = 0;
    do
    {
        if ( !__fVarying || m_pEnv->RunningState().Value( __iGrid ) )
        {
            TqBool _f;
            f->GetBool( _f, __iGrid );
            if ( _f ) return ;
        }
    }
    while ( ++__iGrid < m_pEnv->GridSize() );
    m_PO = lab.m_Offset;
    m_PC = lab.m_pAddress;
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
    OpSUB_CC( A, B, pResult, m_pEnv->RunningState() );
    Push( pResult );
    RELEASE( B );
    RELEASE( A );
}

void CqShaderVM::SO_crscc()
{
    AUTOFUNC;
    POPV( A );
    POPV( B );
    RESULT(type_color, __fVarying?class_varying:class_uniform);
    OpCRS_CC( A, B, pResult, m_pEnv->RunningState() );
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
    OpSUB_FC( A, B, pResult, m_pEnv->RunningState() );
    Push( pResult );
    RELEASE( B );
    RELEASE( A );
}

void CqShaderVM::SO_mulmm()
{
    AUTOFUNC;
    RESULT(type_float, class_uniform);
    pResult->SetFloat( 0.0f );
    Push( pResult );	/* TODO: Implement matrices in the VM*/
}

void CqShaderVM::SO_divmm()
{
    AUTOFUNC;
    RESULT(type_float, class_uniform);
    pResult->SetFloat( 0.0f );
    Push( pResult );	/* TODO: Implement matrices in the VM*/
}

void CqShaderVM::SO_land()
{
    AUTOFUNC;
    POPV( A );
    POPV( B );
    RESULT(type_float, __fVarying?class_varying:class_uniform);
    OpLAND_B( A, B, pResult, m_pEnv->RunningState() );
    Push( pResult );
    RELEASE( B );
    RELEASE( A );
}

void CqShaderVM::SO_lor()
{
    AUTOFUNC;
    POPV( A );
    POPV( B );
    RESULT(type_float, __fVarying?class_varying:class_uniform);
    OpLOR_B( A, B, pResult, m_pEnv->RunningState() );
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

END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------
