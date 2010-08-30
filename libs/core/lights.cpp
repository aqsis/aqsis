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
		\brief Implements the classes for handling RenderMan lightsources, plus any built in sources.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	<aqsis/aqsis.h>
#include	"lights.h"
#include	<aqsis/util/file.h>
#include	"renderer.h"

namespace Aqsis {

//---------------------------------------------------------------------
/** Default constructor.
 */

CqLightsource::CqLightsource( const boost::shared_ptr<IqShader>& pShader, bool fActive ) :
		m_pShader( pShader ),
		m_pAttributes(),
		m_pTransform(),
		m_pShaderExecEnv(IqShaderExecEnv::create(QGetRenderContextI()))
{
	// Set a reference with the current attributes.
	m_pAttributes = QGetRenderContext() ->pattrCurrent();

	m_pShader->SetType(Type_Lightsource);
	m_pTransform = QGetRenderContext() ->ptransCurrent();
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqLightsource::~CqLightsource()
{
}


//---------------------------------------------------------------------
/** Initialise the environment for the specified grid size.
 * \param iGridRes Integer grid resolution.
 * \param iGridRes Integer grid resolution.
 */
void CqLightsource::Initialise( TqInt uGridRes, TqInt vGridRes, TqInt microPolygonCount, TqInt shadingPointCount, bool hasValidDerivatives )
{
	TqInt Uses = gDefLightUses;
	if ( m_pShader )
	{
		Uses |= m_pShader->Uses();
		m_pShaderExecEnv->Initialise( uGridRes, vGridRes, microPolygonCount, shadingPointCount, hasValidDerivatives, m_pAttributes, boost::shared_ptr<IqTransform>(), m_pShader.get(), Uses );
	}

	if ( m_pShader )
		m_pShader->Initialise( uGridRes, vGridRes, shadingPointCount, m_pShaderExecEnv.get() );

	if ( USES( Uses, EnvVars_L ) )
		L() ->Initialise( shadingPointCount );
	if ( USES( Uses, EnvVars_Cl ) )
		Cl() ->Initialise( shadingPointCount );

	// Initialise the geometric parameters in the shader exec env.
	if ( USES( Uses, EnvVars_P ) )
	{
		CqMatrix mat;
		QGetRenderContext() ->matSpaceToSpace( "shader", "current", m_pShader->getTransform(), NULL, QGetRenderContextI()->Time(), mat );
		P() ->SetPoint( mat * CqVector3D( 0.0f, 0.0f, 0.0f ) );
	}
	if ( USES( Uses, EnvVars_u ) )
		u() ->SetFloat( 0.0f );
	if ( USES( Uses, EnvVars_v ) )
		v() ->SetFloat( 0.0f );
	if ( USES( Uses, EnvVars_du ) )
		du() ->SetFloat( 0.0f );
	if ( USES( Uses, EnvVars_dv ) )
		dv() ->SetFloat( 0.0f );
	if ( USES( Uses, EnvVars_s ) )
		s() ->SetFloat( 0.0f );
	if ( USES( Uses, EnvVars_t ) )
		t() ->SetFloat( 0.0f );
	if ( USES( Uses, EnvVars_N ) )
		N() ->SetNormal( CqVector3D( 0.0f, 0.0f, 0.0f ) );
}



//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
// These are the built in shaders, they will be registered as "builtin_<name>"
// these should be used where speed is an issue.

//---------------------------------------------------------------------

} // namespace Aqsis


