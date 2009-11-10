//------------------------------------------------------------------------------
/**
 *	@file	ishader.h
 *	@author	Paul Gregory
 *	@brief	Declare the interface which all shaders must implement.
 *
 *	Last change by:		$Author$
 *	Last change date:	$Date$
 */
//------------------------------------------------------------------------------


#ifndef	ISHADER_H_INCLUDED
#define	ISHADER_H_INCLUDED

#include	<aqsis/aqsis.h>

#include	<string>
#include	<vector>
#include	<iosfwd>

#include	<boost/shared_ptr.hpp>

#include	<aqsis/core/interfacefwd.h>
#include	<aqsis/riutil/primvartype.h>
#include	<aqsis/shadervm/ishaderdata.h>
#include	<aqsis/util/exception.h>

namespace Aqsis {


//----------------------------------------------------------------------
/** \enum EqShaderType
 * Shader type identifier.
 */

enum EqShaderType
{
    Type_Surface,   			///< Surface shader
    Type_Lightsource,   		///< Lightsource shader.
    Type_Volume,   			///< Volume shader.
    Type_Displacement,   		///< Displacement shader.
    Type_Transformation,   	///< Transformation shader.
    Type_Imager,   			///< Image shader.
};

#define	USES(a,b)	((a)&(0x00000001<<(b))?true:false)
#define	isDONE(a,b)	((a)&(0x00000001<<(b))?true:false)
#define	DONE(a,b)	((a)=(a)|(0x00000001<<(b)))


struct IqShaderExecEnv;
struct IqShaderData;
struct IqSurface;
struct IqParameter;
struct IqRenderer;
class CqMatrix;
class CqString;


//----------------------------------------------------------------------
/** \class XqBadShader
 * \brief Exception thrown when loading a shader program fails for some reason.
 *
 * If a compiled shader was generated with the wrong version of aqsis, contains
 * unrecognized shadops or is otherwise invalid, this is the exception to
 * throw.
 */
AQSIS_DECLARE_XQEXCEPTION(XqBadShader, XqInternal);


//----------------------------------------------------------------------
/** \brief Abstract base class from which all shaders must be defined.
 */
struct AQSIS_SHADERVM_SHARE IqShader
{
	virtual	~IqShader()
	{}
	/** Get the shader matrix, the transformation at the time this shader was instantiated.
	 */
	virtual	const CqMatrix&	matCurrent() = 0;
	virtual const IqTransform* getTransform() const = 0;
	virtual void SetTransform(IqTransformPtr pTrans) = 0;
	/** Set the naem of the shader.
	 */
	virtual	void	SetstrName( const char* strName ) = 0;
	/** Get the name of this shader.
	 */
	virtual	const CqString& strName() const = 0;
	virtual	void	PrepareShaderForUse( ) = 0;
	virtual	void	InitialiseParameters( ) = 0;
	virtual	void	SetArgument( const CqString& name, EqVariableType type, const CqString& space, void* val ) = 0;
	virtual	void	SetArgument( IqParameter* pParam, IqSurface* pSurface ) = 0;
	/** Find a named argument.
	 * \param name Character pointer to argument name.
	 * \return A pointer to the argument data if it exists, NULL otherwise.
	 */
	virtual	IqShaderData*	FindArgument( const CqString& name ) = 0;
	/// Get the shader arguments
	virtual const std::vector<IqShaderData*>& GetArguments() const = 0;
	/** Get the value of a named shader paramter.
	 * \param name The name of the shader paramter.
	 * \param res IqShaderData pointer to store the result in, will be typechecked for suitability.
	 * \return Boolean indicating the parameter existed and res was of an appropriate type.
	 */
	virtual	bool	GetVariableValue( const char* name, IqShaderData* res ) const = 0;
	/** Evaluate the shader code.
	 * \param pEnv The shader execution environment to evaluate within.
	 */
	virtual	void	Evaluate( IqShaderExecEnv* pEnv ) = 0;
	/** Initialise the state of any arguments with default values.
	 */
	virtual	void	PrepareDefArgs() = 0;
	/** Prepare the shader for evaluation.
	 * \param uGridRes The resolution of the grid being shaded in u.
	 * \param vGridRes The resolution of the grid being shaded in v
	 * \param pEnv Pointer to the IqShaderExecEnv to evaluate within.
	 */
	virtual void	Initialise( const TqInt uGridRes, const TqInt vGridRes, const TqInt shadingPointCount, IqShaderExecEnv* pEnv ) = 0;
	/** Determine whether this shader is an aambient ligthsource shader.
	 * i.e. A lightsource shader with no Illuminate or Solar constructs.
	 */
	virtual	bool	fAmbient() const = 0;
	/** Duplicate this shader.
	 * \return A pointer to a new shader.
	 */
	virtual boost::shared_ptr<IqShader> Clone() const = 0;
	/** Determine whether this shader uses the specified system variable.
	 * \param Var ID of the variable from EqEnvVars.
	 */
	virtual	bool	Uses( TqInt Var ) const = 0;
	/** Get a bit vector representing the system variables needed by this shader.
	 */
	virtual	TqInt	Uses() const = 0;
	/** Variable creation function.
	 */
	virtual IqShaderData* CreateVariable( EqVariableType Type, EqVariableClass Class, const CqString& name, IqShaderData::EqStorage storage = IqShaderData::Unknown ) = 0;
	/** Variable array creation function.
	 */
	virtual IqShaderData* CreateVariableArray( EqVariableType Type, EqVariableClass Class, const CqString& name, TqInt Count, IqShaderData::EqStorage storage = IqShaderData::Unknown ) = 0;
	/** Function to create some temporary storage which complies to the IqShaderData interface.
	 */
	virtual IqShaderData* CreateTemporaryStorage( EqVariableType type, EqVariableClass _class ) = 0;
	/** Function to destroy temporary storage created with CreateTemporaryStorage.
	 */
	virtual void DeleteTemporaryStorage( IqShaderData* pData ) = 0;

	/** Use the default surface shader for this implementation.
	 */
	virtual void DefaultSurface( ) = 0;
	/** Query if the shader is a layered shader
	 */
	virtual bool IsLayered() = 0;
	virtual void AddLayer(const CqString& layername, const boost::shared_ptr<IqShader>& layer) = 0;
	virtual void AddConnection(const CqString& layer1, const CqString& variable1, const CqString& layer2, const CqString& variable2) = 0;
	virtual void SetType(EqShaderType type) = 0;
	/// Get the shader type
	virtual EqShaderType Type() const = 0;
};


//@{
// \name Transitional interfaces for managing CqShaderVM

//@{
/** \brief Factory functions for CqShaderVM instances
 *
 * \param renderContext - Context within which the shader will operate
 * \param programFile - file from which to read the shader program
 * \param dsoPath - search path for DSO shadeops.
 */
AQSIS_SHADERVM_SHARE boost::shared_ptr<IqShader> createShaderVM(IqRenderer* renderContext);

AQSIS_SHADERVM_SHARE boost::shared_ptr<IqShader> createShaderVM(IqRenderer* renderContext,
										   std::istream& programFile,
										   const std::string& dsoPath);
//@}

/** \brief Reset ShaderVM static variables
 *
 * The shaderVM keeps a pool of shader variables handy to avoid reallocation;
 * this function erases those to free up memory.
 *
 * \todo Move this elsewhere, perhaps into a class managing the current
 * instances of CqShaderVM?
 */
AQSIS_SHADERVM_SHARE void shutdownShaderVM();

//@}

} // namespace Aqsis

#endif	// ISHADER_H_INCLUDED
