// Aqsis
// Copyright ) 1997 - 2001, Paul C. Gregory
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

#include	"MultiTimer.h"

#include	"aqsis.h"
#include	"imagers.h"
#include	"file.h"


START_NAMESPACE( Aqsis )


//---------------------------------------------------------------------
/** Default constructor.
 */

CqImagersource::CqImagersource( const boost::shared_ptr<IqShader>& pShader, TqBool fActive ) :
        m_pShader( pShader ),
        m_pAttributes( NULL ),
        m_pShaderExecEnv( new CqShaderExecEnv )
{

    m_pAttributes = const_cast<CqAttributes*>( QGetRenderContext() ->pattrCurrent() );
    ADDREF( m_pAttributes );
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqImagersource::~CqImagersource()
{
    if ( m_pAttributes )
        RELEASEREF( m_pAttributes );
    m_pAttributes = 0;
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
void CqImagersource::Initialise( IqBucket* pBucket )
{
	TIME_SCOPE("Imager shading")

    TqInt uGridRes = pBucket->Width();
    TqInt vGridRes = pBucket->Height();
    TqInt x = pBucket->XOrigin();
    TqInt y = pBucket->YOrigin();

    m_uYOrigin = static_cast<TqInt>( y );
    m_uXOrigin = static_cast<TqInt>( x );
    m_uGridRes = uGridRes;
    m_vGridRes = vGridRes;

    TqInt mode = QGetRenderContext() ->optCurrent().GetIntegerOption( "System", "DisplayMode" ) [ 0 ];
    TqFloat components;
    TqInt j, i;
    TqFloat shuttertime = QGetRenderContext() ->optCurrent().GetFloatOption( "System", "Shutter" ) [ 0 ];

    components = mode & ModeRGB ? 3 : 0;
    components += mode & ModeA ? 1 : 0;
    components = mode & ModeZ ? 1 : components;

    TqInt Uses = ( 1 << EnvVars_P ) | ( 1 << EnvVars_Ci ) | ( 1 << EnvVars_Oi | ( 1 << EnvVars_ncomps ) | ( 1 << EnvVars_time ) | ( 1 << EnvVars_alpha ) | ( 1 << EnvVars_s ) | ( 1 << EnvVars_t ) );

    m_pShaderExecEnv->Initialise( uGridRes, vGridRes, 0, boost::shared_ptr<IqTransform>(), m_pShader.get(), Uses );

    // Initialise the geometric parameters in the shader exec env.

    P() ->Initialise( uGridRes, vGridRes );
    Ci() ->Initialise( uGridRes, vGridRes );
    Oi() ->Initialise( uGridRes, vGridRes );
    alpha() ->Initialise( uGridRes, vGridRes );
    s() ->Initialise( uGridRes, vGridRes );
    t() ->Initialise( uGridRes, vGridRes );

    //TODO dtime is not initialised yet
    //dtime().Initialise(uGridRes, vGridRes, i);

    ncomps() ->SetFloat( components );
    time() ->SetFloat( shuttertime );


    m_pShader->Initialise( uGridRes, vGridRes, m_pShaderExecEnv );
    for ( j = 0; j < vGridRes; j++ )
    {
        for ( i = 0; i < uGridRes; i++ )
        {
            TqInt off = j * ( uGridRes + 1 ) + i;
            P() ->SetPoint( CqVector3D( x + i, y + j, 0.0 ), off );
            Ci() ->SetColor( pBucket->Color( x + i, y + j ), off );
            CqColor opa = pBucket->Opacity( x + i, y + j );
            Oi() ->SetColor( opa, off );
            TqFloat avopa = ( opa.fRed() + opa.fGreen() + opa.fBlue() ) /3.0f;
            alpha() ->SetFloat( pBucket->Coverage( x + i, y + j ) * avopa, off );
            s() ->SetFloat( x + i + 0.5, off );
            t() ->SetFloat( y + j + 0.5, off );
        }
    }
    // Execute the Shader VM
    if ( m_pShader )
    {
        m_pShader->Evaluate( m_pShaderExecEnv );
        alpha() ->SetFloat( 1.0f ); /* by default 3delight/bmrt set it to 1.0 */
    }
}

//---------------------------------------------------------------------
/** After the running shader on the complete grid return the compute Ci
 * \param x Integer Raster position.
 * \param y Integer Raster position.
 */
CqColor CqImagersource::Color( TqFloat x, TqFloat y )
{
    CqColor result = gColBlack;

    TqInt index = static_cast<TqInt>( ( y - m_uYOrigin ) * ( m_uGridRes + 1 ) + x - m_uXOrigin );

    if ( (TqInt)Ci() ->Size() >= index )
        Ci() ->GetColor( result, index );

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

    TqInt index = static_cast<TqInt>( ( y - m_uYOrigin ) * ( m_uGridRes + 1 ) + x - m_uXOrigin );

    if ((TqInt) Oi() ->Size() >= index )
        Oi() ->GetColor( result, index );

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

    alpha() ->GetFloat( result );

    return result;
}


//---------------------------------------------------------------------

END_NAMESPACE( Aqsis )


