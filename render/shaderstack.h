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

#include	"vector3d.h"
#include	"vector4d.h"
#include	"color.h"
#include	"sstring.h"
#include	"matrix.h"
#include	"shadervariable.h"

#include	"specific.h"	// Needed for namespace macros.

START_NAMESPACE(Aqsis)

#define	OpLSS_FF(a,Res,State)		OpLSS(temp_float,temp_float,a,Res,State)
#define	OpLSS_PP(a,Res,State)		OpLSS(temp_point,temp_point,a,Res,State)
#define	OpLSS_CC(a,Res,State)		OpLSS(temp_color,temp_color,a,Res,State)

#define	OpGRT_FF(a,Res,State)		OpGRT(temp_float,temp_float,a,Res,State)
#define	OpGRT_PP(a,Res,State)		OpGRT(temp_point,temp_point,a,Res,State)
#define	OpGRT_CC(a,Res,State)		OpGRT(temp_color,temp_color,a,Res,State)

#define	OpLE_FF(a,Res,State)		OpLE(temp_float,temp_float,a,Res,State)
#define	OpLE_PP(a,Res,State)		OpLE(temp_point,temp_point,a,Res,State)
#define	OpLE_CC(a,Res,State)		OpLE(temp_color,temp_color,a,Res,State)

#define	OpGE_FF(a,Res,State)		OpGE(temp_float,temp_float,a,Res,State)
#define	OpGE_PP(a,Res,State)		OpGE(temp_point,temp_point,a,Res,State)
#define	OpGE_CC(a,Res,State)		OpGE(temp_color,temp_color,a,Res,State)

#define	OpEQ_FF(a,Res,State)		OpEQ(temp_float,temp_float,a,Res,State)
#define	OpEQ_PP(a,Res,State)		OpEQ(temp_point,temp_point,a,Res,State)
#define	OpEQ_CC(a,Res,State)		OpEQ(temp_color,temp_color,a,Res,State)
#define	OpEQ_SS(a,Res,State)		OpEQ(temp_string,temp_string,a,Res,State)

#define	OpNE_FF(a,Res,State)		OpNE(temp_float,temp_float,a,Res,State)
#define	OpNE_PP(a,Res,State)		OpNE(temp_point,temp_point,a,Res,State)
#define	OpNE_CC(a,Res,State)		OpNE(temp_color,temp_color,a,Res,State)
#define	OpNE_SS(a,Res,State)		OpNE(temp_string,temp_string,a,Res,State)

#define	OpMUL_FF(a,Res,State)		OpMUL(temp_float,temp_float,a,Res,State)
#define	OpDIV_FF(a,Res,State)		OpDIV(temp_float,temp_float,a,Res,State)
#define	OpADD_FF(a,Res,State)		OpADD(temp_float,temp_float,a,Res,State)
#define	OpSUB_FF(a,Res,State)		OpSUB(temp_float,temp_float,a,Res,State)
#define	OpNEG_F(Res,State)			OpNEG(temp_float,Res,State)
									
#define	OpMUL_PP(a,Res,State)		OpMUL(temp_point,temp_point,a,Res,State)
#define	OpDIV_PP(a,Res,State)		OpDIV(temp_point,temp_point,a,Res,State)
#define	OpADD_PP(a,Res,State)		OpADD(temp_point,temp_point,a,Res,State)
#define	OpSUB_PP(a,Res,State)		OpSUB(temp_point,temp_point,a,Res,State)
#define	OpCRS_PP(a,Res,State)		OpCRS(temp_point,temp_point,a,Res,State)
#define	OpDOT_PP(a,Res,State)		OpDOT(temp_point,temp_point,a,Res,State)
#define	OpNEG_P(Res,State)			OpNEG(temp_point,Res,State)
									
#define	OpMUL_CC(a,Res,State)		OpMUL(temp_color,temp_color,a,Res,State)
#define	OpDIV_CC(a,Res,State)		OpDIV(temp_color,temp_color,a,Res,State)
#define	OpADD_CC(a,Res,State)		OpADD(temp_color,temp_color,a,Res,State)
#define	OpSUB_CC(a,Res,State)		OpSUB(temp_color,temp_color,a,Res,State)
#define	OpCRS_CC(a,Res,State)		OpCRS(temp_color,temp_color,a,Res,State)
#define	OpDOT_CC(a,Res,State)		OpDOT(temp_color,temp_color,a,Res,State)
#define	OpNEG_C(Res,State)			OpNEG(temp_color,Res,State)
									
#define	OpMUL_FP(a,Res,State)		OpMUL(temp_float,temp_point,a,Res,State)
#define	OpDIV_FP(a,Res,State)		OpDIV(temp_float,temp_point,a,Res,State)
#define	OpADD_FP(a,Res,State)		OpADD(temp_float,temp_point,a,Res,State)
#define	OpSUB_FP(a,Res,State)		OpSUB(temp_float,temp_point,a,Res,State)
									
#define	OpMUL_FC(a,Res,State)		OpMUL(temp_float,temp_color,a,Res,State)
#define	OpDIV_FC(a,Res,State)		OpDIV(temp_float,temp_color,a,Res,State)
#define	OpADD_FC(a,Res,State)		OpADD(temp_float,temp_color,a,Res,State)
#define	OpSUB_FC(a,Res,State)		OpSUB(temp_float,temp_color,a,Res,State)
									
#define	OpLAND_B(a,Res,State)		OpLAND(temp_bool,temp_bool,a,Res,State)
#define	OpLOR_B(a,Res,State)		OpLOR(temp_bool,temp_bool,a,Res,State)

#define	OpCAST_FC(Res,State)		OpCAST(temp_float,temp_color, Res,State)
#define	OpCAST_FP(Res,State)		OpCAST(temp_float,temp_point, Res,State)
#define	OpCAST_PC(Res,State)		OpCAST(temp_point,temp_color, Res,State)
#define	OpCAST_CP(Res,State)		OpCAST(temp_color,temp_point, Res,State)
#define	OpCAST_FM(Res,State)		OpCAST(temp_float,temp_matrix, Res,State)

#define	OpTRIPLE_C(a,b,c,State)		OpTRIPLE(temp_color,a,b,c,State)
#define	OpTRIPLE_P(a,b,c,State)		OpTRIPLE(temp_point,a,b,c,State)

#define	OpHEXTUPLE_M(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,State)	OpHEXTUPLE(temp_matrix,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,State)

#define	OpCOMP_C(index,Res,State)	OpCOMP(temp_color,index,Res,State)
#define	OpCOMP_P(index,Res,State)	OpCOMP(temp_point,index,Res,State)

#define	OpSETCOMP_C(index,a,State)	OpSETCOMP(temp_color,index,a,State)
#define	OpSETCOMP_P(index,a,State)	OpSETCOMP(temp_point,index,a,State)

/** \enum EqStackEntryType
 */
enum EqStackEntryType
{
	StackEntryType_Float,	///< Float.
	StackEntryType_Int,		///< Integer.
	StackEntryType_Bool,	///< Boolean.
	StackEntryType_Point,	///< 3D point..
	StackEntryType_HPoint,	///< 4D Homogenous point.
	StackEntryType_Color,	///< Color.
	StackEntryType_String,	///< String.
	StackEntryType_Matrix,	///< Matrix.
};

struct SqVMStackEntry
{
	SqVMStackEntry(TqFloat f=0)				: m_float(f), m_Type(StackEntryType_Float)	{}
	SqVMStackEntry(const CqVector3D& v)		: m_Point(v), m_Type(StackEntryType_Point)	{}
	SqVMStackEntry(const CqColor& c)		: m_Color(c), m_Type(StackEntryType_Color)	{}
	SqVMStackEntry(const char* s)			: m_str(s),   m_Type(StackEntryType_String)	{}
	SqVMStackEntry(const CqString& s)	: m_str(s),   m_Type(StackEntryType_String)	{}
	SqVMStackEntry(const CqMatrix& m)		: m_Matrix(m),m_Type(StackEntryType_Matrix)	{}

	// Cast to the various types
							/** Type checked cast to a float
							 */
	operator TqFloat&()		{assert(m_Type==StackEntryType_Float);	return(m_float);}
							/** Type checked cast to an integer
							 */
	operator TqInt&()		{assert(m_Type==StackEntryType_Int);	return(m_int);}
							/** Type checked cast to a boolean
							 */
	operator bool&()		{assert(m_Type==StackEntryType_Bool);	return(m_bool);}
							/** Type checked cast to a 3D vector
							 */
	operator CqVector3D&()	{assert(m_Type==StackEntryType_Point);	return(m_Point);}
							/** Type checked cast to a 4D vector.
							 */
	operator CqVector4D&()	{assert(m_Type==StackEntryType_HPoint);	return(m_HPoint);}
							/** Type checked cast to a color
							 */
	operator CqColor&()	{assert(m_Type==StackEntryType_Color);	return(m_Color);}
							/** Type checked cast to a string
							 */
	operator CqString&()	{assert(m_Type==StackEntryType_String);	return(m_str);}
							/** Type checked cast to a matrix
							 */
	operator CqMatrix&()	{assert(m_Type==StackEntryType_Matrix);	return(m_Matrix);}

							/** Assignment from a float
							 */
	SqVMStackEntry& operator=(TqFloat f)					{m_float=f; m_Type=StackEntryType_Float; return(*this);}
							/** Assignment from a double
							 */
	SqVMStackEntry& operator=(double d)					{m_float=d; m_Type=StackEntryType_Float; return(*this);}
							/** Assignment from an integer
							 */
	SqVMStackEntry& operator=(TqInt i)					{m_int=i; m_Type=StackEntryType_Int; return(*this);}
							/** Assignment from a boolean
							 */
	SqVMStackEntry& operator=(bool b)					{m_bool=b; m_Type=StackEntryType_Bool; return(*this);}
							/** Assignment from a 4D vector
							 */
	SqVMStackEntry& operator=(const CqVector4D& v)		{m_HPoint=v; m_Type=StackEntryType_HPoint; return(*this);}
							/** Assignment from a 3D vector
							 */
	SqVMStackEntry& operator=(const CqVector3D& v)		{m_Point=v; m_Type=StackEntryType_Point; return(*this);}
							/** Assignment from a color
							 */
	SqVMStackEntry& operator=(const CqColor& c)		{m_Color=c; m_Type=StackEntryType_Color; return(*this);}
							/** Assignment from a char pointer (string)
							 */
	SqVMStackEntry& operator=(const char* s)			{m_str=s; m_Type=StackEntryType_String; return(*this);}
							/** Assignment from a string
							 */
	SqVMStackEntry& operator=(const CqString& s)		{m_str=s; m_Type=StackEntryType_String; return(*this);}
							/** Assignment from a matrix
							 */
	SqVMStackEntry& operator=(const CqMatrix& m)		{m_Matrix=m; m_Type=StackEntryType_Matrix; return(*this);}
							/** Assignment from another stack entry
							 */
	SqVMStackEntry& operator=(const SqVMStackEntry& e)	{
															m_Type=e.m_Type;
															switch(m_Type)
															{
																case StackEntryType_Float:
																	m_float=e.m_float;
																	break;
																case StackEntryType_Int:
																	m_int=e.m_int;
																	break;
																case StackEntryType_Bool:
																	m_bool=e.m_bool;
																	break;
																case StackEntryType_Point:
																	m_Point=e.m_Point;
																	break;
																case StackEntryType_HPoint:
																	m_HPoint=e.m_HPoint;
																	break;
																case StackEntryType_Color:
																	m_Color=e.m_Color;
																	break;
																case StackEntryType_String:
																	m_str=e.m_str;
																	break;
																case StackEntryType_Matrix:
																	m_Matrix=e.m_Matrix;
																	break;
															}
															return(*this);
														}

	TqFloat					m_float;		///< Float value
	TqInt					m_int;			///< Integer value
	bool					m_bool;			///< Boolean value
	CqString				m_str;			///< String value
	CqVector3D				m_Point;		///< 3D point value
	CqVector4D				m_HPoint;		///< 4D homogenous point value
	CqColor				m_Color;		///< Color value
	CqMatrix				m_Matrix;		///< Matrix value
	TqInt					m_Type;			///< Type identifier, from EqVariableType.
};

//class CqVMStackEntryAllocator : public std::allocator<SqVMStackEntry>
//{
//	public:
//
//		void construct(pointer _P, const SqVMStackEntry& _V)
//					{new ((void _FARQ *)_P) SqVMStackEntry(); }
//};


class CqVMStackEntry
{
	public:
							CqVMStackEntry(TqInt size=1);
							CqVMStackEntry(CqShaderVariable* pv){m_pVarRef=pv;}
							~CqVMStackEntry()					{}

		// Value access functions
		// NOTE: The seemingly unused T& parameter is to overcome a VC++ compiler problem, you cannot call a template member
		// function as normal i.e. Foo.Value<int>(), but instead must let the compiler decide automatically the template to use.
		// Hopefully this will be fixed in VC++ 7.0
							/** Get the value from the stack entry by index.
							 * \param temp Temporary, to aid in the identification of the template argument for VC++.
							 * \param Index The integer index into the SIMD value array.
							 * \return A reference to the value.
							 */
		template<class T>
		T&	Value(T& temp, TqInt Index=0)			{
														if(m_pVarRef!=0)
														{
															// TODO: Should do some checking!!
															CqShaderVariableTyped<T>* pVar=static_cast<CqShaderVariableTyped<T>*>(m_pVarRef);
															return((*pVar)[Index]);
														}
														else
															return(static_cast<T&>((Size()==1)?m_Value:m_aValues[Index]));
													}
							/** Set the value of the stack entry by index.
							 * \param Index The integer index into the SIMD value array.
							 * \param val The new value.
							 */
		template<class T>
		void SetValue(TqInt Index, const T& val)	{
														if(m_pVarRef!=0)
														{
															// TODO: Should do some checking!!
															CqShaderVariableTyped<T>* pVar=static_cast<CqShaderVariableTyped<T>*>(m_pVarRef);
															(*pVar)[Index]=val;
														}
														else
															(Size()==1)?m_Value=val:m_aValues[Index]=val;
													}

							/** Determine whether the value is varying or uniform.
							 */
			TqBool			fVarying() const		{return(Size()>1);}
			TqInt			Size() const;
							/** Set the size of the SIMD array.
							 * \param size The new size.
							 */
			void			SetSize(TqInt size)		{
														m_Size=size; 
														if(m_aValues.size()<size && size>1)	
															m_aValues.resize(size);
													}
							/** Determine whether the stack entry is in fact a shader variable reference.
							 */
			TqBool			fVariable() const		{return(m_pVarRef!=0);}
							/** Get a pointer to the referenced shader variable.
							 * Only valid if fVariable returns TqTrue.
							 * \return A pointer to a CqShaderVariable.
							 */
		CqShaderVariable*	pVariable() const		{return(m_pVarRef);}
							/** Clear the variable reference pointer.
							 */
			void			ClearVariable()			{m_pVarRef=0;}

							/** Assigment operator.
							 */
		CqVMStackEntry&		operator=(const CqVMStackEntry& e)	
													{
														if(e.m_pVarRef!=0)
															m_pVarRef=e.m_pVarRef;
														else
														{
															m_pVarRef=0;
															SetSize(e.Size());
															if(Size()==1)
																m_Value=e.m_Value;
															else
															{
																TqInt i;
																for(i=Size()-1; i>=0; i--)
																	m_aValues[i]=e.m_aValues[i];
															}
														}
														return(*this);
													}
							/** Assignment from a float.
							 * Takes care of casting to varying by duplication if neccessary.
							 */
		CqVMStackEntry&		operator=(TqFloat f)	{
														m_pVarRef=0;
														TqInt i;
														if(Size()==1)					m_Value=f;
														else
															for(i=Size()-1; i>=0; i--)	m_aValues[i]=f;
														return(*this);
													}
							/** Assignment from a boolean.
							 * Takes care of casting to varying by duplication if neccessary.
							 */
		CqVMStackEntry&		operator=(TqBool b)	{
														m_pVarRef=0;
														TqInt i;
														if(Size()==1)					m_Value=b;
														else
															for(i=Size()-1; i>=0; i--)	m_aValues[i]=b;
														return(*this);
													}
							/** Assignment from a 3D vector.
							 * Takes care of casting to varying by duplication if neccessary.
							 */
		CqVMStackEntry&		operator=(const CqVector3D& v)
													{
														m_pVarRef=0;
														TqInt i;
														if(Size()==1)					m_Value=v;
														else 
															for(i=Size()-1; i>=0; i--)	m_aValues[i]=v;
														return(*this);
													}
							/** Assignment from a color.
							 * Takes care of casting to varying by duplication if neccessary.
							 */
		CqVMStackEntry&		operator=(const CqColor& c)
													{
														m_pVarRef=0;
														TqInt i;
														if(Size()==1)					m_Value=c;
														else
															for(i=Size()-1; i>=0; i--)	m_aValues[i]=c;
														return(*this);
													}
							/** Assignment from a char pointer.
							 * Takes care of casting to varying by duplication if neccessary.
							 */
		CqVMStackEntry&		operator=(const char* s)	{
														m_pVarRef=0;
														TqInt i;
														if(Size()==1)					m_Value=s;
														else
															for(i=Size()-1; i>=0; i--)	m_aValues[i]=s;
														return(*this);
													}
							/** Assignment from a string.
							 * Takes care of casting to varying by duplication if neccessary.
							 */
		CqVMStackEntry&		operator=(const CqString& s)
													{
														m_pVarRef=0;
														TqInt i;
														if(Size()==1)					m_Value=s;
														else
															for(i=Size()-1; i>=0; i--)	m_aValues[i]=s;
														return(*this);
													}
							/** Assignment from a matrix.
							 * Takes care of casting to varying by duplication if neccessary.
							 */
		CqVMStackEntry&		operator=(const CqMatrix& m)
													{
														m_pVarRef=0;
														TqInt i;
														if(Size()==1)					m_Value=m;
														else
															for(i=Size()-1; i>=0; i--)	m_aValues[i]=m;
														return(*this);
													}
		CqVMStackEntry&		operator=(CqShaderVariable* pv);

							/** Templatised less than operator.
							 * The template classes decide the cast used, there must be an appropriate operator between the two types.
							 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
							 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
							 * \param Comp The stack entry to use as the second operand.
							 * \param Res The stack entry to store the result in.
							 * \param RunningState The current SIMD state.
							 */
		template<class A, class B>
		void				OpLSS(A& a, B&b, CqVMStackEntry& Comp, CqVMStackEntry& Res, CqBitVector& RunningState)
													{
														TqInt i=MAX(MAX(Size(),Comp.Size()),Res.Size())-1;
														TqBool __fVarying=i>0;
														for(; i>=0; i--)
															if(!__fVarying || RunningState.Value(i))
																Res.SetValue(i,Value(a,i)<Comp.Value(b,i));
													}
							/** Templatised greater than operator.
							 * The template classes decide the cast used, there must be an appropriate operator between the two types.
							 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
							 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
							 * \param Comp The stack entry to use as the second operand.
							 * \param Res The stack entry to store the result in.
							 * \param RunningState The current SIMD state.
							 */
		template<class A, class B>
		void				OpGRT(A& a, B&b, CqVMStackEntry& Comp, CqVMStackEntry& Res, CqBitVector& RunningState)
													{
														TqInt i=MAX(MAX(Size(),Comp.Size()),Res.Size())-1;
														TqBool __fVarying=i>0;
														for(; i>=0; i--)
															if(!__fVarying || RunningState.Value(i))
																Res.SetValue(i,Value(a,i)>Comp.Value(b,i));
													}
							/** Templatised less than or equal to operator.
							 * The template classes decide the cast used, there must be an appropriate operator between the two types.
							 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
							 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
							 * \param Comp The stack entry to use as the second operand.
							 * \param Res The stack entry to store the result in.
							 * \param RunningState The current SIMD state.
							 */
		template<class A, class B>
		void				OpLE(A& a, B&b, CqVMStackEntry& Comp, CqVMStackEntry& Res, CqBitVector& RunningState)
													{
														TqInt i=MAX(MAX(Size(),Comp.Size()),Res.Size())-1;
														TqBool __fVarying=i>0;
														for(; i>=0; i--)
															if(!__fVarying || RunningState.Value(i))
																Res.SetValue(i,Value(a,i)<=Comp.Value(b,i));
													}
							/** Templatised greater than or equal to operator.
							 * The template classes decide the cast used, there must be an appropriate operator between the two types.
							 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
							 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
							 * \param Comp The stack entry to use as the second operand.
							 * \param Res The stack entry to store the result in.
							 * \param RunningState The current SIMD state.
							 */
		template<class A, class B>
		void				OpGE(A& a, B&b, CqVMStackEntry& Comp, CqVMStackEntry& Res, CqBitVector& RunningState)
													{
														TqInt i=MAX(MAX(Size(),Comp.Size()),Res.Size())-1;
														TqBool __fVarying=i>0;
														for(; i>=0; i--)
															if(!__fVarying || RunningState.Value(i))
																Res.SetValue(i,Value(a,i)>=Comp.Value(b,i));
													}
							/** Templatised equality operator.
							 * The template classes decide the cast used, there must be an appropriate operator between the two types.
							 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
							 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
							 * \param Comp The stack entry to use as the second operand.
							 * \param Res The stack entry to store the result in.
							 * \param RunningState The current SIMD state.
							 */
		template<class A, class B>
		void				OpEQ(A& a, B&b, CqVMStackEntry& Comp, CqVMStackEntry& Res, CqBitVector& RunningState)
													{
														TqInt i=MAX(MAX(Size(),Comp.Size()),Res.Size())-1;
														TqBool __fVarying=i>0;
														for(; i>=0; i--)
															if(!__fVarying || RunningState.Value(i))
																Res.SetValue(i,Value(a,i)==Comp.Value(b,i));
													}
							/** Templatised inequality operator.
							 * The template classes decide the cast used, there must be an appropriate operator between the two types.
							 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
							 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
							 * \param Comp The stack entry to use as the second operand.
							 * \param Res The stack entry to store the result in.
							 * \param RunningState The current SIMD state.
							 */
		template<class A, class B>
		void				OpNE(A& a, B&b, CqVMStackEntry& Comp, CqVMStackEntry& Res, CqBitVector& RunningState)
													{
														TqInt i=MAX(MAX(Size(),Comp.Size()),Res.Size())-1;
														TqBool __fVarying=i>0;
														for(; i>=0; i--)
															if(!__fVarying || RunningState.Value(i))
																Res.SetValue(i,Value(a,i)!=Comp.Value(b,i));
													}
							/** Templatised multiplication operator.
							 * The template classes decide the cast used, there must be an appropriate operator between the two types.
							 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
							 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
							 * \param Comp The stack entry to use as the second operand.
							 * \param Res The stack entry to store the result in.
							 * \param RunningState The current SIMD state.
							 */
		template<class A, class B>
		void				OpMUL(A& a, B&b, CqVMStackEntry& Comp, CqVMStackEntry& Res, CqBitVector& RunningState)
													{
														TqInt i=MAX(MAX(Size(),Comp.Size()),Res.Size())-1;
														TqBool __fVarying=i>0;
														for(; i>=0; i--)
															if(!__fVarying || RunningState.Value(i))
																Res.SetValue(i,Value(a,i)*Comp.Value(b,i));
													}
							/** Special case vector multiplication operator.
							 * The template classes decide the cast used, there must be an appropriate operator between the two types.
							 * \param Comp The stack entry to use as the second operand.
							 * \param Res The stack entry to store the result in.
							 * \param RunningState The current SIMD state.
							 */
		void				OpMULV(CqVMStackEntry& Comp, CqVMStackEntry& Res, CqBitVector& RunningState)
													{
														static CqVector3D	temp_point;
														TqInt i=MAX(MAX(Size(),Comp.Size()),Res.Size())-1;
														TqBool __fVarying=i>0;
														for(; i>=0; i--)
															if(!__fVarying || RunningState.Value(i))
																Res.SetValue(i,CqVector3D(Value(temp_point,i).x()*Comp.Value(temp_point,i).x(),
																					  Value(temp_point,i).y()*Comp.Value(temp_point,i).y(),
																					  Value(temp_point,i).z()*Comp.Value(temp_point,i).z()));
													}
							/** Templatised division operator.
							 * The template classes decide the cast used, there must be an appropriate operator between the two types.
							 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
							 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
							 * \param Comp The stack entry to use as the second operand.
							 * \param Res The stack entry to store the result in.
							 * \param RunningState The current SIMD state.
							 */
		template<class A, class B>
		void				OpDIV(A& a, B&b, CqVMStackEntry& Comp, CqVMStackEntry& Res, CqBitVector& RunningState)
													{
														TqInt i=MAX(MAX(Size(),Comp.Size()),Res.Size())-1;
														TqBool __fVarying=i>0;
														for(; i>=0; i--)
															if(!__fVarying || RunningState.Value(i))
																Res.SetValue(i,Value(a,i)/Comp.Value(b,i));
													}
							/** Templatised addition operator.
							 * The template classes decide the cast used, there must be an appropriate operator between the two types.
							 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
							 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
							 * \param Comp The stack entry to use as the second operand.
							 * \param Res The stack entry to store the result in.
							 * \param RunningState The current SIMD state.
							 */
		template<class A, class B>
		void				OpADD(A& a, B&b, CqVMStackEntry& Comp, CqVMStackEntry& Res, CqBitVector& RunningState)
													{
														TqInt i=MAX(MAX(Size(),Comp.Size()),Res.Size())-1;
														TqBool __fVarying=i>0;
														for(; i>=0; i--)
															if(!__fVarying || RunningState.Value(i))
																Res.SetValue(i,Value(a,i)+Comp.Value(b,i));
													}
							/** Templatised subtraction operator.
							 * The template classes decide the cast used, there must be an appropriate operator between the two types.
							 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
							 * \param a The type of the second operand, used to determine templateisation, needed by VC++..
							 * \param Comp The stack entry to use as the second operand.
							 * \param Res The stack entry to store the result in.
							 * \param RunningState The current SIMD state.
							 */
		template<class A, class B>
		void				OpSUB(A& a, B&b, CqVMStackEntry& Comp, CqVMStackEntry& Res, CqBitVector& RunningState)
													{
														TqInt i=MAX(MAX(Size(),Comp.Size()),Res.Size())-1;
														TqBool __fVarying=i>0;
														for(; i>=0; i--)
															if(!__fVarying || RunningState.Value(i))
																Res.SetValue(i,Value(a,i)-Comp.Value(b,i));
													}
							/** Templatised dot operator.
							 * The template classes decide the cast used, there must be an appropriate operator between the two types.
							 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
							 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
							 * \param Comp The stack entry to use as the second operand.
							 * \param Res The stack entry to store the result in.
							 * \param RunningState The current SIMD state.
							 * \attention Should only ever be called with vector based operands.
							 */
		template<class A, class B>
		void				OpDOT(A& a, B&b, CqVMStackEntry& Comp, CqVMStackEntry& Res, CqBitVector& RunningState)
													{
														TqInt i=MAX(MAX(Size(),Comp.Size()),Res.Size())-1;
														TqBool __fVarying=i>0;
														for(; i>=0; i--)
															if(!__fVarying || RunningState.Value(i))
																Res.SetValue(i,Value(a,i)*Comp.Value(b,i));
													}
							/** Templatised cross product operator.
							 * The template classes decide the cast used, there must be an appropriate operator between the two types.
							 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
							 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
							 * \param Comp The stack entry to use as the second operand.
							 * \param Res The stack entry to store the result in.
							 * \param RunningState The current SIMD state.
							 * \attention Should only ever be called with vector based operands.
							 */
		template<class A, class B>
		void				OpCRS(A& a, B&b, CqVMStackEntry& Comp, CqVMStackEntry& Res, CqBitVector& RunningState)
													{
														TqInt i=MAX(MAX(Size(),Comp.Size()),Res.Size())-1;
														TqBool __fVarying=i>0;
														for(; i>=0; i--)
															if(!__fVarying || RunningState.Value(i))
																Res.SetValue(i,Value(a,i)%Comp.Value(b,i));
													}
							/** Templatised logical AND operator.
							 * The template classes decide the cast used, there must be an appropriate operator between the two types.
							 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
							 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
							 * \param Comp The stack entry to use as the second operand.
							 * \param Res The stack entry to store the result in.
							 * \param RunningState The current SIMD state.
							 */
		template<class A, class B>
		void				OpLAND(A& a, B&b, CqVMStackEntry& Comp, CqVMStackEntry& Res, CqBitVector& RunningState)
													{
														TqInt i=MAX(MAX(Size(),Comp.Size()),Res.Size())-1;
														TqBool __fVarying=i>0;
														for(; i>=0; i--)
															if(!__fVarying || RunningState.Value(i))
																Res.SetValue(i,Value(a,i)&&Comp.Value(b,i));
													}
							/** Templatised logical OR operator.
							 * The template classes decide the cast used, there must be an appropriate operator between the two types.
							 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
							 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
							 * \param Comp The stack entry to use as the second operand.
							 * \param Res The stack entry to store the result in.
							 * \param RunningState The current SIMD state.
							 */
		template<class A, class B>
		void				OpLOR(A& a, B&b, CqVMStackEntry& Comp, CqVMStackEntry& Res, CqBitVector& RunningState)
													{
														TqInt i=MAX(MAX(Size(),Comp.Size()),Res.Size())-1;
														TqBool __fVarying=i>0;
														for(; i>=0; i--)
															if(!__fVarying || RunningState.Value(i))
																Res.SetValue(i,Value(a,i)||Comp.Value(b,i));
													}
							/** Templatised negation operator.
							 * The template classes decide the cast used, there must be an appropriate operator between the two types.
							 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
							 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
							 * \param Comp The stack entry to use as the second operand.
							 * \param Res The stack entry to store the result in.
							 * \param RunningState The current SIMD state.
							 */
		template<class A>
		void				OpNEG(A& a, CqVMStackEntry& Res, CqBitVector& RunningState)
													{
														TqInt i=MAX(Size(),Res.Size())-1;
														TqBool __fVarying=i>0;
														for(; i>=0; i--)
															if(!__fVarying || RunningState.Value(i))
																Res.SetValue(i,-Value(a,i));
													}
							/** Templatised cast operator, cast the current stack entry to the spcified type.
							 * The template classes decide the cast used, there must be an appropriate operator between the two types.
							 * \param a The type of the first operand, used to determine templateisation, needed by VC++..
							 * \param b The type of the second operand, used to determine templateisation, needed by VC++..
							 * \param Comp The stack entry to use as the second operand.
							 * \param Res The stack entry to store the result in.
							 * \param RunningState The current SIMD state.
							 */
		template<class A, class B>
		void				OpCAST(A& a, B& b, CqVMStackEntry& Res, CqBitVector& RunningState)
													{
														TqInt i=MAX(Size(),Res.Size())-1;
														TqBool __fVarying=i>0;
														for(; i>=0; i--)
															if(!__fVarying || RunningState.Value(i))
																Res.SetValue(i,static_cast<B>(Value(a,i)));
													}
							/** Templatised cast three operands to a single triple type (vector/normal/color etc.) and store the result in this stack entry
							 * \param z The type to combine the float values into.
							 * \param a Float first operand 
							 * \param b Float second operand 
							 * \param c Float third operand 
							 * \param RunningState The current SIMD state.
							 */
		template<class A>
		void				OpTRIPLE(A& z, CqVMStackEntry& a, CqVMStackEntry& b, CqVMStackEntry& c, CqBitVector& RunningState)
													{
														TqInt i=MAX(MAX(a.Size(),b.Size()),c.Size())-1;
														TqBool __fVarying=i>0;
														for(; i>=0; i--)
															if(!__fVarying || RunningState.Value(i))
																SetValue(i,A(a.Value(temp_float,i),b.Value(temp_float,i),c.Value(temp_float,i)));
													}
							/** Templatised cast sixteen operands to a single matrix type and store the result in this stack entry
							 * The parameters a-p are the float values to combine.
							 * \param z The type to combine the float values into (currenlty only matrix supported).
							 * \param RunningState The current SIMD state.
							 */
		template<class A>
		void				OpHEXTUPLE(A& z, CqVMStackEntry& a, CqVMStackEntry& b, CqVMStackEntry& c, CqVMStackEntry& d,
											 CqVMStackEntry& e, CqVMStackEntry& f, CqVMStackEntry& g, CqVMStackEntry& h,
											 CqVMStackEntry& i, CqVMStackEntry& j, CqVMStackEntry& k, CqVMStackEntry& l,
											 CqVMStackEntry& m, CqVMStackEntry& n, CqVMStackEntry& o, CqVMStackEntry& p, CqBitVector& RunningState)
													{
														TqInt ii=MAX(MAX(a.Size(),b.Size()),c.Size())-1;
														TqBool __fVarying=ii>0;
														for(; ii>=0; ii--)
														{
															if(!__fVarying || RunningState.Value(ii))
															{
																A tt(a.Value(temp_float,ii),b.Value(temp_float,ii),c.Value(temp_float,ii),d.Value(temp_float,ii),
																	 e.Value(temp_float,ii),f.Value(temp_float,ii),g.Value(temp_float,ii),h.Value(temp_float,ii),
																	 i.Value(temp_float,ii),j.Value(temp_float,ii),k.Value(temp_float,ii),l.Value(temp_float,ii),
																	 m.Value(temp_float,ii),n.Value(temp_float,ii),o.Value(temp_float,ii),p.Value(temp_float,ii));
																SetValue(ii,tt);
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
		template<class A>
		void				OpCOMP(A& z, int index, CqVMStackEntry& Res, CqBitVector& RunningState)
													{
														TqInt i=MAX(Size(),Res.Size())-1;
														TqBool __fVarying=i>0;
														for(; i>=0; i--)
															if(!__fVarying || RunningState.Value(i))
																Res.SetValue(i,Value(z,i)[index]);
													}
							/** Templatised component access operator.
							 * The template classes decide the cast used, there must be an appropriate operator between the two types.
							 * \param z The type to cast this stackentry to.
							 * \param index Integer type stackentry index. 
							 * \param Res The stack entry to store the result in.
							 * \param RunningState The current SIMD state.
							 */
		template<class A>
		void				OpCOMP(A& z, CqVMStackEntry& index, CqVMStackEntry& Res, CqBitVector& RunningState)
													{
														TqInt i=MAX(MAX(Size(),Res.Size()),index.Size())-1;
														TqBool __fVarying=i>0;
														for(; i>=0; i--)
															if(!__fVarying || RunningState.Value(i))
																Res.SetValue(i,Value(z,i)[index.Value(temp_float,i)]);
													}
							/** Templatised component set operator.
							 * \param z The type to cast this to.
							 * \param index Integer index. 
							 * \param a Float type stackentry to set the index to.
							 * \param RunningState The current SIMD state.
							 */
		template<class A>
		void				OpSETCOMP(A& z, int index, CqVMStackEntry& a, CqBitVector& RunningState)
													{
														A temp;
														TqInt i=MAX(Size(),a.Size())-1;
														TqBool __fVarying=i>0;
														for(; i>=0; i--)
														{
															if(!__fVarying || RunningState.Value(i))
															{
																temp=Value(z,i);
																temp[index]=a.Value(temp_float,i);
																SetValue(i,temp);
															}
														}
													}
							/** Templatised component set operator.
							 * \param z The type to cast this to.
							 * \param index Integer type stackentry index. 
							 * \param a Float type stackentry to set the index to.
							 * \param RunningState The current SIMD state.
							 */
		template<class A>
		void				OpSETCOMP(A& z, CqVMStackEntry& index, CqVMStackEntry& a, CqBitVector& RunningState)
													{
														A temp;
														TqInt i=MAX(MAX(Size(),a.Size()),index.Size())-1;
														TqBool __fVarying=i>0;
														for(; i>=0; i--)
														{
															if(!__fVarying || RunningState.Value(i))
															{
																temp=Value(z,i);
																temp[index.Value(temp_float,i)]=a.Value(temp_float,i);
																SetValue(i,temp);
															}
														}
													}

							/** Special case matrix component access.
							 * \param r Integer type stackentry row index.
							 * \param c Integer type stackentry column index.
							 * \param Res The stack entry to store the result in.
							 * \param RunningState The current SIMD state.
							 */
		void				OpCOMPM(CqVMStackEntry& r, CqVMStackEntry& c, CqVMStackEntry& Res, CqBitVector& RunningState)
													{
														static CqMatrix m;
														static TqFloat temp_float;
														TqInt i=MAX(Size(),Res.Size())-1;
														TqBool __fVarying=i>0;
														for(; i>=0; i--)
															if(!__fVarying || RunningState.Value(i))
																Res.SetValue(i,Value(m,i)[static_cast<TqInt>(c.Value(temp_float,i))][static_cast<TqInt>(c.Value(temp_float,i))]);
													}

							/** Special case matrix component access.
							 * \param r Integer type stackentry row index.
							 * \param c Integer type stackentry column index.
							 * \param v Float type stackentry value to set index to.
							 * \param RunningState The current SIMD state.
							 */
		void				OpSETCOMPM(CqVMStackEntry& r, CqVMStackEntry& c, CqVMStackEntry& v, CqBitVector& RunningState)
													{
														static CqMatrix m;
														static TqFloat temp_float;
														TqInt i=MAX(Size(),v.Size())-1;
														TqBool __fVarying=i>0;
														for(; i>=0; i--)
														{	
															if(!__fVarying || RunningState.Value(i))
															{
																m=Value(m,i);
																m[static_cast<TqInt>(r.Value(temp_float,i))][static_cast<TqInt>(c.Value(temp_float,i))]=v.Value(temp_float,i);
																SetValue(i,m);
															}
														}
													}
	
	private:
		std::vector<SqVMStackEntry/*,CqVMStackEntryAllocator*/>	m_aValues;	///< Array of SqVMStackEntry storage stuctures.
		SqVMStackEntry		m_Value;		///< Single value in the case of a uniforn stack entry.
		CqShaderVariable*	m_pVarRef;		///< Pointer to a referenced variable if a variable stack entry.
		TqInt				m_Size;			///< Size of the SIMD data.
};



//----------------------------------------------------------------------
/** \class CqShaderStack
 * Class handling the shader execution stack.
 */

class _qShareC CqShaderStack
{
	public:
	_qShareM 	CqShaderStack()	: m_iTop(0)
								{
									m_Stack.resize(48);
								}
	virtual _qShareM 	~CqShaderStack(){
									m_Stack.clear();
								}	

						/** Push a new stack entry onto the stack.
						 */
				void	Push(const CqVMStackEntry& E)
								{
									if(m_iTop>=m_Stack.size())
									{
										m_Stack.push_back(E);
										m_iTop++;
									}
									else
										m_Stack[m_iTop++]=E;
								}
						/** Push a new float onto the stack.
						 */
				void	Push(const TqFloat f)
								{
									CqVMStackEntry s;
									s.SetSize(1);
									s=f;
									Push(s);
								}
						/** Push a new boolean onto the stack.
						 */
				void	Push(const TqBool b)
								{
									CqVMStackEntry s;
									s.SetSize(1);
									s=b;
									Push(s);
								}
						/** Push a new 3D vector onto the stack.
						 */
				void	Push(const CqVector3D& v)
								{
									CqVMStackEntry s;
									s.SetSize(1);
									s=v;
									Push(s);
								}
						/** Push a new color onto the stack, passed as 3 floats.
						 */
				void	Push(const TqFloat r, TqFloat g, TqFloat b)
								{
									CqVMStackEntry s;
									s.SetSize(1);
									s=CqColor(r,g,b);
									Push(s);
								}
						/** Push a new color onto the stack.
						 */
				void	Push(const CqColor& c)
								{
									CqVMStackEntry s;
									s.SetSize(1);
									s=c;
									Push(s);
								}
						/** Push a new string onto the stack.
						 */
				void	Push(const CqString& str)
								{
									CqVMStackEntry s;
									s.SetSize(1);
									s=str;
									Push(s);
								}
						/** Push a new string, passed as a character onto the stack.
						 */
				void	Push(const char* ps)
								{
									CqVMStackEntry s;
									s.SetSize(1);
									s=ps;
									Push(s);
								}
						/** Push a new matrix onto the stack.
						 */
				void	Push(const CqMatrix& mat)
								{
									CqVMStackEntry s;
									s.SetSize(1);
									s=mat;
									Push(s);
								}
						/** Push a new shader variable reference onto the stack.
						 */
				void	Push(CqShaderVariable* pv)
								{
									CqVMStackEntry s;
									s=pv;
									Push(s);
								}
						/** Pop the top stack entry.
						 * \param f Boolean value to update if this is varying. If not varying, leaves f unaltered.
						 * \return Reference to the top stack entry.
						 */
				CqVMStackEntry& Pop(TqBool& f)	
								{
									CqVMStackEntry& val=m_Stack[--m_iTop];
									f=val.Size()>1||f;
									return(val);
								}
						/** Duplicate the top stack entry.
						 */
				void	Dup()	{
									CqVMStackEntry& s=m_Stack[m_iTop];
									Push(s);
								}
						/** Drop the top stack entry.
						 */
				void	Drop()	{
									m_iTop--;
								}
	protected:
			std::vector<CqVMStackEntry>		m_Stack;		///< Array of stacke entries.
			TqInt							m_iTop;			///< Index of the top entry.
};


//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif	// !SHADERSTACK_H_INCLUDED
