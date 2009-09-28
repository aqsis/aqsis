// Aqsis
// Copyright  1997 - 2001, Paul C. Gregory
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
		\brief Declares the classes and support structures for handling parameters attached to GPrims.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef PARAMETERS_H_INCLUDED
#define PARAMETERS_H_INCLUDED 1

#include	<aqsis/aqsis.h>

#include	<vector>

#include	<boost/shared_ptr.hpp>

#include	<aqsis/util/logging.h>
#include	<aqsis/core/isurface.h>
#include	<aqsis/shadervm/ishaderdata.h>
#include	<aqsis/core/iparameter.h>
#include	"bilinear.h"
#include	<aqsis/riutil/primvartoken.h>
#include	<aqsis/math/vectorcast.h>

namespace Aqsis {

/** \brief Convert a primvar type into the associated shading language type
 *
 * CqParameters have two associated types - the type T of the primvar and the
 * type SLT of the associated shading language type.  Values of type T needs to
 * be converted into type SLT during the dicing process.
 *
 * \param paramVal - primvar value
 * \return the associated shading type 
 */
template<typename SLT, typename T>
SLT paramToShaderType(const T& paramVal);

//----------------------------------------------------------------------
/** \class CqParameter
 * Class storing a parameter with a name and value.
 */

class CqParameter : public IqParameter
{
	public:
		CqParameter( const char* strName, TqInt Count = 1 );
		CqParameter( const CqParameter& From );
		virtual	~CqParameter();

		/** \brief Create a CqParameter derived class of the type defined by tok.
		 *
		 * \param tok - Token specifying the class, type, name and count of the
		 *              primitive variable
		 * \return An instance of CqParameterTypedBlah where Blah corresponds
		 *         to the appropriate name and class etc.
		 */
		static CqParameter* Create(const CqPrimvarToken& tok);

		/** Pure virtual, clone function, which only creates a new parameter with matching type, no data.
		 * \return A pointer to a new parameter with the same type.
		 */
		virtual	CqParameter* CloneType( const char* Name, TqInt Count = 1 ) const = 0;
		/** Pure virtual, duplicate function.
		 * \return A pointer to a new parameter with the same name and value.
		 */
		virtual	CqParameter* Clone() const = 0;
		/** Pure virtual, get value class.
		 * \return Class as an EqVariableClass.
		 */
		virtual	EqVariableClass	Class() const = 0;
		/** Pure virtual, get value type.
		 * \return Type as an EqVariableType.
		 */
		virtual	EqVariableType	Type() const = 0;
		/** Pure virtual, set value size, not array, but varying/vertex size.
		 */
		virtual	void	SetSize( TqInt size ) = 0;
		/** Pure virtual, get value size, not array, but varying/vertex size.
		 */
		virtual	TqUint	Size() const = 0;
		virtual TqInt	ArrayLength() const
		{
			return Count();
		}
		/** Pure virtual, clear value contents.
		 */
		virtual	void	Clear() = 0;

		/** \attention
		 * The subdivide functions perform common splitting and interpolation on primitive variables
		 * they are only of use if the variable is a bilinear quad (the most common kind)
		 * any other type of splitting or interpolation must be performed by the surface which 
		 * instantiates special types (i.e. polygons).
		 */
		/** Subdivide the value in the u direction, place one half in this and the other in the specified parameter.
		 * \param pResult1 Pointer to the parameter class to hold the first half of the split.
		 * \param pResult2 Pointer to the parameter class to hold the first half of the split.
		 * \param u Boolean indicating whether to split in the u direction, false indicates split in v.
		 * \param pSurface Pointer to the surface which this paramter belongs, used if the surface has special handling of parameter splitting.
		 */
		virtual void	Subdivide( CqParameter* /* pResult1 */, CqParameter* /* pResult2 */, bool /* u */, IqSurface* /* pSurface */ = 0 )
		{}
		/** Pure virtual, dice the value into a grid using appropriate interpolation for the class.
		 * \param u Integer dice count for the u direction.
		 * \param v Integer dice count for the v direction.
		 * \param pResult Pointer to storage for the result.
		 * \param pSurface Pointer to the surface we are processing used for vertex class variables to perform natural interpolation.
		 */
		virtual	void	Dice( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface = 0 ) = 0;
		virtual	void	CopyToShaderVariable( IqShaderData* pResult ) const = 0;

		/** Pure virtual, dice a single array element of the value into a grid using appropriate interpolation for the class.
		 * \param u Integer dice count for the u direction.
		 * \param v Integer dice count for the v direction.
		 * \param pResult Pointer to storage for the result.
		 * \param pSurface Pointer to the surface we are processing used for vertex class variables to perform natural interpolation.
		 */
		virtual	void	DiceOne( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface = 0, TqInt ArrayIndex = 0 ) = 0;

		/** Pure virtual, set an indexed value from another parameter (must be the same type).
		 * \param pFrom Pointer to parameter to get values from.
		 * \param idxTarget Index of value to set,
		 * \param idxSource Index of value to get,
		 */
		virtual	void	SetValue(const CqParameter* pFrom, TqInt idxTarget, TqInt idxSource ) = 0;

		/** Get a reference to the parameter name.
		 */
		const	CqString& strName() const
		{
			return ( m_strName );
		}
		const TqUlong hash() const
		{
			return m_hash;
		}

		/** Get the array size.
		 */
		TqInt	Count() const
		{
			return ( m_Count );
		}

	protected:
		CqString	m_strName;		///< String name of the parameter.
		TqInt	m_Count;		///< Array size of value.
		TqUlong m_hash;

}
;


//----------------------------------------------------------------------
/** \class CqParameterTyped
 * Parameter templatised by its value type.
 */

template <class T, class SLT>
class CqParameterTyped : public CqParameter
{
	public:
		CqParameterTyped( const char* strName, TqInt Count = 1 ) :
				CqParameter( strName, Count )
		{}
		CqParameterTyped( const CqParameterTyped<T, SLT>& From ) :
				CqParameter( From )
		{}
		virtual	~CqParameterTyped()
		{}

		/** Get a pointer to the value (presumes uniform).
		 */
		virtual	const	T*	pValue() const = 0;
		/** Get a pointer to the value (presumes uniform).
		 */
		virtual	T*	pValue() = 0;
		/** Get a pointer to the value at the specified index, if uniform index is ignored.
		 */
		virtual	const	T*	pValue( const TqInt Index ) const = 0;
		/** Get a pointer to the value at the specified index, if uniform index is ignored.
		 */
		virtual	T*	pValue( const TqInt Index ) = 0;

		virtual	void	SetValue(const CqParameter* pFrom, TqInt idxTarget, TqInt idxSource )
		{
			assert( pFrom->Type() == this->Type() );

			const CqParameterTyped<T, SLT>* pFromTyped = static_cast<const CqParameterTyped<T, SLT>*>( pFrom );
			*pValue( idxTarget ) = *pFromTyped->pValue( idxSource );
		}

	protected:
};


//----------------------------------------------------------------------
/** \class CqParameterTypedVarying
 * Parameter with a varying type, templatised by value type and type id.
 */

template <class T, EqVariableType I, class SLT>
class CqParameterTypedVarying : public CqParameterTyped<T, SLT>
{
	public:
		CqParameterTypedVarying( const char* strName, TqInt Count = 1 ) :
				CqParameterTyped<T, SLT>( strName, Count )
		{
			m_aValues.resize( 1 );
		}
		CqParameterTypedVarying( const CqParameterTypedVarying<T, I, SLT>& From ) :
				CqParameterTyped<T, SLT>( From )
		{
			*this = From;
		}
		virtual	~CqParameterTypedVarying()
		{}

		// Overrridden from CqParameter



		virtual	CqParameter* CloneType( const char* Name, TqInt Count = 1 ) const
		{
			return ( new CqParameterTypedVarying<T, I, SLT>( Name, Count ) );
		}
		virtual	CqParameter* Clone() const
		{
			return ( new CqParameterTypedVarying<T, I, SLT>( *this ) );
		}
		virtual	EqVariableClass	Class() const
		{
			return ( class_varying );
		}
		virtual	EqVariableType	Type() const
		{
			return ( I );
		}
		virtual	void	SetSize( TqInt size )
		{
			m_aValues.resize( size );
		}
		virtual	TqUint	Size() const
		{
			return ( m_aValues.size() );
		}
		virtual	void	Clear()
		{
			m_aValues.clear();
		}
		virtual void	Subdivide( CqParameter* pResult1, CqParameter* pResult2, bool u, IqSurface* pSurface = 0 )
		{
			assert( pResult1->Type() == this->Type() && pResult1->Type() == this->Type() &&
			        pResult1->Class() == this->Class() && pResult1->Class() == this->Class() );

			CqParameterTypedVarying<T, I, SLT>* pTResult1 = static_cast<CqParameterTypedVarying<T, I, SLT>*>( pResult1 );
			CqParameterTypedVarying<T, I, SLT>* pTResult2 = static_cast<CqParameterTypedVarying<T, I, SLT>*>( pResult2 );
			pTResult1->SetSize( 4 );
			pTResult2->SetSize( 4 );
			// Check if a valid 4 point quad, do nothing if not.
			if ( m_aValues.size() == 4 )
			{
				if ( u )
				{
					pTResult2->pValue( 1 ) [ 0 ] = pValue( 1 ) [ 0 ];
					pTResult2->pValue( 3 ) [ 0 ] = pValue( 3 ) [ 0 ];
					pTResult1->pValue( 1 ) [ 0 ] = pTResult2->pValue( 0 ) [ 0 ] = static_cast<T>( ( pValue( 0 ) [ 0 ] + pValue( 1 ) [ 0 ] ) * 0.5 );
					pTResult1->pValue( 3 ) [ 0 ] = pTResult2->pValue( 2 ) [ 0 ] = static_cast<T>( ( pValue( 2 ) [ 0 ] + pValue( 3 ) [ 0 ] ) * 0.5 );
				}
				else
				{
					pTResult2->pValue( 2 ) [ 0 ] = pValue( 2 ) [ 0 ];
					pTResult2->pValue( 3 ) [ 0 ] = pValue( 3 ) [ 0 ];
					pTResult1->pValue( 2 ) [ 0 ] = pTResult2->pValue( 0 ) [ 0 ] = static_cast<T>( ( pValue( 0 ) [ 0 ] + pValue( 2 ) [ 0 ] ) * 0.5 );
					pTResult1->pValue( 3 ) [ 0 ] = pTResult2->pValue( 1 ) [ 0 ] = static_cast<T>( ( pValue( 1 ) [ 0 ] + pValue( 3 ) [ 0 ] ) * 0.5 );
				}
			}
		}
		virtual	void	Dice( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface = 0 );
		virtual	void	CopyToShaderVariable( IqShaderData* pResult ) const;

		virtual	void	DiceOne( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface = 0, TqInt ArrayIndex = 0 )
		{
			assert( false );
			return;
		}

		// Overridden from CqParameterTyped<T>

		virtual	const	T*	pValue() const
		{
			assert( 0 < m_aValues.size() );
			return ( &m_aValues[ 0 ] );
		}
		virtual	T*	pValue()
		{
			assert( 0 < m_aValues.size() );
			return ( &m_aValues[ 0 ] );
		}
		virtual	const	T*	pValue( const TqInt Index ) const
		{
			assert( Index < static_cast<TqInt>( m_aValues.size() ) );
			return ( &m_aValues[ Index ] );
		}
		virtual	T*	pValue( const TqInt Index )
		{
			assert( Index < static_cast<TqInt>( m_aValues.size() ) );
			return ( &m_aValues[ Index ] );
		}


		/** Assignment operator
		 */
		CqParameterTypedVarying<T, I, SLT>& operator=( const CqParameterTypedVarying<T, I, SLT>& From )
		{

			TqInt size = From.m_aValues.size();

			m_aValues.resize( size );

			for ( TqUint j = 0; j < (TqUint) size; j++ )
			{
				m_aValues[ j ] = From.m_aValues[ j ];
			}
			return ( *this );
		}

		/** Static constructor, to allow type free parameter construction.
		 * \param strName Character pointer to new parameter name.
		 * \param Count Integer array size.
		 */
		static	CqParameter*	Create( const char* strName, TqInt Count = 1 )
		{
			return ( new CqParameterTypedVarying<T, I, SLT>( strName, Count ) );
		}

	private:
		std::vector<T>	m_aValues;		///< Vector of values, one per varying index.
}
;


//----------------------------------------------------------------------
/** \class CqParameterTypedUniform
 * Parameter with a uniform type, templatised by value type and type id.
 */

template <class T, EqVariableType I, class SLT>
class CqParameterTypedUniform : public CqParameterTyped<T, SLT>
{
	public:
		CqParameterTypedUniform( const char* strName, TqInt Count = 1 ) :
				CqParameterTyped<T, SLT>( strName, Count )
		{
			m_aValues.resize( 1 );
		}
		CqParameterTypedUniform( const CqParameterTypedUniform<T, I, SLT>& From ) :
				CqParameterTyped<T, SLT>( From )
		{
			*this = From;
		}
		virtual	~CqParameterTypedUniform()
		{}

		virtual	CqParameter* CloneType( const char* Name, TqInt Count = 1 ) const
		{
			return ( new CqParameterTypedUniform<T, I, SLT>( Name, Count ) );
		}
		virtual	CqParameter* Clone() const
		{
			return ( new CqParameterTypedUniform<T, I, SLT>( *this ) );
		}
		virtual	EqVariableClass	Class() const
		{
			return ( class_uniform );
		}
		virtual	EqVariableType	Type() const
		{
			return ( I );
		}
		virtual	void	SetSize( TqInt size )
		{
			m_aValues.resize( size );
		}
		virtual	TqUint	Size() const
		{
			return ( m_aValues.size() );
		}
		virtual	void	Clear()
		{
			m_aValues.clear();
		}

		virtual void	Subdivide( CqParameter* pResult1, CqParameter* pResult2, bool u, IqSurface* pSurface = 0 )
		{
			assert( pResult1->Type() == this->Type() && pResult1->Type() == this->Type() &&
			        pResult1->Class() == Class() && pResult1->Class() == Class() );

			CqParameterTypedUniform<T, I, SLT>* pTResult1 = static_cast<CqParameterTypedUniform<T, I, SLT>*>( pResult1 );
			CqParameterTypedUniform<T, I, SLT>* pTResult2 = static_cast<CqParameterTypedUniform<T, I, SLT>*>( pResult2 );
			( *pTResult1 ) = ( *pTResult2 ) = ( *this );
		}
		virtual	void	Dice( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface = 0 )
		{
			// Just promote the uniform value to varying by duplication.
			assert( pResult->Type() == this->Type() );
			// Note it is assumed that the variable has been
			// initialised to the correct size prior to calling.
			// Also note that the only time a Uniform value is diced is when it is on a single element, i.e. the patchmesh
			// has been split into isngle patches, or the polymesh has been split into polys.
			TqUint i;
			TqUint size = max<TqInt>(u*v, pResult->Size());
			for ( i = 0; i < size; i++ )
				pResult->SetValue( paramToShaderType<SLT,T>(m_aValues[0]), i );
		}

		virtual	void	CopyToShaderVariable( IqShaderData* pResult ) const
		{
			// Just promote the uniform value to varying by duplication.
			assert( pResult->Type() == this->Type() );
			// Note it is assumed that the variable has been
			// initialised to the correct size prior to calling.
			// Also note that the only time a Uniform value is diced is when it is on a single element, i.e. the patchmesh
			// has been split into isngle patches, or the polymesh has been split into polys.
			TqUint i;
			TqUint size = pResult->Size();
			for ( i = 0; i < size; i++ )
				pResult->SetValue( paramToShaderType<SLT,T>(m_aValues[0]), i );
		}

		virtual	void	DiceOne( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface = 0, TqInt ArrayIndex = 0 )
		{
			assert( false );
			return;
		}

		// Overridden from CqParameterTyped<T>
		virtual	const	T*	pValue() const
		{
			assert( 0 < m_aValues.size() );
			return ( &m_aValues[ 0 ] );
		}
		virtual	T*	pValue()
		{
			assert( 0 < m_aValues.size() );
			return ( &m_aValues[ 0 ] );
		}
		virtual	const	T*	pValue( const TqInt Index ) const
		{
			assert( 0 < m_aValues.size() );
			return ( &m_aValues[ Index ] );
		}
		virtual	T*	pValue( const TqInt Index )
		{
			assert( 0 < m_aValues.size() );
			return ( &m_aValues[ Index ] );
		}


		/** Assignment operator.
		 */
		CqParameterTypedUniform<T, I, SLT>& operator=( const CqParameterTypedUniform<T, I, SLT>& From )
		{
			m_aValues.resize( From.m_aValues.size() );
			for ( TqUint j = 0; j < m_aValues.size(); j++ )
			{
				m_aValues[ j ] = From.m_aValues[ j ];
			}
			return ( *this );
		}

		/** Static constructor, to allow type free parameter construction.
		 * \param strName Character pointer to new parameter name.
		 * \param Count Integer array size.
		 */
		static	CqParameter*	Create( const char* strName, TqInt Count = 1 )
		{
			return ( new CqParameterTypedUniform<T, I, SLT>( strName, Count ) );
		}
	private:
		std::vector<T>	m_aValues;		///< Vector of values, one per uniform index.
}
;


//----------------------------------------------------------------------
/** \class CqParameterTypedConstant
 * Parameter with a constant type, templatised by value type and type id.
 */

template <class T, EqVariableType I, class SLT>
class CqParameterTypedConstant : public CqParameterTyped<T, SLT>
{
	public:
		CqParameterTypedConstant( const char* strName, TqInt Count = 1 ) :
				CqParameterTyped<T, SLT>( strName, Count )
		{}
		CqParameterTypedConstant( const CqParameterTypedConstant<T, I, SLT>& From ) :
				CqParameterTyped<T, SLT>( From )
		{
			m_Value = From.m_Value;
		}
		virtual	~CqParameterTypedConstant()
		{}

		virtual	CqParameter* CloneType( const char* Name, TqInt Count = 1 ) const
		{
			return ( new CqParameterTypedConstant<T, I, SLT>( Name, Count ) );
		}
		virtual	CqParameter* Clone() const
		{
			return ( new CqParameterTypedConstant<T, I, SLT>( *this ) );
		}
		virtual	EqVariableClass	Class() const
		{
			return ( class_constant );
		}
		virtual	EqVariableType	Type() const
		{
			return ( I );
		}
		virtual	void	SetSize( TqInt size )
		{}
		virtual	TqUint	Size() const
		{
			return ( 1 );
		}
		virtual	void	Clear()
		{}

		virtual void	Subdivide( CqParameter* pResult1, CqParameter* pResult2, bool u, IqSurface* pSurface = 0 )
		{
			assert( pResult1->Type() == this->Type() && pResult1->Type() == this->Type() &&
			        pResult1->Class() == this->Class() && pResult1->Class() == this->Class() );

			CqParameterTypedConstant<T, I, SLT>* pTResult1 = static_cast<CqParameterTypedConstant<T, I, SLT>*>( pResult1 );
			CqParameterTypedConstant<T, I, SLT>* pTResult2 = static_cast<CqParameterTypedConstant<T, I, SLT>*>( pResult2 );
			( *pTResult1 ) = ( *pTResult2 ) = ( *this );
		}
		virtual	void	Dice( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface = 0 )
		{
			// Just promote the constant value to varying by duplication.
			assert( pResult->Type() == this->Type() );
			// Note it is assumed that the variable has been
			// initialised to the correct size prior to calling.
			TqUint i;
			TqUint size = max<TqInt>(u*v, pResult->Size());
			for ( i = 0; i < size ; i++ )
				pResult->SetValue( paramToShaderType<SLT,T>(m_Value), i );
		}
		virtual	void	CopyToShaderVariable( IqShaderData* pResult ) const
		{
			// Just promote the constant value to varying by duplication.
			assert( pResult->Type() == this->Type() );
			// Note it is assumed that the variable has been
			// initialised to the correct size prior to calling.
			TqUint i;
			TqUint size = pResult->Size();
			for ( i = 0; i < size ; i++ )
				pResult->SetValue( paramToShaderType<SLT,T>(m_Value), i );
		}

		virtual	void	DiceOne( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface = 0, TqInt ArrayIndex = 0 )
		{
			assert( false );
			return;
		}

		// Overridden from CqParameterTyped<T>
		virtual	const	T*	pValue() const
		{
			return ( &m_Value );
		}
		virtual	T*	pValue()
		{
			return ( &m_Value );
		}
		virtual	const	T*	pValue( const TqInt Index ) const
		{
			return ( &m_Value );
		}
		virtual	T*	pValue( const TqInt Index )
		{
			return ( &m_Value );
		}


		/** Assignment operator.
		 */
		CqParameterTypedConstant<T, I, SLT>& operator=( const CqParameterTypedConstant<T, I, SLT>& From )
		{
			m_Value = From.m_Value;
			return ( *this );
		}

		/** Static constructor, to allow type free parameter construction.
		 * \param strName Character pointer to new parameter name.
		 * \param Count Integer array size.
		 */
		static	CqParameter*	Create( const char* strName, TqInt Count = 1 )
		{
			return ( new CqParameterTypedConstant<T, I, SLT>( strName, Count ) );
		}
	private:
		T	m_Value;	///< Single constant value.
}
;


//----------------------------------------------------------------------
/** \class CqParameterTypedVertex
 * Parameter with a vertex type, templatised by value type and type id.
 */

template <class T, EqVariableType I, class SLT>
class CqParameterTypedVertex : public CqParameterTypedVarying<T, I, SLT>
{
	public:
		CqParameterTypedVertex( const char* strName, TqInt Count = 1 ) :
				CqParameterTypedVarying<T, I, SLT>( strName, Count )
		{}
		CqParameterTypedVertex( const CqParameterTypedVertex<T, I, SLT>& From ) :
				CqParameterTypedVarying<T, I, SLT>( From )
		{}
		virtual	~CqParameterTypedVertex()
		{}

		virtual	CqParameter* CloneType( const char* Name, TqInt Count = 1 ) const
		{
			return ( new CqParameterTypedVertex<T, I, SLT>( Name, Count ) );
		}
		virtual	CqParameter* Clone() const
		{
			return ( new CqParameterTypedVertex<T, I, SLT>( *this ) );
		}
		virtual	EqVariableClass	Class() const
		{
			return ( class_vertex );
		}
		virtual void	Subdivide( CqParameter* pResult1, CqParameter* pResult2, bool u, IqSurface* pSurface = 0 )
		{
			assert( pResult1->Type() == this->Type() && pResult1->Type() == this->Type() &&
			        pResult1->Class() == this->Class() && pResult1->Class() == this->Class() );

			pSurface->NaturalSubdivide( this, pResult1, pResult2, u );
		}
		virtual	void	Dice( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface = 0 )
		{
			// Just promote the constant value to varying by duplication.
			assert( pResult->Type() == this->Type() );
			assert( NULL != pSurface );
			// Note it is assumed that the variable has been
			// initialised to the correct size prior to calling.
			pSurface->NaturalDice( this, u, v, pResult );
		}
		virtual	void	CopyToShaderVariable( IqShaderData* pResult ) const
		{
			assert( pResult->Type() == this->Type() );
			TqUint i;
			TqUint size = pResult->Size();
			for ( i = 0; i < size ; i++ )
				pResult->SetValue( paramToShaderType<SLT,T>(this->pValue(i)[0]), i );
		}

		virtual	void	DiceOne( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface = 0, TqInt ArrayIndex = 0 )
		{
			assert( false );
			return;
		}

		/** Static constructor, to allow type free parameter construction.
		 * \param strName Character pointer to new parameter name.
		 * \param Count Integer array size.
		 */
		static	CqParameter*	Create( const char* strName, TqInt Count = 1 )
		{
			return ( new CqParameterTypedVertex<T, I, SLT>( strName, Count ) );
		}

	private:
};


//----------------------------------------------------------------------
/** \class CqParameterTypedFaceVarying
 * Parameter with a vertex type, templatised by value type and type id.
 */

template <class T, EqVariableType I, class SLT>
class CqParameterTypedFaceVarying : public CqParameterTypedVarying<T, I, SLT>
{
	public:
		CqParameterTypedFaceVarying( const char* strName, TqInt Count = 1 ) :
				CqParameterTypedVarying<T, I, SLT>( strName, Count )
		{}
		CqParameterTypedFaceVarying( const CqParameterTypedVertex<T, I, SLT>& From ) :
				CqParameterTypedVarying<T, I, SLT>( From )
		{}
		virtual	~CqParameterTypedFaceVarying()
		{}

		virtual	CqParameter* CloneType( const char* Name, TqInt Count = 1 ) const
		{
			return ( new CqParameterTypedFaceVarying<T, I, SLT>( Name, Count ) );
		}
		virtual	CqParameter* Clone() const
		{
			return ( new CqParameterTypedFaceVarying<T, I, SLT>( *this ) );
		}
		virtual	EqVariableClass	Class() const
		{
			return ( class_facevarying );
		}

		/** Static constructor, to allow type free parameter construction.
		 * \param strName Character pointer to new parameter name.
		 * \param Count Integer array size.
		 */
		static	CqParameter*	Create( const char* strName, TqInt Count = 1 )
		{
			return ( new CqParameterTypedFaceVarying<T, I, SLT>( strName, Count ) );
		}

	private:
};


//----------------------------------------------------------------------
/** \class CqParameterTypedFaceVertex
 * Parameter with a vertex type, templatised by value type and type id.
 */

template <class T, EqVariableType I, class SLT>
class CqParameterTypedFaceVertex : public CqParameterTypedVertex<T, I, SLT>
{
	public:
		CqParameterTypedFaceVertex( const char* strName, TqInt Count = 1 ) :
				CqParameterTypedVertex<T, I, SLT>( strName, Count )
		{}
		CqParameterTypedFaceVertex( const CqParameterTypedVertex<T, I, SLT>& From ) :
				CqParameterTypedVertex<T, I, SLT>( From )
		{}
		virtual	~CqParameterTypedFaceVertex()
		{}

		virtual	CqParameter* CloneType( const char* Name, TqInt Count = 1 ) const
		{
			return ( new CqParameterTypedFaceVertex<T, I, SLT>( Name, Count ) );
		}
		virtual	CqParameter* Clone() const
		{
			return ( new CqParameterTypedFaceVertex<T, I, SLT>( *this ) );
		}
		virtual	EqVariableClass	Class() const
		{
			return ( class_facevertex );
		}

		/** Static constructor, to allow type free parameter construction.
		 * \param strName Character pointer to new parameter name.
		 * \param Count Integer array size.
		 */
		static	CqParameter*	Create( const char* strName, TqInt Count = 1 )
		{
			return ( new CqParameterTypedFaceVertex<T, I, SLT>( strName, Count ) );
		}

	private:
};

//----------------------------------------------------------------------
/** \class CqParameterTypedVaryingArray
 * Parameter with a varying array type, templatised by value type and type id.
 */

template <class T, EqVariableType I, class SLT>
class CqParameterTypedVaryingArray : public CqParameterTyped<T, SLT>
{
	public:
		CqParameterTypedVaryingArray( const char* strName, TqInt Count = 1 ) :
				CqParameterTyped<T, SLT>( strName, Count ),
				m_size(1),
				m_aValues(Count)
		{ }
		CqParameterTypedVaryingArray( const CqParameterTypedVaryingArray<T, I, SLT>& From ) :
				CqParameterTyped<T, SLT>( From )
		{
			*this = From;
		}
		virtual	~CqParameterTypedVaryingArray()
		{}

		virtual	CqParameter* CloneType( const char* Name, TqInt Count = 1 ) const
		{
			return ( new CqParameterTypedVaryingArray<T, I, SLT>( Name, Count ) );
		}
		virtual	CqParameter* Clone() const
		{
			return ( new CqParameterTypedVaryingArray<T, I, SLT>( *this ) );
		}
		virtual	EqVariableClass	Class() const
		{
			return ( class_varying );
		}
		virtual	EqVariableType	Type() const
		{
			return ( I );
		}
		virtual	void	SetSize( TqInt size )
		{
			m_size = size;
			m_aValues.resize(m_size*this->m_Count);
		}
		virtual	TqUint	Size() const
		{
			return m_size;
		}
		virtual	void	Clear()
		{
			m_aValues.clear();
		}
		virtual void	Subdivide( CqParameter* pResult1, CqParameter* pResult2, bool u, IqSurface* pSurface = 0 )
		{
			assert( pResult1->Type() == this->Type() && pResult1->Type() == this->Type() &&
			        pResult1->Class() == Class() && pResult1->Class() == Class() );

			CqParameterTypedVaryingArray<T, I, SLT>* pTResult1 = static_cast<CqParameterTypedVaryingArray<T, I, SLT>*>( pResult1 );
			CqParameterTypedVaryingArray<T, I, SLT>* pTResult2 = static_cast<CqParameterTypedVaryingArray<T, I, SLT>*>( pResult2 );
			pTResult1->SetSize( 4 );
			pTResult2->SetSize( 4 );
			// Check if a valid 4 point quad, do nothing if not.
			if ( Size() == 4 )
			{
				if ( u )
				{
					TqInt index;
					for( index = this->Count()-1; index >= 0; index-- )
					{
						pTResult2->pValue( 1 ) [ index ] = pValue( 1 ) [ index ];
						pTResult2->pValue( 3 ) [ index ] = pValue( 3 ) [ index ];
						pTResult1->pValue( 1 ) [ index ] = pTResult2->pValue( 0 ) [ index ] = static_cast<T>( ( pValue( 0 ) [ index ] + pValue( 1 ) [ index ] ) * 0.5 );
						pTResult1->pValue( 3 ) [ index ] = pTResult2->pValue( 2 ) [ index ] = static_cast<T>( ( pValue( 2 ) [ index ] + pValue( 3 ) [ index ] ) * 0.5 );
					}
				}
				else
				{
					TqInt index;
					for( index = this->Count()-1; index >= 0; index-- )
					{
						pTResult2->pValue( 2 ) [ index ] = pValue( 2 ) [ index ];
						pTResult2->pValue( 3 ) [ index ] = pValue( 3 ) [ index ];
						pTResult1->pValue( 2 ) [ index ] = pTResult2->pValue( 0 ) [ index ] = static_cast<T>( ( pValue( 0 ) [ index ] + pValue( 2 ) [ index ] ) * 0.5 );
						pTResult1->pValue( 3 ) [ index ] = pTResult2->pValue( 1 ) [ index ] = static_cast<T>( ( pValue( 1 ) [ index ] + pValue( 3 ) [ index ] ) * 0.5 );
					}
				}
			}
		}
		virtual	void	Dice( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface = 0 );
		virtual	void	CopyToShaderVariable( IqShaderData* pResult ) const;
		virtual	void	DiceOne( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface = 0, TqInt ArrayIndex = 0 );

		// Overridden from CqParameterTyped<T>
		virtual	const	T*	pValue() const
		{
			assert( 0 < m_aValues.size() );
			return &m_aValues[0];
		}
		virtual	T*	pValue()
		{
			assert( 0 < m_aValues.size() );
			return &m_aValues[0];
		}
		virtual	const	T*	pValue( const TqInt Index ) const
		{
			assert(Index < static_cast<TqInt>(Size()));
			return &m_aValues[this->m_Count*Index];
		}
		virtual	T*	pValue( const TqInt Index )
		{
			assert(Index < static_cast<TqInt>(Size()));
			return (&m_aValues[this->m_Count*Index]);
		}


		virtual	void	SetValue(const CqParameter* pFrom, TqInt idxTarget, TqInt idxSource )
		{
			assert( pFrom->Type() == this->Type() );
			assert( pFrom->Count() == this->Count() );

			const CqParameterTyped<T, SLT>* pFromTyped = static_cast<const CqParameterTyped<T, SLT>*>( pFrom );
			T* pTargetValues = pValue( idxTarget );
			const T* pSourceValues = pFromTyped->pValue( idxSource );
			for(TqInt index = 0; index < this->Count(); index++ )
				pTargetValues[ index ] = pSourceValues[ index ];
		}

		/** Assignment operator.
		 */
		CqParameterTypedVaryingArray<T, I, SLT>& operator=( const CqParameterTypedVaryingArray<T, I, SLT>& From )
		{
			m_size = From.m_size;
			m_aValues.assign(From.m_aValues.begin(), From.m_aValues.end());
			return ( *this );
		}

		/** Static constructor, to allow type free parameter construction.
		 * \param strName Character pointer to new parameter name.
		 * \param Count Integer array size.
		 */
		static	CqParameter*	Create( const char* strName, TqInt Count = 1 )
		{
			return ( new CqParameterTypedVaryingArray<T, I, SLT>( strName, Count ) );
		}

	private:
		TqInt m_size;  ///< number of values stored ( == m_aValues.size()/m_Count )
		std::vector<T>	m_aValues;		///< Array of varying values.
}
;


//----------------------------------------------------------------------
/** \class CqParameterTypedUniformArray
 * Parameter with a uniform array type, templatised by value type and type id.
 */

template <class T, EqVariableType I, class SLT>
class CqParameterTypedUniformArray : public CqParameterTyped<T, SLT>
{
	public:
		CqParameterTypedUniformArray( const char* strName, TqInt Count = 1 ) :
				CqParameterTyped<T, SLT>( strName, Count )
		{
			m_aValues.resize( Count );
		}
		CqParameterTypedUniformArray( const CqParameterTypedUniformArray<T, I, SLT>& From ) :
				CqParameterTyped<T, SLT>( From )
		{
			m_aValues.resize( From.m_Count );
			TqUint i;
			for ( i = 0; i < (TqUint)From.m_Count; i++ )
				m_aValues[ i ] = From.m_aValues[ i ];
		}
		virtual	~CqParameterTypedUniformArray()
	{}

		virtual	CqParameter* CloneType( const char* Name, TqInt Count = 1 ) const
		{
			return ( new CqParameterTypedUniformArray<T, I, SLT>( Name, Count ) );
		}
		virtual	CqParameter* Clone() const
		{
			return ( new CqParameterTypedUniformArray<T, I, SLT>( *this ) );
		}
		virtual	EqVariableClass	Class() const
		{
			return ( class_uniform );
		}
		virtual	EqVariableType	Type() const
		{
			return ( I );
		}
		virtual	void	SetSize( TqInt size )
		{}
		virtual	TqUint	Size() const
		{
			return ( 1 );
		}
		virtual	void	Clear()
		{}

		virtual void	Subdivide( CqParameter* pResult1, CqParameter* pResult2, bool u, IqSurface* pSurface = 0 )
		{}
		virtual	void	Dice( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface = 0 )
		{
			// Just promote the uniform value to varying by duplication.
			assert( pResult->Type() == this->Type() && pResult->isArray() );
			// Note it is assumed that the variable has been
			// initialised to the correct size prior to calling.
			TqUint i; 
			TqInt  j;
			TqUint size = max<TqInt>(u*v, pResult->Size());
			for ( i = 0; i < size; ++i )
				for( j = 0; j < this->ArrayLength(); ++j )
					pResult->ArrayEntry(j)->SetValue( paramToShaderType<SLT,T>(pValue( 0 ) [ j ]), i );
		}
		virtual	void	CopyToShaderVariable( IqShaderData* pResult ) const
		{
			// Just promote the uniform value to varying by duplication.
			assert( pResult->Type() == this->Type() && pResult->isArray() );
			// Note it is assumed that the variable has been
			// initialised to the correct size prior to calling.
			TqInt size = pResult->Size();
			TqInt arLen = this->ArrayLength();
			for(TqInt i = 0; i < size; ++i)
				for(TqInt j = 0; j < arLen; ++j)
					pResult->ArrayEntry(j)->SetValue( paramToShaderType<SLT,T>(pValue( 0 ) [ j ]), i );
		}

		virtual	void	DiceOne( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface = 0, TqInt ArrayIndex = 0 )
		{
			// Just promote the uniform value to varying by duplication.
			assert( pResult->Type() == this->Type() );
			assert( this->Count() > ArrayIndex );
			// Note it is assumed that the variable has been
			// initialised to the correct size prior to calling.
			TqUint i;
			TqUint size = max<TqInt>(u*v, pResult->Size());
			for ( i = 0; i < size; i++ )
				pResult->ArrayEntry(ArrayIndex)->SetValue( paramToShaderType<SLT,T>(pValue( 0 ) [ ArrayIndex ]), i );
		}

		// Overridden from CqParameterTyped<T>
		virtual	const	T*	pValue() const
		{
			assert( 0 < m_aValues.size() );
			return ( &m_aValues[ 0 ] );
		}
		virtual	T*	pValue()
		{
			assert( 0 < m_aValues.size() );
			return ( &m_aValues[ 0 ] );
		}
		virtual	const	T*	pValue( const TqInt Index ) const
		{
			assert( 0 < m_aValues.size() );
			return ( &m_aValues[ 0 ] );
		}
		virtual	T*	pValue( const TqInt Index )
		{
			assert( 0 < m_aValues.size() );
			return ( &m_aValues[ 0 ] );
		}

		/** Assignment operator.
		 */
		CqParameterTypedUniformArray<T, I, SLT>& operator=( const CqParameterTypedUniformArray<T, I, SLT>& From )
		{
			m_aValues.resize( From.m_aValues.size() );
			TqInt i2 = 0;
			for (typename  std::vector<T>::iterator i = From.m_aValues.begin(); i != From.m_aValues.end(); i++, i2++ )
				m_aValues[ i2 ] = ( *i );
			return ( *this );
		}

		/** Static constructor, to allow type free parameter construction.
		 * \param strName Character pointer to new parameter name.
		 * \param Count Integer array size.
		 */
		static	CqParameter*	Create( const char* strName, TqInt Count = 1 )
		{
			return ( new CqParameterTypedUniformArray<T, I, SLT>( strName, Count ) );
		}
	private:
		std::vector<T>	m_aValues;	///< Array of uniform values.
}
;

//----------------------------------------------------------------------
/** \class CqParameterTypedConstantArray
 * Parameter with a constant array type, templatised by value type and type id.
 */

template <class T, EqVariableType I, class SLT>
class CqParameterTypedConstantArray : public CqParameterTyped<T, SLT>
{
	public:
		CqParameterTypedConstantArray( const char* strName, TqInt Count = 1 ) :
				CqParameterTyped<T, SLT>( strName, Count )
		{
			m_aValues.resize( Count );
		}
		CqParameterTypedConstantArray( const CqParameterTypedConstantArray<T, I, SLT>& From ) :
				CqParameterTyped<T, SLT>( From )
		{
			m_aValues.resize( From.m_Count );
			TqInt i;
			for ( i = 0; i < From.m_Count; i++ )
				m_aValues[ i ] = From.m_aValues[ i ];
		}
		virtual	~CqParameterTypedConstantArray()
	{}

		virtual	CqParameter* CloneType( const char* Name, TqInt Count = 1 ) const
		{
			return ( new CqParameterTypedConstantArray<T, I, SLT>( Name, Count ) );
		}
		virtual	CqParameter* Clone() const
		{
			return ( new CqParameterTypedConstantArray<T, I, SLT>( *this ) );
		}
		virtual	EqVariableClass	Class() const
		{
			return ( class_constant );
		}
		virtual	EqVariableType	Type() const
		{
			return ( I );
		}
		virtual	void	SetSize( TqInt size )
		{}
		virtual	TqUint	Size() const
		{
			return ( 1 );
		}
		virtual	void	Clear()
		{}

		virtual void	Subdivide( CqParameter* pResult1, CqParameter* pResult2, bool u, IqSurface* pSurface = 0 )
		{}
		virtual	void	Dice( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface = 0 )
		{
			// Just promote the constant value to varying by duplication.
			assert( pResult->Type() == this->Type() && pResult->isArray() );
			// Note it is assumed that the variable has been
			// initialised to the correct size prior to calling.
			TqUint i; 
			TqInt  j;
			TqUint size = max<TqInt>(u*v, pResult->Size());
			for ( i = 0; i < size; ++i )
				for( j = 0; j < this->Count(); ++j )
					pResult->ArrayEntry(j)->SetValue( paramToShaderType<SLT,T>(pValue( 0 ) [ j ]), i );
		}
		virtual	void	CopyToShaderVariable( IqShaderData* pResult ) const
		{
			// Just promote the constant value to varying by duplication.
			assert( pResult->Type() == this->Type() && pResult->isArray() );
			// Note it is assumed that the variable has been
			// initialised to the correct size prior to calling.
			TqInt size = pResult->Size();
			TqInt arLen = this->ArrayLength();
			for(TqInt i = 0; i < size; ++i)
				for(TqInt j = 0; j < arLen; ++j)
					pResult->ArrayEntry(j)->SetValue( paramToShaderType<SLT,T>(pValue( 0 ) [ j ]), i );
		}

		virtual	void	DiceOne( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface = 0, TqInt ArrayIndex = 0 )
		{
			// Just promote the constant value to varying by duplication.
			assert( pResult->Type() == this->Type() && pResult->isArray() );
			assert( this->ArrayLength() > ArrayIndex );
			// Note it is assumed that the variable has been
			// initialised to the correct size prior to calling.
			TqUint i;
			TqUint size = max<TqInt>(u*v, pResult->Size());
			for ( i = 0; i < size; i++ )
				pResult->ArrayEntry(ArrayIndex)->SetValue( paramToShaderType<SLT,T>(pValue( 0 ) [ ArrayIndex ]), i );
		}

		// Overridden from CqParameterTyped<T>
		virtual	const	T*	pValue() const
		{
			assert( 0 < m_aValues.size() );
			return ( &m_aValues[ 0 ] );
		}
		virtual	T*	pValue()
		{
			assert( 0 < m_aValues.size() );
			return ( &m_aValues[ 0 ] );
		}
		virtual	const	T*	pValue( const TqInt Index ) const
		{
			assert( 0 < m_aValues.size() );
			return ( &m_aValues[ 0 ] );
		}
		virtual	T*	pValue( const TqInt Index )
		{
			assert( 0 < m_aValues.size() );
			return ( &m_aValues[ 0 ] );
		}

		/** Assignment operator.
		 */
		CqParameterTypedConstantArray<T, I, SLT>& operator=( const CqParameterTypedUniformArray<T, I, SLT>& From )
		{
			m_aValues.resize( From.m_aValues.size() );
			TqInt i2 = 0;
			for ( typename std::vector<T>::iterator i = From.m_aValues.being(); i != From.m_aValues.end(); i++, i2++ )
				m_aValues[ i2 ] = ( *i );
			return ( *this );
		}

		/** Static constructor, to allow type free parameter construction.
		 * \param strName Character pointer to new parameter name.
		 * \param Count Integer array size.
		 */
		static	CqParameter*	Create( const char* strName, TqInt Count = 1 )
		{
			return ( new CqParameterTypedConstantArray<T, I, SLT>( strName, Count ) );
		}
	private:
		std::vector<T>	m_aValues;	///< Array of uniform values.
}
;


//----------------------------------------------------------------------
/** \class CqParameterTypedVertexArray
 * Parameter with a vertex array type, templatised by value type and type id.
 */

template <class T, EqVariableType I, class SLT>
class CqParameterTypedVertexArray : public CqParameterTypedVaryingArray<T, I, SLT>
{
	public:
		CqParameterTypedVertexArray( const char* strName, TqInt Count ) :
				CqParameterTypedVaryingArray<T, I, SLT>( strName, Count )
		{}
		CqParameterTypedVertexArray( const CqParameterTypedVertexArray<T, I, SLT>& From ) :
				CqParameterTypedVaryingArray<T, I, SLT>( From )
		{}
		virtual	~CqParameterTypedVertexArray()
		{}

		virtual	CqParameter* CloneType( const char* Name, TqInt Count = 1 ) const
		{
			return ( new CqParameterTypedVertexArray<T, I, SLT>( Name, Count ) );
		}
		virtual	CqParameter* Clone() const
		{
			return ( new CqParameterTypedVertexArray<T, I, SLT>( *this ) );
		}
		virtual	EqVariableClass	Class() const
		{
			return ( class_vertex );
		}
		virtual	void	Dice( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface = 0 )
		{
			// Just promote the uniform value to varying by duplication.
			assert( pResult->Type() == this->Type() );
			assert( NULL != pSurface );
			// Note it is assumed that the variable has been
			// initialised to the correct size prior to calling.
			pSurface->NaturalDice( this, u, v, pResult );
		}
		virtual	void	CopyToShaderVariable( IqShaderData* pResult ) const
		{
			// Just promote the uniform value to varying by duplication.
			assert( pResult->Type() == this->Type() && pResult->isArray() );
			// Note it is assumed that the variable has been
			// initialised to the correct size prior to calling.
			TqInt size = pResult->Size();
			TqInt arLen = this->ArrayLength();
			for(TqInt i = 0; i < size; ++i)
				for(TqInt j = 0; j < arLen; ++j)
					pResult->ArrayEntry(j)->SetValue( paramToShaderType<SLT,T>(this->pValue( 0 ) [ j ]), i );
		}

		virtual	void	DiceOne( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface = 0, TqInt ArrayIndex = 0 )
		{
			/// \note: Need to work out how to do this...
			return;
		}

		/** Static constructor, to allow type free parameter construction.
		 * \param strName Character pointer to new parameter name.
		 * \param Count Integer array size.
		 */
		static	CqParameter*	Create( const char* strName, TqInt Count = 1 )
		{
			return ( new CqParameterTypedVertexArray<T, I, SLT>( strName, Count ) );
		}

	private:
};


//----------------------------------------------------------------------
/** \class CqParameterTypedFaceVaryingArray
 * Parameter with a facevarying array type, templatised by value type and type id.
 */

template <class T, EqVariableType I, class SLT>
class CqParameterTypedFaceVaryingArray : public CqParameterTypedVaryingArray<T, I, SLT>
{
	public:
		CqParameterTypedFaceVaryingArray( const char* strName, TqInt Count = 1 ) :
				CqParameterTypedVaryingArray<T, I, SLT>( strName, Count )
		{}
		virtual	~CqParameterTypedFaceVaryingArray()
		{}

		virtual	CqParameter* CloneType( const char* Name, TqInt Count = 1 ) const
		{
			return ( new CqParameterTypedFaceVaryingArray<T, I, SLT>( Name, Count ) );
		}
		virtual	CqParameter* Clone() const
		{
			return ( new CqParameterTypedFaceVaryingArray<T, I, SLT>( *this ) );
		}
		virtual	EqVariableClass	Class() const
		{
			return ( class_facevarying );
		}

		/** Static constructor, to allow type free parameter construction.
		 * \param strName Character pointer to new parameter name.
		 * \param Count Integer array size.
		 */
		static	CqParameter*	Create( const char* strName, TqInt Count = 1 )
		{
			return ( new CqParameterTypedFaceVaryingArray<T, I, SLT>( strName, Count ) );
		}

	private:
};


//----------------------------------------------------------------------
/** \class CqParameterTypedFaceVertexArray
 * Parameter with a facevertex array type, templatised by value type and type id.
 */

template <class T, EqVariableType I, class SLT>
class CqParameterTypedFaceVertexArray : public CqParameterTypedVertexArray<T, I, SLT>
{
	public:
		CqParameterTypedFaceVertexArray( const char* strName, TqInt Count = 1 ) :
				CqParameterTypedVertexArray<T, I, SLT>( strName, Count )
		{}
		virtual	~CqParameterTypedFaceVertexArray()
		{}

		virtual	CqParameter* CloneType( const char* Name, TqInt Count = 1 ) const
		{
			return ( new CqParameterTypedFaceVertexArray<T, I, SLT>( Name, Count ) );
		}
		virtual	CqParameter* Clone() const
		{
			return ( new CqParameterTypedFaceVertexArray<T, I, SLT>( *this ) );
		}
		virtual	EqVariableClass	Class() const
		{
			return ( class_facevertex );
		}

		/** Static constructor, to allow type free parameter construction.
		 * \param strName Character pointer to new parameter name.
		 * \param Count Integer array size.
		 */
		static	CqParameter*	Create( const char* strName, TqInt Count = 1 )
		{
			return ( new CqParameterTypedFaceVertexArray<T, I, SLT>( strName, Count ) );
		}

	private:
};

/** Dice the value into a grid using bilinear interpolation.
 * \param u Integer dice count for the u direction.
 * \param v Integer dice count for the v direction.
 * \param pResult Pointer to storage for the result.
 * \param pSurface Pointer to the surface to which this parameter belongs. Used if the surface type has special handling for parameter dicing.
 */

template <class T, EqVariableType I, class SLT>
void CqParameterTypedVarying<T, I, SLT>::Dice( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface )
{
	assert( pResult->Type() == this->Type() );

	// Check if the target is a varying variable, if not, this is an error.
	if(pResult->Class() != class_varying)
	{
		Aqsis::log() << error << "\"" << "Attempt to assign a varying value to uniform variable \"" <<
			pResult->strName() << "\"" << std::endl;
		return;
	}
		

	T res;

	SLT* pResData;
	pResult->GetValuePtr( pResData );
	assert( NULL != pResData );

	// Check if a valid 4 point quad, do nothing if not.
	if ( m_aValues.size() >= 4 )
	{
		// Note it is assumed that the variable has been
		// initialised to the correct size prior to calling.
		TqFloat diu = 1.0 / u;
		TqFloat div = 1.0 / v;
		TqInt iv;
		for ( iv = 0; iv <= v; iv++ )
		{
			TqInt iu;
			for ( iu = 0; iu <= u; iu++ )
			{
				res = BilinearEvaluate<T>( pValue( 0 ) [ 0 ],
				                           pValue( 1 ) [ 0 ],
				                           pValue( 2 ) [ 0 ],
				                           pValue( 3 ) [ 0 ],
				                           iu * diu, iv * div );
				( *pResData++ ) = paramToShaderType<SLT,T>(res);
			}
		}
	}
	else
	{
		TqInt iv;
		res = pValue( 0 ) [ 0 ];
		for ( iv = 0; iv <= v; iv++ )
		{
			TqInt iu;
			for ( iu = 0; iu <= u; iu++ )
				( *pResData++ ) = paramToShaderType<SLT,T>(res);
		}
	}
}

template <class T, EqVariableType I, class SLT>
void CqParameterTypedVarying<T, I, SLT>::CopyToShaderVariable( IqShaderData* pResult ) const
{
	assert( pResult->Type() == this->Type() );
	assert( pResult->Class() == class_varying );
	assert( pResult->Size() == Size() );

	SLT* pResData;
	pResult->GetValuePtr( pResData );
	assert( NULL != pResData );

	TqUint iu;
	for ( iu = 0; iu <= pResult->Size(); iu++ )
		( *pResData++ ) = paramToShaderType<SLT,T>(pValue(iu)[0]);
}

/** Dice the value into a grid using bilinear interpolation.
 * \param u Integer dice count for the u direction.
 * \param v Integer dice count for the v direction.
 * \param pResult Pointer to storage for the result.
 * \param pSurface Pointer to the surface to which this parameter belongs. Used if the surface type has special handling for parameter dicing.
 */

template <class T, EqVariableType I, class SLT>
void CqParameterTypedVaryingArray<T, I, SLT>::Dice( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface )
{
	assert( pResult->Type() == this->Type() );
	assert( pResult->Class() == class_varying );
	assert( pResult->Size() == Size() );
	assert( pResult->isArray() && pResult->ArrayLength() == this->ArrayLength() );

	T res;

	std::vector<SLT*> pResData(this->Count());
	
	TqInt arrayIndex;
	for(arrayIndex = 0; arrayIndex < this->Count(); arrayIndex++)
		pResult->ArrayEntry(arrayIndex)->GetValuePtr( pResData[arrayIndex] );

	// Check if a valid 4 point quad, do nothing if not.
	if ( Size() == 4 )
	{
		// Note it is assumed that the variable has been
		// initialised to the correct size prior to calling.
		TqFloat diu = 1.0 / u;
		TqFloat div = 1.0 / v;
		TqInt iv;
		for ( iv = 0; iv <= v; iv++ )
		{
			TqInt iu;
			for ( iu = 0; iu <= u; iu++ )
			{
				for( arrayIndex = 0; arrayIndex < this->Count(); arrayIndex++ )
				{
					res = BilinearEvaluate<T>( pValue( 0 ) [ arrayIndex ],
								   pValue( 1 ) [ arrayIndex ],
								   pValue( 2 ) [ arrayIndex ],
								   pValue( 3 ) [ arrayIndex ],
								   iu * diu, iv * div );
					( *(pResData[arrayIndex])++ ) = paramToShaderType<SLT,T>(res);
				}
			}
		}
	}
}

template <class T, EqVariableType I, class SLT>
void CqParameterTypedVaryingArray<T, I, SLT>::CopyToShaderVariable( IqShaderData* pResult ) const
{
	assert( pResult->Type() == this->Type() );
	assert( pResult->Class() == class_varying );
	assert( pResult->Size() == Size() );
	assert( pResult->isArray() && pResult->ArrayLength() == this->ArrayLength() );

	SLT* pResData;
	TqInt size = pResult->Size();
	TqInt arLen = pResult->ArrayLength();
	for (TqInt ia = 0; ia <= arLen; ++ia)
	{
		pResult->ArrayEntry(ia)->GetValuePtr(pResData);
		for (TqInt iu = 0; iu <= size; ++iu)
			(*pResData++) = paramToShaderType<SLT,T>(pValue(iu)[ia]);
	}
}

/** Dice the value into a grid using bilinear interpolation.
 * \param u Integer dice count for the u direction.
 * \param v Integer dice count for the v direction.
 * \param pResult Pointer to storage for the result.
 * \param pSurface Pointer to the surface to which this parameter belongs. Used if the surface type has special handling for parameter dicing.
 */

template <class T, EqVariableType I, class SLT>
void CqParameterTypedVaryingArray<T, I, SLT>::DiceOne( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface, TqInt ArrayIndex )
{
	assert( pResult->Type() == this->Type() );
	assert( pResult->Class() == class_varying );
	assert( this->Count() > ArrayIndex );

	T res;

	SLT* pResData;
	pResult->GetValuePtr( pResData );
	assert( NULL != pResData );

	// Check if a valid 4 point quad, do nothing if not.
	if ( Size() == 4 )
	{
		// Note it is assumed that the variable has been
		// initialised to the correct size prior to calling.
		TqFloat diu = 1.0 / u;
		TqFloat div = 1.0 / v;
		TqInt iv;
		for ( iv = 0; iv <= v; iv++ )
		{
			TqInt iu;
			for ( iu = 0; iu <= u; iu++ )
			{
				res = BilinearEvaluate<T>( pValue( 0 ) [ ArrayIndex ],
				                           pValue( 1 ) [ ArrayIndex ],
				                           pValue( 2 ) [ ArrayIndex ],
				                           pValue( 3 ) [ ArrayIndex ],
				                           iu * diu, iv * div );
				( *pResData++ ) = paramToShaderType<SLT,T>(res);
			}
		}
	}
}


//----------------------------------------------------------------------
/** \class CqNamedParameterList
 */

class CqNamedParameterList
{
	public:
		CqNamedParameterList( const char* strName ) : m_strName( strName )
		{
			m_hash = CqString::hash( strName );

		}
		CqNamedParameterList( const CqNamedParameterList& From );
		~CqNamedParameterList()
		{
			for ( std::vector<CqParameter*>::iterator i = m_aParameters.begin(); i != m_aParameters.end(); i++ )
				delete( ( *i ) );
		}

#ifdef _DEBUG
		CqString className() const
		{
			return CqString("CqNamedParameterList");
		}
#endif

		/** Get a refernece to the option name.
		 * \return A constant CqString reference.
		 */
		const	CqString&	strName() const
		{
			return ( m_strName );
		}

		/** Add a new name/value pair to this option/attribute.
		 * \param pParameter Pointer to a CqParameter containing the name/value pair.
		 */
		void	AddParameter( const CqParameter* pParameter )
		{
			for ( std::vector<CqParameter*>::iterator i = m_aParameters.begin(); i != m_aParameters.end(); i++ )
			{
				if ( ( *i ) ->hash() == pParameter->hash() )
				{
					delete( *i );
					( *i ) = const_cast<CqParameter*>( pParameter );
					return ;
				}
			}
			// If not append it.
			m_aParameters.push_back( const_cast<CqParameter*>( pParameter ) );
		}
		/** Get a read only pointer to a named parameter.
		 * \param strName Character pointer pointing to zero terminated parameter name.
		 * \return A pointer to a CqParameter or 0 if not found.
		 */
		const	CqParameter* pParameter( const char* strName ) const
		{
			const TqUlong hash = CqString::hash( strName );
			for ( std::vector<CqParameter*>::const_iterator i = m_aParameters.begin(); i != m_aParameters.end(); i++ )
				if ( ( *i ) ->hash() == hash )
					return ( *i );
			return ( 0 );
		}
		/** Get a pointer to a named parameter.
		 * \param strName Character pointer pointing to zero terminated parameter name.
		 * \return A pointer to a CqParameter or 0 if not found.
		 */
		CqParameter* pParameter( const char* strName )
		{
			const TqUlong hash = CqString::hash( strName );
			for ( std::vector<CqParameter*>::iterator i = m_aParameters.begin(); i != m_aParameters.end(); i++ )
				if ( ( *i ) ->hash() == hash )
					return ( *i );
			return ( 0 );
		}
		TqUlong hash()
		{
			return m_hash;
		}
	private:
		CqString	m_strName;			///< The name of this parameter list.
		std::vector<CqParameter*>	m_aParameters;		///< A vector of name/value parameters.
		TqUlong m_hash;
}
;

///////////////////////////////////////////////////////////////////////////////
	typedef CqParameterTyped<TqFloat, TqFloat> CqFloatParameter;
	typedef boost::shared_ptr<CqFloatParameter> CqFloatParameterPtr;
	
	typedef CqParameterTyped<TqInt, TqFloat> CqIntParameter;
	typedef boost::shared_ptr<CqIntParameter> CqIntParameterPtr;

	typedef CqParameterTyped<CqVector3D, CqVector3D> CqPointParameter;
	typedef boost::shared_ptr<CqPointParameter> CqPointParameterPtr;

	typedef CqParameterTyped<CqString, CqString> CqStringParameter;
	typedef boost::shared_ptr<CqStringParameter> CqStringParameterPtr;

	typedef CqParameterTyped<CqColor, CqColor> CqColorParameter;
	typedef boost::shared_ptr<CqColorParameter> CqColorParameterPtr;

	typedef CqParameterTyped<CqVector4D, CqVector3D> CqHPointParameter;
	typedef boost::shared_ptr<CqHPointParameter> CqHPointParameterPtr;

	typedef CqParameterTyped<CqVector3D, CqVector3D> CqNormalParameter;
	typedef boost::shared_ptr<CqNormalParameter> CqNormalParameterPtr;

	typedef CqParameterTyped<CqVector3D, CqVector3D> CqVectorParameter;
	typedef boost::shared_ptr<CqVectorParameter> CqVectorParameterPtr;

	typedef CqParameterTyped<CqMatrix, CqMatrix> CqMatrixParameter;
	typedef boost::shared_ptr<CqMatrixParameter> CqMatrixParameterPtr;

// Typedefs for the constants
	typedef CqParameterTypedConstant<TqFloat, type_float, TqFloat> CqFloatConstantParameter;
	typedef boost::shared_ptr<CqFloatConstantParameter> CqFloatConstantParameterPtr;
	
	typedef CqParameterTypedConstant<TqInt, type_integer, TqFloat> CqIntConstantParameter;
	typedef boost::shared_ptr<CqIntConstantParameter> CqIntConstantParameterPtr;

	typedef CqParameterTypedConstant<CqVector3D, type_point, CqVector3D> CqPointConstantParameter;
	typedef boost::shared_ptr<CqPointConstantParameter> CqPointConstantParameterPtr;

	typedef CqParameterTypedConstant<CqString, type_string, CqString> CqStringConstantParameter;
	typedef boost::shared_ptr<CqStringConstantParameter> CqStringConstantParameterPtr;

	typedef CqParameterTypedConstant<CqColor, type_color, CqColor> CqColorConstantParameter;
	typedef boost::shared_ptr<CqColorConstantParameter> CqColorConstantParameterPtr;

	typedef CqParameterTypedConstant<CqVector4D, type_hpoint, CqVector3D> CqHPointConstantParameter;
	typedef boost::shared_ptr<CqHPointConstantParameter> CqHPointConstantParameterPtr;

	typedef CqParameterTypedConstant<CqVector3D, type_normal, CqVector3D> CqNormalConstantParameter;
	typedef boost::shared_ptr<CqNormalConstantParameter> CqNormalConstantParameterPtr;

	typedef CqParameterTypedConstant<CqVector3D, type_vector, CqVector3D> CqVectorConstantParameter;
	typedef boost::shared_ptr<CqVectorConstantParameter> CqVectorConstantParameterPtr;

	typedef CqParameterTypedConstant<CqMatrix, type_matrix, CqMatrix> CqMatrixConstantParameter;
	typedef boost::shared_ptr<CqMatrixConstantParameter> CqMatrixConstantParameterPtr;

// Uniforms
	typedef CqParameterTypedUniform<TqFloat, type_float, TqFloat> CqFloatUniformParameter;
	typedef boost::shared_ptr<CqFloatUniformParameter> CqFloatUniformParameterPtr;
	
	typedef CqParameterTypedUniform<TqInt, type_integer, TqFloat> CqIntUniformParameter;
	typedef boost::shared_ptr<CqIntUniformParameter> CqIntUniformParameterPtr;

	typedef CqParameterTypedUniform<CqVector3D, type_point, CqVector3D> CqPointUniformParameter;
	typedef boost::shared_ptr<CqPointUniformParameter> CqPointUniformParameterPtr;

	typedef CqParameterTypedUniform<CqString, type_string, CqString> CqStringUniformParameter;
	typedef boost::shared_ptr<CqStringUniformParameter> CqStringUniformParameterPtr;

	typedef CqParameterTypedUniform<CqColor, type_color, CqColor> CqColorUniformParameter;
	typedef boost::shared_ptr<CqColorUniformParameter> CqColorUniformParameterPtr;

	typedef CqParameterTypedUniform<CqVector4D, type_hpoint, CqVector3D> CqHPointUniformParameter;
	typedef boost::shared_ptr<CqHPointUniformParameter> CqHPointUniformParameterPtr;

	typedef CqParameterTypedUniform<CqVector3D, type_normal, CqVector3D> CqNormalUniformParameter;
	typedef boost::shared_ptr<CqNormalUniformParameter> CqNormalUniformParameterPtr;

	typedef CqParameterTypedUniform<CqVector3D, type_vector, CqVector3D> CqVectorUniformParameter;
	typedef boost::shared_ptr<CqVectorUniformParameter> CqVectorUniformParameterPtr;

	typedef CqParameterTypedUniform<CqMatrix, type_matrix, CqMatrix> CqMatrixUniformParameter;
	typedef boost::shared_ptr<CqMatrixUniformParameter> CqMatrixUniformParameterPtr;

// Typedefs for Varying
	typedef CqParameterTypedVarying<TqFloat, type_float, TqFloat> CqFloatVaryingParameter;
	typedef boost::shared_ptr<CqFloatVaryingParameter> CqFloatVaryingParameterPtr;
	
	typedef CqParameterTypedVarying<TqInt, type_integer, TqFloat> CqIntVaryingParameter;
	typedef boost::shared_ptr<CqIntVaryingParameter> CqIntVaryingParameterPtr;

	typedef CqParameterTypedVarying<CqVector3D, type_point, CqVector3D> CqPointVaryingParameter;
	typedef boost::shared_ptr<CqPointVaryingParameter> CqPointVaryingParameterPtr;

	typedef CqParameterTypedVarying<CqString, type_string, CqString> CqStringVaryingParameter;
	typedef boost::shared_ptr<CqStringVaryingParameter> CqStringVaryingParameterPtr;

	typedef CqParameterTypedVarying<CqColor, type_color, CqColor> CqColorVaryingParameter;
	typedef boost::shared_ptr<CqColorVaryingParameter> CqColorVaryingParameterPtr;

	typedef CqParameterTypedVarying<CqVector4D, type_hpoint, CqVector3D> CqHPointVaryingParameter;
	typedef boost::shared_ptr<CqHPointVaryingParameter> CqHPointVaryingParameterPtr;

	typedef CqParameterTypedVarying<CqVector3D, type_normal, CqVector3D> CqNormalVaryingParameter;
	typedef boost::shared_ptr<CqNormalVaryingParameter> CqNormalVaryingParameterPtr;

	typedef CqParameterTypedVarying<CqVector3D, type_vector, CqVector3D> CqVectorVaryingParameter;
	typedef boost::shared_ptr<CqVectorVaryingParameter> CqVectorVaryingParameterPtr;

	typedef CqParameterTypedVarying<CqMatrix, type_matrix, CqMatrix> CqMatrixVaryingParameter;
	typedef boost::shared_ptr<CqMatrixVaryingParameter> CqMatrixVaryingParameterPtr;
	
// Vertex
	typedef CqParameterTypedVertex<TqFloat, type_float, TqFloat> CqFloatVertexParameter;
	typedef boost::shared_ptr<CqFloatVertexParameter> CqFloatVertexParameterPtr;
	
	typedef CqParameterTypedVertex<TqInt, type_integer, TqFloat> CqIntVertexParameter;
	typedef boost::shared_ptr<CqIntVertexParameter> CqIntVertexParameterPtr;

	typedef CqParameterTypedVertex<CqVector3D, type_point, CqVector3D> CqPointVertexParameter;
	typedef boost::shared_ptr<CqPointVertexParameter> CqPointVertexParameterPtr;

	typedef CqParameterTypedVertex<CqString, type_string, CqString> CqStringVertexParameter;
	typedef boost::shared_ptr<CqStringVertexParameter> CqStringVertexParameterPtr;

	typedef CqParameterTypedVertex<CqColor, type_color, CqColor> CqColorVertexParameter;
	typedef boost::shared_ptr<CqColorVertexParameter> CqColorVertexParameterPtr;

	typedef CqParameterTypedVertex<CqVector4D, type_hpoint, CqVector3D> CqHPointVertexParameter;
	typedef boost::shared_ptr<CqHPointVertexParameter> CqHPointVertexParameterPtr;

	typedef CqParameterTypedVertex<CqVector3D, type_normal, CqVector3D> CqNormalVertexParameter;
	typedef boost::shared_ptr<CqNormalVertexParameter> CqNormalVertexParameterPtr;

	typedef CqParameterTypedVertex<CqVector3D, type_vector, CqVector3D> CqVectorVertexParameter;
	typedef boost::shared_ptr<CqVectorVertexParameter> CqVectorVertexParameterPtr;

	typedef CqParameterTypedVertex<CqMatrix, type_matrix, CqMatrix> CqMatrixVertexParameter;
	typedef boost::shared_ptr<CqMatrixVertexParameter> CqMatrixVertexParameterPtr;

// FaceVarying
	typedef CqParameterTypedFaceVarying<TqFloat, type_float, TqFloat> CqFloatFaceVaryingParameter;
	typedef boost::shared_ptr<CqFloatFaceVaryingParameter> CqFloatFaceVaryingParameterPtr;
	
	typedef CqParameterTypedFaceVarying<TqInt, type_integer, TqFloat> CqIntFaceVaryingParameter;
	typedef boost::shared_ptr<CqIntFaceVaryingParameter> CqIntFaceVaryingParameterPtr;

	typedef CqParameterTypedFaceVarying<CqVector3D, type_point, CqVector3D> CqPointFaceVaryingParameter;
	typedef boost::shared_ptr<CqPointFaceVaryingParameter> CqPointFaceVaryingParameterPtr;

	typedef CqParameterTypedFaceVarying<CqString, type_string, CqString> CqStringFaceVaryingParameter;
	typedef boost::shared_ptr<CqStringFaceVaryingParameter> CqStringFaceVaryingParameterPtr;

	typedef CqParameterTypedFaceVarying<CqColor, type_color, CqColor> CqColorFaceVaryingParameter;
	typedef boost::shared_ptr<CqColorFaceVaryingParameter> CqColorFaceVaryingParameterPtr;

	typedef CqParameterTypedFaceVarying<CqVector4D, type_hpoint, CqVector3D> CqHPointFaceVaryingParameter;
	typedef boost::shared_ptr<CqHPointFaceVaryingParameter> CqHPointFaceVaryingParameterPtr;

	typedef CqParameterTypedFaceVarying<CqVector3D, type_normal, CqVector3D> CqNormalFaceVaryingParameter;
	typedef boost::shared_ptr<CqNormalFaceVaryingParameter> CqNormalFaceVaryingParameterPtr;

	typedef CqParameterTypedFaceVarying<CqVector3D, type_vector, CqVector3D> CqVectorFaceVaryingParameter;
	typedef boost::shared_ptr<CqVectorFaceVaryingParameter> CqVectorFaceVaryingParameterPtr;

	typedef CqParameterTypedFaceVarying<CqMatrix, type_matrix, CqMatrix> CqMatrixFaceVaryingParameter;
	typedef boost::shared_ptr<CqMatrixFaceVaryingParameter> CqMatrixFaceVaryingParameterPtr;

// Constant Array
	typedef CqParameterTypedConstantArray<TqFloat, type_float, TqFloat> CqFloatConstantArrayParameter;
	typedef boost::shared_ptr<CqFloatConstantArrayParameter> CqFloatConstantArrayParameterPtr;
	
	typedef CqParameterTypedConstantArray<TqInt, type_integer, TqFloat> CqIntConstantArrayParameter;
	typedef boost::shared_ptr<CqIntConstantArrayParameter> CqIntConstantArrayParameterPtr;

	typedef CqParameterTypedConstantArray<CqVector3D, type_point, CqVector3D> CqPointConstantArrayParameter;
	typedef boost::shared_ptr<CqPointConstantArrayParameter> CqPointConstantArrayParameterPtr;

	typedef CqParameterTypedConstantArray<CqString, type_string, CqString> CqStringConstantArrayParameter;
	typedef boost::shared_ptr<CqStringConstantArrayParameter> CqStringConstantArrayParameterPtr;

	typedef CqParameterTypedConstantArray<CqColor, type_color, CqColor> CqColorConstantArrayParameter;
	typedef boost::shared_ptr<CqColorConstantArrayParameter> CqColorConstantArrayParameterPtr;

	typedef CqParameterTypedConstantArray<CqVector4D, type_hpoint, CqVector3D> CqHPointConstantArrayParameter;
	typedef boost::shared_ptr<CqHPointConstantArrayParameter> CqHPointConstantArrayParameterPtr;

	typedef CqParameterTypedConstantArray<CqVector3D, type_normal, CqVector3D> CqNormalConstantArrayParameter;
	typedef boost::shared_ptr<CqNormalConstantArrayParameter> CqNormalConstantArrayParameterPtr;

	typedef CqParameterTypedConstantArray<CqVector3D, type_vector, CqVector3D> CqVectorConstantArrayParameter;
	typedef boost::shared_ptr<CqVectorConstantArrayParameter> CqVectorConstantArrayParameterPtr;

	typedef CqParameterTypedConstantArray<CqMatrix, type_matrix, CqMatrix> CqMatrixConstantArrayParameter;
	typedef boost::shared_ptr<CqMatrixConstantArrayParameter> CqMatrixConstantArrayParameterPtr;

// Uniform array
	typedef CqParameterTypedUniformArray<TqFloat, type_float, TqFloat> CqFloatUniformArrayParameter;
	typedef boost::shared_ptr<CqFloatUniformArrayParameter> CqFloatUniformArrayParameterPtr;
	
	typedef CqParameterTypedUniformArray<TqInt, type_integer, TqFloat> CqIntUniformArrayParameter;
	typedef boost::shared_ptr<CqIntUniformArrayParameter> CqIntUniformArrayParameterPtr;

	typedef CqParameterTypedUniformArray<CqVector3D, type_point, CqVector3D> CqPointUniformArrayParameter;
	typedef boost::shared_ptr<CqPointUniformArrayParameter> CqPointUniformArrayParameterPtr;

	typedef CqParameterTypedUniformArray<CqString, type_string, CqString> CqStringUniformArrayParameter;
	typedef boost::shared_ptr<CqStringUniformArrayParameter> CqStringUniformArrayParameterPtr;

	typedef CqParameterTypedUniformArray<CqColor, type_color, CqColor> CqColorUniformArrayParameter;
	typedef boost::shared_ptr<CqColorUniformArrayParameter> CqColorUniformArrayParameterPtr;

	typedef CqParameterTypedUniformArray<CqVector4D, type_hpoint, CqVector3D> CqHPointUniformArrayParameter;
	typedef boost::shared_ptr<CqHPointUniformArrayParameter> CqHPointUniformArrayParameterPtr;

	typedef CqParameterTypedUniformArray<CqVector3D, type_normal, CqVector3D> CqNormalUniformArrayParameter;
	typedef boost::shared_ptr<CqNormalUniformArrayParameter> CqNormalUniformArrayParameterPtr;

	typedef CqParameterTypedUniformArray<CqVector3D, type_vector, CqVector3D> CqVectorUniformArrayParameter;
	typedef boost::shared_ptr<CqVectorUniformArrayParameter> CqVectorUniformArrayParameterPtr;

	typedef CqParameterTypedUniformArray<CqMatrix, type_matrix, CqMatrix> CqMatrixUniformArrayParameter;
	typedef boost::shared_ptr<CqMatrixUniformArrayParameter> CqMatrixUniformArrayParameterPtr;

// Varying array
	typedef CqParameterTypedVaryingArray<TqFloat, type_float, TqFloat> CqFloatVaryingArrayParameter;
	typedef boost::shared_ptr<CqFloatVaryingArrayParameter> CqFloatVaryingArrayParameterPtr;
	
	typedef CqParameterTypedVaryingArray<TqInt, type_integer, TqFloat> CqIntVaryingArrayParameter;
	typedef boost::shared_ptr<CqIntVaryingArrayParameter> CqIntVaryingArrayParameterPtr;

	typedef CqParameterTypedVaryingArray<CqVector3D, type_point, CqVector3D> CqPointVaryingArrayParameter;
	typedef boost::shared_ptr<CqPointVaryingArrayParameter> CqPointVaryingArrayParameterPtr;

	typedef CqParameterTypedVaryingArray<CqString, type_string, CqString> CqStringVaryingArrayParameter;
	typedef boost::shared_ptr<CqStringVaryingArrayParameter> CqStringVaryingArrayParameterPtr;

	typedef CqParameterTypedVaryingArray<CqColor, type_color, CqColor> CqColorVaryingArrayParameter;
	typedef boost::shared_ptr<CqColorVaryingArrayParameter> CqColorVaryingArrayParameterPtr;

	typedef CqParameterTypedVaryingArray<CqVector4D, type_hpoint, CqVector3D> CqHPointVaryingArrayParameter;
	typedef boost::shared_ptr<CqHPointVaryingArrayParameter> CqHPointVaryingArrayParameterPtr;

	typedef CqParameterTypedVaryingArray<CqVector3D, type_normal, CqVector3D> CqNormalVaryingArrayParameter;
	typedef boost::shared_ptr<CqNormalVaryingArrayParameter> CqNormalVaryingArrayParameterPtr;

	typedef CqParameterTypedVaryingArray<CqVector3D, type_vector, CqVector3D> CqVectorVaryingArrayParameter;
	typedef boost::shared_ptr<CqVectorVaryingArrayParameter> CqVectorVaryingArrayParameterPtr;

	typedef CqParameterTypedVaryingArray<CqMatrix, type_matrix, CqMatrix> CqMatrixVaryingArrayParameter;
	typedef boost::shared_ptr<CqMatrixVaryingArrayParameter> CqMatrixVaryingArrayParameterPtr;

// Vertex array
	typedef CqParameterTypedVertexArray<TqFloat, type_float, TqFloat> CqFloatVertexArrayParameter;
	typedef boost::shared_ptr<CqFloatVertexArrayParameter> CqFloatVertexArrayParameterPtr;
	
	typedef CqParameterTypedVertexArray<TqInt, type_integer, TqFloat> CqIntVertexArrayParameter;
	typedef boost::shared_ptr<CqIntVertexArrayParameter> CqIntVertexArrayParameterPtr;

	typedef CqParameterTypedVertexArray<CqVector3D, type_point, CqVector3D> CqPointVertexArrayParameter;
	typedef boost::shared_ptr<CqPointVertexArrayParameter> CqPointVertexArrayParameterPtr;

	typedef CqParameterTypedVertexArray<CqString, type_string, CqString> CqStringVertexArrayParameter;
	typedef boost::shared_ptr<CqStringVertexArrayParameter> CqStringVertexArrayParameterPtr;

	typedef CqParameterTypedVertexArray<CqColor, type_color, CqColor> CqColorVertexArrayParameter;
	typedef boost::shared_ptr<CqColorVertexArrayParameter> CqColorVertexArrayParameterPtr;

	typedef CqParameterTypedVertexArray<CqVector4D, type_hpoint, CqVector3D> CqHPointVertexArrayParameter;
	typedef boost::shared_ptr<CqHPointVertexArrayParameter> CqHPointVertexArrayParameterPtr;

	typedef CqParameterTypedVertexArray<CqVector3D, type_normal, CqVector3D> CqNormalVertexArrayParameter;
	typedef boost::shared_ptr<CqNormalVertexArrayParameter> CqNormalVertexArrayParameterPtr;

	typedef CqParameterTypedVertexArray<CqVector3D, type_vector, CqVector3D> CqVectorVertexArrayParameter;
	typedef boost::shared_ptr<CqVectorVertexArrayParameter> CqVectorVertexArrayParameterPtr;

	typedef CqParameterTypedVertexArray<CqMatrix, type_matrix, CqMatrix> CqMatrixVertexArrayParameter;
	typedef boost::shared_ptr<CqMatrixVertexArrayParameter> CqMatrixVertexArrayParameterPtr;

// FaceVarying array
	typedef CqParameterTypedFaceVaryingArray<TqFloat, type_float, TqFloat> CqFloatFaceVaryingArrayParameter;
	typedef boost::shared_ptr<CqFloatFaceVaryingArrayParameter> CqFloatFaceVaryingArrayParameterPtr;
	
	typedef CqParameterTypedFaceVaryingArray<TqInt, type_integer, TqFloat> CqIntFaceVaryingArrayParameter;
	typedef boost::shared_ptr<CqIntFaceVaryingArrayParameter> CqIntFaceVaryingArrayParameterPtr;

	typedef CqParameterTypedFaceVaryingArray<CqVector3D, type_point, CqVector3D> CqPointFaceVaryingArrayParameter;
	typedef boost::shared_ptr<CqPointFaceVaryingArrayParameter> CqPointFaceVaryingArrayParameterPtr;

	typedef CqParameterTypedFaceVaryingArray<CqString, type_string, CqString> CqStringFaceVaryingArrayParameter;
	typedef boost::shared_ptr<CqStringFaceVaryingArrayParameter> CqStringFaceVaryingArrayParameterPtr;

	typedef CqParameterTypedFaceVaryingArray<CqColor, type_color, CqColor> CqColorFaceVaryingArrayParameter;
	typedef boost::shared_ptr<CqColorFaceVaryingArrayParameter> CqColorFaceVaryingArrayParameterPtr;

	typedef CqParameterTypedFaceVaryingArray<CqVector4D, type_hpoint, CqVector3D> CqHPointFaceVaryingArrayParameter;
	typedef boost::shared_ptr<CqHPointFaceVaryingArrayParameter> CqHPointFaceVaryingArrayParameterPtr;

	typedef CqParameterTypedFaceVaryingArray<CqVector3D, type_normal, CqVector3D> CqNormalFaceVaryingArrayParameter;
	typedef boost::shared_ptr<CqNormalFaceVaryingArrayParameter> CqNormalFaceVaryingArrayParameterPtr;

	typedef CqParameterTypedFaceVaryingArray<CqVector3D, type_vector, CqVector3D> CqVectorFaceVaryingArrayParameter;
	typedef boost::shared_ptr<CqVectorFaceVaryingArrayParameter> CqVectorFaceVaryingArrayParameterPtr;

	typedef CqParameterTypedFaceVaryingArray<CqMatrix, type_matrix, CqMatrix> CqMatrixFaceVaryingArrayParameter;
	typedef boost::shared_ptr<CqMatrixFaceVaryingArrayParameter> CqMatrixFaceVaryingArrayParameterPtr;
///////////////////////////////////////////////////////////////////////////////

extern CqParameter* ( *gVariableCreateFuncsConstant[] ) ( const char* strName, TqInt Count );
extern CqParameter* ( *gVariableCreateFuncsUniform[] ) ( const char* strName, TqInt Count );
extern CqParameter* ( *gVariableCreateFuncsVarying[] ) ( const char* strName, TqInt Count );
extern CqParameter* ( *gVariableCreateFuncsVertex[] ) ( const char* strName, TqInt Count );
extern CqParameter* ( *gVariableCreateFuncsFaceVarying[] ) ( const char* strName, TqInt Count );
extern CqParameter* ( *gVariableCreateFuncsFaceVertex[] ) ( const char* strName, TqInt Count );
extern CqParameter* ( *gVariableCreateFuncsConstantArray[] ) ( const char* strName, TqInt Count );
extern CqParameter* ( *gVariableCreateFuncsUniformArray[] ) ( const char* strName, TqInt Count );
extern CqParameter* ( *gVariableCreateFuncsVaryingArray[] ) ( const char* strName, TqInt Count );
extern CqParameter* ( *gVariableCreateFuncsVertexArray[] ) ( const char* strName, TqInt Count );
extern CqParameter* ( *gVariableCreateFuncsFaceVaryingArray[] ) ( const char* strName, TqInt Count );
extern CqParameter* ( *gVariableCreateFuncsFaceVertexArray[] ) ( const char* strName, TqInt Count );


//==============================================================================
// Implementation details
//==============================================================================
template<typename SLT, typename T>
inline SLT paramToShaderType(const T& paramVal)
{
	return SLT(paramVal);
}

template<>
inline CqVector3D paramToShaderType(const CqVector4D& paramVal)
{
	return vectorCast<CqVector3D>(paramVal);
}


} // namespace Aqsis

#endif	// !PARAMETERS_H_INCLUDED
