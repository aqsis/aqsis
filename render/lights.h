// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
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
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef LIGHTS_H_INCLUDED 
//{
#define LIGHTS_H_INCLUDED 1

#include	<vector>

#include	"aqsis.h"

#include "ri.h"
#include "color.h"
#include "list.h"
#include "matrix.h"
#include "shaderexecenv.h"
#include "vector4d.h"
#include "imagebuffer.h"
#include "version.h"
#include "ilightsource.h"

#define		_qShareName	CORE
#include	"share.h"

START_NAMESPACE( Aqsis )

//----------------------------------------------------------------------
/** \class CqLightsource
 * Abstract base class from which all lightsources are derived.
 */

class CqLightsource : public CqListEntry<CqLightsource>, public IqLightsource
{
	public:
		CqLightsource( IqShader* pShader, TqBool fActive = TqTrue );
		virtual	~CqLightsource();

		/** Get a pointer to the associated lightsource shader.
		 * \return a pointer to a IqShader derived class.
		 */
		virtual IqShader*	pShader()
		{
			return ( m_pShader );
		}
		/** Initialise the shader execution environment.
		 * \param uGridRes Integer grid size, not used.
		 * \param vGridRes Integer grid size, not used.
		 */
		virtual void	Initialise( TqInt uGridRes, TqInt vGridRes );
		//			void		GenerateShadowMap(const char* strShadowName);
		/** Evaluate the shader.
		 * \param pPs the point being lit.
		 */
		virtual void	Evaluate( IqShaderData* pPs )
		{
			Ps()->SetValueFromVariable( pPs );
			m_pShader->Evaluate( m_pShaderExecEnv );
		}
		/** Get a pointer to the attributes associated with this lightsource.
		 * \return a CqAttributes pointer.
		 */
		virtual IqAttributes*	pAttributes() const
		{
			return ( m_pAttributes );
		}

		// Redirect acces via IqShaderExecEnv
		virtual	TqInt	uGridRes() const	{ assert( NULL != m_pShaderExecEnv ); return( m_pShaderExecEnv->uGridRes() ); }
		virtual	TqInt	vGridRes() const	{ assert( NULL != m_pShaderExecEnv ); return( m_pShaderExecEnv->vGridRes() ); }
		virtual	TqInt	GridSize() const	{ assert( NULL != m_pShaderExecEnv ); return( m_pShaderExecEnv->GridSize() ); }
		virtual	const CqMatrix&	matObjectToWorld() const { assert( NULL != m_pShaderExecEnv ); return( m_pShaderExecEnv->matObjectToWorld() ); }
		virtual	IqShaderData* Cs()			{ assert( NULL != m_pShaderExecEnv ); return( m_pShaderExecEnv->Cs() ); }
		virtual	IqShaderData* Os()			{ assert( NULL != m_pShaderExecEnv ); return( m_pShaderExecEnv->Os() ); }
		virtual	IqShaderData* Ng()			{ assert( NULL != m_pShaderExecEnv ); return( m_pShaderExecEnv->Ng() ); }
		virtual	IqShaderData* du()			{ assert( NULL != m_pShaderExecEnv ); return( m_pShaderExecEnv->du() ); }
		virtual	IqShaderData* dv()			{ assert( NULL != m_pShaderExecEnv ); return( m_pShaderExecEnv->dv() ); }
		virtual	IqShaderData* L()			{ assert( NULL != m_pShaderExecEnv ); return( m_pShaderExecEnv->L() ); }
		virtual	IqShaderData* Cl()			{ assert( NULL != m_pShaderExecEnv ); return( m_pShaderExecEnv->Cl() ); }
		virtual IqShaderData* Ol()			{ assert( NULL != m_pShaderExecEnv ); return( m_pShaderExecEnv->Ol() ); }
		virtual IqShaderData* P()			{ assert( NULL != m_pShaderExecEnv ); return( m_pShaderExecEnv->P() ); }
		virtual IqShaderData* dPdu()		{ assert( NULL != m_pShaderExecEnv ); return( m_pShaderExecEnv->dPdu() ); }
		virtual IqShaderData* dPdv()		{ assert( NULL != m_pShaderExecEnv ); return( m_pShaderExecEnv->dPdv() ); }
		virtual IqShaderData* N()			{ assert( NULL != m_pShaderExecEnv ); return( m_pShaderExecEnv->N() ); }
		virtual IqShaderData* u()			{ assert( NULL != m_pShaderExecEnv ); return( m_pShaderExecEnv->u() ); }
		virtual IqShaderData* v()			{ assert( NULL != m_pShaderExecEnv ); return( m_pShaderExecEnv->v() ); }
		virtual IqShaderData* s()			{ assert( NULL != m_pShaderExecEnv ); return( m_pShaderExecEnv->s() ); }
		virtual IqShaderData* t()			{ assert( NULL != m_pShaderExecEnv ); return( m_pShaderExecEnv->t() ); }
		virtual IqShaderData* I()			{ assert( NULL != m_pShaderExecEnv ); return( m_pShaderExecEnv->I() ); }
		virtual IqShaderData* Ci()			{ assert( NULL != m_pShaderExecEnv ); return( m_pShaderExecEnv->Ci() ); }
		virtual IqShaderData* Oi()			{ assert( NULL != m_pShaderExecEnv ); return( m_pShaderExecEnv->Oi() ); }
		virtual IqShaderData* Ps()			{ assert( NULL != m_pShaderExecEnv ); return( m_pShaderExecEnv->Ps() ); }
		virtual IqShaderData* E()			{ assert( NULL != m_pShaderExecEnv ); return( m_pShaderExecEnv->E() ); }
		virtual IqShaderData* ncomps()		{ assert( NULL != m_pShaderExecEnv ); return( m_pShaderExecEnv->ncomps() ); }
		virtual IqShaderData* time()		{ assert( NULL != m_pShaderExecEnv ); return( m_pShaderExecEnv->time() ); }
		virtual IqShaderData* alpha()		{ assert( NULL != m_pShaderExecEnv ); return( m_pShaderExecEnv->alpha() ); }

	private:
		IqShader*		m_pShader;				///< Pointer to the associated shader.
		CqAttributes*	m_pAttributes;			///< Pointer to the associated attributes.
		IqShaderExecEnv*	m_pShaderExecEnv;	///< Pointer to the shader execution environment.
}
;

extern CqList<CqLightsource>	Lightsource_stack;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
// These are the built in shaders, they will be registered as "builtin_<name>"
// these should be used where speed is an issue.

//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

//}  // End of #ifdef LIGHTS_H_INCLUDED
#endif
