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
		\brief Declares classes and support structures for the shader execution environment.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef SHADEREXECENV_H_INCLUDED
#define SHADEREXECENV_H_INCLUDED 1

#include	<aqsis/aqsis.h>

#include	<vector>

#include <boost/noncopyable.hpp>

#include	<aqsis/math/math.h>
#include	<aqsis/util/bitvector.h>
#include	<aqsis/math/color.h>
#include	<aqsis/math/noise.h>
#include	<aqsis/math/random.h>
#include	<aqsis/math/cellnoise.h>
#include	<aqsis/math/vector3d.h>
#include	<aqsis/shadervm/ishaderdata.h>
#include	<aqsis/shadervm/ishader.h>
#include	<aqsis/shadervm/ishaderexecenv.h>
#include	<aqsis/core/isurface.h>
#include	<aqsis/core/irenderer.h>
#include	<aqsis/math/matrix.h>
#include	<aqsis/math/derivatives.h>

#include	<aqsis/core/iattributes.h>
#include	<aqsis/core/itransform.h>

namespace Aqsis {

//----------------------------------------------------------------------
/** \class CqShaderExecEnv
 * Standard shader execution environment. Contains standard variables, and provides SIMD functionality.
 */
#ifdef AQSIS_SYSTEM_WIN32
class AQSIS_SHADERVM_SHARE boost::noncopyable_::noncopyable;
#endif
class AQSIS_SHADERVM_SHARE CqShaderExecEnv : public IqShaderExecEnv, boost::noncopyable
{
	public:
		CqShaderExecEnv(IqRenderer* pRenderContext);
		virtual	~CqShaderExecEnv();

		// Overidden from IqShaderExecEnv, see ishaderexecenv.h for descriptions.
		virtual	void	Initialise( const TqInt uGridRes, const TqInt vGridRes, 
			TqInt microPolygonCount, TqInt shadingPointCount, 
			bool hasValidDerivatives,
			const IqConstAttributesPtr& pAttr, 
			const IqConstTransformPtr& pTrans, 
			IqShader* pShader, 
			TqInt Uses );
		virtual	TqInt	uGridRes() const
		{
			return ( m_uGridRes );
		}
		virtual	TqInt	vGridRes() const
		{
			return ( m_vGridRes );
		}
		virtual	TqUint	microPolygonCount() const
		{
			return ( m_microPolygonCount );
		}
		virtual	TqUint	shadingPointCount() const
		{
			return ( m_shadingPointCount );
		}
		virtual	const CqMatrix&	matObjectToWorld() const;
		const IqConstAttributesPtr	pAttributes() const
		{
			return ( m_pAttributes );
		}
		const IqConstTransformPtr	pTransform() const
		{
			return ( boost::static_pointer_cast<const IqTransform>(m_pTransform) );
		}
		virtual CqGridDiff GridDiff() const
		{
			return m_diff;
		}
		virtual void SetCurrentSurface(IqSurface* pEnv)
		{
			m_pCurrentSurface = pEnv;
		}
		virtual const IqSurface* GetCurrentSurface() const
		{
			return(m_pCurrentSurface);
		}
		virtual	void	ValidateIlluminanceCache( IqShaderData* pP, IqShaderData* pN, IqShader* pShader );
		virtual	void	InvalidateIlluminanceCache()
		{
			m_IlluminanceCacheValid = false;
		}
		virtual	CqBitVector& CurrentState()
		{
			return ( m_CurrentState );
		}
		virtual	const CqBitVector& RunningState() const
		{
			return ( m_RunningState );
		}
		virtual	void	GetCurrentState()
		{
			m_RunningState = m_CurrentState;
			m_isRunning = m_RunningState.Count() != 0;
		}
		virtual	void	ClearCurrentState()
		{
			m_CurrentState.SetAll( false );
		}
		virtual	void	PushState()
		{
			m_stkState.push_back( m_RunningState );
		}
		virtual	void	PopState()
		{
			m_RunningState = m_stkState.back();
			m_stkState.pop_back();
			m_isRunning = m_RunningState.Count() != 0;
		}
		virtual	void	InvertRunningState()
		{
			m_RunningState.Complement();
			if ( !m_stkState.empty() )
				m_RunningState.Intersect( m_stkState.back() );
			m_isRunning = m_RunningState.Count() != 0;
		}
		virtual void RunningStatesBreak(TqInt numLevels)
		{
			assert(numLevels >= 0);
			assert(numLevels <= static_cast<TqInt>(1+m_stkState.size()));
			m_RunningState.Complement();
			for(std::vector<CqBitVector>::reverse_iterator i = m_stkState.rbegin(),
					end = m_stkState.rbegin()+numLevels; i != end; ++i)
			{
				// Cause all running states down the stack for numLevels
				// positions to stop running for those elements of the current
				// state which are running.
				i->Intersect(m_RunningState);
			}
			// Current state needs to stop executing.
			m_RunningState.SetAll(false);
			m_isRunning = false;
		}
		virtual bool IsRunning()
		{
			return m_isRunning;
		}
		virtual IqShaderData* FindStandardVar( const char* pname );

		virtual	TqInt	FindStandardVarIndex( const char* pname );

		virtual IqShaderData*	pVar( TqInt Index )
		{
			return ( m_apVariables[ Index ] );
		}
		virtual	void	DeleteVariable( TqInt Index )
		{
			delete( m_apVariables[ Index ] );
			m_apVariables[ Index ] = 0;
		}
		virtual IqShaderData* Cs()
		{
			return ( m_apVariables[ EnvVars_Cs ] );
		}
		virtual IqShaderData* Os()
		{
			return ( m_apVariables[ EnvVars_Os ] );
		}
		virtual IqShaderData* Ng()
		{
			return ( m_apVariables[ EnvVars_Ng ] );
		}
		virtual IqShaderData* du()
		{
			return ( m_apVariables[ EnvVars_du ] );
		}
		virtual IqShaderData* dv()
		{
			return ( m_apVariables[ EnvVars_dv ] );
		}
		virtual IqShaderData* L()
		{
			return ( m_apVariables[ EnvVars_L ] );
		}
		virtual IqShaderData* Cl()
		{
			return ( m_apVariables[ EnvVars_Cl ] );
		}
		virtual IqShaderData* Ol()
		{
			return ( m_apVariables[ EnvVars_Ol ] );
		}
		virtual IqShaderData* P()
		{
			return ( m_apVariables[ EnvVars_P ] );
		}
		virtual IqShaderData* dPdu()
		{
			return ( m_apVariables[ EnvVars_dPdu ] );
		}
		virtual IqShaderData* dPdv()
		{
			return ( m_apVariables[ EnvVars_dPdv ] );
		}
		virtual IqShaderData* N()
		{
			return ( m_apVariables[ EnvVars_N ] );
		}
		virtual IqShaderData* u()
		{
			return ( m_apVariables[ EnvVars_u ] );
		}
		virtual IqShaderData* v()
		{
			return ( m_apVariables[ EnvVars_v ] );
		}
		virtual IqShaderData* s()
		{
			return ( m_apVariables[ EnvVars_s ] );
		}
		virtual IqShaderData* t()
		{
			return ( m_apVariables[ EnvVars_t ] );
		}
		virtual IqShaderData* I()
		{
			return ( m_apVariables[ EnvVars_I ] );
		}
		virtual IqShaderData* Ci()
		{
			return ( m_apVariables[ EnvVars_Ci ] );
		}
		virtual IqShaderData* Oi()
		{
			return ( m_apVariables[ EnvVars_Oi ] );
		}
		virtual IqShaderData* Ps()
		{
			return ( m_apVariables[ EnvVars_Ps ] );
		}
		virtual IqShaderData* E()
		{
			return ( m_apVariables[ EnvVars_E ] );
		}
		virtual IqShaderData* ncomps()
		{
			return ( m_apVariables[ EnvVars_ncomps ] );
		}
		virtual IqShaderData* time()
		{
			return ( m_apVariables[ EnvVars_time ] );
		}
		virtual IqShaderData* alpha()
		{
			return ( m_apVariables[ EnvVars_alpha ] );
		}
		virtual IqShaderData* Ns()
		{
			return ( m_apVariables[ EnvVars_Ns ] );
		}
		virtual IqRenderer* getRenderContext() const
		{
			return ( m_pRenderContext );
		}

	private:
		/** \brief Evaluate discrete difference of a shader variable in the u-direction
		 *
		 * This is the discrete analogue to differentiation: for a 1D grid, "Y",
		 * the discrete first order difference is conceptually just the
		 * difference between consecutive grid points:
		 *
		 *   diff(Y, i) = Y[i+1] - Y[i];
		 *
		 * We choose the asymmetric forward difference here instead of the
		 * second order centred difference [ diff(Y, i) = (Y[i+1] - Y[i-1])/2 ]
		 * since the forward difference preserves more surface detail.
		 *
		 * In practise a mixture of difference schemes are necessary to work
		 * with grid boundaries, but the normalization is consistent with the
		 * forward difference.
		 *
		 * \param var - variable to take the difference of.
		 * \param gridIdx - 1D index into the 2D grid of data.
		 */
		template<typename T>
		T diffU(IqShaderData* var, TqInt gridIdx);
		/** \brief Evaluate discrete difference of a shader variable in the v-direction
		 *
		 * This is the discrete analogue to differentiation
		 * \see diffU for more details.
		 *
		 * \param var - variable to take the difference of.
		 * \param gridIdx - 1D index into the 2D grid of data.
		 */
		template<typename T>
		T diffV(IqShaderData* var, TqInt gridIdx);
		/** \brief Evaluate the partial derivative of a shader var with respect to u.
		 *
		 * This is just diffU(var, gridIdx)/du(gridIdx) with some checking for
		 * the case when du == 0.
		 *
		 * \note In many cases it is more appropriate to use the discrete
		 * analogue, diffU() rather than this function.
		 *
		 * \param var - variable to take the derivative of
		 * \param gridIdx - 1D index into the var grid at which to compute the
		 *                  derivative.
		 * \param undefVal - value to be returned when the result is undefined
		 *                   (ie, when du = 0).
		 *
		 * \return d(var)/du at the index gridIdx
		 */
		template<typename T>
		inline T derivU(IqShaderData* var, TqInt gridIdx, const T& undefVal = T());
		/** \brief Evaluate the partial derivative of a shader var with respect to v.
		 *
		 * This is just diffV(var, gridIdx)/dv(gridIdx) with some checking for
		 * the case when dv == 0.
		 *
		 * \note In many cases it is more appropriate to use the discrete
		 * analogue, diffU() rather than this function.
		 *
		 * \param var - variable to take the derivative of
		 * \param gridIdx - 1D index into the var grid at which to compute the
		 *                  derivative.
		 * \param undefVal - value to be returned when the result is undefined
		 *                   (ie, when dv = 0).
		 *
		 * \return d(var)/dv at the index gridIdx
		 */
		template<typename T>
		inline T derivV(IqShaderData* var, TqInt gridIdx, const T& undefVal = T());
		/** \brief Compute the differential dy/dx.
		 *
		 * Computes the derivative of a shader variable of type T with respect
		 * to a given float variable.
		 *
		 * \param y - a variable of type T.
		 * \param x - a float variable.
		 * \param gridIdx - 1D index into the 2D grids of data.
		 *
		 * \return dy/dx if dx is nonzero.  If dx == 0, return a default
		 * constructed value for T.
		 */
		template<typename T>
		T deriv(IqShaderData* y, IqShaderData* x, TqInt gridIdx);


		std::vector<IqShaderData*>	m_apVariables;	///< Vector of pointers to shader variables.
		struct SqVarName
		{
			char*	m_strName;
			EqEnvVars	m_Index;
		};
		static	CqNoise	m_noise;		///< One off noise generator, used by all envs.
		static	CqCellNoise	m_cellnoise;	///< One off cell noise generator, used by all envs.
		static	CqRandom	m_random;		///< One off random number generator used by all envs.
		static	CqMatrix	m_matIdentity;

		TqInt	m_uGridRes;				///< The resolution of the grid in u.
		TqInt	m_vGridRes;				///< The resolution of the grid in u.
		TqInt	m_microPolygonCount;			///< The resolution of the grid.
		TqInt	m_shadingPointCount;			///< The resolution of the grid.
		TqUint	m_li;					///< Light index, used during illuminance loop.
		TqInt	m_Illuminate;
		bool	m_IlluminanceCacheValid;	///< Flag indicating whether the illuminance cache is valid.
		TqUint	m_gatherSample;				///< Sample index, used during gather loop.
		IqConstAttributesPtr m_pAttributes;	///< Pointer to the associated attributes.
		IqConstTransformPtr m_pTransform;		///< Pointer to the associated transform.
		CqBitVector	m_CurrentState;			///< SIMD execution state bit vector accumulator.
		CqBitVector	m_RunningState;			///< SIMD running execution state bit vector.
		bool m_isRunning;               ///< True if any bits in the running state are set.
		std::vector<CqBitVector>	m_stkState;				///< Stack of execution state bit vectors.
		IqRenderer*	m_pRenderContext;
		TqInt	m_LocalIndex;			///< Local cached variable index to speed repeated access to the same local variable.
		IqSurface*	m_pCurrentSurface;	///< Pointer to the surface being shaded.
		bool	m_hasValidDerivatives;	///< Is this shading collection able to provide valid derivatives. RiPoints, can't.
		std::vector<TqInt>	m_diffUidx;	///< Precomputed derivative index for the left hand side of the difference calculation.
		std::vector<TqInt>	m_diffVidx;	///< Precomputed derivative index for the right hand side of the difference calculation.

		CqGridDiff m_diff;

	public:

		virtual	bool	SO_init_illuminance();
		virtual	bool	SO_advance_illuminance();

		virtual	STD_SO	SO_init_gather( FLOATVAL samples, DEFVOIDPARAM );
		virtual	bool	SO_advance_gather();

		// ShadeOps
		virtual STD_SO	SO_radians( FLOATVAL degrees, DEFPARAM );
		virtual STD_SO	SO_degrees( FLOATVAL radians, DEFPARAM );
		virtual STD_SO	SO_sin( FLOATVAL a, DEFPARAM );
		virtual STD_SO	SO_asin( FLOATVAL a, DEFPARAM );
		virtual STD_SO	SO_cos( FLOATVAL a, DEFPARAM );
		virtual STD_SO	SO_acos( FLOATVAL a, DEFPARAM );
		virtual STD_SO	SO_tan( FLOATVAL a, DEFPARAM );
		virtual STD_SO	SO_atan( FLOATVAL yoverx, DEFPARAM );
		virtual STD_SO	SO_atan( FLOATVAL y, FLOATVAL x, DEFPARAM );
		virtual STD_SO	SO_pow( FLOATVAL x, FLOATVAL y, DEFPARAM );
		virtual STD_SO	SO_exp( FLOATVAL x, DEFPARAM );
		virtual STD_SO	SO_sqrt( FLOATVAL x, DEFPARAM );
		virtual STD_SO	SO_log( FLOATVAL x, DEFPARAM );
		virtual STD_SO	SO_log( FLOATVAL x, FLOATVAL base, DEFPARAM );
		virtual STD_SO	SO_mod( FLOATVAL a, FLOATVAL b, DEFPARAM );
		virtual STD_SO	SO_abs( FLOATVAL x, DEFPARAM );
		virtual STD_SO	SO_sign( FLOATVAL x, DEFPARAM );
		virtual STD_SO	SO_min( FLOATVAL a, FLOATVAL b, DEFPARAMVAR );
		virtual STD_SO	SO_max( FLOATVAL a, FLOATVAL b, DEFPARAMVAR );
		virtual STD_SO	SO_pmin( POINTVAL a, POINTVAL b, DEFPARAMVAR );
		virtual STD_SO	SO_pmax( POINTVAL a, POINTVAL b, DEFPARAMVAR );
		virtual STD_SO	SO_cmin( COLORVAL a, COLORVAL b, DEFPARAMVAR );
		virtual STD_SO	SO_cmax( COLORVAL a, COLORVAL b, DEFPARAMVAR );
		virtual STD_SO	SO_clamp( FLOATVAL a, FLOATVAL _min, FLOATVAL _max, DEFPARAM );
		virtual STD_SO	SO_pclamp( POINTVAL a, POINTVAL _min, POINTVAL _max, DEFPARAM );
		virtual STD_SO	SO_cclamp( COLORVAL a, COLORVAL _min, COLORVAL _max, DEFPARAM );
		virtual STD_SO	SO_floor( FLOATVAL x, DEFPARAM );
		virtual STD_SO	SO_ceil( FLOATVAL x, DEFPARAM );
		virtual STD_SO	SO_round( FLOATVAL x, DEFPARAM );
		virtual STD_SO	SO_step( FLOATVAL _min, FLOATVAL value, DEFPARAM );
		virtual STD_SO	SO_smoothstep( FLOATVAL _min, FLOATVAL _max, FLOATVAL value, DEFPARAM );
		virtual STD_SO	SO_fspline( FLOATVAL value, DEFPARAMVAR );
		virtual STD_SO	SO_cspline( FLOATVAL value, DEFPARAMVAR );
		virtual STD_SO	SO_pspline( FLOATVAL value, DEFPARAMVAR );
		virtual STD_SO	SO_sfspline( STRINGVAL basis, FLOATVAL value, DEFPARAMVAR );
		virtual STD_SO	SO_scspline( STRINGVAL basis, FLOATVAL value, DEFPARAMVAR );
		virtual STD_SO	SO_spspline( STRINGVAL basis, FLOATVAL value, DEFPARAMVAR );
		virtual STD_SO	SO_fDu( FLOATVAL p, DEFPARAM );
		virtual STD_SO	SO_fDv( FLOATVAL p, DEFPARAM );
		virtual STD_SO	SO_fDeriv( FLOATVAL p, FLOATVAL den, DEFPARAM );
		virtual STD_SO	SO_cDu( COLORVAL p, DEFPARAM );
		virtual STD_SO	SO_cDv( COLORVAL p, DEFPARAM );
		virtual STD_SO	SO_cDeriv( COLORVAL p, FLOATVAL den, DEFPARAM );
		virtual STD_SO	SO_pDu( POINTVAL p, DEFPARAM );
		virtual STD_SO	SO_pDv( POINTVAL p, DEFPARAM );
		virtual STD_SO	SO_pDeriv( POINTVAL p, FLOATVAL den, DEFPARAM );
		virtual STD_SO	SO_frandom( DEFPARAM );
		virtual STD_SO	SO_crandom( DEFPARAM );
		virtual STD_SO	SO_prandom( DEFPARAM );
		virtual STD_SO	SO_fnoise1( FLOATVAL v, DEFPARAM );
		virtual STD_SO	SO_fnoise2( FLOATVAL u, FLOATVAL v, DEFPARAM );
		virtual STD_SO	SO_fnoise3( POINTVAL p, DEFPARAM );
		virtual STD_SO	SO_fnoise4( POINTVAL p, FLOATVAL t, DEFPARAM );
		virtual STD_SO	SO_cnoise1( FLOATVAL v, DEFPARAM );
		virtual STD_SO	SO_cnoise2( FLOATVAL u, FLOATVAL v, DEFPARAM );
		virtual STD_SO	SO_cnoise3( POINTVAL p, DEFPARAM );
		virtual STD_SO	SO_cnoise4( POINTVAL p, FLOATVAL t, DEFPARAM );
		virtual STD_SO	SO_pnoise1( FLOATVAL v, DEFPARAM );
		virtual STD_SO	SO_pnoise2( FLOATVAL u, FLOATVAL v, DEFPARAM );
		virtual STD_SO	SO_pnoise3( POINTVAL p, DEFPARAM );
		virtual STD_SO	SO_pnoise4( POINTVAL p, FLOATVAL t, DEFPARAM );
		virtual STD_SO	SO_setcomp( COLORVAL p, FLOATVAL i, FLOATVAL v, DEFVOIDPARAM );
		virtual STD_SO	SO_setxcomp( POINTVAL p, FLOATVAL v, DEFVOIDPARAM );
		virtual STD_SO	SO_setycomp( POINTVAL p, FLOATVAL v, DEFVOIDPARAM );
		virtual STD_SO	SO_setzcomp( POINTVAL p, FLOATVAL v, DEFVOIDPARAM );
		virtual STD_SO	SO_length( VECTORVAL V, DEFPARAM );
		virtual STD_SO	SO_distance( POINTVAL P1, POINTVAL P2, DEFPARAM );
		virtual STD_SO	SO_area( POINTVAL p, DEFPARAM );
		virtual STD_SO	SO_normalize( VECTORVAL V, DEFPARAM );
		virtual STD_SO	SO_faceforward( NORMALVAL N, VECTORVAL I, DEFPARAM );
		virtual STD_SO	SO_faceforward2( NORMALVAL N, VECTORVAL I, NORMALVAL Nref, DEFPARAM );
		virtual STD_SO	SO_reflect( VECTORVAL I, NORMALVAL N, DEFPARAM );
		virtual STD_SO	SO_refract( VECTORVAL I, NORMALVAL N, FLOATVAL eta, DEFPARAM );
		virtual STD_SO	SO_fresnel( VECTORVAL I, NORMALVAL N, FLOATVAL eta, FLOATVAL Kr, FLOATVAL Kt, DEFVOIDPARAM );
		virtual STD_SO	SO_fresnel( VECTORVAL I, NORMALVAL N, FLOATVAL eta, FLOATVAL Kr, FLOATVAL Kt, VECTORVAL R, VECTORVAL T, DEFVOIDPARAM );
		virtual STD_SO	SO_transform( STRINGVAL fromspace, STRINGVAL tospace, POINTVAL p, DEFPARAM );
		virtual STD_SO	SO_transform( STRINGVAL tospace, POINTVAL p, DEFPARAM );
		virtual STD_SO	SO_transformm( MATRIXVAL tospace, POINTVAL p, DEFPARAM );
		virtual STD_SO	SO_vtransform( STRINGVAL fromspace, STRINGVAL tospace, VECTORVAL p, DEFPARAM );
		virtual STD_SO	SO_vtransform( STRINGVAL tospace, VECTORVAL p, DEFPARAM );
		virtual STD_SO	SO_vtransformm( MATRIXVAL tospace, VECTORVAL p, DEFPARAM );
		virtual STD_SO	SO_ntransform( STRINGVAL fromspace, STRINGVAL tospace, NORMALVAL p, DEFPARAM );
		virtual STD_SO	SO_ntransform( STRINGVAL tospace, NORMALVAL p, DEFPARAM );
		virtual STD_SO	SO_ntransformm( MATRIXVAL tospace, NORMALVAL p, DEFPARAM );
		virtual STD_SO	SO_depth( POINTVAL p, DEFPARAM );
		virtual STD_SO	SO_calculatenormal( POINTVAL p, DEFPARAM );
		virtual STD_SO	SO_cmix( COLORVAL color0, COLORVAL color1, FLOATVAL value, DEFPARAM );
		virtual STD_SO	SO_fmix( FLOATVAL f0, FLOATVAL f1, FLOATVAL value, DEFPARAM );
		virtual STD_SO	SO_pmix( POINTVAL p0, POINTVAL p1, FLOATVAL value, DEFPARAM );
		virtual STD_SO	SO_vmix( VECTORVAL v0, VECTORVAL v1, FLOATVAL value, DEFPARAM );
		virtual STD_SO	SO_nmix( NORMALVAL n0, NORMALVAL n1, FLOATVAL value, DEFPARAM );
		virtual STD_SO	SO_cmixc( COLORVAL color0, COLORVAL color1, COLORVAL value, DEFPARAM );
		virtual STD_SO	SO_pmixc( POINTVAL p0, POINTVAL p1, COLORVAL value, DEFPARAM );
		virtual STD_SO	SO_vmixc( VECTORVAL v0, VECTORVAL v1, COLORVAL value, DEFPARAM );
		virtual STD_SO	SO_nmixc( NORMALVAL n0, NORMALVAL n1, COLORVAL value, DEFPARAM );
		virtual STD_SO	SO_ambient( DEFPARAM );
		virtual STD_SO	SO_diffuse( NORMALVAL N, DEFPARAM );
		virtual STD_SO	SO_specular( NORMALVAL N, VECTORVAL V, FLOATVAL roughness, DEFPARAM );
		virtual STD_SO	SO_phong( NORMALVAL N, VECTORVAL V, FLOATVAL size, DEFPARAM );
		virtual STD_SO	SO_trace( POINTVAL P, VECTORVAL R, DEFPARAM );
		virtual STD_SO	SO_ftexture1( STRINGVAL name, FLOATVAL channel, DEFPARAMVAR );
		virtual STD_SO	SO_ftexture2( STRINGVAL name, FLOATVAL channel, FLOATVAL s, FLOATVAL t, DEFPARAMVAR );
		virtual STD_SO	SO_ftexture3( STRINGVAL name, FLOATVAL channel, FLOATVAL s1, FLOATVAL t1, FLOATVAL s2, FLOATVAL t2, FLOATVAL s3, FLOATVAL t3, FLOATVAL s4, FLOATVAL t4, DEFPARAMVAR );
		virtual STD_SO	SO_ctexture1( STRINGVAL name, FLOATVAL channel, DEFPARAMVAR );
		virtual STD_SO	SO_ctexture2( STRINGVAL name, FLOATVAL channel, FLOATVAL s, FLOATVAL t, DEFPARAMVAR );
		virtual STD_SO	SO_ctexture3( STRINGVAL name, FLOATVAL channel, FLOATVAL s1, FLOATVAL t1, FLOATVAL s2, FLOATVAL t2, FLOATVAL s3, FLOATVAL t3, FLOATVAL s4, FLOATVAL t4, DEFPARAMVAR );
		virtual STD_SO	SO_fenvironment2( STRINGVAL name, FLOATVAL channel, VECTORVAL R, DEFPARAMVAR );
		virtual STD_SO	SO_fenvironment3( STRINGVAL name, FLOATVAL channel, VECTORVAL R1, VECTORVAL R2, VECTORVAL R3, VECTORVAL R4, DEFPARAMVAR );
		virtual STD_SO	SO_cenvironment2( STRINGVAL name, FLOATVAL channel, VECTORVAL R, DEFPARAMVAR );
		virtual STD_SO	SO_cenvironment3( STRINGVAL name, FLOATVAL channel, VECTORVAL R1, VECTORVAL R2, VECTORVAL R3, VECTORVAL R4, DEFPARAMVAR );
		virtual STD_SO	SO_bump1( STRINGVAL name, FLOATVAL channel, DEFPARAMVAR );
		virtual STD_SO	SO_bump2( STRINGVAL name, FLOATVAL channel, FLOATVAL s, FLOATVAL t, DEFPARAMVAR );
		virtual STD_SO	SO_bump3( STRINGVAL name, FLOATVAL channel, FLOATVAL s1, FLOATVAL t1, FLOATVAL s2, FLOATVAL t2, FLOATVAL s3, FLOATVAL t3, FLOATVAL s4, FLOATVAL t4, DEFPARAMVAR );
		virtual STD_SO	SO_shadow( STRINGVAL name, FLOATVAL channel, POINTVAL P, DEFPARAMVAR );
		virtual STD_SO	SO_shadow1( STRINGVAL name, FLOATVAL channel, POINTVAL P1, POINTVAL P2, POINTVAL P3, POINTVAL P4, DEFPARAMVAR );
		virtual STD_SO	SO_illuminance( STRINGVAL Category, POINTVAL P, DEFVOIDPARAM );
		virtual STD_SO	SO_illuminance( STRINGVAL Category, POINTVAL P, VECTORVAL Axis, FLOATVAL Angle, DEFVOIDPARAM );
		virtual STD_SO	SO_illuminate( POINTVAL P, VECTORVAL Axis, FLOATVAL Angle, DEFVOIDPARAM );
		virtual STD_SO	SO_illuminate( POINTVAL P, DEFVOIDPARAM );
		virtual STD_SO	SO_solar( VECTORVAL Axis, FLOATVAL Angle, DEFVOIDPARAM );
		virtual STD_SO	SO_solar( DEFVOIDPARAM );
		virtual STD_SO	SO_gather( STRINGVAL category, POINTVAL P, VECTORVAL dir, FLOATVAL angle, FLOATVAL samples, DEFVOIDPARAMVAR );
		virtual STD_SO	SO_printf( STRINGVAL str, DEFVOIDPARAMVAR );
		virtual STD_SO	SO_format( STRINGVAL str, DEFPARAMVAR );
		virtual STD_SO	SO_concat( STRINGVAL stra, STRINGVAL strb, DEFPARAMVAR );
		virtual STD_SO	SO_atmosphere( STRINGVAL name, IqShaderData* pV, DEFPARAM );
		virtual STD_SO	SO_displacement( STRINGVAL name, IqShaderData* pV, DEFPARAM );
		virtual STD_SO	SO_lightsource( STRINGVAL name, IqShaderData* pV, DEFPARAM );
		virtual STD_SO	SO_surface( STRINGVAL name, IqShaderData* pV, DEFPARAM );
		virtual STD_SO	SO_attribute( STRINGVAL name, IqShaderData* pV, DEFPARAM );
		virtual STD_SO	SO_option( STRINGVAL name, IqShaderData* pV, DEFPARAM );
		virtual STD_SO	SO_rendererinfo( STRINGVAL name, IqShaderData* pV, DEFPARAM );
		virtual STD_SO	SO_incident( STRINGVAL name, IqShaderData* pV, DEFPARAM );
		virtual STD_SO	SO_opposite( STRINGVAL name, IqShaderData* pV, DEFPARAM );
		virtual STD_SO	SO_fcellnoise1( FLOATVAL v, DEFPARAM );
		virtual STD_SO	SO_fcellnoise2( FLOATVAL u, FLOATVAL v, DEFPARAM );
		virtual STD_SO	SO_fcellnoise3( POINTVAL p, DEFPARAM );
		virtual STD_SO	SO_fcellnoise4( POINTVAL p, FLOATVAL v, DEFPARAM );
		virtual STD_SO	SO_ccellnoise1( FLOATVAL v, DEFPARAM );
		virtual STD_SO	SO_ccellnoise2( FLOATVAL u, FLOATVAL v, DEFPARAM );
		virtual STD_SO	SO_ccellnoise3( POINTVAL p, DEFPARAM );
		virtual STD_SO	SO_ccellnoise4( POINTVAL p, FLOATVAL v, DEFPARAM );
		virtual STD_SO	SO_pcellnoise1( FLOATVAL v, DEFPARAM );
		virtual STD_SO	SO_pcellnoise2( FLOATVAL u, FLOATVAL v, DEFPARAM );
		virtual STD_SO	SO_pcellnoise3( POINTVAL p, DEFPARAM );
		virtual STD_SO	SO_pcellnoise4( POINTVAL p, FLOATVAL v, DEFPARAM );
		virtual STD_SO	SO_fpnoise1( FLOATVAL v, FLOATVAL period, DEFPARAM );
		virtual STD_SO	SO_fpnoise2( FLOATVAL u, FLOATVAL v, FLOATVAL uperiod, FLOATVAL vperiod, DEFPARAM );
		virtual STD_SO	SO_fpnoise3( POINTVAL p, POINTVAL pperiod, DEFPARAM );
		virtual STD_SO	SO_fpnoise4( POINTVAL p, FLOATVAL t, POINTVAL pperiod, FLOATVAL tperiod, DEFPARAM );
		virtual STD_SO	SO_cpnoise1( FLOATVAL v, FLOATVAL period, DEFPARAM );
		virtual STD_SO	SO_cpnoise2( FLOATVAL u, FLOATVAL v, FLOATVAL uperiod, FLOATVAL vperiod, DEFPARAM );
		virtual STD_SO	SO_cpnoise3( POINTVAL p, POINTVAL pperiod, DEFPARAM );
		virtual STD_SO	SO_cpnoise4( POINTVAL p, FLOATVAL t, POINTVAL pperiod, FLOATVAL tperiod, DEFPARAM );
		virtual STD_SO	SO_ppnoise1( FLOATVAL v, FLOATVAL period, DEFPARAM );
		virtual STD_SO	SO_ppnoise2( FLOATVAL u, FLOATVAL v, FLOATVAL uperiod, FLOATVAL vperiod, DEFPARAM );
		virtual STD_SO	SO_ppnoise3( POINTVAL p, POINTVAL pperiod, DEFPARAM );
		virtual STD_SO	SO_ppnoise4( POINTVAL p, FLOATVAL t, POINTVAL pperiod, FLOATVAL tperiod, DEFPARAM );
		virtual STD_SO	SO_ctransform( STRINGVAL fromspace, STRINGVAL tospace, COLORVAL c, DEFPARAM );
		virtual STD_SO	SO_ctransform( STRINGVAL tospace, COLORVAL c, DEFPARAM );
		virtual STD_SO	SO_ptlined( POINTVAL P0, POINTVAL P1, POINTVAL Q, DEFPARAM );
		virtual STD_SO	SO_inversesqrt( FLOATVAL x, DEFPARAM );
		virtual STD_SO	SO_match( STRINGVAL a, STRINGVAL b, DEFPARAM );
		virtual STD_SO	SO_rotate( VECTORVAL Q, FLOATVAL angle, POINTVAL P0, POINTVAL P1, DEFPARAM );
		virtual STD_SO	SO_filterstep( FLOATVAL edge, FLOATVAL s1, DEFPARAMVAR );
		virtual STD_SO	SO_filterstep2( FLOATVAL edge, FLOATVAL s1, FLOATVAL s2, DEFPARAMVAR );
		virtual STD_SO	SO_specularbrdf( VECTORVAL L, NORMALVAL N, VECTORVAL V, FLOATVAL rough, DEFPARAM );
		virtual STD_SO	SO_mtransform( STRINGVAL fromspace, STRINGVAL tospace, MATRIXVAL m, DEFPARAM );
		virtual STD_SO	SO_mtransform( STRINGVAL tospace, MATRIXVAL m, DEFPARAM );
		virtual STD_SO	SO_setmcomp( MATRIXVAL M, FLOATVAL row, FLOATVAL column, FLOATVAL val, DEFVOIDPARAM );
		virtual STD_SO	SO_determinant( MATRIXVAL M, DEFPARAM );
		virtual STD_SO	SO_mtranslate( MATRIXVAL M, VECTORVAL V, DEFPARAM );
		virtual STD_SO	SO_mrotate( MATRIXVAL M, FLOATVAL angle, VECTORVAL axis, DEFPARAM );
		virtual STD_SO	SO_mscale( MATRIXVAL M, POINTVAL s, DEFPARAM );
		virtual STD_SO	SO_fsplinea( FLOATVAL value, FLOATARRAYVAL a, DEFPARAM );
		virtual STD_SO	SO_csplinea( FLOATVAL value, COLORARRAYVAL a, DEFPARAM );
		virtual STD_SO	SO_psplinea( FLOATVAL value, POINTARRAYVAL a, DEFPARAM );
		virtual STD_SO	SO_sfsplinea( STRINGVAL basis, FLOATVAL value, FLOATARRAYVAL a, DEFPARAM );
		virtual STD_SO	SO_scsplinea( STRINGVAL basis, FLOATVAL value, COLORARRAYVAL a, DEFPARAM );
		virtual STD_SO	SO_spsplinea( STRINGVAL basis, FLOATVAL value, POINTARRAYVAL a, DEFPARAM );
		virtual STD_SO	SO_shadername( DEFPARAM );
		virtual STD_SO	SO_shadername2( STRINGVAL shader, DEFPARAM );
		virtual STD_SO	SO_textureinfo( STRINGVAL shader, STRINGVAL dataname, IqShaderData* pV, DEFPARAM );
		virtual STD_SO	SO_bake_f( STRINGVAL name, FLOATVAL s, FLOATVAL t, FLOATVAL f, DEFVOIDPARAMVAR );
		virtual STD_SO	SO_bake_3c( STRINGVAL name, FLOATVAL s, FLOATVAL t, COLORVAL f, DEFVOIDPARAMVAR );
		virtual STD_SO	SO_bake_3p( STRINGVAL name, FLOATVAL s, FLOATVAL t, POINTVAL f, DEFVOIDPARAMVAR );
		virtual STD_SO	SO_bake_3v( STRINGVAL name, FLOATVAL s, FLOATVAL t, VECTORVAL f, DEFVOIDPARAMVAR );
		virtual STD_SO	SO_bake_3n( STRINGVAL name, FLOATVAL s, FLOATVAL t, NORMALVAL f, DEFVOIDPARAMVAR );
		virtual STD_SO	SO_external( DSOMethod method, void* initData, DEFPARAMVAR );
		virtual STD_SO	SO_occlusion( STRINGVAL occlmap, FLOATVAL channel, POINTVAL P, NORMALVAL N, FLOATVAL samples, DEFPARAMVAR );
		virtual STD_SO	SO_occlusion_rt( POINTVAL P, NORMALVAL N, FLOATVAL samples, DEFPARAMVAR );
		virtual STD_SO	SO_rayinfo( STRINGVAL dataname, IqShaderData* pV, DEFPARAM );
		virtual STD_SO	SO_bake3d( STRINGVAL ptc, STRINGVAL channels, POINTVAL P, NORMALVAL N, DEFPARAMVAR );
		virtual STD_SO	SO_texture3d( STRINGVAL ptc, POINTVAL P, NORMALVAL N, DEFPARAMVAR );
};


//==============================================================================
// Implementation details
//==============================================================================

template<typename T>
T CqShaderExecEnv::diffU(IqShaderData* var, TqInt gridIdx)
{
	const T* data = 0;
	var->GetValuePtr(data);
	return m_diff.diffU(data, m_diffUidx[gridIdx], m_diffVidx[gridIdx]);
}

template<typename T>
T CqShaderExecEnv::diffV(IqShaderData* var, TqInt gridIdx)
{
	const T* data = 0;
	var->GetValuePtr(data);
	return m_diff.diffV(data, m_diffUidx[gridIdx], m_diffVidx[gridIdx]);
}

template<typename T>
T CqShaderExecEnv::deriv(IqShaderData* y, IqShaderData* x, TqInt gridIdx)
{
	// At first sight this seems like a strange kind of derivative operation,
	// since we may view both x and y as values which vary across the grid:
	//
	// x = x(u,v)
	// y = y(u,v)
	//
	// However, if y is a function of x,  y = y(x(u,v)), we have:
	//
	// dy/du = dy/dx * dx/du
	// dy/dv = dy/dx * dx/dv
	//
	// (all derivatives with respect to u and v are partial derivatives in
	// these expressions.)
	//
	// This gives us two equivilant possibilities for calculating dy/dx:
	//
	// dy/dx = (dy/du) / (dx/du) = diffU(y) / diffU(x)
	// dy/dx = (dy/dv) / (dx/dv) = diffV(y) / diffV(x)
	//
	// Either of these is fine, as long as dx/du and dx/dv are nonzero to
	// within floating poing rounding error.  For numerical stability, we
	// choose the direction u or v which has the maximum value for the value of
	// dx.
	//
	TqFloat dxu = diffU<TqFloat>(x, gridIdx);
	TqFloat dxv = diffV<TqFloat>(x, gridIdx);
	TqFloat absDxu = std::fabs(dxu);
	if(absDxu >= std::fabs(dxv))
	{	
		if(absDxu > 0) 
			return diffU<T>(y, gridIdx) / dxu;
		else
			return T();
	}
	else
	{
		return diffV<T>(y, gridIdx) / dxv;
	}
}


template<typename T>
inline T CqShaderExecEnv::derivU(IqShaderData* var, TqInt gridIdx, const T& undefVal)
{
	TqFloat duVal = 1;
	du()->GetFloat(duVal, gridIdx);
	if(duVal == 0)
		return undefVal;
	return diffU<T>(var, gridIdx) * (1/duVal);
}

template<typename T>
inline T CqShaderExecEnv::derivV(IqShaderData* var, TqInt gridIdx, const T& undefVal)
{
	TqFloat dvVal = 1;
	dv()->GetFloat(dvVal, gridIdx);
	if(dvVal == 0)
		return undefVal;
	return diffV<T>(var, gridIdx) * (1/dvVal);
}

//-----------------------------------------------------------------------

} // namespace Aqsis

#endif	// !SHADEREXECENV_H_INCLUDED
