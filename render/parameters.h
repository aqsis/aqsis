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

#include	"aqsis.h"

#include	"ishaderdata.h"
#include	"bilinear.h"

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
		virtual	CqParameter* CloneType(const char* Name, TqInt Count = 1 ) const = 0;
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
		 * \param pResult Pointer to storage for the result.
		 */
		virtual void	uSubdivide( CqParameter* pResult )
		{}
		/** Subdivide the value in the v direction, place one half in this and the other in the specified parameter.
		 * \param pResult Pointer to storage for the result.
		 */
		virtual void	vSubdivide( CqParameter* pResult )
		{}
		/** Pure virtual, dice the value into a grid using linear interpolation.
		 * \param u Integer dice count for the u direction.
		 * \param v Integer dice count for the v direction.
		 * \param pResult Pointer to storage for the result.
		 */
		virtual	void	BilinearDice( TqInt u, TqInt v, IqShaderData* pResult ) = 0;

		/** Pure virtual, set an indexed value from another parameter (must be the same type).
		 * \param pFrom Pointer to parameter to get values from.
		 * \param idxTarget Index of value to set,
		 * \param idxSource Index of value to get,
		 */
		virtual	void	SetValue( CqParameter* pFrom, TqInt idxTarget, TqInt idxSource ) = 0;

		/** Get a reference to the parameter name.
		 */
		const	CqString& strName() const
		{
			return ( m_strName );
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

			CqParameterTyped<T,SLT>* pFromTyped = static_cast<CqParameterTyped<T, SLT>*>(pFrom);
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


		virtual	CqParameter* CloneType(const char* Name, TqInt Count = 1 ) const
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
		virtual void	uSubdivide( CqParameter* pResult )
		{
			assert( pResult->Type() == Type() );
			CqParameterTypedVarying<T, I, SLT>* pVS = static_cast<CqParameterTypedVarying<T, I, SLT>*>( pResult );
			// Check if a valid 4 point quad, do nothing if not.
			if ( m_aValues.size() == 4 && pVS->m_aValues.size() == 4 )
			{
				pVS->pValue( 1 ) [ 0 ] = pValue( 1 ) [ 0 ];
				pVS->pValue( 3 ) [ 0 ] = pValue( 3 ) [ 0 ];
				pValue( 1 ) [ 0 ] = pVS->pValue( 0 ) [ 0 ] = static_cast<T>( ( pValue( 0 ) [ 0 ] + pValue( 1 ) [ 0 ] ) * 0.5 );
				pValue( 3 ) [ 0 ] = pVS->pValue( 2 ) [ 0 ] = static_cast<T>( ( pValue( 2 ) [ 0 ] + pValue( 3 ) [ 0 ] ) * 0.5 );
			}
		}
		virtual void	vSubdivide( CqParameter* pResult )
		{
			assert( pResult->Type() == Type() );
			CqParameterTypedVarying<T, I, SLT>* pVS = static_cast<CqParameterTypedVarying<T, I, SLT>*>( pResult );
			// Check if a valid 4 point quad, do nothing if not.
			if ( m_aValues.size() == 4 && pVS->m_aValues.size() == 4 )
			{
				pVS->pValue( 2 ) [ 0 ] = pValue( 2 ) [ 0 ];
				pVS->pValue( 3 ) [ 0 ] = pValue( 3 ) [ 0 ];
				pValue( 2 ) [ 0 ] = pVS->pValue( 0 ) [ 0 ] = static_cast<T>( ( pValue( 0 ) [ 0 ] + pValue( 2 ) [ 0 ] ) * 0.5 );
				pValue( 3 ) [ 0 ] = pVS->pValue( 1 ) [ 0 ] = static_cast<T>( ( pValue( 1 ) [ 0 ] + pValue( 3 ) [ 0 ] ) * 0.5 );
			}
		}
		virtual	void	BilinearDice( TqInt u, TqInt v, IqShaderData* pResult );

		// Overridden from CqParameterTyped<T>

		virtual	const	T*	pValue() const
		{
			return ( &m_aValues[ 0 ] );
		}
		virtual	T*	pValue()
		{
			return ( &m_aValues[ 0 ] );
		}
		virtual	const	T*	pValue( const TqInt Index ) const
		{
			return ( &m_aValues[ Index ] );
		}
		virtual	T*	pValue( const TqInt Index )
		{
			return ( &m_aValues[ Index ] );
		}


		/** Indexed access to values.
		 * \param Index Integer index into the varying value list.
		 */
		const	T&	operator[] ( const TqInt Index ) const
		{
			return( m_aValues[ Index ] );
		}
		/** Indexed access to values.
		 * \param Index Integer index into the varying value list.
		 */
		T&	operator[] ( const TqInt Index )
		{
			return( m_aValues[ Index ] );
		}

		/** Assignment operator
		 */
		CqParameterTypedVarying<T, I, SLT>& operator=( const CqParameterTypedVarying<T, I, SLT>& From )
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
			m_aValues.resize(1);
		}
		CqParameterTypedUniform( const CqParameterTypedUniform<T, I, SLT>& From ) :
				CqParameterTyped<T, SLT>( From )
		{
			*this = From;
		}
		virtual	~CqParameterTypedUniform()
		{}

		virtual	CqParameter* CloneType(const char* Name, TqInt Count = 1 ) const
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

		virtual void	uSubdivide( CqParameter* pResult )
		{}
		virtual void	vSubdivide( CqParameter* pResult )
		{}
		virtual	void	BilinearDice( TqInt u, TqInt v, IqShaderData* pResult )
		{
			// Just promote the uniform value to varying by duplication.
			assert( pResult->Type() == Type() );
			assert( pResult->Class() == class_varying );
			// Note it is assumed that the variable has been
			// initialised to the correct size prior to calling.
			// Also note that the only time a Uniform value is diced is when it is on a single element, i.e. the patchmesh
			// has been split into isngle patches, or the polymesh has been split into polys.
			TqInt i;
			for ( i = 0; i < u*v; i++ )
				pResult->SetValue( m_aValues[0], i );
		}


		// Overridden from CqParameterTyped<T>
		virtual	const	T*	pValue() const
		{
			return ( &m_aValues[0] );
		}
		virtual	T*	pValue()
		{
			return ( &m_aValues[0] );
		}
		virtual	const	T*	pValue( const TqInt Index ) const
		{
			return ( &m_aValues[Index] );
		}
		virtual	T*	pValue( const TqInt Index )
		{
			return ( &m_aValues[Index] );
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

		virtual	CqParameter* CloneType(const char* Name, TqInt Count = 1 ) const
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

		virtual void	uSubdivide( CqParameter* pResult )
		{}
		virtual void	vSubdivide( CqParameter* pResult )
		{}
		virtual	void	BilinearDice( TqInt u, TqInt v, IqShaderData* pResult )
		{
			// Just promote the constant value to varying by duplication.
			assert( pResult->Type() == Type() );
			assert( pResult->Class() == class_varying );
			// Note it is assumed that the variable has been
			// initialised to the correct size prior to calling.
			TqInt i;
			for ( i = 0; i < u*v; i++ )
				pResult->SetValue( m_Value, i );
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

		virtual	CqParameter* CloneType(const char* Name, TqInt Count = 1 ) const
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

		virtual	CqParameter* CloneType(const char* Name, TqInt Count = 1 ) const
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
			m_aValues.resize( 1 );
			m_aValues[ 0 ].resize( Count );
		}
		CqParameterTypedVaryingArray( const CqParameterTypedVaryingArray<T, I, SLT>& From ) :
				CqParameterTyped<T, SLT>( From )
		{
			*this = From;
		}
		virtual	~CqParameterTypedVaryingArray()
		{}

		virtual	CqParameter* CloneType(const char* Name, TqInt Count = 1 ) const
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
			m_aValues.resize( size );
			TqInt j;
			for ( j = 0; j < size; j++ )
				m_aValues[ j ].resize( m_Count );
		}
		virtual	TqUint	Size() const
		{
			return ( m_aValues[ 0 ].size() );
		}
		virtual	void	Clear()
		{
			m_aValues.clear();
		}
		virtual void	uSubdivide( CqParameter* pResult )
		{
			assert( pResult->Type() == Type() );
			CqParameterTypedVaryingArray<T, I, SLT>* pVS = static_cast<CqParameterTypedVaryingArray<T, I, SLT>*>( pResult );
			// Check if a valid 4 point quad, do nothing if not.
			if ( m_aValues.size() == 4 && pVS->m_aValues.size() == 4 )
			{
				pValue( 1 ) [ 0 ] = pVS->pValue( 0 ) [ 0 ] = static_cast<T>( ( pValue( 0 ) [ 0 ] + pValue( 1 ) [ 0 ] ) * 0.5 );
				pValue( 3 ) [ 0 ] = pVS->pValue( 2 ) [ 0 ] = static_cast<T>( ( pValue( 2 ) [ 0 ] + pValue( 3 ) [ 0 ] ) * 0.5 );
			}
		}
		virtual void	vSubdivide( CqParameter* pResult )
		{
			assert( pResult->Type() == Type() );
			CqParameterTypedVaryingArray<T, I, SLT>* pVS = static_cast<CqParameterTypedVaryingArray<T, I, SLT>*>( pResult );
			// Check if a valid 4 point quad, do nothing if not.
			if ( m_aValues.size() == 4 && pVS->m_aValues.size() == 4 )
			{
				pValue( 2 ) [ 0 ] = pVS->pValue( 0 ) [ 0 ] = static_cast<T>( ( pValue( 0 ) [ 0 ] + pValue( 2 ) [ 0 ] ) * 0.5 );
				pValue( 3 ) [ 0 ] = pVS->pValue( 1 ) [ 0 ] = static_cast<T>( ( pValue( 1 ) [ 0 ] + pValue( 3 ) [ 0 ] ) * 0.5 );
			}
		}
		virtual	void	BilinearDice( TqInt u, TqInt v, IqShaderData* pResult );

		// Overridden from CqParameterTyped<T>
		virtual	const	T*	pValue() const
		{
			return ( &m_aValues[ 0 ][ 0 ] );
		}
		virtual	T*	pValue()
		{
			return ( &m_aValues[ 0 ][ 0 ] );
		}
		virtual	const	T*	pValue( const TqInt Index ) const
		{
			return ( &m_aValues[ Index ][ 0 ] );
		}
		virtual	T*	pValue( const TqInt Index )
		{
			return ( &m_aValues[ Index ][ 0 ] );
		}


		/** Indexed access to array values.
		 * \param Index Integer index into the varying value list.
		 * \return A vector reference for the array of values at the specified varying index.
		 */
		const	std::vector<T>& operator[] ( const TqInt Index ) const
		{
			return( m_aValues[ Index ] );
		}
		/** Indexed access to array values.
		 * \param Index Integer index into the varying value list.
		 * \return A vector reference for the array of values at the specified varying index.
		 */
		std::vector<T>& operator[] ( const TqInt Index )
		{
			return( m_aValues[ Index ] );
		}

		/** Assignment operator.
		 */
		CqParameterTypedVaryingArray<T, I, SLT>& operator=( const CqParameterTypedVaryingArray<T, I, SLT>& From )
		{
			m_aValues.resize( From.m_aValues.size() );
			TqUint j;
			for ( j = 0; j < m_aValues.size(); j++ )
			{
				m_aValues[ j ].resize( m_Count );
				TqInt i;
				for ( i = 0; i < m_Count; i++ )
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
			TqInt i;
			for ( i = 0; i < From.m_Count; i++ )
				m_aValues[ i ] = From.m_aValues[ i ];
		}
		virtual	~CqParameterTypedUniformArray()
	{}

		virtual	CqParameter* CloneType(const char* Name, TqInt Count = 1 ) const
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

		virtual void	uSubdivide( CqParameter* pResult )
		{}
		virtual void	vSubdivide( CqParameter* pResult )
		{}
		virtual	void	BilinearDice( TqInt u, TqInt v, IqShaderData* pResult )
		{
			// Just promote the uniform value to varying by duplication.
			assert( pResult->Type() == Type() );
			assert( pResult->Class() == class_varying );
			// Note it is assumed that the variable has been
			// initialised to the correct size prior to calling.
			TqInt i;
			for ( i = 0; i < u*v; i++ )
				pResult->SetValue( pValue( 0 ) [ 0 ], i );
		}

		// Overridden from CqParameterTyped<T>
		virtual	const	T*	pValue() const
		{
			return ( &m_aValues[ 0 ] );
		}
		virtual	T*	pValue()
		{
			return ( &m_aValues[ 0 ] );
		}
		virtual	const	T*	pValue( const TqInt Index ) const
		{
			return ( &m_aValues[ Index ] );
		}
		virtual	T*	pValue( const TqInt Index )
		{
			return ( &m_aValues[ Index ] );
		}

		/** Assignment operator.
		 */
		CqParameterTypedUniformArray<T, I, SLT>& operator=( const CqParameterTypedUniformArray<T, I, SLT>& From )
		{
			m_aValues.resize( From.m_aValues.size() );
			TqInt i2 = 0;
			for ( std::vector<T>::iterator i = From.m_aValues.being(); i != From.m_aValues.end(); i++, i2++ )
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

		virtual	CqParameter* CloneType(const char* Name, TqInt Count = 1 ) const
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

		virtual void	uSubdivide( CqParameter* pResult )
		{}
		virtual void	vSubdivide( CqParameter* pResult )
		{}
		virtual	void	BilinearDice( TqInt u, TqInt v, IqShaderData* pResult )
		{
			// Just promote the constant value to varying by duplication.
			assert( pResult->Type() == Type() );
			assert( pResult->Class() == class_varying );
			// Note it is assumed that the variable has been
			// initialised to the correct size prior to calling.
			TqInt i;
			for ( i = 0; i < u*v; i++ )
				pResult->SetValue( pValue( 0 ) [ 0 ], i );
		}

		// Overridden from CqParameterTyped<T>
		virtual	const	T*	pValue() const
		{
			return ( &m_aValues[ 0 ] );
		}
		virtual	T*	pValue()
		{
			return ( &m_aValues[ 0 ] );
		}
		virtual	const	T*	pValue( const TqInt Index ) const
		{
			return ( &m_aValues[ Index ] );
		}
		virtual	T*	pValue( const TqInt Index )
		{
			return ( &m_aValues[ Index ] );
		}

		/** Assignment operator.
		 */
		CqParameterTypedConstantArray<T, I, SLT>& operator=( const CqParameterTypedUniformArray<T, I, SLT>& From )
		{
			m_aValues.resize( From.m_aValues.size() );
			TqInt i2 = 0;
			for ( std::vector<T>::iterator i = From.m_aValues.being(); i != From.m_aValues.end(); i++, i2++ )
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

		virtual	CqParameter* CloneType(const char* Name, TqInt Count = 1 ) const
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

		virtual	CqParameter* CloneType(const char* Name, TqInt Count = 1 ) const
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
 */

template <class T, EqVariableType I, class SLT>
void CqParameterTypedVarying<T, I, SLT>::BilinearDice( TqInt u, TqInt v, IqShaderData* pResult )
{
	assert( pResult->Class() == class_varying );
	
	T res;
	
	// Not much point bilinear dicing into a uniform storage.
	assert( pResult->Size() > 1 );

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
				(*pResData++) = res;
			}
		}
	}
	else
	{
		TqInt i = 0;
		TqInt iv;
		res = pValue( 0 ) [ 0 ];
		for ( iv = 0; iv <= v; iv++ )
		{
			TqInt iu;
			for ( iu = 0; iu <= u; iu++ )
				(*pResData++) = res;
		}
	}
}


/** Dice the value into a grid using bilinear interpolation.
 * \param u Integer dice count for the u direction.
 * \param v Integer dice count for the v direction.
 * \param pResult Pointer to storage for the result.
 */

template <class T, EqVariableType I, class SLT>
void CqParameterTypedVaryingArray<T, I, SLT>::BilinearDice( TqInt u, TqInt v, IqShaderData* pResult )
{
	assert( pResult->Type() == Type() );
	assert( pResult->Class() == class_varying );
	
	T res;

	// Not much point bilinear dicing into a uniform storage.
	assert( pResult->Size() > 1 );

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
				(*pResData++) = res;
			}
		}
	}
	else
	{
		TqInt i = 0;
		TqInt iv;
		res = pValue( 0 ) [ 0 ];
		for ( iv = 0; iv <= v; iv++ )
		{
			TqInt iu;
			for ( iu = 0; iu <= u; iu++ )
				(*pResData++) = res;
		}
	}
}


_qShareM	extern CqParameter* ( *gVariableCreateFuncsConstant[] ) ( const char* strName, TqInt Count );
_qShareM	extern CqParameter* ( *gVariableCreateFuncsUniform[] ) ( const char* strName, TqInt Count );
_qShareM	extern CqParameter* ( *gVariableCreateFuncsVarying[] ) ( const char* strName, TqInt Count );
_qShareM	extern CqParameter* ( *gVariableCreateFuncsVertex[] ) ( const char* strName, TqInt Count );
_qShareM	extern CqParameter* ( *gVariableCreateFuncsFaceVarying[] ) ( const char* strName, TqInt Count );
_qShareM	extern CqParameter* ( *gVariableCreateFuncsConstantArray[] ) ( const char* strName, TqInt Count );
_qShareM	extern CqParameter* ( *gVariableCreateFuncsUniformArray[] ) ( const char* strName, TqInt Count );
_qShareM	extern CqParameter* ( *gVariableCreateFuncsVaryingArray[] ) ( const char* strName, TqInt Count );
_qShareM	extern CqParameter* ( *gVariableCreateFuncsVertexArray[] ) ( const char* strName, TqInt Count );
_qShareM	extern CqParameter* ( *gVariableCreateFuncsFaceVaryingArray[] ) ( const char* strName, TqInt Count );

//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !PARAMETERS_H_INCLUDED
