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

#define	OpLAND_B(a,b,Res,State)		OpLAND(temp_bool,temp_bool,a,b,Res,State)
#define	OpLOR_B(a,b,Res,State)		OpLOR(temp_bool,temp_bool,a,b,Res,State)

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
		inline void	Op##NAME( A& a, B&b, CqVMStackEntry* pA, CqVMStackEntry* pB, CqVMStackEntry& Res, CqBitVector& RunningState ) \
		{ \
			A vA; \
			B vB; \
			A* pdA; \
			B* pdB; \
			TqInt i, ii; \
			\
			TqBool fAVar = pA->Size() > 1; \
			TqBool fBVar = pB->Size() > 1; \
			TqBool fADat = !pA->fIsRef(); \
			TqBool fBDat = !pB->fIsRef(); \
			\
			if( !fADat ) \
			{ \
				CqVMStackEntryVarRef* pVRA = static_cast<CqVMStackEntryVarRef*>(pA); \
				if( !fBDat ) \
				{ \
					CqVMStackEntryVarRef* pVRB = static_cast<CqVMStackEntryVarRef*>(pB); \
					/* Both A and B are variables. */  \
					if( fAVar && fBVar )\
					{ \
						/* Both are varying, must go accross all processing each element. */ \
						pVRA->m_pVarRef->GetValuePtr( pdA ); \
						pVRB->m_pVarRef->GetValuePtr( pdB ); \
						ii = pA->Size(); \
						for ( i = 0; i < ii; i++ ) \
						{ \
							if ( RunningState.Value( i ) ) \
								Res.SetValue( (*pdA) OP (*pdB), i ); \
							pdA++; \
							pdB++; \
						} \
					} \
					else if( !fBVar && fAVar) \
					{ \
						/* A is varying, can just get B's value once. */ \
						ii = pA->Size(); \
						pVRA->m_pVarRef->GetValuePtr( pdA ); \
						pVRB->m_pVarRef->GetValue( vB ); \
						for ( i = 0; i < ii; i++ ) \
						{ \
							if ( RunningState.Value( i ) ) \
								Res.SetValue( (*pdA) OP vB, i ); \
							pdA++; \
						} \
					} \
					else if( !fAVar && fBVar) \
					{ \
						/* B is varying, can just get A's value once. */ \
						ii = pB->Size(); \
						pVRB->m_pVarRef->GetValuePtr( pdB ); \
						pVRA->m_pVarRef->GetValue( vA ); \
						for ( i = 0; i < ii; i++ ) \
						{ \
							if ( RunningState.Value( i ) ) \
								Res.SetValue( vA OP (*pdB), i ); \
							pdB++; \
						} \
					} \
					else \
					{ \
						/* Both are uniform, simple one shot case. */ \
						pVRA->m_pVarRef->GetValue( vA ); \
						pVRB->m_pVarRef->GetValue( vB ); \
						Res.SetValue( vA OP vB ); \
					} \
				} \
				else \
				{ \
					/* A is a variable, B is data. */  \
					if( fAVar && fBVar )\
					{ \
						/* Both are varying, must go accross all processing each element. */ \
						pVRA->m_pVarRef->GetValuePtr( pdA ); \
						ii = pA->Size(); \
						for ( i = 0; i < ii; i++ ) \
						{ \
							if ( RunningState.Value( i ) ) \
							{ \
								pB->m_aValues[ i ].GetValue( vB ); \
								Res.SetValue( (*pdA) OP vB, i ); \
							} \
							pdA++; \
						} \
					} \
					else if( !fBVar && fAVar) \
					{ \
						/* A is varying, can just get B's value once. */ \
						ii = pA->Size(); \
						pVRA->m_pVarRef->GetValuePtr( pdA ); \
						pB->m_Value.GetValue( vB ); \
						for ( i = 0; i < ii; i++ ) \
						{ \
							if ( RunningState.Value( i ) ) \
								Res.SetValue( (*pdA) OP vB, i ); \
							pdA++; \
						} \
					} \
					else if( !fAVar && fBVar) \
					{ \
						/* B is varying, can just get A's value once. */ \
						ii = pB->Size(); \
						pA->GetValue( vA ); \
						for ( i = 0; i < ii; i++ ) \
						{ \
							if ( RunningState.Value( i ) ) \
							{ \
								pB->m_aValues[ i ].GetValue( vB ); \
								Res.SetValue( vA OP vB, i ); \
							} \
						} \
					} \
					else \
					{ \
						/* Both are uniform, simple one shot case. */ \
						pA->GetValue( vA ); \
						pB->m_Value.GetValue( vB ); \
						Res.SetValue( vA OP vB ); \
					} \
				} \
			} \
			else \
			{ \
				if( !fBDat ) \
				{ \
					CqVMStackEntryVarRef* pVRB = static_cast<CqVMStackEntryVarRef*>(pB); \
					/* A is data, B is a variable. */  \
					if( fAVar && fBVar )\
					{ \
						/* Both are varying, must go accross all processing each element. */ \
						ii = pA->Size(); \
						pVRB->m_pVarRef->GetValuePtr( pdB ); \
						for ( i = 0; i < ii; i++ ) \
						{ \
							if ( RunningState.Value( i ) ) \
							{ \
								pA->GetValue( vA, i ); \
								Res.SetValue( vA OP (*pdB), i ); \
							} \
							pdB++; \
						} \
					} \
					else if( !fBVar && fAVar) \
					{ \
						/* A is varying, can just get B's value once. */ \
						ii = pA->Size(); \
						pVRB->m_pVarRef->GetValue( vB ); \
						for ( i = 0; i < ii; i++ ) \
						{ \
							if ( RunningState.Value( i ) ) \
							{ \
								pA->GetValue( vA, i ); \
								Res.SetValue( vA OP vB, i ); \
							} \
						} \
					} \
					else if( !fAVar && fBVar) \
					{ \
						/* B is varying, can just get A's value once. */ \
						ii = pB->Size(); \
						pVRB->m_pVarRef->GetValuePtr( pdB ); \
						pA->GetValue( vA ); \
						for ( i = 0; i < ii; i++ ) \
						{ \
							if ( RunningState.Value( i ) ) \
								Res.SetValue( vA OP (*pdB), i ); \
							pdB++; \
						} \
					} \
					else \
					{ \
						/* Both are uniform, simple one shot case. */ \
						pA->GetValue( vA ); \
						pVRB->m_pVarRef->GetValue( vB ); \
						Res.SetValue( vA OP vB ); \
					} \
				} \
				else \
				{ \
					/* Both A and B are data. */  \
					if( fAVar && fBVar )\
					{ \
						/* Both are varying, must go accross all processing each element. */ \
						ii = pA->Size(); \
						for ( i = 0; i < ii; i++ ) \
						{ \
							if ( RunningState.Value( i ) ) \
							{ \
								pA->GetValue( vA, i ); \
								pB->m_aValues[ i ].GetValue( vB ); \
								Res.SetValue( vA OP vB, i ); \
							} \
						} \
					} \
					else if( !fBVar && fAVar) \
					{ \
						/* A is varying, can just get B's value once. */ \
						ii = pA->Size(); \
						pB->m_Value.GetValue( vB ); \
						for ( i = 0; i < ii; i++ ) \
						{ \
							if ( RunningState.Value( i ) ) \
							{ \
								pA->GetValue( vA, i ); \
								Res.SetValue( vA OP vB, i ); \
							} \
						} \
					} \
					else if( !fAVar && fBVar) \
					{ \
						/* B is varying, can just get A's value once. */ \
						ii = pB->Size(); \
						pA->GetValue( vA ); \
						for ( i = 0; i < ii; i++ ) \
						{ \
							if ( RunningState.Value( i ) ) \
							{ \
								pB->m_aValues[ i ].GetValue( vB ); \
								Res.SetValue( vA OP vB, i ); \
							} \
						} \
					} \
					else \
					{ \
						/* Both are uniform, simple one shot case. */ \
						pA->m_Value.GetValue( vA ); \
						pB->m_Value.GetValue( vB ); \
						Res.SetValue( vA OP vB ); \
					} \
				} \
			} \
		} 



/** \enum EqStackEntryType
 */
enum EqStackEntryType
{
    StackEntryType_Float, 	///< Float.
    StackEntryType_Int, 		///< Integer.
    StackEntryType_Bool, 	///< Boolean.
    StackEntryType_Point, 	///< 3D point..
    StackEntryType_HPoint, 	///< 4D Homogenous point.
    StackEntryType_Color, 	///< Color.
    StackEntryType_String, 	///< String.
    StackEntryType_Matrix, 	///< Matrix.
};

struct CqVMStackData
{
	CqVMStackData( TqFloat f = 0 )
	{
		*this = f;
	}
	CqVMStackData( const CqVector3D& v )
	{
		*this = v;
	}
	CqVMStackData( const CqColor& c )
	{
		*this = c;
	}
	CqVMStackData( const char* s )
	{
		m_Value.m_str = NULL;
		*this = s;
	}
	CqVMStackData( const CqString& s )
	{
		m_Value.m_str = NULL;
		*this = s;
	}
	CqVMStackData( const CqMatrix& m )
	{
		*this = m;
	}
	~CqVMStackData()
	{
		if(	m_Type == StackEntryType_String )
			delete[]( m_Value.m_str );
	}

	// Cast to the various types
	/** Type checked cast to a float
	 */
	void GetValue( TqFloat& res) const
	{
		if ( m_Type == StackEntryType_Bool )
		   	res = static_cast<TqFloat>(m_Value.m_bool);
		else if ( m_Type == StackEntryType_Int )
			res = static_cast<TqFloat>( m_Value.m_int );
		else
			res = m_Value.m_float;
	}
	/** Type checked cast to an integer
	 */
	void GetValue( TqInt& res ) const
	{
		if ( m_Type == StackEntryType_Bool )
		   	res = static_cast<TqInt>(m_Value.m_bool);
		else if ( m_Type == StackEntryType_Float )
			res = static_cast<TqInt>( m_Value.m_float );
		else
		res = m_Value.m_int;
	}
	/** Type checked cast to a boolean
	 */
	void GetValue( bool& res) const
	{
		if ( m_Type == StackEntryType_Float )
			res = static_cast<TqBool>( m_Value.m_float != 0.0 );
		else if ( m_Type == StackEntryType_Int )
			res = static_cast<TqBool>( m_Value.m_int != 0 );
		else if ( m_Type == StackEntryType_Bool )
		res = m_Value.m_bool;
	}
	/** Type checked cast to a 3D vector
	 */
	void GetValue( CqVector3D& res ) const
	{
		if ( m_Type == StackEntryType_Float )
		{
			res.x(m_Value.m_float);
			res.y(m_Value.m_float);
			res.z(m_Value.m_float);
		}
		else if ( m_Type == StackEntryType_Int )
		{
			res.x(static_cast<TqFloat>( m_Value.m_int ));
			res.y(static_cast<TqFloat>( m_Value.m_int ));
			res.z(static_cast<TqFloat>( m_Value.m_int ));
		}
		else if ( m_Type == StackEntryType_Bool )
		{
			res.x(static_cast<TqFloat>( m_Value.m_bool ));
			res.y(static_cast<TqFloat>( m_Value.m_bool ));
			res.z(static_cast<TqFloat>( m_Value.m_bool ));
		}
		else if ( m_Type == StackEntryType_HPoint )
		{
			res.x(m_Value.m_hpoint[0]/m_Value.m_hpoint[3]);
			res.y(m_Value.m_hpoint[1]/m_Value.m_hpoint[3]);
			res.z(m_Value.m_hpoint[2]/m_Value.m_hpoint[3]);
		}
		else
		{
			res.x(m_Value.m_point[0]);
			res.y(m_Value.m_point[1]);
			res.z(m_Value.m_point[2]);
		}
	}

	/** Type checked cast to a 4D vector.
	 */
	void GetValue( CqVector4D& res ) const
	{
		if ( m_Type == StackEntryType_Float )
		{
			res.x(m_Value.m_float);
			res.y(m_Value.m_float);
			res.z(m_Value.m_float);
			res.h(1.0f);
		}
		else if ( m_Type == StackEntryType_Int )
		{
			res.x(static_cast<TqFloat>( m_Value.m_int ));
			res.y(static_cast<TqFloat>( m_Value.m_int ));
			res.z(static_cast<TqFloat>( m_Value.m_int ));
			res.h(1.0f);
		}
		else if ( m_Type == StackEntryType_Bool )
		{
			res.x(static_cast<TqFloat>( m_Value.m_bool ));
			res.y(static_cast<TqFloat>( m_Value.m_bool ));
			res.z(static_cast<TqFloat>( m_Value.m_bool ));
			res.h(1.0f);
		}
		else if ( m_Type == StackEntryType_Point )
		{
			res.x(m_Value.m_point[0]);
			res.y(m_Value.m_point[1]);
			res.z(m_Value.m_point[2]);
			res.h(1.0f);
		}
		else
		{
			res.x(m_Value.m_hpoint[0]);
			res.y(m_Value.m_hpoint[1]);
			res.z(m_Value.m_hpoint[2]);
			res.h(m_Value.m_hpoint[3]);
		}
	}
	/** Type checked cast to a color
	 */
	void GetValue( CqColor& res ) const
	{
		if ( m_Type == StackEntryType_Float )
		{
			res.SetfRed(m_Value.m_float);
			res.SetfGreen(m_Value.m_float);
			res.SetfBlue(m_Value.m_float);
		}
		else if ( m_Type == StackEntryType_Int )
		{
			res.SetfRed(static_cast<TqFloat>( m_Value.m_int ));
			res.SetfGreen(static_cast<TqFloat>( m_Value.m_int ));
			res.SetfBlue(static_cast<TqFloat>( m_Value.m_int ));
		}
		else if ( m_Type == StackEntryType_Bool )
		{
			res.SetfRed(static_cast<TqFloat>( m_Value.m_bool ));
			res.SetfGreen(static_cast<TqFloat>( m_Value.m_bool ));
			res.SetfBlue(static_cast<TqFloat>( m_Value.m_bool ));
		}
		else if ( m_Type == StackEntryType_HPoint )
		{
			res.SetfRed(m_Value.m_hpoint[0]/m_Value.m_hpoint[3]);
			res.SetfGreen(m_Value.m_hpoint[1]/m_Value.m_hpoint[3]);
			res.SetfBlue(m_Value.m_hpoint[2]/m_Value.m_hpoint[3]);
		}
		else
		{
			res.SetfRed(m_Value.m_color[0]);
			res.SetfGreen(m_Value.m_color[1]);
			res.SetfBlue(m_Value.m_color[2]);
		}
	}
	/** Type checked cast to a string
	 */
	void GetValue( CqString& res ) const
	{
		assert( m_Type == StackEntryType_String );
		res = m_Value.m_str;
	}
	/** Type checked cast to a matrix
	 */
	void GetValue( CqMatrix& res ) const
	{
		assert( m_Type == StackEntryType_Matrix );
		res[0][0] = m_Value.m_matrix[0 ];
		res[0][1] = m_Value.m_matrix[1 ];
		res[0][2] = m_Value.m_matrix[2 ];
		res[0][3] = m_Value.m_matrix[3 ];
		res[1][0] = m_Value.m_matrix[4 ];
		res[1][1] = m_Value.m_matrix[5 ];
		res[1][2] = m_Value.m_matrix[6 ];
		res[1][3] = m_Value.m_matrix[7 ];
		res[2][0] = m_Value.m_matrix[8 ];
		res[2][1] = m_Value.m_matrix[9 ];
		res[2][2] = m_Value.m_matrix[10];
		res[2][3] = m_Value.m_matrix[11];
		res[3][0] = m_Value.m_matrix[12];
		res[3][1] = m_Value.m_matrix[13];
		res[3][2] = m_Value.m_matrix[14];
		res[3][3] = m_Value.m_matrix[15];
		res.SetfIdentity( TqFalse );
	}



	/** Assignment from a float
	 */
	CqVMStackData& operator=( TqFloat f )
	{
		m_Value.m_float = f; 
		m_Type = StackEntryType_Float; 
		return ( *this );
	}
	/** Assignment from an integer
	 */
	CqVMStackData& operator=( TqInt i )
	{
		m_Value.m_int = i; 
		m_Type = StackEntryType_Int; 
		return ( *this );
	}
	/** Assignment from a boolean
	 */
	CqVMStackData& operator=( bool b )
	{
		m_Value.m_bool = b; 
		m_Type = StackEntryType_Bool; 
		return ( *this );
	}
	/** Assignment from a 4D vector
	 */
	CqVMStackData& operator=( const CqVector4D& v )
	{
		m_Value.m_hpoint[0] = v.x();
		m_Value.m_hpoint[1] = v.y();
		m_Value.m_hpoint[2] = v.z();
		m_Value.m_hpoint[3] = v.h();
		m_Type = StackEntryType_HPoint; 
		return ( *this );
	}
	/** Assignment from a 3D vector
	 */
	CqVMStackData& operator=( const CqVector3D& v )
	{
		m_Value.m_point[0] = v.x();
		m_Value.m_point[1] = v.y();
		m_Value.m_point[2] = v.z();
		m_Type = StackEntryType_Point; 
		return ( *this );
	}
	/** Assignment from a color
	 */
	CqVMStackData& operator=( const CqColor& c )
	{
		m_Value.m_color[0] = c.fRed();
		m_Value.m_color[1] = c.fGreen();
		m_Value.m_color[2] = c.fBlue();
		m_Type = StackEntryType_Color; 
		return ( *this );
	}
	/** Assignment from a char pointer (string)
	 */
	CqVMStackData& operator=( const char* s )
	{
		if( m_Type == StackEntryType_String && m_Value.m_str != NULL )
			delete[]( m_Value.m_str );
		m_Value.m_str = new char[strlen(s)+1]; 
		strcpy(m_Value.m_str, s);
		m_Type = StackEntryType_String; 
		return ( *this );
	}
	/** Assignment from a string
	 */
	CqVMStackData& operator=( const CqString& s )
	{
		if( m_Type == StackEntryType_String && m_Value.m_str != NULL )
			delete[]( m_Value.m_str );
		m_Value.m_str = new char[s.size()+1]; 
		strcpy(m_Value.m_str, s.c_str());
		m_Type = StackEntryType_String; 
		return ( *this );
	}
	/** Assignment from a matrix
	 */
	CqVMStackData& operator=( const CqMatrix& m )
	{
		m_Value.m_matrix[0 ] = m[0][0];
		m_Value.m_matrix[1 ] = m[0][1];
		m_Value.m_matrix[2 ] = m[0][2];
		m_Value.m_matrix[3 ] = m[1][3];
		m_Value.m_matrix[4 ] = m[1][0];
		m_Value.m_matrix[5 ] = m[1][1];
		m_Value.m_matrix[6 ] = m[1][2];
		m_Value.m_matrix[7 ] = m[2][3];
		m_Value.m_matrix[8 ] = m[2][0];
		m_Value.m_matrix[9 ] = m[2][1];
		m_Value.m_matrix[10] = m[2][2];
		m_Value.m_matrix[11] = m[2][3];
		m_Value.m_matrix[12] = m[3][0];
		m_Value.m_matrix[13] = m[3][1];
		m_Value.m_matrix[14] = m[3][2];
		m_Value.m_matrix[15] = m[3][3];
		m_Type = StackEntryType_Matrix;
		return ( *this );
	}

	union
	{
		TqFloat	m_float;		///< Float value
		TqInt	m_int;			///< Integer value
		bool	m_bool;			///< Boolean value
		char*	m_str;			///< String value
		TqFloat	m_point[3];		///< 3D point value
		TqFloat	m_hpoint[4];	///< 4D homogenous point value
		TqFloat	m_color[3];		///< Color value
		TqFloat	m_matrix[16];	///< Matrix value
	}m_Value;
	TqInt	m_Type;			///< Type identifier, from EqVariableType.
}
;


class CqVMStackEntry : public IqShaderData
{
	public:
		CqVMStackEntry( TqInt size = 1 );
	virtual	~CqVMStackEntry()
		{}

		virtual TqBool	fIsRef()	{return(TqFalse);}

		// Value access functions overridden from IqShaderData
		virtual void GetFloat( TqFloat& f, TqInt index = 0 ) const
		{
			( m_Size == 1 )? m_Value.GetValue( f ) : m_aValues[ index ].GetValue( f );
		}
		virtual void GetBool( TqBool& b, TqInt index = 0 ) const
		{
			( m_Size == 1 )? m_Value.GetValue( b ) : m_aValues[ index ].GetValue( b );
		}
		virtual void GetString( CqString& s, TqInt index = 0 ) const
		{
			( m_Size == 1 )? m_Value.GetValue( s ) : m_aValues[ index ].GetValue( s );
		}
		virtual void GetPoint( CqVector3D& p, TqInt index = 0 ) const
		{
			( m_Size == 1 )? m_Value.GetValue( p ) : m_aValues[ index ].GetValue( p );
		}
		virtual void GetVector( CqVector3D& v, TqInt index = 0 ) const
		{
			( m_Size == 1 )? m_Value.GetValue( v ) : m_aValues[ index ].GetValue( v );
		}
		virtual void GetNormal( CqVector3D& n, TqInt index = 0 ) const
		{
			( m_Size == 1 )? m_Value.GetValue( n ) : m_aValues[ index ].GetValue( n );
		}
		virtual void GetColor( CqColor& c, TqInt index = 0 ) const
		{
			( m_Size == 1 )? m_Value.GetValue( c ) : m_aValues[ index ].GetValue( c );
		}
		virtual void GetMatrix( CqMatrix& m, TqInt index = 0 ) const
		{
			( m_Size == 1 )? m_Value.GetValue( m ) : m_aValues[ index ].GetValue( m );
		}

		virtual	void		GetFloatPtr(const TqFloat*& res) const			{ assert(TqFalse); }
		virtual void		GetBoolPtr( const TqBool*& res ) const			{ assert(TqFalse); }
		virtual	void		GetStringPtr(const CqString*& res) const		{ assert(TqFalse); }
		virtual	void		GetPointPtr(const CqVector3D*& res) const		{ assert(TqFalse); }
		virtual	void		GetVectorPtr(const CqVector3D*& res) const		{ assert(TqFalse); }
		virtual	void		GetNormalPtr(const CqVector3D*& res) const		{ assert(TqFalse); }
		virtual	void		GetColorPtr(const CqColor*& res) const			{ assert(TqFalse); }
		virtual	void		GetMatrixPtr(const CqMatrix*& res) const		{ assert(TqFalse); }

		// Value setters, overridden from IqShaderData
		virtual void SetFloat( const TqFloat& f )
		{
			*this = f;
		}
		virtual void SetBool( const TqBool& b )
		{
			*this = b;
		}
		virtual void SetString( const CqString& s )
		{
			*this = s;
		}
		virtual void SetPoint( const CqVector3D& p )
		{
			*this = p;
		}
		virtual void SetVector( const CqVector3D& v )
		{
			*this = v;
		}
		virtual void SetNormal( const CqVector3D& n )
		{
			*this = n;
		}
		virtual void SetColor( const CqColor& c )
		{
			*this = c;
		}
		virtual void SetMatrix( const CqMatrix& m )
		{
			*this = m;
		}


		// Value setters, overridden from IqShaderData
		virtual void SetFloat( const TqFloat& f, TqInt index )
		{
			( m_Size == 1 )? m_Value = f : m_aValues[index] = f;
		}
		virtual void SetBool( const TqBool& b, TqInt index )
		{
			( m_Size == 1 )? m_Value = b : m_aValues[index] = b;
		}
		virtual void SetString( const CqString& s, TqInt index )
		{
			( m_Size == 1 )? m_Value = s : m_aValues[index] = s;
		}
		virtual void SetPoint( const CqVector3D& p, TqInt index )
		{
			( m_Size == 1 )? m_Value = p : m_aValues[index] = p;
		}
		virtual void SetVector( const CqVector3D& v, TqInt index )
		{
			( m_Size == 1 )? m_Value = v : m_aValues[index] = v;
		}
		virtual void SetNormal( const CqVector3D& n, TqInt index )
		{
			( m_Size == 1 )? m_Value = n : m_aValues[index] = n;
		}
		virtual void SetColor( const CqColor& c, TqInt index )
		{
			( m_Size == 1 )? m_Value = c : m_aValues[index] = c;
		}
		virtual void SetMatrix( const CqMatrix& m, TqInt index )
		{
			( m_Size == 1 )? m_Value = m : m_aValues[index] = m;
		}


		virtual	const CqString&	strName()			{ return(m_strName); }
		virtual	TqBool	fParameter() const			{ return(TqFalse); }

		virtual	void	Initialise( const TqInt uGridRes, const TqInt vGridRes )
		{
			m_Size = ( uGridRes + 1 ) * ( vGridRes + 1 );
			if ( m_aValues.size() < m_Size && m_Size > 1 )
				m_aValues.resize( m_Size );
		}

		virtual	IqShaderData* Clone() const
		{
			return( new CqVMStackEntry( *this ) );
		}

		virtual	void	SetValueFromVariable( IqShaderData* pVal, TqInt index )
		{
			switch( pVal->Type() )	
			{
				case type_float:
				{
					TqFloat f;
					pVal->GetFloat( f, index );
					SetFloat( f, index );
				}
				break;
				
				case type_point:
				{
					CqVector3D p;
					pVal->GetPoint( p, index );
					SetPoint( p, index );
				}
				break;

				case type_normal:
				{
					CqVector3D n;
					pVal->GetNormal( n, index );
					SetNormal( n, index );
				}
				break;

				case type_vector:
				{
					CqVector3D v;
					pVal->GetVector( v, index );
					SetVector( v, index );
				}
				break;

				case type_string:
				{
					CqString s;
					pVal->GetString( s, index );
					SetString( s, index );
				}
				break;

				case type_color:
				{
					CqColor c;
					pVal->GetColor( c, index );
					SetColor( c, index );
				}
				break;

				case type_matrix:
				{
					CqMatrix m;
					pVal->GetMatrix( m, index );
					SetMatrix( m, index );
				}
				break;
			}
		}

		virtual	void	SetValueFromVariable( IqShaderData* pVal )
		{
			switch( pVal->Type() )	
			{
				case type_float:
				{
					TqFloat f;
					pVal->GetFloat( f );
					SetFloat( f );
				}
				break;
				
				case type_point:
				{
					CqVector3D p;
					pVal->GetPoint( p );
					SetPoint( p );
				}
				break;

				case type_normal:
				{
					CqVector3D n;
					pVal->GetNormal( n );
					SetNormal( n );
				}
				break;

				case type_vector:
				{
					CqVector3D v;
					pVal->GetVector( v );
					SetVector( v );
				}
				break;

				case type_string:
				{
					CqString s;
					pVal->GetString( s );
					SetString( s );
				}
				break;

				case type_color:
				{
					CqColor c;
					pVal->GetColor( c );
					SetColor( c );
				}
				break;

				case type_matrix:
				{
					CqMatrix m;
					pVal->GetMatrix( m );
					SetMatrix( m );
				}
				break;
			}
		}

		virtual	EqVariableClass	Class() const
		{
			return( ( m_Size == 1 )? class_uniform : class_varying );
		}

		virtual	EqVariableType	Type() const
		{
			return( static_cast<EqVariableType>( ( m_Size == 1 )? m_Value.m_Type : m_aValues[0].m_Type ) );
		}

		virtual TqUint	Size() const
		{
			return( m_Size );
		}

		virtual TqInt	ArrayLength() const		
		{
			return( 0 );
		}
		virtual IqShaderData*	ArrayEntry(TqInt i)
		{
			return( this ); 
		}
		

		/** Assigment operator.
		 */
		CqVMStackEntry&	operator=( const CqVMStackEntry& e )
		{
			m_Size = e.Size();
			if ( Size() == 1 )
				m_Value = e.m_Value;
			else
			{
				if(m_aValues.size() < m_Size )
					m_aValues.resize( m_Size );
				TqInt i;
				for ( i = Size() - 1; i >= 0; i-- )
					m_aValues[ i ] = e.m_aValues[ i ];
			}
			return ( *this );
		}
		/** Assignment from a float.
		 * Takes care of casting to varying by duplication if neccessary.
		 */
		CqVMStackEntry&	operator=( TqFloat f )
		{
			TqInt i;
			if ( Size() == 1 ) m_Value = f;
			else
				for ( i = Size() - 1; i >= 0; i-- ) m_aValues[ i ] = f;
			return ( *this );
		}
		/** Assignment from an int.
		 * Takes care of casting to varying by duplication if neccessary.
		 */
		CqVMStackEntry&	operator=( TqInt v )
		{
			TqInt i;
			if ( Size() == 1 ) m_Value = v;
			else
				for ( i = Size() - 1; i >= 0; i-- ) m_aValues[ i ] = v;
			return ( *this );
		}
		/** Assignment from a boolean.
		 * Takes care of casting to varying by duplication if neccessary.
		 */
		CqVMStackEntry&	operator=( TqBool b )
		{
			TqInt i;
			if ( Size() == 1 ) m_Value = b;
			else
				for ( i = Size() - 1; i >= 0; i-- ) m_aValues[ i ] = b;
			return ( *this );
		}
		/** Assignment from a 3D vector.
		 * Takes care of casting to varying by duplication if neccessary.
		 */
		CqVMStackEntry&	operator=( const CqVector3D& v )
		{
			TqInt i;
			if ( Size() == 1 ) m_Value = v;
			else
				for ( i = Size() - 1; i >= 0; i-- ) m_aValues[ i ] = v;
			return ( *this );
		}
		/** Assignment from a color.
		 * Takes care of casting to varying by duplication if neccessary.
		 */
		CqVMStackEntry&	operator=( const CqColor& c )
		{
			TqInt i;
			if ( Size() == 1 ) m_Value = c;
			else
				for ( i = Size() - 1; i >= 0; i-- ) m_aValues[ i ] = c;
			return ( *this );
		}
		/** Assignment from a char pointer.
		 * Takes care of casting to varying by duplication if neccessary.
		 */
		CqVMStackEntry&	operator=( const char* s )
		{
			TqInt i;
			if ( Size() == 1 ) m_Value = s;
			else
				for ( i = Size() - 1; i >= 0; i-- ) m_aValues[ i ] = s;
			return ( *this );
		}
		/** Assignment from a string.
		 * Takes care of casting to varying by duplication if neccessary.
		 */
		CqVMStackEntry&	operator=( const CqString& s )
		{
			TqInt i;
			if ( Size() == 1 ) m_Value = s;
			else
				for ( i = Size() - 1; i >= 0; i-- ) m_aValues[ i ] = s;
			return ( *this );
		}
		/** Assignment from a matrix.
		 * Takes care of casting to varying by duplication if neccessary.
		 */
		CqVMStackEntry&	operator=( const CqMatrix& m )
		{
			TqInt i;
			if ( Size() == 1 ) m_Value = m;
			else
				for ( i = Size() - 1; i >= 0; i-- ) m_aValues[ i ] = m;
			return ( *this );
		}

		std::vector < CqVMStackData /*,CqVMStackEntryAllocator*/ > m_aValues;	///< Array of CqVMStackData storage stuctures.
		CqVMStackData	m_Value;		///< Single value in the case of a uniforn stack entry.
		TqUint	m_Size;			///< Size of the SIMD data.

	static	CqString m_strName;
}
;


class CqVMStackEntryVarRef : public CqVMStackEntry
{
	public:
		CqVMStackEntryVarRef() : m_pVarRef(0)	{}
	virtual	~CqVMStackEntryVarRef()
		{}

		virtual TqBool	fIsRef()	{return(TqTrue);}

		// Value access functions overridden from IqShaderData
		virtual void GetFloat( TqFloat& f, TqInt index = 0 ) const
		{
			assert( NULL != m_pVarRef );
			m_pVarRef->GetFloat( f, index );
		}
		virtual void GetBool( TqBool& b, TqInt index = 0 ) const
		{
			assert( false );
		}
		virtual void GetString( CqString& s, TqInt index = 0 ) const
		{
			assert( NULL != m_pVarRef );
			m_pVarRef->GetString( s, index );
		}
		virtual void GetPoint( CqVector3D& p, TqInt index = 0 ) const
		{
			assert( NULL != m_pVarRef );
			m_pVarRef->GetPoint( p, index );
		}
		virtual void GetVector( CqVector3D& v, TqInt index = 0 ) const
		{
			assert( NULL != m_pVarRef );
			m_pVarRef->GetVector( v, index );
		}
		virtual void GetNormal( CqVector3D& n, TqInt index = 0 ) const
		{
			assert( NULL != m_pVarRef );
			m_pVarRef->GetNormal( n, index );
		}
		virtual void GetColor( CqColor& c, TqInt index = 0 ) const
		{
			assert( NULL != m_pVarRef );
			m_pVarRef->GetColor( c, index );
		}
		virtual void GetMatrix( CqMatrix& m, TqInt index = 0 ) const
		{
			assert( NULL != m_pVarRef );
			m_pVarRef->GetMatrix( m, index );
		}

		// Value setters, overridden from IqShaderData
		virtual void SetFloat( const TqFloat& f )
		{
			assert( false );
		}
		virtual void SetBool( const TqBool& b )
		{
			assert( false );
		}
		virtual void SetString( const CqString& s )
		{
			assert( false );
		}
		virtual void SetPoint( const CqVector3D& p )
		{
			assert( false );
		}
		virtual void SetVector( const CqVector3D& v )
		{
			assert( false );
		}
		virtual void SetNormal( const CqVector3D& n )
		{
			assert( false );
		}
		virtual void SetColor( const CqColor& c )
		{
			assert( false );
		}
		virtual void SetMatrix( const CqMatrix& m )
		{
			assert( false );
		}


		// Value setters, overridden from IqShaderData
		virtual void SetFloat( const TqFloat& f, TqInt index )
		{
			assert( NULL != m_pVarRef );
			m_pVarRef->SetFloat( f, index );
		}
		virtual void SetBool( const TqBool& b, TqInt index )
		{
			assert( false );
		}
		virtual void SetString( const CqString& s, TqInt index )
		{
			assert( NULL != m_pVarRef );
			m_pVarRef->SetString( s, index );
		}
		virtual void SetPoint( const CqVector3D& p, TqInt index )
		{
			assert( NULL != m_pVarRef );
			m_pVarRef->SetPoint( p, index );
		}
		virtual void SetVector( const CqVector3D& v, TqInt index )
		{
			assert( NULL != m_pVarRef );
			m_pVarRef->SetVector( v, index );
		}
		virtual void SetNormal( const CqVector3D& n, TqInt index )
		{
			assert( NULL != m_pVarRef );
			m_pVarRef->SetNormal( n, index );
		}
		virtual void SetColor( const CqColor& c, TqInt index )
		{
			assert( NULL != m_pVarRef );
			m_pVarRef->SetColor( c, index );
		}
		virtual void SetMatrix( const CqMatrix& m, TqInt index )
		{
			assert( NULL != m_pVarRef );
			m_pVarRef->SetMatrix( m, index );
		}

		virtual	IqShaderData* Clone() const
		{
			return( new CqVMStackEntryVarRef( *this ) );
		}


		virtual	EqVariableClass	Class() const
		{
			assert( NULL != m_pVarRef );
			return( m_pVarRef->Class() );
		}

		virtual	EqVariableType	Type() const
		{
			assert( NULL != m_pVarRef );
			return( m_pVarRef->Type() );
		}

		virtual TqUint	Size() const
		{
			assert( NULL != m_pVarRef );
			return( m_pVarRef->Size() );
		}

		virtual TqInt	ArrayLength() const		
		{
			assert( NULL != m_pVarRef );
			return( m_pVarRef->ArrayLength() );
		}
		virtual IqShaderData*	ArrayEntry(TqInt i)
		{
			assert( NULL != m_pVarRef );
			return( m_pVarRef->ArrayEntry(i) );
		}
		

		/** Assigment operator.
		 */
		CqVMStackEntryVarRef&	operator=( const CqVMStackEntryVarRef& e )
		{
			m_pVarRef = e.m_pVarRef;
			return ( *this );
		}

		CqVMStackEntry&	operator=( IqShaderData* pv )
		{
			m_pVarRef = pv;
			return( *this );
		}
		IqShaderData*	m_pVarRef;		///< Pointer to a referenced variable if a variable stack entry.

	friend class CqVMStackEntry;
}
;



//----------------------------------------------------------------------
/** \class CqShaderStack
 * Class handling the shader execution stack.
 */

class _qShareC CqShaderStack
{
	public:
		_qShareM CqShaderStack() : m_iTop( 0 ), m_iEntriesTop( 0 ), m_iRefsTop( 0 )
		{
			m_Stack.resize( 48 );
			m_EntriesPool.resize( 48 );
			m_RefsPool.resize( 48 );
		}
		virtual _qShareM ~CqShaderStack()
		{
			m_Stack.clear();
			m_EntriesPool.clear();
			m_RefsPool.clear();
		}

		void Push( CqVMStackEntry* pEntry)
		{
			if ( m_iTop >= m_Stack.size() )
				m_Stack.resize( m_Stack.size() + 1 );

			m_Stack[ m_iTop++ ] = pEntry;
		}

		/** Get a reference to the next item on the stack to be pushed.
		 */
		CqVMStackEntry& GetPush()
		{
			if ( m_iEntriesTop >= m_EntriesPool.size() )
				m_EntriesPool.resize( m_EntriesPool.size() + 1 );

			return ( m_EntriesPool[ m_iEntriesTop++ ] );
		}

		/** Get a reference to the next item on the stack to be pushed.
		 */
		CqVMStackEntryVarRef& GetRefPush()
		{
			if ( m_iRefsTop >= m_RefsPool.size() )
				m_RefsPool.resize( m_RefsPool.size() + 1 );

			return ( m_RefsPool[ m_iRefsTop++ ] );
		}

		/** Push a new stack entry onto the stack.
		 */
		void	Push( const CqVMStackEntry& E )
		{
			CqVMStackEntry & s = GetPush();
			s = E;
			Push( &s );
		}
		/** Push a new float onto the stack.
		 */
		void	Push( const TqFloat f )
		{
			CqVMStackEntry & s = GetPush();
			s.Initialise( 0, 0 );
			s = f;
			Push( &s );
		}
		/** Push a new boolean onto the stack.
		 */
		void	Push( const TqBool b )
		{
			CqVMStackEntry & s = GetPush();
			s.Initialise( 0, 0 );
			s = b;
			Push( &s );
		}
		/** Push a new 3D vector onto the stack.
		 */
		void	Push( const CqVector3D& v )
		{
			CqVMStackEntry & s = GetPush();
			s.Initialise( 0, 0 );
			s = v;
			Push( &s );
		}
		/** Push a new color onto the stack, passed as 3 floats.
		 */
		void	Push( const TqFloat r, TqFloat g, TqFloat b )
		{
			CqVMStackEntry & s = GetPush();
			s.Initialise( 0, 0 );
			s = CqColor( r, g, b );
			Push( &s );
		}
		/** Push a new color onto the stack.
		 */
		void	Push( const CqColor& c )
		{
			CqVMStackEntry & s = GetPush();
			s.Initialise( 0, 0 );
			s = c;
			Push( &s );
		}
		/** Push a new string onto the stack.
		 */
		void	Push( const CqString& str )
		{
			CqVMStackEntry & s = GetPush();
			s.Initialise( 0, 0 );
			s = str;
			Push( &s );
		}
		/** Push a new string, passed as a character onto the stack.
		 */
		void	Push( const char* ps )
		{
			CqVMStackEntry & s = GetPush();
			s.Initialise( 0, 0 );
			s = ps;
			Push( &s );
		}
		/** Push a new matrix onto the stack.
		 */
		void	Push( const CqMatrix& mat )
		{
			CqVMStackEntry & s = GetPush();
			s.Initialise( 0, 0 );
			s = mat;
			Push( &s );
		}
		/** Push a new shader variable reference onto the stack.
		 */
		void	Push( IqShaderData* pv )
		{
			CqVMStackEntryVarRef & s = GetRefPush();
			s = pv;
			Push( &s );
		}
		/** Pop the top stack entry.
		 * \param f Boolean value to update if this is varying. If not varying, leaves f unaltered.
		 * \return Reference to the top stack entry.
		 */
		CqVMStackEntry* Pop( TqBool& f )
		{
			if ( m_iTop ) m_iTop--;
			CqVMStackEntry* pVal = m_Stack[ m_iTop ];
			
			if ( !pVal->fIsRef() && m_iEntriesTop ) m_iEntriesTop--;
			if ( pVal->fIsRef() && m_iRefsTop ) m_iRefsTop--;

			f = pVal->Size() > 1 || f;
			return ( pVal );
		}
		/** Duplicate the top stack entry.
		 */
		void	Dup()
		{
			TqInt iTop = m_iTop;
			
			if( m_Stack[ iTop ]->fIsRef() )
			{
				CqVMStackEntryVarRef & s = GetRefPush();
				CqVMStackEntryVarRef* pTop = static_cast<CqVMStackEntryVarRef*>(m_Stack[ iTop ]);
				s = *pTop;
				Push( &s );
			}
			else
			{
				CqVMStackEntry & s = GetPush();
				s = *(m_Stack[ iTop ]);
				Push( &s );
			}
		}
		/** Drop the top stack entry.
		 */
		void	Drop()
		{
			if ( m_iTop )			m_iTop--;
			CqVMStackEntry* pVal = m_Stack[ m_iTop ];
			if ( !pVal->fIsRef() && m_iEntriesTop ) m_iEntriesTop--;
			if ( pVal->fIsRef() && m_iRefsTop ) m_iRefsTop--;
		}
	protected:
		std::vector<CqVMStackEntry>			m_EntriesPool;	///< Array of stacke entries.
		TqUint	m_iEntriesTop;
		std::vector<CqVMStackEntryVarRef>	m_RefsPool;		///< Array of stacke entries.
		TqUint	m_iRefsTop;
		std::vector<CqVMStackEntry*>		m_Stack;
		TqUint	m_iTop;										///< Index of the top entry.
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
inline void	OpMULV( CqVMStackEntry* pA, CqVMStackEntry* pB, CqVMStackEntry& Res, CqBitVector& RunningState )
{
	CqVector3D	vecA, vecB;

	TqInt i = MAX( MAX( pA->Size(), pB->Size() ), Res.Size() ) - 1;
	TqBool __fVarying = i > 0;
	for ( ; i >= 0; i-- )
	{
		if ( !__fVarying || RunningState.Value( i ) )
		{
			pA->GetValue( vecA, i);
			pB->GetValue( vecB, i);
			Res.SetValue( CqVector3D( vecA.x() * vecB.x(),
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
inline void	OpNEG( A& a, CqVMStackEntry* pA, CqVMStackEntry& Res, CqBitVector& RunningState )
{
	A vA;

	TqInt i = MAX( pA->Size(), Res.Size() ) - 1;
	TqBool __fVarying = i > 0;
	for ( ; i >= 0; i-- )
		if ( !__fVarying || RunningState.Value( i ) )
		{
			pA->GetValue( vA, i );
			Res.SetValue( -vA, i );
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
inline void	OpCAST( A& a, B& b, CqVMStackEntry* pA, CqVMStackEntry& Res, CqBitVector& RunningState )
{
	A vA;

	TqInt i = MAX( pA->Size(), Res.Size() ) - 1;
	TqBool __fVarying = i > 0;
	for ( ; i >= 0; i-- )
		if ( !__fVarying || RunningState.Value( i ) )
		{
			pA->GetValue( vA, i );
			Res.SetValue( static_cast<B>( vA ), i );
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
inline void	OpTRIPLE( A&, CqVMStackEntry* pRes, CqVMStackEntry* pA, CqVMStackEntry* pB, CqVMStackEntry* pC, CqBitVector& RunningState )
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
inline void	OpHEXTUPLE( A& z, CqVMStackEntry* pRes,
						  CqVMStackEntry* pA, CqVMStackEntry* pB, CqVMStackEntry* pC, CqVMStackEntry* pD,
						  CqVMStackEntry* pE, CqVMStackEntry* pF, CqVMStackEntry* pG, CqVMStackEntry* pH,
						  CqVMStackEntry* pI, CqVMStackEntry* pJ, CqVMStackEntry* pK, CqVMStackEntry* pL,
						  CqVMStackEntry* pM, CqVMStackEntry* pN, CqVMStackEntry* pO, CqVMStackEntry* pP, CqBitVector& RunningState )
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
inline void	OpCOMP( A& z, CqVMStackEntry* pA, int index, CqVMStackEntry& Res, CqBitVector& RunningState )
{
	A vA;
	
	TqInt i = MAX( pA->Size(), Res.Size() ) - 1;
	TqBool __fVarying = i > 0;
	for ( ; i >= 0; i-- )
	{
		if ( !__fVarying || RunningState.Value( i ) )
		{
			pA->GetValue( vA, i );
			Res.SetValue( vA [ index ], i );
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
inline void	OpCOMP( A& z, CqVMStackEntry* pA, CqVMStackEntry* index, CqVMStackEntry& Res, CqBitVector& RunningState )
{
	A vA;
	TqFloat fi;

	TqInt i = MAX( MAX( pA->Size(), Res.Size() ), index->Size() ) - 1;
	TqBool __fVarying = i > 0;
	for ( ; i >= 0; i-- )
	{
		if ( !__fVarying || RunningState.Value( i ) )
		{
			pA->GetValue( vA, i );
			index->GetValue( fi, i );
			Res.SetValue( vA [ static_cast<TqInt>( fi ) ], i );
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
inline void	OpSETCOMP( A& z, CqVMStackEntry* pRes, int index, CqVMStackEntry* pA, CqBitVector& RunningState )
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
inline void	OpSETCOMP( A& z, CqVMStackEntry* pRes, CqVMStackEntry* index, CqVMStackEntry* pA, CqBitVector& RunningState )
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
inline void	OpCOMPM( CqVMStackEntry* pA, CqVMStackEntry* pR, CqVMStackEntry* pC, CqVMStackEntry& Res, CqBitVector& RunningState )
{
	CqMatrix m;
	TqFloat fr, fc;

	TqInt i = MAX( pA->Size(), Res.Size() ) - 1;
	TqBool __fVarying = i > 0;
	for ( ; i >= 0; i-- )
		if ( !__fVarying || RunningState.Value( i ) )
		{
			pA->GetValue( m, i );
			pR->GetValue( fr, i );
			pC->GetValue( fc, i );
			Res.SetValue( m [ static_cast<TqInt>( fr ) ][ static_cast<TqInt>( fc ) ], i );
		}
}

/** Special case matrix component access.
 * \param r Integer type stackentry row index.
 * \param c Integer type stackentry column index.
 * \param v Float type stackentry value to set index to.
 * \param RunningState The current SIMD state.
 */
inline void	OpSETCOMPM( CqVMStackEntry* pA, CqVMStackEntry* pR, CqVMStackEntry* pC, CqVMStackEntry* pV, CqBitVector& RunningState )
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
