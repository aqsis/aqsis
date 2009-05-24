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
\brief Implements latlong/cubic environment handling.
\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	<aqsis/aqsis.h>

#include	<cstring>
#include	<iostream>
#include	<fstream>

#include	"texturemap_old.h"
#include	<aqsis/math/random.h>
#include	<aqsis/version.h>
#include	"renderer.h"
#include	<aqsis/util/logging.h>

#ifndef AQSIS_SYSTEM_WIN32
#include	"unistd.h"
#endif

namespace Aqsis {


//----------------------------------------------------------------------
/** Check if a texture map exists in the cache, return a pointer to it if so, else
 * load it if possible..
 */

IqTextureMapOld* CqTextureMapOld::GetLatLongMap( const CqString& strName )
{
	QGetRenderContext() ->Stats().IncTextureMisses( 2 );

	TqUlong hash = CqString::hash(strName.c_str());

	// First search the texture map cache
	for ( std::vector<CqTextureMapOld*>::iterator i = m_TextureMap_Cache.begin(); i != m_TextureMap_Cache.end(); i++ )
	{
		if ( ( *i ) ->m_hash == hash )
		{
			if ( ( *i ) ->Type() == MapType_LatLong )
			{
				QGetRenderContext() ->Stats().IncTextureHits( 1, 2 );
				return ( *i );
			} else
			{
				return NULL;
			}
		}
	}

	QGetRenderContext() ->Stats().IncTextureHits( 0, 2 );

	// If we got here, it doesn't exist yet, so we must create and load it.
	CqTextureMapOld* pNew = new CqLatLongMapOld( strName );
	m_TextureMap_Cache.push_back( pNew );
	pNew->Open();

	TqPchar ptexfmt;

	// Invalid only if this is not a LatLong Env. map file
	if ( pNew->m_pImage == 0 ||
	        TIFFGetField( pNew->m_pImage, TIFFTAG_PIXAR_TEXTUREFORMAT, &ptexfmt ) != 1 ||
	        strcmp( ptexfmt, LATLONG_HEADER ) != 0 )
	{
		static bool done = false;
		if (!done)
		{
			Aqsis::log() << error << "Map \"" << strName.c_str() << "\" is not an environment map, use RiMakeLatLongEnvironment" << std::endl;
			done = true;
		}

		pNew->SetInvalid();
   	}
	return ( pNew );
}

//----------------------------------------------------------------------
/** Check if a texture map exists in the cache, return a pointer to it if so, else
 * load it if possible..
 */

IqTextureMapOld* CqTextureMapOld::GetEnvironmentMap( const CqString& strName )
{
	QGetRenderContext() ->Stats().IncTextureMisses( 1 );

	TqUlong hash = CqString::hash(strName.c_str());

	// First search the texture map cache
	for ( std::vector<CqTextureMapOld*>::iterator i = m_TextureMap_Cache.begin(); i != m_TextureMap_Cache.end(); i++ )
	{
		if ( ( *i ) ->m_hash == hash )
		{
			if ( ( *i ) ->Type() == MapType_Environment )
			{
				QGetRenderContext() ->Stats().IncTextureHits( 1, 1 );
				return ( *i );
			} else
			{
				return NULL;
			}
		}
	}

	QGetRenderContext() ->Stats().IncTextureHits( 0, 1 );

	// If we got here, it doesn't exist yet, so we must create and load it.
	CqTextureMapOld* pNew = new CqEnvironmentMapOld( strName );
	m_TextureMap_Cache.push_back( pNew );
	pNew->Open();

	TqPchar ptexfmt = 0;

	// Invalid if the m_pImage is not there or it is not cube or latlong env. map file
	if ( pNew->m_pImage == 0 ||
	        TIFFGetField( pNew->m_pImage, TIFFTAG_PIXAR_TEXTUREFORMAT, &ptexfmt ) != 1 ||
	        ( strcmp( ptexfmt, CUBEENVMAP_HEADER ) != 0 ) && ( strcmp( ptexfmt, LATLONG_HEADER ) != 0 ) )
	{
		static bool done = false;
		if (!done)
		{
			Aqsis::log() << error << "Map \"" << strName.c_str() << "\" is not an environment map, use RiMakeCubeFaceEnvironment" << std::endl;
			done = true;
		}
		pNew->SetInvalid();
		delete pNew;
		pNew = NULL;

	}
	else
	{
		TqFloat fov;
		if (TIFFGetField( pNew->m_pImage, TIFFTAG_PIXAR_FOVCOT, &fov) == 1)
			((CqEnvironmentMapOld *)pNew)->SetFov(fov);
		else
			((CqEnvironmentMapOld *)pNew)->SetFov(1.0);
	}

	// remove from the list a LatLong env. map since in shadeops.cpp we will cope with it.
	if ( ptexfmt && strcmp( ptexfmt, LATLONG_HEADER ) == 0 )
	{
		pNew->SetInvalid();
		delete pNew;
		pNew = NULL;
   	} 
   
	return ( pNew );
}

//----------------------------------------------------------------------
/** Retrieve a color sample from the environment map using R as the reflection vector.
 * Filtering is done using swidth, twidth and nsamples.
 */

void CqEnvironmentMapOld::SampleMap( CqVector3D& R1,
                                  CqVector3D& swidth, CqVector3D& twidth,
                                  std::valarray<TqFloat>& val, TqInt index,
                                  TqFloat* average_depth, TqFloat* shadow_depth )
{
	// Check the memory and make sure we don't abuse it
	CriticalMeasure();

	if ( m_pImage != 0 )
	{

		if ( Type() != MapType_LatLong )
		{
			CqVector3D	R2, R3, R4;
			R2 = R1 + swidth;
			R3 = R1 + twidth;
			R4 = R1 + swidth + twidth;
			SampleMap( R1, R2, R3, R4, val );
		}
		else if ( Type() == MapType_LatLong )
		{
			CqVector3D V = R1;
			V.Unit();
			TqFloat sswidth = swidth.Magnitude();
			TqFloat stwidth = twidth.Magnitude();

			TqFloat ss1 = atan2( V.y(), V.x() ) / ( 2.0 * RI_PI );  // -.5 -> .5
			ss1 = ss1 + 0.5; // remaps to 0 -> 1
			TqFloat tt1 = acos( V.z() ) / RI_PI;

			CqTextureMapOld::SampleMap( ss1, tt1, sswidth/RI_PI, stwidth/RI_PI, val );
		}
	}
}

} // namespace Aqsis

