//------------------------------------------------------------------------------
/**
 *	@file	ishadervariable.h
 *	@author	Paul Gregory
 *	@brief	Decares the interface to generic shader variables.
 *
 *	Last change by:		$Author: pgregory $
 *	Last change date:	$Date: 2002/04/11 21:56:53 $
 */
//------------------------------------------------------------------------------


#ifndef	___ishadervariable_Loaded___
#define	___ishadervariable_Loaded___

#include	"aqsis.h"


START_NAMESPACE( Aqsis )

class CqShaderExecEnv;
class CqVMStackEntry;


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

	type_last,
};

//----------------------------------------------------------------------
/** \struct IqShaderVariable
 * Interface to the shader engine specific implementation of a shader variable.
 */

struct IqShaderVariable
{
		/** Get the name of this variable.
		 * \return Read only reference to a CqString class.
		 */
		virtual	const CqString&	strName()=0;

		/** Pure virtual, prepare the variable for the SIMD size.
		 * \param uGridRes The size of the SIMD grid in u.
		 * \param vGridRes The size of the SIMD grid in v.
		 * \param index A Reference to a SIMD index used to obtain the 'current', in terms of SIMD execution, value.
		 */
		virtual	void	Initialise( const TqInt uGridRes, const TqInt vGridRes, TqInt& index ) = 0;
		/** Create a duplicate of this variable.
		 * \return A IqShaderVariable pointer.
		 */
		virtual	IqShaderVariable* Clone() const = 0;
		/** Set the 'current', in terms of SIMD execution, value.
		 * \param Val The new value.
		 */
		virtual	void	SetValue( CqVMStackEntry& Val ) = 0;
		/** Set a specified SIMD index value.
		 * \param index Integer SIMD index.
		 * \param Val The new value.
		 */
		virtual	void	SetValue( TqInt index, CqVMStackEntry& Val ) = 0;
		/** Set the all SIMD data ased on a state vector, only indexes whose bit is set are modified.
		 * \param Val The stack entry to assign.
		 * \param State The bit vector to control modification.
		 */
		virtual	void	SetValue( CqVMStackEntry& Val, CqBitVector& State ) = 0;
		/** Get an indexed SIMD data value.
		 * \param index Integer SIMD index.
		 * \param Val A reference to a stackentry to store the value.
		 */
		virtual	void	GetValue( TqInt index, CqVMStackEntry& Val ) const = 0;
		/** Get the variable class.
		 * \return Class as a member of EqVariableClass.
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
		/** Get the length of the variable array.
		 * \return Integer array length.
		 */
		virtual TqInt	ArrayLength() const=0;
}
;

//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )


#endif	//	___ishadervariable_Loaded___
