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
		\brief Implements the classes for handling RenderMan imagersources, plus any built in sources.
		\author Michel Joron (joron@sympatico.ca)
*/

#include	"aqsis.h"
#include	"imagers.h"
#include	"file.h"


START_NAMESPACE( Aqsis )


//---------------------------------------------------------------------
/** Default constructor.
 */

CqImagersource::CqImagersource( IqShader* pShader, TqBool fActive ) :
		m_pShader( pShader ),
		m_pAttributes( NULL ),
		m_pShaderExecEnv( NULL )
{

	m_pAttributes = const_cast<CqAttributes*>( QGetRenderContext() ->pattrCurrent() );
	m_pAttributes->AddRef();
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqImagersource::~CqImagersource()
{
	if ( m_pAttributes )
		m_pAttributes->Release();
	m_pAttributes = 0;

	/// \note This should delete through the interface that created it.
	if( NULL != m_pShaderExecEnv )	
		delete( m_pShaderExecEnv );
}

//---------------------------------------------------------------------
/** Initialise the environment for the specified grid size
 *   and execute the shader on the complete grid.
 * \param uGridRes Integer grid resolution == 1.
 * \param vGridRes Integer grid resolution == 1.
 * \param x Integer Raster position.
 * \param y Integer Raster position.
 * \param color Initial value Ci.
 * \param opacity Initial value Oi.
 * \param depth Initial value depth (not required).
 * \param coverage Initial value "alpha"
 */
void CqImagersource::Initialise( TqInt uGridRes, TqInt vGridRes,
                                 float x, float y,
                                 CqColor *color, CqColor *opacity,
                                 TqFloat *depth, TqFloat *coverage )
{
	QGetRenderContext() ->Stats().ImagerTimer().Start();

	TqInt mode = QGetRenderContext() ->optCurrent().GetIntegerOption("System", "DisplayMode")[0];
	TqFloat components;
	TqInt j, i;
	TqFloat shuttertime = QGetRenderContext() ->optCurrent().GetFloatOption("System", "Shutter")[0];

	components = mode & ModeRGB ? 3 : 0;
	components += mode & ModeA ? 1 : 0;
	components = mode & ModeZ ? 1 : components;

	TqInt Uses = ( 1 << EnvVars_P ) | ( 1 << EnvVars_Ci ) | ( 1 << EnvVars_Oi | ( 1 << EnvVars_ncomps ) | ( 1 << EnvVars_time ) | ( 1 << EnvVars_alpha ) );

	/// \note This should delete through the interface that created it.
	if( NULL != m_pShaderExecEnv )	
		delete( m_pShaderExecEnv );

	m_pShaderExecEnv = new CqShaderExecEnv;
	m_pShaderExecEnv->Initialise( uGridRes, vGridRes, 0, m_pShader, Uses );

	// Initialise the geometric parameters in the shader exec env.

	i = ( vGridRes + 1 ) * ( uGridRes + 1 );
	P()->Initialise( uGridRes, vGridRes );
	Ci()->Initialise( uGridRes, vGridRes );
	Oi()->Initialise( uGridRes, vGridRes );
	alpha()->Initialise( uGridRes, vGridRes );

	//TODO dtime is not initialised yet
	//dtime().Initialise(uGridRes, vGridRes, i);

	alpha()->SetFloat( coverage[ 0 ] );
	ncomps()->SetFloat( components );
	time()->SetFloat( shuttertime );


	m_pShader->Initialise( uGridRes, vGridRes, m_pShaderExecEnv );
	for ( j = 0; j < vGridRes; j++ )
		for ( i = 0; i < uGridRes; i++ )
		{

			P()->SetPoint( CqVector3D( x + i, y + j, 0.0 ), j * ( uGridRes + 1 ) + i);
			Ci()->SetColor( color[ j * ( uGridRes ) + i ], j * ( uGridRes + 1 ) + i);
			Oi()->SetColor( opacity[ j * ( uGridRes ) + i ], j * ( uGridRes + 1 ) + i);

		}
	// Execute the Shader VM
	if ( m_pShader )
	{
		m_pShader->Evaluate( m_pShaderExecEnv );
		alpha()->SetFloat( 1.0f ); /* by default 3delight/bmrt set it to 1.0 */
	}

	m_uYOrigin = static_cast<TqInt>(y);
	m_uXOrigin = static_cast<TqInt>(x);
	m_uGridRes = uGridRes;
	m_vGridRes = vGridRes;

	QGetRenderContext() ->Stats().ImagerTimer().Stop();
}

//---------------------------------------------------------------------
/** After the running shader on the complete grid return the compute Ci
 * \param x Integer Raster position.
 * \param y Integer Raster position.
 */
CqColor CqImagersource::Color( TqFloat x, TqFloat y )
{
	CqColor result = gColBlack;

	TqInt index = static_cast<TqInt>(( y - m_uYOrigin ) * ( m_uGridRes + 1 ) + x - m_uXOrigin);

	if ( Ci()->Size() >= index )
		Ci()->GetColor( result, index );

	return result;
}

//---------------------------------------------------------------------
/** After the running shader on the complete grid return the compute Oi
 * \param x Integer Raster position.
 * \param y Integer Raster position.
 */
CqColor CqImagersource::Opacity( TqFloat x, TqFloat y )
{
	CqColor result = gColWhite;

	TqInt index = static_cast<TqInt>(( y - m_uYOrigin ) * ( m_uGridRes + 1 ) + x - m_uXOrigin);

	if ( Oi()->Size() >= index )
		Oi()->GetColor( result, index );

	return result;
}

//---------------------------------------------------------------------
/** After the running shader on the complete grid return the compute alpha
 * \param x Integer Raster position.
 * \param y Integer Raster position.
 * PS this is always 1.0 after running imager shader!
 */
TqFloat CqImagersource::Alpha( TqFloat x, TqFloat y )
{
	TqFloat result;

	alpha()->GetFloat( result );

	return result;
}


//---------------------------------------------------------------------

END_NAMESPACE( Aqsis )


