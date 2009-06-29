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
		\brief Declares the classes for handling RenderMan imagersources, plus any built in sources.
		\author Michel Joron (joron@sympatico.ca)
*/

//? Is .h included already?
#ifndef IMAGERS_H_INCLUDED
//{
#define IMAGERS_H_INCLUDED 1

#include	<vector>

#include	<aqsis/aqsis.h>

#include <aqsis/ri/ri.h>
#include <aqsis/math/color.h>
#include <aqsis/util/list.h>
#include <aqsis/math/matrix.h>
#include <aqsis/shadervm/ishaderexecenv.h>
#include <aqsis/math/vector4d.h>
#include <aqsis/version.h>
#include "attributes.h"

namespace Aqsis {

//----------------------------------------------------------------------
/** \class CqImagersource
 * Abstract base class from which all Imagersources are derived.
 */

class CqImagersource : public CqListEntry<CqImagersource>
{
	public:
		CqImagersource( const boost::shared_ptr<IqShader>& pShader, bool fActive = true );
		virtual	~CqImagersource();

		/** Get a pointer to the associated Imagersource shader.
		 * \return a pointer to a CqShader derived class.
		 */
		boost::shared_ptr<IqShader>	pShader()
		{
			return ( m_pShader );
		}

		/** Get a pointer to the associated Imagersource shader.
		 * \return a pointer to a IqShader derived class.
		 */
		const boost::shared_ptr<IqShader>	pShader() const
		{
			return ( m_pShader );
		}

		/** \brief Initialise and execute the imager shader over the provided bucket.
		 *
		 * \param pBucket - Bucket from which to take the source shader data for the
		 *                  imager shader.
		 */
		void Initialise(const CqRegion& DRegion, IqChannelBuffer* buffer);


		// Forwarding functions for IqShaderExecEnv
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
		virtual	const CqMatrix&	matObjectToWorld() const
		{
			assert( m_pShaderExecEnv );
			return ( m_pShaderExecEnv->matObjectToWorld() );
		}
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


	public:
		CqColor Color( TqFloat x, TqFloat y );
		CqColor Opacity( TqFloat x, TqFloat y );
		TqFloat Alpha( TqFloat x, TqFloat y );

	private:
		boost::shared_ptr<IqShader>	m_pShader;				///< Pointer to the associated shader.
		CqConstAttributesPtr	m_pAttributes;			///< Pointer to the associated attributes.

		TqInt m_vGridRes;						///< Size of the bucket X
		TqInt m_uGridRes;						///<                    Y in pixels
		TqInt m_uXOrigin;						///< its origin 0,0 top left corner
		TqInt m_uYOrigin;						///<                      in pixels
		boost::shared_ptr<IqShaderExecEnv>	m_pShaderExecEnv;
}
;


//--------------------
// background is hard-coded within in sync with options.cpp,options.h
//--------------------

//-----------------------------------------------------------------------

} // namespace Aqsis

//}  // End of #ifdef IMAGERS_H_INCLUDED
#endif
