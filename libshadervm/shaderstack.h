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

#define	OpLSS_FF(a,b,Res,State)		OpLSS(temp_float,temp_float,a,b,Res,State)
#define	OpLSS_PP(a,b,Res,State)		OpLSS(temp_point,temp_point,a,b,Res,State)
#define	OpLSS_CC(a,b,Res,State)		OpLSS(temp_color,temp_color,a,b,Res,State)

#define	OpGRT_FF(a,b,Res,State)		OpGRT(temp_float,temp_float,a,b,Res,State)
#define	OpGRT_PP(a,b,Res,State)		OpGRT(temp_point,temp_point,a,b,Res,State)
#define	OpGRT_CC(a,b,Res,State)		OpGRT(temp_color,temp_color,a,b,Res,State)

#define	OpLE_FF(a,b,Res,State)		OpLE(temp_float,temp_float,a,b,Res,State)
#define	OpLE_PP(a,b,Res,State)		OpLE(temp_point,temp_point,a,b,Res,State)
#define	OpLE_CC(a,b,Res,State)		OpLE(temp_color,temp_color,a,b,Res,State)

#define	OpGE_FF(a,b,Res,State)		OpGE(temp_float,temp_float,a,b,Res,State)
#define	OpGE_PP(a,b,Res,State)		OpGE(temp_point,temp_point,a,b,Res,State)
#define	OpGE_CC(a,b,Res,State)		OpGE(temp_color,temp_color,a,b,Res,State)

#define	OpEQ_FF(a,b,Res,State)		OpEQ(temp_float,temp_float,a,b,Res,State)
#define	OpEQ_PP(a,b,Res,State)		OpEQ(temp_point,temp_point,a,b,Res,State)
#define	OpEQ_CC(a,b,Res,State)		OpEQ(temp_color,temp_color,a,b,Res,State)
#define	OpEQ_SS(a,b,Res,State)		OpEQ(temp_string,temp_string,a,b,Res,State)

#define	OpNE_FF(a,b,Res,State)		OpNE(temp_float,temp_float,a,b,Res,State)
#define	OpNE_PP(a,b,Res,State)		OpNE(temp_point,temp_point,a,b,Res,State)
#define	OpNE_CC(a,b,Res,State)		OpNE(temp_color,temp_color,a,b,Res,State)
#define	OpNE_SS(a,b,Res,State)		OpNE(temp_string,temp_string,a,b,Res,State)

#define	OpMUL_FF(a,b,Res,State)		OpMUL(temp_float,temp_float,a,b,Res,State)
#define	OpDIV_FF(a,b,Res,State)		OpDIV(temp_float,temp_float,a,b,Res,State)
#define	OpADD_FF(a,b,Res,State)		OpADD(temp_float,temp_float,a,b,Res,State)
#define	OpSUB_FF(a,b,Res,State)		OpSUB(temp_float,temp_float,a,b,Res,State)
#define	OpNEG_F(a,Res,State)		OpNEG(temp_float,a,Res,State)

#define	OpMUL_PP(a,b,Res,State)		OpMUL(temp_point,temp_point,a,b,Res,State)
#define	OpDIV_PP(a,b,Res,State)		OpDIV(temp_point,temp_point,a,b,Res,State)
#define	OpADD_PP(a,b,Res,State)		OpADD(temp_point,temp_point,a,b,Res,State)
#define	OpSUB_PP(a,b,Res,State)		OpSUB(temp_point,temp_point,a,b,Res,State)
#define	OpCRS_PP(a,b,Res,State)		OpCRS(temp_point,temp_point,a,b,Res,State)
#define	OpDOT_PP(a,b,Res,State)		OpDOT(temp_point,temp_point,a,b,Res,State)
#define	OpNEG_P(a,Res,State)		OpNEG(temp_point,a,Res,State)

#define	OpMUL_CC(a,b,Res,State)		OpMUL(temp_color,temp_color,a,b,Res,State)
#define	OpDIV_CC(a,b,Res,State)		OpDIV(temp_color,temp_color,a,b,Res,State)
#define	OpADD_CC(a,b,Res,State)		OpADD(temp_color,temp_color,a,b,Res,State)
#define	OpSUB_CC(a,b,Res,State)		OpSUB(temp_color,temp_color,a,b,Res,State)
#define	OpCRS_CC(a,b,Res,State)		OpCRS(temp_color,temp_color,a,b,Res,State)
#define	OpDOT_CC(a,b,Res,State)		OpDOT(temp_color,temp_color,a,b,Res,State)
#define	OpNEG_C(a,Res,State)		OpNEG(temp_color,a,Res,State)

#define	OpMUL_FP(a,b,Res,State)		OpMUL(temp_float,temp_point,a,b,Res,State)
#define	OpDIV_FP(a,b,Res,State)		OpDIV(temp_float,temp_point,a,b,Res,State)
#define	OpADD_FP(a,b,Res,State)		OpADD(temp_float,temp_point,a,b,Res,State)
#define	OpSUB_FP(a,b,Res,State)		OpSUB(temp_float,temp_point,a,b,Res,State)

#define	OpMUL_FC(a,b,Res,State)		OpMUL(temp_float,temp_color,a,b,Res,State)
#define	OpDIV_FC(a,b,Res,State)		OpDIV(temp_float,temp_color,a,b,Res,State)
#define	OpADD_FC(a,b,Res,State)		OpADD(temp_float,temp_color,a,b,Res,State)
#define	OpSUB_FC(a,b,Res,State)		OpSUB(temp_float,temp_color,a,b,Res,State)

#define	OpLAND_B(a,b,Res,State)		OpLAND(temp_float,temp_float,a,b,Res,State)
#define	OpLOR_B(a,b,Res,State)		OpLOR(temp_float,temp_float,a,b,Res,State)

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

#define OpABRS(OP, NAME) \
		template <class A, class B>	\
		inline void	Op##NAME( A& a, B&b, IqShaderData* pA, IqShaderData* pB, IqShaderData* pRes, CqBitVector& RunningState ) \
		{ \
			A vA; \
			B vB; \
			A* pdA; \
			B* pdB; \
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
				ii = pA->Size(); \
				for ( i = 0; i < ii; i++ ) \
				{ \
					if ( RunningState.Value( i ) ) \
						pRes->SetValue( (*pdA) OP (*pdB), i ); \
					pdA++; \
					pdB++; \
				} \
			} \
			else if( !fBVar && fAVar) \
			{ \
				/* A is varying, can just get B's value once. */ \
				ii = pA->Size(); \
				pA->GetValuePtr( pdA ); \
				pB->GetValue( vB ); \
				for ( i = 0; i < ii; i++ ) \
				{ \
					if ( RunningState.Value( i ) ) \
						pRes->SetValue( (*pdA) OP vB, i ); \
					pdA++; \
				} \
			} \
			else if( !fAVar && fBVar) \
			{ \
				/* B is varying, can just get A's value once. */ \
				ii = pB->Size(); \
				pB->GetValuePtr( pdB ); \
				pA->GetValue( vA ); \
				for ( i = 0; i < ii; i++ ) \
				{ \
					if ( RunningState.Value( i ) ) \
						pRes->SetValue( vA OP (*pdB), i ); \
					pdB++; \
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

class _qShareC CqShaderStack
{
	public:
		_qShareM CqShaderStack() : m_iTop( 0 )
		{
			m_Stack.resize( 48 );

			m_aUFPool.resize( 48 );
			m_aUPPool.resize( 48 );
			m_aUSPool.resize( 48 );
			m_aUCPool.resize( 48 );
			m_aUNPool.resize( 48 );
			m_aUVPool.resize( 48 );
			m_aUMPool.resize( 48 );

			m_aVFPool.resize( 48 );
			m_aVPPool.resize( 48 );
			m_aVSPool.resize( 48 );
			m_aVCPool.resize( 48 );
			m_aVNPool.resize( 48 );
			m_aVVPool.resize( 48 );
			m_aVMPool.resize( 48 );

			TqInt i;
			for( i = 0; i < type_last; i++ )
				m_iUPoolTops[ i ] = 0;
			for( i = 0; i < type_last; i++ )
				m_iVPoolTops[ i ] = 0;
		}
		virtual _qShareM ~CqShaderStack()
		{
			m_Stack.clear();

			m_aUFPool.clear();
			m_aUPPool.clear();
			m_aUSPool.clear();
			m_aUCPool.clear();
			m_aUNPool.clear();
			m_aUVPool.clear();
			m_aUMPool.clear();

			m_aVFPool.clear();
			m_aVPPool.clear();
			m_aVSPool.clear();
			m_aVCPool.clear();
			m_aVNPool.clear();
			m_aVVPool.clear();
			m_aVMPool.clear();
		}


		IqShaderData* GetNextTemp(EqVariableType type, EqVariableClass _class)
		{
			switch( type )
			{
				case type_point:
					if( _class == class_uniform)
					{
						if ( m_iUPoolTops[type_point] >= m_aUPPool.size() )
							m_aUPPool.resize( m_aUPPool.size() + 1 );
						return ( &m_aUPPool[ m_iUPoolTops[ type_point ] ] );
					}
					else
					{
						if ( m_iVPoolTops[type_point] >= m_aVPPool.size() )
							m_aVPPool.resize( m_aVPPool.size() + 1 );
						return ( &m_aVPPool[ m_iVPoolTops[ type_point ] ] );
					}

				case type_string:
					if( _class == class_uniform)
					{
						if ( m_iUPoolTops[type_string] >= m_aUSPool.size() )
							m_aUSPool.resize( m_aUSPool.size() + 1 );
						return ( &m_aUSPool[ m_iUPoolTops[ type_string ] ] );
					}
					else
					{
						if ( m_iVPoolTops[type_string] >= m_aVSPool.size() )
							m_aVSPool.resize( m_aVSPool.size() + 1 );
						return ( &m_aVSPool[ m_iVPoolTops[ type_string ] ] );
					}

				case type_color:
					if( _class == class_uniform)
					{
						if ( m_iUPoolTops[type_color] >= m_aUCPool.size() )
							m_aUCPool.resize( m_aUCPool.size() + 1 );
						return ( &m_aUCPool[ m_iUPoolTops[ type_color ] ] );
					}
					else
					{
						if ( m_iVPoolTops[type_color] >= m_aVCPool.size() )
							m_aVCPool.resize( m_aVCPool.size() + 1 );
						return ( &m_aVCPool[ m_iVPoolTops[ type_color ] ] );
					}

				case type_normal:
					if( _class == class_uniform)
					{
						if ( m_iUPoolTops[type_normal] >= m_aUNPool.size() )
							m_aUNPool.resize( m_aUNPool.size() + 1 );
						return ( &m_aUNPool[ m_iUPoolTops[ type_normal ] ] );
					}
					else
					{
						if ( m_iVPoolTops[type_normal] >= m_aVNPool.size() )
							m_aVNPool.resize( m_aVNPool.size() + 1 );
						return ( &m_aVNPool[ m_iVPoolTops[ type_normal ] ] );
					}

				case type_vector:
					if( _class == class_uniform)
					{
						if ( m_iUPoolTops[type_vector] >= m_aUVPool.size() )
							m_aUVPool.resize( m_aUVPool.size() + 1 );
						return ( &m_aUVPool[ m_iUPoolTops[ type_vector ] ] );
					}
					else
					{
						if ( m_iVPoolTops[type_vector] >= m_aVVPool.size() )
							m_aVVPool.resize( m_aVVPool.size() + 1 );
						return ( &m_aVVPool[ m_iVPoolTops[ type_vector ] ] );
					}

				case type_matrix:
					if( _class == class_uniform)
					{
						if ( m_iUPoolTops[type_matrix] >= m_aUMPool.size() )
							m_aUMPool.resize( m_aUMPool.size() + 1 );
						return ( &m_aUMPool[ m_iUPoolTops[ type_matrix ] ] );
					}
					else
					{
						if ( m_iVPoolTops[type_matrix] >= m_aVMPool.size() )
							m_aVMPool.resize( m_aVMPool.size() + 1 );
						return ( &m_aVMPool[ m_iVPoolTops[ type_matrix ] ] );
					}

				default:
					if( _class == class_uniform)
					{
						if ( m_iUPoolTops[type_float] >= m_aUFPool.size() )
							m_aUFPool.resize( m_aUFPool.size() + 1 );
						return ( &m_aUFPool[ m_iUPoolTops[ type_float ] ] );
					}
					else
					{
						if ( m_iVPoolTops[type_float] >= m_aVFPool.size() )
							m_aVFPool.resize( m_aVFPool.size() + 1 );
						return ( &m_aVFPool[ m_iVPoolTops[ type_float ] ] );
					}
			}
		}


		/** Push a new shader variable reference onto the stack.
		 */
		void	Push( IqShaderData* pv )
		{
			if ( m_iTop >= m_Stack.size() )
				m_Stack.resize( m_Stack.size() + 1 );

			m_Stack[ m_iTop++ ] = pv;
			if( pv->Class() == class_uniform)
				m_iUPoolTops[ pv->Type() ]++;
			else
				m_iVPoolTops[ pv->Type() ]++;
		}
		/** Pop the top stack entry.
		 * \param f Boolean value to update if this is varying. If not varying, leaves f unaltered.
		 * \return Reference to the top stack entry.
		 */
		IqShaderData* Pop( TqBool& f )
		{
			if ( m_iTop ) m_iTop--;
			IqShaderData* pVal = m_Stack[ m_iTop ];
			
			f = pVal->Size() > 1 || f;

			if( pVal->Class() == class_uniform)
			{
				m_iUPoolTops[ pVal->Type() ]--;
				assert( m_iUPoolTops[ pVal->Type() ] >= 0 );
			}
			else
			{
				m_iVPoolTops[ pVal->Type() ]--;
				assert( m_iVPoolTops[ pVal->Type() ] >= 0 );
			}

			return ( pVal );
		}
		/** Duplicate the top stack entry.
		 */
		void	Dup()
		{
			TqInt iTop = m_iTop;
			
			IqShaderData* s = GetNextTemp(m_Stack[ iTop ]->Type(), m_Stack[ iTop ]->Class());
			s->SetValueFromVariable(m_Stack[ iTop ]);
			Push( s );
		}
		/** Drop the top stack entry.
		 */
		void	Drop()
		{
			TqBool f = TqFalse;
			Pop(f);
		}
	protected:
		std::vector<IqShaderData*>		m_Stack;
		TqUint	m_iTop;										///< Index of the top entry.


		std::vector<CqShaderVariableUniformFloat>	m_aUFPool;
		// Integer
		std::vector<CqShaderVariableUniformPoint>	m_aUPPool;
		std::vector<CqShaderVariableUniformString>	m_aUSPool;
		std::vector<CqShaderVariableUniformColor>	m_aUCPool;
		// Triple
		// hPoint
		std::vector<CqShaderVariableUniformNormal>	m_aUNPool;
		std::vector<CqShaderVariableUniformVector>	m_aUVPool;
		// Void
		std::vector<CqShaderVariableUniformMatrix>	m_aUMPool;
		// SixteenTuple

		std::vector<CqShaderVariableVaryingFloat>	m_aVFPool;
		// Integer
		std::vector<CqShaderVariableVaryingPoint>	m_aVPPool;
		std::vector<CqShaderVariableVaryingString>	m_aVSPool;
		std::vector<CqShaderVariableVaryingColor>	m_aVCPool;
		// Triple
		// hPoint
		std::vector<CqShaderVariableVaryingNormal>	m_aVNPool;
		std::vector<CqShaderVariableVaryingVector>	m_aVVPool;
		// Void
		std::vector<CqShaderVariableVaryingMatrix>	m_aVMPool;
		// SixteenTuple

		TqInt	m_iUPoolTops[type_last];
		TqInt	m_iVPoolTops[type_last];
}
;




/** Templatised less than operator.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
 * \param Comp The stack entry to use as the second operand.
 * \param Res The stack entry to store the result in.
 * \param RunningState The current SIMD state.
 */
OpABRS(<,LSS)
/** Templatised greater than operator.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
 * \param Comp The stack entry to use as the second operand.
 * \param Res The stack entry to store the result in.
 * \param RunningState The current SIMD state.
 */
OpABRS(>,GRT)
/** Templatised less than or equal to operator.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
 * \param Comp The stack entry to use as the second operand.
 * \param Res The stack entry to store the result in.
 * \param RunningState The current SIMD state.
 */
OpABRS(<=,LE)
/** Templatised greater than or equal to operator.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
 * \param Comp The stack entry to use as the second operand.
 * \param Res The stack entry to store the result in.
 * \param RunningState The current SIMD state.
 */
OpABRS(>=,GE)
/** Templatised equality operator.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
 * \param Comp The stack entry to use as the second operand.
 * \param Res The stack entry to store the result in.
 * \param RunningState The current SIMD state.
 */
OpABRS(==,EQ)
/** Templatised inequality operator.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
 * \param Comp The stack entry to use as the second operand.
 * \param Res The stack entry to store the result in.
 * \param RunningState The current SIMD state.
 */
OpABRS(!=,NE)
/** Templatised multiplication operator.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
 * \param Comp The stack entry to use as the second operand.
 * \param Res The stack entry to store the result in.
 * \param RunningState The current SIMD state.
 */
OpABRS(*,MUL)
/** Special case vector multiplication operator.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 * \param Comp The stack entry to use as the second operand.
 * \param Res The stack entry to store the result in.
 * \param RunningState The current SIMD state.
 */
inline void	OpMULV( IqShaderData* pA, IqShaderData* pB, IqShaderData* pRes, CqBitVector& RunningState )
{
	CqVector3D	vecA, vecB;

	TqInt i = MAX( MAX( pA->Size(), pB->Size() ), pRes->Size() ) - 1;
	TqBool __fVarying = i > 0;
	for ( ; i >= 0; i-- )
	{
		if ( !__fVarying || RunningState.Value( i ) )
		{
			pA->GetValue( vecA, i);
			pB->GetValue( vecB, i);
			pRes->SetValue( CqVector3D( vecA.x() * vecB.x(),
					                  vecA.y() * vecB.y(),
					                  vecA.z() * vecB.z() ), i );
		}
	}
}
/** Templatised division operator.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
 * \param Comp The stack entry to use as the second operand.
 * \param Res The stack entry to store the result in.
 * \param RunningState The current SIMD state.
 */
OpABRS(/,DIV)
/** Templatised addition operator.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
 * \param Comp The stack entry to use as the second operand.
 * \param Res The stack entry to store the result in.
 * \param RunningState The current SIMD state.
 */
OpABRS(+,ADD)
/** Templatised subtraction operator.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
 * \param a The type of the second operand, used to determine templateisation, needed by VC++..
 * \param Comp The stack entry to use as the second operand.
 * \param Res The stack entry to store the result in.
 * \param RunningState The current SIMD state.
 */
OpABRS(-,SUB)
/** Templatised dot operator.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
 * \param Comp The stack entry to use as the second operand.
 * \param Res The stack entry to store the result in.
 * \param RunningState The current SIMD state.
 * \attention Should only ever be called with vector based operands.
 */
OpABRS(*,DOT)
/** Templatised cross product operator.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
 * \param Comp The stack entry to use as the second operand.
 * \param Res The stack entry to store the result in.
 * \param RunningState The current SIMD state.
 * \attention Should only ever be called with vector based operands.
 */
OpABRS(%,CRS)
/** Templatised logical AND operator.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
 * \param Comp The stack entry to use as the second operand.
 * \param Res The stack entry to store the result in.
 * \param RunningState The current SIMD state.
 */
OpABRS(&&,LAND)
/** Templatised logical OR operator.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
 * \param Comp The stack entry to use as the second operand.
 * \param Res The stack entry to store the result in.
 * \param RunningState The current SIMD state.
 */
OpABRS(||,LOR)
/** Templatised negation operator.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
 * \param Comp The stack entry to use as the second operand.
 * \param Res The stack entry to store the result in.
 * \param RunningState The current SIMD state.
 */
template <class A>
inline void	OpNEG( A& a, IqShaderData* pA, IqShaderData* pRes, CqBitVector& RunningState )
{
	A vA;

	TqInt i = MAX( pA->Size(), pRes->Size() ) - 1;
	TqBool __fVarying = i > 0;
	for ( ; i >= 0; i-- )
		if ( !__fVarying || RunningState.Value( i ) )
		{
			pA->GetValue( vA, i );
			pRes->SetValue( -vA, i );
		}
}
/** Templatised cast operator, cast the current stack entry to the spcified type.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
 * \param Comp The stack entry to use as the second operand.
 * \param Res The stack entry to store the result in.
 * \param RunningState The current SIMD state.
 */
template <class A, class B>
inline void	OpCAST( A& a, B& b, IqShaderData* pA, IqShaderData* pRes, CqBitVector& RunningState )
{
	A vA;

	TqInt i = MAX( pA->Size(), pRes->Size() ) - 1;
	TqBool __fVarying = i > 0;
	for ( ; i >= 0; i-- )
		if ( !__fVarying || RunningState.Value( i ) )
		{
			pA->GetValue( vA, i );
			pRes->SetValue( static_cast<B>( vA ), i );
		}
}
/** Templatised cast three operands to a single triple type (vector/normal/color etc.) and store the result in this stack entry
 * \param z The type to combine the float values into.
 * \param a Float first operand 
 * \param b Float second operand 
 * \param c Float third operand 
 * \param RunningState The current SIMD state.
 */
template <class A>
inline void	OpTRIPLE( A&, IqShaderData* pRes, IqShaderData* pA, IqShaderData* pB, IqShaderData* pC, CqBitVector& RunningState )
{
	TqFloat x,y,z;

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
/** Templatised cast sixteen operands to a single matrix type and store the result in this stack entry
 * The parameters a-p are the float values to combine.
 * \param z The type to combine the float values into (currenlty only matrix supported).
 * \param RunningState The current SIMD state.
 */
template <class A>
inline void	OpHEXTUPLE( A& z, IqShaderData* pRes,
						  IqShaderData* pA, IqShaderData* pB, IqShaderData* pC, IqShaderData* pD,
						  IqShaderData* pE, IqShaderData* pF, IqShaderData* pG, IqShaderData* pH,
						  IqShaderData* pI, IqShaderData* pJ, IqShaderData* pK, IqShaderData* pL,
						  IqShaderData* pM, IqShaderData* pN, IqShaderData* pO, IqShaderData* pP, CqBitVector& RunningState )
{
	TqFloat a1,a2,a3,a4;
	TqFloat b1,b2,b3,b4;
	TqFloat c1,c2,c3,c4;
	TqFloat d1,d2,d3,d4;
	
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
			pA->GetValue( a1, ii );	pB->GetValue( a2, ii );	pC->GetValue( a3, ii );	pD->GetValue( a4, ii );
			pE->GetValue( b1, ii );	pF->GetValue( b2, ii );	pG->GetValue( b3, ii );	pH->GetValue( b4, ii );
			pI->GetValue( c1, ii );	pJ->GetValue( c2, ii );	pK->GetValue( c3, ii );	pL->GetValue( c4, ii );
			pM->GetValue( d1, ii );	pN->GetValue( d2, ii );	pO->GetValue( d3, ii );	pP->GetValue( d4, ii );
			A tt( a1, a2, a3, a4,
				  b1, b2, b3, b4,
				  c1, c2, c3, c4,
				  d1, d2, d3, d4);
			tt.SetfIdentity( TqFalse );
			pRes->SetValue( tt, ii );
		}
	}
}
/** Templatised component access operator.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 * \param z The type to cast this stackentry to.
 * \param index Integer index. 
 * \param Res The stack entry to store the result in.
 * \param RunningState The current SIMD state.
 */
template <class A>
inline void	OpCOMP( A& z, IqShaderData* pA, int index, IqShaderData* pRes, CqBitVector& RunningState )
{
	A vA;
	
	TqInt i = MAX( pA->Size(), pRes->Size() ) - 1;
	TqBool __fVarying = i > 0;
	for ( ; i >= 0; i-- )
	{
		if ( !__fVarying || RunningState.Value( i ) )
		{
			pA->GetValue( vA, i );
			pRes->SetValue( vA [ index ], i );
		}
	}
}
/** Templatised component access operator.
 * The template classes decide the cast used, there must be an appropriate operator between the two types.
 * \param z The type to cast this stackentry to.
 * \param index Integer type stackentry index. 
 * \param Res The stack entry to store the result in.
 * \param RunningState The current SIMD state.
 */
template <class A>
inline void	OpCOMP( A& z, IqShaderData* pA, IqShaderData* index, IqShaderData* pRes, CqBitVector& RunningState )
{
	A vA;
	TqFloat fi;

	TqInt i = MAX( MAX( pA->Size(), pRes->Size() ), index->Size() ) - 1;
	TqBool __fVarying = i > 0;
	for ( ; i >= 0; i-- )
	{
		if ( !__fVarying || RunningState.Value( i ) )
		{
			pA->GetValue( vA, i );
			index->GetValue( fi, i );
			pRes->SetValue( vA [ static_cast<TqInt>( fi ) ], i );
		}
	}
}
/** Templatised component set operator.
 * \param z The type to cast this to.
 * \param index Integer index. 
 * \param a Float type stackentry to set the index to.
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
/** Templatised component set operator.
 * \param z The type to cast this to.
 * \param index Integer type stackentry index. 
 * \param a Float type stackentry to set the index to.
 * \param RunningState The current SIMD state.
 */
template <class A>
inline void	OpSETCOMP( A& z, IqShaderData* pRes, IqShaderData* index, IqShaderData* pA, CqBitVector& RunningState )
{
	A vA;
	TqFloat val, fi;

	TqInt i = MAX( MAX( Size(), pA->Size() ), index->Size() ) - 1;
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

/** Special case matrix component access.
 * \param r Integer type stackentry row index.
 * \param c Integer type stackentry column index.
 * \param Res The stack entry to store the result in.
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

/** Special case matrix component access.
 * \param r Integer type stackentry row index.
 * \param c Integer type stackentry column index.
 * \param v Float type stackentry value to set index to.
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
