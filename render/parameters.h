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
		\brief Declares the classes and support structures for handling parameters attached to GPrims.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef PARAMETERS_H_INCLUDED
#define PARAMETERS_H_INCLUDED 1

#include	<vector>

#include	"specific.h"	// Needed for namespace macros.
#include	"shadervariable.h"
#include	"bilinear.h"

START_NAMESPACE(Aqsis)


//----------------------------------------------------------------------
/** \class CqParameter
 * Class storing a parameter with a name and value.
 */

class CqParameter
{
	public:
						CqParameter(const char* strName, TqInt Count=1);
						CqParameter(const CqParameter& From);
	virtual				~CqParameter();

						/** Pure virtual, duplicate function.
						 * \return A pointer to a new parameter with the same name and value.
						 */
	virtual	CqParameter* Clone() const=0;
						/** Pure virtual, get value type.
						 * \return Type as an EqVariableType.
						 */
	virtual	EqVariableType	Type() const=0;
						/** Pure virtual, set value size, not array, but varying/vertex size.
						 */
	virtual	void		SetSize(TqInt size)=0;
						/** Pure virtual, get value size, not array, but varying/vertex size.
						 */
	virtual	TqInt		Size() const=0;
						/** Pure virtual, clear value contents.
						 */
	virtual	void		Clear()=0;

	/** \attention
	 * The subdivide functions perform common splitting and interpolation on primitive variables
	 * they are only of use if the variable is a bilinear quad (the most common kind)
	 * any other type of splitting or interpolation must be performed by the surface which 
	 * instantiates special types (i.e. polygons).
	 */
						/** Subdivide the value in the u direction, place one half in this and the other in the specified parameter.
						 * \param pResult Pointer to storage for the result.
						 */
	virtual void		uSubdivide(CqParameter* pResult)	{}
						/** Subdivide the value in the v direction, place one half in this and the other in the specified parameter.
						 * \param pResult Pointer to storage for the result.
						 */
	virtual void		vSubdivide(CqParameter* pResult)	{}
						/** Pure virtual, dice the value into a grid using linear interpolation.
						 * \param u Integer dice count for the u direction.
						 * \param v Integer dice count for the v direction.
						 * \param pResult Pointer to storage for the result.
						 */
	virtual	void		BilinearDice(TqInt u, TqInt v, CqShaderVariable* pResult)=0;

						/** Get a reference to the parameter name.
						 */
	const	CqString& strName() const		{return(m_strName);}
						/** Get the array size.
						 */
			TqInt		Count() const		{return(m_Count);}

	protected:
			CqString	m_strName;		///< String name of the parameter.
			TqInt		m_Count;		///< Array size of value.
};


//----------------------------------------------------------------------
/** \class CqParameterTyped
 * Parameter templatised by its value type.
 */

template<class T>
class CqParameterTyped : public CqParameter
{
	public:
						CqParameterTyped(const char* strName, TqInt Count=1) : 
											CqParameter(strName,Count)
											{}
						CqParameterTyped(const CqParameterTyped<T>& From) :
											CqParameter(From)
											{}
	virtual				~CqParameterTyped()	{}

						/** Get a pointer to the value (presumes uniform).
						 */
	virtual	const	T*	pValue() const=0;
						/** Get a pointer to the value (presumes uniform).
						 */
	virtual			T*	pValue() =0;
						/** Get a pointer to the value at the specified index, if uniform index is ignored.
						 */
	virtual	const	T*	pValue(const TqInt Index) const=0;
						/** Get a pointer to the value at the specified index, if uniform index is ignored.
						 */
	virtual			T*	pValue(const TqInt Index) =0;

	protected:
};


//----------------------------------------------------------------------
/** \class CqParameterTypedVarying
 * Parameter with a varying type, templatised by value type and type id.
 */

template<class T, EqVariableType I>
class CqParameterTypedVarying : public CqParameterTyped<T>
{
	public:
						CqParameterTypedVarying(const char* strName, TqInt Count=1) : 
											CqParameterTyped<T>(strName, Count)
											{
												m_aValues.resize(1);
											}
						CqParameterTypedVarying(const CqParameterTypedVarying<T,I>& From) :
											CqParameterTyped<T>(From)
											{
												*this=From;
											}
	virtual				~CqParameterTypedVarying()
											{}

	// Overrridden from CqParameter
	virtual	CqParameter* Clone()const	{return(new CqParameterTypedVarying<T,I>(*this));}
	virtual	EqVariableType	Type() const {return((EqVariableType)(I|Type_Varying));}
	virtual	void		SetSize(TqInt size)	{m_aValues.resize(size);}
	virtual	TqInt		Size() const		{return(m_aValues.size());}
	virtual	void		Clear()				{m_aValues.clear();}
	virtual void		uSubdivide(CqParameter* pResult)	
											{
												assert(pResult->Type()==Type());
												CqParameterTypedVarying<T,I>* pVS=static_cast<CqParameterTypedVarying<T,I>*>(pResult);
												// Check if a valid 4 point quad, do nothing if not.
												if(m_aValues.size()==4 && pVS->m_aValues.size()==4)
												{
													pValue(1)[0]=pVS->pValue(0)[0]=(pValue(0)[0]+pValue(1)[0])*0.5;
													pValue(3)[0]=pVS->pValue(2)[0]=(pValue(2)[0]+pValue(3)[0])*0.5;
												}
											}
	virtual void		vSubdivide(CqParameter* pResult)	
											{
												assert(pResult->Type()==Type());
												CqParameterTypedVarying<T,I>* pVS=static_cast<CqParameterTypedVarying<T,I>*>(pResult);
												// Check if a valid 4 point quad, do nothing if not.
												if(m_aValues.size()==4 && pVS->m_aValues.size()==4)
												{
													pValue(2)[0]=pVS->pValue(0)[0]=(pValue(0)[0]+pValue(2)[0])*0.5;
													pValue(3)[0]=pVS->pValue(1)[0]=(pValue(1)[0]+pValue(3)[0])*0.5;
												}
											}
	virtual	void		BilinearDice(TqInt u, TqInt v, CqShaderVariable* pResult);

	// Overridden from CqParameterTyped<T>
						
	virtual	const	T*	pValue() const		{return(&m_aValues[0]);}
	virtual			T*	pValue()			{return(&m_aValues[0]);}
	virtual	const	T*	pValue(const TqInt Index) const	{return(&m_aValues[Index]);}
	virtual			T*	pValue(const TqInt Index)		{return(&m_aValues[Index]);}


						/** Indexed access to values.
						 * \param Index Integer index into the varying value list.
						 */
	const	T&			operator[](const TqInt Index) const
											{return(m_aValues[Index]);}
						/** Indexed access to values.
						 * \param Index Integer index into the varying value list.
						 */
			T&			operator[](const TqInt Index)
											{return(m_aValues[Index]);}

						/** Assignment operator
						 */
		CqParameterTypedVarying<T,I>& operator=(const CqParameterTypedVarying<T,I>& From)
											{
												m_aValues.resize(From.m_aValues.size());
												TqInt j;
												for(j=0; j<m_aValues.size(); j++)
												{
													m_aValues[j]=From.m_aValues[j];
												}
												return(*this);
											}

						/** Static constructor, to allow type free parameter construction.
						 * \param strName Character pointer to new parameter name.
						 * \param Count Integer array size.
						 */
	static	CqParameter*	Create(const char* strName, TqInt Count=1)
											{return(new CqParameterTypedVarying<T,I>(strName, Count));}

	private:
		std::vector<T>	m_aValues;		///< Vector of values, one per varying index.
};


//----------------------------------------------------------------------
/** \class CqParameterTypedUniform
 * Parameter with a uniform type, templatised by value type and type id.
 */

template<class T, EqVariableType I>
class CqParameterTypedUniform : public CqParameterTyped<T>
{
	public:
						CqParameterTypedUniform(const char* strName, TqInt Count=1) : 
											CqParameterTyped<T>(strName, Count)
											{}
						CqParameterTypedUniform(const CqParameterTypedUniform<T,I>& From) :
											CqParameterTyped<T>(From)
											{
												m_Value=From.m_Value;
											}
	virtual				~CqParameterTypedUniform() {}

	virtual	CqParameter* Clone()const	{return(new CqParameterTypedUniform<T,I>(*this));}
	virtual	EqVariableType	Type() const {return((EqVariableType)(I|Type_Uniform));}
	virtual	void		SetSize(TqInt size)	{}
	virtual	TqInt		Size() const	{return(1);}
	virtual	void		Clear()			{}

	virtual void		uSubdivide(CqParameter* pResult)	{}
	virtual void		vSubdivide(CqParameter* pResult)	{}
	virtual	void		BilinearDice(TqInt u, TqInt v, CqShaderVariable* pResult)
										{
											// Just promote the uniform value to varying by duplication.
											assert((pResult->Type()&Type_Mask)==(Type()&Type_Mask));
											assert((pResult->Type()&Storage_Mask)==Type_Varying);
											CqShaderVariableVarying<I,T>* pVR=static_cast<CqShaderVariableVarying<I,T>*>(pResult);
											// Note it is assumed that the variable has been 
											// initialised to the correct size prior to calling.
											TqInt i;
											for(i=0; i<u*v; i++)
												(*pVR)[i]=m_Value;
										}


	// Overridden from CqParameterTyped<T>
	virtual	const	T*	pValue() const					{return(&m_Value);}
	virtual			T*	pValue()						{return(&m_Value);}
	virtual	const	T*	pValue(const TqInt Index) const	{return(&m_Value);}
	virtual			T*	pValue(const TqInt Index)		{return(&m_Value);}


						/** Assignment operator.
						 */
		CqParameterTypedUniform<T,I>& operator=(const CqParameterTypedUniform<T,I>& From)
											{
												m_Value=From.m_Value;
												return(*this);
											}

						/** Static constructor, to allow type free parameter construction.
						 * \param strName Character pointer to new parameter name.
						 * \param Count Integer array size.
						 */
	static	CqParameter*	Create(const char* strName, TqInt Count=1)
											{return(new CqParameterTypedUniform<T,I>(strName, Count));}
	private:
			T			m_Value;	///< Single uniform value.
};


//----------------------------------------------------------------------
/** \class CqParameterTypedVertex
 * Parameter with a vertex type, templatised by value type and type id.
 */

template<class T, EqVariableType I>
class CqParameterTypedVertex : public CqParameterTypedVarying<T,I>
{
	public:
						CqParameterTypedVertex(const char* strName, TqInt Count) : 
											CqParameterTypedVarying<T,I>(strName, Count)
											{}
						CqParameterTypedVertex(const CqParameterTypedVertex<T,I>& From) :
											CqParameterTypedVarying<T,I>(From)
											{}
	virtual				~CqParameterTypedVertex()
											{}

	virtual	CqParameter* Clone()const	{return(new CqParameterTypedVertex<T,I>(*this));}
	virtual	EqVariableType	Type() const {return((EqVariableType)(I|Type_Vertex));}

						/** Static constructor, to allow type free parameter construction.
						 * \param strName Character pointer to new parameter name.
						 * \param Count Integer array size.
						 */
	static	CqParameter*	Create(const char* strName, TqInt Count=1)
											{return(new CqParameterTypedVertex<T,I>(strName, Count));}

	private:
};


//----------------------------------------------------------------------
/** \class CqParameterTypedVaryingArray
 * Parameter with a varying array type, templatised by value type and type id.
 */

template<class T, EqVariableType I>
class CqParameterTypedVaryingArray : public CqParameterTyped<T>
{
	public:
						CqParameterTypedVaryingArray(const char* strName, TqInt Count=1) : 
											CqParameterTyped<T>(strName, Count)
											{
												m_aValues.resize(1);
												m_aValues[0].resize(Count);
											}
						CqParameterTypedVaryingArray(const CqParameterTypedVaryingArray<T,I>& From) :
											CqParameterTyped<T>(From)
											{
												*this=From;
											}
	virtual				~CqParameterTypedVaryingArray()
											{}

	virtual	CqParameter* Clone()const	{return(new CqParameterTypedVaryingArray<T,I>(*this));}
	virtual	EqVariableType	Type() const {return((EqVariableType)(I|Type_Varying));}
	virtual	void		SetSize(TqInt size)	{
												m_aValues.resize(size);
												TqInt j;
												for(j=0; j<size; j++)
													m_aValues[j].resize(m_Count);
											}
	virtual	TqInt		Size() const		{return(m_aValues[0].size());}
	virtual	void		Clear()				{m_aValues.clear();}
	virtual void		uSubdivide(CqParameter* pResult)	
											{
												assert(pResult->Type()==Type());
												CqParameterTypedVaryingArray<T,I>* pVS=static_cast<CqParameterTypedVaryingArray<T,I>*>(pResult);
												// Check if a valid 4 point quad, do nothing if not.
												if(m_aValues.size()==4 && pVS->m_aValues.size()==4)
												{
													pValue(1)[0]=pVS->pValue(0)[0]=(pValue(0)[0]+pValue(1)[0])*0.5;
													pValue(3)[0]=pVS->pValue(2)[0]=(pValue(2)[0]+pValue(3)[0])*0.5;
												}
											}
	virtual void		vSubdivide(CqParameter* pResult)	
											{
												assert(pResult->Type()==Type());
 												CqParameterTypedVaryingArray<T,I>* pVS=static_cast<CqParameterTypedVaryingArray<T,I>*>(pResult);
												// Check if a valid 4 point quad, do nothing if not.
												if(m_aValues.size()==4 && pVS->m_aValues.size()==4)
												{
													pValue(2)[0]=pVS->pValue(0)[0]=(pValue(0)[0]+pValue(2)[0])*0.5;
													pValue(3)[0]=pVS->pValue(1)[0]=(pValue(1)[0]+pValue(3)[0])*0.5;
												}
											}
	virtual	void		BilinearDice(TqInt u, TqInt v, CqShaderVariable* pResult);

	// Overridden from CqParameterTyped<T>
	virtual	const	T*	pValue() const					{return(&m_aValues[0][0]);}
	virtual			T*	pValue()						{return(&m_aValues[0][0]);}
	virtual	const	T*	pValue(const TqInt Index) const	{return(&m_aValues[Index][0]);}
	virtual			T*	pValue(const TqInt Index)		{return(&m_aValues[Index][0]);}


						/** Indexed access to array values.
						 * \param Index Integer index into the varying value list.
						 * \return A vector reference for the array of values at the specified varying index.
						 */
	const	std::vector<T>& operator[](const TqInt Index) const
											{return(m_aValues[Index]);}
						/** Indexed access to array values.
						 * \param Index Integer index into the varying value list.
						 * \return A vector reference for the array of values at the specified varying index.
						 */
			std::vector<T>& operator[](const TqInt Index)
											{return(m_aValues[Index]);}

						/** Assignment operator.
						 */
		CqParameterTypedVaryingArray<T,I>& operator=(const CqParameterTypedVaryingArray<T,I>& From)
											{
												m_aValues.resize(From.m_aValues.size());
												TqInt j;
												for(j=0; j<m_aValues.size(); j++)
												{
													m_aValues[j].resize(m_Count);
													TqInt i;
													for(i=0; i<m_Count; i++)
														m_aValues[j][i]=From.m_aValues[j][i];
												}
												return(*this);
											}

						/** Static constructor, to allow type free parameter construction.
						 * \param strName Character pointer to new parameter name.
						 * \param Count Integer array size.
						 */
	static	CqParameter*	Create(const char* strName, TqInt Count=1)
											{return(new CqParameterTypedVaryingArray<T,I>(strName, Count));}

	private:
		std::vector<std::vector<T> >	m_aValues;		///< Array of varying values.
};


//----------------------------------------------------------------------
/** \class CqParameterTypedUniformArray
 * Parameter with a uniform array type, templatised by value type and type id.
 */

template<class T, EqVariableType I>
class CqParameterTypedUniformArray : public CqParameterTyped<T>
{
	public:
						CqParameterTypedUniformArray(const char* strName, TqInt Count=1) : 
											CqParameterTyped<T>(strName, Count)
											{
												m_aValues.resize(Count);
											}
						CqParameterTypedUniformArray(const CqParameterTypedUniformArray<T,I>& From) :
											CqParameterTyped<T>(From)
											{
												m_aValues.resize(From.m_Count);
												TqInt i;
												for(i=0; i<From.m_Count; i++)
													m_aValues[i]=From.m_aValues[i];
											}
	virtual				~CqParameterTypedUniformArray() {}

	virtual	CqParameter* Clone()const	{return(new CqParameterTypedUniformArray<T,I>(*this));}
	virtual	EqVariableType	Type() const {return((EqVariableType)(I|Type_Uniform));}
	virtual	void		SetSize(TqInt size)	{}
	virtual	TqInt		Size() const		{return(1);}
	virtual	void		Clear()			{}

	virtual void		uSubdivide(CqParameter* pResult)	{}
	virtual void		vSubdivide(CqParameter* pResult)	{}
	virtual	void		BilinearDice(TqInt u, TqInt v, CqShaderVariable* pResult)
										{
											// Just promote the uniform value to varying by duplication.
											assert((pResult->Type()&Type_Mask)==(Type()&Type_Mask));
											assert((pResult->Type()&Storage_Mask)==Type_Varying);
											CqShaderVariableVarying<I,T>* pVR=static_cast<CqShaderVariableVarying<I,T>*>(pResult);
											// Note it is assumed that the variable has been 
											// initialised to the correct size prior to calling.
											TqInt i;
											for(i=0; i<u*v; i++)
												(*pVR)[i]=pValue(0)[0];
										}

	// Overridden from CqParameterTyped<T>
	virtual	const	T*	pValue() const					{return(&m_aValues[0]);}
	virtual			T*	pValue()						{return(&m_aValues[0]);}
	virtual	const	T*	pValue(const TqInt Index) const	{return(&m_aValues[Index]);}
	virtual			T*	pValue(const TqInt Index)		{return(&m_aValues[Index]);}

						/** Assignment operator.
						 */
		CqParameterTypedUniformArray<T,I>& operator=(const CqParameterTypedUniformArray<T,I>& From)
											{
												m_aValues.resize(From.m_aValues.size());
												TqInt i2=0;
												for(std::vector<T>::iterator i=From.m_aValues.being(); i!=From.m_aValues.end(); i++, i2++)
													m_aValues[i2]=(*i);
												return(*this);
											}

						/** Static constructor, to allow type free parameter construction.
						 * \param strName Character pointer to new parameter name.
						 * \param Count Integer array size.
						 */
	static	CqParameter*	Create(const char* strName, TqInt Count=1)
											{return(new CqParameterTypedUniformArray<T,I>(strName, Count));}
	private:
			std::vector<T>	m_aValues;	///< Array of uniform values.
};


//----------------------------------------------------------------------
/** \class CqParameterTypedVertexArray
 * Parameter with a vertex array type, templatised by value type and type id.
 */

template<class T, EqVariableType I>
class CqParameterTypedVertexArray : public CqParameterTypedVaryingArray<T,I>
{
	public:
						CqParameterTypedVertexArray(const char* strName, TqInt Count) : 
											CqParameterTypedVaryingArray<T,I>(strName, Count)
											{}
						CqParameterTypedVertexArray(const CqParameterTypedVertexArray<T,I>& From) :
											CqParameterTypedVaryingArray<T,I>(From)
											{}
	virtual				~CqParameterTypedVertexArray()
											{}

	virtual	CqParameter* Clone()const	{return(new CqParameterTypedVertexArray<T,I>(*this));}
	virtual	EqVariableType	Type() const {return((EqVariableType)(I|Type_Vertex));}

						/** Static constructor, to allow type free parameter construction.
						 * \param strName Character pointer to new parameter name.
						 * \param Count Integer array size.
						 */
	static	CqParameter*	Create(const char* strName, TqInt Count=1)
											{return(new CqParameterTypedVertexArray<T,I>(strName, Count));}

	private:
};



/** \fn void CqParameterTypedVarying<T,I>::BilinearDice(TqInt u, TqInt v, CqShaderVariable* pResult)
 * Dice the value into a grid using bilinear interpolation.
 * \param u Integer dice count for the u direction.
 * \param v Integer dice count for the v direction.
 * \param pResult Pointer to storage for the result.
 */

template<class T, EqVariableType I>
void CqParameterTypedVarying<T,I>::BilinearDice(TqInt u, TqInt v, CqShaderVariable* pResult)
{
	assert((pResult->Type()&Type_Mask)==(Type()&Type_Mask));
	assert((pResult->Type()&Storage_Mask)==Type_Varying);
	CqShaderVariableVarying<I,T>* pVR=static_cast<CqShaderVariableVarying<I,T>*>(pResult);
	// Check if a valid 4 point quad, do nothing if not.
	if(m_aValues.size()==4)
	{
		// Note it is assumed that the variable has been 
		// initialised to the correct size prior to calling.
		TqFloat diu=1.0/u;
		TqFloat div=1.0/v;
		TqInt i=0;
		TqInt iv;
		for(iv=0; iv<=v; iv++)
		{
			TqInt iu;
			for(iu=0; iu<=u; iu++)
			{
				(*pVR)[i]=BilinearEvaluate<T>(pValue(0)[0],
											  pValue(1)[0],
											  pValue(2)[0],
											  pValue(3)[0],
											  iu*diu,iv*div);
				i++;
			}
		}
	}
	else
	{
		TqInt i=0;
		TqInt iv;
		for(iv=0; iv<=v; iv++)
		{
			TqInt iu;
			for(iu=0; iu<=u; iu++)
			{
				(*pVR)[i]=pValue(0)[0];
				i++;
			}
		}
	}
}


/** \fn void CqParameterTypedVaryingArray<T,I>::BilinearDice(TqInt u, TqInt v, CqShaderVariable* pResult)
 * Dice the value into a grid using bilinear interpolation.
 * \param u Integer dice count for the u direction.
 * \param v Integer dice count for the v direction.
 * \param pResult Pointer to storage for the result.
 */

template<class T, EqVariableType I>
void CqParameterTypedVaryingArray<T,I>::BilinearDice(TqInt u, TqInt v, CqShaderVariable* pResult)
{
	assert((pResult->Type()&Type_Mask)==(Type()&Type_Mask));
	assert((pResult->Type()&Storage_Mask)==Type_Varying);
	CqShaderVariableVarying<I,T>* pVR=static_cast<CqShaderVariableVarying<I,T>*>(pResult);
	// Check if a valid 4 point quad, do nothing if not.
	if(m_aValues.size()==4)
	{
		// Note it is assumed that the variable has been 
		// initialised to the correct size prior to calling.
		TqFloat diu=1.0/u;
		TqFloat div=1.0/v;
		TqInt i=0;
		TqInt iv;
		for(iv=0; iv<=v; iv++)
		{
			TqInt iu;
			for(iu=0; iu<=u; iu++)
			{
				(*pVR)[i]=BilinearEvaluate<T>(pValue(0)[0],
											  pValue(1)[0],
											  pValue(2)[0],
											  pValue(3)[0],
											  iu*diu,iv*div);
				i++;
			}
		}
	}
	else
	{
		TqInt i=0;
		TqInt iv;
		for(iv=0; iv<=v; iv++)
		{
			TqInt iu;
			for(iu=0; iu<=u; iu++)
			{
				(*pVR)[i]=pValue(0)[0];
				i++;
			}
		}
	}
}


_qShareM	extern CqParameter* (*gVariableCreateFuncsUniform[])(const char* strName, TqInt Count);
_qShareM	extern CqParameter* (*gVariableCreateFuncsVarying[])(const char* strName, TqInt Count);
_qShareM	extern CqParameter* (*gVariableCreateFuncsVertex[])(const char* strName, TqInt Count);

//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif	// !PARAMETERS_H_INCLUDED
