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
#include "shadervm.h"
#include "shaderexecenv.h"
#include "vector4d.h"
#include "imagebuffer.h"
#include "version.h"

#define		_qShareName	CORE
#include	"share.h"

START_NAMESPACE( Aqsis )

//----------------------------------------------------------------------
/** \class CqLightsource
 * Abstract base class from which all lightsources are derived.
 */

class CqLightsource : public CqListEntry<CqLightsource>, public CqShaderExecEnv
{
	public:
		CqLightsource( IqShader* pShader, TqBool fActive = TqTrue );
		virtual	~CqLightsource();

		/** Get a pointer to the associated lightsource shader.
		 * \return a pointer to a IqShader derived class.
		 */
		IqShader*	pShader()
		{
			return ( m_pShader );
		}
		/** Initialise the shader execution environment.
		 * \param uGridRes Integer grid size, not used.
		 * \param cGridRes Integer grid size, not used.
		 */
		void	Initialise( TqInt uGridRes, TqInt vGridRes );
		//			void		GenerateShadowMap(const char* strShadowName);
		/** Evaluate the shader.
		 * \param Ps the point being lit.
		 */
		void	Evaluate( IqShaderData* pPs )
		{
			CqShaderExecEnv::Ps()->SetValueFromVariable( pPs );
			m_pShader->Evaluate( this );
		}
		/** Get a pointer to the attributes associated with this lightsource.
		 * \return a CqAttributes pointer.
		 */
		CqAttributes*	pAttributes() const
		{
			return ( m_pAttributes );
		}
	private:
		IqShader*		m_pShader;				///< Pointer to the associated shader.
		CqAttributes*	m_pAttributes;			///< Pointer to the associated attributes.
		//			CqMatrix		m_matLightToWorld;		///< Light to world transformation matrix.
		//			CqMatrix		m_matWorldToLight;		///< World to light transformation matrix.
}
;

extern CqList<CqLightsource>	Lightsource_stack;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
// These are the built in shaders, they will be registered as "builtin_<name>"
// these should be used where speed is an issue.

//----------------------------------------------------------------------
/** \class CqShaderLightsourceAmbient
 * Ambient lightsource shader
 */

class CqShaderLightsourceAmbient : public CqShader
{
	public:
		CqShaderLightsourceAmbient();
		virtual	~CqShaderLightsourceAmbient();

		virtual	void	Evaluate( IqShaderExecEnv* pEnv );
		virtual	void	SetValue( const char* name, TqPchar val );
		virtual IqShader* Clone() const
		{
			return ( new CqShaderLightsourceAmbient );
		}

		virtual	TqBool	fAmbient() const
		{
			return ( TqTrue );
		}

	private:
		IqShaderData*	m_intensity;
		IqShaderData*	m_lightcolor;
};



//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

//}  // End of #ifdef LIGHTS_H_INCLUDED
#endif
