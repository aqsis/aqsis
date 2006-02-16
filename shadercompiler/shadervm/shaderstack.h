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
		\brief Declares the classes and support structures for the shader VM stack.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef SHADERSTACK_H_INCLUDED
#define SHADERSTACK_H_INCLUDED 1

#include	<stack>
#include	<vector>
#include	<deque>

#include	"aqsis.h"

#include	"vector3d.h"
#include	"vector4d.h"
#include	"color.h"
#include	"sstring.h"
#include	"matrix.h"
#include	"ishaderdata.h"
#include	"bitvector.h"
#include	"shadervariable.h"

START_NAMESPACE( Aqsis )

#define DECLARE_SHADERSTACK_TEMPS \
static TqFloat temp_float; \
static CqVector3D temp_point; \
static CqColor temp_color; \
static CqString temp_string; \
static CqMatrix temp_matrix; 

#define	OpLSS_FF(a,b,Res,State)		OpLSS(temp_float,temp_float,temp_float,a,b,Res,State)
#define	OpLSS_PP(a,b,Res,State)		OpLSS(temp_point,temp_point,temp_float,a,b,Res,State)
#define	OpLSS_CC(a,b,Res,State)		OpLSS(temp_color,temp_color,temp_float,a,b,Res,State)

#define	OpGRT_FF(a,b,Res,State)		OpGRT(temp_float,temp_float,temp_float,a,b,Res,State)
#define	OpGRT_PP(a,b,Res,State)		OpGRT(temp_point,temp_point,temp_float,a,b,Res,State)
#define	OpGRT_CC(a,b,Res,State)		OpGRT(temp_color,temp_color,temp_float,a,b,Res,State)

#define	OpLE_FF(a,b,Res,State)		OpLE(temp_float,temp_float,temp_float,a,b,Res,State)
#define	OpLE_PP(a,b,Res,State)		OpLE(temp_point,temp_point,temp_float,a,b,Res,State)
#define	OpLE_CC(a,b,Res,State)		OpLE(temp_color,temp_color,temp_float,a,b,Res,State)

#define	OpGE_FF(a,b,Res,State)		OpGE(temp_float,temp_float,temp_float,a,b,Res,State)
#define	OpGE_PP(a,b,Res,State)		OpGE(temp_point,temp_point,temp_float,a,b,Res,State)
#define	OpGE_CC(a,b,Res,State)		OpGE(temp_color,temp_color,temp_float,a,b,Res,State)

#define	OpEQ_FF(a,b,Res,State)		OpEQ(temp_float,temp_float,temp_float,a,b,Res,State)
#define	OpEQ_PP(a,b,Res,State)		OpEQ(temp_point,temp_point,temp_float,a,b,Res,State)
#define	OpEQ_CC(a,b,Res,State)		OpEQ(temp_color,temp_color,temp_float,a,b,Res,State)
#define	OpEQ_SS(a,b,Res,State)		OpEQ(temp_string,temp_string,temp_float,a,b,Res,State)

#define	OpNE_FF(a,b,Res,State)		OpNE(temp_float,temp_float,temp_float,a,b,Res,State)
#define	OpNE_PP(a,b,Res,State)		OpNE(temp_point,temp_point,temp_float,a,b,Res,State)
#define	OpNE_CC(a,b,Res,State)		OpNE(temp_color,temp_color,temp_float,a,b,Res,State)
#define	OpNE_SS(a,b,Res,State)		OpNE(temp_string,temp_string,temp_float,a,b,Res,State)

#define	OpMUL_FF(a,b,Res,State)		OpMUL(temp_float,temp_float,temp_float,a,b,Res,State)
#define	OpDIV_FF(a,b,Res,State)		OpDIV(temp_float,temp_float,temp_float,a,b,Res,State)
#define	OpADD_FF(a,b,Res,State)		OpADD(temp_float,temp_float,temp_float,a,b,Res,State)
#define	OpSUB_FF(a,b,Res,State)		OpSUB(temp_float,temp_float,temp_float,a,b,Res,State)
#define	OpNEG_F(a,Res,State)		OpNEG(temp_float,a,Res,State)

#define	OpMUL_PP(a,b,Res,State)		OpMUL(temp_point,temp_point,temp_point,a,b,Res,State)
#define	OpDIV_PP(a,b,Res,State)		OpDIV(temp_point,temp_point,temp_point,a,b,Res,State)
#define	OpADD_PP(a,b,Res,State)		OpADD(temp_point,temp_point,temp_point,a,b,Res,State)
#define	OpSUB_PP(a,b,Res,State)		OpSUB(temp_point,temp_point,temp_point,a,b,Res,State)
#define	OpCRS_PP(a,b,Res,State)		OpCRS(temp_point,temp_point,temp_point,a,b,Res,State)
#define	OpDOT_PP(a,b,Res,State)		OpDOT(temp_point,temp_point,temp_float,a,b,Res,State)
#define	OpNEG_P(a,Res,State)		OpNEG(temp_point,a,Res,State)

#define	OpMUL_CC(a,b,Res,State)		OpMUL(temp_color,temp_color,temp_color,a,b,Res,State)
#define	OpDIV_CC(a,b,Res,State)		OpDIV(temp_color,temp_color,temp_color,a,b,Res,State)
#define	OpADD_CC(a,b,Res,State)		OpADD(temp_color,temp_color,temp_color,a,b,Res,State)
#define	OpSUB_CC(a,b,Res,State)		OpSUB(temp_color,temp_color,temp_color,a,b,Res,State)
#define	OpCRS_CC(a,b,Res,State)		OpCRS(temp_color,temp_color,temp_color,a,b,Res,State)
#define	OpDOT_CC(a,b,Res,State)		OpDOT(temp_color,temp_color,temp_color,a,b,Res,State)
#define	OpNEG_C(a,Res,State)		OpNEG(temp_color,a,Res,State)

#define	OpMUL_FP(a,b,Res,State)		OpMUL(temp_float,temp_point,temp_point,a,b,Res,State)
#define	OpDIV_FP(a,b,Res,State)		OpDIV(temp_float,temp_point,temp_point,a,b,Res,State)
#define	OpADD_FP(a,b,Res,State)		OpADD(temp_float,temp_point,temp_point,a,b,Res,State)
#define	OpSUB_FP(a,b,Res,State)		OpSUB(temp_float,temp_point,temp_point,a,b,Res,State)

#define	OpMUL_FC(a,b,Res,State)		OpMUL(temp_float,temp_color,temp_color,a,b,Res,State)
#define	OpDIV_FC(a,b,Res,State)		OpDIV(temp_float,temp_color,temp_color,a,b,Res,State)
#define	OpADD_FC(a,b,Res,State)		OpADD(temp_float,temp_color,temp_color,a,b,Res,State)
#define	OpSUB_FC(a,b,Res,State)		OpSUB(temp_float,temp_color,temp_color,a,b,Res,State)

#define	OpLAND_B(a,b,Res,State)		OpLAND(temp_float,temp_float,temp_float,a,b,Res,State)
#define	OpLOR_B(a,b,Res,State)		OpLOR(temp_float,temp_float,temp_float,a,b,Res,State)

#define	OpCAST_FC(a,Res,State)		OpCAST(temp_float,temp_color, a,Res,State)
#define	OpCAST_FP(a,Res,State)		OpCAST(temp_float,temp_point, a,Res,State)
#define	OpCAST_PC(a,Res,State)		OpCAST(temp_point,temp_color, a,Res,State)
#define	OpCAST_CP(a,Res,State)		OpCAST(temp_color,temp_point, a,Res,State)
#define	OpCAST_FM(a,Res,State)		OpCAST(temp_float,temp_matrix, a,Res,State)

#define	OpTRIPLE_C(r,a,b,c,State)	OpTRIPLE(temp_color,r,a,b,c,State)
#define	OpTRIPLE_P(r,a,b,c,State)	OpTRIPLE(temp_point,r,a,b,c,State)

#define	OpHEXTUPLE_M(r,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,State)	OpHEXTUPLE(temp_matrix,r,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,State)

#define	OpCOMP_C(a,index,Res,State)	OpCOMP(temp_color,a,index,Res,State)
#define	OpCOMP_P(a,index,Res,State)	OpCOMP(temp_point,a,index,Res,State)

#define	OpSETCOMP_C(r,index,a,State)	OpSETCOMP(temp_color,r,index,a,State)
#define	OpSETCOMP_P(r,index,a,State)	OpSETCOMP(temp_point,r,index,a,State)

//---------------------------------------------------------------------
//
// Define macros for defining Opcodes efficiently
// a The type of the first operand, used to determine templateisation, needed by VC++..
// b The type of the second operand, used to determine templateisation, needed by VC++..
// Comp The stack entry to use as the second operand.
// Res The stack entry to store the result in.
// RunningState The current SIMD state.

#define OpABRS(OP, NAME) \
		template <class A, class B, class R>	\
		inline void	Op##NAME( A& a, B&b, R& r, IqShaderData* pA, IqShaderData* pB, IqShaderData* pRes, CqBitVector& RunningState ) \
		{ \
			A vA; \
			B vB; \
			A* pdA; \
			B* pdB; \
			R* pdR; \
			TqInt i, ii; \
			\
			TqBool fAVar = pA->Size() > 1; \
			TqBool fBVar = pB->Size() > 1; \
			\
			if( fAVar && fBVar )\
			{ \
				/* Both are varying, must go accross all processing each element. */ \
				pA->GetValuePtr( pdA ); \
				pB->GetValuePtr( pdB ); \
				pRes->GetValuePtr( pdR ); \
				ii = pA->Size(); \
				for ( i = 0; i < ii; i++ ) \
				{ \
					if ( RunningState.Value( i ) ) \
						(*pdR) = ( (*pdA) OP (*pdB) ); \
					pdA++; \
					pdB++; \
					pdR++; \
				} \
			} \
			else if( !fBVar && fAVar) \
			{ \
				/* A is varying, can just get B's value once. */ \
				ii = pA->Size(); \
				pA->GetValuePtr( pdA ); \
				pB->GetValue( vB ); \
				pRes->GetValuePtr( pdR ); \
				for ( i = 0; i < ii; i++ ) \
				{ \
					if ( RunningState.Value( i ) ) \
						(*pdR) = ( (*pdA) OP vB ); \
					pdA++; \
					pdR++; \
				} \
			} \
			else if( !fAVar && fBVar) \
			{ \
				/* B is varying, can just get A's value once. */ \
				ii = pB->Size(); \
				pB->GetValuePtr( pdB ); \
				pA->GetValue( vA ); \
				pRes->GetValuePtr( pdR ); \
				for ( i = 0; i < ii; i++ ) \
				{ \
					if ( RunningState.Value( i ) ) \
						(*pdR) = ( vA OP (*pdB) ); \
					pdB++; \
					pdR++; \
				} \
			} \
			else \
			{ \
				/* Both are uniform, simple one shot case. */ \
				pA->GetValue( vA ); \
				pB->GetValue( vB ); \
				pRes->SetValue( vA OP vB ); \
			} \
		}

//----------------------------------------------------------------------
/** \class CqShaderStack
 * Class handling the shader execution stack.
 */

struct	SqStackEntry
{
	TqBool			m_IsTemp;
	IqShaderData*	m_Data;
};

class CqShaderStack
{
	public:
		CqShaderStack() : m_iTop( 0 )
		{
			TqInt n = MAX(m_maxsamples, m_samples);
			m_Stack.resize( n );
		}
		virtual ~CqShaderStack()
		{
			m_Stack.clear();
			Statistics();
		}


		IqShaderData* GetNextTemp( EqVariableType type, EqVariableClass _class );

		//----------------------------------------------------------------------
		/** Push a new shader variable reference onto the stack.
		 */
		void	Push( IqShaderData* pv )
		{
			while ( m_iTop >= m_Stack.size() )
			{
				TqInt n = m_Stack.size() + 1;
				m_Stack.resize( n );
				m_Stack.reserve( n );
			}

			m_Stack[ m_iTop ].m_Data = pv;
			m_Stack[ m_iTop ].m_IsTemp = TqTrue;
			m_iTop ++;


		}

		//----------------------------------------------------------------------
		/** Push a new shader variable reference onto the stack.
		 */
		void	PushV( IqShaderData* pv )
		{
			assert( NULL != pv );
			while ( m_iTop >= m_Stack.size() )
			{
				TqInt n = m_Stack.size() + 1;
				m_Stack.resize( n );
				m_Stack.reserve( n );
			}

			m_Stack[ m_iTop ].m_Data = pv;
			m_Stack[ m_iTop ].m_IsTemp = TqFalse;
			m_iTop ++;


		}

		//----------------------------------------------------------------------
		/** Pop the top stack entry.
		 * \param f Boolean value to update if this is varying. If not varying, leaves f unaltered.
		 * \return Reference to the top stack entry.
		 */
		SqStackEntry Pop( TqBool& f )
		{
			if ( m_iTop )
				m_iTop--;

			SqStackEntry Val = m_Stack[ m_iTop ];
			f = Val.m_Data->Size() > 1 || f;

			return ( Val );
		}

		void Release( SqStackEntry s );

		//----------------------------------------------------------------------
		/** Duplicate the top stack entry.
		 */
		void	Dup()
		{
			TqInt iTop = m_iTop-1;

			IqShaderData* s = GetNextTemp( m_Stack[ iTop ].m_Data ->Type(), m_Stack[ iTop ].m_Data ->Class() );
			s->SetValueFromVariable( m_Stack[ iTop ].m_Data );
			Push( s );


		}

		/** Drop the top stack entry.
		 */
		void	Drop()
		{
			TqBool f = TqFalse;
			Pop( f );


		}

		/**
		 * Print the max number of depth if compiled for it.
		 */
		static void Statistics();


		/** set the more efficient number of samples per type of variable at run-time.
		 */
		static void	SetSamples(TqInt n)
		{
			m_samples = n;

		}



	protected:
		std::vector<SqStackEntry>	m_Stack;
		TqUint	m_iTop;										///< Index of the top entry.

		static std::deque<CqShaderVariableUniformFloat*>				m_UFPool;
		// Integer
		static std::deque<CqShaderVariableUniformPoint*>				m_UPPool;
		static std::deque<CqShaderVariableUniformString*>			m_USPool;
		static std::deque<CqShaderVariableUniformColor*>				m_UCPool;
		// Triple
		// hPoint
		static std::deque<CqShaderVariableUniformNormal*>			m_UNPool;
		static std::deque<CqShaderVariableUniformVector*>			m_UVPool;
		// Void
		static std::deque<CqShaderVariableUniformMatrix*>			m_UMPool;
		// SixteenTuple

		static std::deque<CqShaderVariableVaryingFloat*>				m_VFPool;
		// Integer
		static std::deque<CqShaderVariableVaryingPoint*>				m_VPPool;
		static std::deque<CqShaderVariableVaryingString*>			m_VSPool;
		static std::deque<CqShaderVariableVaryingColor*>				m_VCPool;
		// Triple
		// hPoint
		static std::deque<CqShaderVariableVaryingNormal*>			m_VNPool;
		static std::deque<CqShaderVariableVaryingVector*>			m_VVPool;
		// Void
		static std::deque<CqShaderVariableVaryingMatrix*>			m_VMPool;
		// SixteenTuple

		static TqInt    m_samples; // by default == 18 see shaderstack.cpp
		static TqInt    m_maxsamples;
}
;




/* Templatised less than operator.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 */
OpABRS( < , LSS )
/* Templatised greater than operator.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 */
OpABRS( > , GRT )
/* Templatised less than or equal to operator.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 */
OpABRS( <= , LE )
/* Templatised greater than or equal to operator.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 */
OpABRS( >= , GE )
/* Templatised equality operator.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 */
OpABRS( == , EQ )
/* Templatised inequality operator.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 */
OpABRS( != , NE )
/* Templatised multiplication operator.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 */
OpABRS( *, MUL )
/** Special case vector multiplication operator.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 */
inline void	OpMULV( IqShaderData* pA, IqShaderData* pB, IqShaderData* pRes, CqBitVector& RunningState )
{
	CqVector3D vA, vB;
	CqVector3D* pdA;
	CqVector3D* pdB;
	TqInt i, ii;

	TqBool fAVar = pA->Size() > 1;
	TqBool fBVar = pB->Size() > 1;

	if ( fAVar && fBVar )
	{
		/* Both are varying, must go accross all processing each element. */
		pA->GetValuePtr( pdA );
		pB->GetValuePtr( pdB );
		ii = pA->Size();
		for ( i = 0; i < ii; i++ )
		{
			if ( RunningState.Value( i ) )
				pRes->SetValue( CqVector3D( pdA->x() * pdB->x(),
				                            pdA->y() * pdB->y(),
				                            pdA->z() * pdB->z() ), i );
			pdA++;
			pdB++;
		}
	}
	else if ( !fBVar && fAVar )
	{
		/* A is varying, can just get B's value once. */
		ii = pA->Size();
		pA->GetValuePtr( pdA );
		pB->GetValue( vB );
		for ( i = 0; i < ii; i++ )
		{
			if ( RunningState.Value( i ) )
				pRes->SetValue( CqVector3D( pdA->x() * vB.x(),
				                            pdA->y() * vB.y(),
				                            pdA->z() * vB.z() ), i );
			pdA++;
		}
	}
	else if ( !fAVar && fBVar )
		\
	{
		/* B is varying, can just get A's value once. */
		ii = pB->Size();
		pB->GetValuePtr( pdB );
		pA->GetValue( vA );
		for ( i = 0; i < ii; i++ )
		{
			if ( RunningState.Value( i ) )
				pRes->SetValue( CqVector3D( vA.x() * pdB->x(),
				                            vA.y() * pdB->y(),
				                            vA.z() * pdB->z() ), i );
			pdB++;
		}
	}
	else
	{
		/* Both are uniform, simple one shot case. */
		pA->GetValue( vA );
		pB->GetValue( vB );
		pRes->SetValue( CqVector3D( vA.x() * vB.x(),
		                            vA.y() * vB.y(),
		                            vA.z() * vB.z() ) );
	}
}
/* Templatised division operator.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 */
OpABRS( / , DIV )
/* Templatised addition operator.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 */
OpABRS( + , ADD )
/* Templatised subtraction operator.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 */
OpABRS( -, SUB )
/* Templatised dot operator.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 */
OpABRS( *, DOT )
/* Templatised cross product operator.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 */
OpABRS( % , CRS )
/* Templatised logical AND operator.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 */
OpABRS( && , LAND )
/* Templatised logical OR operator.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 */
OpABRS( || , LOR )

/* Templatised negation operator. The template classes decide the cast used, there must be an appropriate operator between the two types.
 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
 * \param pA The shader data to use as the second operand.
 * \param pRes The shader data to store the result in.
 * \param RunningState The current SIMD state.
 */
template <class A>
inline void	OpNEG( A& a, IqShaderData* pA, IqShaderData* pRes, CqBitVector& RunningState )
{
	A vA;
	A* pdA;
	TqInt i, ii;

	TqBool fAVar = pA->Size() > 1;

	if ( fAVar )
	{
		/* Varying, must go accross all processing each element. */
		pA->GetValuePtr( pdA );
		ii = pA->Size();
		for ( i = 0; i < ii; i++ )
		{
			if ( RunningState.Value( i ) )
				pRes->SetValue( -( *pdA ), i );
			pdA++;
		}
	}
	else
	{
		/* Uniform, simple one shot case. */
		pA->GetValue( vA );
		pRes->SetValue( -vA );
	}
}
/* Templatised cast operator, cast the current stack entry to the spcified type.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
 * \param pA The shader data to use as the second operand.
 * \param pRes The shader data to store the result in.
 * \param RunningState The current SIMD state.
 */
template <class A, class B>
inline void	OpCAST( A& a, B& b, IqShaderData* pA, IqShaderData* pRes, CqBitVector& RunningState )
{
	A vA;
	A* pdA;
	TqInt i, ii;

	TqBool fAVar = pA->Size() > 1;

	if ( fAVar )
	{
		/* Varying, must go accross all processing each element. */
		pA->GetValuePtr( pdA );
		ii = pA->Size();
		for ( i = 0; i < ii; i++ )
		{
			if ( RunningState.Value( i ) )
				pRes->SetValue( static_cast<B>( *pdA ), i );
			pdA++;
		}
	}
	else
	{
		/* Uniform, simple one shot case. */
		pA->GetValue( vA );
		pRes->SetValue( static_cast<B>( vA ) );
	}
}
/* Templatised cast three operands to a single triple type (vector/normal/color etc.) and store the result in this stack entry
 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
 * \param pA The shader data to use as the first triple element.
 * \param pB The shader data to use as the second triple element.
 * \param pC The shader data to use as the third triple element.
 * \param pRes The shader data to store the result in.
 * \param RunningState The current SIMD state.
 */
template <class A>
inline void	OpTRIPLE( A&a, IqShaderData* pRes, IqShaderData* pA, IqShaderData* pB, IqShaderData* pC, CqBitVector& RunningState )
{
	TqFloat x, y, z;

	TqInt i = MAX( MAX( pA->Size(), pB->Size() ), pC->Size() ) - 1;
	TqBool __fVarying = i > 0;
	for ( ; i >= 0; i-- )
		if ( !__fVarying || RunningState.Value( i ) )
		{
			pA->GetValue( x, i );
			pB->GetValue( y, i );
			pC->GetValue( z, i );
			pRes->SetValue( A( x, y, z ), i );
		}
}
/* Templatised cast sixteen operands to a single matrix type and store the result in this stack entry
 * \param z The type of the operand, used to determine templateisation, needed by VC++..
 * \param pRes The shader data to store the result in.
 * \param pA The shader data to use as the 0,0 element.
 * \param pB The shader data to use as the 1,0 element.
 * \param pC The shader data to use as the 2,0 element.
 * \param pD The shader data to use as the 3,0 element.
 * \param pE The shader data to use as the 0,1 element.
 * \param pF The shader data to use as the 1,1 element.
 * \param pG The shader data to use as the 2,1 element.
 * \param pH The shader data to use as the 3,1 element.
 * \param pI The shader data to use as the 0,2 element.
 * \param pJ The shader data to use as the 1,2 element.
 * \param pK The shader data to use as the 2,2 element.
 * \param pL The shader data to use as the 3,2 element.
 * \param pM The shader data to use as the 0,3 element.
 * \param pN The shader data to use as the 1,3 element.
 * \param pO The shader data to use as the 2,3 element.
 * \param pP The shader data to use as the 3,3 element.
 * \param RunningState The current SIMD state.
 */
template <class A>
inline void	OpHEXTUPLE( A& z, IqShaderData* pRes,
                        IqShaderData* pA, IqShaderData* pB, IqShaderData* pC, IqShaderData* pD,
                        IqShaderData* pE, IqShaderData* pF, IqShaderData* pG, IqShaderData* pH,
                        IqShaderData* pI, IqShaderData* pJ, IqShaderData* pK, IqShaderData* pL,
                        IqShaderData* pM, IqShaderData* pN, IqShaderData* pO, IqShaderData* pP, CqBitVector& RunningState )
{
	TqFloat a1, a2, a3, a4;
	TqFloat b1, b2, b3, b4;
	TqFloat c1, c2, c3, c4;
	TqFloat d1, d2, d3, d4;

	TqInt ii1 = MAX( MAX( MAX( pA->Size(), pB->Size() ), pC->Size() ), pD->Size() );
	TqInt ii2 = MAX( MAX( MAX( pE->Size(), pF->Size() ), pG->Size() ), pH->Size() );
	TqInt ii3 = MAX( MAX( MAX( pI->Size(), pJ->Size() ), pK->Size() ), pL->Size() );
	TqInt ii4 = MAX( MAX( MAX( pM->Size(), pN->Size() ), pO->Size() ), pP->Size() );
	TqInt ii = MAX( MAX( MAX( ii1, ii2 ), ii3 ), ii4 );
	TqBool __fVarying = ii > 0;
	for ( ; ii >= 0; ii-- )
	{
		if ( !__fVarying || RunningState.Value( ii ) )
		{
			pA->GetValue( a1, ii );
			pB->GetValue( a2, ii );
			pC->GetValue( a3, ii );
			pD->GetValue( a4, ii );
			pE->GetValue( b1, ii );
			pF->GetValue( b2, ii );
			pG->GetValue( b3, ii );
			pH->GetValue( b4, ii );
			pI->GetValue( c1, ii );
			pJ->GetValue( c2, ii );
			pK->GetValue( c3, ii );
			pL->GetValue( c4, ii );
			pM->GetValue( d1, ii );
			pN->GetValue( d2, ii );
			pO->GetValue( d3, ii );
			pP->GetValue( d4, ii );
			A tt( a1, a2, a3, a4,
			      b1, b2, b3, b4,
			      c1, c2, c3, c4,
			      d1, d2, d3, d4 );
			tt.SetfIdentity( TqFalse );
			pRes->SetValue( tt, ii );
		}
	}
}
/* Templatised component access operator.
 * \param z The type of the operand, used to determine templatisation, needed by VC++..
 * \param pA The shader data to extract the component from.
 * \param index The index of the component to extract.
 * \param pRes The shader data to store the result in.
 * \param RunningState The current SIMD state.
 */
template <class A>
inline void	OpCOMP( A& z, IqShaderData* pA, int index, IqShaderData* pRes, CqBitVector& RunningState )
{
	A vA;
	A* pdA;
	TqInt i, ii;

	TqBool fAVar = pA->Size() > 1;

	if ( fAVar )
	{
		/* Varying, must go accross all processing each element. */
		pA->GetValuePtr( pdA );
		ii = pA->Size();
		for ( i = 0; i < ii; i++ )
		{
			if ( RunningState.Value( i ) )
				pRes->SetValue( ( *pdA ) [ index ], i );
			pdA++;
		}
	}
	else
	{
		/* Uniform, simple one shot case. */
		pA->GetValue( vA );
		pRes->SetValue( vA[ index ] );
	}
}
/* Templatised component access operator.
 * \param z The type of the operand, used to determine templatisation, needed by VC++..
 * \param pA The shader data to extract the component from.
 * \param pB The shader data to use to get the index to extract.
 * \param pRes The shader data to store the result in.
 * \param RunningState The current SIMD state.
 */
template <class A>
inline void	OpCOMP( A& z, IqShaderData* pA, IqShaderData* pB, IqShaderData* pRes, CqBitVector& RunningState )
{
	A vA;
	TqFloat vB;
	A* pdA;
	TqFloat* pdB;
	TqInt i, ii;

	TqBool fAVar = pA->Size() > 1;
	TqBool fBVar = pB->Size() > 1;

	if ( fAVar && fBVar )
	{
		/* Both are varying, must go accross all processing each element. */
		pA->GetValuePtr( pdA );
		pB->GetValuePtr( pdB );
		ii = pA->Size();
		for ( i = 0; i < ii; i++ )
		{
			if ( RunningState.Value( i ) )
				pRes->SetValue( ( *pdA ) [ static_cast<TqInt>( *pdB ) ], i );
			pdA++;
			pdB++;
		}
	}
	else if ( !fBVar && fAVar )
	{
		/* A is varying, can just get B's value once. */
		ii = pA->Size();
		pA->GetValuePtr( pdA );
		pB->GetValue( vB );
		TqInt index = static_cast<TqInt>( vB );
		for ( i = 0; i < ii; i++ )
		{
			if ( RunningState.Value( i ) )
				pRes->SetValue( ( *pdA ) [ index ], i );
			pdA++;
		}
	}
	else if ( !fAVar && fBVar )
		\
	{
		/* B is varying, can just get A's value once. */
		ii = pB->Size();
		pB->GetValuePtr( pdB );
		pA->GetValue( vA );
		for ( i = 0; i < ii; i++ )
		{
			if ( RunningState.Value( i ) )
				pRes->SetValue( vA[ static_cast<TqInt>( *pdB ) ], i );
			pdB++;
		}
	}
	else
	{
		/* Both are uniform, simple one shot case. */
		pA->GetValue( vA );
		pB->GetValue( vB );
		TqInt index = static_cast<TqInt>( vB );
		pRes->SetValue( vA[ index ] );
	}
}
/* Templatised component set operator.
 * \param z The type of the operand, used to determine templatisation, needed by VC++..
 * \param pRes The shader data to store the result in.
 * \param index The index of the component to set.
 * \param pA The shader data to set the component within.
 * \param RunningState The current SIMD state.
 */
template <class A>
inline void	OpSETCOMP( A& z, IqShaderData* pRes, int index, IqShaderData* pA, CqBitVector& RunningState )
{
	A vA;
	TqFloat val;

	TqInt i = MAX( pRes->Size(), pA->Size() ) - 1;
	TqBool __fVarying = i > 0;
	for ( ; i >= 0; i-- )
	{
		if ( !__fVarying || RunningState.Value( i ) )
		{
			GetValue( vA, i );
			pA->GetValue( val, i );
			vA [ index ] = val;
			pRes->SetValue( vA, i );
		}
	}
}
/* Templatised component set operator.
 * \param z The type of the operand, used to determine templatisation, needed by VC++..
 * \param pRes The shader data to store the result in.
 * \param index The shader data to get the index of the component to set from.
 * \param pA The shader data to set the component within.
 * \param RunningState The current SIMD state.
 */
template <class A>
inline void	OpSETCOMP( A& z, IqShaderData* pRes, IqShaderData* index, IqShaderData* pA, CqBitVector& RunningState )
{
	A vA;
	TqFloat val, fi;

	TqInt i = MAX( MAX( pRes->Size(), pA->Size() ), index->Size() ) - 1;
	TqBool __fVarying = i > 0;
	for ( ; i >= 0; i-- )
	{
		if ( !__fVarying || RunningState.Value( i ) )
		{
			pRes->GetValue( vA, i );
			pA->GetValue( val, i );
			index->GetValue( fi, i );
			vA [ static_cast<TqInt>( fi ) ] = val;
			pRes->SetValue( vA, i );
		}
	}
}

/* Special case matrix component access.
 * \param pA The shader data that stores the matrix.
 * \param pR The shader data to get the index into the rows from.
 * \param pC The shader data to get the index into the columns from.
 * \param pRes The shader data to store the result in.
 * \param RunningState The current SIMD state.
 */
inline void	OpCOMPM( IqShaderData* pA, IqShaderData* pR, IqShaderData* pC, IqShaderData* pRes, CqBitVector& RunningState )
{
	CqMatrix m;
	TqFloat fr, fc;

	TqInt i = MAX( pA->Size(), pRes->Size() ) - 1;
	TqBool __fVarying = i > 0;
	for ( ; i >= 0; i-- )
		if ( !__fVarying || RunningState.Value( i ) )
		{
			pA->GetValue( m, i );
			pR->GetValue( fr, i );
			pC->GetValue( fc, i );
			pRes->SetValue( m [ static_cast<TqInt>( fr ) ][ static_cast<TqInt>( fc ) ], i );
		}
}

/* Special case matrix component access.
 * \param pA The shader data that stores the matrix.
 * \param pR The shader data to get the index into the rows from.
 * \param pC The shader data to get the index into the columns from.
 * \param pV The shader data which holds the value to place in the appropriate row/column.
 * \param RunningState The current SIMD state.
 */
inline void	OpSETCOMPM( IqShaderData* pA, IqShaderData* pR, IqShaderData* pC, IqShaderData* pV, CqBitVector& RunningState )
{
	CqMatrix m;
	TqFloat fr, fc, val;

	TqInt i = MAX( pA->Size(), pV->Size() ) - 1;
	TqBool __fVarying = i > 0;
	for ( ; i >= 0; i-- )
	{
		if ( !__fVarying || RunningState.Value( i ) )
		{
			pA->GetValue( m, i );
			pR->GetValue( fr, i );
			pC->GetValue( fc, i );
			pV->GetValue( val, i );
			m[ static_cast<TqInt>( fr ) ][ static_cast<TqInt>( fc ) ] = val;
			pA->SetValue( m, i );
		}
	}
}


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !SHADERSTACK_H_INCLUDED
