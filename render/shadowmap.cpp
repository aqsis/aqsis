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
		\brief Implements shadowmap handling.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	"aqsis.h"

#include	<math.h>
#include	<iostream>
#include	<fstream>

#include	"texturemap.h"
#include	"random.h"
#include	"version.h"
#include	"renderer.h"

#ifndef		AQSIS_SYSTEM_WIN32
#include	"unistd.h"
#endif

START_NAMESPACE( Aqsis )

// Local Constants

#define	dsres		1.0f
#define	dtres		1.0f
#define	ResFactor	1.0f
#define	MinSize		0.0f
#define	NumSamples	16
#define	MinSamples	3

// Local Variables

TqInt	CqShadowMap::m_rand_index = 0;
TqFloat	CqShadowMap::m_aRand_no[ 256 ];


//---------------------------------------------------------------------
/** Allocate the memory required by the depthmap.
 */

void CqShadowMap::AllocateMap( TqInt XRes, TqInt YRes )
{
	static CqRandom rand;

	std::list<CqTextureMapBuffer*>::iterator s;
	for ( s = m_apSegments.begin(); s != m_apSegments.end(); s++ )
		delete( *s );

	m_XRes = XRes;
	m_YRes = YRes;
	m_apSegments.push_back( CreateBuffer( 0, 0, m_XRes, m_YRes, 1 ) );

	TqInt i;
	for ( i = 0; i < 256; i++ )
		m_aRand_no[ i ] = ( rand.RandomFloat( 2.0f ) - 1.0f );
}



//----------------------------------------------------------------------
/** Check if a texture map exists in the cache, return a pointer to it if so, else
 * load it if possible..
 */

CqTextureMap* CqTextureMap::GetShadowMap( const CqString& strName )
{
	static int size = -1;
	static CqTextureMap *previous = NULL;

	QGetRenderContext() ->Stats().IncTextureMisses( 3 );

	if ( size == static_cast<int>( m_TextureMap_Cache.size() ) )
		if ( ( previous ) && ( previous->m_strName == strName ) )
		{
			QGetRenderContext() ->Stats().IncTextureHits( 0, 3 );
			return previous;
		}


	// First search the texture map cache
	for ( std::vector<CqTextureMap*>::iterator i = m_TextureMap_Cache.begin(); i != m_TextureMap_Cache.end(); i++ )
	{
		if ( ( *i ) ->m_strName == strName )
		{
			if ( ( *i ) ->Type() == MapType_Shadow )
			{
				previous = *i;
				size = m_TextureMap_Cache.size();
				QGetRenderContext() ->Stats().IncTextureHits( 1, 3 );
				return ( *i );
			}
			else
			{
				return ( NULL );
			}
		}
	}
	// If we got here, it doesn't exist yet, so we must create and load it.
	CqShadowMap* pNew = new CqShadowMap( strName );
	m_TextureMap_Cache.push_back( pNew );
	pNew->Open();

	TqPchar ptexfmt;
	if ( pNew->m_pImage == 0 ||
	        TIFFGetField( pNew->m_pImage, TIFFTAG_PIXAR_TEXTUREFORMAT, &ptexfmt ) != 1 ||
	        strcmp( ptexfmt, SHADOWMAP_HEADER ) != 0 )
	{
		//CqString strError( strName );
		//strError += " not a shadow map, use RiMakeShadow";
		//CqBasicError( 0, Severity_Normal, strError.c_str() );
		QGetRenderContextI() ->Logger() ->error( "Map \"%s\" is not a valid shadow map, use RiMakeShadow", strName.c_str() );
		pNew->SetInvalid();
	}
	else
		pNew->ReadMatrices();

	previous = pNew;
	size = m_TextureMap_Cache.size();
	return ( pNew );
}


//----------------------------------------------------------------------
/** Load the shadowmap data.
 */

void CqShadowMap::LoadZFile()
{
	// Load the shadowmap from a binary file.
	if ( m_strName != "" )
	{
		std::ifstream file( m_strName.c_str(), std::ios::in | std::ios::binary );

		if ( file != NULL )
		{
			// Save a file type and version marker
			TqPchar origHeader = ZFILE_HEADER;
			TqInt headerLength = strlen( ZFILE_HEADER );
			TqPchar strHeader = new TqChar[ headerLength ];
			file.read( strHeader, headerLength );
			// Check validity of shadow map.
			if ( strncmp( strHeader, origHeader, headerLength ) != 0 )
			{
				//CqString strErr( "Error : Invalid shadowmap format - " );
				//strErr += m_strName;
				//CqBasicError( ErrorID_InvalidShadowMap, Severity_Normal, strErr.c_str() );
				QGetRenderContextI() ->Logger() ->error( "Invalid shadow map format \"%s\"", m_strName.c_str() );
				return ;
			}

			// Save the xres and yres.
			file.read( reinterpret_cast<TqPchar >( &m_XRes ), sizeof( m_XRes ) );
			file.read( reinterpret_cast<TqPchar >( &m_YRes ), sizeof( m_YRes ) );

			// Save the transformation matrices.
			m_WorldToScreenMatrices.resize(1);
			m_WorldToCameraMatrices.resize(1);
			m_NumberOfMaps = 0;
			file.read( reinterpret_cast<TqPchar>( matWorldToCamera()[ 0 ] ), sizeof( matWorldToCamera()[ 0 ][ 0 ] ) * 4 );
			file.read( reinterpret_cast<TqPchar>( matWorldToCamera()[ 1 ] ), sizeof( matWorldToCamera()[ 0 ][ 0 ] ) * 4 );
			file.read( reinterpret_cast<TqPchar>( matWorldToCamera()[ 2 ] ), sizeof( matWorldToCamera()[ 0 ][ 0 ] ) * 4 );
			file.read( reinterpret_cast<TqPchar>( matWorldToCamera()[ 3 ] ), sizeof( matWorldToCamera()[ 0 ][ 0 ] ) * 4 );

			file.read( reinterpret_cast<TqPchar>( matWorldToScreen()[ 0 ] ), sizeof( matWorldToScreen()[ 0 ][ 0 ] ) * 4 );
			file.read( reinterpret_cast<TqPchar>( matWorldToScreen()[ 1 ] ), sizeof( matWorldToScreen()[ 0 ][ 0 ] ) * 4 );
			file.read( reinterpret_cast<TqPchar>( matWorldToScreen()[ 2 ] ), sizeof( matWorldToScreen()[ 0 ][ 0 ] ) * 4 );
			file.read( reinterpret_cast<TqPchar>( matWorldToScreen()[ 3 ] ), sizeof( matWorldToScreen()[ 0 ][ 0 ] ) * 4 );

			// Now output the depth values
			AllocateMap( m_XRes, m_YRes );
			file.read( reinterpret_cast<TqPchar>( m_apSegments.front() ->pVoidBufferData() ), sizeof( TqFloat ) * ( m_XRes * m_YRes ) );

			// Set the matrixes to general, not Identity as default.
			matWorldToCamera().SetfIdentity( TqFalse );
			matWorldToScreen().SetfIdentity( TqFalse );
		}
		else
		{
			//CqString strErr( "Shadow map not found " );
			//strErr += m_strName;
			//CqBasicError( ErrorID_FileNotFound, Severity_Normal, strErr.c_str() );
			QGetRenderContextI() ->Logger() ->error( "Shadow map \"%s\" not found", m_strName.c_str());
		}
	}
}

//----------------------------------------------------------------------
/** Read the matrices out of the tiff file.
 */

void CqShadowMap::ReadMatrices()
{
	// Read the transform matrices.
	TqFloat*	WToC;
	TqFloat*	WToS;
	CqMatrix	matWToC, matWToS;
	
	// Set the number of shadow maps initially to 0.
	m_NumberOfMaps = 0;

	CqMatrix matCToW = QGetRenderContextI() ->matSpaceToSpace( "camera", "world" );
	
	while(1)
	{
		TqInt reta = TIFFGetField( m_pImage, TIFFTAG_PIXAR_MATRIX_WORLDTOCAMERA, &WToC );
		TqInt retb = TIFFGetField( m_pImage, TIFFTAG_PIXAR_MATRIX_WORLDTOSCREEN, &WToS );
		if ( !reta || !retb )
			SetInvalid();
		else
		{
			TqInt r, c;
			for ( r = 0; r < 4; r++ )
			{
				for ( c = 0; c < 4; c++ )
				{
					matWToC[ r ][ c ] = WToC[ ( r * 4 ) + c ];
					matWToS[ r ][ c ] = WToS[ ( r * 4 ) + c ];
				}
			}
		}
		// Set the matrixes to general, not Identity as default.
		matWToC.SetfIdentity( TqFalse );
		matWToS.SetfIdentity( TqFalse );

		matWToC *= matCToW;
		matWToS *= matCToW;

		// Generate normal conversion matrices to save time.
		CqMatrix matITTCToL = matWToC;
		matITTCToL[ 3 ][ 0 ] = matITTCToL[ 3 ][ 1 ] = matITTCToL[ 3 ][ 2 ] = matITTCToL[ 0 ][ 3 ] = matITTCToL[ 1 ][ 3 ] = matITTCToL[ 2 ][ 3 ] = 0.0;
		matITTCToL[ 3 ][ 3 ] = 1.0;
		matITTCToL.Inverse();
		matITTCToL.Transpose();

		m_WorldToCameraMatrices.push_back( matWToC );
		m_WorldToScreenMatrices.push_back( matWToS );
		m_ITTCameraToLightMatrices.push_back( matITTCToL );

		m_NumberOfMaps++;	// Increment the number of maps.

		if( TIFFReadDirectory( m_pImage ) == 0 )
			break;
	}
}


void CqShadowMap::PrepareSampleOptions( std::map<std::string, IqShaderData*>& paramMap )
{
	CqTextureMap::PrepareSampleOptions( paramMap );
	
	// Extend the shadow() call to accept bias, if set, override global bias
	m_bias = 0.0f;
	m_bias0 = 0.0f;
	m_bias1 = 0.0f;

	if ( ( paramMap.size() != 0 ) && ( paramMap.find( "bias" ) != paramMap.end() ) )
	{
		paramMap[ "bias" ] ->GetFloat( m_bias );
	}
	else
	{
		// Add in the bias at this point in camera coordinates.
		const TqFloat* poptBias = QGetRenderContextI() ->GetFloatOption( "shadow", "bias0" );
		if ( poptBias != 0 )
			m_bias0 = poptBias[ 0 ];

		poptBias = QGetRenderContextI() ->GetFloatOption( "shadow", "bias1" );
		if ( poptBias != 0 )
			m_bias1 = poptBias[ 0 ];

		// Get bias, if set override bias0 and bias1
		poptBias = QGetRenderContextI() ->GetFloatOption( "shadow", "bias" );
		if ( poptBias != 0 )
			m_bias = poptBias[ 0 ];
	}
}


//---------------------------------------------------------------------
/** Sample the shadow map data to see if the point vecPoint is in shadow.
 */

void CqShadowMap::SampleMap( CqVector3D& vecPoint, CqVector3D& swidth, CqVector3D& twidth, std::valarray<TqFloat>& val, TqInt index, TqFloat* average_depth, TqFloat* shadow_depth )
{
	if ( m_pImage != 0 )
	{
		swidth *= m_pswidth;
		twidth *= m_ptwidth;

		CqVector3D	R1, R2, R3, R4;
		R1 = vecPoint;
		R1 -= ( swidth / 2.0f );
		R1 -= ( twidth / 2.0f );
		R2 = vecPoint;
		R2 += ( swidth / 2.0f );
		R2 -= ( twidth / 2.0f );
		R3 = vecPoint;
		R3 -= ( swidth / 2.0f );
		R3 += ( twidth / 2.0f );
		R4 = vecPoint;
		R4 += ( swidth / 2.0f );
		R4 += ( twidth / 2.0f );

		SampleMap( R1, R2, R3, R4, val, index, average_depth, shadow_depth );
	}
	else
	{
		// If no map defined, not in shadow.
		val.resize( 1 );
		val[ 0 ] = 0.0f;
	}
}

//---------------------------------------------------------------------
/** Sample the shadow map data using R1, R2, R3, R4
 */

void	CqShadowMap::SampleMap( CqVector3D& R1, CqVector3D& R2, CqVector3D& R3, CqVector3D& R4, std::valarray<TqFloat>& val, TqInt index, TqFloat* average_depth, TqFloat* shadow_depth )
{
	// Check the memory and make sure we don't abuse it
	CriticalMeasure();

	// If no map defined, not in shadow.
	val.resize( 1 );
	val[ 0 ] = 0.0f;

	CqVector3D	vecR1l;
	CqVector3D	vecR1m, vecR2m, vecR3m, vecR4m;

	static CqRandom random( 42 );

	// If bias not set ( i.e. 0 ), try to use bias0 and bias1
	if ( !( m_bias > 0 ) )
	{
		if ( m_bias1 >= m_bias0 )
		{
			m_bias = random.RandomFloat( m_bias1 - m_bias0 ) + m_bias0;
			if ( m_bias > m_bias1 ) m_bias = m_bias1;
		}
		else
		{
			m_bias = random.RandomFloat( m_bias0 - m_bias1 ) + m_bias1;
			if ( m_bias > m_bias0 ) m_bias = m_bias0;
		}
	}

	CqVector3D vecBias( 0, 0, m_bias );
	// Generate a matrix to transform points from camera space into the space of the light source used in the
	// definition of the shadow map.
	CqMatrix& matCameraToLight = matWorldToCamera( index );
	// Generate a matrix to transform points from camera space into the space of the shadow map.
	CqMatrix& matCameraToMap = matWorldToScreen( index );

	R1 -= vecBias;
	R2 -= vecBias;
	R3 -= vecBias;
	R4 -= vecBias;
	vecR1l = matCameraToLight * ( ( R1 + R2 + R3 + R4 ) * 0.25f );
	TqFloat z = vecR1l.z();

	vecR1m = matCameraToMap * R1;
	vecR2m = matCameraToMap * R2;
	vecR3m = matCameraToMap * R3;
	vecR4m = matCameraToMap * R4;

	TqFloat sbo2 = ( m_sblur * 0.5f ) * m_XRes;
	TqFloat tbo2 = ( m_tblur * 0.5f ) * m_YRes;

	// If point is behind light, call it not in shadow.
	//if(z1<0.0)	return;

	TqFloat xro2 = m_XRes * 0.5f;
	TqFloat yro2 = m_YRes * 0.5f;

	TqFloat s1 = vecR1m.x() * xro2 + xro2;
	TqFloat t1 = m_YRes - ( vecR1m.y() * yro2 + yro2 );
	TqFloat s2 = vecR2m.x() * xro2 + xro2;
	TqFloat t2 = m_YRes - ( vecR2m.y() * yro2 + yro2 );
	TqFloat s3 = vecR3m.x() * xro2 + xro2;
	TqFloat t3 = m_YRes - ( vecR3m.y() * yro2 + yro2 );
	TqFloat s4 = vecR4m.x() * xro2 + xro2;
	TqFloat t4 = m_YRes - ( vecR4m.y() * yro2 + yro2 );

	TqFloat smin = ( s1 < s2 ) ? s1 : ( s2 < s3 ) ? s2 : ( s3 < s4 ) ? s3 : s4;
	TqFloat smax = ( s1 > s2 ) ? s1 : ( s2 > s3 ) ? s2 : ( s3 > s4 ) ? s3 : s4;
	TqFloat tmin = ( t1 < t2 ) ? t1 : ( t2 < t3 ) ? t2 : ( t3 < t4 ) ? t3 : t4;
	TqFloat tmax = ( t1 > t2 ) ? t1 : ( t2 > t3 ) ? t2 : ( t3 > t4 ) ? t3 : t4;

	// Cull if outside bounding box.
	TqUint lu = static_cast<TqInt>( FLOOR( smin ) );
	TqUint hu = static_cast<TqInt>( CEIL ( smax ) );
	TqUint lv = static_cast<TqInt>( FLOOR( tmin ) );
	TqUint hv = static_cast<TqInt>( CEIL ( tmax ) );

	lu -= static_cast<TqInt>( sbo2 );
	lv -= static_cast<TqInt>( tbo2 );
	hu += static_cast<TqInt>( sbo2 );
	hv += static_cast<TqInt>( tbo2 );

	if ( lu >= m_XRes || hu < 0 || lv >= m_YRes || hv < 0 )
		return ;

	lu = MAX(0,lu);
	lv = MAX(0,lv);
	hu = MIN(m_XRes - 1,hu);
	hv = MIN(m_YRes - 1,hv);

	TqFloat sres = hu - lu;
	TqFloat tres = hv - lv;

	// Calculate no. of samples.
	TqInt nt, ns;
	if ( m_samples > 0 )
	{
		nt = ns = static_cast<TqInt>( CEIL( sqrt( m_samples ) ) );
	}
	else
	{
		if ( sres * tres * 4.0 < NumSamples )
		{
			ns = static_cast<TqInt>( sres * 2.0 + 0.5 );
			ns = ( ns < MinSamples ? MinSamples : ( ns > NumSamples ? NumSamples : ns ) );
			nt = static_cast<TqInt>( tres * 2.0 + 0.5 );
			nt = ( nt < MinSamples ? MinSamples : ( nt > NumSamples ? NumSamples : nt ) );
		}
		else
		{
			nt = static_cast<TqInt>( sqrt( tres * NumSamples / sres ) + 0.5 );
			nt = ( nt < MinSamples ? MinSamples : ( nt > NumSamples ? NumSamples : nt ) );
			ns = static_cast<TqInt>( static_cast<TqFloat>( NumSamples ) / nt + 0.5 );
			ns = ( ns < MinSamples ? MinSamples : ( ns > NumSamples ? NumSamples : ns ) );
		}
	}

	// Setup jitter variables
	TqFloat ds = sres / ns;
	TqFloat dt = tres / nt;
	TqFloat js = ds * 0.5f;
	TqFloat jt = dt * 0.5f;

	// Test the samples.
	TqInt inshadow = 0;
	TqFloat avz = 0.0f;
	TqFloat sample_z = 0.0f; // How deep we're in the shadow

	TqFloat s = lu;
	TqInt i;
	CqTextureMapBuffer * pTMBa = NULL;
	for ( i = 0; i < ns; i++ )
	{
		TqFloat t = lv;
		TqInt j;
		for ( j = 0; j < nt; j++ )
		{
			// Jitter s and t
			m_rand_index = ( m_rand_index + 1 ) & 255;
			TqUint iu = static_cast<TqUint>( s + m_aRand_no[ m_rand_index ] * js );
			m_rand_index = ( m_rand_index + 1 ) & 255;
			TqUint iv = static_cast<TqUint>( t + m_aRand_no[ m_rand_index ] * jt );

			if( ( pTMBa == NULL )  || !pTMBa->IsValid( iu, iv, index ) )
				pTMBa = GetBuffer( iu, iv, index );
			if ( pTMBa != 0 && pTMBa->pVoidBufferData() != 0 )
			{
				iu -= pTMBa->sOrigin();
				iv -= pTMBa->tOrigin();
				TqFloat mapz = pTMBa->GetValue( iu, iv, 0 );
				if ( z > mapz )
				{
					inshadow += 1;
					sample_z = z - mapz;
				}
				avz += mapz;
			}
			t += dt;
		}
		s += ds;
	}

	if( NULL != average_depth )
	{
		avz /= ( ns * nt );
		*average_depth = avz;
	}

	if( NULL != shadow_depth )
	{
		sample_z /= ( ns * nt );
		
		*shadow_depth = sample_z;
	}

	val[ 0 ] = ( static_cast<TqFloat>( inshadow ) / ( ns * nt ) );
}


//----------------------------------------------------------------------
/** Save the shadowmap data in system specifirc image format.
 */

void CqShadowMap::SaveShadowMap( const CqString& strShadowName, TqBool append )
{
	TqChar version[ 80 ];

	const char* mode = (append)? "a" : "w";

	// Save the shadowmap to a binary file.
	if ( m_strName.compare( "" ) != 0 )
	{
		if ( m_apSegments.size() != 0 )
		{
			TIFF * pshadow = TIFFOpen( strShadowName.c_str(), mode );
			TIFFCreateDirectory( pshadow );

			// Write the transform matrices.
			TqFloat	matWToC[ 16 ];
			TqFloat	matWToS[ 16 ];
			TqInt r, c;
			for ( r = 0; r < 4; r++ )
			{
				for ( c = 0; c < 4; c++ )
				{
					matWToC[ ( r * 4 ) + c ] = matWorldToCamera()[ r ][ c ];
					matWToS[ ( r * 4 ) + c ] = matWorldToScreen()[ r ][ c ];
				}
			}
#if defined(AQSIS_SYSTEM_WIN32) || defined(AQSIS_SYSTEM_MACOSX)
			sprintf( version, "%s %s", STRNAME, VERSION_STR );
#else
			sprintf( version, "%s %s", STRNAME, VERSION );
#endif
			TIFFSetField( pshadow, TIFFTAG_SOFTWARE, ( uint32 ) version );
			TIFFSetField( pshadow, TIFFTAG_PIXAR_MATRIX_WORLDTOCAMERA, matWToC );
			TIFFSetField( pshadow, TIFFTAG_PIXAR_MATRIX_WORLDTOSCREEN, matWToS );
			TIFFSetField( pshadow, TIFFTAG_PIXAR_TEXTUREFORMAT, SHADOWMAP_HEADER );
			TIFFSetField( pshadow, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK );

			// Write the floating point image to the directory.
			TqFloat *depths = reinterpret_cast<TqFloat*>( m_apSegments.front() ->pVoidBufferData() );
			WriteTileImage( pshadow, depths, XRes(), YRes(), 32, 32, 1, m_Compression, m_Quality );
			TIFFClose( pshadow );
		}
	}
}

//----------------------------------------------------------------------
/** Save the shadowmap data in system specifirc image format.
 */

void CqShadowMap::SaveZFile()
{
	// Save the shadowmap to a binary file.
	if ( m_strName != "" )
	{
		std::ofstream ofile( m_strName.c_str(), std::ios::out | std::ios::binary );
		if ( ofile.is_open() )
		{
			// Save a file type and version marker
			ofile << ZFILE_HEADER;

			// Save the xres and yres.
			ofile.write( reinterpret_cast<TqPchar >( &m_XRes ), sizeof( m_XRes ) );
			ofile.write( reinterpret_cast<TqPchar >( &m_YRes ), sizeof( m_XRes ) );

			// Save the transformation matrices.
			ofile.write( reinterpret_cast<TqPchar>( matWorldToCamera()[ 0 ] ), sizeof( matWorldToCamera()[ 0 ][ 0 ] ) * 4 );
			ofile.write( reinterpret_cast<TqPchar>( matWorldToCamera()[ 1 ] ), sizeof( matWorldToCamera()[ 0 ][ 0 ] ) * 4 );
			ofile.write( reinterpret_cast<TqPchar>( matWorldToCamera()[ 2 ] ), sizeof( matWorldToCamera()[ 0 ][ 0 ] ) * 4 );
			ofile.write( reinterpret_cast<TqPchar>( matWorldToCamera()[ 3 ] ), sizeof( matWorldToCamera()[ 0 ][ 0 ] ) * 4 );

			ofile.write( reinterpret_cast<TqPchar>( matWorldToScreen()[ 0 ] ), sizeof( matWorldToScreen()[ 0 ][ 0 ] ) * 4 );
			ofile.write( reinterpret_cast<TqPchar>( matWorldToScreen()[ 1 ] ), sizeof( matWorldToScreen()[ 0 ][ 0 ] ) * 4 );
			ofile.write( reinterpret_cast<TqPchar>( matWorldToScreen()[ 2 ] ), sizeof( matWorldToScreen()[ 0 ][ 0 ] ) * 4 );
			ofile.write( reinterpret_cast<TqPchar>( matWorldToScreen()[ 3 ] ), sizeof( matWorldToScreen()[ 0 ][ 0 ] ) * 4 );

			// Now output the depth values
			ofile.write( reinterpret_cast<TqPchar>( m_apSegments.front() ->pVoidBufferData() ), sizeof( TqFloat ) * ( m_XRes * m_YRes ) );
			ofile.close();
		}
	}
}





END_NAMESPACE( Aqsis )

