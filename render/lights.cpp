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
		\brief Implements the classes for handling RenderMan lightsources, plus any built in sources.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	"aqsis.h"
#include	"lights.h"
#include	"file.h"

START_NAMESPACE( Aqsis )

CqList<CqLightsource>	Lightsource_stack;

//---------------------------------------------------------------------
/** Default constructor.
 */

CqLightsource::CqLightsource( IqShader* pShader, TqBool fActive ) :
        m_pShader( pShader ),
        m_pAttributes( NULL ),
        m_pShaderExecEnv( NULL )
{
    // Set a reference with the current attributes.
    m_pAttributes = const_cast<CqAttributes*>( QGetRenderContext() ->pattrCurrent() );
    ADDREF( m_pAttributes );

    // Link into the lightsource stack.
    Lightsource_stack.LinkFirst( this );
    // Add a reference from the stack.
    ADDREF( this );
    m_pShaderExecEnv = new CqShaderExecEnv;
    ADDREF( m_pShaderExecEnv );
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqLightsource::~CqLightsource()
{
    // Release our reference on the current attributes.
    if ( m_pAttributes )
        RELEASEREF( m_pAttributes );
    m_pAttributes = 0;

    // Delete the shader execution environment..
    if ( m_pShaderExecEnv ) {
        RELEASEREF( m_pShaderExecEnv );
    }
    m_pShaderExecEnv = 0;
}


//---------------------------------------------------------------------
/** Initialise the environment for the specified grid size.
 * \param iGridRes Integer grid resolution.
 * \param iGridRes Integer grid resolution.
 */
void CqLightsource::Initialise( TqInt uGridRes, TqInt vGridRes )
{
    TqInt Uses = gDefLightUses;
    if ( m_pShader )
    {
        Uses |= m_pShader->Uses();
        m_pShaderExecEnv->Initialise( uGridRes, vGridRes, m_pAttributes, 0, m_pShader, Uses );
    }

    if ( m_pShader )
        m_pShader->Initialise( uGridRes, vGridRes, m_pShaderExecEnv );

    if ( USES( Uses, EnvVars_L ) )	L() ->Initialise( uGridRes, vGridRes );
    if ( USES( Uses, EnvVars_Cl ) )	Cl() ->Initialise( uGridRes, vGridRes );

    // Initialise the geometric parameters in the shader exec env.
    if ( USES( Uses, EnvVars_P ) )	P() ->SetPoint( QGetRenderContext() ->matSpaceToSpace( "shader", "current", m_pShader->matCurrent() ) * CqVector3D( 0.0f, 0.0f, 0.0f ) );
    if ( USES( Uses, EnvVars_u ) )	u() ->SetFloat( 0.0f );
    if ( USES( Uses, EnvVars_v ) )	v() ->SetFloat( 0.0f );
    if ( USES( Uses, EnvVars_du ) )	du() ->SetFloat( 0.0f );
    if ( USES( Uses, EnvVars_du ) )	dv() ->SetFloat( 0.0f );
    if ( USES( Uses, EnvVars_s ) )	s() ->SetFloat( 0.0f );
    if ( USES( Uses, EnvVars_t ) )	t() ->SetFloat( 0.0f );
    if ( USES( Uses, EnvVars_N ) )	N() ->SetNormal( CqVector3D( 0.0f, 0.0f, 0.0f ) );
}



//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
// These are the built in shaders, they will be registered as "builtin_<name>"
// these should be used where speed is an issue.

//---------------------------------------------------------------------

END_NAMESPACE( Aqsis )


