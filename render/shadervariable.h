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
		\brief Declares the classes for supporting shader variables on micropolygrids.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef SHADERVARIABLE_H_INCLUDED
#define SHADERVARIABLE_H_INCLUDED 1

#include	<vector>
#include	<iostream>

#include	"bitvector.h"
#include	"sstring.h"
#include	"stats.h"

#include	"specific.h"	// Needed for namespace macros.

START_NAMESPACE(Aqsis)

//----------------------------------------------------------------------
/** \enum EqVariableType
 * Shader variable type identifier.
 * \attention Any change to this MUST be mirrored in the type identifier and name string tables.
 */

enum EqVariableType
{
	Type_Nil=0,

	Type_Float=1,
	Type_Integer,
	Type_Point,
	Type_String,
	Type_Color,
	Type_Triple,
	Type_hPoint,
	Type_Normal,
	Type_Vector,
	Type_Void,
	Type_Matrix,
	Type_HexTuple,
	Type_Last,

	Type_Uniform= 0x8000,
	Type_Varying= 0x4000,
	Type_Vertex=  0x2000,
	
	Type_Drop=    0x0800,
	Type_Variable=0x0400,
	Type_Param=   0x0200,
	Type_Array=	  0x0100,

	Type_VaryingFloat=Type_Float|Type_Varying,
	Type_VaryingInteger=Type_Integer|Type_Varying,
	Type_VaryingPoint=Type_Point|Type_Varying,
	Type_VaryingString=Type_String|Type_Varying,
	Type_VaryingColor=Type_Color|Type_Varying,
	Type_VaryinghPoint=Type_hPoint|Type_Varying,
	Type_VaryingNormal=Type_Normal|Type_Varying,
	Type_VaryingVector=Type_Vector|Type_Varying,
	Type_VaryingMatrix=Type_Matrix|Type_Varying,

	Type_UniformFloat=Type_Float|Type_Uniform,
	Type_UniformInteger=Type_Integer|Type_Uniform,
	Type_UniformPoint=Type_Point|Type_Uniform,
	Type_UniformString=Type_String|Type_Uniform,
	Type_UniformColor=Type_Color|Type_Uniform,
	Type_UniformhPoint=Type_hPoint|Type_Uniform,
	Type_UniformNormal=Type_Normal|Type_Uniform,
	Type_UniformVector=Type_Vector|Type_Uniform,
	Type_UniformMatrix=Type_Matrix|Type_Uniform,

	Type_VertexFloat=Type_Float|Type_Vertex,
	Type_VertexInteger=Type_Integer|Type_Vertex,
	Type_VertexPoint=Type_Point|Type_Vertex,
	Type_VertexString=Type_String|Type_Vertex,
	Type_VertexColor=Type_Color|Type_Vertex,
	Type_VertexhPoint=Type_hPoint|Type_Vertex,
	Type_VertexNormal=Type_hPoint|Type_Vertex,
	Type_VertexVector=Type_hPoint|Type_Vertex,
	Type_VertexMatrix=Type_Matrix|Type_Vertex,

	Type_UniformFloatVariable=Type_Float|Type_Variable|Type_Uniform,
	Type_UniformPointVariable=Type_Point|Type_Variable|Type_Uniform,
	Type_UniformStringVariable=Type_String|Type_Variable|Type_Uniform,
	Type_UniformColorVariable=Type_Color|Type_Variable|Type_Uniform,
	Type_UniformNormalVariable=Type_Normal|Type_Variable|Type_Uniform,
	Type_UniformVectorVariable=Type_Vector|Type_Variable|Type_Uniform,
	Type_UniformMatrixVariable=Type_Matrix|Type_Variable|Type_Uniform,

	Type_VaryingFloatVariable=Type_Float|Type_Variable|Type_Varying,
	Type_VaryingPointVariable=Type_Point|Type_Variable|Type_Varying,
	Type_VaryingStringVariable=Type_String|Type_Variable|Type_Varying,
	Type_VaryingColorVariable=Type_Color|Type_Variable|Type_Varying,
	Type_VaryingNormalVariable=Type_Normal|Type_Variable|Type_Varying,
	Type_VaryingVectorVariable=Type_Vector|Type_Variable|Type_Varying,
	Type_VaryingMatrixVariable=Type_Matrix|Type_Variable|Type_Varying,

	Type_VertexFloatVariable=Type_Float|Type_Variable|Type_Vertex,
	Type_VertexPointVariable=Type_Point|Type_Variable|Type_Vertex,
	Type_VertexStringVariable=Type_String|Type_Variable|Type_Vertex,
	Type_VertexColorVariable=Type_Color|Type_Variable|Type_Vertex,
	Type_VertexNormalVariable=Type_Normal|Type_Variable|Type_Vertex,
	Type_VertexVectorVariable=Type_Vector|Type_Variable|Type_Vertex,
	Type_VertexMatrixVariable=Type_Matrix|Type_Variable|Type_Vertex,

	Type_VaryingFloatArray=Type_Float|Type_Varying|Type_Array,
	Type_VaryingIntegerArray=Type_Integer|Type_Varying|Type_Array,
	Type_VaryingPointArray=Type_Point|Type_Varying|Type_Array,
	Type_VaryingStringArray=Type_String|Type_Varying|Type_Array,
	Type_VaryingColorArray=Type_Color|Type_Varying|Type_Array,
	Type_VaryinghPointArray=Type_hPoint|Type_Varying|Type_Array,
	Type_VaryingNormalArray=Type_Normal|Type_Varying|Type_Array,
	Type_VaryingVectorArray=Type_Vector|Type_Varying|Type_Array,
	Type_VaryingMatrixArray=Type_Matrix|Type_Varying|Type_Array,

	Type_UniformFloatArray=Type_Float|Type_Uniform|Type_Array,
	Type_UniformIntegerArray=Type_Integer|Type_Uniform|Type_Array,
	Type_UniformPointArray=Type_Point|Type_Uniform|Type_Array,
	Type_UniformStringArray=Type_String|Type_Uniform|Type_Array,
	Type_UniformColorArray=Type_Color|Type_Uniform|Type_Array,
	Type_UniformhPointArray=Type_hPoint|Type_Uniform|Type_Array,
	Type_UniformNormalArray=Type_Normal|Type_Uniform|Type_Array,
	Type_UniformVectorArray=Type_Vector|Type_Uniform|Type_Array,
	Type_UniformMatrixArray=Type_Matrix|Type_Uniform|Type_Array,

	Type_VertexFloatArray=Type_Float|Type_Vertex|Type_Array,
	Type_VertexIntegerArray=Type_Integer|Type_Vertex|Type_Array,
	Type_VertexPointArray=Type_Point|Type_Vertex|Type_Array,
	Type_VertexStringArray=Type_String|Type_Vertex|Type_Array,
	Type_VertexColorArray=Type_Color|Type_Vertex|Type_Array,
	Type_VertexhPointArray=Type_hPoint|Type_Vertex|Type_Array,
	Type_VertexNormalArray=Type_hPoint|Type_Vertex|Type_Array,
	Type_VertexVectorArray=Type_hPoint|Type_Vertex|Type_Array,
	Type_VertexMatrixArray=Type_Matrix|Type_Vertex|Type_Array,

	Type_Mask=0x00FF,
	Storage_Mask=0xF000,
	Usage_Mask=0x0F00,
	
	Storage_Shift=12,
	Usage_Shift=8,
};

_qShareM	extern char*	gVariableTypeIdentifiers[];
_qShareM	extern TqInt	gcVariableTypeIdentifiers;
_qShareM	extern char*	gVariableStorageNames[];
_qShareM	extern TqInt	gcVariableStorageNames;
_qShareM	extern char*	gVariableUsageNames[];
_qShareM	extern TqInt	gcVariableUsageNames;
_qShareM	extern char*	gVariableTypeNames[];
_qShareM	extern TqInt	gcVariableTypeNames;

_qShare	std::ostream &operator<<(std::ostream &Stream, EqVariableType t);


class CqShaderExecEnv;
class CqVMStackEntry;

//----------------------------------------------------------------------
/** \class CqShaderVariable
 * Abstract base class from which all shaders variables must be defined.
 */

class CqShaderVariable
{
	public:
					CqShaderVariable();
					CqShaderVariable(const char* strName);
					CqShaderVariable(const CqShaderVariable& From);
	virtual			~CqShaderVariable();

					/** Get the name of this variable.
					 * \return Read only reference to a CqString class.
					 */
			const CqString&	strName()		{return(m_strName);}
	
					/** Pure virtual, prepare the variable for the SIMD size.
					 * \param uGridRes The size of the SIMD grid in u.
					 * \param vGridRes The size of the SIMD grid in v.
					 * \param index A Reference to a SIMD index used to obtain the 'current', in terms of SIMD execution, value.
					 */
	virtual	void	Initialise(const TqInt uGridRes, const TqInt vGridRes, TqInt& index)=0;
					/** Set the 'current', in terms of SIMD execution, value.
					 * \param Val The new value.
					 */
	virtual	void	SetValue(CqVMStackEntry& Val)=0;
					/** Set a specified SIMD index value.
					 * \param index Integer SIMD index.
					 * \param Val The new value.
					 */
	virtual	void	SetValue(TqInt index, CqVMStackEntry& Val)=0;
					/** Set the all SIMD data ased on a state vector, only indexes whose bit is set are modified.
					 * \param Val The stack entry to assign.
					 * \param State The bit vector to control modification.
					 */
	virtual	void	SetValue(CqVMStackEntry& Val, CqBitVector& State)=0;
					/** Get an indexed SIMD data value.
					 * \param index Integer SIMD index.
					 * \param Val A reference to a stackentry to store the value.
					 */
	virtual	void	GetValue(TqInt index, CqVMStackEntry& Val)const =0;
					/** Get the variable type.
					 * \return Type as a member of EqVariableType.
					 */
	virtual	EqVariableType	Type() const=0;
					/** Create a duplicate of this variable.
					 * \return A CqShaderVariable pointer.
					 */
	virtual	CqShaderVariable* Clone() const=0;
					/** Get the SIMD size of this variable.
					 * \return Integer SIMD data size.
					 */
	virtual TqInt	Size() const=0;

	protected:
			CqString	m_strName;		///< Name of this variable.
};


//----------------------------------------------------------------------
/** \class CqShaderVariableArray
 * Array of variable pointers.
 */

class CqShaderVariableArray: public CqShaderVariable
{
	public:
					/** Default constructor.
					 * \param name Character pointer to the name to use.
					 * \param Count, the size of the array.
					 */
					CqShaderVariableArray(const char* name, TqInt Count) : CqShaderVariable(name)	
												{
													assert(Count>0);
													m_aVariables.resize(Count);
												}
					/** Copy constructor.
					 */
					CqShaderVariableArray(const CqShaderVariableArray& From) : CqShaderVariable(From.m_strName.data())
												{
													m_aVariables.resize(From.m_aVariables.size());
													TqInt i;
													for(i=0; i<From.m_aVariables.size(); i++)
														m_aVariables[i]=From.m_aVariables[i]->Clone();
												}
	virtual			~CqShaderVariableArray()	{}

	// Overridded from CqShaderVariable.
	virtual	void	Initialise(const TqInt uGridRes, const TqInt vGridRes, TqInt& index)
												{
													TqInt i;
													for(i=0; i<m_aVariables.size(); i++)
														m_aVariables[i]->Initialise(uGridRes,vGridRes,index);
												}
	virtual	void	SetValue(CqVMStackEntry& Val)
												{
													m_aVariables[0]->SetValue(Val);
												}
	virtual	void	SetValue(TqInt index, CqVMStackEntry& Val)
												{
													m_aVariables[0]->SetValue(index, Val);
												}
	virtual	void	SetValue(CqVMStackEntry& Val, CqBitVector& State)
												{
													m_aVariables[0]->SetValue(Val,State);
												}
	virtual	void	GetValue(TqInt index, CqVMStackEntry& Val)const
												{
													m_aVariables[0]->GetValue(index, Val);
												}
	virtual	EqVariableType	Type() const		{return(static_cast<EqVariableType>(m_aVariables[0]->Type()|Type_Array));}
	virtual	CqShaderVariable* Clone() const;
	virtual TqInt			Size() const		{return(m_aVariables[0]->Size());}

					/** Get a reference to the variable array.
					 */
			std::vector<CqShaderVariable*>& aVariables(){return(m_aVariables);}
					/** Array index access to the values in the array.
					 * \param index Integer index intot he array.
					 */
			CqShaderVariable* operator[](TqInt index)	{return(m_aVariables[index]);}
					/** Get the length of the variable array.
					 * \return Integer array length.
					 */
			TqInt	ArrayLength() const	{return(m_aVariables.size());}

	private:
			std::vector<CqShaderVariable*>	m_aVariables;		///< Array of pointers to variables.
};


//----------------------------------------------------------------------
/** \class CqShaderVariableTyped
 * Templatised base class for shader variables of a specific type.
 */

template<class R>
class CqShaderVariableTyped : public CqShaderVariable
{
	public:
					CqShaderVariableTyped()	{}
					CqShaderVariableTyped(const char* strName) : CqShaderVariable(strName)
										{}
					CqShaderVariableTyped(const CqShaderVariable& From) : CqShaderVariable(From)
										{}
	virtual			~CqShaderVariableTyped()	{}

					/** Pure virtual set value.
					 * \param Val The new value for the variable, promotes through fuplication if required.
					 */
	virtual	void	SetValue(const R& Val)=0;
					/** Pure virtual set value.
					 * \param Val The new value for the variable, promotes through fuplication if required.
					 * \param State Bit vector contolling the setting of SIMD values, only indexes with the appropriate bit set in the state will be modified.
					 */
	virtual	void	SetValue(const R& Val, CqBitVector& State)=0;
					/** Get the 'current', in terms of SIMD execution, value of this variable.
					 */
	virtual	R		GetValue()const =0;
					/** Get the 'current', in terms of SIMD execution, value of this variable.
					 */
	virtual			operator R&()=0;
					/** Set the 'current', in terms of SIMD execution, value of this variable.
					 * \param v The new value to use.
					 */
	virtual	void	operator=(const R& v)=0;
					/** Indexed access to the SIMD data on this variable.
					 * \param i Integer SIMD index.
					 * \return Reference to the data at that index, or at aero if uniform.
					 */
	virtual	const	R&		operator[](const TqInt i) const=0;
					/** Indexed access to the SIMD data on this variable.
					 * \param i Integer SIMD index.
					 * \return Reference to the data at that index, or at aero if uniform.
					 */
	virtual			R&		operator[](const TqInt i)=0;

	private:
};


//----------------------------------------------------------------------
/** \class CqShaderVariableUniform
 * Uniform variable templatised by type.
 */

template<const EqVariableType T,class R>
class CqShaderVariableUniform : public CqShaderVariableTyped<R>
{
	public:
					CqShaderVariableUniform(const char* strName) : CqShaderVariableTyped<R>(strName)	
												{}
					CqShaderVariableUniform(const char* strName, const R& val) :	CqShaderVariableTyped<R>(strName),
										m_Value(val)
												{}
					CqShaderVariableUniform(const CqShaderVariableUniform<T,R>& val) :
										CqShaderVariableTyped<R>(val),
										m_Value(val.m_Value)
												{}
	virtual			~CqShaderVariableUniform()	{}

	virtual	void	Initialise(const TqInt uGridRes, const TqInt vGridRes, TqInt& index)
												{}
	virtual	void	SetValue(CqVMStackEntry& Val)
												{m_Value=Val.Value(m_Value);}
	virtual	void	SetValue(TqInt index, CqVMStackEntry& Val)
												{m_Value=Val.Value(m_Value,index);}
	virtual	void	SetValue(CqVMStackEntry& Val, CqBitVector& State)
												{SetValue(Val);}
	virtual	void	GetValue(TqInt index, CqVMStackEntry& Val) const
												{
													Val.SetValue(index,m_Value);
												}
	virtual	void	SetValue(const R& Val)		{m_Value=Val;}
	virtual	void	SetValue(const R& Val, CqBitVector& State)
												{
													m_Value=Val;
												}
	virtual	R		GetValue()const				{return(m_Value);}
	virtual	EqVariableType	Type() const		{return(static_cast<EqVariableType>(T|Type_Uniform|Type_Variable));}
	virtual	CqShaderVariable* Clone() const		{return(new CqShaderVariableUniform<T,R>(*this));}
	virtual TqInt	Size() const				{return(1);}

	virtual	void	operator=(const CqShaderVariableTyped<R>* From)
												{
													m_Value=(*From)[0];	
												}
					operator R&()				{return(m_Value);}
			void	operator=(const R& v)		{m_Value=v;}
	const	R&		operator[](const TqInt i) const
												{return(m_Value);}
			R&		operator[](const TqInt i)	{return(m_Value);}

	private:
			R				m_Value;	///< Simgle uniform value of the appropriate type.
};



//----------------------------------------------------------------------
/** \class CqShaderVariableVarying
 * Varying variable templatised by type.
 */

template<const EqVariableType T,class R>
class CqShaderVariableVarying : public CqShaderVariableTyped<R>
{
	public:
					CqShaderVariableVarying(const char* strName) : CqShaderVariableTyped<R>(strName), m_Size(1)
												{m_aValue.resize(1);}
					CqShaderVariableVarying(const char* strName, const R& val) : CqShaderVariableTyped<R>(strName), m_Size(1)
												{
													m_aValue.resize(1);
													m_aValue[0]=val;													
												}
					CqShaderVariableVarying(const CqShaderVariableVarying<T,R>& val) : CqShaderVariableTyped<R>(val)
												{
													m_aValue.resize(val.m_aValue.size());
													m_aValue.assign(val.m_aValue.begin(), val.m_aValue.begin());
													m_pIndex=val.m_pIndex;
													m_Size=val.m_Size;
												}
	virtual			~CqShaderVariableVarying()	{}

	virtual	void	Initialise(const TqInt uGridRes, const TqInt vGridRes, TqInt& index)
												{
													m_Size=(uGridRes+1)*(vGridRes+1);
													if(m_aValue.size()<m_Size)
														m_aValue.resize(m_Size);
													m_pIndex=&index;
												}

	virtual	void	SetValue(CqVMStackEntry& Val){
													// Note we can do this because uniform stack entries just return 
													// the single value irrespective of the array index.
													TqInt i;
													for(i=Size()-1; i>=0; i--)
														m_aValue[i]=Val.Value(m_temp_R,i);
												}
	virtual	void	SetValue(TqInt index, CqVMStackEntry& Val)	
												{
													// Note we can do this because uniform stack entries just return 
													// the single value irrespective of the array index.
													m_aValue[index]=Val.Value(m_temp_R,index);
												}
	virtual	void	SetValue(CqVMStackEntry& Val, CqBitVector& State)
												{
													// Note we can do this because uniform stack entries just return 
													// the single value irrespective of the array index.
													TqInt i;
													for(i=Size()-1; i>=0; i--)
													{
														if(State.Value(i))
															m_aValue[i]=Val.Value(m_temp_R,i);
													}
												}
	virtual	void	GetValue(TqInt index, CqVMStackEntry& Val) const
												{
													Val.SetValue(index, m_aValue[index]);
												}
	virtual	void	SetValue(const R& Val)		{
													TqInt i;
													for(i=Size()-1; i>=0; i--)
														m_aValue[i]=Val;
												}
	virtual	void	SetValue(const R& Val, CqBitVector& State)
												{
													TqInt i;
													for(i=Size()-1; i>=0; i--)
													{
														if(State.Value(i))
															m_aValue[i]=Val;
													}
												}
	virtual	void	SetValue(const CqShaderVariableTyped<R>& From)
												{
													TqInt i;
													for(i=Size()-1; i>=0; i--)
														m_aValue[i]=From[i];	
												}
	virtual	R		GetValue()const				{return(m_aValue[*m_pIndex]);}
	virtual	EqVariableType	Type() const		{return(static_cast<EqVariableType>(T|Type_Varying|Type_Variable));}
	virtual	CqShaderVariable* Clone() const		{return(new CqShaderVariableVarying<T,R>(*this));}
	virtual TqInt	Size() const				{return(m_Size);}

	virtual	void	operator=(const CqShaderVariableTyped<R>& From)
												{
													TqInt i;
													for(i=m_aValue.size()-1; i>=0; i--)
														m_aValue[i]=From[i];	
												}
					operator R&()				{return(m_aValue[*m_pIndex]);}
			void	operator=(const R& v)		{m_aValue[*m_pIndex]=v;}
	const	R&		operator[](const TqInt i)	const
												{
													assert(i<m_Size);
													return(m_aValue[i]);
												}
			R&		operator[](const TqInt i)	{
													assert(i<m_Size);
													return(m_aValue[i]);
												}

	private:
			std::vector<R>	m_aValue;		///< Array of values of the appropriate type.
			TqInt*			m_pIndex;		///< Pointer to the SIMD index.
			TqInt			m_Size;			///< Integer size of the SIMD data.
			R				m_temp_R;		///< Temp value to use in template functions, problem with VC++.
};


//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif	// !SHADERVARIABLE_H_INCLUDED
