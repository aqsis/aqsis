//------------------------------------------------------------------------------
/**
 *	@file	ishader.h
 *	@author	Paul Gregory
 *	@brief	Declare the interface which all shaders must implement.
 *
 *	Last change by:		$Author: pgregory $
 *	Last change date:	$Date: 2002/04/24 00:25:37 $
 */
//------------------------------------------------------------------------------


#ifndef	___ishader_Loaded___
#define	___ishader_Loaded___

#include	"aqsis.h"


START_NAMESPACE( Aqsis )


//----------------------------------------------------------------------
/** \class CqShader
 * Abstract base class from which all shaders must be defined.
 */

struct IqShader
{
		virtual	~IqShader()	{}
		/** Get the shader matrix, the transformation at the time this shader was instantiated.
		 */
		virtual	CqMatrix&	matCurrent() = 0;
		/** Set the naem of the shader.
		 */
		virtual	void	SetstrName( const char* strName ) = 0;
		/** Get the name of this shader.
		 */
		virtual	const CqString& strName() const = 0;
		/** Set a named shader paramter to the specified value.
		 * \param name Character pointer to parameter name.
		 * \param val Character pointer to the vale storage, will be cast to a pointer of the appropriate type.
		 */
		virtual	void	SetValue( const char* name, TqPchar val ) = 0;
		/** Get the value of a named shader paramter.
		 * \param name The name of the shader paramter.
		 * \param res IqShaderData pointer to store the result in, will be typechecked for suitability.
		 * \return Boolean indicating the parameter existed and res was of an appropriate type.
		 */
		virtual	TqBool	GetValue( const char* name, IqShaderData* res ) = 0;
		/** Evaluate the shader code.
		 * \param Env The shader execution environment to evaluate within.
		 */
		virtual	void	Evaluate( CqShaderExecEnv& Env ) = 0;
		/** Initialise the state of any arguments with default values.
		 */
		virtual	void	PrepareDefArgs() = 0;
		/** Prepare the shader for evaluation.
		 * \param uGridRes The resolution of the grid being shaded in u.
		 * \param vGridRes The resolution of the grid being shaded in v
		 * \param Env Pointer to the CqShaderExecEnv to evaluate within.
		 */
		virtual void	Initialise( const TqInt uGridRes, const TqInt vGridRes, CqShaderExecEnv& Env ) = 0;
		/** Determine whether this shader is an aambient ligthsource shader.
		 * i.e. A lightsource shader with no Illuminate or Solar constructs.
		 */
		virtual	TqBool	fAmbient() const = 0;
		/** Duplicate this shader.
		 * \return A pointer to a new shader.
		 */
		virtual IqShader*	Clone() const = 0;
		/** Determine whether this shader uses the specified system variable.
		 * \param Var ID of the variable from EqEnvVars.
		 */
		virtual	TqBool	Uses( TqInt Var ) const = 0;
		/** Get a bit vector representing the system variables needed by this shader.
		 */
		virtual	TqInt	Uses() const = 0;
};




END_NAMESPACE( Aqsis )

#endif	//	___ishader_Loaded___
