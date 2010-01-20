//------------------------------------------------------------------------------
/**
 *	@file	ishaderdata.h
 *	@author	Paul Gregory
 *	@brief	Decares the interface to generic shader variables.
 *
 *	Last change by:		$Author$
 *	Last change date:	$Date$
 */
//------------------------------------------------------------------------------


#ifndef	ISHADERDATA_H_INCLUDED
#define	ISHADERDATA_H_INCLUDED

#include <aqsis/aqsis.h>

#include <aqsis/math/matrix.h>
#include <aqsis/riutil/primvartype.h>
#include <aqsis/util/sstring.h>
#include <aqsis/math/vecfwd.h>

namespace Aqsis {

//----------------------------------------------------------------------
/** \struct IqShaderData
 * Interface to the shader engine specific implementation of a shader variable.
 */

struct IqShaderData
{
	/** \brief Storage type within the shader context
 	 */
	enum EqStorage
	{
		Unknown,         ///< Don't know the storage
		Temporary,       ///< An internal temporary variable
		Parameter,       ///< A parameter to the shader
		OutputParameter  ///< An output parameter for the shader
	};

	/// Virtual destructor so that derived classes get cleaned up properly.
	virtual	~IqShaderData()
	{}

	///	Get the data as a float..

	virtual void GetFloat( TqFloat& res, TqInt index = 0 ) const = 0;
	///	Get the data as a boolean value..
	virtual void GetBool( bool& res, TqInt index = 0 ) const = 0;
	///	Get the data as a string..
	virtual void GetString( CqString& res, TqInt index = 0 ) const = 0;
	///	Get the data as a point..
	virtual void GetPoint( CqVector3D& res, TqInt index = 0 ) const = 0;
	///	Get the data as a vector..
	virtual void GetVector( CqVector3D& res, TqInt index = 0 ) const = 0;
	///	Get the data as a normal..
	virtual void GetNormal( CqVector3D& res, TqInt index = 0 ) const = 0;
	///	Get the data as a color..
	virtual void GetColor( CqColor& res, TqInt index = 0 ) const = 0;
	///	Get the data as a matrix..
	virtual void GetMatrix( CqMatrix& res, TqInt index = 0 ) const = 0;

	///	Get a const pointer to the data as a float..
	virtual void GetFloatPtr( const TqFloat*& res ) const = 0;
	///	Get a const pointer to the data as a boolean value..
	virtual void GetBoolPtr( const bool*& res ) const = 0;
	///	Get a const pointer to the data as a string..
	virtual void GetStringPtr( const CqString*& res ) const = 0;
	///	Get a const pointer to the data as a point..
	virtual void GetPointPtr( const CqVector3D*& res ) const = 0;
	///	Get a const pointer to the data as a vector..
	virtual void GetVectorPtr( const CqVector3D*& res ) const = 0;
	///	Get a const pointer to the data as a normal..
	virtual void GetNormalPtr( const CqVector3D*& res ) const = 0;
	///	Get a const pointer to the data as a color..
	virtual void GetColorPtr( const CqColor*& res ) const = 0;
	///	Get a const pointer to the data as a matrix..
	virtual void GetMatrixPtr( const CqMatrix*& res ) const = 0;

	///	Get a pointer to the data as a float..
	virtual void GetFloatPtr( TqFloat*& res ) = 0;
	///	Get a pointer to the data as a boolean value..
	virtual void GetBoolPtr( bool*& res ) = 0;
	///	Get a pointer to the data as a string..
	virtual void GetStringPtr( CqString*& res ) = 0;
	///	Get a pointer to the data as a point..
	virtual void GetPointPtr( CqVector3D*& res ) = 0;
	///	Get a pointer to the data as a vector..
	virtual void GetVectorPtr( CqVector3D*& res ) = 0;
	///	Get a pointer to the data as a normal..
	virtual void GetNormalPtr( CqVector3D*& res ) = 0;
	///	Get a pointer to the data as a color..
	virtual void GetColorPtr( CqColor*& res ) = 0;
	///	Get a pointer to the data as a matrix..
	virtual void GetMatrixPtr( CqMatrix*& res ) = 0;

	///	Set the value to the specified float.
	virtual void SetFloat( const TqFloat& val ) = 0;
	///	Set the value to the specified boolean value.
	virtual void SetBool( const bool& val ) = 0;
	///	Set the value to the specified string.
	virtual void SetString( const CqString& val ) = 0;
	///	Set the value to the specified point.
	virtual void SetPoint( const CqVector3D& val ) = 0;
	///	Set the value to the specified vector.
	virtual void SetVector( const CqVector3D& val ) = 0;
	///	Set the value to the specified normal.
	virtual void SetNormal( const CqVector3D& val ) = 0;
	///	Set the value to the specified color.
	virtual void SetColor( const CqColor& val ) = 0;
	///	Set the value to the specified matrix.
	virtual void SetMatrix( const CqMatrix& val ) = 0;

	///	Set the value to the specified float.
	virtual void SetFloat( const TqFloat& val, TqInt index ) = 0;
	///	Set the value to the specified boolean value.
	virtual void SetBool( const bool& val, TqInt index ) = 0;
	///	Set the value to the specified string.
	virtual void SetString( const CqString& val, TqInt index ) = 0;
	///	Set the value to the specified point.
	virtual void SetPoint( const CqVector3D& val, TqInt index ) = 0;
	///	Set the value to the specified vector.
	virtual void SetVector( const CqVector3D& val, TqInt index ) = 0;
	///	Set the value to the specified normal.
	virtual void SetNormal( const CqVector3D& val, TqInt index ) = 0;
	///	Set the value to the specified color.
	virtual void SetColor( const CqColor& val, TqInt index ) = 0;
	///	Set the value to the specified matrix.
	virtual void SetMatrix( const CqMatrix& val, TqInt index ) = 0;


	void GetValue( TqFloat& f, TqInt index = 0 ) const
	{
		GetFloat( f, index );
	}
	void GetValue( bool& b, TqInt index = 0 ) const
	{
		GetBool( b, index );
	}
	void GetValue( CqString& s, TqInt index = 0 ) const
	{
		GetString( s, index );
	}
	void GetValue( CqVector3D& p, TqInt index = 0 ) const
	{
		GetPoint( p, index );
	}
	void GetValue( CqColor& c, TqInt index = 0 ) const
	{
		GetColor( c, index );
	}
	void GetValue( CqMatrix& m, TqInt index = 0 ) const
	{
		GetMatrix( m, index );
	}

	void GetValuePtr( const TqFloat*& f ) const
	{
		GetFloatPtr( f );
	}
	void GetValuePtr( const bool*& b ) const
	{
		GetBoolPtr( b );
	}
	void GetValuePtr( const CqString*& s ) const
	{
		GetStringPtr( s );
	}
	void GetValuePtr( const CqVector3D*& p ) const
	{
		GetPointPtr( p );
	}
	void GetValuePtr( const CqColor*& c ) const
	{
		GetColorPtr( c );
	}
	void GetValuePtr( const CqMatrix*& m ) const
	{
		GetMatrixPtr( m );
	}

	void GetValuePtr( TqFloat*& f )
	{
		GetFloatPtr( f );
	}
	void GetValuePtr( bool*& b )
	{
		GetBoolPtr( b );
	}
	void GetValuePtr( CqString*& s )
	{
		GetStringPtr( s );
	}
	void GetValuePtr( CqVector3D*& p )
	{
		GetPointPtr( p );
	}
	void GetValuePtr( CqColor*& c )
	{
		GetColorPtr( c );
	}
	void GetValuePtr( CqMatrix*& m )
	{
		GetMatrixPtr( m );
	}

	void SetValue( const TqFloat& f )
	{
		SetFloat( f );
	}
	void SetValue( const TqInt& i )
	{
		SetFloat( static_cast<TqFloat>( i ) );
	}
	void SetValue( const bool& b )
	{
		SetBool( b );
	}
	void SetValue( const CqString& s )
	{
		SetString( s );
	}
	void SetValue( const CqVector3D& p )
	{
		SetPoint( p );
	}
	void SetValue( const CqColor& c )
	{
		SetColor( c );
	}
	void SetValue( const CqMatrix& m )
	{
		SetMatrix( m );
	}

	void SetValue( const TqFloat& f, TqInt index )
	{
		SetFloat( f, index );
	}
	void SetValue( const TqInt& i , TqInt index )
	{
		SetFloat( static_cast<TqFloat>( i ), index );
	}
	void SetValue( const bool& b, TqInt index )
	{
		SetBool( b, index );
	}
	void SetValue( const CqString& s, TqInt index )
	{
		SetString( s, index );
	}
	void SetValue( const CqVector3D& p, TqInt index )
	{
		SetPoint( p, index );
	}
	void SetValue( const CqColor& c, TqInt index )
	{
		SetColor( c, index );
	}
	void SetValue( const CqMatrix& m, TqInt index )
	{
		SetMatrix( m, index );
	}

	/** Get the name of this variable.
	 * \return Read only reference to a CqString class.
	 */
	virtual	const CqString&	strName() const = 0;

	virtual	TqUlong	strNameHash() const = 0;

	/** Get the storage type
 	 *
	 * \return the storage type on the shader, that is, whether it's a
	 * temporary, parameter, etc.
	 */
	virtual	EqStorage Storage() const = 0;

	/** Pure virtual, prepare the variable for the SIMD size.
	 * \param uGridRes The size of the SIMD grid in u.
	 * \param vGridRes The size of the SIMD grid in v.
	 */
	virtual	void	Initialise( const TqInt varyingSize ) = 0;
	/** Create a duplicate of this variable.
	 * \return A IqShaderData pointer.
	 */
	virtual	IqShaderData* Clone() const = 0;

	/** Set the all SIMD data ased on a state vector, only indexes whose bit is set are modified.
	 * \param pVal The stack entry to assign.
	 * \param index Integer SIMD index.
	 */
	virtual	void	SetValueFromVariable( const IqShaderData* pVal, TqInt index ) = 0;
	/** Copy the values from the passed variable into this, taking into account any class differences.
	 * \param pVal The variable to copy from.
	 */
	virtual	void	SetValueFromVariable( const IqShaderData* pVal ) = 0;
	/** Get an indexed SIMD data value.
	 */
	virtual	EqVariableClass	Class() const = 0;
	/** Get the variable type.
	 * \return Type as a member of EqVariableType.
	 */
	virtual	EqVariableType	Type() const = 0;
	/** Get the SIMD size of this variable.
	 * \return Integer SIMD data size.
	 */
	virtual TqUint	Size() const = 0;
	/** Set the SIMD size of this variable.
	 * \param size Integer SIMD data size.
	 */
	virtual void	SetSize( const TqUint size ) = 0;
	/** Get the length of the variable array.
	 * \return Integer array length.
	 */
	virtual TqInt	ArrayLength() const = 0;
	/** Determine if this variable is an array.
	 * \return True if an array, false otherwise.
	 */
	virtual bool	isArray() const = 0;
	/** Get the i'th entry of the variable array.
	 * \return Pointer to an object implementing IqShaderData.
	 */
	virtual IqShaderData*	ArrayEntry( TqInt i ) = 0;
}
;

//-----------------------------------------------------------------------

} // namespace Aqsis

#endif //ISHADERDATA_H_INCLUDED
