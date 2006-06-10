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


#ifndef	___ishadervariable_Loaded___
#define	___ishadervariable_Loaded___

#include	"aqsis.h"

#include	"vector3d.h"
#include	"matrix.h"
#include	"sstring.h"


START_NAMESPACE( Aqsis )


//----------------------------------------------------------------------
/** \enum EqVariableType
 * Shader variable type identifier.
 * \attention Any change to this MUST be mirrored in the type identifier and name string tables.
 */
enum EqVariableClass
{
    class_invalid = 0,

    class_constant,
    class_uniform,
    class_varying,
    class_vertex,
    class_facevarying,
};

//----------------------------------------------------------------------
/** \enum EqVariableType
 * Shader variable type identifier.
 * \attention Any change to this MUST be mirrored in the type identifier and name string tables.
 */
enum EqVariableType
{
    type_invalid = 0,

    type_float = 1,
    type_integer,
    type_point,
    type_string,
    type_color,
    type_triple,
    type_hpoint,
    type_normal,
    type_vector,
    type_void,
    type_matrix,
    type_sixteentuple,
    type_bool,

    type_last,
};

//----------------------------------------------------------------------
/** \struct IqShaderData
 * Interface to the shader engine specific implementation of a shader variable.
 */

struct IqShaderData
{
	/// Virtual destructor so that derived classes get cleaned up properly.
	virtual	~IqShaderData()
	{}

	///	Get the data as a float..

	virtual void GetFloat( TqFloat& res, TqInt index = 0 ) const = 0;
	///	Get the data as a boolean value..
	virtual void GetBool( TqBool& res, TqInt index = 0 ) const = 0;
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
	virtual void GetBoolPtr( const TqBool*& res ) const = 0;
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
	virtual void GetBoolPtr( TqBool*& res ) = 0;
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
	virtual void SetBool( const TqBool& val ) = 0;
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
	virtual void SetBool( const TqBool& val, TqInt index ) = 0;
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
	void GetValue( TqBool& b, TqInt index = 0 ) const
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
	void GetValuePtr( const TqBool*& b ) const
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
	void GetValuePtr( TqBool*& b )
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
	void SetValue( const TqBool& b )
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
	void SetValue( const TqBool& b, TqInt index )
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
	virtual	const CqString&	strName() = 0;
	/** Determine whether this data storage represents a shader argument.
	 * \return Read only reference to a CqString class.
	 */
	virtual	TqBool	fParameter() const = 0;

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
	/** Get the i'th entry of the variable array.
	 * \return Pointer to an object implementing IqShaderData.
	 */
	virtual IqShaderData*	ArrayEntry( TqInt i ) = 0;
}
;

//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )


#endif	//	___ishadervariable_Loaded___
