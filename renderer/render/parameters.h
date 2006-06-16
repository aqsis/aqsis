// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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

#include	<vector>

#include	"aqsis.h"

#include	"isurface.h"
#include	"ishaderdata.h"
#include	"bilinear.h"
#include	<boost/shared_ptr.hpp>

START_NAMESPACE( Aqsis )


//----------------------------------------------------------------------
/** \class CqParameter
 * Class storing a parameter with a name and value.
 */

class CqParameter
{
	public:
		CqParameter( const char* strName, TqInt Count = 1 );
		CqParameter( const CqParameter& From );
		virtual	~CqParameter();

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
		virtual void	Subdivide( CqParameter* pResult1, CqParameter* pResult2, TqBool u, IqSurface* pSurface = 0 )
		{}
		/** Pure virtual, dice the value into a grid using appropriate interpolation for the class.
		 * \param u Integer dice count for the u direction.
		 * \param v Integer dice count for the v direction.
		 * \param pResult Pointer to storage for the result.
		 * \param pSurface Pointer to the surface we are processing used for vertex class variables to perform natural interpolation.
		 */
		virtual	void	Dice( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface = 0 ) = 0;
		virtual	void	CopyToShaderVariable( IqShaderData* pResult ) = 0;

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
		virtual	void	SetValue( CqParameter* pFrom, TqInt idxTarget, TqInt idxSource ) = 0;

		/** Get a reference to the parameter name.
		 */
		const	TqChar * strName() const
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

      		static CqStringTable* m_sLocalTable;

      		static const TqChar * GetId(const char *strName)
      		{
         		if (m_sLocalTable == NULL)
            			m_sLocalTable = new CqStringTable;

         		return m_sLocalTable->Get(strName);
      		}
		static void Dump();

		static void Wipe();

	protected:
		TqChar  *m_strName;		///< String name of the parameter.
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

		virtual	void	SetValue( CqParameter* pFrom, TqInt idxTarget, TqInt idxSource )
		{
			assert( pFrom->Type() == Type() );

			CqParameterTyped<T, SLT>* pFromTyped = static_cast<CqParameterTyped<T, SLT>*>( pFrom );
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
		virtual void	Subdivide( CqParameter* pResult1, CqParameter* pResult2, TqBool u, IqSurface* pSurface = 0 )
		{
			assert( pResult1->Type() == Type() && pResult1->Type() == Type() &&
			        pResult1->Class() == Class() && pResult1->Class() == Class() );

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
		virtual	void	CopyToShaderVariable( IqShaderData* pResult );

		virtual	void	DiceOne( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface = 0, TqInt ArrayIndex = 0 )
		{
			assert( TqFalse );
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

		virtual void	Subdivide( CqParameter* pResult1, CqParameter* pResult2, TqBool u, IqSurface* pSurface = 0 )
		{
			assert( pResult1->Type() == Type() && pResult1->Type() == Type() &&
			        pResult1->Class() == Class() && pResult1->Class() == Class() );

			CqParameterTypedUniform<T, I, SLT>* pTResult1 = static_cast<CqParameterTypedUniform<T, I, SLT>*>( pResult1 );
			CqParameterTypedUniform<T, I, SLT>* pTResult2 = static_cast<CqParameterTypedUniform<T, I, SLT>*>( pResult2 );
			( *pTResult1 ) = ( *pTResult2 ) = ( *this );
		}
		virtual	void	Dice( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface = 0 )
		{
			// Just promote the uniform value to varying by duplication.
			assert( pResult->Type() == Type() );
			// Note it is assumed that the variable has been
			// initialised to the correct size prior to calling.
			// Also note that the only time a Uniform value is diced is when it is on a single element, i.e. the patchmesh
			// has been split into isngle patches, or the polymesh has been split into polys.
			TqUint i;
			TqUint max = MAX( (TqUint)u * (TqUint)v, pResult->Size() );
			for ( i = 0; i < max; i++ )
				pResult->SetValue( m_aValues[ 0 ], i );
		}

		virtual	void	CopyToShaderVariable( IqShaderData* pResult )
		{
			// Just promote the uniform value to varying by duplication.
			assert( pResult->Type() == Type() );
			// Note it is assumed that the variable has been
			// initialised to the correct size prior to calling.
			// Also note that the only time a Uniform value is diced is when it is on a single element, i.e. the patchmesh
			// has been split into isngle patches, or the polymesh has been split into polys.
			TqUint i;
			TqUint max = pResult->Size();
			for ( i = 0; i < max; i++ )
				pResult->SetValue( m_aValues[ 0 ], i );
		}

		virtual	void	DiceOne( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface = 0, TqInt ArrayIndex = 0 )
		{
			assert( TqFalse );
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

		virtual void	Subdivide( CqParameter* pResult1, CqParameter* pResult2, TqBool u, IqSurface* pSurface = 0 )
		{
			assert( pResult1->Type() == Type() && pResult1->Type() == Type() &&
			        pResult1->Class() == Class() && pResult1->Class() == Class() );

			CqParameterTypedConstant<T, I, SLT>* pTResult1 = static_cast<CqParameterTypedConstant<T, I, SLT>*>( pResult1 );
			CqParameterTypedConstant<T, I, SLT>* pTResult2 = static_cast<CqParameterTypedConstant<T, I, SLT>*>( pResult2 );
			( *pTResult1 ) = ( *pTResult2 ) = ( *this );
		}
		virtual	void	Dice( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface = 0 )
		{
			// Just promote the constant value to varying by duplication.
			assert( pResult->Type() == Type() );
			// Note it is assumed that the variable has been
			// initialised to the correct size prior to calling.
			TqUint i;
			TqUint max = MAX( (TqUint) u * (TqUint) v, pResult->Size() );
			for ( i = 0; i < max ; i++ )
				pResult->SetValue( m_Value, i );
		}
		virtual	void	CopyToShaderVariable( IqShaderData* pResult )
		{
			// Just promote the constant value to varying by duplication.
			assert( pResult->Type() == Type() );
			// Note it is assumed that the variable has been
			// initialised to the correct size prior to calling.
			TqUint i;
			TqUint max = pResult->Size();
			for ( i = 0; i < max ; i++ )
				pResult->SetValue( m_Value, i );
		}

		virtual	void	DiceOne( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface = 0, TqInt ArrayIndex = 0 )
		{
			assert( TqFalse );
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
		virtual void	Subdivide( CqParameter* pResult1, CqParameter* pResult2, TqBool u, IqSurface* pSurface = 0 )
		{
			assert( pResult1->Type() == Type() && pResult1->Type() == Type() &&
			        pResult1->Class() == Class() && pResult1->Class() == Class() );

			pSurface->NaturalSubdivide( this, pResult1, pResult2, u );
		}
		virtual	void	Dice( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface = 0 )
		{
			// Just promote the constant value to varying by duplication.
			assert( pResult->Type() == Type() );
			assert( NULL != pSurface );
			// Note it is assumed that the variable has been
			// initialised to the correct size prior to calling.
			pSurface->NaturalDice( this, u, v, pResult );
		}
		virtual	void	CopyToShaderVariable( IqShaderData* pResult )
		{
			assert( pResult->Type() == Type() );
			TqUint i;
			TqUint max = pResult->Size();
			for ( i = 0; i < max ; i++ )
				pResult->SetValue( this->pValue(i)[0], i );
		}

		virtual	void	DiceOne( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface = 0, TqInt ArrayIndex = 0 )
		{
			assert( TqFalse );
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
/** \class CqParameterTypedVaryingArray
 * Parameter with a varying array type, templatised by value type and type id.
 */

template <class T, EqVariableType I, class SLT>
class CqParameterTypedVaryingArray : public CqParameterTyped<T, SLT>
{
	public:
		CqParameterTypedVaryingArray( const char* strName, TqInt Count = 1 ) :
				CqParameterTyped<T, SLT>( strName, Count )
		{
			m_aValues.resize( 1, std::vector<T>(Count) );
		}
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
			m_aValues.resize( size, std::vector< T >(this->m_Count) );
		}
		virtual	TqUint	Size() const
		{
			return ( m_aValues.size() );
		}
		virtual	void	Clear()
		{
			m_aValues.clear();
		}
		virtual void	Subdivide( CqParameter* pResult1, CqParameter* pResult2, TqBool u, IqSurface* pSurface = 0 )
		{
			assert( pResult1->Type() == Type() && pResult1->Type() == Type() &&
			        pResult1->Class() == Class() && pResult1->Class() == Class() );

			CqParameterTypedVaryingArray<T, I, SLT>* pTResult1 = static_cast<CqParameterTypedVaryingArray<T, I, SLT>*>( pResult1 );
			CqParameterTypedVaryingArray<T, I, SLT>* pTResult2 = static_cast<CqParameterTypedVaryingArray<T, I, SLT>*>( pResult2 );
			pTResult1->SetSize( 4 );
			pTResult2->SetSize( 4 );
			// Check if a valid 4 point quad, do nothing if not.
			if ( m_aValues.size() == 4 )
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
		virtual	void	CopyToShaderVariable( IqShaderData* pResult );
		virtual	void	DiceOne( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface = 0, TqInt ArrayIndex = 0 );

		// Overridden from CqParameterTyped<T>
		virtual	const	T*	pValue() const
		{
			assert( 0 < m_aValues.size() );
			assert( 0 < m_aValues[0].size() );
			return ( &m_aValues[ 0 ][ 0 ] );
		}
		virtual	T*	pValue()
		{
			assert( 0 < m_aValues.size() );
			assert( 0 < m_aValues[0].size() );
			return ( &m_aValues[ 0 ][ 0 ] );
		}
		virtual	const	T*	pValue( const TqInt Index ) const
		{
			assert( Index < static_cast<TqInt>( m_aValues.size() ) );
			assert( 0 < m_aValues[0].size() );
			return ( &m_aValues[ Index ][ 0 ] );
		}
		virtual	T*	pValue( const TqInt Index )
		{
			assert( Index < static_cast<TqInt>( m_aValues.size() ) );
			assert( 0 < m_aValues[0].size() );
			return ( &m_aValues[ Index ][ 0 ] );
		}


		virtual	void	SetValue( CqParameter* pFrom, TqInt idxTarget, TqInt idxSource )
		{
			assert( pFrom->Type() == Type() );
			assert( pFrom->Count() == Count() );

			CqParameterTyped<T, SLT>* pFromTyped = static_cast<CqParameterTyped<T, SLT>*>( pFrom );
			TqInt index;
			T* pTargetValues = pValue( idxTarget );
			T* pSourceValues = pFromTyped->pValue( idxSource );
			for( index = 0; index < this->Count(); index++ )
				pTargetValues[ index ] = pSourceValues[ index ];
		}

		/** Assignment operator.
		 */
		CqParameterTypedVaryingArray<T, I, SLT>& operator=( const CqParameterTypedVaryingArray<T, I, SLT>& From )
		{
			m_aValues.resize( From.m_aValues.size(), std::vector<T>(From.Count()) );
			this->m_Count = From.m_Count;
			TqUint j;
			for ( j = 0; j < m_aValues.size(); j++ )
			{
				TqUint i;
				for ( i = 0; i < (TqUint) this->m_Count; i++ )
					m_aValues[ j ][ i ] = From.m_aValues[ j ][ i ];
			}
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
		std::vector<std::vector<T> >	m_aValues;		///< Array of varying values.
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

		virtual void	Subdivide( CqParameter* pResult1, CqParameter* pResult2, TqBool u, IqSurface* pSurface = 0 )
		{}
		virtual	void	Dice( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface = 0 )
		{
			// Just promote the uniform value to varying by duplication.
			assert( pResult->Type() == Type() );
			// Note it is assumed that the variable has been
			// initialised to the correct size prior to calling.
			TqUint i;
			TqUint max = ( MAX( (TqUint)u * (TqUint) v, pResult->Size() ) );
			for ( i = 0; i < max; i++ )
				pResult->SetValue( pValue( 0 ) [ 0 ], i );
		}
		virtual	void	CopyToShaderVariable( IqShaderData* pResult )
		{
			// Just promote the uniform value to varying by duplication.
			assert( pResult->Type() == Type() );
			// Note it is assumed that the variable has been
			// initialised to the correct size prior to calling.
			TqUint i;
			TqUint max = pResult->Size();
			for ( i = 0; i < max; i++ )
				pResult->SetValue( pValue( 0 ) [ 0 ], i );
		}

		virtual	void	DiceOne( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface = 0, TqInt ArrayIndex = 0 )
		{
			// Just promote the uniform value to varying by duplication.
			assert( pResult->Type() == Type() );
			assert( Count() > ArrayIndex );
			// Note it is assumed that the variable has been
			// initialised to the correct size prior to calling.
			TqUint i;
			TqUint max = ( MAX( (TqUint)u * (TqUint) v, pResult->Size() ) );
			for ( i = 0; i < max; i++ )
				pResult->SetValue( pValue( 0 ) [ ArrayIndex ], i );
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

		virtual void	Subdivide( CqParameter* pResult1, CqParameter* pResult2, TqBool u, IqSurface* pSurface = 0 )
		{}
		virtual	void	Dice( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface = 0 )
		{
			// Just promote the constant value to varying by duplication.
			assert( pResult->Type() == Type() );
			// Note it is assumed that the variable has been
			// initialised to the correct size prior to calling.
			TqUint i;
			TqUint max = ( MAX( (TqUint) u * (TqUint) v, pResult->Size() ) );
			for ( i = 0; i < max; i++ )
				pResult->SetValue( pValue( 0 ) [ 0 ], i );
		}
		virtual	void	CopyToShaderVariable( IqShaderData* pResult )
		{
			// Just promote the constant value to varying by duplication.
			assert( pResult->Type() == Type() );
			// Note it is assumed that the variable has been
			// initialised to the correct size prior to calling.
			TqUint i;
			TqUint max = pResult->Size();
			for ( i = 0; i < max; i++ )
				pResult->SetValue( pValue( 0 ) [ 0 ], i );
		}

		virtual	void	DiceOne( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface = 0, TqInt ArrayIndex = 0 )
		{
			// Just promote the constant value to varying by duplication.
			assert( pResult->Type() == Type() );
			assert( Count() > ArrayIndex );
			// Note it is assumed that the variable has been
			// initialised to the correct size prior to calling.
			TqUint i;
			TqUint max = ( MAX( (TqUint) u * (TqUint) v, pResult->Size() ) );
			for ( i = 0; i < max; i++ )
				pResult->SetValue( pValue( 0 ) [ ArrayIndex ], i );
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
			assert( pResult->Type() == Type() );
			assert( NULL != pSurface );
			// Note it is assumed that the variable has been
			// initialised to the correct size prior to calling.
			pSurface->NaturalDice( this, u, v, pResult );
		}
		virtual	void	CopyToShaderVariable( IqShaderData* pResult )
		{
			// Just promote the uniform value to varying by duplication.
			assert( pResult->Type() == Type() );
			// Note it is assumed that the variable has been
			// initialised to the correct size prior to calling.
			TqUint i;
			TqUint max = pResult->Size();
			for ( i = 0; i < max; i++ )
				pResult->SetValue( this->pValue( 0 ) [ 0 ], i );
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
		CqParameterTypedFaceVaryingArray( const CqParameterTypedVertexArray<T, I, SLT>& From ) :
				CqParameterTypedVaryingArray<T, I, SLT>( From )
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



/** Dice the value into a grid using bilinear interpolation.
 * \param u Integer dice count for the u direction.
 * \param v Integer dice count for the v direction.
 * \param pResult Pointer to storage for the result.
 * \param pSurface Pointer to the surface to which this parameter belongs. Used if the surface type has special handling for parameter dicing.
 */

template <class T, EqVariableType I, class SLT>
void CqParameterTypedVarying<T, I, SLT>::Dice( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface )
{
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
				( *pResData++ ) = res;
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
				( *pResData++ ) = res;
		}
	}
}

template <class T, EqVariableType I, class SLT>
void CqParameterTypedVarying<T, I, SLT>::CopyToShaderVariable( IqShaderData* pResult )
{
	T res;

	SLT* pResData;
	pResult->GetValuePtr( pResData );
	assert( NULL != pResData );

	TqInt iu;
	for ( iu = 0; iu <= pResult->Size(); iu++ )
		( *pResData++ ) = pValue(iu)[0];
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
	assert( pResult->Type() == Type() );
	assert( pResult->Class() == class_varying );
	assert( pResult->Size() == m_aValues.size() );

	T res;

	SLT* pResData;
	pResult->GetValuePtr( pResData );
	assert( NULL != pResData );

	// Check if a valid 4 point quad, do nothing if not.
	if ( m_aValues.size() == 4 )
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
				( *pResData++ ) = res;
			}
		}
	}
}

template <class T, EqVariableType I, class SLT>
void CqParameterTypedVaryingArray<T, I, SLT>::CopyToShaderVariable( IqShaderData* pResult )
{
	T res;

	SLT* pResData;
	pResult->GetValuePtr( pResData );
	assert( NULL != pResData );

	TqInt iu;
	for ( iu = 0; iu <= pResult->Size(); iu++ )
		( *pResData++ ) = pValue(iu)[0];
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
	assert( pResult->Type() == Type() );
	assert( pResult->Class() == class_varying );
	assert( Count() > ArrayIndex );
	assert( pResult->Size() == m_aValues.size() );

	T res;

	SLT* pResData;
	pResult->GetValuePtr( pResData );
	assert( NULL != pResData );

	// Check if a valid 4 point quad, do nothing if not.
	if ( m_aValues.size() == 4 )
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
				( *pResData++ ) = res;
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
		CqNamedParameterList( const char* strName ) 
		{
			m_strName = (char *) CqParameter::GetId(strName);
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
		 * \return A constant TqChar * reference.
		 */
		const	TqChar *strName() const
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
         		const TqChar * hash = CqParameter::GetId( strName );
			for ( std::vector<CqParameter*>::const_iterator i = m_aParameters.begin(); i != m_aParameters.end(); i++ )
				if ( ( *i ) ->strName() == hash )
					return ( *i );
			return ( 0 );
		}

		/** Get a pointer to a named parameter.
		 * \param strName Character pointer pointing to zero terminated parameter name.
		 * \return A pointer to a CqParameter or 0 if not found.
		 */
		CqParameter* pParameter( const char* strName )
		{
			const TqChar * hash = CqParameter::GetId( strName );
			for ( std::vector<CqParameter*>::iterator i = m_aParameters.begin(); i != m_aParameters.end(); i++ )
				if ( ( *i ) ->strName() == hash )
					return ( *i );
			return ( 0 );
		}

		TqUlong hash()
		{
			return m_hash;
		}
	private:
		TqChar *m_strName;		///< The name of this parameter list.
		std::vector<CqParameter*>	m_aParameters;		///< A vector of name/value parameters.
		TqUlong m_hash;
}
;



extern CqParameter* ( *gVariableCreateFuncsConstant[] ) ( const char* strName, TqInt Count );
extern CqParameter* ( *gVariableCreateFuncsUniform[] ) ( const char* strName, TqInt Count );
extern CqParameter* ( *gVariableCreateFuncsVarying[] ) ( const char* strName, TqInt Count );
extern CqParameter* ( *gVariableCreateFuncsVertex[] ) ( const char* strName, TqInt Count );
extern CqParameter* ( *gVariableCreateFuncsFaceVarying[] ) ( const char* strName, TqInt Count );
extern CqParameter* ( *gVariableCreateFuncsConstantArray[] ) ( const char* strName, TqInt Count );
extern CqParameter* ( *gVariableCreateFuncsUniformArray[] ) ( const char* strName, TqInt Count );
extern CqParameter* ( *gVariableCreateFuncsVaryingArray[] ) ( const char* strName, TqInt Count );
extern CqParameter* ( *gVariableCreateFuncsVertexArray[] ) ( const char* strName, TqInt Count );
extern CqParameter* ( *gVariableCreateFuncsFaceVaryingArray[] ) ( const char* strName, TqInt Count );

//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !PARAMETERS_H_INCLUDED
