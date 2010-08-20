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
		\brief Declares the classes for handling RenderMan lightsources, plus any built in sources.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef LIGHTS_H_INCLUDED
//{
#define LIGHTS_H_INCLUDED 1

#include	<vector>
#include	<deque>
#include	<boost/shared_ptr.hpp>
#include	<boost/enable_shared_from_this.hpp>

#include	<aqsis/aqsis.h>

#include <aqsis/ri/ri.h>
#include <aqsis/math/color.h>
#include <aqsis/util/list.h>
#include <aqsis/math/matrix.h>
#include <aqsis/shadervm/ishaderexecenv.h>
#include <aqsis/math/vector4d.h>
#include <aqsis/version.h>
#include <aqsis/core/ilightsource.h>
#include "attributes.h"
#include "transform.h"

namespace Aqsis {

class CqLightsource;
typedef boost::shared_ptr<CqLightsource> CqLightsourcePtr;

//----------------------------------------------------------------------
/** \class CqLightsource
 * Abstract base class from which all lightsources are derived.
 */

class CqLightsource : public IqLightsource, public boost::enable_shared_from_this<CqLightsource>
{
	public:
		CqLightsource( const boost::shared_ptr<IqShader>& pShader, bool fActive = true );
		virtual	~CqLightsource();

#ifdef _DEBUG

		CqString className() const
		{
			return CqString("CqLightsource");
		}
#endif

		/** Get a pointer to the associated lightsource shader.
		 * \return a pointer to a IqShader derived class.
		 */
		virtual boost::shared_ptr<IqShader>	pShader() const
		{
			return ( m_pShader );
		}
		/** Initialise the shader execution environment.
		 * \param uGridRes Integer grid size, not used.
		 * \param vGridRes Integer grid size, not used.
		 */
		virtual void	Initialise( TqInt uGridRes, TqInt vGridRes, TqInt microPolygonCount, TqInt shadingPointCount, bool hasValidDerivatives );
		//			void		GenerateShadowMap(const char* strShadowName);
		/** Evaluate the shader.
		 * \param pPs the point being lit.
		 */
		virtual void	Evaluate( IqShaderData* pPs, IqShaderData* pNs, IqSurface* pSurface )
		{
			Ps() ->SetValueFromVariable( pPs );
			Ns() ->SetValueFromVariable( pNs );
			m_pShaderExecEnv->SetCurrentSurface(pSurface);
			m_pShader->Evaluate( m_pShaderExecEnv.get() );
		}
		/** Get a pointer to the attributes state associated with this GPrim.
		 * \return A pointer to a CqAttributes class.
		 */
		virtual IqConstAttributesPtr pAttributes() const
		{
			return ( m_pAttributes );
		}

		/** Get a pointer to the transformation state associated with this GPrim.
		 * \return A pointer to a CqTransform class.
		 */
		virtual CqTransformPtr pTransform() const
		{
			return ( m_pTransform );
		}
		// Redirect acces via IqShaderExecEnv
		virtual	TqInt	uGridRes() const
		{
			assert( m_pShaderExecEnv );
			return ( m_pShaderExecEnv->uGridRes() );
		}
		virtual	TqInt	vGridRes() const
		{
			assert( m_pShaderExecEnv );
			return ( m_pShaderExecEnv->vGridRes() );
		}
		virtual	TqInt	microPolygonCount() const
		{
			assert( m_pShaderExecEnv );
			return ( m_pShaderExecEnv->microPolygonCount() );
		}
		virtual	TqInt	shadingPointCount() const
		{
			assert( m_pShaderExecEnv );
			return ( m_pShaderExecEnv->shadingPointCount() );
		}
/*		virtual	const CqMatrix&	matObjectToWorld() const
		{
			assert( m_pShaderExecEnv );
			return ( m_pShaderExecEnv->matObjectToWorld() );
		}*/
		virtual	IqShaderData* Cs()
		{
			assert( m_pShaderExecEnv );
			return ( m_pShaderExecEnv->Cs() );
		}
		virtual	IqShaderData* Os()
		{
			assert( m_pShaderExecEnv );
			return ( m_pShaderExecEnv->Os() );
		}
		virtual	IqShaderData* Ng()
		{
			assert( m_pShaderExecEnv );
			return ( m_pShaderExecEnv->Ng() );
		}
		virtual	IqShaderData* du()
		{
			assert( m_pShaderExecEnv );
			return ( m_pShaderExecEnv->du() );
		}
		virtual	IqShaderData* dv()
		{
			assert( m_pShaderExecEnv );
			return ( m_pShaderExecEnv->dv() );
		}
		virtual	IqShaderData* L()
		{
			assert( m_pShaderExecEnv );
			return ( m_pShaderExecEnv->L() );
		}
		virtual	IqShaderData* Cl()
		{
			assert( m_pShaderExecEnv );
			return ( m_pShaderExecEnv->Cl() );
		}
		virtual IqShaderData* Ol()
		{
			assert( m_pShaderExecEnv );
			return ( m_pShaderExecEnv->Ol() );
		}
		virtual IqShaderData* P()
		{
			assert( m_pShaderExecEnv );
			return ( m_pShaderExecEnv->P() );
		}
		virtual IqShaderData* dPdu()
		{
			assert( m_pShaderExecEnv );
			return ( m_pShaderExecEnv->dPdu() );
		}
		virtual IqShaderData* dPdv()
		{
			assert( m_pShaderExecEnv );
			return ( m_pShaderExecEnv->dPdv() );
		}
		virtual IqShaderData* N()
		{
			assert( m_pShaderExecEnv );
			return ( m_pShaderExecEnv->N() );
		}
		virtual IqShaderData* u()
		{
			assert( m_pShaderExecEnv );
			return ( m_pShaderExecEnv->u() );
		}
		virtual IqShaderData* v()
		{
			assert( m_pShaderExecEnv );
			return ( m_pShaderExecEnv->v() );
		}
		virtual IqShaderData* s()
		{
			assert( m_pShaderExecEnv );
			return ( m_pShaderExecEnv->s() );
		}
		virtual IqShaderData* t()
		{
			assert( m_pShaderExecEnv );
			return ( m_pShaderExecEnv->t() );
		}
		virtual IqShaderData* I()
		{
			assert( m_pShaderExecEnv );
			return ( m_pShaderExecEnv->I() );
		}
		virtual IqShaderData* Ci()
		{
			assert( m_pShaderExecEnv );
			return ( m_pShaderExecEnv->Ci() );
		}
		virtual IqShaderData* Oi()
		{
			assert( m_pShaderExecEnv );
			return ( m_pShaderExecEnv->Oi() );
		}
		virtual IqShaderData* Ps()
		{
			assert( m_pShaderExecEnv );
			return ( m_pShaderExecEnv->Ps() );
		}
		virtual IqShaderData* E()
		{
			assert( m_pShaderExecEnv );
			return ( m_pShaderExecEnv->E() );
		}
		virtual IqShaderData* ncomps()
		{
			assert( m_pShaderExecEnv );
			return ( m_pShaderExecEnv->ncomps() );
		}
		virtual IqShaderData* time()
		{
			assert( m_pShaderExecEnv );
			return ( m_pShaderExecEnv->time() );
		}
		virtual IqShaderData* alpha()
		{
			assert( m_pShaderExecEnv );
			return ( m_pShaderExecEnv->alpha() );
		}
		virtual IqShaderData* Ns()
		{
			assert( m_pShaderExecEnv );
			return ( m_pShaderExecEnv->Ns() );
		}

	private:
		boost::shared_ptr<IqShader>	m_pShader;				///< Pointer to the associated shader.
		CqAttributesPtr	m_pAttributes;			///< Pointer to the associated attributes.
		CqTransformPtr m_pTransform;		///< Pointer to the transformation state associated with this GPrim.
		boost::shared_ptr<IqShaderExecEnv>	m_pShaderExecEnv;	///< Pointer to the shader execution environment.
}
;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
// These are the built in shaders, they will be registered as "builtin_<name>"
// these should be used where speed is an issue.

//-----------------------------------------------------------------------

} // namespace Aqsis

//}  // End of #ifdef LIGHTS_H_INCLUDED
#endif
