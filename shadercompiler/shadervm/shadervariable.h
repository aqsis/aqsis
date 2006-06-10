// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
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

#include	"aqsis.h"

#include	"bitvector.h"
#include	"sstring.h"
#include	"vector3d.h"
#include	"matrix.h"
#include	"color.h"
#include	"stats.h"

#include	"ishaderdata.h"


START_NAMESPACE( Aqsis )


//----------------------------------------------------------------------
/** \class CqShaderVariable
 
 * Abstract base class from which all shaders variables must be defined.
 
 */

class CqShaderVariable : public IqShaderData
{
	public:
		CqShaderVariable();
		CqShaderVariable( const char* strName, TqBool fParameter = TqFalse );
		virtual	~CqShaderVariable();

		virtual void GetBool( TqBool& res, TqInt index = 0 ) const
		{
			assert( TqFalse );
		}
		virtual void GetBoolPtr( const TqBool*& res ) const
		{
			assert( TqFalse );
		}
		virtual void GetBoolPtr( TqBool*& res )
		{
			assert( TqFalse );
		}
		virtual void SetBool( const TqBool& val )
		{
			assert( TqFalse );
		}
		virtual void SetBool( const TqBool& val, TqInt index )
		{
			assert( TqFalse );
		}

		/** Get the name of this variable.
		 * \return Read only reference to a CqString class.
		 */
		virtual const CqString&	strName()
		{
			return ( m_strName );
		}

		/** Determine if this variable is storage for a shader argument.
		 */
		virtual TqBool fParameter() const
		{
			return ( m_fParameter );
		}

		/** Indicate whether or not this variable is a shader argument.
		 */
		void SetfParameter( TqBool fParameter = TqTrue )
		{
			m_fParameter = fParameter;
		}

		virtual TqInt	ArrayLength() const
		{
			return ( 0 );
		}
		virtual IqShaderData*	ArrayEntry( TqInt i )
		{
			return ( this );
		}

	protected:
		CqString	m_strName;		///< Name of this variable.
		TqBool	m_fParameter;	///< Flag indicating this variable is a shader argument.
}
;


//----------------------------------------------------------------------
/** \class CqShaderVariableArray
 
 * Array of variable pointers.
 
 */

class CqShaderVariableArray: public CqShaderVariable
{
	public:
		/** Default constructor.
		 * \param name Character pointer to the name to use.
		 * \param Count the size of the array.
		 * \param fParameter Boolean value, true indicates the shader variable is a shader parameter.
		 */
		CqShaderVariableArray( const char* name, TqInt Count, TqBool fParameter = TqFalse ) : CqShaderVariable( name, fParameter )
		{
			assert( Count > 0 );
			m_aVariables.resize( Count );
		}
		/** Copy constructor.
		 */
		CqShaderVariableArray( const CqShaderVariableArray& From ) : CqShaderVariable( From.m_strName.c_str() )
		{
			m_aVariables.resize( From.m_aVariables.size() );
			for ( TqUint i = 0; i < From.m_aVariables.size(); i++ )
				m_aVariables[ i ] = From.m_aVariables[ i ] ->Clone();
		}
		virtual	~CqShaderVariableArray()
		{
		}

		// Overridden from IqShaderData.


		virtual	void	Initialise( const TqInt varyingSize )
		{
			for ( std::vector<IqShaderData*>::iterator i = m_aVariables.begin(); i != m_aVariables.end(); i++ )
				( *i ) ->Initialise( varyingSize );
		}

		virtual	void	SetSize( const TqUint size )
		{
			for ( std::vector<IqShaderData*>::iterator i = m_aVariables.begin(); i != m_aVariables.end(); i++ )
				( *i ) ->SetSize( size );
		}


		virtual	void	GetFloat( TqFloat& res, TqInt index = 0 ) const
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->GetFloat( res, index );
		}
		virtual	void	GetString( CqString& res, TqInt index = 0 ) const
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->GetString( res, index );
		}
		virtual	void	GetPoint( CqVector3D& res, TqInt index = 0 ) const
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->GetPoint( res, index );
		}
		virtual	void	GetVector( CqVector3D& res, TqInt index = 0 ) const
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->GetVector( res, index );
		}
		virtual	void	GetNormal( CqVector3D& res, TqInt index = 0 ) const
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->GetNormal( res, index );
		}
		virtual	void	GetColor( CqColor& res, TqInt index = 0 ) const
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->GetColor( res, index );
		}
		virtual	void	GetMatrix( CqMatrix& res, TqInt index = 0 ) const
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->GetMatrix( res, index );
		}

		virtual	void	GetFloatPtr( const TqFloat*& res ) const
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->GetFloatPtr( res );
		}
		virtual	void	GetStringPtr( const CqString*& res ) const
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->GetStringPtr( res );
		}
		virtual	void	GetPointPtr( const CqVector3D*& res ) const
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->GetPointPtr( res );
		}
		virtual	void	GetVectorPtr( const CqVector3D*& res ) const
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->GetVectorPtr( res );
		}
		virtual	void	GetNormalPtr( const CqVector3D*& res ) const
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->GetNormalPtr( res );
		}
		virtual	void	GetColorPtr( const CqColor*& res ) const
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->GetColorPtr( res );
		}
		virtual	void	GetMatrixPtr( const CqMatrix*& res ) const
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->GetMatrixPtr( res );
		}

		virtual	void	GetFloatPtr( TqFloat*& res )
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->GetFloatPtr( res );
		}
		virtual	void	GetStringPtr( CqString*& res )
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->GetStringPtr( res );
		}
		virtual	void	GetPointPtr( CqVector3D*& res )
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->GetPointPtr( res );
		}
		virtual	void	GetVectorPtr( CqVector3D*& res )
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->GetVectorPtr( res );
		}
		virtual	void	GetNormalPtr( CqVector3D*& res )
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->GetNormalPtr( res );
		}
		virtual	void	GetColorPtr( CqColor*& res )
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->GetColorPtr( res );
		}
		virtual	void	GetMatrixPtr( CqMatrix*& res )
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->GetMatrixPtr( res );
		}

		virtual	void	SetFloat( const TqFloat& f )
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->SetFloat( f );
		}
		virtual	void	SetString( const CqString& s )
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->SetString( s );
		}
		virtual	void	SetPoint( const CqVector3D& p )
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->SetPoint( p );
		}
		virtual	void	SetVector( const CqVector3D& v )
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->SetVector( v );
		}
		virtual	void	SetNormal( const CqVector3D& n )
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->SetNormal( n );
		}
		virtual	void	SetColor( const CqColor& c )
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->SetColor( c );
		}
		virtual	void	SetMatrix( const CqMatrix& m )
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->SetMatrix( m );
		}

		virtual	void	SetFloat( const TqFloat& f, TqInt index )
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->SetFloat( f, index );
		}
		virtual	void	SetString( const CqString& s, TqInt index )
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->SetString( s, index );
		}
		virtual	void	SetPoint( const CqVector3D& p, TqInt index )
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->SetPoint( p, index );
		}
		virtual	void	SetVector( const CqVector3D& v, TqInt index )
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->SetVector( v, index );
		}
		virtual	void	SetNormal( const CqVector3D& n, TqInt index )
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->SetNormal( n, index );
		}
		virtual	void	SetColor( const CqColor& c, TqInt index )
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->SetColor( c, index );
		}
		virtual	void	SetMatrix( const CqMatrix& m, TqInt index )
		{
			assert( TqFalse );
			m_aVariables[ 0 ] ->SetMatrix( m, index );
		}

		/** Copy the values from the passed variable into this, taking into account any class differences.
		 * \param pVal The variable to copy from.
		 */
		virtual	void	SetValueFromVariable( const IqShaderData* pVal )
		{
			m_aVariables[ 0 ] ->SetValueFromVariable( pVal );
		}
		/** Set the all SIMD data ased on a state vector, only indexes whose bit is set are modified.
		 * \param pVal The stack entry to assign.
		 * \param index Integer SIMD index.
		 */
		virtual	void	SetValueFromVariable( const IqShaderData* pVal, TqInt index )
		{
			m_aVariables[ 0 ] ->SetValueFromVariable( pVal, index );
		}

		virtual	IqShaderData* Clone() const
		{
			return ( new CqShaderVariableArray( *this ) );
		}


		virtual	EqVariableClass	Class() const
		{
			return ( static_cast<EqVariableClass>( m_aVariables[ 0 ] ->Class() ) );
		}
		virtual	EqVariableType	Type() const
		{
			return ( static_cast<EqVariableType>( m_aVariables[ 0 ] ->Type() ) );
		}
		virtual TqUint	Size() const
		{
			return ( m_aVariables[ 0 ] ->Size() );
		}
		virtual	TqInt	ArrayLength() const
		{
			return ( m_aVariables.size() );
		}
		virtual IqShaderData*	ArrayEntry( TqInt i )
		{
			assert( i < this->ArrayLength() );
			return ( ( *this ) [ i ] );
		}

		/** Get a reference to the variable array.
		 */
		std::vector<IqShaderData*>& aVariables()
		{
			return ( m_aVariables );
		}
		/** Get a const reference to the variable array.
		 */
		const std::vector<IqShaderData*>& aVariables() const
		{
			return ( m_aVariables );
		}
		/** Array index access to the values in the array.
		 * \param index Integer index intot he array.
		 */
		IqShaderData* operator[] ( TqUint index ) const
		{
			return( m_aVariables[ index ] );
		}

	private:
		std::vector<IqShaderData*>	m_aVariables;		///< Array of pointers to variables.
}
;



//----------------------------------------------------------------------
/** \class CqShaderVariableUniform
 
 * Uniform variable templatised by type.
 
 */

template <const EqVariableType T, class R>
class CqShaderVariableUniform : public CqShaderVariable
{
	public:
		CqShaderVariableUniform() : CqShaderVariable()
		{
		}
		CqShaderVariableUniform( const char* strName, TqBool fParameter = TqFalse ) : CqShaderVariable( strName, fParameter )
		{

		}
		CqShaderVariableUniform( const char* strName, const R& val ) : CqShaderVariable( strName ),
				m_Value( val )
		{

		}
		CqShaderVariableUniform( const CqShaderVariableUniform<T, R>& val ) :
				CqShaderVariable( val ),
				m_Value( val.m_Value )
		{
		}
		virtual	~CqShaderVariableUniform()
		{
		}

		virtual	void	Initialise( const TqInt varyingSize )
		{
		}
		virtual	void	SetSize( const TqUint size )
		{}


		virtual	void	GetFloat( TqFloat& res, TqInt index = 0 ) const
		{
			assert( TqFalse );
		}
		virtual	void	GetString( CqString& res, TqInt index = 0 ) const
		{
			assert( TqFalse );
		}
		virtual	void	GetPoint( CqVector3D& res, TqInt index = 0 ) const
		{
			assert( TqFalse );
		}
		virtual	void	GetVector( CqVector3D& res, TqInt index = 0 ) const
		{
			assert( TqFalse );
		}
		virtual	void	GetNormal( CqVector3D& res, TqInt index = 0 ) const
		{
			assert( TqFalse );
		}
		virtual	void	GetColor( CqColor& res, TqInt index = 0 ) const
		{
			assert( TqFalse );
		}
		virtual	void	GetMatrix( CqMatrix& res, TqInt index = 0 ) const
		{
			assert( TqFalse );
		}

		virtual	void	GetFloatPtr( const TqFloat*& res ) const
		{
			assert( TqFalse );
		}
		virtual	void	GetStringPtr( const CqString*& res ) const
		{
			assert( TqFalse );
		}
		virtual	void	GetPointPtr( const CqVector3D*& res ) const
		{
			assert( TqFalse );
		}
		virtual	void	GetVectorPtr( const CqVector3D*& res ) const
		{
			assert( TqFalse );
		}
		virtual	void	GetNormalPtr( const CqVector3D*& res ) const
		{
			assert( TqFalse );
		}
		virtual	void	GetColorPtr( const CqColor*& res ) const
		{
			assert( TqFalse );
		}
		virtual	void	GetMatrixPtr( const CqMatrix*& res ) const
		{
			assert( TqFalse );
		}

		virtual	void	GetFloatPtr( TqFloat*& res )
		{
			assert( TqFalse );
		}
		virtual	void	GetStringPtr( CqString*& res )
		{
			assert( TqFalse );
		}
		virtual	void	GetPointPtr( CqVector3D*& res )
		{
			assert( TqFalse );
		}
		virtual	void	GetVectorPtr( CqVector3D*& res )
		{
			assert( TqFalse );
		}
		virtual	void	GetNormalPtr( CqVector3D*& res )
		{
			assert( TqFalse );
		}
		virtual	void	GetColorPtr( CqColor*& res )
		{
			assert( TqFalse );
		}
		virtual	void	GetMatrixPtr( CqMatrix*& res )
		{
			assert( TqFalse );
		}


		virtual	void	SetFloat( const TqFloat& f )
		{
			assert( TqFalse );
		}
		virtual	void	SetString( const CqString& s )
		{
			assert( TqFalse );
		}
		virtual	void	SetPoint( const CqVector3D& p )
		{
			assert( TqFalse );
		}
		virtual	void	SetVector( const CqVector3D& v )
		{
			assert( TqFalse );
		}
		virtual	void	SetNormal( const CqVector3D& n )
		{
			assert( TqFalse );
		}
		virtual	void	SetColor( const CqColor& c )
		{
			assert( TqFalse );
		}
		virtual	void	SetMatrix( const CqMatrix& m )
		{
			assert( TqFalse );
		}

		virtual	void	SetFloat( const TqFloat& f, TqInt index )
		{
			SetFloat( f );
		}
		virtual	void	SetString( const CqString& s, TqInt index )
		{
			SetString( s );
		}
		virtual	void	SetPoint( const CqVector3D& p, TqInt index )
		{
			SetPoint( p );
		}
		virtual	void	SetVector( const CqVector3D& v, TqInt index )
		{
			SetVector( v );
		}
		virtual	void	SetNormal( const CqVector3D& n, TqInt index )
		{
			SetNormal( n );
		}
		virtual	void	SetColor( const CqColor& c, TqInt index )
		{
			SetColor( c );
		}
		virtual	void	SetMatrix( const CqMatrix& m, TqInt index )
		{
			SetMatrix( m );
		}

		/** Set the all SIMD data ased on a state vector, only indexes whose bit is set are modified.
		 * \param pVal The stack entry to assign.
		 * \param index Integer SIMD index.
		 */
		virtual	void	SetValueFromVariable( const IqShaderData* pVal, TqInt index )
		{
			SetValueFromVariable( pVal );
		}
		/** Copy the values from the passed variable into this, taking into account any class differences.
		 * \param pVal The variable to copy from.
		 */
		virtual	void	SetValueFromVariable( const IqShaderData* pVal )
		{
			assert( TqFalse );
		}

		virtual	EqVariableClass	Class() const
		{
			return ( class_uniform );
		}
		virtual	EqVariableType	Type() const
		{
			return ( T );
		}
		virtual TqUint	Size() const
		{
			return ( 1 );
		}

		virtual	void	operator=( const CqShaderVariableUniform<T, R>& From )
		{
			m_Value = From.m_Value;
		}

	protected:
		R	m_Value;	///< Simgle uniform value of the appropriate type.
}
;


class CqShaderVariableUniformFloat : public CqShaderVariableUniform<type_float, TqFloat>
{
	public:
		CqShaderVariableUniformFloat( ) : CqShaderVariableUniform<type_float, TqFloat>()
		{
		}
		CqShaderVariableUniformFloat( const char* strName, TqBool fParameter = TqFalse ) : CqShaderVariableUniform<type_float, TqFloat>( strName, fParameter )
		{
		}
		virtual	void	GetFloat( TqFloat& res, TqInt index = 0 ) const
		{
			res = m_Value;
		}
		virtual	void	GetFloatPtr( const TqFloat*& res ) const
		{
			res = & m_Value;
		}
		virtual	void	SetFloat( const TqFloat& f )
		{
			m_Value = f;
		}
		virtual void	GetBool( TqBool& res, TqInt index = 0 ) const
		{
			res = m_Value != 0.0f;
		}
		virtual void	SetBool( const TqBool& val )
		{
			m_Value = val;
		}
		virtual void	SetBool( const TqBool& val, TqInt index )
		{
			m_Value = val;
		}
		/** Copy the values from the passed variable into this, taking into account any class differences.
		 * \param pVal The variable to copy from.
		 */
		virtual	void	SetValueFromVariable( const IqShaderData* pVal )
		{
			pVal->GetFloat( m_Value );
		}
		virtual	IqShaderData* Clone() const
		{
			CqShaderVariableUniformFloat* newVar = new CqShaderVariableUniformFloat( *this );
			newVar->SetSize( Size() );
			newVar->SetValueFromVariable( this );
			return ( newVar );
		}
};

class CqShaderVariableUniformString : public CqShaderVariableUniform<type_string, CqString>
{
	public:
		CqShaderVariableUniformString( ) : CqShaderVariableUniform<type_string, CqString>()
		{
		}
		CqShaderVariableUniformString( const char* strName, TqBool fParameter = TqFalse ) : CqShaderVariableUniform<type_string, CqString>( strName, fParameter )
		{
		}
		virtual	void	GetString( CqString& res, TqInt index = 0 ) const
		{
			res = m_Value;
		}
		virtual	void	GetStringPtr( const CqString*& res ) const
		{
			res = &m_Value;
		}
		virtual	void	SetString( const CqString& s )
		{
			m_Value = s;
		}
		virtual void	GetBool( TqBool& res, TqInt index = 0 ) const
		{
			res = m_Value.compare( "" ) == 0;
		}
		/** Copy the values from the passed variable into this, taking into account any class differences.
		 * \param pVal The variable to copy from.
		 */
		virtual	void	SetValueFromVariable( const IqShaderData* pVal )
		{
			pVal->GetString( m_Value );
		}
		virtual	IqShaderData* Clone() const
		{
			CqShaderVariableUniformString* newVar = new CqShaderVariableUniformString( *this );
			newVar->SetSize( Size() );
			newVar->SetValueFromVariable( this );
			return ( newVar );
		}
};

class CqShaderVariableUniformPoint : public CqShaderVariableUniform<type_point, CqVector3D>
{
	public:
		CqShaderVariableUniformPoint() : CqShaderVariableUniform<type_point, CqVector3D>()
		{
		}
		CqShaderVariableUniformPoint( const char* strName, TqBool fParameter = TqFalse ) : CqShaderVariableUniform<type_point, CqVector3D>( strName, fParameter )
		{
		}
		virtual	void	GetPoint( CqVector3D& res, TqInt index = 0 ) const
		{
			res = m_Value;
		}
		virtual	void	GetPointPtr( const CqVector3D*& res ) const
		{
			res = &m_Value;
		}
		virtual	void	GetPointPtr( CqVector3D*& res )
		{
			res = &m_Value;
		}
		virtual	void	SetPoint( const CqVector3D& p )
		{
			m_Value = p;
		}
		virtual	void	GetVector( CqVector3D& res, TqInt index = 0 ) const
		{
			res = m_Value;
		}
		virtual	void	GetVectorPtr( const CqVector3D*& res ) const
		{
			res = &m_Value;
		}
		virtual	void	SetVector( const CqVector3D& p )
		{
			m_Value = p;
		}
		virtual	void	GetNormal( CqVector3D& res, TqInt index = 0 ) const
		{
			res = m_Value;
		}
		virtual	void	GetNormalPtr( const CqVector3D*& res ) const
		{
			res = &m_Value;
		}
		virtual	void	SetNormal( const CqVector3D& p )
		{
			m_Value = p;
		}
		virtual void	GetBool( TqBool& res, TqInt index = 0 ) const
		{
			res = ( ( m_Value.x() != 0.0f ) || ( m_Value.y() != 0.0f ) || ( m_Value.z() != 0.0f ) );
		}
		/** Copy the values from the passed variable into this, taking into account any class differences.
		 * \param pVal The variable to copy from.
		 */
		virtual	void	SetValueFromVariable( const IqShaderData* pVal )
		{
			pVal->GetPoint( m_Value );
		}
		virtual	IqShaderData* Clone() const
		{
			CqShaderVariableUniformPoint* newVar = new CqShaderVariableUniformPoint( *this );
			newVar->SetSize( Size() );
			newVar->SetValueFromVariable( this );
			return ( newVar );
		}
};

class CqShaderVariableUniformVector : public CqShaderVariableUniform<type_vector, CqVector3D>
{
	public:
		CqShaderVariableUniformVector() : CqShaderVariableUniform<type_vector, CqVector3D>()
		{
		}
		CqShaderVariableUniformVector( const char* strName, TqBool fParameter = TqFalse ) : CqShaderVariableUniform<type_vector, CqVector3D>( strName, fParameter )
		{
		}
		virtual	void	GetPoint( CqVector3D& res, TqInt index = 0 ) const
		{
			res = m_Value;
		}
		virtual	void	GetPointPtr( const CqVector3D*& res ) const
		{
			res = &m_Value;
		}
		virtual	void	GetPointPtr( CqVector3D*& res )
		{
			res = &m_Value;
		}
		virtual	void	SetPoint( const CqVector3D& p )
		{
			m_Value = p;
		}
		virtual	void	GetVector( CqVector3D& res, TqInt index = 0 ) const
		{
			res = m_Value;
		}
		virtual	void	GetVectorPtr( const CqVector3D*& res ) const
		{
			res = &m_Value;
		}
		virtual	void	SetVector( const CqVector3D& p )
		{
			m_Value = p;
		}
		virtual	void	GetNormal( CqVector3D& res, TqInt index = 0 ) const
		{
			res = m_Value;
		}
		virtual	void	GetNormalPtr( const CqVector3D*& res ) const
		{
			res = &m_Value;
		}
		virtual	void	SetNormal( const CqVector3D& p )
		{
			m_Value = p;
		}
		virtual void	GetBool( TqBool& res, TqInt index = 0 ) const
		{
			res = ( ( m_Value.x() != 0.0f ) || ( m_Value.y() != 0.0f ) || ( m_Value.z() != 0.0f ) );
		}
		/** Copy the values from the passed variable into this, taking into account any class differences.
		 * \param pVal The variable to copy from.
		 */
		virtual	void	SetValueFromVariable( const IqShaderData* pVal )
		{
			pVal->GetVector( m_Value );
		}
		virtual	IqShaderData* Clone() const
		{
			CqShaderVariableUniformVector* newVar = new CqShaderVariableUniformVector( *this );
			newVar->SetSize( Size() );
			newVar->SetValueFromVariable( this );
			return ( newVar );
		}
};

class CqShaderVariableUniformNormal : public CqShaderVariableUniform<type_normal, CqVector3D>
{
	public:
		CqShaderVariableUniformNormal() : CqShaderVariableUniform<type_normal, CqVector3D>()
		{
		}
		CqShaderVariableUniformNormal( const char* strName, TqBool fParameter = TqFalse ) : CqShaderVariableUniform<type_normal, CqVector3D>( strName, fParameter )
		{}
		virtual	void	GetPoint( CqVector3D& res, TqInt index = 0 ) const
		{
			res = m_Value;
		}
		virtual	void	GetPointPtr( const CqVector3D*& res ) const
		{
			res = &m_Value;
		}
		virtual	void	GetPointPtr( CqVector3D*& res )
		{
			res = &m_Value;
		}
		virtual	void	SetPoint( const CqVector3D& p )
		{
			m_Value = p;
		}
		virtual	void	GetVector( CqVector3D& res, TqInt index = 0 ) const
		{
			res = m_Value;
		}
		virtual	void	GetVectorPtr( const CqVector3D*& res ) const
		{
			res = &m_Value;
		}
		virtual	void	SetVector( const CqVector3D& p )
		{
			m_Value = p;
		}
		virtual	void	GetNormal( CqVector3D& res, TqInt index = 0 ) const
		{
			res = m_Value;
		}
		virtual	void	GetNormalPtr( const CqVector3D*& res ) const
		{
			res = &m_Value;
		}
		virtual	void	SetNormal( const CqVector3D& p )
		{
			m_Value = p;
		}
		virtual void	GetBool( TqBool& res, TqInt index = 0 ) const
		{
			res = ( ( m_Value.x() != 0.0f ) || ( m_Value.y() != 0.0f ) || ( m_Value.z() != 0.0f ) );
		}
		/** Copy the values from the passed variable into this, taking into account any class differences.
		 * \param pVal The variable to copy from.
		 */
		virtual	void	SetValueFromVariable( const IqShaderData* pVal )
		{
			pVal->GetNormal( m_Value );
		}
		virtual	IqShaderData* Clone() const
		{
			CqShaderVariableUniformNormal* newVar = new CqShaderVariableUniformNormal( *this );
			newVar->SetSize( Size() );
			newVar->SetValueFromVariable( this );
			return ( newVar );
		}
};

class CqShaderVariableUniformColor : public CqShaderVariableUniform<type_color, CqColor>
{
	public:
		CqShaderVariableUniformColor() : CqShaderVariableUniform<type_color, CqColor>()
		{
		}
		CqShaderVariableUniformColor( const char* strName, TqBool fParameter = TqFalse ) : CqShaderVariableUniform<type_color, CqColor>( strName, fParameter )
		{
		}
		virtual	void	GetColor( CqColor& res, TqInt index = 0 ) const
		{
			res = m_Value;
		}
		virtual	void	GetColorPtr( const CqColor*& res ) const
		{
			res = &m_Value;
		}
		virtual	void	SetColor( const CqColor& c )
		{
			m_Value = c;
		}
		/** Copy the values from the passed variable into this, taking into account any class differences.
		 * \param pVal The variable to copy from.
		 */
		virtual	void	SetValueFromVariable( const IqShaderData* pVal )
		{
			pVal->GetColor( m_Value );
		}
		virtual void	GetBool( TqBool& res, TqInt index = 0 ) const
		{
			res = ( ( m_Value.fRed() != 0.0f ) || ( m_Value.fGreen() != 0.0f ) || ( m_Value.fBlue() != 0.0f ) );
		}
		virtual	IqShaderData* Clone() const
		{
			CqShaderVariableUniformColor* newVar = new CqShaderVariableUniformColor( *this );
			newVar->SetSize( Size() );
			newVar->SetValueFromVariable( this );
			return ( newVar );
		}
};

class CqShaderVariableUniformMatrix : public CqShaderVariableUniform<type_matrix, CqMatrix>
{
	public:
		CqShaderVariableUniformMatrix() : CqShaderVariableUniform<type_matrix, CqMatrix>()
		{
		}
		CqShaderVariableUniformMatrix( const char* strName, TqBool fParameter = TqFalse ) : CqShaderVariableUniform<type_matrix, CqMatrix>( strName, fParameter )
		{
		}
		virtual	void	GetMatrix( CqMatrix& res, TqInt index = 0 ) const
		{
			res = m_Value;
		}
		virtual	void	GetMatrixPtr( const CqMatrix*& res ) const
		{
			res = &m_Value;
		}
		virtual	void	SetMatrix( const CqMatrix& m )
		{
			m_Value = m;
		}
		/** Copy the values from the passed variable into this, taking into account any class differences.
		 * \param pVal The variable to copy from.
		 */
		virtual	void	SetValueFromVariable( const IqShaderData* pVal )
		{
			pVal->GetMatrix( m_Value );
		}
		virtual	IqShaderData* Clone() const
		{
			CqShaderVariableUniformMatrix* newVar = new CqShaderVariableUniformMatrix( *this );
			newVar->SetSize( Size() );
			newVar->SetValueFromVariable( this );
			return ( newVar );
		}
};


//----------------------------------------------------------------------
/** \class CqShaderVariableVarying
 
 * Varying variable templatised by type.
 
 */

template <const EqVariableType T, class R>
class CqShaderVariableVarying : public CqShaderVariable
{
	public:
		CqShaderVariableVarying( ) : CqShaderVariable( )
		{
			m_aValue.resize( 1 );
		}

		CqShaderVariableVarying( const char* strName, TqBool fParameter = TqFalse ) : CqShaderVariable( strName, fParameter )
		{
			m_aValue.resize( 1 );
		}
		CqShaderVariableVarying( const char* strName, const R& val ) : CqShaderVariable( strName )
		{
			m_aValue.resize( 1 );
			m_aValue[ 0 ] = val;
		}
		CqShaderVariableVarying( const CqShaderVariableVarying<T, R>& val ) : CqShaderVariable( val )
		{
			m_aValue.resize( val.m_aValue.size() );
			m_aValue.assign( val.m_aValue.begin(), val.m_aValue.end() );
		}
		virtual	~CqShaderVariableVarying()
		{
		}

		virtual	void	Initialise( const TqInt varyingSize )
		{
			R Def;
			if ( m_aValue.size() > 0 )
				Def = m_aValue[ 0 ];
			m_aValue.assign( varyingSize, Def );
		}

		virtual	void	SetSize( const TqUint size )
		{
			m_aValue.resize( size );
		}

		virtual	void	GetFloat( TqFloat &res, TqInt index = 0 ) const
		{
			assert( TqFalse );
		}
		virtual	void	GetString( CqString& res, TqInt index = 0 ) const
		{
			assert( TqFalse );
		}
		virtual	void	GetPoint( CqVector3D& res, TqInt index = 0 ) const
		{
			assert( TqFalse );
		}
		virtual	void	GetVector( CqVector3D& res, TqInt index = 0 ) const
		{
			assert( TqFalse );
		}
		virtual	void	GetNormal( CqVector3D& res, TqInt index = 0 ) const
		{
			assert( TqFalse );
		}
		virtual	void	GetColor( CqColor& res, TqInt index = 0 ) const
		{
			assert( TqFalse );
		}
		virtual	void	GetMatrix( CqMatrix& res, TqInt index = 0 ) const
		{
			assert( TqFalse );
		}

		virtual	void	GetFloatPtr( const TqFloat*& res ) const
		{
			assert( TqFalse );
		}
		virtual	void	GetStringPtr( const CqString*& res ) const
		{
			assert( TqFalse );
		}
		virtual	void	GetPointPtr( const CqVector3D*& res ) const
		{
			assert( TqFalse );
		}
		virtual	void	GetVectorPtr( const CqVector3D*& res ) const
		{
			assert( TqFalse );
		}
		virtual	void	GetNormalPtr( const CqVector3D*& res ) const
		{
			assert( TqFalse );
		}
		virtual	void	GetColorPtr( const CqColor*& res ) const
		{
			assert( TqFalse );
		}
		virtual	void	GetMatrixPtr( const CqMatrix*& res ) const
		{
			assert( TqFalse );
		}

		virtual	void	GetFloatPtr( TqFloat*& res )
		{
			assert( TqFalse );
		}
		virtual	void	GetStringPtr( CqString*& res )
		{
			assert( TqFalse );
		}
		virtual	void	GetPointPtr( CqVector3D*& res )
		{
			assert( TqFalse );
		}
		virtual	void	GetVectorPtr( CqVector3D*& res )
		{
			assert( TqFalse );
		}
		virtual	void	GetNormalPtr( CqVector3D*& res )
		{
			assert( TqFalse );
		}
		virtual	void	GetColorPtr( CqColor*& res )
		{
			assert( TqFalse );
		}
		virtual	void	GetMatrixPtr( CqMatrix*& res )
		{
			assert( TqFalse );
		}

		virtual	void	SetFloat( const TqFloat& f )
		{
			assert( TqFalse );
		}
		virtual	void	SetString( const CqString& s )
		{
			assert( TqFalse );
		}
		virtual	void	SetPoint( const CqVector3D& p )
		{
			assert( TqFalse );
		}
		virtual	void	SetVector( const CqVector3D& v )
		{
			assert( TqFalse );
		}
		virtual	void	SetNormal( const CqVector3D& n )
		{
			assert( TqFalse );
		}
		virtual	void	SetColor( const CqColor& c )
		{
			assert( TqFalse );
		}
		virtual	void	SetMatrix( const CqMatrix& m )
		{
			assert( TqFalse );
		}

		virtual	void	SetFloat( const TqFloat& f, TqInt index )
		{
			assert( TqFalse );
		}
		virtual	void	SetString( const CqString& s, TqInt index )
		{
			assert( TqFalse );
		}
		virtual	void	SetPoint( const CqVector3D& p, TqInt index )
		{
			assert( TqFalse );
		}
		virtual	void	SetVector( const CqVector3D& v, TqInt index )
		{
			assert( TqFalse );
		}
		virtual	void	SetNormal( const CqVector3D& n, TqInt index )
		{
			assert( TqFalse );
		}
		virtual	void	SetColor( const CqColor& c, TqInt index )
		{
			assert( TqFalse );
		}
		virtual	void	SetMatrix( const CqMatrix& m, TqInt index )
		{
			assert( TqFalse );
		}

		virtual	EqVariableClass	Class() const
		{
			return ( class_varying );
		}
		virtual	EqVariableType	Type() const
		{
			return ( T );
		}
		virtual TqUint	Size() const
		{
			return ( m_aValue.size() );
		}

		virtual	void	operator=( const CqShaderVariableVarying<T, R>& From )
		{
			// use std::vector assignment
			m_aValue = From.m_aValue;
		}

	protected:
		std::vector<R>	m_aValue;		///< Array of values of the appropriate type.
		R	m_temp_R;		///< Temp value to use in template functions, problem with VC++.
}
;


class CqShaderVariableVaryingFloat : public CqShaderVariableVarying<type_float, TqFloat>
{
	public:
		CqShaderVariableVaryingFloat( ) : CqShaderVariableVarying<type_float, TqFloat>( )
		{
		}
		CqShaderVariableVaryingFloat( const char* strName, TqBool fParameter = TqFalse ) : CqShaderVariableVarying<type_float, TqFloat>( strName, fParameter )
		{
		}
		virtual	void	GetFloat( TqFloat& res, TqInt index = 0 ) const
		{
			res = m_aValue[ index ];
		}
		virtual	void	GetFloatPtr( const TqFloat*& res ) const
		{
			res = &m_aValue[ 0 ];
		}
		virtual	void	GetFloatPtr( TqFloat*& res )
		{
			res = &m_aValue[ 0 ];
		}
		virtual	void	SetFloat( const TqFloat& f, TqInt index )
		{
			m_aValue[ index ] = f;
		}
		virtual	void	SetFloat( const TqFloat& f )
		{
			m_aValue.assign( m_aValue.size(), f );
		}
		/** Copy the values from the passed variable into this, taking into account any class differences.
		 * \param pVal The variable to copy from.
		 */
		virtual	void	SetValueFromVariable( const IqShaderData* pVal )
		{
			TqUint i;
			if ( pVal->Size() > 1 )
			{
				const TqFloat * pData;
				pVal->GetFloatPtr( pData );
				for ( i = 0; i < Size(); i++ )
					m_aValue[ i ] = pData [ i ];
			}
			else
			{
				TqFloat temp;
				pVal->GetFloat( temp );
				m_aValue.assign( m_aValue.size(), temp );
			}
		}
		virtual void	GetBool( TqBool& res, TqInt index = 0 ) const
		{
			res = m_aValue[ index ] != 0.0f;
		}
		virtual void	SetBool( const TqBool& val )
		{
			m_aValue.assign( m_aValue.size(), val );
		}
		virtual void	SetBool( const TqBool& val, TqInt index )
		{
			m_aValue[ index ] = val;
		}
		/** Set the all SIMD data ased on a state vector, only indexes whose bit is set are modified.
		 * \param pVal The stack entry to assign.
		 * \param index Integer SIMD index.
		 */
		virtual	void	SetValueFromVariable( const IqShaderData* pVal, TqInt index )
		{
			pVal->GetFloat( m_aValue[ index ], index );
		}
		virtual	IqShaderData* Clone() const
		{
			CqShaderVariableVaryingFloat* newVar = new CqShaderVariableVaryingFloat( *this );
			newVar->SetSize( Size() );
			newVar->SetValueFromVariable( this );
			return ( newVar );
		}
};


class CqShaderVariableVaryingString : public CqShaderVariableVarying<type_string, CqString>
{
	public:
		CqShaderVariableVaryingString( ) : CqShaderVariableVarying<type_string, CqString>( )
		{
		}
		CqShaderVariableVaryingString( const char* strName, TqBool fParameter = TqFalse ) : CqShaderVariableVarying<type_string, CqString>( strName, fParameter )
		{
		}
		virtual	void	GetString( CqString& res, TqInt index = 0 ) const
		{
			res = m_aValue[ 0 ];
		}
		virtual	void	GetStringPtr( const CqString*& res ) const
		{
			res = &m_aValue[ 0 ];
		}
		virtual	void	GetStringPtr( CqString*& res )
		{
			res = &m_aValue[ 0 ];
		}
		virtual	void	SetString( const CqString& s, TqInt index )
		{
			m_aValue[ 0 ] = s;
		}
		virtual	void	SetString( const CqString& s )
		{
			m_aValue.assign( 0, s );
		}
		/** Copy the values from the passed variable into this, taking into account any class differences.
		 * \param pVal The variable to copy from.
		 */
		virtual	void	SetValueFromVariable( const IqShaderData* pVal )
		{
			CqString temp;
			pVal->GetString( temp );
			m_aValue.assign( 0, temp );
		}
		virtual void	GetBool( TqBool& res, TqInt index = 0 ) const
		{
			res = m_aValue[ 0 ].compare( "" ) == 0;
		}
		/** Set the all SIMD data ased on a state vector, only indexes whose bit is set are modified.
		 * \param pVal The stack entry to assign.
		 * \param index Integer SIMD index.
		 */
		virtual	void	SetValueFromVariable( const IqShaderData* pVal, TqInt index )
		{
			pVal->GetString( m_aValue[ 0 ], index );
		}
		virtual	IqShaderData* Clone() const
		{
			CqShaderVariableVaryingString* newVar = new CqShaderVariableVaryingString( *this );
			newVar->SetSize( Size() );
			newVar->SetValueFromVariable( this );
			return ( newVar );
		}
};

class CqShaderVariableVaryingPoint : public CqShaderVariableVarying<type_point, CqVector3D>
{
	public:
		CqShaderVariableVaryingPoint( ) : CqShaderVariableVarying<type_point, CqVector3D>( )
		{
		}
		CqShaderVariableVaryingPoint( const char* strName, TqBool fParameter = TqFalse ) : CqShaderVariableVarying<type_point, CqVector3D>( strName, fParameter )
		{
		}
		virtual	void	GetPoint( CqVector3D& res, TqInt index = 0 ) const
		{
			res = m_aValue[ index ];
		}
		virtual	void	GetPointPtr( const CqVector3D*& res ) const
		{
			res = &m_aValue[ 0 ];
		}
		virtual	void	GetPointPtr( CqVector3D*& res )
		{
			res = &m_aValue[ 0 ];
		}
		virtual	void	SetPoint( const CqVector3D& p, TqInt index )
		{
			m_aValue[ index ] = p;
		}
		virtual	void	SetPoint( const CqVector3D& p )
		{
			m_aValue.assign( m_aValue.size(), p );
		}
		virtual	void	GetVector( CqVector3D& res, TqInt index = 0 ) const
		{
			GetPoint( res, index );
		}
		virtual	void	GetVectorPtr( const CqVector3D*& res ) const
		{
			res = &m_aValue[ 0 ];
		}
		virtual	void	GetVectorPtr( CqVector3D*& res )
		{
			res = &m_aValue[ 0 ];
		}
		virtual	void	SetVector( const CqVector3D& p, TqInt index )
		{
			SetPoint( p, index );
		}
		virtual	void	SetVector( const CqVector3D& p )
		{
			SetPoint( p );
		}
		virtual	void	GetNormal( CqVector3D& res, TqInt index = 0 ) const
		{
			GetPoint( res, index );
		}
		virtual	void	GetNormalPtr( const CqVector3D*& res ) const
		{
			res = &m_aValue[ 0 ];
		}
		virtual	void	GetNormalPtr( CqVector3D*& res )
		{
			res = &m_aValue[ 0 ];
		}
		virtual	void	SetNormal( const CqVector3D& p, TqInt index )
		{
			SetPoint( p, index );
		}
		virtual	void	SetNormal( const CqVector3D& p )
		{
			SetPoint( p );
		}
		virtual void	GetBool( TqBool& res, TqInt index = 0 ) const
		{
			res = ( ( m_aValue[ index ].x() != 0.0f ) || ( m_aValue[ index ].y() != 0.0f ) || ( m_aValue[ index ].z() != 0.0f ) );
		}
		/** Copy the values from the passed variable into this, taking into account any class differences.
		 * \param pVal The variable to copy from.
		 */
		virtual	void	SetValueFromVariable( const IqShaderData* pVal )
		{
			TqUint i;
			if ( pVal->Size() > 1 )
			{
				const CqVector3D * pData;
				pVal->GetPointPtr( pData );
				for ( i = 0; i < Size(); i++ )
					m_aValue[ i ] = pData [ i ];
			}
			else
			{
				CqVector3D temp;
				pVal->GetPoint( temp );
				m_aValue.assign( m_aValue.size(), temp );
			}
		}
		/** Set the all SIMD data ased on a state vector, only indexes whose bit is set are modified.
		 * \param pVal The stack entry to assign.
		 * \param index Integer SIMD index.
		 */
		virtual	void	SetValueFromVariable( const IqShaderData* pVal, TqInt index )
		{
			pVal->GetPoint( m_aValue[ index ], index );
		}
		virtual	IqShaderData* Clone() const
		{
			CqShaderVariableVaryingPoint* newVar = new CqShaderVariableVaryingPoint( *this );
			newVar->SetSize( Size() );
			newVar->SetValueFromVariable( this );
			return ( newVar );
		}
};

class CqShaderVariableVaryingVector : public CqShaderVariableVarying<type_vector, CqVector3D>
{
	public:
		CqShaderVariableVaryingVector( ) : CqShaderVariableVarying<type_vector, CqVector3D>( )
		{
		}
		CqShaderVariableVaryingVector( const char* strName, TqBool fParameter = TqFalse ) : CqShaderVariableVarying<type_vector, CqVector3D>( strName, fParameter )
		{
		}
		virtual	void	GetPoint( CqVector3D& res, TqInt index = 0 ) const
		{
			res = m_aValue[ index ];
		}
		virtual	void	GetPointPtr( const CqVector3D*& res ) const
		{
			res = &m_aValue[ 0 ];
		}
		virtual	void	GetPointPtr( CqVector3D*& res )
		{
			res = &m_aValue[ 0 ];
		}
		virtual	void	SetPoint( const CqVector3D& p, TqInt index )
		{
			m_aValue[ index ] = p;
		}
		virtual	void	SetPoint( const CqVector3D& p )
		{
			m_aValue.assign( m_aValue.size(), p );
		}
		virtual	void	GetVector( CqVector3D& res, TqInt index = 0 ) const
		{
			GetPoint( res, index );
		}
		virtual	void	GetVectorPtr( const CqVector3D*& res ) const
		{
			res = &m_aValue[ 0 ];
		}
		virtual	void	GetVectorPtr( CqVector3D*& res )
		{
			res = &m_aValue[ 0 ];
		}
		virtual	void	SetVector( const CqVector3D& p, TqInt index )
		{
			SetPoint( p, index );
		}
		virtual	void	SetVector( const CqVector3D& p )
		{
			SetPoint( p );
		}
		virtual	void	GetNormal( CqVector3D& res, TqInt index = 0 ) const
		{
			GetPoint( res, index );
		}
		virtual	void	GetNormalPtr( const CqVector3D*& res ) const
		{
			res = &m_aValue[ 0 ];
		}
		virtual	void	GetNormalPtr( CqVector3D*& res )
		{
			res = &m_aValue[ 0 ];
		}
		virtual	void	SetNormal( const CqVector3D& p, TqInt index )
		{
			SetPoint( p, index );
		}
		virtual	void	SetNormal( const CqVector3D& p )
		{
			SetPoint( p );
		}
		virtual void	GetBool( TqBool& res, TqInt index = 0 ) const
		{
			res = ( ( m_aValue[ index ].x() != 0.0f ) || ( m_aValue[ index ].y() != 0.0f ) || ( m_aValue[ index ].z() != 0.0f ) );
		}
		/** Copy the values from the passed variable into this, taking into account any class differences.
		 * \param pVal The variable to copy from.
		 */
		virtual	void	SetValueFromVariable( const IqShaderData* pVal )
		{
			TqUint i;
			if ( pVal->Size() > 1 )
			{
				const CqVector3D * pData;
				pVal->GetVectorPtr( pData );
				for ( i = 0; i < Size(); i++ )
					m_aValue[ i ] = pData [ i ];
			}
			else
			{
				CqVector3D temp;
				pVal->GetVector( temp );
				m_aValue.assign( m_aValue.size(), temp );
			}
		}
		/** Set the all SIMD data ased on a state vector, only indexes whose bit is set are modified.
		 * \param pVal The stack entry to assign.
		 * \param index Integer SIMD index.
		 */
		virtual	void	SetValueFromVariable( const IqShaderData* pVal, TqInt index )
		{
			pVal->GetVector( m_aValue[ index ], index );
		}
		virtual	IqShaderData* Clone() const
		{
			CqShaderVariableVaryingVector* newVar = new CqShaderVariableVaryingVector( *this );
			newVar->SetSize( Size() );
			newVar->SetValueFromVariable( this );
			return ( newVar );
		}
};

class CqShaderVariableVaryingNormal : public CqShaderVariableVarying<type_normal, CqVector3D>
{
	public:
		CqShaderVariableVaryingNormal( ) : CqShaderVariableVarying<type_normal, CqVector3D>( )
		{
		}
		CqShaderVariableVaryingNormal( const char* strName, TqBool fParameter = TqFalse ) : CqShaderVariableVarying<type_normal, CqVector3D>( strName, fParameter )
		{
		}
		virtual	void	GetPoint( CqVector3D& res, TqInt index = 0 ) const
		{
			res = m_aValue[ index ];
		}
		virtual	void	GetPointPtr( const CqVector3D*& res ) const
		{
			res = &m_aValue[ 0 ];
		}
		virtual	void	GetPointPtr( CqVector3D*& res )
		{
			res = &m_aValue[ 0 ];
		}
		virtual	void	SetPoint( const CqVector3D& p, TqInt index )
		{
			m_aValue[ index ] = p;
		}
		virtual	void	SetPoint( const CqVector3D& p )
		{
			m_aValue.assign( m_aValue.size(), p );
		}
		virtual	void	GetVector( CqVector3D& res, TqInt index = 0 ) const
		{
			GetPoint( res, index );
		}
		virtual	void	GetVectorPtr( const CqVector3D*& res ) const
		{
			res = &m_aValue[ 0 ];
		}
		virtual	void	GetVectorPtr( CqVector3D*& res )
		{
			res = &m_aValue[ 0 ];
		}
		virtual	void	SetVector( const CqVector3D& p, TqInt index )
		{
			SetPoint( p, index );
		}
		virtual	void	SetVector( const CqVector3D& p )
		{
			SetPoint( p );
		}
		virtual	void	GetNormal( CqVector3D& res, TqInt index = 0 ) const
		{
			GetPoint( res, index );
		}
		virtual	void	GetNormalPtr( const CqVector3D*& res ) const
		{
			res = &m_aValue[ 0 ];
		}
		virtual	void	GetNormalPtr( CqVector3D*& res )
		{
			res = &m_aValue[ 0 ];
		}
		virtual	void	SetNormal( const CqVector3D& p, TqInt index )
		{
			SetPoint( p, index );
		}
		virtual	void	SetNormal( const CqVector3D& p )
		{
			SetPoint( p );
		}
		virtual void	GetBool( TqBool& res, TqInt index = 0 ) const
		{
			res = ( ( m_aValue[ index ].x() != 0.0f ) || ( m_aValue[ index ].y() != 0.0f ) || ( m_aValue[ index ].z() != 0.0f ) );
		}
		/** Copy the values from the passed variable into this, taking into account any class differences.
		 * \param pVal The variable to copy from.
		 */
		virtual	void	SetValueFromVariable( const IqShaderData* pVal )
		{
			TqUint i;
			if ( pVal->Size() > 1 )
			{
				const CqVector3D * pData;
				pVal->GetNormalPtr( pData );
				for ( i = 0; i < Size(); i++ )
					m_aValue[ i ] = pData [ i ];
			}
			else
			{
				CqVector3D temp;
				pVal->GetNormal( temp );
				m_aValue.assign( m_aValue.size(), temp );
			}
		}
		/** Set the all SIMD data ased on a state vector, only indexes whose bit is set are modified.
		 * \param pVal The stack entry to assign.
		 * \param index Integer SIMD index.
		 */
		virtual	void	SetValueFromVariable( const IqShaderData* pVal, TqInt index )
		{
			pVal->GetNormal( m_aValue[ index ], index );
		}
		virtual	IqShaderData* Clone() const
		{
			CqShaderVariableVaryingNormal* newVar = new CqShaderVariableVaryingNormal( *this );
			newVar->SetSize( Size() );
			newVar->SetValueFromVariable( this );
			return ( newVar );
		}
};

class CqShaderVariableVaryingColor : public CqShaderVariableVarying<type_color, CqColor>
{
	public:
		CqShaderVariableVaryingColor( ) : CqShaderVariableVarying<type_color, CqColor>( )
		{
		}
		CqShaderVariableVaryingColor( const char* strName, TqBool fParameter = TqFalse ) : CqShaderVariableVarying<type_color, CqColor>( strName, fParameter )
		{
		}
		virtual	void	GetColor( CqColor& res, TqInt index = 0 ) const
		{
			res = m_aValue[ index ];
		}
		virtual	void	GetColorPtr( const CqColor*& res ) const
		{
			res = &m_aValue[ 0 ];
		}
		virtual	void	GetColorPtr( CqColor*& res )
		{
			res = &m_aValue[ 0 ];
		}
		virtual	void	SetColor( const CqColor& c, TqInt index )
		{
			m_aValue[ index ] = c;
		}
		virtual	void	SetColor( const CqColor& c )
		{
			m_aValue.assign( m_aValue.size(), c );
		}
		/** Copy the values from the passed variable into this, taking into account any class differences.
		 * \param pVal The variable to copy from.
		 */
		virtual	void	SetValueFromVariable( const IqShaderData* pVal )
		{
			TqUint i;
			if ( pVal->Size() > 1 )
			{
				const CqColor * pData;
				pVal->GetColorPtr( pData );
				for ( i = 0; i < Size(); i++ )
					m_aValue[ i ] = pData [ i ];
			}
			else
			{
				CqColor temp;
				pVal->GetColor( temp );
				m_aValue.assign( m_aValue.size(), temp );
			}
		}
		virtual void	GetBool( TqBool& res, TqInt index = 0 ) const
		{
			res = ( ( m_aValue[ index ].fRed() != 0.0f ) || ( m_aValue[ index ].fGreen() != 0.0f ) || ( m_aValue[ index ].fBlue() != 0.0f ) );
		}
		/** Set the all SIMD data ased on a state vector, only indexes whose bit is set are modified.
		 * \param pVal The stack entry to assign.
		 * \param index Integer SIMD index.
		 */
		virtual	void	SetValueFromVariable( const IqShaderData* pVal, TqInt index )
		{
			pVal->GetColor( m_aValue[ index ], index );
		}
		virtual	IqShaderData* Clone() const
		{
			CqShaderVariableVaryingColor* newVar = new CqShaderVariableVaryingColor( *this );
			newVar->SetSize( Size() );
			newVar->SetValueFromVariable( this );
			return ( newVar );
		}
};

class CqShaderVariableVaryingMatrix : public CqShaderVariableVarying<type_matrix, CqMatrix>
{
	public:
		CqShaderVariableVaryingMatrix( ) : CqShaderVariableVarying<type_matrix, CqMatrix>( )
		{
		}
		CqShaderVariableVaryingMatrix( const char* strName, TqBool fParameter = TqFalse ) : CqShaderVariableVarying<type_matrix, CqMatrix>( strName, fParameter )
		{
		}
		virtual	void	GetMatrix( CqMatrix& res, TqInt index = 0 ) const
		{
			res = m_aValue[ index ];
		}
		virtual	void	GetMatrixPtr( const CqMatrix*& res ) const
		{
			res = &m_aValue[ 0 ];
		}
		virtual	void	GetMatrixPtr( CqMatrix*& res )
		{
			res = &m_aValue[ 0 ];
		}
		virtual	void	SetMatrix( const CqMatrix& m, TqInt index )
		{
			m_aValue[ index ] = m;
		}
		virtual	void	SetMatrix( const CqMatrix& m )
		{
			m_aValue.assign( m_aValue.size(), m );
		}
		/** Copy the values from the passed variable into this, taking into account any class differences.
		 * \param pVal The variable to copy from.
		 */
		virtual	void	SetValueFromVariable( const IqShaderData* pVal )
		{
			TqUint i;
			if ( pVal->Size() > 1 )
			{
				const CqMatrix * pData;
				pVal->GetMatrixPtr( pData );
				for ( i = 0; i < Size(); i++ )
					m_aValue[ i ] = pData [ i ];
			}
			else
			{
				CqMatrix temp;
				pVal->GetMatrix( temp );
				m_aValue.assign( m_aValue.size(), temp );
			}
		}
		/** Set the all SIMD data ased on a state vector, only indexes whose bit is set are modified.
		 * \param pVal The stack entry to assign.
		 * \param index Integer SIMD index.
		 */
		virtual	void	SetValueFromVariable( const IqShaderData* pVal, TqInt index )
		{
			pVal->GetMatrix( m_aValue[ index ], index );
		}
		virtual	IqShaderData* Clone() const
		{
			CqShaderVariableVaryingMatrix* newVar = new CqShaderVariableVaryingMatrix( *this );
			newVar->SetSize( Size() );
			newVar->SetValueFromVariable( this );
			return ( newVar );
		}
};

std::ostream &operator<<( std::ostream &Stream, EqVariableType t );
std::ostream &operator<<( std::ostream &Stream, EqVariableClass t );


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !SHADERVARIABLE_H_INCLUDED

