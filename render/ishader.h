//------------------------------------------------------------------------------
/**
 *	@file	ishader.h
 *	@author	Paul Gregory
 *	@brief	Declare the interface which all shaders must implement.
 *
 *	Last change by:		$Author: pgregory $
 *	Last change date:	$Date: 2002/05/09 07:14:09 $
 */
//------------------------------------------------------------------------------


#ifndef	___ishader_Loaded___
#define	___ishader_Loaded___

#include	"aqsis.h"


START_NAMESPACE( Aqsis )


//----------------------------------------------------------------------
/** \enum EqShaderType
 * Shader type identifier.
 */

enum EqShaderType
{
    Type_Surface,  			///< Surface shader
    Type_Lightsource,  		///< Lightsource shader.
    Type_Volume,  			///< Volume shader.
    Type_Displacement,  		///< Displacement shader.
    Type_Transformation,  	///< Transformation shader.
    Type_Imager,  			///< Image shader.
};

#define	USES(a,b)	((a)&(0x00000001<<(b))?TqTrue:TqFalse)


struct IqShaderExecEnv;

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
		virtual	void	SetArgument( const CqString& name, EqVariableType type, const CqString& space, void* val ) = 0;
		/** Get the value of a named shader paramter.
		 * \param name The name of the shader paramter.
		 * \param res IqShaderData pointer to store the result in, will be typechecked for suitability.
		 * \return Boolean indicating the parameter existed and res was of an appropriate type.
		 */
		virtual	TqBool	GetValue( const char* name, IqShaderData* res ) = 0;
		/** Evaluate the shader code.
		 * \param Env The shader execution environment to evaluate within.
		 */
		virtual	void	Evaluate( IqShaderExecEnv* pEnv ) = 0;
		/** Initialise the state of any arguments with default values.
		 */
		virtual	void	PrepareDefArgs() = 0;
		/** Prepare the shader for evaluation.
		 * \param uGridRes The resolution of the grid being shaded in u.
		 * \param vGridRes The resolution of the grid being shaded in v
		 * \param Env Pointer to the IqShaderExecEnv to evaluate within.
		 */
		virtual void	Initialise( const TqInt uGridRes, const TqInt vGridRes, IqShaderExecEnv* pEnv ) = 0;
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
		/** Variable creation function.
		 */
		virtual IqShaderData* CreateVariable(EqVariableType Type, EqVariableClass Class, const CqString& name, TqBool fArgument = TqFalse) = 0;
		/** Variable array creation function.
		 */
		virtual IqShaderData* CreateVariableArray(EqVariableType Type, EqVariableClass Class, const CqString& name, TqInt Count, TqBool fArgument = TqFalse) = 0;
		/** Function to create some temporary storage which complies to the IqShaderData interface.
		 */
		virtual IqShaderData* CreateTemporaryStorage() = 0;
		/** Function to destroy temporary storage created with CreateTemporaryStorage.
		 */
		virtual void DeleteTemporaryStorage( IqShaderData* pData ) = 0;
};




END_NAMESPACE( Aqsis )

#endif	//	___ishader_Loaded___
