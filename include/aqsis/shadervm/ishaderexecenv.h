// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
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
		\brief Declares interface used to access the shader execution environment.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#ifndef	ISHADEREXECENV_H_INCLUDED
#define	ISHADEREXECENV_H_INCLUDED

#include	<aqsis/aqsis.h>

#include	<boost/shared_ptr.hpp>

#include	<aqsis/shadervm/ishaderdata.h>
#include	<aqsis/math/vector3d.h>
#include	<aqsis/math/derivatives.h>
#include	<aqsis/math/matrix.h>
#include	<aqsis/util/sstring.h>
#include	<aqsis/util/bitvector.h>
#include	<aqsis/core/interfacefwd.h>

namespace Aqsis {

struct IqShader;
struct IqRenderer;

// We declare these here for access from shaderexecenv
typedef void (*DSOMethod)(void*,int,void**);
typedef void* (*DSOInit)(int,void*);
typedef void (*DSOShutdown)(void*);

/** \enum EqEnvVars
 * Identifiers for the standard environment variables.
 */

enum EqEnvVars
{
    EnvVars_Cs,   		///< Surface color.
    EnvVars_Os,   		///< Surface opacity.
    EnvVars_Ng,   		///< Geometric normal.
    EnvVars_du,   		///< First derivative in u.
    EnvVars_dv,   		///< First derivative in v.
    EnvVars_L,   		///< Incoming light direction.
    EnvVars_Cl,   		///< Light color.
    EnvVars_Ol,   		///< Light opacity.
    EnvVars_P,   		///< Point being shaded.
    EnvVars_dPdu,   	///< Change in P with respect to change in u.
    EnvVars_dPdv,   	///< Change in P with respect to change in v.
    EnvVars_N,   		///< Surface normal.
    EnvVars_u,   		///< Surface u coordinate.
    EnvVars_v,   		///< Surface v coordinate.
    EnvVars_s,   		///< Texture s coordinate.
    EnvVars_t,   		///< Texture t coordinate.
    EnvVars_I,   		///< Incident ray direction.
    EnvVars_Ci,   		///< Incident color.
    EnvVars_Oi,   		///< Incident opacity.
    EnvVars_Ps,   		///< Point being lit.
    EnvVars_E,   		///< Viewpoint position.
    EnvVars_ncomps,   	///< Number of color components.
    EnvVars_time,   	///< Frame time.
    EnvVars_alpha,   	///< Fractional pixel coverage.

    EnvVars_Ns,   		///< Normal at point being lit.

    EnvVars_Last
};

/// Vector of variable names corresponding to EqEnvVars
AQSIS_SHADERVM_SHARE extern const char*	gVariableNames[];
/// Vector of hash key from gVariableNames
AQSIS_SHADERVM_SHARE extern TqUlong	gVariableTokens[];

/// Variables needed by default for normal shaders
AQSIS_SHADERVM_SHARE extern TqInt gDefUses;
/// Variables needed by default for light shaders
AQSIS_SHADERVM_SHARE extern TqInt gDefLightUses;


#define	STD_SO		void
#define	STD_SOIMPL	void

#define	FLOATVAL	IqShaderData*
#define	POINTVAL	IqShaderData*
#define	VECTORVAL	IqShaderData*
#define	NORMALVAL	IqShaderData*
#define	COLORVAL	IqShaderData*
#define	STRINGVAL	IqShaderData*
#define	MATRIXVAL	IqShaderData*

#define	FLOATPTR	IqShaderData**
#define	POINTPTR	IqShaderData**
#define	VECTORPTR	IqShaderData**
#define	NORMALPTR	IqShaderData**
#define	COLORPTR	IqShaderData**
#define	STRINGPTR	IqShaderData**
#define	MATRIXPTR	IqShaderData**

#define	FLOATARRAYVAL	IqShaderData*
#define	POINTARRAYVAL	IqShaderData*
#define	VECTORARRAYVAL	IqShaderData*
#define	NORMALARRAYVAL	IqShaderData*
#define	COLORARRAYVAL	IqShaderData*
#define	STRINGARRAYVAL	IqShaderData*
#define	MATRIXARRAYVAL	IqShaderData*

#define	DEFPARAM		IqShaderData* Result,IqShader* pShader=0
#define	DEFVOIDPARAM	        IqShader* pShader=0
#define	DEFPARAMVAR		DEFPARAM, int cParams=0, IqShaderData** apParams=0
#define	DEFVOIDPARAMVAR	DEFVOIDPARAM, int cParams=0, IqShaderData** apParams=0


//----------------------------------------------------------------------
/** \struct IqShaderExecEnv
 * Interface to shader execution environment.
 * The shader execution environment is responsible for providing functionality for accessing shader data on a
 * 'shadable' item, and providing shadeops to process that data.
 */

struct AQSIS_SHADERVM_SHARE IqShaderExecEnv
{
	/// Create an IqShaderExecEnv instance with the given render context.
	static boost::shared_ptr<IqShaderExecEnv> create(IqRenderer* context);

	virtual	~IqShaderExecEnv()
	{}

	virtual	void	Initialise( const TqInt uGridRes, const TqInt vGridRes, 
		TqInt microPolygonCount, TqInt shadingPointCount, 
		bool hasValidDerivatives,
		const IqConstAttributesPtr& pAttr, 
		const IqConstTransformPtr& pTrans, 
		IqShader* pShader, 
		TqInt Uses ) = 0;
	/** Get grid size in u
	 */
	virtual	TqInt	uGridRes() const = 0;
	/** Get grid size in v
	 */
	virtual	TqInt	vGridRes() const = 0;
	/** Get total grid size.
	 */
	virtual	TqUint	microPolygonCount() const = 0;
	/** Get shading point count.
	 */
	virtual	TqUint	shadingPointCount() const = 0;
	/** Get the matrix which describes the transformation from Object space to World space for the surface related to this execution environment.
	 */
	virtual	const CqMatrix&	matObjectToWorld() const = 0;
	/** Get a pointer to the associated attributes.
	 */
	virtual	const IqConstAttributesPtr pAttributes() const = 0;
	/** Get a pointer to the associated transform.
	 */
	virtual	const IqConstTransformPtr pTransform() const = 0;
	/** Set the pointer to the currently being lit surface
	 */
	virtual void SetCurrentSurface(IqSurface* pEnv) = 0;

	/// Get the grid difference computation object.
	virtual CqGridDiff GridDiff() const = 0;

	/** Get the pointer to the currently being lit surface
	 */
	virtual const IqSurface* GetCurrentSurface() const = 0;
	/** Update all cached lighting results.
	 */
	virtual	void	ValidateIlluminanceCache( IqShaderData* pP, IqShaderData* pN, IqShader* pShader ) = 0;
	/** Reset the illuminance cache.
	 */
	virtual	void	InvalidateIlluminanceCache() = 0;
	/** Get the current execution state. Bits in the vector indicate which SIMD indexes have passed the current condition.
	 */
	virtual	CqBitVector& CurrentState() = 0;
	/** Get the running execution state. Bits in the vector indicate which SIMD indexes are valid.
	 */
	virtual	const CqBitVector& RunningState() const = 0;
	/** Transfer the current state into the running state.
	 */
	virtual	void	GetCurrentState() = 0;
	/** Clear the current state ready for a new condition.
	 */
	virtual	void	ClearCurrentState() = 0;
	/** Push the running state onto the stack.
	 */
	virtual	void	PushState() = 0;
	/** Pop the running state from the stack.
	 */
	virtual	void	PopState() = 0;
	/** Invert the bits in the running state, to perform the opposite to the condition, i.e. else.
	 */
	virtual	void	InvertRunningState() = 0;
	/** \brief Nonlocal "break" and "continue" operations for the running state
	 * stack.
	 *
	 * This function causes all numLevels states on the top of the stack to be
	 * set to non-running for all bits of the running state which on.
	 *
	 * The running state is set to non-running to let the VM fall out of the
	 * current loop scope.
	 */
	virtual void RunningStatesBreak(TqInt numLevels) = 0;
	/** \brief Determine if any of the current running state bits are set.
	 *
	 * Use this in preference to RunningState().Count() != 0, since it may be
	 * cached for efficiency.
	 *
	 * \return true if any SIMD elements are currently running.
	 */
	virtual bool IsRunning() = 0;
	/** Find a named standard variable in the list.
	 * \param pname Character pointer to the name.
	 * \return IqShaderData pointer or 0.
	 */
	virtual	IqShaderData* FindStandardVar( const char* pname ) = 0;
	/** Find a named standard variable in the list.
	 * \param pname Character pointer to the name.
	 * \return Integer index in the list or -1.
	 */
	virtual	TqInt	FindStandardVarIndex( const char* pname ) = 0;
	/** Get a standard variable pointer given an index.
	 * \param Index The integer index returned from FindStandardVarIndex.
	 * \return IqShaderData pointer.
	 */
	virtual	IqShaderData*	pVar( TqInt Index ) = 0;
	/** Delete an indexed variable from the list.
	 * \param Index The integer index returned from FindStandardVarIndex.
	 */
	virtual	void	DeleteVariable( TqInt Index ) = 0;
	/** Get a reference to the Cs standard variable.
	 */
	virtual	IqShaderData* Cs() = 0;
	/** Get a reference to the Os standard variable.
	 */
	virtual	IqShaderData* Os() = 0;
	/** Get a reference to the Ng standard variable.
	 */
	virtual	IqShaderData* Ng() = 0;
	/** Get a reference to the du standard variable.
	 */
	virtual	IqShaderData* du() = 0;
	/** Get a reference to the dv standard variable.
	 */
	virtual	IqShaderData* dv() = 0;
	/** Get a reference to the L standard variable.
	 */
	virtual	IqShaderData* L() = 0;
	/** Get a reference to the Cl standard variable.
	 */
	virtual	IqShaderData* Cl() = 0;
	/** Get a reference to the Ol standard variable.
	 */
	virtual IqShaderData* Ol() = 0;
	/** Get a reference to the P standard variable.
	 */
	virtual IqShaderData* P() = 0;
	/** Get a reference to the dPdu standard variable.
	 */
	virtual IqShaderData* dPdu() = 0;
	/** Get a reference to the dPdv standard variable.
	 */
	virtual IqShaderData* dPdv() = 0;
	/** Get a reference to the N standard variable.
	 */
	virtual IqShaderData* N() = 0;
	/** Get a reference to the u standard variable.
	 */
	virtual IqShaderData* u() = 0;
	/** Get a reference to the v standard variable.
	 */
	virtual IqShaderData* v() = 0;
	/** Get a reference to the s standard variable.
	 */
	virtual IqShaderData* s() = 0;
	/** Get a reference to the t standard variable.
	 */
	virtual IqShaderData* t() = 0;
	/** Get a reference to the I standard variable.
	 */
	virtual IqShaderData* I() = 0;
	/** Get a reference to the Ci standard variable.
	 */
	virtual IqShaderData* Ci() = 0;
	/** Get a reference to the Oi standard variable.
	 */
	virtual IqShaderData* Oi() = 0;
	/** Get a reference to the Ps standard variable.
	 */
	virtual IqShaderData* Ps() = 0;
	/** Get a reference to the E standard variable.
	 */
	virtual IqShaderData* E() = 0;
	/** Get a reference to the ncomps standard variable.
	 */
	virtual IqShaderData* ncomps() = 0;
	/** Get a reference to the time standard variable.
	 */
	virtual IqShaderData* time() = 0;
	/** Get a reference to the alpha standard variable.
	 */
	virtual IqShaderData* alpha() = 0;
	/** Get a reference to the Ns standard variable.
	 */
	virtual IqShaderData* Ns() = 0;

	virtual IqRenderer* getRenderContext() const = 0;

	virtual	bool	SO_init_illuminance() = 0;
	virtual	bool	SO_advance_illuminance() = 0;

	virtual	STD_SO	SO_init_gather(FLOATVAL samples, DEFVOIDPARAM ) = 0;
	virtual	bool	SO_advance_gather() = 0;

	// ShadeOps
	virtual STD_SO	SO_radians( FLOATVAL degrees, DEFPARAM ) = 0;
	virtual STD_SO	SO_degrees( FLOATVAL radians, DEFPARAM ) = 0;
	virtual STD_SO	SO_sin( FLOATVAL a, DEFPARAM ) = 0;
	virtual STD_SO	SO_asin( FLOATVAL a, DEFPARAM ) = 0;
	virtual STD_SO	SO_cos( FLOATVAL a, DEFPARAM ) = 0;
	virtual STD_SO	SO_acos( FLOATVAL a, DEFPARAM ) = 0;
	virtual STD_SO	SO_tan( FLOATVAL a, DEFPARAM ) = 0;
	virtual STD_SO	SO_atan( FLOATVAL yoverx, DEFPARAM ) = 0;
	virtual STD_SO	SO_atan( FLOATVAL y, FLOATVAL x, DEFPARAM ) = 0;
	virtual STD_SO	SO_pow( FLOATVAL x, FLOATVAL y, DEFPARAM ) = 0;
	virtual STD_SO	SO_exp( FLOATVAL x, DEFPARAM ) = 0;
	virtual STD_SO	SO_sqrt( FLOATVAL x, DEFPARAM ) = 0;
	virtual STD_SO	SO_log( FLOATVAL x, DEFPARAM ) = 0;
	virtual STD_SO	SO_log( FLOATVAL x, FLOATVAL base, DEFPARAM ) = 0;
	virtual STD_SO	SO_mod( FLOATVAL a, FLOATVAL b, DEFPARAM ) = 0;
	virtual STD_SO	SO_abs( FLOATVAL x, DEFPARAM ) = 0;
	virtual STD_SO	SO_sign( FLOATVAL x, DEFPARAM ) = 0;
	virtual STD_SO	SO_min( FLOATVAL a, FLOATVAL b, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_max( FLOATVAL a, FLOATVAL b, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_pmin( POINTVAL a, POINTVAL b, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_pmax( POINTVAL a, POINTVAL b, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_cmin( COLORVAL a, COLORVAL b, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_cmax( COLORVAL a, COLORVAL b, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_clamp( FLOATVAL a, FLOATVAL _min, FLOATVAL _max, DEFPARAM ) = 0;
	virtual STD_SO	SO_pclamp( POINTVAL a, POINTVAL _min, POINTVAL _max, DEFPARAM ) = 0;
	virtual STD_SO	SO_cclamp( COLORVAL a, COLORVAL _min, COLORVAL _max, DEFPARAM ) = 0;
	virtual STD_SO	SO_floor( FLOATVAL x, DEFPARAM ) = 0;
	virtual STD_SO	SO_ceil( FLOATVAL x, DEFPARAM ) = 0;
	virtual STD_SO	SO_round( FLOATVAL x, DEFPARAM ) = 0;
	virtual STD_SO	SO_step( FLOATVAL _min, FLOATVAL value, DEFPARAM ) = 0;
	virtual STD_SO	SO_smoothstep( FLOATVAL _min, FLOATVAL _max, FLOATVAL value, DEFPARAM ) = 0;
	virtual STD_SO	SO_fspline( FLOATVAL value, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_cspline( FLOATVAL value, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_pspline( FLOATVAL value, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_sfspline( STRINGVAL basis, FLOATVAL value, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_scspline( STRINGVAL basis, FLOATVAL value, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_spspline( STRINGVAL basis, FLOATVAL value, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_fDu( FLOATVAL p, DEFPARAM ) = 0;
	virtual STD_SO	SO_fDv( FLOATVAL p, DEFPARAM ) = 0;
	virtual STD_SO	SO_fDeriv( FLOATVAL p, FLOATVAL den, DEFPARAM ) = 0;
	virtual STD_SO	SO_cDu( COLORVAL p, DEFPARAM ) = 0;
	virtual STD_SO	SO_cDv( COLORVAL p, DEFPARAM ) = 0;
	virtual STD_SO	SO_cDeriv( COLORVAL p, FLOATVAL den, DEFPARAM ) = 0;
	virtual STD_SO	SO_pDu( POINTVAL p, DEFPARAM ) = 0;
	virtual STD_SO	SO_pDv( POINTVAL p, DEFPARAM ) = 0;
	virtual STD_SO	SO_pDeriv( POINTVAL p, FLOATVAL den, DEFPARAM ) = 0;
	virtual STD_SO	SO_frandom( DEFPARAM ) = 0;
	virtual STD_SO	SO_crandom( DEFPARAM ) = 0;
	virtual STD_SO	SO_prandom( DEFPARAM ) = 0;
	virtual STD_SO	SO_fnoise1( FLOATVAL v, DEFPARAM ) = 0;
	virtual STD_SO	SO_fnoise2( FLOATVAL u, FLOATVAL v, DEFPARAM ) = 0;
	virtual STD_SO	SO_fnoise3( POINTVAL p, DEFPARAM ) = 0;
	virtual STD_SO	SO_fnoise4( POINTVAL p, FLOATVAL t, DEFPARAM ) = 0;
	virtual STD_SO	SO_cnoise1( FLOATVAL v, DEFPARAM ) = 0;
	virtual STD_SO	SO_cnoise2( FLOATVAL u, FLOATVAL v, DEFPARAM ) = 0;
	virtual STD_SO	SO_cnoise3( POINTVAL p, DEFPARAM ) = 0;
	virtual STD_SO	SO_cnoise4( POINTVAL p, FLOATVAL t, DEFPARAM ) = 0;
	virtual STD_SO	SO_pnoise1( FLOATVAL v, DEFPARAM ) = 0;
	virtual STD_SO	SO_pnoise2( FLOATVAL u, FLOATVAL v, DEFPARAM ) = 0;
	virtual STD_SO	SO_pnoise3( POINTVAL p, DEFPARAM ) = 0;
	virtual STD_SO	SO_pnoise4( POINTVAL p, FLOATVAL t, DEFPARAM ) = 0;
	virtual STD_SO	SO_setcomp( COLORVAL p, FLOATVAL i, FLOATVAL v, DEFVOIDPARAM ) = 0;
	virtual STD_SO	SO_setxcomp( POINTVAL p, FLOATVAL v, DEFVOIDPARAM ) = 0;
	virtual STD_SO	SO_setycomp( POINTVAL p, FLOATVAL v, DEFVOIDPARAM ) = 0;
	virtual STD_SO	SO_setzcomp( POINTVAL p, FLOATVAL v, DEFVOIDPARAM ) = 0;
	virtual STD_SO	SO_length( VECTORVAL V, DEFPARAM ) = 0;
	virtual STD_SO	SO_distance( POINTVAL P1, POINTVAL P2, DEFPARAM ) = 0;
	virtual STD_SO	SO_area( POINTVAL p, DEFPARAM ) = 0;
	virtual STD_SO	SO_normalize( VECTORVAL V, DEFPARAM ) = 0;
	virtual STD_SO	SO_faceforward( NORMALVAL N, VECTORVAL I, DEFPARAM ) = 0;
	virtual STD_SO	SO_faceforward2( NORMALVAL N, VECTORVAL I, NORMALVAL Nref, DEFPARAM ) = 0;
	virtual STD_SO	SO_reflect( VECTORVAL I, NORMALVAL N, DEFPARAM ) = 0;
	virtual STD_SO	SO_refract( VECTORVAL I, NORMALVAL N, FLOATVAL eta, DEFPARAM ) = 0;
	virtual STD_SO	SO_fresnel( VECTORVAL I, NORMALVAL N, FLOATVAL eta, FLOATVAL Kr, FLOATVAL Kt, DEFVOIDPARAM ) = 0;
	virtual STD_SO	SO_fresnel( VECTORVAL I, NORMALVAL N, FLOATVAL eta, FLOATVAL Kr, FLOATVAL Kt, VECTORVAL R, VECTORVAL T, DEFVOIDPARAM ) = 0;
	virtual STD_SO	SO_transform( STRINGVAL fromspace, STRINGVAL tospace, POINTVAL p, DEFPARAM ) = 0;
	virtual STD_SO	SO_transform( STRINGVAL tospace, POINTVAL p, DEFPARAM ) = 0;
	virtual STD_SO	SO_transformm( MATRIXVAL tospace, POINTVAL p, DEFPARAM ) = 0;
	virtual STD_SO	SO_vtransform( STRINGVAL fromspace, STRINGVAL tospace, VECTORVAL p, DEFPARAM ) = 0;
	virtual STD_SO	SO_vtransform( STRINGVAL tospace, VECTORVAL p, DEFPARAM ) = 0;
	virtual STD_SO	SO_vtransformm( MATRIXVAL tospace, VECTORVAL p, DEFPARAM ) = 0;
	virtual STD_SO	SO_ntransform( STRINGVAL fromspace, STRINGVAL tospace, NORMALVAL p, DEFPARAM ) = 0;
	virtual STD_SO	SO_ntransform( STRINGVAL tospace, NORMALVAL p, DEFPARAM ) = 0;
	virtual STD_SO	SO_ntransformm( MATRIXVAL tospace, NORMALVAL p, DEFPARAM ) = 0;
	virtual STD_SO	SO_depth( POINTVAL p, DEFPARAM ) = 0;
	virtual STD_SO	SO_calculatenormal( POINTVAL p, DEFPARAM ) = 0;
	virtual STD_SO	SO_cmix( COLORVAL color0, COLORVAL color1, FLOATVAL value, DEFPARAM ) = 0;
	virtual STD_SO	SO_cmixc( COLORVAL color0, COLORVAL color1, COLORVAL value, DEFPARAM ) = 0;
	virtual STD_SO	SO_fmix( FLOATVAL f0, FLOATVAL f1, FLOATVAL value, DEFPARAM ) = 0;
	virtual STD_SO	SO_pmix( POINTVAL p0, POINTVAL p1, FLOATVAL value, DEFPARAM ) = 0;
	virtual STD_SO	SO_vmix( VECTORVAL v0, VECTORVAL v1, FLOATVAL value, DEFPARAM ) = 0;
	virtual STD_SO	SO_nmix( NORMALVAL n0, NORMALVAL n1, FLOATVAL value, DEFPARAM ) = 0;
	virtual STD_SO	SO_pmixc( POINTVAL p0, POINTVAL p1, COLORVAL value, DEFPARAM ) = 0;
	virtual STD_SO	SO_vmixc( VECTORVAL v0, VECTORVAL v1, COLORVAL value, DEFPARAM ) = 0;
	virtual STD_SO	SO_nmixc( NORMALVAL n0, NORMALVAL n1, COLORVAL value, DEFPARAM ) = 0;
	virtual STD_SO	SO_ambient( DEFPARAM ) = 0;
	virtual STD_SO	SO_diffuse( NORMALVAL N, DEFPARAM ) = 0;
	virtual STD_SO	SO_specular( NORMALVAL N, VECTORVAL V, FLOATVAL roughness, DEFPARAM ) = 0;
	virtual STD_SO	SO_phong( NORMALVAL N, VECTORVAL V, FLOATVAL size, DEFPARAM ) = 0;
	virtual STD_SO	SO_trace( POINTVAL P, VECTORVAL R, DEFPARAM ) = 0;
	virtual STD_SO	SO_ftexture1( STRINGVAL name, FLOATVAL channel, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_ftexture2( STRINGVAL name, FLOATVAL channel, FLOATVAL s, FLOATVAL t, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_ftexture3( STRINGVAL name, FLOATVAL channel, FLOATVAL s1, FLOATVAL t1, FLOATVAL s2, FLOATVAL t2, FLOATVAL s3, FLOATVAL t3, FLOATVAL s4, FLOATVAL t4, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_ctexture1( STRINGVAL name, FLOATVAL channel, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_ctexture2( STRINGVAL name, FLOATVAL channel, FLOATVAL s, FLOATVAL t, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_ctexture3( STRINGVAL name, FLOATVAL channel, FLOATVAL s1, FLOATVAL t1, FLOATVAL s2, FLOATVAL t2, FLOATVAL s3, FLOATVAL t3, FLOATVAL s4, FLOATVAL t4, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_fenvironment2( STRINGVAL name, FLOATVAL channel, VECTORVAL R, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_fenvironment3( STRINGVAL name, FLOATVAL channel, VECTORVAL R1, VECTORVAL R2, VECTORVAL R3, VECTORVAL R4, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_cenvironment2( STRINGVAL name, FLOATVAL channel, VECTORVAL R, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_cenvironment3( STRINGVAL name, FLOATVAL channel, VECTORVAL R1, VECTORVAL R2, VECTORVAL R3, VECTORVAL R4, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_bump1( STRINGVAL name, FLOATVAL channel, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_bump2( STRINGVAL name, FLOATVAL channel, FLOATVAL s, FLOATVAL t, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_bump3( STRINGVAL name, FLOATVAL channel, FLOATVAL s1, FLOATVAL t1, FLOATVAL s2, FLOATVAL t2, FLOATVAL s3, FLOATVAL t3, FLOATVAL s4, FLOATVAL t4, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_shadow( STRINGVAL name, FLOATVAL channel, POINTVAL P, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_shadow1( STRINGVAL name, FLOATVAL channel, POINTVAL P1, POINTVAL P2, POINTVAL P3, POINTVAL P4, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_illuminance( POINTVAL P, FLOATVAL nsamples, DEFVOIDPARAM ) = 0;
	virtual STD_SO	SO_illuminance( POINTVAL P, VECTORVAL Axis, FLOATVAL Angle, FLOATVAL nsamples, DEFVOIDPARAM ) = 0;
	virtual STD_SO	SO_illuminate( POINTVAL P, VECTORVAL Axis, FLOATVAL Angle, DEFVOIDPARAM ) = 0;
	virtual STD_SO	SO_illuminate( POINTVAL P, DEFVOIDPARAM ) = 0;
	virtual STD_SO	SO_solar( VECTORVAL Axis, FLOATVAL Angle, DEFVOIDPARAM ) = 0;
	virtual STD_SO	SO_solar( DEFVOIDPARAM ) = 0;
	virtual STD_SO	SO_gather( STRINGVAL category, POINTVAL P, VECTORVAL dir, FLOATVAL angle, FLOATVAL nsamples, DEFVOIDPARAMVAR ) = 0;
	virtual STD_SO	SO_printf( STRINGVAL str, DEFVOIDPARAMVAR ) = 0;
	virtual STD_SO	SO_format( STRINGVAL str, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_concat( STRINGVAL stra, STRINGVAL strb, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_atmosphere( STRINGVAL name, IqShaderData* pV, DEFPARAM ) = 0;
	virtual STD_SO	SO_displacement( STRINGVAL name, IqShaderData* pV, DEFPARAM ) = 0;
	virtual STD_SO	SO_lightsource( STRINGVAL name, IqShaderData* pV, DEFPARAM ) = 0;
	virtual STD_SO	SO_surface( STRINGVAL name, IqShaderData* pV, DEFPARAM ) = 0;
	virtual STD_SO	SO_attribute( STRINGVAL name, IqShaderData* pV, DEFPARAM ) = 0;
	virtual STD_SO	SO_option( STRINGVAL name, IqShaderData* pV, DEFPARAM ) = 0;
	virtual STD_SO	SO_rendererinfo( STRINGVAL name, IqShaderData* pV, DEFPARAM ) = 0;
	virtual STD_SO	SO_incident( STRINGVAL name, IqShaderData* pV, DEFPARAM ) = 0;
	virtual STD_SO	SO_opposite( STRINGVAL name, IqShaderData* pV, DEFPARAM ) = 0;
	virtual STD_SO	SO_fcellnoise1( FLOATVAL v, DEFPARAM ) = 0;
	virtual STD_SO	SO_fcellnoise2( FLOATVAL u, FLOATVAL v, DEFPARAM ) = 0;
	virtual STD_SO	SO_fcellnoise3( POINTVAL p, DEFPARAM ) = 0;
	virtual STD_SO	SO_fcellnoise4( POINTVAL p, FLOATVAL v, DEFPARAM ) = 0;
	virtual STD_SO	SO_ccellnoise1( FLOATVAL v, DEFPARAM ) = 0;
	virtual STD_SO	SO_ccellnoise2( FLOATVAL u, FLOATVAL v, DEFPARAM ) = 0;
	virtual STD_SO	SO_ccellnoise3( POINTVAL p, DEFPARAM ) = 0;
	virtual STD_SO	SO_ccellnoise4( POINTVAL p, FLOATVAL v, DEFPARAM ) = 0;
	virtual STD_SO	SO_pcellnoise1( FLOATVAL v, DEFPARAM ) = 0;
	virtual STD_SO	SO_pcellnoise2( FLOATVAL u, FLOATVAL v, DEFPARAM ) = 0;
	virtual STD_SO	SO_pcellnoise3( POINTVAL p, DEFPARAM ) = 0;
	virtual STD_SO	SO_pcellnoise4( POINTVAL p, FLOATVAL v, DEFPARAM ) = 0;
	virtual STD_SO	SO_fpnoise1( FLOATVAL v, FLOATVAL period, DEFPARAM ) = 0;
	virtual STD_SO	SO_fpnoise2( FLOATVAL u, FLOATVAL v, FLOATVAL uperiod, FLOATVAL vperiod, DEFPARAM ) = 0;
	virtual STD_SO	SO_fpnoise3( POINTVAL p, POINTVAL pperiod, DEFPARAM ) = 0;
	virtual STD_SO	SO_fpnoise4( POINTVAL p, FLOATVAL t, POINTVAL pperiod, FLOATVAL tperiod, DEFPARAM ) = 0;
	virtual STD_SO	SO_cpnoise1( FLOATVAL v, FLOATVAL period, DEFPARAM ) = 0;
	virtual STD_SO	SO_cpnoise2( FLOATVAL u, FLOATVAL v, FLOATVAL uperiod, FLOATVAL vperiod, DEFPARAM ) = 0;
	virtual STD_SO	SO_cpnoise3( POINTVAL p, POINTVAL pperiod, DEFPARAM ) = 0;
	virtual STD_SO	SO_cpnoise4( POINTVAL p, FLOATVAL t, POINTVAL pperiod, FLOATVAL tperiod, DEFPARAM ) = 0;
	virtual STD_SO	SO_ppnoise1( FLOATVAL v, FLOATVAL period, DEFPARAM ) = 0;
	virtual STD_SO	SO_ppnoise2( FLOATVAL u, FLOATVAL v, FLOATVAL uperiod, FLOATVAL vperiod, DEFPARAM ) = 0;
	virtual STD_SO	SO_ppnoise3( POINTVAL p, POINTVAL pperiod, DEFPARAM ) = 0;
	virtual STD_SO	SO_ppnoise4( POINTVAL p, FLOATVAL t, POINTVAL pperiod, FLOATVAL tperiod, DEFPARAM ) = 0;
	virtual STD_SO	SO_ctransform( STRINGVAL fromspace, STRINGVAL tospace, COLORVAL c, DEFPARAM ) = 0;
	virtual STD_SO	SO_ctransform( STRINGVAL tospace, COLORVAL c, DEFPARAM ) = 0;
	virtual STD_SO	SO_ptlined( POINTVAL P0, POINTVAL P1, POINTVAL Q, DEFPARAM ) = 0;
	virtual STD_SO	SO_inversesqrt( FLOATVAL x, DEFPARAM ) = 0;
	virtual STD_SO	SO_match( STRINGVAL a, STRINGVAL b, DEFPARAM ) = 0;
	virtual STD_SO	SO_rotate( VECTORVAL Q, FLOATVAL angle, POINTVAL P0, POINTVAL P1, DEFPARAM ) = 0;
	virtual STD_SO	SO_filterstep( FLOATVAL edge, FLOATVAL s1, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_filterstep2( FLOATVAL edge, FLOATVAL s1, FLOATVAL s2, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_specularbrdf( VECTORVAL L, NORMALVAL N, VECTORVAL V, FLOATVAL rough, DEFPARAM ) = 0;
	virtual STD_SO	SO_mtransform( STRINGVAL fromspace, STRINGVAL tospace, MATRIXVAL m, DEFPARAM ) = 0;
	virtual STD_SO	SO_mtransform( STRINGVAL tospace, MATRIXVAL m, DEFPARAM ) = 0;
	virtual STD_SO	SO_setmcomp( MATRIXVAL M, FLOATVAL row, FLOATVAL column, FLOATVAL val, DEFVOIDPARAM ) = 0;
	virtual STD_SO	SO_determinant( MATRIXVAL M, DEFPARAM ) = 0;
	virtual STD_SO	SO_mtranslate( MATRIXVAL M, VECTORVAL V, DEFPARAM ) = 0;
	virtual STD_SO	SO_mrotate( MATRIXVAL M, FLOATVAL angle, VECTORVAL axis, DEFPARAM ) = 0;
	virtual STD_SO	SO_mscale( MATRIXVAL M, POINTVAL s, DEFPARAM ) = 0;
	virtual STD_SO	SO_fsplinea( FLOATVAL value, FLOATARRAYVAL a, DEFPARAM ) = 0;
	virtual STD_SO	SO_csplinea( FLOATVAL value, COLORARRAYVAL a, DEFPARAM ) = 0;
	virtual STD_SO	SO_psplinea( FLOATVAL value, POINTARRAYVAL a, DEFPARAM ) = 0;
	virtual STD_SO	SO_sfsplinea( STRINGVAL basis, FLOATVAL value, FLOATARRAYVAL a, DEFPARAM ) = 0;
	virtual STD_SO	SO_scsplinea( STRINGVAL basis, FLOATVAL value, COLORARRAYVAL a, DEFPARAM ) = 0;
	virtual STD_SO	SO_spsplinea( STRINGVAL basis, FLOATVAL value, POINTARRAYVAL a, DEFPARAM ) = 0;
	virtual STD_SO	SO_shadername( DEFPARAM ) = 0;
	virtual STD_SO	SO_shadername2( STRINGVAL shader, DEFPARAM ) = 0;
	virtual STD_SO	SO_textureinfo( STRINGVAL shader, STRINGVAL dataname, IqShaderData* pV, DEFPARAM ) = 0;
	virtual STD_SO	SO_bake_f( STRINGVAL shader, FLOATVAL s, FLOATVAL t, FLOATVAL f, DEFVOIDPARAMVAR) = 0;
	virtual STD_SO	SO_bake_3c( STRINGVAL shader, FLOATVAL s, FLOATVAL t, FLOATVAL f, DEFVOIDPARAMVAR) = 0;
	virtual STD_SO	SO_bake_3p( STRINGVAL shader, FLOATVAL s, FLOATVAL t, FLOATVAL f, DEFVOIDPARAMVAR) = 0;
	virtual STD_SO	SO_bake_3n( STRINGVAL shader, FLOATVAL s, FLOATVAL t, FLOATVAL f, DEFVOIDPARAMVAR) = 0;
	virtual STD_SO	SO_bake_3v( STRINGVAL shader, FLOATVAL s, FLOATVAL t, FLOATVAL f, DEFVOIDPARAMVAR) = 0;
	virtual STD_SO	SO_external(DSOMethod method, void *initData, DEFPARAMVAR) = 0;
	virtual STD_SO	SO_occlusion( STRINGVAL occlmap, FLOATVAL channel, POINTVAL P, NORMALVAL N, FLOATVAL samples, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_occlusion_rt( POINTVAL P, NORMALVAL N, FLOATVAL samples, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_rayinfo( STRINGVAL dataname, IqShaderData* pV, DEFPARAM ) = 0;
	virtual STD_SO	SO_bake3d( STRINGVAL ptc, STRINGVAL format, POINTVAL P, NORMALVAL N, DEFPARAMVAR ) = 0;
	virtual STD_SO	SO_texture3d( STRINGVAL ptc, POINTVAL P, NORMALVAL N, DEFPARAMVAR ) = 0;
};

//-----------------------------------------------------------------------

} // namespace Aqsis


#endif // ISHADEREXECENV_H_INCLUDED
