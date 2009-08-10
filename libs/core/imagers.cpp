// Aqsis
// Copyright ) 1997 - 2001, Paul C. Gregory
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
		\brief Implements the classes for handling RenderMan imagersources, plus any built in sources.
		\author Michel Joron (joron@sympatico.ca)
*/

#include	"imagers.h"
#include	"iddmanager.h"
#include	"attributes.h"
#include	"renderer.h"
#include	<aqsis/util/file.h>
#include	<aqsis/math/region.h>

#include	"stats.h"

namespace Aqsis {


//---------------------------------------------------------------------
/** Default constructor.
 */

CqImagersource::CqImagersource( const boost::shared_ptr<IqShader>& pShader, bool fActive ) :
		m_pShader( pShader ),
		m_pAttributes(), 
		m_pShaderExecEnv(IqShaderExecEnv::create(QGetRenderContextI()))
{
	m_pAttributes = QGetRenderContext()->pattrCurrent();
	m_pShader->SetType(Type_Imager);
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqImagersource::~CqImagersource()
{
}

//---------------------------------------------------------------------
void CqImagersource::Initialise( const CqRegion& DRegion, IqChannelBuffer* buffer )
{
	AQSIS_TIME_SCOPE(Imager_shading);

	// We use one less than the bucket width and height here, since these
	// resolutions really represent one less than the number of shaded points
	// in each direction.  (Usually they describe the number of micropolygons
	// on a grid which is one less than the number of shaded vertices.  This
	// concept has no real analogue in context of an imager shader.)
	TqInt uGridRes = DRegion.width()-1;
	TqInt vGridRes = DRegion.height()-1;
	TqInt x = DRegion.xMin();
	TqInt y = DRegion.yMin();

	m_uYOrigin = static_cast<TqInt>( y );
	m_uXOrigin = static_cast<TqInt>( x );
	m_uGridRes = uGridRes;
	m_vGridRes = vGridRes;

	TqInt mode = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "System", "DisplayMode" ) [ 0 ];
	TqFloat components;
	TqInt j, i;
	TqFloat shuttertime = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "Shutter" ) [ 0 ];

	components = mode & DMode_RGB ? 3 : 0;
	components += mode & DMode_A ? 1 : 0;
	components = mode & DMode_Z ? 1 : components;

	TqInt Uses = ( 1 << EnvVars_P ) | ( 1 << EnvVars_Ci ) | ( 1 << EnvVars_Oi | ( 1 << EnvVars_ncomps ) | ( 1 << EnvVars_time ) | ( 1 << EnvVars_alpha ) | ( 1 << EnvVars_s ) | ( 1 << EnvVars_t ) );

	m_pShaderExecEnv->Initialise( uGridRes, vGridRes, uGridRes * vGridRes, (uGridRes+1)*(vGridRes+1), true, IqAttributesPtr(), IqTransformPtr(), m_pShader.get(), Uses );

	// Initialise the geometric parameters in the shader exec env.

	TqInt numShadingPoints = (uGridRes+1) * (vGridRes+1);
	P() ->Initialise( numShadingPoints );
	Ci() ->Initialise( numShadingPoints );
	Oi() ->Initialise( numShadingPoints );
	alpha() ->Initialise( numShadingPoints );
	s() ->Initialise( numShadingPoints );
	t() ->Initialise( numShadingPoints );

	//TODO dtime is not initialised yet
	//dtime().Initialise(uGridRes, vGridRes, i);

	ncomps() ->SetFloat( components );
	time() ->SetFloat( shuttertime );

	m_pShader->Initialise( uGridRes, vGridRes, (uGridRes+1)*(vGridRes+1), m_pShaderExecEnv.get() );

	TqUint CiIndex = buffer->getChannelIndex("Ci");
	TqUint OiIndex = buffer->getChannelIndex("Oi");
	TqUint coverageIndex = buffer->getChannelIndex("coverage");
	for ( j = 0; j < vGridRes+1; j++ )
	{
		for ( i = 0; i < uGridRes+1; i++ )
		{
			TqInt off = j * ( uGridRes + 1 ) + i;
			P() ->SetPoint( CqVector3D( x + i, y + j, 0.0 ), off );
			Ci() ->SetColor( CqColor((*buffer)(i, j, CiIndex)[0], (*buffer)(i, j, CiIndex)[1], (*buffer)(i, j, CiIndex)[2]), off );
			CqColor opa((*buffer)(i, j, OiIndex)[0], (*buffer)(i, j, OiIndex)[1], (*buffer)(i, j, OiIndex)[2]);
			Oi() ->SetColor( opa, off );
			TqFloat avopa = ( opa.r() + opa.g() + opa.b() ) /3.0f;
			alpha() ->SetFloat( (*buffer)(i, j, coverageIndex)[0] * avopa, off );
			s() ->SetFloat( x + i + 0.5, off );
			t() ->SetFloat( y + j + 0.5, off );
		}
	}
	// Execute the Shader VM
	if ( m_pShader )
	{
		m_pShader->Evaluate( m_pShaderExecEnv.get() );
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

} // namespace Aqsis



