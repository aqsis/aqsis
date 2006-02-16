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
		\brief Implements classes and support structures for the shader execution environment.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	"aqsis.h"
#include	"ishaderexecenv.h"
#include	"surface.h"

#include	"shadervm.h"

START_NAMESPACE( Aqsis )

char* gVariableTypeNames[] =
    {
        "invalid",
        "float",
        "integer",
        "point",
        "string",
        "color",
        "triple",
        "hpoint",
        "normal",
        "vector",
        "void",
        "matrix",
        "sixteentuple",
    };
TqInt gcVariableTypeNames = sizeof( gVariableTypeNames ) / sizeof( gVariableTypeNames[ 0 ] );

char* gVariableClassNames[] =
    {
        "invalid",
        "constant",
        "uniform",
        "varying",
        "vertex",
        "facevarying",
    };
TqInt gcVariableClassNames = sizeof( gVariableClassNames ) / sizeof( gVariableClassNames[ 0 ] );


CqNoise	CqShaderExecEnv::m_noise;
CqCellNoise	CqShaderExecEnv::m_cellnoise;
CqRandom	CqShaderExecEnv::m_random;
CqMatrix	CqShaderExecEnv::m_matIdentity;

char*	gVariableNames[ EnvVars_Last ] =
    {
        "Cs",
        "Os",
        "Ng",
        "du",
        "dv",
        "L",
        "Cl",
        "Ol",
        "P",
        "dPdu",
        "dPdv",
        "N",
        "u",
        "v",
        "s",
        "t",
        "I",
        "Ci",
        "Oi",
        "Ps",
        "E",
        "ncomps",
        "time",
        "alpha",
        "Ns",
    };
TqUlong	gVariableTokens[ EnvVars_Last ] =
    {
        CqString::hash( gVariableNames[ 0 ] ),
        CqString::hash( gVariableNames[ 1 ] ),
        CqString::hash( gVariableNames[ 2 ] ),
        CqString::hash( gVariableNames[ 3 ] ),
        CqString::hash( gVariableNames[ 4 ] ),
        CqString::hash( gVariableNames[ 5 ] ),
        CqString::hash( gVariableNames[ 6 ] ),
        CqString::hash( gVariableNames[ 7 ] ),
        CqString::hash( gVariableNames[ 8 ] ),
        CqString::hash( gVariableNames[ 9 ] ),
        CqString::hash( gVariableNames[ 10 ] ),
        CqString::hash( gVariableNames[ 11 ] ),
        CqString::hash( gVariableNames[ 12 ] ),
        CqString::hash( gVariableNames[ 13 ] ),
        CqString::hash( gVariableNames[ 14 ] ),
        CqString::hash( gVariableNames[ 15 ] ),
        CqString::hash( gVariableNames[ 16 ] ),
        CqString::hash( gVariableNames[ 17 ] ),
        CqString::hash( gVariableNames[ 18 ] ),
        CqString::hash( gVariableNames[ 19 ] ),
        CqString::hash( gVariableNames[ 20 ] ),
        CqString::hash( gVariableNames[ 21 ] ),
        CqString::hash( gVariableNames[ 22 ] ),
        CqString::hash( gVariableNames[ 23 ] ),
        CqString::hash( gVariableNames[ 24 ] ),
    };


// TODO: See if we can reduce these default requires further!
TqInt	gDefUses = ( 1 << EnvVars_P ) | ( 1 << EnvVars_I ) | ( 1 << EnvVars_N ) | ( 1 << EnvVars_Ng ) | ( 1 << EnvVars_L ) | ( 1 << EnvVars_Cl ) | ( 1 << EnvVars_Ci ) | ( 1 << EnvVars_Oi ) | ( 1 << EnvVars_u ) | ( 1 << EnvVars_v );
TqInt	gDefLightUses = ( 1 << EnvVars_P ) | ( 1 << EnvVars_L ) | ( 1 << EnvVars_Ps ) | ( 1 << EnvVars_Ns );

//----------------------------------------------------------------------
/** Constructor.
 */

CqShaderExecEnv::CqShaderExecEnv() : m_li( 0 ), m_Illuminate( 0 ), m_pAttributes( 0 ), m_LocalIndex( 0 )
{
	m_apVariables.resize( EnvVars_Last );
	TqInt i;
	for ( i = 0; i < EnvVars_Last; i++ )
		m_apVariables[ i ] = 0;
}


//----------------------------------------------------------------------
/** Destructor.
 */

CqShaderExecEnv::~CqShaderExecEnv()
{
	TqInt i;
	for ( i = 0; i < EnvVars_Last; i++ )
		delete( m_apVariables[ i ] );

	if ( m_pAttributes )
		RELEASEREF( m_pAttributes );
}

//---------------------------------------------------------------------
/** Initialise variables to correct size for current grid.
 */

void CqShaderExecEnv::Initialise( const TqInt uGridRes, const TqInt vGridRes, IqAttributes* pAttr, const boost::shared_ptr<IqTransform>& pTrans, IqShader* pShader, TqInt Uses )
{
	m_uGridRes = uGridRes;
	m_vGridRes = vGridRes;

	m_GridSize = ( uGridRes + 1 ) * ( vGridRes + 1 );
	m_LocalIndex = 0;

	// Store a pointer to the attributes definition.
	if ( NULL != pAttr )
	{
		if( NULL != m_pAttributes )
			RELEASEREF(m_pAttributes);
		m_pAttributes = pAttr;
		ADDREF(m_pAttributes);
	}
	else
		m_pAttributes = NULL;

	// Store a pointer to the transform.
	if (pTrans)
	{
		m_pTransform = boost::static_pointer_cast<CqTransform>(pTrans);
	}

	m_li = 0;
	m_Illuminate = 0;
	m_IlluminanceCacheValid = TqFalse;

	// Initialise the state bitvectors
	m_CurrentState.SetSize( m_GridSize );
	m_RunningState.SetSize( m_GridSize );
	m_RunningState.SetAll( TqTrue );


	if ( pShader )
	{
		if ( USES( Uses, EnvVars_P ) && m_apVariables[ EnvVars_P ] == 0 )
			m_apVariables[ EnvVars_P ] = pShader->CreateVariable( type_point, class_varying, gVariableNames[ EnvVars_P ] );
		if ( USES( Uses, EnvVars_Cs ) && m_apVariables[ EnvVars_Cs ] == 0 )
			m_apVariables[ EnvVars_Cs ] = pShader->CreateVariable( type_color, class_varying, gVariableNames[ EnvVars_Cs ] );
		if ( USES( Uses, EnvVars_Os ) && m_apVariables[ EnvVars_Os ] == 0 )
			m_apVariables[ EnvVars_Os ] = pShader->CreateVariable( type_color, class_varying, gVariableNames[ EnvVars_Os ] );
		if ( USES( Uses, EnvVars_Ng ) && m_apVariables[ EnvVars_Ng ] == 0 )
			m_apVariables[ EnvVars_Ng ] = pShader->CreateVariable( type_normal, class_varying, gVariableNames[ EnvVars_Ng ] );
		if ( USES( Uses, EnvVars_du ) && m_apVariables[ EnvVars_du ] == 0 )
			m_apVariables[ EnvVars_du ] = pShader->CreateVariable( type_float, class_varying, gVariableNames[ EnvVars_du ] );
		if ( USES( Uses, EnvVars_dv ) && m_apVariables[ EnvVars_dv ] == 0 )
			m_apVariables[ EnvVars_dv ] = pShader->CreateVariable( type_float, class_varying, gVariableNames[ EnvVars_dv ] );
		if ( USES( Uses, EnvVars_L ) && m_apVariables[ EnvVars_L ] == 0 )
			m_apVariables[ EnvVars_L ] = pShader->CreateVariable( type_vector, class_varying, gVariableNames[ EnvVars_L ] );
		if ( USES( Uses, EnvVars_Cl ) && m_apVariables[ EnvVars_Cl ] == 0 )
			m_apVariables[ EnvVars_Cl ] = pShader->CreateVariable( type_color, class_varying, gVariableNames[ EnvVars_Cl ] );
		if ( USES( Uses, EnvVars_Ol ) && m_apVariables[ EnvVars_Ol ] == 0 )
			m_apVariables[ EnvVars_Ol ] = pShader->CreateVariable( type_color, class_varying, gVariableNames[ EnvVars_Ol ] );
		if ( USES( Uses, EnvVars_dPdu ) && m_apVariables[ EnvVars_dPdu ] == 0 )
			m_apVariables[ EnvVars_dPdu ] = pShader->CreateVariable( type_vector, class_varying, gVariableNames[ EnvVars_dPdu ] );
		if ( USES( Uses, EnvVars_dPdv ) && m_apVariables[ EnvVars_dPdv ] == 0 )
			m_apVariables[ EnvVars_dPdv ] = pShader->CreateVariable( type_vector, class_varying, gVariableNames[ EnvVars_dPdv ] );
		if ( USES( Uses, EnvVars_N ) && m_apVariables[ EnvVars_N ] == 0 )
			m_apVariables[ EnvVars_N ] = pShader->CreateVariable( type_normal, class_varying, gVariableNames[ EnvVars_N ] );
		if ( USES( Uses, EnvVars_u ) && m_apVariables[ EnvVars_u ] == 0 )
			m_apVariables[ EnvVars_u ] = pShader->CreateVariable( type_float, class_varying, gVariableNames[ EnvVars_u ] );
		if ( USES( Uses, EnvVars_v ) && m_apVariables[ EnvVars_v ] == 0 )
			m_apVariables[ EnvVars_v ] = pShader->CreateVariable( type_float, class_varying, gVariableNames[ EnvVars_v ] );
		if ( USES( Uses, EnvVars_s ) && m_apVariables[ EnvVars_s ] == 0 )
			m_apVariables[ EnvVars_s ] = pShader->CreateVariable( type_float, class_varying, gVariableNames[ EnvVars_s ] );
		if ( USES( Uses, EnvVars_t ) && m_apVariables[ EnvVars_t ] == 0 )
			m_apVariables[ EnvVars_t ] = pShader->CreateVariable( type_float, class_varying, gVariableNames[ EnvVars_t ] );
		if ( USES( Uses, EnvVars_I ) && m_apVariables[ EnvVars_I ] == 0 )
			m_apVariables[ EnvVars_I ] = pShader->CreateVariable( type_vector, class_varying, gVariableNames[ EnvVars_I ] );
		if ( USES( Uses, EnvVars_Ci ) && m_apVariables[ EnvVars_Ci ] == 0 )
			m_apVariables[ EnvVars_Ci ] = pShader->CreateVariable( type_color, class_varying, gVariableNames[ EnvVars_Ci ] );
		if ( USES( Uses, EnvVars_Oi ) && m_apVariables[ EnvVars_Oi ] == 0 )
			m_apVariables[ EnvVars_Oi ] = pShader->CreateVariable( type_color, class_varying, gVariableNames[ EnvVars_Oi ] );
		if ( USES( Uses, EnvVars_Ps ) && m_apVariables[ EnvVars_Ps ] == 0 )
			m_apVariables[ EnvVars_Ps ] = pShader->CreateVariable( type_point, class_varying, gVariableNames[ EnvVars_Ps ] );
		if ( USES( Uses, EnvVars_E ) && m_apVariables[ EnvVars_E ] == 0 )
			m_apVariables[ EnvVars_E ] = pShader->CreateVariable( type_point, class_uniform, gVariableNames[ EnvVars_E ] );
		if ( USES( Uses, EnvVars_ncomps ) && m_apVariables[ EnvVars_ncomps ] == 0 )
			m_apVariables[ EnvVars_ncomps ] = pShader->CreateVariable( type_float, class_uniform, gVariableNames[ EnvVars_ncomps ] );
		if ( USES( Uses, EnvVars_time ) && m_apVariables[ EnvVars_time ] == 0 )
			m_apVariables[ EnvVars_time ] = pShader->CreateVariable( type_float, class_uniform, gVariableNames[ EnvVars_time ] );
		if ( USES( Uses, EnvVars_alpha ) && m_apVariables[ EnvVars_alpha ] == 0 )
			m_apVariables[ EnvVars_alpha ] = pShader->CreateVariable( type_float, class_varying, gVariableNames[ EnvVars_alpha ] );
		if ( USES( Uses, EnvVars_Ns ) && m_apVariables[ EnvVars_Ns ] == 0 )
			m_apVariables[ EnvVars_Ns ] = pShader->CreateVariable( type_normal, class_varying, gVariableNames[ EnvVars_Ns ] );
	}

	TqInt i;
	for ( i = 0; i < EnvVars_Last; i++ )
	{
		if ( m_apVariables[ i ] && USES( Uses, i ) )
			m_apVariables[ i ] ->Initialise( uGridRes, vGridRes );
	}

	if( USES( Uses, EnvVars_time ) )
	{
		// First try setting this to the shutter open time
		// @todo: Think about an algorithm which distributes samples in time

		const TqFloat* shutter = QGetRenderContextI()->GetFloatOption( "System", "Shutter" );
		if( shutter )
		{
			const TqFloat* shutteroffset = QGetRenderContextI()->GetFloatOption( "shutter", "offset" );
			float offset = 0;
			if( shutteroffset != 0 )
			{
				offset = *shutteroffset;
			}

			// insert the open time plus shutter offset
			m_apVariables[ EnvVars_time ]->SetFloat(  shutter[ 0 ] + offset );
		}
	}
}

IqShaderData* CqShaderExecEnv::FindStandardVar( const char* pname )
{
	TqInt tmp = m_LocalIndex;
	TqUlong htoken = CqString::hash( pname );

	for ( ; m_LocalIndex < EnvVars_Last; m_LocalIndex++ )
	{
		if ( gVariableTokens[ m_LocalIndex ] == htoken )
			return ( m_apVariables[ m_LocalIndex ] );
	}

	for ( m_LocalIndex = 0; m_LocalIndex < tmp; m_LocalIndex++ )
	{
		if ( gVariableTokens[ m_LocalIndex ] == htoken )
			return ( m_apVariables[ m_LocalIndex ] );
	}
	return ( 0 );
}
TqInt	CqShaderExecEnv::FindStandardVarIndex( const char* pname )
{
	TqInt tmp = m_LocalIndex;
	TqUlong htoken = CqString::hash( pname );
	for ( ; m_LocalIndex < EnvVars_Last; m_LocalIndex++ )
	{
		if ( gVariableTokens[ m_LocalIndex ] == htoken )
			return ( m_LocalIndex );
	}

	for ( m_LocalIndex = 0; m_LocalIndex < tmp; m_LocalIndex++ )
	{
		if ( gVariableTokens[ m_LocalIndex ] == htoken )
			return ( m_LocalIndex );
	}
	return ( -1 );
}

//---------------------------------------------------------------------
/** Return the object to world matrix for the surface this shader is attached to, if
 * no surface (i.e. a lightsource) return identity.
 */

const CqMatrix& CqShaderExecEnv::matObjectToWorld() const
{
	//	return ( m_pSurface == 0 ? m_matIdentity : m_pSurface->pTransform() ->matObjectToWorld() );
	return ( !m_pTransform ? m_matIdentity : m_pTransform ->matObjectToWorld(m_pTransform->Time(0)) );
}


END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------
