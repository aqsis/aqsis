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
#include	"stats.h"

#include	"ishadervariable.h"

START_NAMESPACE( Aqsis )


_qShareM	extern char*	gVariableStorageNames[];
_qShareM	extern TqInt	gcVariableStorageNames;
_qShareM	extern char*	gVariableTypeNames[];
_qShareM	extern TqInt	gcVariableTypeNames;

_qShare	std::ostream &operator<<( std::ostream &Stream, EqVariableType t );


class CqShaderExecEnv;
class CqVMStackEntry;


//----------------------------------------------------------------------
/** \class CqShaderVariable
 * Abstract base class from which all shaders variables must be defined.
 */

class CqShaderVariable : public IqShaderVariable
{
	public:
		CqShaderVariable();
		CqShaderVariable( const char* strName );
		CqShaderVariable( const CqShaderVariable& From );
		virtual	~CqShaderVariable();

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
		virtual IqShaderVariable*	ArrayEntry(TqInt i) const
		{
			return(NULL);
		}

	protected:
		CqString	m_strName;		///< Name of this variable.
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

		// Overridded from CqShaderVariable.


		virtual	void	Initialise( const TqInt uGridRes, const TqInt vGridRes, TqInt& index )
		{
			for ( std::vector<IqShaderVariable*>::iterator i = m_aVariables.begin(); i != m_aVariables.end(); i++ )
				( *i ) ->Initialise( uGridRes, vGridRes, index );
		}
		virtual	void	SetValue( CqVMStackEntry& Val )
		{
			m_aVariables[ 0 ] ->SetValue( Val );
		}
		virtual	void	SetValue( TqInt index, CqVMStackEntry& Val )
		{
			m_aVariables[ 0 ] ->SetValue( index, Val );
		}
		virtual	void	SetValue( CqVMStackEntry& Val, CqBitVector& State )
		{
			m_aVariables[ 0 ] ->SetValue( Val, State );
		}
		virtual	void	GetValue( TqInt index, CqVMStackEntry& Val ) const
		{
			m_aVariables[ 0 ] ->GetValue( index, Val );
		}
		virtual	EqVariableClass	Class() const
		{
			return ( static_cast<EqVariableClass>( m_aVariables[ 0 ] ->Class()) );
		}
		virtual	EqVariableType	Type() const
		{
			return ( static_cast<EqVariableType>( m_aVariables[ 0 ] ->Type()) );
		}
		virtual	IqShaderVariable* Clone() const;
		virtual TqUint	Size() const
		{
			return ( m_aVariables[ 0 ] ->Size() );
		}
		virtual	TqInt	ArrayLength() const
		{
			return ( m_aVariables.size() );
		}
		virtual IqShaderVariable*	ArrayEntry(TqInt i) const
		{
			return((*this)[i]);
		}

		/** Get a reference to the variable array.
		 */
		std::vector<IqShaderVariable*>& aVariables()
		{
			return ( m_aVariables );
		}
		/** Get a const reference to the variable array.
		 */
		const std::vector<IqShaderVariable*>& aVariables() const
		{
			return ( m_aVariables );
		}
		/** Array index access to the values in the array.
		 * \param index Integer index intot he array.
		 */
		IqShaderVariable* operator[] ( TqUint index ) const
		{
			return( m_aVariables[ index ] );
		}

	private:
		std::vector<IqShaderVariable*>	m_aVariables;		///< Array of pointers to variables.
}
;


//----------------------------------------------------------------------
/** \class CqShaderVariableTyped
 * Templatised base class for shader variables of a specific type.
 */

template <class R>
class CqShaderVariableTyped : public CqShaderVariable
{
	public:
		CqShaderVariableTyped()
		{}
		CqShaderVariableTyped( const char* strName ) : CqShaderVariable( strName )
		{}
		CqShaderVariableTyped( const CqShaderVariable& From ) : CqShaderVariable( From )
		{}
		virtual	~CqShaderVariableTyped()
		{}

		/** Pure virtual set value.
		 * \param Val The new value for the variable, promotes through fuplication if required.
		 */
//		virtual	void	SetValue( const R& Val ) = 0;
		/** Pure virtual set value.
		 * \param Val The new value for the variable, promotes through fuplication if required.
		 * \param State Bit vector contolling the setting of SIMD values, only indexes with the appropriate bit set in the state will be modified.
		 */
//		virtual	void	SetValue( const R& Val, CqBitVector& State ) = 0;
		/** Get the 'current', in terms of SIMD execution, value of this variable.
		 */
//		virtual	R	GetValue() const = 0;
		/** Get the 'current', in terms of SIMD execution, value of this variable.
		 */
//		virtual	operator R&() = 0;
		/** Set the 'current', in terms of SIMD execution, value of this variable.
		 * \param v The new value to use.
		 */
//		virtual	void	operator=( const R& v ) = 0;
		/** Indexed access to the SIMD data on this variable.
		 * \param i Integer SIMD index.
		 * \return Reference to the data at that index, or at aero if uniform.
		 */
//		virtual	const	R&	operator[] ( const TqUint i ) const=0;
		/** Indexed access to the SIMD data on this variable.
		 * \param i Integer SIMD index.
		 * \return Reference to the data at that index, or at aero if uniform.
		 */
//		virtual	R&	operator[] ( const TqUint i ) =0;

	private:
};


//----------------------------------------------------------------------
/** \class CqShaderVariableUniform
 * Uniform variable templatised by type.
 */

template <const EqVariableType T, class R>
class CqShaderVariableUniform : public CqShaderVariableTyped<R>
{
	public:
		CqShaderVariableUniform( const char* strName ) : CqShaderVariableTyped<R>( strName )
		{}
		CqShaderVariableUniform( const char* strName, const R& val ) : CqShaderVariableTyped<R>( strName ),
				m_Value( val )
		{}
		CqShaderVariableUniform( const CqShaderVariableUniform<T, R>& val ) :
				CqShaderVariableTyped<R>( val ),
				m_Value( val.m_Value )
		{}
		virtual	~CqShaderVariableUniform()
		{}

		virtual	void	Initialise( const TqInt uGridRes, const TqInt vGridRes, TqInt& index )
		{}
		virtual	void	SetValue( CqVMStackEntry& Val )
		{
			Val.Value( m_Value );
		}
		virtual	void	SetValue( TqInt index, CqVMStackEntry& Val )
		{
			Val.Value( m_Value, index );
		}
		virtual	void	SetValue( CqVMStackEntry& Val, CqBitVector& State )
		{
			SetValue( Val );
		}
		virtual	void	GetValue( TqInt index, CqVMStackEntry& Val ) const
		{
			Val.SetValue( index, m_Value );
		}
//		virtual	void	SetValue( const R& Val )
//		{
//			m_Value = Val;
//		}
//		virtual	void	SetValue( const R& Val, CqBitVector& State )
//		{
//			m_Value = Val;
//		}
//		virtual	R	GetValue() const
//		{
//			return ( m_Value );
//		}
		virtual	EqVariableClass	Class() const
		{
			return ( class_uniform );
		}
		virtual	EqVariableType	Type() const
		{
			return ( T );
		}
		virtual	IqShaderVariable* Clone() const
		{
			return ( new CqShaderVariableUniform<T, R>( *this ) );
		}
		virtual TqUint	Size() const
		{
			return ( 1 );
		}

		virtual	void	operator=( const CqShaderVariableUniform<T,R>& From )
		{
			m_Value = From.m_Value;
		}
//		operator R&()
//		{
//			return ( m_Value );
//		}
//		void	operator=( const R& v )
//		{
//			m_Value = v;
//		}
//		const	R&	operator[] ( const TqUint i ) const
//		{
//			return( m_Value );
//		}
//		R&	operator[] ( const TqUint i )
//		{
//			return( m_Value );
//		}

	private:
		R	m_Value;	///< Simgle uniform value of the appropriate type.
}
;



//----------------------------------------------------------------------
/** \class CqShaderVariableVarying
 * Varying variable templatised by type.
 */

template <const EqVariableType T, class R>
class CqShaderVariableVarying : public CqShaderVariableTyped<R>
{
	public:
		CqShaderVariableVarying( const char* strName ) : CqShaderVariableTyped<R>( strName ), m_Size( 1 )
		{
			m_aValue.resize( 1 );
		}
		CqShaderVariableVarying( const char* strName, const R& val ) : CqShaderVariableTyped<R>( strName ), m_Size( 1 )
		{
			m_aValue.resize( 1 );
			m_aValue[ 0 ] = val;
		}
		CqShaderVariableVarying( const CqShaderVariableVarying<T, R>& val ) : CqShaderVariableTyped<R>( val )
		{
			m_aValue.resize( val.m_aValue.size() );
			m_aValue.assign( val.m_aValue.begin(), val.m_aValue.begin() );
			m_pIndex = val.m_pIndex;
			m_Size = val.m_Size;
		}
		virtual	~CqShaderVariableVarying()
		{}

		virtual	void	Initialise( const TqInt uGridRes, const TqInt vGridRes, TqInt& index )
		{
			m_Size = ( uGridRes + 1 ) * ( vGridRes + 1 );
			if ( m_aValue.size() < m_Size )
				m_aValue.resize( m_Size );
			m_pIndex = &index;
		}

		virtual	void	SetValue( CqVMStackEntry& Val )
		{
			// Note we can do this because uniform stack entries just return
			// the single value irrespective of the array index.
			TqInt i;
			for ( i = Size() - 1; i >= 0; i-- )
			{
				Val.Value( m_temp_R, i );
				m_aValue[ i ] = m_temp_R;
			}
		}
		virtual	void	SetValue( TqInt index, CqVMStackEntry& Val )
		{
			// Note we can do this because uniform stack entries just return
			// the single value irrespective of the array index.
			Val.Value( m_temp_R, index );
			m_aValue[ index ] = m_temp_R;
		}
		virtual	void	SetValue( CqVMStackEntry& Val, CqBitVector& State )
		{
			// Note we can do this because uniform stack entries just return
			// the single value irrespective of the array index.
			TqInt i;
			for ( i = Size() - 1; i >= 0; i-- )
			{
				if ( State.Value( i ) )
				{
					Val.Value( m_temp_R, i );
					m_aValue[ i ] = m_temp_R;
				}
			}
		}
		virtual	void	GetValue( TqInt index, CqVMStackEntry& Val ) const
		{
			Val.SetValue( index, m_aValue[ index ] );
		}
//		virtual	void	SetValue( const R& Val )
//		{
//			TqInt i;
//			for ( i = Size() - 1; i >= 0; i-- )
//				m_aValue[ i ] = Val;
//		}
//		virtual	void	SetValue( const R& Val, CqBitVector& State )
//		{
//			TqInt i;
//			for ( i = Size() - 1; i >= 0; i-- )
//			{
//				if ( State.Value( i ) )
//					m_aValue[ i ] = Val;
//			}
//		}
//		virtual	void	SetValue( const CqShaderVariableTyped<R>& From )
//		{
//			TqInt i;
//			for ( i = Size() - 1; i >= 0; i-- )
//				m_aValue[ i ] = From[ i ];
//		}
		virtual	R	GetValue() const
		{
			return ( m_aValue[ *m_pIndex ] );
		}
		virtual	EqVariableClass	Class() const
		{
			return ( class_varying );
		}
		virtual	EqVariableType	Type() const
		{
			return ( T );
		}
		virtual	IqShaderVariable* Clone() const
		{
			return ( new CqShaderVariableVarying<T, R>( *this ) );
		}
		virtual TqUint	Size() const
		{
			return ( m_Size );
		}

		virtual	void	operator=( const CqShaderVariableVarying<T,R>& From )
		{
			TqInt i;
			for ( i = m_aValue.size() - 1; i >= 0; i-- )
				m_aValue[ i ] = From.m_aValue[ i ];
		}
//		operator R&()
//		{
//			return ( m_aValue[ *m_pIndex ] );
//		}
//		void	operator=( const R& v )
//		{
//			m_aValue[ *m_pIndex ] = v;
//		}
//		const	R&	operator[] ( const TqUint i ) const
//		{
//			assert( i<m_Size );
//			return ( m_aValue[ i ] );
//		}
//		R&	operator[] ( const TqUint i )
//		{
//			assert( i<m_Size );
//			return ( m_aValue[ i ] );
//		}

	private:
		std::vector<R>	m_aValue;		///< Array of values of the appropriate type.
		TqInt*	m_pIndex;		///< Pointer to the SIMD index.
		TqUint	m_Size;			///< Integer size of the SIMD data.
		R	m_temp_R;		///< Temp value to use in template functions, problem with VC++.
}
;


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !SHADERVARIABLE_H_INCLUDED
