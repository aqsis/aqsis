//------------------------------------------------------------------------------
/**
 *	@file	iattributes.h
 *	@author	Paul Gregory
 *	@brief	Declare the interface class for common attributes access.
 *
 *	Last change by:		$Author$
 *	Last change date:	$Date$
 */
//------------------------------------------------------------------------------


#ifndef	___iattributes_Loaded___
#define	___iattributes_Loaded___

#include	<aqsis/aqsis.h>

#include	<aqsis/util/sstring.h>
#include	<aqsis/math/vector3d.h>
#include	<aqsis/math/matrix.h>
#include	<aqsis/math/color.h>


namespace Aqsis {

enum ShadingInterpolation
{
	ShadingInterp_Constant,	///< use constant shading, i.e. one value per micropoly.
	ShadingInterp_Smooth,	///< use smooth shading, i.e. interpolate the values at the corners of a micropoly.
};


struct IqLightsource;
struct IqShader;

struct IqAttributes
{
	virtual ~IqAttributes()
	{}

	virtual const	IqParameter* GetAttribute( const char* strName, const char* strParam ) const = 0;
	virtual IqParameter* GetAttributeWrite( const char* strName, const char* strParam ) = 0;

	/** Get a named float attribute as read only
	 */
	virtual	const	TqFloat*	GetFloatAttribute( const char* strName, const char* strParam ) const = 0;
	/** Get a named integer attribute as read only
	 */
	virtual	const	TqInt*	GetIntegerAttribute( const char* strName, const char* strParam ) const = 0;
	/** Get a named string attribute as read only
	 */
	virtual	const	CqString* GetStringAttribute( const char* strName, const char* strParam ) const = 0;
	/** Get a named point attribute as read only
	 */
	virtual	const	CqVector3D*	GetPointAttribute( const char* strName, const char* strParam ) const = 0;
	/** Get a named point attribute as read only
	 */
	virtual	const	CqVector3D*	GetVectorAttribute( const char* strName, const char* strParam ) const = 0;
	/** Get a named point attribute as read only
	 */
	virtual	const	CqVector3D*	GetNormalAttribute( const char* strName, const char* strParam ) const = 0;
	/** Get a named color attribute as read only
	 */
	virtual	const	CqColor*	GetColorAttribute( const char* strName, const char* strParam ) const = 0;
	/** Get a named matrix attribute as read only
	 */
	virtual	const	CqMatrix*	GetMatrixAttribute( const char* strName, const char* strParam ) const = 0;

	/** Get a named integer attribute; if not found, return the default value provided.
	 */
	virtual const	TqInt	GetIntegerAttributeDef( const char* strName, const char* strParam, TqInt defaultVal) const = 0;

	/** Get a named float attribute as writable
	 */
	virtual	TqFloat*	GetFloatAttributeWrite( const char* strName, const char* strParam ) = 0;
	/** Get a named integer attribute as writable
	 */
	virtual	TqInt*	GetIntegerAttributeWrite( const char* strName, const char* strParam ) = 0;
	/** Get a named string attribute as writable
	 */
	virtual	CqString* GetStringAttributeWrite( const char* strName, const char* strParam ) = 0;
	/** Get a named point attribute as writable
	 */
	virtual	CqVector3D*	GetPointAttributeWrite( const char* strName, const char* strParam ) = 0;
	/** Get a named point attribute as writable
	 */
	virtual	CqVector3D*	GetVectorAttributeWrite( const char* strName, const char* strParam ) = 0;
	/** Get a named point attribute as writable
	 */
	virtual	CqVector3D*	GetNormalAttributeWrite( const char* strName, const char* strParam ) = 0;
	/** Get a named color attribute as writable
	 */
	virtual	CqColor*	GetColorAttributeWrite( const char* strName, const char* strParam ) = 0;
	/** Get a named matrix attribute as writable
	 */
	virtual	CqMatrix*	GetMatrixAttributeWrite( const char* strName, const char* strParam ) = 0;

	/** Get the current dislacement shader.
	 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
	 * \return a pointer to the displacement shader.
	 */
	virtual	boost::shared_ptr<IqShader>	pshadDisplacement( TqFloat time ) const = 0;
	/** Set the current displacement shader.
	 * \param pshadDisplacement a pointer to a shader to use as the displacement shader.
	 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
	 */
	virtual	void	SetpshadDisplacement( const boost::shared_ptr<IqShader>& pshadDisplacement, TqFloat time ) = 0;

	/** Get the current area light source shader.
	 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
	 * \return a pointer to the displacement shader.
	 */
	virtual	boost::shared_ptr<IqShader>	pshadAreaLightSource( TqFloat time ) const = 0;
	/** Set the current area light source shader.
	 * \param pshadAreaLightSource a pointer to a shader to use as the area light source shader.
	 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
	 */
	virtual	void	SetpshadAreaLightSource( const boost::shared_ptr<IqShader>& pshadAreaLightSource, TqFloat time ) = 0;

	/** Get the current surface shader.
	 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
	 * \return a pointer to the displacement shader.
	 */
	virtual	boost::shared_ptr<IqShader>	pshadSurface( TqFloat time ) const = 0;
	/** Set the current surface shader.
	 * \param pshadSurface a pointer to a shader to use as the surface shader.
	 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
	 */
	virtual	void	SetpshadSurface( const boost::shared_ptr<IqShader>& pshadSurface, TqFloat time ) = 0;

	/** Get the current atmosphere shader.
	 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
	 * \return a pointer to the displacement shader.
	 */
	virtual	boost::shared_ptr<IqShader>	pshadAtmosphere( TqFloat time ) const = 0;
	/** Set the current atmosphere shader.
	 * \param pshadAtmosphere a pointer to a shader to use as the atmosphere shader.
	 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
	 */
	virtual	void	SetpshadAtmosphere( const boost::shared_ptr<IqShader>& pshadAtmosphere, TqFloat time ) = 0;

	/** Get the current external volume shader.
	 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
	 * \return a pointer to the displacement shader.
	 */
	virtual	boost::shared_ptr<IqShader>	pshadExteriorVolume( TqFloat time ) const = 0;
	/** Set the current external volume shader.
	 * \param pshadExteriorVolume a pointer to a shader to use as the exterior volume shader.
	 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
	 */
	virtual	void	SetpshadExteriorVolume( const boost::shared_ptr<IqShader>& pshadExteriorVolume, TqFloat time ) = 0;

	/** Get the current internal volume shader.
	 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
	 * \return a pointer to the displacement shader.
	 */
	virtual	boost::shared_ptr<IqShader>	pshadAreaInteriorVolume( TqFloat time ) const = 0;
	/** Set the current internal volume shader.
	 * \param pshadInteriorVolume a pointer to a shader to use as the interior volume shader.
	 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
	 */
	virtual	void	SetpshadInteriorVolume( const boost::shared_ptr<IqShader>& pshadInteriorVolume, TqFloat time ) = 0;

	virtual	TqUint	cLights() const	= 0;
	virtual	IqLightsource*	pLight( TqInt index ) const = 0;
};

//-----------------------------------------------------------------------

} // namespace Aqsis

#endif	//	___iattributes_Loaded___
