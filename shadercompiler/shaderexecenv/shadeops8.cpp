// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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
		\brief Implements the basic shader operations. (interrogation like surface(), atmosphere(), ... related)
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	"aqsis.h"

#include	<math.h>
#include	<map>
#include	<vector>
#include	<string>
#include	<stdio.h>

#if defined(REGEXP)
#include        <regex.h>
#endif

#include	"shaderexecenv.h"
#include	"shadervm.h"
#include	"irenderer.h"
#include	"ilightsource.h"
#include	"version.h"
#include	"logging.h"

START_NAMESPACE(    Aqsis )

IqRenderer* QGetRenderContextI();

static TqFloat match(const char *string, const char *pattern);

//----------------------------------------------------------------------
// atmosphere
//

void CqShaderExecEnv::SO_atmosphere( IqShaderData* name, IqShaderData* pV, IqShaderData* Result, IqShader* pShader )
{
	TqUint __iGrid;

	boost::shared_ptr<IqShader> pAtmosphere;

	if ( NULL != m_pAttributes && (m_pAttributes ->pshadAtmosphere(QGetRenderContextI()->Time())) )
		pAtmosphere = m_pAttributes ->pshadAtmosphere(QGetRenderContextI()->Time());

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	if ( pAtmosphere )
		Result->SetValue( pAtmosphere->GetVariableValue( _aq_name.c_str(), pV ) ? 1.0f : 0.0f, 0 );
	else
		Result->SetValue( 0.0f, 0 );

}


//----------------------------------------------------------------------
// displacement
//

void CqShaderExecEnv::SO_displacement( IqShaderData* name, IqShaderData* pV, IqShaderData* Result, IqShader* pShader )
{
	TqUint __iGrid;

	boost::shared_ptr<IqShader> pDisplacement;

	if ( NULL != m_pAttributes && (m_pAttributes ->pshadDisplacement(QGetRenderContextI()->Time())) )
		pDisplacement = m_pAttributes ->pshadDisplacement(QGetRenderContextI()->Time());

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	if ( pDisplacement )
		Result->SetValue( pDisplacement->GetVariableValue( _aq_name.c_str(), pV ) ? 1.0f : 0.0f, 0 );
	else
		Result->SetValue( 0.0f, 0 );

}


//----------------------------------------------------------------------
// lightsource
//

void CqShaderExecEnv::SO_lightsource( IqShaderData* name, IqShaderData* pV, IqShaderData* Result, IqShader* pShader )
{
	TqUint __iGrid;

	// This should only be called within an Illuminance construct, so m_li should be valid.
	boost::shared_ptr<IqShader> pLightsource;

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	if ( m_li < m_pAttributes ->cLights() )
		pLightsource = m_pAttributes ->pLight( m_li ) ->pShader();
	if ( pLightsource )
		Result->SetValue( pLightsource->GetVariableValue( _aq_name.c_str(), pV ) ? 1.0f : 0.0f, 0 );
	else
		Result->SetValue( 0.0f, 0 );

}


//----------------------------------------------------------------------
// surface
//

void CqShaderExecEnv::SO_surface( IqShaderData* name, IqShaderData* pV, IqShaderData* Result, IqShader* pShader )
{
	TqUint __iGrid;

	boost::shared_ptr<IqShader> pSurface;

	if ( GetCurrentSurface() &&
	        NULL != GetCurrentSurface()->pAttributes() &&
	        GetCurrentSurface()->pAttributes() ->pshadSurface(QGetRenderContextI()->Time()) )
		pSurface = GetCurrentSurface()->pAttributes() ->pshadSurface(QGetRenderContextI()->Time());

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	if ( pSurface )
		Result->SetValue( pSurface->GetVariableValue( _aq_name.c_str(), pV ) ? 1.0f : 0.0f, 0 );
	else
		Result->SetValue( 0.0f, 0 );

}


//----------------------------------------------------------------------
// attribute
//

void CqShaderExecEnv::SO_attribute( IqShaderData* name, IqShaderData* pV, IqShaderData* Result, IqShader* pShader )
{
	TqUint __iGrid;

	//Find out if it is a specific attribute request
	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	TqFloat Ret = 0.0f;

	if ( _aq_name.compare( "ShadingRate" ) == 0 )
	{
		if ( pV->Type() == type_float )
		{
			pV->SetFloat( m_pAttributes ->GetFloatAttribute( "System", "ShadingRate" ) [ 0 ] );
			Ret = 1.0f;
		}
	}
	else if ( _aq_name.compare( "Sides" ) == 0 )
	{
		if ( pV->Type() == type_float )
		{
			pV->SetFloat( m_pAttributes ->GetIntegerAttribute( "System", "Sides" ) [ 0 ] );
			Ret = 1.0f;
		}
	}
	else if ( _aq_name.compare( "Matte" ) == 0 )
	{
		if ( pV->Type() == type_float )
		{
			pV->SetFloat( m_pAttributes ->GetIntegerAttribute( "System", "Matte" ) [ 0 ] );
			Ret = 1.0f;
		}
	}
	else
	{
		int iColon = _aq_name.find_first_of( ':' );
		if ( iColon >= 0 )
		{
			CqString strParam = _aq_name.substr( iColon + 1, _aq_name.size() - iColon - 1 );
			_aq_name = _aq_name.substr( 0, iColon );
			//const CqParameter* pParam = m_pAttributes ->pParameter( STRING( name ).c_str(), strParam.c_str() );

			Ret = 1.0f;
			if ( NULL != pAttributes() ->GetFloatAttribute( _aq_name.c_str(), strParam.c_str() ) )
				pV->SetFloat( pAttributes() ->GetFloatAttribute( _aq_name.c_str(), strParam.c_str() ) [ 0 ] );
			else if ( NULL != pAttributes() ->GetIntegerAttribute( _aq_name.c_str(), strParam.c_str() ) )
				pV->SetFloat( pAttributes() ->GetIntegerAttribute( _aq_name.c_str(), strParam.c_str() ) [ 0 ] );
			else if ( NULL != pAttributes() ->GetStringAttribute( _aq_name.c_str(), strParam.c_str() ) )
				pV->SetString( pAttributes() ->GetStringAttribute( _aq_name.c_str(), strParam.c_str() ) [ 0 ] );
			else if ( NULL != pAttributes() ->GetPointAttribute( _aq_name.c_str(), strParam.c_str() ) )
				pV->SetPoint( pAttributes() ->GetPointAttribute( _aq_name.c_str(), strParam.c_str() ) [ 0 ] );
			else if ( NULL != pAttributes() ->GetVectorAttribute( _aq_name.c_str(), strParam.c_str() ) )
				pV->SetVector( pAttributes() ->GetVectorAttribute( _aq_name.c_str(), strParam.c_str() ) [ 0 ] );
			else if ( NULL != pAttributes() ->GetNormalAttribute( _aq_name.c_str(), strParam.c_str() ) )
				pV->SetNormal( pAttributes() ->GetNormalAttribute( _aq_name.c_str(), strParam.c_str() ) [ 0 ] );
			else if ( NULL != pAttributes() ->GetColorAttribute( _aq_name.c_str(), strParam.c_str() ) )
				pV->SetColor( pAttributes() ->GetColorAttribute( _aq_name.c_str(), strParam.c_str() ) [ 0 ] );
			else if ( NULL != pAttributes() ->GetMatrixAttribute( _aq_name.c_str(), strParam.c_str() ) )
				pV->SetMatrix( pAttributes() ->GetMatrixAttribute( _aq_name.c_str(), strParam.c_str() ) [ 0 ] );
			else
				Ret = 0.0f;
		}
	}
	Result->SetValue( Ret, 0 );

}


//----------------------------------------------------------------------
// option
//

void CqShaderExecEnv::SO_option( IqShaderData* name, IqShaderData* pV, IqShaderData* Result, IqShader* pShader )
{
	TqUint __iGrid;

	if ( NULL == QGetRenderContextI() )
		return ;

	__iGrid = 0;
	//Find out if it is a specific option request
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	TqFloat Ret = 0.0f;

	if ( _aq_name.compare( "Format" ) == 0 )
	{
		if ( pV->Type() == type_float &&
		        pV->ArrayLength() > 0 )
		{
			if ( pV->ArrayLength() >= 3 )
			{
				pV->ArrayEntry( 0 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetIntegerOption( "System", "Resolution" ) [ 0 ] ) );
				pV->ArrayEntry( 1 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetIntegerOption( "System", "Resolution" ) [ 1 ] ) );
				pV->ArrayEntry( 2 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "PixelAspectRatio" ) [ 2 ] ) );
				Ret = 1.0f;
			}
		}
	}
	else if ( _aq_name.compare( "CropWindow" ) == 0 )
	{
		if ( pV->Type() == type_float &&
		        pV->ArrayLength() > 0 )
		{
			if ( pV->ArrayLength() >= 4 )
			{
				pV->ArrayEntry( 0 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "CropWindow" ) [ 0 ] ) );
				pV->ArrayEntry( 1 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "CropWindow" ) [ 1 ] ) );
				pV->ArrayEntry( 2 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "CropWindow" ) [ 2 ] ) );
				pV->ArrayEntry( 3 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "CropWindow" ) [ 3 ] ) );
				Ret = 1.0f;
			}
		}
	}
	else if ( _aq_name.compare( "FrameAspectRatio" ) == 0 )
	{
		if ( pV->Type() == type_float )
		{
			pV->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "FrameAspectRatio" ) [ 0 ] ) );
			Ret = 1.0f;
		}
	}
	else if ( _aq_name.compare( "DepthOfField" ) == 0 )
	{
		if ( pV->Type() == type_float &&
		        pV->ArrayLength() > 0 )
		{
			if ( pV->ArrayLength() >= 3 )
			{
				pV->ArrayEntry( 0 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "DepthOfField" ) [ 0 ] ) );
				pV->ArrayEntry( 1 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "DepthOfField" ) [ 1 ] ) );
				pV->ArrayEntry( 2 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "DepthOfField" ) [ 2 ] ) );
				Ret = 1.0f;
			}
		}
	}
	else if ( _aq_name.compare( "Shutter" ) == 0 )
	{
		if ( pV->Type() == type_float &&
		        pV->ArrayLength() > 0 )
		{
			if ( pV->ArrayLength() >= 2 )
			{
				pV->ArrayEntry( 0 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "Shutter" ) [ 0 ] ) );
				pV->ArrayEntry( 1 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "Shutter" ) [ 1 ] ) );
				Ret = 1.0f;
			}
		}
	}
	else if ( _aq_name.compare( "Clipping" ) == 0 )
	{
		if ( pV->Type() == type_float &&
		        pV->ArrayLength() > 0 )
		{
			if ( pV->ArrayLength() >= 2 )
			{
				pV->ArrayEntry( 0 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "Clipping" ) [ 0 ] ) );
				pV->ArrayEntry( 1 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "Clipping" ) [ 1 ] ) );
				Ret = 1.0f;
			}
		}
	}
	else
	{
		CqString strName = _aq_name.c_str();
		int iColon = strName.find_first_of( ':' );
		if ( iColon >= 0 )
		{
			CqString strParam = strName.substr( iColon + 1, strName.size() - iColon - 1 );
			strName = strName.substr( 0, iColon );
			//const CqParameter* pParam = m_pAttributes ->pParameter( strName.c_str(), strParam.c_str() );

			Ret = 1.0f;

			if ( NULL != QGetRenderContextI() ->GetStringOption( strName.c_str(), strParam.c_str() ) )
				pV->SetString( QGetRenderContextI() ->GetStringOption( strName.c_str(), strParam.c_str() ) [ 0 ] );
			else if ( NULL != QGetRenderContextI() ->GetIntegerOption( strName.c_str(), strParam.c_str() ) )
				pV->SetFloat( QGetRenderContextI() ->GetIntegerOption( strName.c_str(), strParam.c_str() ) [ 0 ] );

			else if ( NULL != QGetRenderContextI() ->GetPointOption( strName.c_str(), strParam.c_str() ) )
				pV->SetPoint( QGetRenderContextI() ->GetPointOption( strName.c_str(), strParam.c_str() ) [ 0 ] );

			else if ( NULL != QGetRenderContextI() ->GetColorOption( strName.c_str(), strParam.c_str() ) )
				pV->SetColor( QGetRenderContextI() ->GetColorOption( strName.c_str(), strParam.c_str() ) [ 0 ] );
			else if ( NULL != QGetRenderContextI() ->GetFloatOption( strName.c_str(), strParam.c_str() ) )
				pV->SetFloat( QGetRenderContextI() ->GetFloatOption( strName.c_str(), strParam.c_str() ) [ 0 ] );
			/* did not deal with Vector, Normal and Matrix yet */
			else
				Ret = 0.0f;
		}
	}

	Result->SetValue( Ret, 0 );

}


//----------------------------------------------------------------------
// rendererinfo
//

void CqShaderExecEnv::SO_rendererinfo( IqShaderData* name, IqShaderData* pV, IqShaderData* Result, IqShader* pShader )
{
	TqUint __iGrid;

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	TqFloat Ret = 0.0f;

	if ( _aq_name.compare( "renderer" ) == 0 )
	{
		if ( pV->Type() == type_string )
		{
			pV->SetString( STRNAME );
			Ret = 1.0f;
		}
	}
	else if ( _aq_name.compare( "version" ) == 0 )
	{
		if ( pV->Type() == type_float &&
		        pV->ArrayLength() > 0 )
		{
			if ( pV->ArrayLength() >= 4 )
			{
				pV->ArrayEntry( 0 ) ->SetFloat( static_cast<TqFloat>( VERMAJOR ) );
				pV->ArrayEntry( 1 ) ->SetFloat( static_cast<TqFloat>( VERMINOR ) );
				pV->ArrayEntry( 2 ) ->SetFloat( static_cast<TqFloat>( BUILD ) );
				pV->ArrayEntry( 3 ) ->SetFloat( 0.0f );
				Ret = 1.0f;
			}
		}
	}
	else if ( _aq_name.compare( "versionstring" ) == 0 )
	{
		if ( pV->Type() == type_string )
		{
			pV->SetString( VERSION_STR );
			Ret = 1.0f;
		}
	}
	Result->SetValue( Ret, 0 );

}


void	CqShaderExecEnv::SO_match( IqShaderData* a, IqShaderData* b, IqShaderData* Result, IqShader* pShader )
{
	// TODO: Do this properly.
	TqUint __iGrid;

	__iGrid = 0;
	float r = 0.0f;
	CqString _aq_a;
	(a)->GetString(_aq_a,__iGrid);
	CqString _aq_b;
	(b)->GetString(_aq_b,__iGrid);
	if ( _aq_a.size() == 0 )
		r = 0.0f;
	else if ( _aq_b.size() == 0 )
		r = 0.0f;
	else
	{
		// Check the simple case first where both strings are identical
		TqUlong hasha = CqString::hash(_aq_a.c_str());
		TqUlong hashb = CqString::hash(_aq_b.c_str());

		if (hasha == hashb)
		{
			r = 1.0f;
		}
		else
		{
			/*
			* Match string b into a
			*/
			r = match(_aq_a.c_str(), _aq_b.c_str());
		}


	}

	(Result)->SetFloat(r,__iGrid);

}

static TqFloat match(const char *string, const char *pattern)
{
#if defined(REGEXP)
	int status;
	regex_t re;
	if (regcomp(&re, pattern, REG_EXTENDED|REG_NOSUB) != 0)
	{
		return(0.0f);      /* report error */
	}
	status = regexec(&re, string, (size_t) 0, NULL, 0);
	regfree(&re);

	if (status != 0)
	{
		return(0.0f);      /* report error */
	}
	return(1.0f);
#else

	return (TqFloat) (strstr(string, pattern) != 0);
#endif
}

//----------------------------------------------------------------------
// shadername()
void	CqShaderExecEnv::SO_shadername( IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying;
	TqUint __iGrid;

	__fVarying=(Result)->Class()==class_varying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			(Result)->SetString(pShader->strName(),__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


//----------------------------------------------------------------------
// shadername(s)
void	CqShaderExecEnv::SO_shadername2( IqShaderData* shader, IqShaderData* Result, IqShader* pShader )
{
	TqBool __fVarying;
	TqUint __iGrid;

	CqString strName( "" );
	CqString strShader;
	boost::shared_ptr<IqShader> pSurface;
	boost::shared_ptr<IqShader> pDisplacement;
	boost::shared_ptr<IqShader> pAtmosphere;
	if( m_pAttributes )
	{
		pSurface = m_pAttributes ->pshadSurface(QGetRenderContextI()->Time());
		pDisplacement = m_pAttributes ->pshadDisplacement(QGetRenderContextI()->Time());
		pAtmosphere = m_pAttributes ->pshadAtmosphere(QGetRenderContextI()->Time());
	}

	__fVarying=(Result)->Class()==class_varying;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			strName = "";
			CqString _aq_shader;
			(shader)->GetString(_aq_shader,__iGrid);
			if ( _aq_shader.compare( "surface" ) == 0 && pSurface )
				strName = pSurface->strName();
			else if ( _aq_shader.compare( "displacement" ) == 0 && pDisplacement )
				strName = pDisplacement->strName();
			else if ( _aq_shader.compare( "atmosphere" ) == 0 && pAtmosphere )
				strName = pAtmosphere->strName();
			(Result)->SetString(strName,__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}



END_NAMESPACE(    Aqsis )
//---------------------------------------------------------------------
