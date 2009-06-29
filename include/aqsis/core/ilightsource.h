//------------------------------------------------------------------------------
/**
 *	@file	ilightsource.h
 *	@author	Authors name
 *	@brief	Brief description of the file contents
 *
 *	Last change by:		$Author$
 *	Last change date:	$Date$
 */
//------------------------------------------------------------------------------


#ifndef	ILIGHTSOURCE_H_INCLUDED
#define	ILIGHTSOURCE_H_INCLUDED

#include <aqsis/aqsis.h>

#include <aqsis/core/interfacefwd.h>

namespace Aqsis {

struct IqShader;
struct IqShaderData;

//----------------------------------------------------------------------
/** \struct IqLightsource
 * Interface definition for access to lightsources.
 */

struct IqLightsource
{
	virtual	~IqLightsource()
	{}

	/** Get a pointer to the associated lightsource shader.
	 * \return a pointer to a IqShader derived class.
	 */
	virtual	boost::shared_ptr<IqShader>	pShader() const = 0;
	/** Initialise the shader execution environment.
	 * \param uGridRes Integer grid size, not used.
	 * \param vGridRes Integer grid size, not used.
	 */
	virtual	void	Initialise( TqInt uGridRes, TqInt vGridRes, TqInt microPolygonCount, TqInt shadingPointCount, bool hasValidDerivatives ) = 0;
	//			void		GenerateShadowMap(const char* strShadowName);
	/** Evaluate the shader.
	 * \param pPs the point being lit.
	 */
	virtual	void	Evaluate( IqShaderData* pPs, IqShaderData* pNs, IqSurface* pSurface ) = 0;
	/** Get a pointer to the attributes associated with this lightsource.
	 * \return a CqAttributes pointer.
	 */
	virtual IqConstAttributesPtr	pAttributes() const = 0;

	// Redirect acces via IqShaderExecEnv
	virtual	TqInt	uGridRes() const = 0;
	virtual	TqInt	vGridRes() const = 0;
	virtual	TqInt	microPolygonCount() const = 0;
	virtual	TqInt	shadingPointCount() const = 0;
	virtual	IqShaderData* Cs() = 0;
	virtual	IqShaderData* Os() = 0;
	virtual	IqShaderData* Ng() = 0;
	virtual	IqShaderData* du() = 0;
	virtual	IqShaderData* dv() = 0;
	virtual	IqShaderData* L() = 0;
	virtual	IqShaderData* Cl() = 0;
	virtual IqShaderData* Ol() = 0;
	virtual IqShaderData* P() = 0;
	virtual IqShaderData* dPdu() = 0;
	virtual IqShaderData* dPdv() = 0;
	virtual IqShaderData* N() = 0;
	virtual IqShaderData* u() = 0;
	virtual IqShaderData* v() = 0;
	virtual IqShaderData* s() = 0;
	virtual IqShaderData* t() = 0;
	virtual IqShaderData* I() = 0;
	virtual IqShaderData* Ci() = 0;
	virtual IqShaderData* Oi() = 0;
	virtual IqShaderData* Ps() = 0;
	virtual IqShaderData* E() = 0;
	virtual IqShaderData* ncomps() = 0;
	virtual IqShaderData* time() = 0;
	virtual IqShaderData* alpha() = 0;

	virtual IqShaderData* Ns() = 0;
};

//-----------------------------------------------------------------------

} // namespace Aqsis

#endif // ILIGHTSOURCE_H_INCLUDED
