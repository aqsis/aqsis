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
		CqShaderVariable( const char* strName );
		virtual	~CqShaderVariable();

		virtual void GetBool( TqBool& res, TqInt index = 0 ) const	{ assert(TqFalse); }
		virtual void SetBool( const TqBool& val )					{ assert(TqFalse); }
		virtual void SetBool( const TqBool& val, TqInt index )		{ assert(TqFalse); }

		/** Get the name of this variable.
		 * \return Read only reference to a CqString class.
		 */
		const CqString&	strName()
		{
			return ( m_strName );
		}

		virtual TqInt	ArrayLength() const
		{
			return ( 0 );
		}
		virtual IqShaderData*	ArrayEntry(TqInt i)
		{
			return( this );
		}

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
		CqShaderVariableArray( const char* name, TqInt Count ) : CqShaderVariable( name )
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
		{}

		// Overridden from IqShaderData.

		virtual	void	Initialise( const TqInt uGridRes, const TqInt vGridRes )
		{
			for ( std::vector<IqShaderData*>::iterator i = m_aVariables.begin(); i != m_aVariables.end(); i++ )
				( *i ) ->Initialise( uGridRes, vGridRes );
		}



		virtual	void		GetFloat(TqFloat& res, TqInt index=0) const		{assert(false); m_aVariables[0]->GetFloat(res, index); }
		virtual	void		GetString(CqString& res, TqInt index=0) const	{assert(false); m_aVariables[0]->GetString(res, index); }
		virtual	void		GetPoint(CqVector3D& res, TqInt index=0) const	{assert(false); m_aVariables[0]->GetPoint(res, index); }
		virtual	void		GetVector(CqVector3D& res, TqInt index=0) const	{assert(false); m_aVariables[0]->GetVector(res, index); }
		virtual	void		GetNormal(CqVector3D& res, TqInt index=0) const	{assert(false); m_aVariables[0]->GetNormal(res, index); }
		virtual	void		GetColor(CqColor& res, TqInt index=0) const		{assert(false); m_aVariables[0]->GetColor(res, index); }
		virtual	void		GetMatrix(CqMatrix& res, TqInt index=0) const	{assert(false); m_aVariables[0]->GetMatrix(res, index); }

		virtual	void		SetFloat(const TqFloat& f)						{assert(false); m_aVariables[0]->SetFloat(f); }
		virtual	void		SetString(const CqString& s)					{assert(false); m_aVariables[0]->SetString(s); }
		virtual	void		SetPoint(const CqVector3D& p)					{assert(false); m_aVariables[0]->SetPoint(p); }
		virtual	void		SetVector(const CqVector3D& v)					{assert(false); m_aVariables[0]->SetVector(v); }
		virtual	void		SetNormal(const CqVector3D& n)					{assert(false); m_aVariables[0]->SetNormal(n); }
		virtual	void		SetColor(const CqColor& c)						{assert(false); m_aVariables[0]->SetColor(c); }
		virtual	void		SetMatrix(const CqMatrix& m)					{assert(false); m_aVariables[0]->SetMatrix(m); }

		virtual	void		SetFloat(const TqFloat& f, TqInt index)			{assert(false); m_aVariables[0]->SetFloat(f, index); }
		virtual	void		SetString(const CqString& s, TqInt index)		{assert(false); m_aVariables[0]->SetString(s, index); }
		virtual	void		SetPoint(const CqVector3D& p, TqInt index)		{assert(false); m_aVariables[0]->SetPoint(p, index); }
		virtual	void		SetVector(const CqVector3D& v, TqInt index)		{assert(false); m_aVariables[0]->SetVector(v, index); }
		virtual	void		SetNormal(const CqVector3D& n, TqInt index)		{assert(false); m_aVariables[0]->SetNormal(n, index); }
		virtual	void		SetColor(const CqColor& c, TqInt index)			{assert(false); m_aVariables[0]->SetColor(c, index); }
		virtual	void		SetMatrix(const CqMatrix& m, TqInt index)		{assert(false); m_aVariables[0]->SetMatrix(m, index); }

		virtual	void	SetValueFromVariable( IqShaderData* pFrom )
		{
			m_aVariables[ 0 ] ->SetValueFromVariable( pFrom );
		}
		virtual	void	SetValueFromVariable( IqShaderData* pFrom, TqInt index )
		{
			m_aVariables[ 0 ] ->SetValueFromVariable( pFrom, index );
		}

		virtual	IqShaderData* Clone() const						{ return( new CqShaderVariableArray( *this ) ); }
	

		virtual	EqVariableClass	Class() const
		{
			return ( static_cast<EqVariableClass>( m_aVariables[ 0 ] ->Class()) );
		}
		virtual	EqVariableType	Type() const
		{
			return ( static_cast<EqVariableType>( m_aVariables[ 0 ] ->Type()) );
		}
		virtual TqUint	Size() const
		{
			return ( m_aVariables[ 0 ] ->Size() );
		}
		virtual	TqInt	ArrayLength() const
		{
			return ( m_aVariables.size() );
		}
		virtual IqShaderData*	ArrayEntry(TqInt i)
		{
			return((*this)[i]);
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
		CqShaderVariableUniform( const char* strName ) : CqShaderVariable( strName )
		{}
		CqShaderVariableUniform( const char* strName, const R& val ) : CqShaderVariable( strName ),
				m_Value( val )
		{}
		CqShaderVariableUniform( const CqShaderVariableUniform<T, R>& val ) :
				CqShaderVariable( val ),
				m_Value( val.m_Value )
		{}
		virtual	~CqShaderVariableUniform()
		{}

		virtual	void	Initialise( const TqInt uGridRes, const TqInt vGridRes )
		{}


		virtual	void		GetFloat(TqFloat& res, TqInt index=0) const		{ assert(false); }
		virtual	void		GetString(CqString& res, TqInt index=0) const	{ assert(false); }
		virtual	void		GetPoint(CqVector3D& res, TqInt index=0) const	{ assert(false); }
		virtual	void		GetVector(CqVector3D& res, TqInt index=0) const	{ assert(false); }
		virtual	void		GetNormal(CqVector3D& res, TqInt index=0) const	{ assert(false); }
		virtual	void		GetColor(CqColor& res, TqInt index=0) const		{ assert(false); }
		virtual	void		GetMatrix(CqMatrix& res, TqInt index=0) const	{ assert(false); }

		virtual	void		SetFloat(const TqFloat& f)						{ assert(false); }
		virtual	void		SetString(const CqString& s)					{ assert(false); }
		virtual	void		SetPoint(const CqVector3D& p)					{ assert(false); }
		virtual	void		SetVector(const CqVector3D& v)					{ assert(false); }
		virtual	void		SetNormal(const CqVector3D& n)					{ assert(false); }
		virtual	void		SetColor(const CqColor& c)						{ assert(false); }
		virtual	void		SetMatrix(const CqMatrix& m)					{ assert(false); }

		virtual	void		SetFloat(const TqFloat& f, TqInt index)			{ SetFloat( f ); }
		virtual	void		SetString(const CqString& s, TqInt index)		{ SetString( s ); }
		virtual	void		SetPoint(const CqVector3D& p, TqInt index)		{ SetPoint( p ); }
		virtual	void		SetVector(const CqVector3D& v, TqInt index)		{ SetVector( v ); }
		virtual	void		SetNormal(const CqVector3D& n, TqInt index)		{ SetNormal( n ); }
		virtual	void		SetColor(const CqColor& c, TqInt index)			{ SetColor( c ); }
		virtual	void		SetMatrix(const CqMatrix& m, TqInt index)		{ SetMatrix( m ); }

		virtual	void		SetValueFromVariable( IqShaderData* pVal, TqInt index ) { SetValueFromVariable( pVal ); }
		virtual	void		SetValueFromVariable( IqShaderData* pVal )				{ assert(false); }

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

		virtual	void	operator=( const CqShaderVariableUniform<T,R>& From )
		{
			m_Value = From.m_Value;
		}

	protected:
		R	m_Value;	///< Simgle uniform value of the appropriate type.
};


class CqShaderVariableUniformFloat : public CqShaderVariableUniform<type_float, TqFloat>
{
	public:
		CqShaderVariableUniformFloat( const char* strName )	: CqShaderVariableUniform<type_float, TqFloat>( strName )	
																	{}
		virtual	void		GetFloat(TqFloat& res, TqInt index=0) const	{ res = m_Value; }
		virtual	void		SetFloat(const TqFloat& f)					{ m_Value = f; }
		virtual	void		SetValueFromVariable( IqShaderData* pFrom )			{ pFrom->GetFloat( m_Value ); }
		virtual	IqShaderData* Clone() const							{ return( new CqShaderVariableUniformFloat( *this ) ); }
};

class CqShaderVariableUniformString : public CqShaderVariableUniform<type_string, CqString>
{
	public:
		CqShaderVariableUniformString( const char* strName )	: CqShaderVariableUniform<type_string, CqString>( strName )	
																	{}
		virtual	void		GetString(CqString& res, TqInt index=0) const	{ res = m_Value; }
		virtual	void		SetString(const CqString& s)					{ m_Value = s; }
		virtual	void		SetValueFromVariable( IqShaderData* pFrom )				{ pFrom->GetString( m_Value ); }
		virtual	IqShaderData* Clone() const								{ return( new CqShaderVariableUniformString( *this ) ); }
};

class CqShaderVariableUniformPoint : public CqShaderVariableUniform<type_point, CqVector3D>
{
	public:
		CqShaderVariableUniformPoint( const char* strName )	: CqShaderVariableUniform<type_point, CqVector3D>( strName )	
																	{}
		virtual	void		GetPoint(CqVector3D& res, TqInt index=0) const	{ res = m_Value; }
		virtual	void		SetPoint(const CqVector3D& p)					{ m_Value = p; }
		virtual	void		GetVector(CqVector3D& res, TqInt index=0) const	{ res = m_Value; }
		virtual	void		SetVector(const CqVector3D& p)					{ m_Value = p; }
		virtual	void		GetNormal(CqVector3D& res, TqInt index=0) const	{ res = m_Value; }
		virtual	void		SetNormal(const CqVector3D& p)					{ m_Value = p; }
		virtual	void		SetValueFromVariable( IqShaderData* pFrom )				{ pFrom->GetPoint( m_Value ); }
		virtual	IqShaderData* Clone() const								{ return( new CqShaderVariableUniformPoint( *this ) ); }
};

class CqShaderVariableUniformVector : public CqShaderVariableUniform<type_vector, CqVector3D>
{
	public:
		CqShaderVariableUniformVector( const char* strName )	: CqShaderVariableUniform<type_vector, CqVector3D>( strName )	
																	{}
		virtual	void		GetPoint(CqVector3D& res, TqInt index=0) const	{ res = m_Value; }
		virtual	void		SetPoint(const CqVector3D& p)					{ m_Value = p; }
		virtual	void		GetVector(CqVector3D& res, TqInt index=0) const	{ res = m_Value; }
		virtual	void		SetVector(const CqVector3D& p)					{ m_Value = p; }
		virtual	void		GetNormal(CqVector3D& res, TqInt index=0) const	{ res = m_Value; }
		virtual	void		SetNormal(const CqVector3D& p)					{ m_Value = p; }
		virtual	void		SetValueFromVariable( IqShaderData* pFrom )				{ pFrom->GetVector( m_Value ); }
		virtual	IqShaderData* Clone() const								{ return( new CqShaderVariableUniformVector( *this ) ); }
};

class CqShaderVariableUniformNormal : public CqShaderVariableUniform<type_normal, CqVector3D>
{
	public:
		CqShaderVariableUniformNormal( const char* strName )	: CqShaderVariableUniform<type_normal, CqVector3D>( strName )	
																	{}
		virtual	void		GetPoint(CqVector3D& res, TqInt index=0) const	{ res = m_Value; }
		virtual	void		SetPoint(const CqVector3D& p)					{ m_Value = p; }
		virtual	void		GetVector(CqVector3D& res, TqInt index=0) const	{ res = m_Value; }
		virtual	void		SetVector(const CqVector3D& p)					{ m_Value = p; }
		virtual	void		GetNormal(CqVector3D& res, TqInt index=0) const	{ res = m_Value; }
		virtual	void		SetNormal(const CqVector3D& p)					{ m_Value = p; }
		virtual	void		SetValueFromVariable( IqShaderData* pFrom )				{ pFrom->GetNormal( m_Value ); }
		virtual	IqShaderData* Clone() const								{ return( new CqShaderVariableUniformNormal( *this ) ); }
};

class CqShaderVariableUniformColor : public CqShaderVariableUniform<type_color, CqColor>
{
	public:
		CqShaderVariableUniformColor( const char* strName )	: CqShaderVariableUniform<type_color, CqColor>( strName )	
																	{}
		virtual	void		GetColor(CqColor& res, TqInt index=0) const		{ res = m_Value; }
		virtual	void		SetColor(const CqColor& c)						{ m_Value = c; }
		virtual	void		SetValueFromVariable( IqShaderData* pFrom )				{ pFrom->GetColor( m_Value ); }
		virtual	IqShaderData* Clone() const								{ return( new CqShaderVariableUniformColor( *this ) ); }
};

class CqShaderVariableUniformMatrix : public CqShaderVariableUniform<type_matrix, CqMatrix>
{
	public:
		CqShaderVariableUniformMatrix( const char* strName )	: CqShaderVariableUniform<type_matrix, CqMatrix>( strName )	
																	{}
		virtual	void		GetMatrix(CqMatrix& res, TqInt index=0) const	{ res = m_Value; }
		virtual	void		SetMatrix(const CqMatrix& m)					{ m_Value = m; }
		virtual	void		SetValueFromVariable( IqShaderData* pFrom )				{ pFrom->GetMatrix( m_Value ); }
		virtual	IqShaderData* Clone() const								{ return( new CqShaderVariableUniformMatrix( *this ) ); }
};


//----------------------------------------------------------------------
/** \class CqShaderVariableVarying
 * Varying variable templatised by type.
 */

template <const EqVariableType T, class R>
class CqShaderVariableVarying : public CqShaderVariable
{
	public:
		CqShaderVariableVarying( const char* strName ) : CqShaderVariable( strName )
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
			m_aValue.assign( val.m_aValue.begin(), val.m_aValue.begin() );
		}
		virtual	~CqShaderVariableVarying()
		{}

		virtual	void	Initialise( const TqInt uGridRes, const TqInt vGridRes )
		{
			m_aValue.resize( ( uGridRes + 1 ) * ( vGridRes + 1 ) );
		}

		virtual	void		GetFloat(TqFloat &res, TqInt index=0) const		{assert(false); }
		virtual	void		GetString(CqString& res, TqInt index=0) const	{assert(false); }
		virtual	void		GetPoint(CqVector3D& res, TqInt index=0) const	{assert(false); }
		virtual	void		GetVector(CqVector3D& res, TqInt index=0) const	{assert(false); }
		virtual	void		GetNormal(CqVector3D& res, TqInt index=0) const	{assert(false); }
		virtual	void		GetColor(CqColor& res, TqInt index=0) const		{assert(false); }
		virtual	void		GetMatrix(CqMatrix& res, TqInt index=0) const	{assert(false); }

		virtual	void		SetFloat(const TqFloat& f)						{assert(false); }
		virtual	void		SetString(const CqString& s)					{assert(false); }
		virtual	void		SetPoint(const CqVector3D& p)					{assert(false); }
		virtual	void		SetVector(const CqVector3D& v)					{assert(false); }
		virtual	void		SetNormal(const CqVector3D& n)					{assert(false); }
		virtual	void		SetColor(const CqColor& c)						{assert(false); }
		virtual	void		SetMatrix(const CqMatrix& m)					{assert(false); }

		virtual	void		SetFloat(const TqFloat& f, TqInt index)			{assert(false); }
		virtual	void		SetString(const CqString& s, TqInt index)		{assert(false); }
		virtual	void		SetPoint(const CqVector3D& p, TqInt index)		{assert(false); }
		virtual	void		SetVector(const CqVector3D& v, TqInt index)		{assert(false); }
		virtual	void		SetNormal(const CqVector3D& n, TqInt index)		{assert(false); }
		virtual	void		SetColor(const CqColor& c, TqInt index)			{assert(false); }
		virtual	void		SetMatrix(const CqMatrix& m, TqInt index)		{assert(false); }

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

		virtual	void	operator=( const CqShaderVariableVarying<T,R>& From )
		{
			TqInt i;
			for ( i = m_aValue.size() - 1; i >= 0; i-- )
				m_aValue[ i ] = From.m_aValue[ i ];
		}

	protected:
		std::vector<R>	m_aValue;		///< Array of values of the appropriate type.
		R	m_temp_R;		///< Temp value to use in template functions, problem with VC++.
};


class CqShaderVariableVaryingFloat : public CqShaderVariableVarying<type_float, TqFloat>
{
	public:
		CqShaderVariableVaryingFloat( const char* strName )	: CqShaderVariableVarying<type_float, TqFloat>( strName )	
																	{}
		virtual	void		GetFloat(TqFloat& res, TqInt index=0) const	{ res = m_aValue[index]; }
		virtual	void		SetFloat(const TqFloat& f, TqInt index)		{ m_aValue[index] = f; }
		virtual	void		SetFloat(const TqFloat& f)		
							{
								TqInt i;
								for( i = 0; i < Size(); i++ )
									m_aValue[i] = f;
							}
		virtual	void		SetValueFromVariable( IqShaderData* pFrom )		
							{ 
								TqInt i;
								for(i = 0; i < Size(); i++)
									pFrom->GetFloat( m_aValue[i], i );
							}
		virtual	void		SetValueFromVariable( IqShaderData* pFrom, TqInt index )		
							{ 
								pFrom->GetFloat( m_aValue[ index ], index );
							}
		virtual	IqShaderData* Clone() const						{ return( new CqShaderVariableVaryingFloat( *this ) ); }
};

class CqShaderVariableVaryingString : public CqShaderVariableVarying<type_string, CqString>
{
	public:
		CqShaderVariableVaryingString( const char* strName )	: CqShaderVariableVarying<type_string, CqString>( strName )	
																	{}
		virtual	void		GetString(CqString& res, TqInt index=0) const	{ res = m_aValue[index]; }
		virtual	void		SetString(const CqString& s, TqInt index)		{ m_aValue[index] = s; }
		virtual	void		SetString(const CqString& s)		
							{
								TqInt i;
								for( i = 0; i < Size(); i++ )
									m_aValue[i] = s;
							}
		virtual	void		SetValueFromVariable( IqShaderData* pFrom )		
							{ 
								TqInt i;
								for(i = 0; i < Size(); i++)
									pFrom->GetString( m_aValue[ i ], i );
							}
		virtual	void		SetValueFromVariable( IqShaderData* pFrom, TqInt index )		
							{ 
								pFrom->GetString( m_aValue[ index ], index );
							}
		virtual	IqShaderData* Clone() const						{ return( new CqShaderVariableVaryingString( *this ) ); }
};

class CqShaderVariableVaryingPoint : public CqShaderVariableVarying<type_point, CqVector3D>
{
	public:
		CqShaderVariableVaryingPoint( const char* strName )	: CqShaderVariableVarying<type_point, CqVector3D>( strName )	
																	{}
		virtual	void		GetPoint(CqVector3D& res, TqInt index=0) const	{ res = m_aValue[index]; }
		virtual	void		SetPoint(const CqVector3D& p, TqInt index)		{ m_aValue[index] = p; }
		virtual	void		SetPoint(const CqVector3D& p)		
							{
								TqInt i;
								for( i = 0; i < Size(); i++ )
									m_aValue[i] = p;
							}
		virtual	void		GetVector(CqVector3D& res, TqInt index=0) const	{ GetPoint(res, index); }
		virtual	void		SetVector(const CqVector3D& p, TqInt index)		{ SetPoint(p, index); }
		virtual	void		SetVector(const CqVector3D& p)					{ SetPoint( p ); }
		virtual	void		GetNormal(CqVector3D& res, TqInt index=0) const	{ GetPoint(res, index); }
		virtual	void		SetNormal(const CqVector3D& p, TqInt index)		{ SetPoint(p, index); }
		virtual	void		SetNormal(const CqVector3D& p)					{ SetPoint( p ); }
		virtual	void		SetValueFromVariable( IqShaderData* pFrom )		
							{ 
								TqInt i;
								for(i = 0; i < Size(); i++)
									pFrom->GetPoint( m_aValue[ i ], i );
							}
		virtual	void		SetValueFromVariable( IqShaderData* pFrom, TqInt index )		
							{ 
								pFrom->GetPoint( m_aValue[ index ], index );
							}
		virtual	IqShaderData* Clone() const						{ return( new CqShaderVariableVaryingPoint( *this ) ); }
};

class CqShaderVariableVaryingVector : public CqShaderVariableVarying<type_vector, CqVector3D>
{
	public:
		CqShaderVariableVaryingVector( const char* strName )	: CqShaderVariableVarying<type_vector, CqVector3D>( strName )	
																	{}
		virtual	void		GetPoint(CqVector3D& res, TqInt index=0) const	{ res = m_aValue[index]; }
		virtual	void		SetPoint(const CqVector3D& p, TqInt index)		{ m_aValue[index] = p; }
		virtual	void		SetPoint(const CqVector3D& p)		
							{
								TqInt i;
								for( i = 0; i < Size(); i++ )
									m_aValue[i] = p;
							}
		virtual	void		GetVector(CqVector3D& res, TqInt index=0) const	{ GetPoint(res, index); }
		virtual	void		SetVector(const CqVector3D& p, TqInt index)		{ SetPoint(p, index); }
		virtual	void		SetVector(const CqVector3D& p)					{ SetPoint( p ); }
		virtual	void		GetNormal(CqVector3D& res, TqInt index=0) const	{ GetPoint(res, index); }
		virtual	void		SetNormal(const CqVector3D& p, TqInt index)		{ SetPoint(p, index); }
		virtual	void		SetNormal(const CqVector3D& p)					{ SetPoint( p ); }
		virtual	void		SetValueFromVariable( IqShaderData* pFrom )		
							{ 
								TqInt i;
								for(i = 0; i < Size(); i++)
									pFrom->GetVector( m_aValue[ i ], i );
							}
		virtual	void		SetValueFromVariable( IqShaderData* pFrom, TqInt index )		
							{ 
								pFrom->GetVector( m_aValue[ index ], index );
							}
		virtual	IqShaderData* Clone() const						{ return( new CqShaderVariableVaryingVector( *this ) ); }
};

class CqShaderVariableVaryingNormal : public CqShaderVariableVarying<type_normal, CqVector3D>
{
	public:
		CqShaderVariableVaryingNormal( const char* strName )	: CqShaderVariableVarying<type_normal, CqVector3D>( strName )	
																	{}
		virtual	void		GetPoint(CqVector3D& res, TqInt index=0) const	{ res = m_aValue[index]; }
		virtual	void		SetPoint(const CqVector3D& p, TqInt index)		{ m_aValue[index] = p; }
		virtual	void		SetPoint(const CqVector3D& p)		
							{
								TqInt i;
								for( i = 0; i < Size(); i++ )
									m_aValue[i] = p;
							}
		virtual	void		GetVector(CqVector3D& res, TqInt index=0) const	{ GetPoint(res, index); }
		virtual	void		SetVector(const CqVector3D& p, TqInt index)		{ SetPoint(p, index); }
		virtual	void		SetVector(const CqVector3D& p)					{ SetPoint( p ); }
		virtual	void		GetNormal(CqVector3D& res, TqInt index=0) const	{ GetPoint(res, index); }
		virtual	void		SetNormal(const CqVector3D& p, TqInt index)		{ SetPoint(p, index); }
		virtual	void		SetNormal(const CqVector3D& p)					{ SetPoint( p ); }
		virtual	void		SetValueFromVariable( IqShaderData* pFrom )		
							{ 
								TqInt i;
								for(i = 0; i < Size(); i++)
									pFrom->GetNormal( m_aValue[ i ], i );
							}
		virtual	void		SetValueFromVariable( IqShaderData* pFrom, TqInt index )		
							{ 
								pFrom->GetNormal( m_aValue[ index ], index );
							}
		virtual	IqShaderData* Clone() const						{ return( new CqShaderVariableVaryingNormal( *this ) ); }
};

class CqShaderVariableVaryingColor : public CqShaderVariableVarying<type_color, CqColor>
{
	public:
		CqShaderVariableVaryingColor( const char* strName )	: CqShaderVariableVarying<type_color, CqColor>( strName )	
																	{}
		virtual	void		GetColor(CqColor& res, TqInt index=0) const	{ res = m_aValue[index]; }
		virtual	void		SetColor(const CqColor& c, TqInt index)		{ m_aValue[index] = c; }
		virtual	void		SetColor(const CqColor& c)		
							{
								TqInt i;
								for( i = 0; i < Size(); i++ )
									m_aValue[i] = c;
							}
		virtual	void		SetValueFromVariable( IqShaderData* pFrom )		
							{ 
								TqInt i;
								for(i = 0; i < Size(); i++)
									pFrom->GetColor( m_aValue[ i ], i );
							}
		virtual	void		SetValueFromVariable( IqShaderData* pFrom, TqInt index )		
							{ 
								pFrom->GetColor( m_aValue[ index ], index );
							}
		virtual	IqShaderData* Clone() const						{ return( new CqShaderVariableVaryingColor( *this ) ); }
};

class CqShaderVariableVaryingMatrix : public CqShaderVariableVarying<type_matrix, CqMatrix>
{
	public:
		CqShaderVariableVaryingMatrix( const char* strName )	: CqShaderVariableVarying<type_matrix, CqMatrix>( strName )	
																	{}
		virtual	void		GetMatrix(CqMatrix& res, TqInt index=0) const	{ res = m_aValue[index]; }
		virtual	void		SetMatrix(const CqMatrix& m, TqInt index)		{ m_aValue[index] = m; }
		virtual	void		SetMatrix(const CqMatrix& m)		
							{
								TqInt i;
								for( i = 0; i < Size(); i++ )
									m_aValue[i] = m;
							}
		virtual	void		SetValueFromVariable( IqShaderData* pFrom )		
							{ 
								TqInt i;
								for(i = 0; i < Size(); i++)
									pFrom->GetMatrix( m_aValue[ i ], i );
							}
		virtual	void		SetValueFromVariable( IqShaderData* pFrom, TqInt index )		
							{ 
								pFrom->GetMatrix( m_aValue[ index ], index );
							}
		virtual	IqShaderData* Clone() const						{ return( new CqShaderVariableVaryingMatrix( *this ) ); }
};


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !SHADERVARIABLE_H_INCLUDED
