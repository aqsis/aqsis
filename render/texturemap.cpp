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
		\brief Implements texture map handling and cacheing classes.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	"aqsis.h"

#include	<math.h>
#include	<iostream>
#include	<fstream>

#include	"texturemap.h"
#include	"rifile.h"
#include	"exception.h"
#include	"irenderer.h"
#include	"random.h"
#include	"version.h"

#include	"renderer.h"
#ifdef		PLUGINS
#include	"plugins.h"
#endif

#ifndef		AQSIS_SYSTEM_WIN32
#include	"unistd.h"
#endif

START_NAMESPACE( Aqsis )

IqRenderer* QGetRenderContextI();


static void get_face_intersection( CqVector3D *normal, CqVector3D *pt, TqInt* face );
static void get_edge_intersection( CqVector3D* n1, CqVector3D* n2, TqInt edge, CqVector3D* pt );
static void project( TqInt face );

#define	max_no	30

// Face and edge numberings. see fig 6.12 page 192 [watt]
#define	pz	1
#define	px	2
#define	py	4
#define	nx	8
#define	ny	16
#define	nz	32

#define	edge01	3
#define	edge02	5
#define	edge03	9
#define	edge04	17
#define	edge12	6
#define	edge23	12
#define	edge34	24
#define	edge41	18
#define	edge51	34
#define	edge52	36
#define	edge53	40
#define	edge54	48
#define MEG1    1024*1024

CqVector3D	cube[ max_no ];	// Stores the projection of the reflected beam onto the cube.
TqInt	cube_no; 		// Stores the number of points making up the projection.
TqFloat	uv[ max_no ][ 2 ];	// Stores the values of this projection for a given face.

TqInt	CqShadowMap::m_rand_index = 0;
TqFloat	CqShadowMap::m_aRand_no[ 256 ];
static TqBool m_critical = TqFalse;
static CqTextureMapBuffer *previous = NULL;

//---------------------------------------------------------------------
/** Static array of cached texture maps.
 */

std::vector<CqTextureMap*>	CqTextureMap::m_TextureMap_Cache;
std::vector<CqString*> CqTextureMap::m_ConvertString_Cache;

#undef ALLOCSEGMENTSTATUS

#ifdef ALLOCSEGMENTSTATUS
static TqInt alloc_cnt = 0;
static TqInt free_cnt = 0;
#endif 
//---------------------------------------------------------------------
/** Allocate a cache segment to hold the specified image tile.
 */

TqPuchar CqTextureMapBuffer::AllocSegment( TqUlong width, TqUlong height, TqInt samples )
{
	TqChar warnings[ 400 ];
	static TqInt limit = -1;
	static TqInt report = 1;
	static TqInt megs = 10; /* a number to hint it is abusing enough memory */
	TqInt demand = width * height * ElemSize();

#ifdef ALLOCSEGMENTSTATUS
	alloc_cnt ++;
#endif
	if ( limit == -1 )
	{
		const TqInt * poptMem = QGetRenderContextI() ->GetIntegerOption( "limits", "texturememory" );
		limit = MEG1;
		if ( poptMem )
			limit = poptMem[ 0 ] * 1024;
	}

	TqInt more = QGetRenderContext() ->Stats().GetTextureMemory() + demand;


	if ( more > limit )
	{

		// Critical level of memory will be reached;
		// I'm better starting the cleanup cache memory buffer
		// We need to zap the cache we are exceeding the required texturememory demand
		// For now, we will just warn the user the texturememory's demand will exceed the
		// request number.

		sprintf( warnings, "Exceeding the allocated texturememory by %d",
		         more - limit );
		if ( report )
			RiErrorPrint( 0, 1, warnings );
		report = 0;
		m_critical = TqTrue;
	}

#ifdef _DEBUG
	if ( ( more > MEG1 ) && ( ( more / ( 1024 * 1024 ) ) > megs ) )
	{
		sprintf( warnings, "Texturememory is more than %d Megs", megs );
		RiErrorPrint( 0, 1, warnings );
		megs += 10;
	}
#endif
	QGetRenderContext() ->Stats().IncTextureMemory( demand );

	// just do a malloc since the texturemapping will set individual member of the allocated buffer
	// a calloc is wastefull operation in this case.
	return ( static_cast<TqPuchar>( malloc( demand ) ) );
}
//---------------------------------------------------------------------
/** Static array of cached texture maps.
 */

void	CqTextureMapBuffer::FreeSegment( TqPuchar pBufferData, TqUlong width, TqUlong height, TqInt samples )
{
	TqInt demand = ( width * height * samples );
#ifdef ALLOCSEGMENTSTATUS
	free_cnt ++;
#endif

	QGetRenderContext() ->Stats().IncTextureMemory( -demand );
	free( pBufferData );

}


//---------------------------------------------------------------------
/** Destructor.
 */

CqTextureMap::~CqTextureMap()
{
	// Search for it in the cache and remove the reference.
	std::vector<CqTextureMap*>::iterator i;

	for ( i = m_TextureMap_Cache.begin(); i != m_TextureMap_Cache.end(); i++ )
	{
		if ( ( *i ) == this )
		{
			m_TextureMap_Cache.erase( i );
			break;
		}
	}

	std::vector<CqString*>::iterator j;
	for ( j = m_ConvertString_Cache.begin(); j != m_ConvertString_Cache.end(); j++ )
	{
		if ( *j )
		{
			unlink( ( *j ) ->c_str() );
			delete( *j );
		}
	}

	m_ConvertString_Cache.resize( 0 );

	// Delete any held cache buffer segments.
	std::vector<CqTextureMapBuffer*>::iterator s;
	for ( s = m_apSegments.begin(); s != m_apSegments.end(); s++ )
		delete( *s );

	m_apSegments.resize( 0 );


#ifdef ALLOCSEGMENTSTATUS
	{
		// We count each allocation/free at the end they should match
		TqChar report[ 200 ];

		sprintf( report, "alloc/free %d %d\n Memory Usage %d", alloc_cnt, free_cnt, QGetRenderContext() ->Stats().GetTextureMemory() );
		RiErrorPrint( 0, 1, report );
	}
#endif
}


//--------------------------------------------------------------------------
/** Support for plugins mainly converter from any bitmap format to .tif file.
 */

TqInt CqTextureMap::Convert( CqString &strName )
{
	TqInt result = 0;
#ifdef	PLUGINS
	TqInt lenght = 0;
	TqChar library[ 1024 ];
	TqChar function[ 1024 ];
	char * ( *convert ) ( char * );
	char *ext = NULL;
	char *tiff = NULL;

	convert = NULL;

	char *aqsis_home = getenv( "AQSIS_BASE_PATH" );

	if ( aqsis_home == NULL )
#ifdef	AQSIS_SYSTEM_POSIX
		aqsis_home = BASE_PATH;
#else
		aqsis_home = ".";
#endif

	ext = ( TqPchar ) strName.c_str();
	if ( ext && *ext )
		lenght = strlen( ext );

	if ( lenght > 4 )
	{

		/* find the extension in the string
		 * than try to match which extension goes with which dll 
		 **/
		ext = strstr( &ext[ lenght - 5 ], "." );
		if ( ext == NULL ) return 0;
		ext++;
	}
	else
	{
		return 0;
	}

	sprintf( function, "%s2tif", ext );
#ifdef AQSIS_SYSTEM_WIN32
	/*********************************/
	/* Get the dynamic library on NT */
	/*********************************/
	sprintf( library, "%s\\procedures\\%s.dll", aqsis_home, function );
#else
	/***********************************/
	/* Get the shared library on UNIX  */
	/***********************************/
	sprintf( library, "%s/lib/lib%s.so", aqsis_home, function );
#endif
	CqPlugins *plug = new CqPlugins( "", library, function );
	if ( ( convert = ( char * ( * ) ( char * s ) ) plug->Function() ) != NULL )
	{

		if ( ( tiff = ( *convert ) ( ( char * ) strName.c_str() ) ) != NULL )
		{
			strName = tiff;
			result = 1; // success
		}
		plug->Close();
	}
	delete plug;
#endif
	return result;
}

//---------------------------------------------------------------------
/** Open a named texture map.
 */

void CqTextureMap::Open()
{
	TqChar swrap[ 80 ], twrap[ 80 ], filterfunc[ 80 ];
	TqFloat swidth, twidth;
	TqInt wasconverted = 0;

	m_IsValid = TqFalse;


	// Find the file required.
	CqRiFile	fileImage( m_strName.c_str(), "texture" );
	if ( !fileImage.IsValid() )
	{
		CqString strErr( "Cannot open texture file : " );
		strErr += m_strName;
		CqBasicError( 1, Severity_Fatal, strErr.c_str() );
		return ;
	}
	CqString strRealName( fileImage.strRealName() );
	fileImage.Close();

	// Now try to converted first to tif file
	wasconverted = Convert( strRealName );
	if ( wasconverted )
	{
		CqString * strnew = new CqString( strRealName );
		m_ConvertString_Cache.push_back( strnew );
	}

	// Now open it as a tiff file.
	m_pImage = TIFFOpen( strRealName.c_str(), "r" );
	if ( m_pImage )
	{
		TqPchar pFormat = 0;
		TqPchar pModes = 0;

		TIFFGetField( m_pImage, TIFFTAG_IMAGEWIDTH, &m_XRes );
		TIFFGetField( m_pImage, TIFFTAG_IMAGELENGTH, &m_YRes );

#ifdef AQSIS_SYSTEM_MACOSX
		uint16 planarconfig;
		TIFFGetField( m_pImage, TIFFTAG_PLANARCONFIG, &planarconfig );
		m_PlanarConfig = planarconfig;
		uint16 samplesperpixel;
		TIFFGetField( m_pImage, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel );
		m_SamplesPerPixel = samplesperpixel;
#else
		TIFFGetField( m_pImage, TIFFTAG_PLANARCONFIG, &m_PlanarConfig );
		TIFFGetField( m_pImage, TIFFTAG_SAMPLESPERPIXEL, &m_SamplesPerPixel );
#endif

		TIFFGetField( m_pImage, TIFFTAG_PIXAR_TEXTUREFORMAT, &pFormat );
		TIFFGetField( m_pImage, TIFFTAG_PIXAR_WRAPMODES, &pModes );

		// Resize the temporary storage values to the new depth.
		m_tempval1.resize( m_SamplesPerPixel );
		m_tempval2.resize( m_SamplesPerPixel );
		m_tempval3.resize( m_SamplesPerPixel );
		m_tempval4.resize( m_SamplesPerPixel );
		m_low_color.resize( m_SamplesPerPixel );
		m_high_color.resize( m_SamplesPerPixel );

		if ( pModes )
		{
			sscanf( pModes, "%s %s %s %f %f", swrap, twrap, filterfunc, &swidth, &twidth );

			/// smode
			if ( strcmp( swrap, RI_PERIODIC ) == 0 )
			{
				m_smode = WrapMode_Periodic;
			}
			else if ( strcmp( swrap, RI_BLACK ) == 0 )
			{
				m_smode = WrapMode_Black;
			}
			else if ( strcmp( swrap, RI_CLAMP ) == 0 )
			{
				m_smode = WrapMode_Clamp;
			}

			/// t mode
			if ( strcmp( twrap, RI_PERIODIC ) == 0 )
			{
				m_tmode = WrapMode_Periodic;
			}
			else if ( strcmp( twrap, RI_BLACK ) == 0 )
			{
				m_tmode = WrapMode_Black;
			}
			else if ( strcmp( twrap, RI_CLAMP ) == 0 )
			{
				m_tmode = WrapMode_Clamp;
			}
			/// Pixel's Filter
			if ( strcmp( filterfunc, "gaussian" ) == 0 )
			{
				m_FilterFunc = RiGaussianFilter;
			}
			else if ( strcmp( filterfunc, "box" ) == 0 )
			{
				m_FilterFunc = RiBoxFilter;
			}
			else if ( strcmp( filterfunc, "triangle" ) == 0 )
			{
				m_FilterFunc = RiTriangleFilter;
			}
			else if ( strcmp( filterfunc, "catmull-rom" ) == 0 )
			{
				m_FilterFunc = RiCatmullRomFilter;
			}
			else if ( strcmp( filterfunc, "sinc" ) == 0 )
			{
				m_FilterFunc = RiSincFilter;
			}
			else if ( strcmp( filterfunc, "disk" ) == 0 )
			{
				m_FilterFunc = RiDiskFilter;
			}
			else if ( strcmp( filterfunc, "bessel" ) == 0 )
			{
				m_FilterFunc = RiBesselFilter;
			}

			/// Pixel's Filter x,y
			m_swidth = swidth;
			m_twidth = twidth;
		}


		if ( pFormat && strcmp( pFormat, MIPMAP_HEADER ) == 0 ||
		        ( pFormat && strcmp( pFormat, CUBEENVMAP_HEADER ) == 0 ) ||
		        ( pFormat && strcmp( pFormat, LATLONG_HEADER ) == 0 ) )
		{
			m_Format = TexFormat_MIPMAP;
			m_IsValid = TqTrue;
		}
		else
		{
			m_Format = TexFormat_Plain;
			m_IsValid = TqTrue;
		}
	}

}


//---------------------------------------------------------------------
/** If properly opened, close the TIFF file.
 */

void CqTextureMap::Close()
{

	if ( m_pImage != 0 ) TIFFClose( m_pImage );
	m_pImage = 0;

}


//----------------------------------------------------------------------
/** Check if a texture map exists in the cache, return a pointer to it if so, else
 * load it if possible..
 */

CqTextureMap* CqTextureMap::GetTextureMap( const CqString& strName )
{
	static int size = -1;
	static CqTextureMap *previous = NULL;

	QGetRenderContext() ->Stats().IncTextureMisses( 0 );

	/* look if the last item return by this function was ok */
	if ( size == m_TextureMap_Cache.size() )
		if ( ( previous ) && ( previous->m_strName == strName ) )
		{
			QGetRenderContext() ->Stats().IncTextureHits( 0, 0 );
			return previous;
		}



	// First search the texture map cache
	for ( std::vector<CqTextureMap*>::iterator i = m_TextureMap_Cache.begin(); i != m_TextureMap_Cache.end(); i++ )
	{
		if ( ( *i ) ->m_strName == strName )
		{
			if ( ( *i ) ->Type() == MapType_Texture )
			{
				previous = *i;
				size = m_TextureMap_Cache.size();
				QGetRenderContext() ->Stats().IncTextureHits( 1, 0 );
				return ( *i );
			}
			else
			{
				return ( NULL );
			}
		}
	}
	// If we got here, it doesn't exist yet, so we must create and load it.
	CqTextureMap* pNew = new CqTextureMap( strName );
	m_TextureMap_Cache.push_back( pNew );
	pNew->Open();

	// Ensure that it is in the correct format
	if ( pNew->Format() != TexFormat_MIPMAP )
	{
		pNew->CreateMIPMAP();
		pNew->Close();
	}

	previous = pNew;
	size = m_TextureMap_Cache.size();
	return ( pNew );
}


//----------------------------------------------------------------------
/** Check if a texture map exists in the cache, return a pointer to it if so, else
 * load it if possible..
 */

CqTextureMap* CqTextureMap::GetEnvironmentMap( const CqString& strName )
{
	static int size = -1;
	static CqTextureMap *previous = NULL;

	QGetRenderContext() ->Stats().IncTextureMisses( 1 );

	/* look if the last item return by this function was ok */
	if ( size == m_TextureMap_Cache.size() )
		if ( ( previous ) && ( previous->m_strName == strName ) )
		{
			QGetRenderContext() ->Stats().IncTextureHits( 0, 1 );
			return previous;
		}



	// First search the texture map cache
	for ( std::vector<CqTextureMap*>::iterator i = m_TextureMap_Cache.begin(); i != m_TextureMap_Cache.end(); i++ )
	{
		if ( ( *i ) ->m_strName == strName )
		{
			if ( ( *i ) ->Type() == MapType_Environment )
			{
				previous = *i;
				size = m_TextureMap_Cache.size();
				QGetRenderContext() ->Stats().IncTextureHits( 1, 1 );
				return ( *i );
			}
			else
			{
				return ( NULL );
			}
		}
	}
	// If we got here, it doesn't exist yet, so we must create and load it.
	CqTextureMap* pNew = new CqEnvironmentMap( strName );
	m_TextureMap_Cache.push_back( pNew );
	pNew->Open();

	TqPchar ptexfmt = 0;

	// Invalid if the m_pImage is not there or it is not cube or latlong env. map file
	if ( pNew->m_pImage == 0 ||
	        TIFFGetField( pNew->m_pImage, TIFFTAG_PIXAR_TEXTUREFORMAT, &ptexfmt ) != 1 ||
	        ( strcmp( ptexfmt, CUBEENVMAP_HEADER ) != 0 ) && ( strcmp( ptexfmt, LATLONG_HEADER ) != 0 ) )
	{
		CqString strError( strName );
		strError += " not an environment map, use RiMakeCubeFaceEnvironment";
		CqBasicError( 0, Severity_Normal, strError.c_str() );
		pNew->SetInvalid();
		delete pNew;
		pNew = NULL;

	}

	// remove from the list a LatLong env. map since in shadeops.cpp we will cope with it.
	if ( ptexfmt && strcmp( ptexfmt, LATLONG_HEADER ) == 0 )
	{
		pNew->SetInvalid();
		delete pNew;
		pNew = NULL;
	}

	previous = pNew;
	size = m_TextureMap_Cache.size();
	return ( pNew );
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

	if ( size == m_TextureMap_Cache.size() )
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
		CqString strError( strName );
		strError += " not a shadow map, use RiMakeShadow";
		CqBasicError( 0, Severity_Normal, strError.c_str() );
		pNew->SetInvalid();
	}
	else
		pNew->ReadMatrices();

	previous = pNew;
	size = m_TextureMap_Cache.size();
	return ( pNew );
}
//----------------------------------------------------------------------
/** Check if a texture map exists in the cache, return a pointer to it if so, else
 * load it if possible..
 */

CqTextureMap* CqTextureMap::GetLatLongMap( const CqString& strName )
{
	static int size = -1;
	static CqTextureMap *previous = NULL;
	QGetRenderContext() ->Stats().IncTextureMisses( 2 );

	if ( size == m_TextureMap_Cache.size() )
		if ( previous && ( previous->m_strName == strName ) )
		{
			QGetRenderContext() ->Stats().IncTextureHits( 0, 2 );
			return previous;
		}



	// First search the texture map cache
	for ( std::vector<CqTextureMap*>::iterator i = m_TextureMap_Cache.begin(); i != m_TextureMap_Cache.end(); i++ )
	{
		if ( ( *i ) ->m_strName == strName )
		{
			if ( ( *i ) ->Type() == MapType_LatLong )
			{
				previous = *i;
				size = m_TextureMap_Cache.size();
				QGetRenderContext() ->Stats().IncTextureHits( 1, 2 );
				return ( *i );
			}
			else
			{
				return ( NULL );
			}
		}
	}
	// If we got here, it doesn't exist yet, so we must create and load it.
	CqTextureMap* pNew = new CqLatLongMap( strName );
	m_TextureMap_Cache.push_back( pNew );
	pNew->Open();

	TqPchar ptexfmt;

	// Invalid only if this is not a LatLong Env. map file
	if ( pNew->m_pImage == 0 ||
	        TIFFGetField( pNew->m_pImage, TIFFTAG_PIXAR_TEXTUREFORMAT, &ptexfmt ) != 1 ||
	        strcmp( ptexfmt, LATLONG_HEADER ) != 0 )
	{
		CqString strError( strName );
		strError += " not an environment map, use RiMakeLatLongEnvironment";
		CqBasicError( 0, Severity_Normal, strError.c_str() );
		pNew->SetInvalid();
	}

	previous = pNew;
	size = m_TextureMap_Cache.size();
	return ( pNew );
}



//----------------------------------------------------------------------
/** Get the buffer segment from the cache which conmtains the requested s,t coordinates, read from the file if not
 * already cached.
 */

CqTextureMapBuffer* CqTextureMap::GetBuffer( TqUlong s, TqUlong t, TqInt directory )
{
	static TqInt size = -1;
	static CqString name = "";

	QGetRenderContext() ->Stats().IncTextureMisses( 4 );


	/* look if the last item return by this function was ok */
	if ( ( size == m_apSegments.size() ) && previous && ( name == m_strName ) )
		if ( previous->IsValid( s, t, directory ) )
		{
			QGetRenderContext() ->Stats().IncTextureHits( 0, 4 );
			return previous;
		}




	// Search already cached segments first.
	for ( std::vector<CqTextureMapBuffer*>::iterator i = m_apSegments.begin(); i != m_apSegments.end(); i++ )
		if ( ( *i ) ->IsValid( s, t, directory ) )
		{
			previous = ( *i );
			name = m_strName;
			size = m_apSegments.size();
			QGetRenderContext() ->Stats().IncTextureHits( 1, 4 );
			return ( *i );
		}

	// If we got here, segment is not currently loaded, so load the correct segement and store it in the cache.
	CqTextureMapBuffer* pTMB = 0;

	if ( !m_pImage )
	{
		CqRiFile	fileImage( m_strName.c_str(), "texture" );
		if ( !fileImage.IsValid() )
		{
			CqString strErr( "Cannot open texture file : " );
			strErr += m_strName;
			CqBasicError( 1, Severity_Fatal, strErr.c_str() );
			return pTMB;
		}
		CqString strRealName( fileImage.strRealName() );
		fileImage.Close();

		// Now open it as a tiff file.
		m_pImage = TIFFOpen( strRealName.c_str(), "r" );
	}

	if ( m_pImage )
	{
		uint32 tsx, tsy;
		TqInt ret = TIFFGetField( m_pImage, TIFFTAG_TILEWIDTH, &tsx );
		TIFFGetField( m_pImage, TIFFTAG_TILELENGTH, &tsy );
		// If a tiled image, read the appropriate tile.
		if ( ret )
		{
			// Work out the coordinates of this tile.
			TqUlong ox = ( s / tsx ) * tsx;
			TqUlong oy = ( t / tsy ) * tsy;
			pTMB = CreateBuffer( ox, oy, tsx, tsy, m_SamplesPerPixel, directory );

			TIFFSetDirectory( m_pImage, directory );

			void* pData = pTMB->pVoidBufferData();
			TIFFReadTile( m_pImage, pData, s, t, 0, 0 );
			m_apSegments.push_back( pTMB );
		}
		else
		{
			pTMB = CreateBuffer( 0, 0, m_XRes, m_YRes, m_SamplesPerPixel, directory );

			TIFFSetDirectory( m_pImage, directory );
			void* pdata = pTMB->pVoidBufferData();
			TqUint i;
			for ( i = 0; i < m_YRes; i++ )
			{
				TIFFReadScanline( m_pImage, pdata, i );
				pdata = reinterpret_cast<void*>(reinterpret_cast<char*>( pdata ) + m_XRes * pTMB->ElemSize() );
			}
			m_apSegments.push_back( pTMB );
		}
	}

	name = m_strName;
	previous = pTMB;
	size = m_TextureMap_Cache.size();
	return ( pTMB );
}

//----------------------------------------------------------------------
/** this is used for re-intrepreted the filter/wrap mode when using
 *  RiMakeTextureV() for downsampling/filter the tif file
 *
 **/
void CqTextureMap::Interpreted( TqPchar mode )
{
	TqChar filter[ 80 ];
	TqChar smode[ 80 ];
	TqChar tmode[ 80 ];

	sscanf( mode, "%s %s %s %f %f", smode, tmode, filter, &m_swidth, &m_twidth );

	m_FilterFunc = RiBoxFilter;
	if ( strcmp( filter, "gaussian" ) == 0 ) m_FilterFunc = RiGaussianFilter;
	if ( strcmp( filter, "box" ) == 0 ) m_FilterFunc = RiBoxFilter;
	if ( strcmp( filter, "triangle" ) == 0 ) m_FilterFunc = RiTriangleFilter;
	if ( strcmp( filter, "catmull-rom" ) == 0 ) m_FilterFunc = RiCatmullRomFilter;
	if ( strcmp( filter, "sinc" ) == 0 ) m_FilterFunc = RiSincFilter;
	if ( strcmp( filter, "disk" ) == 0 ) m_FilterFunc = RiDiskFilter;
	if ( strcmp( filter, "bessel" ) == 0 ) m_FilterFunc = RiBesselFilter;

	m_smode = m_tmode = WrapMode_Clamp;
	if ( strcmp( smode, RI_PERIODIC ) == 0 )
		m_smode = WrapMode_Periodic;
	else if ( strcmp( smode, RI_CLAMP ) == 0 )
		m_smode = WrapMode_Clamp;
	else if ( strcmp( smode, RI_BLACK ) == 0 )
		m_smode = WrapMode_Black;

	if ( strcmp( tmode, RI_PERIODIC ) == 0 )
		m_tmode = WrapMode_Periodic;
	else if ( strcmp( tmode, RI_CLAMP ) == 0 )
		m_tmode = WrapMode_Clamp;
	else if ( strcmp( tmode, RI_BLACK ) == 0 )
		m_tmode = WrapMode_Black;

}
//----------------------------------------------------------------------
/** this is used for downsampling the texture at lower resolution
  *   
  * it will use the filtervalues. breakdown rgba values in floats. 
  * Accumulate the floating value rgba and ponderate the sum with the filter values.
  * and convert back to uint32 the rgba floating values.
  * The values of the current filterfunc/swrap/twrap are used ; if ever swrap or twrap is equal to
  * zero than the filterfunc is not done anymore.
  **/
TqUlong CqTextureMap::ImageFilterVal( uint32* p, TqInt x, TqInt y, TqInt directory )
{
	TqUlong val = 0;
	RtFilterFunc pFilter = m_FilterFunc;

	TqInt ydelta = ( 1 << directory );
	TqInt xdelta = ( 1 << directory );
	TqFloat div = 0.0;
	TqFloat mul;
	TqFloat accum[ 4 ];

	accum[ 0 ] = accum[ 1 ] = accum[ 2 ] = accum[ 3 ] = 0.0;

	if ( directory )
	{
		TqInt i, j;
		for ( j = 0; j < ydelta; j++ )
		{
			for ( i = 0; i < xdelta; i++ )
			{
				/* find the filter value */
				TqFloat xfilt = i - directory;
				TqFloat yfilt = j - directory;
				mul = ( *pFilter ) ( xfilt, yfilt, xdelta, ydelta );

				/* find the value in the original image */
				TqInt pos = m_YRes - ( ( y * ydelta ) + j ) - 1;
				pos *= m_XRes;
				pos += ( ( x * xdelta ) + i );
				val = p[ pos ];

				/* ponderate the value */
				accum[ 0 ] += ( TIFFGetR( val ) / 255.0 ) * mul;
				accum[ 1 ] += ( TIFFGetG( val ) / 255.0 ) * mul;
				accum[ 2 ] += ( TIFFGetB( val ) / 255.0 ) * mul;
				accum[ 3 ] += ( TIFFGetA( val ) / 255.0 ) * mul;

				/* accumulate the ponderation factor */
				div += mul;

			}
		}

		/* use the accumulated ponderation factor */
		accum[ 0 ] /= static_cast<TqFloat>( div );
		accum[ 1 ] /= static_cast<TqFloat>( div );
		accum[ 2 ] /= static_cast<TqFloat>( div );
		accum[ 3 ] /= static_cast<TqFloat>( div );

		/* restore the byte from the floating values RGB */
		/* this is assuming tiff decoding is using shifting operations */
		val = ( static_cast<TqUint>( accum[ 0 ] * 255.0 ) & 0xff ) +         /* R */
		      ( ( static_cast<TqUint>( accum[ 1 ] * 255.0 ) << 8 ) & 0x0ff00 ) +        /* G */
		      ( ( static_cast<TqUint>( accum[ 2 ] * 255.0 ) << 16 ) & 0x0ff0000 ) +        /* B */
		      ( ( static_cast<TqUint>( accum[ 3 ] * 255.0 ) << 24 ) & 0x0ff000000 ); /* A */


	}
	else
	{
		/* copy the byte don't bother much */
		val = p[ ( ( m_YRes - y - 1 ) * m_XRes ) + x ];

	}

	return val;
}



void CqTextureMap::CreateMIPMAP()
{


	if ( m_pImage != 0 )
	{
		uint32 * pImage = static_cast<uint32*>( _TIFFmalloc( m_XRes * m_YRes * sizeof( uint32 ) ) );
		TIFFReadRGBAImage( m_pImage, m_XRes, m_YRes, pImage, 0 );
		TqInt m_xres = m_XRes;
		TqInt m_yres = m_YRes;
		TqInt directory = 0;


		do
		{

			//CqTextureMapBuffer* pTMB = new CqTextureMapBuffer();
			CqTextureMapBuffer* pTMB = CreateBuffer( 0, 0, m_xres, m_yres, m_SamplesPerPixel, directory );
			//pTMB->Init( 0, 0, m_xres, m_yres, m_SamplesPerPixel, directory );

			if ( pTMB->pBufferData() != NULL )
			{
				TqPuchar pMIPMAP = pTMB->pBufferData();
				long rowlen = m_xres * pTMB->ElemSize();

				if ( pImage != NULL )
				{
					for ( TqInt y = 0; y < m_yres; y++ )
					{
						unsigned char accum[ 4 ];


						for ( TqInt x = 0; x < m_xres; x++ )
						{
							uint32 val = ImageFilterVal( pImage, x, y, directory );

							accum[ 0 ] = TIFFGetR( val );
							accum[ 1 ] = TIFFGetG( val );
							accum[ 2 ] = TIFFGetB( val );
							accum[ 3 ] = TIFFGetA( val );

							for ( TqInt sample = 0; sample < m_SamplesPerPixel; sample++ )
								pMIPMAP[ ( y * rowlen ) + ( x * m_SamplesPerPixel ) + sample ] = accum[ sample ];
						}
					}

				}

				m_apSegments.push_back( pTMB );

			}

			m_xres /= 2;
			m_yres /= 2;
			directory++;

		}
		while ( ( m_xres > 2 ) && ( m_yres > 2 ) ) ;

		_TIFFfree( pImage );

	}

}


void CqTextureMap::SampleMap( TqFloat s1, TqFloat t1, TqFloat swidth, TqFloat twidth, std::valarray<TqFloat>& val, std::map<std::string, IqShaderData*>& paramMap )
{
	// Check the memory and make sure we don't abuse it
	CriticalMeasure();

	if ( !IsValid() ) return ;

	TqFloat sblur = 0.0f;
	TqFloat tblur = 0.0f;
	TqFloat pswidth = 1.0f;
	TqFloat ptwidth = 1.0f;

	// Get parameters out of the map.
	if( paramMap.find("width") != paramMap.end() )
	{
		paramMap["width"]->GetFloat( pswidth );
		ptwidth = pswidth;
	}
	else
	{
		if( paramMap.find("swidth") != paramMap.end() )
			paramMap["swidth"]->GetFloat( pswidth );
		if( paramMap.find("twidth") != paramMap.end() )
			paramMap["twidth"]->GetFloat( ptwidth );
	}
	if( paramMap.find("blur") != paramMap.end() )
	{
		paramMap["blur"]->GetFloat( sblur );
		tblur = sblur;
	}
	else
	{
		if( paramMap.find("sblur") != paramMap.end() )
			paramMap["sblur"]->GetFloat( sblur );
		if( paramMap.find("tblur") != paramMap.end() )
			paramMap["tblur"]->GetFloat( tblur );
	}
	
	swidth *= pswidth;
	twidth *= ptwidth;

	// T(s2,t2)-T(s2,t1)-T(s1,t2)+T(s1,t1)
	TqInt i;

	val.resize( m_SamplesPerPixel );

	TqFloat ss1 = s1 - swidth - (sblur*0.5f);
	TqFloat tt1 = t1 - twidth - (tblur*0.5f);
	TqFloat ss2 = s1 + swidth + (sblur*0.5f);
	TqFloat tt2 = t1 + twidth + (tblur*0.5f);

	m_tempval1 = 0.0f;
	m_tempval2 = 0.0f;
	m_tempval3 = 0.0f;
	m_tempval4 = 0.0f;


	if ( m_smode == WrapMode_Periodic )
	{
		ss1 = fmod( ss1, 1.0f );
		if ( ss1 < 0 ) ss1 += 1.0f;
		ss2 = fmod( ss2, 1.0f );
		if ( ss2 < 0 ) ss2 += 1.0f;
	}
	if ( m_tmode == WrapMode_Periodic )
	{
		tt1 = fmod( tt1, 1.0f );
		if ( tt1 < 0 ) tt1 += 1.0f;
		tt2 = fmod( tt2, 1.0f );
		if ( tt2 < 0 ) tt2 += 1.0f;
	}

	if ( m_smode == WrapMode_Black )
	{
		if ( ( ss1 < 0.0f ) ||
		        ( ss2 < 0.0f ) ||
		        ( ss2 > 1.0f ) ||
		        ( ss1 > 1.0f ) ) return ;
	}
	if ( m_tmode == WrapMode_Black )
	{
		if ( ( tt1 < 0.0f ) ||
		        ( tt2 < 0.0f ) ||
		        ( tt2 > 1.0f ) ||
		        ( tt1 > 1.0f ) ) return ;
	}

	if ( m_smode == WrapMode_Clamp || Type() == MapType_Environment )
	{
		ss1 = CLAMP( ss1, 0.0f, 1.0f );
		ss2 = CLAMP( ss2, 0.0f, 1.0f );
	}
	if ( m_tmode == WrapMode_Clamp || Type() == MapType_Environment )
	{
		tt1 = CLAMP( tt1, 0.0f, 1.0f );
		tt2 = CLAMP( tt2, 0.0f, 1.0f );
	}

	// If no boundaries are crossed, just do a single sample (the most common case)
	if ( ( ss1 < ss2 ) && ( tt1 < tt2 ) )
	{
		GetSample( ss1, tt1, ss2, tt2, val );
	}
	// If it crosses only the s boundary, we need to get two samples.
	else if ( ( ss1 > ss2 ) && ( tt1 < tt2 ) )
	{
		GetSample( 0, tt1, ss2, tt2, m_tempval1 );
		GetSample( ss1, tt1, 1.0f, tt2, m_tempval2 );
		val = ( m_tempval1 + m_tempval2 );
		val *= 0.5f;

	}
	// If it crosses only the t boundary, we need to get two samples.
	else if ( ( ss1 < ss2 ) && ( tt1 > tt2 ) )
	{
		GetSample( ss1, 0, ss2, tt2, m_tempval1 );
		GetSample( ss1, tt1, ss2, 1.0f, m_tempval2 );
		val = ( m_tempval1 + m_tempval2 );
		val *= 0.5f;

	}
	// If it crosses the s and t boundary, we need to get four samples.
	else
	{
		GetSample( 0, 0, ss2, tt2, m_tempval1 );
		GetSample( ss1, 0, 1.0f, tt2, m_tempval2 );
		GetSample( 0, tt1, ss2, 1.0f, m_tempval3 );
		GetSample( ss1, tt1, 1.0f, 1.0f, m_tempval4 );
		val = ( m_tempval1 + m_tempval2 + m_tempval3 + m_tempval4 );
		val *= 0.25f;
	}


	// Clamp the result
	for ( i = 0; i < m_SamplesPerPixel; i++ )
		val[ i ] = CLAMP( val[ i ], 0.0f, 1.0f );
}
//----------------------------------------------------------------------
/** this is used for remove any memory exceed the command Option "limits" "texturememory"
  * directive
  *   
  * It zaps the m_apSegments for this TextureMap object completely.
  * The idea here is to erase any "GetBuffer()" memory to respect the directive
  * It looks into the big m_TextureMap_Cache release every things
  **/

void CqTextureMap::CriticalMeasure()
{
	const TqInt * poptMem = QGetRenderContextI() ->GetIntegerOption( "limits", "texturememory" );
#ifdef _DEBUG
	TqChar warnings[ 400 ];
#endif
	TqInt current, limit, now;
	std::vector<CqTextureMap*>::iterator i;
	std::vector<CqTextureMapBuffer*>::iterator j;

	limit = MEG1;
	if ( poptMem )
		limit = poptMem[ 0 ] * 1024;

	now = QGetRenderContext() ->Stats().GetTextureMemory();

	if ( m_critical )
	{

		/* Extreme case no more memory to play */

		/* It is time to delete some tile's memory associated with
		               * texturemap objects.
		 *
		 * In principle the oldest texturemaps are freed first 
		               * (regardless of their current usage. Therefore this method 
		               * could only be 
		               * called at a place where the fact of release 
		               * texturemapbuffer will not impact the subsequent 
		               * GetBuffer() calls.
		 *
		 */
		for ( i = m_TextureMap_Cache.begin(); i != m_TextureMap_Cache.end(); i++ )
		{
			for ( j = ( *i ) ->m_apSegments.begin(); j != ( *i ) ->m_apSegments.end(); j++ )
			{
				if ( *j == previous ) previous = NULL;
				( *j ) ->Release();
			}
			( *i ) ->m_apSegments.resize( 0 );
			current = QGetRenderContext() ->Stats().GetTextureMemory();
			if ( ( now - current ) > ( limit / 4 ) ) break;
		}

	}
	current = QGetRenderContext() ->Stats().GetTextureMemory();

	m_critical = TqFalse;

#ifdef _DEBUG

	sprintf( warnings, "I was forced to zap the tile segment buffers for %dK",
	         ( now - current ) / 1024 );
	if ( now - current )
		RiErrorPrint( 0, 1, warnings );
#endif

}
void CqTextureMap::GetSample( TqFloat u1, TqFloat v1, TqFloat u2, TqFloat v2, std::valarray<TqFloat>& val )
{
	// u/v is average of sample positions.
	TqFloat u = ( ( u2 - u1 ) * 0.5f ) + u1;
	TqFloat v = ( ( v2 - v1 ) * 0.5f ) + v1;

	// Calculate the appropriate mipmap level from the area subtended by the sample.
	TqFloat UVArea = ( u2 - u1 ) * ( v2 - v1 );
	TqFloat d = sqrt( fabs( UVArea ) );

	// Find out which two layers the of the pyramid d lies between.
	d = CLAMP( d, 0.0, 1.0 );
	// Adjust u and v for the pyramid level.
	TqUint idu = FLOOR( d * m_XRes );
	TqUint idv = FLOOR( d * m_YRes );
	TqUint id = 0;
	TqBool singlelevel = ( ( idu == 0 ) || ( idu >= ( m_XRes / 2 ) ) || ( idv == 0 ) || ( idv >= ( m_YRes / 2 ) ) );
	TqInt umapsize, vmapsize;
	for ( umapsize = m_XRes, vmapsize = m_YRes; idu > 1 && idv > 1; idu >>= 1, umapsize >>= 1, idv >>= 1, vmapsize >>= 1, id++ );

	TqUint iu = FLOOR( u * umapsize );
	TqFloat ru = u * umapsize - iu;
	TqUint iu_n = iu + 1;
	iu = iu % umapsize;		/// \todo This is wrap mode periodic.
	iu_n = iu_n % umapsize;	/// \todo This is wrap mode periodic.

	TqUint iv = FLOOR( v * vmapsize );
	TqFloat rv = v * vmapsize - iv;
	TqUint iv_n = iv + 1;
	iv = iv % vmapsize;		/// \todo This is wrap mode periodic.
	iv_n = iv_n % vmapsize;	/// \todo This is wrap mode periodic.

	
	// Read in the relevant texture tiles.
	CqTextureMapBuffer* pTMBa = GetBuffer( iu, iv, id );		// Val00
	CqTextureMapBuffer* pTMBb = pTMBa;
    if (iv != iv_n) 
    {
       pTMBb = GetBuffer( iu, iv_n, id );	// Val01
    }
     
	CqTextureMapBuffer* pTMBc = pTMBa;
    if (iu_n != iu)
    {
		pTMBc = GetBuffer( iu_n, iv, id );	// Val10
    }
	CqTextureMapBuffer* pTMBd = NULL;
	if ( iv == iv_n) pTMBd = pTMBc;
	else if (iu == iu_n) pTMBd = pTMBb;
	else
		pTMBd = GetBuffer( iu_n, iv_n, id);	// Val11

	TqInt c;
	/* cannot find anything than goodbye */
	if ( !pTMBa || !pTMBb || !pTMBc || !pTMBd )
	{
		TqChar warnings[ 400 ];
		for ( c = 0; c < m_SamplesPerPixel; c++ )
			val[c] = 1.0f;

		sprintf( warnings, "Cannot find value for either pTMPB[a,b,c,d]" );
		RiErrorPrint( 0, 1, warnings );
		return ;
	}

	// all the tile are using the same size therefore the number is ok
	long rowlen = pTMBa->Width() * m_SamplesPerPixel;

	// Store the sample positions forl later use if need be.
	TqUint iu_c = iu;
	TqUint iv_c = iv;

	// Bilinear intepolate the values at the corners of the sample.
	iu -= pTMBa->sOrigin();
	iu_n -= pTMBc->sOrigin();
	iv -= pTMBa->tOrigin();
	iv_n -= pTMBb->tOrigin();

	for ( c = 0; c < m_SamplesPerPixel; c++ )
	{
		TqFloat Val00 = pTMBa->GetValue(iu,iv,c);
		TqFloat Val01 = pTMBb->GetValue(iu,iv_n,c);
		TqFloat Val10 = pTMBc->GetValue(iu_n,iv,c);
		TqFloat Val11 = pTMBd->GetValue(iu_n,iv_n,c);
		TqFloat bot = Val00 + ( ru * ( Val10 - Val00 ) );
		TqFloat top = Val01 + ( ru * ( Val11 - Val01 ) );
		m_low_color[ c ] = bot + ( rv * ( top - bot ) );
	}


	if ( singlelevel )
	{
		val = m_low_color;
	}
	else
	{
		umapsize >>= 1;
		vmapsize >>= 1;

		ru = ( ru + ( iu_c % 2 ) ) / 2;
		iu = iu_c >> 1;
		iu_n = iu + 1;
		iu = iu % umapsize;		/// \todo This is wrap mode periodic.
		iu_n = iu_n % umapsize;	/// \todo This is wrap mode periodic.

		rv = ( rv + ( iv_c % 2 ) ) / 2;
		iv = iv_c >> 1;
		iv_n = iv + 1;
		iv = iv % vmapsize;		/// \todo This is wrap mode periodic.
		iv_n = iv_n % vmapsize;	/// \todo This is wrap mode periodic.

		// Read in the relevant texture tiles.
		CqTextureMapBuffer* pTMBa = GetBuffer( iu, iv, id + 1 );		// Val00
		CqTextureMapBuffer* pTMBb = pTMBa;
        if (iv != iv_n) 
        {
           	pTMBb = GetBuffer( iu, iv_n, id + 1 );	// Val01
        }
 
		CqTextureMapBuffer* pTMBc = pTMBa;
        if (iu_n != iu)
        {
	   		pTMBc = GetBuffer( iu_n, iv, id + 1 );	// Val10
        }
		CqTextureMapBuffer* pTMBd = NULL;
		if ( iv == iv_n) pTMBd = pTMBc;
		else if (iu == iu_n) pTMBd = pTMBb;
		else
			pTMBd = GetBuffer( iu_n, iv_n, id + 1);	// Val11

		/* cannot find anything than goodbye */
		if ( !pTMBa || !pTMBb || !pTMBc || !pTMBd )
		{
			TqChar warnings[ 400 ];

			val = m_low_color;
			sprintf( warnings, "Cannot find value for either pTMPB[a,b,c,d]" );
			RiErrorPrint( 0, 1, warnings );
			return ;
		}

		// all the tile are using the same size therefore the number is ok
		long rowlen = pTMBa->Width() * m_SamplesPerPixel;

		// Bilinear intepolate the values at the corners of the sample.
		iu -= pTMBa->sOrigin();
		iu_n -= pTMBc->sOrigin();
		iv -= pTMBa->tOrigin();
		iv_n -= pTMBb->tOrigin();

		TqInt c;
		for ( c = 0; c < m_SamplesPerPixel; c++ )
		{
			TqFloat Val00 = pTMBa->GetValue(iu,iv,c);
			TqFloat Val01 = pTMBb->GetValue(iu,iv_n,c);
			TqFloat Val10 = pTMBc->GetValue(iu_n,iv,c);
			TqFloat Val11 = pTMBd->GetValue(iu_n,iv_n,c);
			TqFloat bot = Val00 + ( ru * ( Val10 - Val00 ) );
			TqFloat top = Val01 + ( ru * ( Val11 - Val01 ) );
			m_high_color[ c ] = bot + ( rv * ( top - bot ) );
		}

		// Linearly interpolate between low_color and high_color by dinterp.

		TqFloat dinterp = ( MAX( umapsize, vmapsize ) * d ) - 1;
		for ( c = 0; c < m_SamplesPerPixel; c++ )
			val[ c ] = m_low_color[ c ] + dinterp * ( m_high_color[ c ] - m_low_color[ c ] );



	}
	
}

//----------------------------------------------------------------------
/** Retrieve a sample from the MIP MAP over the area specified by the four vertices
 */

void CqTextureMap::SampleMap( TqFloat s1, TqFloat t1, TqFloat s2, TqFloat t2, TqFloat s3, TqFloat t3, TqFloat s4, TqFloat t4,
                              std::valarray<TqFloat>& val, std::map<std::string, IqShaderData*>& paramMap )
{
	// Work out the width and height
	TqFloat ss1, tt1, ss2, tt2;
	ss1 = MIN( MIN( MIN( s1, s2 ), s3 ), s4 );
	tt1 = MIN( MIN( MIN( t1, t2 ), t3 ), t4 );
	ss2 = MAX( MAX( MAX( s1, s2 ), s3 ), s4 );
	tt2 = MAX( MAX( MAX( t1, t2 ), t3 ), t4 );

	TqFloat swidth = ss2 - ss1;
	TqFloat twidth = tt2 - tt1;
	ss1 = ss1 + ( swidth * 0.5f );
	tt1 = tt1 + ( twidth * 0.5f );

	SampleMap( ss1, tt1, swidth, twidth, val, paramMap );
}



//----------------------------------------------------------------------
/** Retrieve a color sample from the environment map using R as the reflection vector.
 * Filtering is done using swidth, twidth and nsamples.
 */

void CqEnvironmentMap::SampleMap( CqVector3D& R1, CqVector3D& swidth, CqVector3D& twidth,
                                  std::valarray<TqFloat>& val, std::map<std::string, IqShaderData*>& paramMap )
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

			SampleMap( R1, R2, R3, R4, val, paramMap );
		}
		else if ( Type() == MapType_LatLong )
		{
			CqVector3D V = R1;
			V.Unit();
			TqFloat ss1, tt1;
			TqFloat sswidth = swidth.Magnitude();
			TqFloat stwidth = twidth.Magnitude();

			ss1 = atan2( V.y(), V.x() ) / ( 2.0 * RI_PI );  /* -.5 -> .5 */
			ss1 = ss1 + 0.5; /* remaps to 0 -> 1 */
			tt1 = acos( -V.z() ) / RI_PI;

			CqTextureMap::SampleMap( ss1, tt1, sswidth, stwidth, val, paramMap );
		}
	}
}


//----------------------------------------------------------------------
/** Retrieve a sample from the environment map using R as the reflection vector.
 */

void CqEnvironmentMap::SampleMap( CqVector3D& R1, CqVector3D& R2, CqVector3D& R3, CqVector3D& R4,
                                  std::valarray<TqFloat>& val, std::map<std::string, IqShaderData*>& paramMap )
{
	if ( m_pImage != 0 )
	{

		CqVector3D	last_R, R, pt;
		TqFloat	texture_area, total_area;
		TqInt	i, j;
		TqInt	projection_bit;		// Stores all the faces the reflected beam projects onto.
		TqInt	edge, last_face, current_face, project_face;

		CqVector3D	vertex_list[ 4 ] =
		    {
		        R1,
		        R2,
		        R3,
		        R4
		    };

		val.resize( m_SamplesPerPixel );

		vertex_list[ 0 ].Unit();
		vertex_list[ 1 ].Unit();
		vertex_list[ 2 ].Unit();
		vertex_list[ 3 ].Unit();

		cube_no = 0;
		projection_bit = 0;
		total_area = 0.0;

		// Find intersection the reflected vector from the last vertex in the list makes with the cube.
		last_R = vertex_list[ 3 ];

		get_face_intersection( &last_R, &cube[ cube_no ], &last_face );
		cube_no++;
		projection_bit |= last_face;

		for ( i = 0; i < 4; i++ )
		{
			R = vertex_list[ i ];
			get_face_intersection( &R, &pt, &current_face );

			// If the last reflected ray intersected a face different from the current
			// ray, we must find the intersection the beam makes with the corresponding edge.
			if ( current_face != last_face )
			{
				edge = current_face | last_face;
				get_edge_intersection( &last_R, &R, edge, &cube[ cube_no ] );
				cube_no++;
				projection_bit |= current_face;
			}
			cube[ cube_no ] = pt;
			cube_no++;
			last_face = current_face;
			last_R = R;
		}

		std::valarray<TqFloat>	run_val;
		run_val.resize( m_SamplesPerPixel );
		run_val = 0.0f;
		val = 0.0f;

		TqFloat facewidth = 1.0f / 3.0f;
		TqFloat faceheight = 1.0f * 0.5f;

		for ( i = 0, project_face = 1; i < 6; i++, project_face += project_face )
		{
			if ( project_face & projection_bit )
			{
				// Get the projection in UVSpace on the project_face
				project( project_face );
				TqFloat s1, t1, s2, t2;
				s1 = s2 = uv[ 0 ][ 0 ];
				t1 = t2 = uv[ 0 ][ 1 ];
				texture_area = 0.0;

				for ( j = 1; j < cube_no; j++ )
				{
					if ( uv[ j ][ 0 ] < s1 ) s1 = uv[ j ][ 0 ];
					if ( uv[ j ][ 1 ] < t1 ) t1 = uv[ j ][ 1 ];
					if ( uv[ j ][ 0 ] > s2 ) s2 = uv[ j ][ 0 ];
					if ( uv[ j ][ 1 ] > t2 ) t2 = uv[ j ][ 1 ];
				}
				texture_area = ( s2 - s1 ) * ( t2 - t1 );
				if ( texture_area <= 0.0 ) texture_area = 1.0f;

				// Adjust for the appropriate section of the cube map.
				// Clamp to the cubemap face, this is OK, as the wrap over a boundary is handled
				// by the preceeding code which gets multiple samples for the cube if a sample crosses
				// edges.
				s1 = CLAMP( s1, 0.0f, 1.0f );
				s2 = CLAMP( s2, 0.0f, 1.0f );
				t1 = CLAMP( t1, 0.0f, 1.0f );
				t2 = CLAMP( t2, 0.0f, 1.0f );
				s1 = ( s1 * facewidth ) + ( facewidth * ( i % 3 ) );
				s2 = ( s2 * facewidth ) + ( facewidth * ( i % 3 ) );
				t1 = ( t1 * faceheight ) + ( faceheight * ( i / 3 ) );
				t2 = ( t2 * faceheight ) + ( faceheight * ( i / 3 ) );

				GetSample( s1, t1, s2, t2, run_val );

				// Add the color contribution weighted by the area
				val += run_val * texture_area;
				total_area += texture_area;
			}
		}
		// Normalize the weightings
		val /= total_area;
	}

}


static void get_face_intersection( CqVector3D *normal, CqVector3D *pt, TqInt* face )
{
	CqVector3D n = *normal;
	TqFloat t;

	// Test nz direction
	if ( n.z() < 0 )       	// Test intersection with nz
	{
		t = -0.5 / n.z();
		pt->x( n.x() * t );
		pt->y( n.y() * t );
		pt->z( n.z() * t );
		if ( fabs( pt->x() ) < 0.5 && fabs( pt->y() ) < 0.5 )
		{
			*face = nz;
			return ;
		}
	}
	else if ( n.z() > 0 )       	// Test intersection with pz
	{
		t = 0.5 / n.z();
		pt->x( n.x() * t );
		pt->y( n.y() * t );
		pt->z( n.z() * t );
		if ( fabs( pt->x() ) < 0.5 && fabs( pt->y() ) < 0.5 )
		{
			*face = pz;
			return ;
		}
	}


	// Test ny direction
	if ( n.y() < 0 )       	// Test intersection with ny
	{
		t = -0.5 / n.y();
		pt->x( n.x() * t );
		pt->y( n.y() * t );
		pt->z( n.z() * t );
		if ( fabs( pt->x() ) < 0.5 && fabs( pt->z() ) < 0.5 )
		{
			*face = ny;
			return ;
		}
	}
	else if ( n.y() > 0 )       	// Test intersection with py
	{
		t = 0.5 / n.y();
		pt->x( n.x() * t );
		pt->y( n.y() * t );
		pt->z( n.z() * t );
		if ( fabs( pt->x() ) < 0.5 && fabs( pt->z() ) < 0.5 )
		{
			*face = py;
			return ;
		}
	}

	// Test nx direction
	if ( n.x() < 0 )       	// Test intersection with nx
	{
		t = -0.5 / n.x();
		pt->x( n.x() * t );
		pt->y( n.y() * t );
		pt->z( n.z() * t );
		if ( fabs( pt->y() ) < 0.5 && fabs( pt->z() ) < 0.5 )
		{
			*face = nx;
			return ;
		}
	}
	else if ( n.x() > 0 )       	// Test intersection with px
	{
		t = 0.5 / n.x();
		pt->x( n.x() * t );
		pt->y( n.y() * t );
		pt->z( n.z() * t );
		if ( fabs( pt->y() ) < 0.5 && fabs( pt->z() ) < 0.5 )
		{
			*face = px;
			return ;
		}
	}
}

static void get_edge_intersection( CqVector3D* n1, CqVector3D* n2, TqInt edge, CqVector3D* pt )
{
	TqFloat a, b, c;
	TqFloat x0, y0, z0, f, g, h;
	TqFloat denom, t;

	// Get plane eqn: ax+by+cz=0 from two normals n1 and n2
	a = n1->y() * n2->z() - n1->z() * n2->y();
	b = n1->z() * n2->x() - n1->x() * n2->z();
	c = n1->x() * n2->y() - n1->y() * n2->x();

	// Set up line equation of edge.
	x0 = y0 = z0 = 0.0;
	f = g = h = 0.0;
	switch ( edge )
	{
			case edge01: x0 = z0 = 0.5; g = 1; break;
			case edge02: y0 = z0 = 0.5; f = 1; break;
			case edge03: x0 = -0.5; z0 = 0.5; g = 1; break;
			case edge04: y0 = -0.5; z0 = 0.5; f = 1; break;
			case edge12: x0 = y0 = 0.5; h = 1; break;
			case edge23: x0 = -0.5; y0 = 0.5; h = 1; break;
			case edge34: x0 = y0 = -0.5; h = 1; break;
			case edge41: x0 = 0.5; y0 = -0.5; h = 1; break;
			case edge51: x0 = 0.5; z0 = -0.5; g = 1; break;
			case edge52: y0 = 0.5; z0 = -0.5; f = 1; break;
			case edge53: x0 = z0 = -0.5; g = 1; break;
			case edge54: y0 = z0 = -0.5; f = 1; break;
	}

	// Return the intersection of the plane and edge
	denom = a * f + b * g + c * h;
	t = -( a * x0 + b * y0 + c * z0 ) / denom;
	pt->x( x0 + f * t );
	pt->y( y0 + g * t );
	pt->z( z0 + h * t );
}


static void project( TqInt face )
{
	TqInt i;
	switch ( face )
	{
			case pz:
			for ( i = 0; i < cube_no; i++ )
			{
				uv[ i ][ 0 ] = cube[ i ].x() + 0.5;
				uv[ i ][ 1 ] = -cube[ i ].y() + 0.5;
			}
			break;

			case px:
			for ( i = 0; i < cube_no; i++ )
			{
				uv[ i ][ 0 ] = -cube[ i ].z() + 0.5;
				uv[ i ][ 1 ] = -cube[ i ].y() + 0.5;
			}
			break;

			case py:
			for ( i = 0; i < cube_no; i++ )
			{
				uv[ i ][ 0 ] = cube[ i ].x() + 0.5;
				uv[ i ][ 1 ] = cube[ i ].z() + 0.5;
			}
			break;

			case nx:
			for ( i = 0; i < cube_no; i++ )
			{
				uv[ i ][ 0 ] = cube[ i ].z() + 0.5;
				uv[ i ][ 1 ] = -cube[ i ].y() + 0.5;
			}
			break;

			case ny:
			for ( i = 0; i < cube_no; i++ )
			{
				uv[ i ][ 0 ] = cube[ i ].x() + 0.5;
				uv[ i ][ 1 ] = -cube[ i ].z() + 0.5;
			}
			break;

			case nz:
			for ( i = 0; i < cube_no; i++ )
			{
				uv[ i ][ 0 ] = -cube[ i ].x() + 0.5;
				uv[ i ][ 1 ] = -cube[ i ].y() + 0.5;
			}
			break;
	}
}


#define	dsres		1.0f
#define	dtres		1.0f
#define	ResFactor	1.0f
#define	MinSize		0.0f
#define	NumSamples	16
#define	MinSamples	3

//---------------------------------------------------------------------
/** Sample the shadow map data to see if the point vecPoint is in shadow.
 */

void CqShadowMap::SampleMap( CqVector3D& vecPoint, CqVector3D& swidth, CqVector3D& twidth, std::valarray<TqFloat>& val, std::map<std::string, IqShaderData*>& paramMap )
{
	if ( m_pImage != 0 )
	{
		TqFloat pswidth = 1.0f;
		TqFloat ptwidth = 1.0f;

		// Get parameters out of the map.
		if( paramMap.find("width") != paramMap.end() )
		{
			paramMap["width"]->GetFloat( pswidth );
			ptwidth = pswidth;
		}
		else
		{
			if( paramMap.find("swidth") != paramMap.end() )
				paramMap["swidth"]->GetFloat( pswidth );
			if( paramMap.find("twidth") != paramMap.end() )
				paramMap["twidth"]->GetFloat( ptwidth );
		}
		
		swidth *= pswidth;
		twidth *= ptwidth;

		CqVector3D	R1, R2, R3, R4;
		R1 = vecPoint - ( swidth / 2.0f ) - ( twidth / 2.0f );
		R2 = vecPoint + ( swidth / 2.0f ) - ( twidth / 2.0f );
		R3 = vecPoint - ( swidth / 2.0f ) + ( twidth / 2.0f );
		R4 = vecPoint + ( swidth / 2.0f ) + ( twidth / 2.0f );

		SampleMap( R1, R2, R3, R4, val, paramMap );
	} else 
	{
		// If no map defined, not in shadow.
		val.resize( 1 );
		val[ 0 ] = 0.0f;
	}
}


void	CqShadowMap::SampleMap( CqVector3D& R1, CqVector3D& R2, CqVector3D& R3, CqVector3D& R4, std::valarray<TqFloat>& val, std::map<std::string, IqShaderData*>& paramMap )
{
	// Check the memory and make sure we don't abuse it
	CriticalMeasure();

	TqFloat sblur = 0.0f;
	TqFloat tblur = 0.0f;
	TqFloat samples = 0.0f;

	// Get parameters out of the map.
	if( paramMap.find("blur") != paramMap.end() )
	{
		paramMap["blur"]->GetFloat( sblur );
		tblur = sblur;
	}
	else
	{
		if( paramMap.find("sblur") != paramMap.end() )
			paramMap["sblur"]->GetFloat( sblur );
		if( paramMap.find("tblur") != paramMap.end() )
			paramMap["tblur"]->GetFloat( tblur );
	}
	if( paramMap.find("samples") != paramMap.end() )
		paramMap["samples"]->GetFloat( samples );

	// If no map defined, not in shadow.
	val.resize( 1 );
	val[ 0 ] = 0.0f;

	CqVector3D	vecR1l, vecR2l, vecR3l, vecR4l;
	CqVector3D	vecR1m, vecR2m, vecR3m, vecR4m;

	// Add in the bias at this point in camera coordinates.
	TqFloat bias0 = 0.0f;
	const TqFloat* poptBias = QGetRenderContextI() ->GetFloatOption( "shadow", "bias0" );
	if ( poptBias != 0 )
		bias0 = poptBias[ 0 ];

	TqFloat bias1 = 0.0f;
	poptBias = QGetRenderContextI() ->GetFloatOption( "shadow", "bias1" );
	if ( poptBias != 0 )
		bias1 = poptBias[ 0 ];

	static CqRandom random( 42 );
	TqFloat bias;


	if ( bias1 >= bias0 )
	{
		bias = random.RandomFloat( bias1 - bias0 ) + bias0;
		if ( bias > bias1 ) bias = bias1;
	}
	else
	{
		bias = random.RandomFloat( bias0 - bias1 ) + bias1;
		if ( bias > bias0 ) bias = bias0;
	}
	CqVector3D vecBias( 0, 0, bias );
	// Generate a matrix to transform points from camera space into the space of the light source used in the
	// definition of the shadow map.
	CqMatrix matCameraToLight = m_matWorldToCamera * QGetRenderContextI() ->matSpaceToSpace( "camera", "world" );
	// Generate a matrix to transform points from camera space into the space of the shadow map.
	CqMatrix matCameraToMap = m_matWorldToScreen * QGetRenderContextI() ->matSpaceToSpace( "camera", "world" );

	vecR1l = matCameraToLight * ( R1 - vecBias );
	vecR2l = matCameraToLight * ( R2 - vecBias );
	vecR3l = matCameraToLight * ( R3 - vecBias );
	vecR4l = matCameraToLight * ( R4 - vecBias );

	vecR1m = matCameraToMap * ( R1 - vecBias );
	vecR2m = matCameraToMap * ( R2 - vecBias );
	vecR3m = matCameraToMap * ( R3 - vecBias );
	vecR4m = matCameraToMap * ( R4 - vecBias );

	TqFloat z1 = vecR1l.z();
	TqFloat z2 = vecR2l.z();
	TqFloat z3 = vecR3l.z();
	TqFloat z4 = vecR4l.z();
	TqFloat z = ( z1 + z2 + z3 + z4 ) * 0.25;

	TqFloat sbo2 = ( sblur * 0.5f ) * m_XRes;
	TqFloat tbo2 = ( tblur * 0.5f ) * m_YRes;

	// If point is behind light, call it not in shadow.
	//if(z1<0.0)	return;

	TqFloat xro2 = m_XRes * 0.5;
	TqFloat yro2 = m_YRes * 0.5;

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

	if ( lu > m_XRes || hu < 0 || lv > m_YRes || hv < 0 )
		return ;

	TqFloat sres = hu - lu;
	TqFloat tres = hv - lv;

	// Calculate no. of samples.
	TqInt nt, ns;
	if( samples > 0 )
	{
		nt = ns = static_cast<TqInt>( sqrt( samples ) );
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
	TqFloat ds =       /*2.0f**/sres / ns;
	TqFloat dt =       /*2.0f**/tres / nt;
	TqFloat js = ds * 0.5f;
	TqFloat jt = dt * 0.5f;

	// Test the samples.
	TqInt inshadow = 0;

	TqFloat s = lu;
	TqInt i;
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
			// Clip to bounding box.
			if ( iu >= 0 && iu < m_XRes &&
			        iv >= 0 && iv < m_YRes )
			{
				CqTextureMapBuffer * pTMBa = GetBuffer( iu, iv, 0 );
				if ( pTMBa != 0 && pTMBa->pBufferData() != 0 )
				{
					iu -= pTMBa->sOrigin();
					iv -= pTMBa->tOrigin();
					TqInt rowlen = pTMBa->Width();
					TqFloat *depths = ( TqFloat * ) pTMBa->pBufferData();
					if ( z > pTMBa->GetValue(iu, iv, 0) )
						inshadow += 1;
				}
			}
			t = t + dt;
		}
		s = s + ds;
	}

	val[ 0 ] = ( static_cast<TqFloat>( inshadow ) / ( ns * nt ) );
}


//---------------------------------------------------------------------
/** Allocate the memory required by the depthmap.
 */

void CqShadowMap::AllocateMap( TqInt XRes, TqInt YRes )
{
	static CqRandom rand;

	std::vector<CqTextureMapBuffer*>::iterator s;
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
			ofile.write( reinterpret_cast<TqPchar>( m_matWorldToCamera[ 0 ] ), sizeof( m_matWorldToCamera[ 0 ][ 0 ] ) * 4 );
			ofile.write( reinterpret_cast<TqPchar>( m_matWorldToCamera[ 1 ] ), sizeof( m_matWorldToCamera[ 0 ][ 0 ] ) * 4 );
			ofile.write( reinterpret_cast<TqPchar>( m_matWorldToCamera[ 2 ] ), sizeof( m_matWorldToCamera[ 0 ][ 0 ] ) * 4 );
			ofile.write( reinterpret_cast<TqPchar>( m_matWorldToCamera[ 3 ] ), sizeof( m_matWorldToCamera[ 0 ][ 0 ] ) * 4 );

			ofile.write( reinterpret_cast<TqPchar>( m_matWorldToScreen[ 0 ] ), sizeof( m_matWorldToScreen[ 0 ][ 0 ] ) * 4 );
			ofile.write( reinterpret_cast<TqPchar>( m_matWorldToScreen[ 1 ] ), sizeof( m_matWorldToScreen[ 0 ][ 0 ] ) * 4 );
			ofile.write( reinterpret_cast<TqPchar>( m_matWorldToScreen[ 2 ] ), sizeof( m_matWorldToScreen[ 0 ][ 0 ] ) * 4 );
			ofile.write( reinterpret_cast<TqPchar>( m_matWorldToScreen[ 3 ] ), sizeof( m_matWorldToScreen[ 0 ][ 0 ] ) * 4 );

			// Now output the depth values
			ofile.write( reinterpret_cast<TqPchar>( m_apSegments[ 0 ] ->pBufferData() ), sizeof( TqFloat ) * ( m_XRes * m_YRes ) );
			ofile.close();
		}
	}
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
				CqString strErr( "Error : Invalid shadowmap format - " );
				strErr += m_strName;
				CqBasicError( ErrorID_InvalidShadowMap, Severity_Normal, strErr.c_str() );
				return ;
			}

			// Save the xres and yres.
			file.read( reinterpret_cast<TqPchar >( &m_XRes ), sizeof( m_XRes ) );
			file.read( reinterpret_cast<TqPchar >( &m_YRes ), sizeof( m_YRes ) );

			// Save the transformation matrices.
			file.read( reinterpret_cast<TqPchar>( m_matWorldToCamera[ 0 ] ), sizeof( m_matWorldToCamera[ 0 ][ 0 ] ) * 4 );
			file.read( reinterpret_cast<TqPchar>( m_matWorldToCamera[ 1 ] ), sizeof( m_matWorldToCamera[ 0 ][ 0 ] ) * 4 );
			file.read( reinterpret_cast<TqPchar>( m_matWorldToCamera[ 2 ] ), sizeof( m_matWorldToCamera[ 0 ][ 0 ] ) * 4 );
			file.read( reinterpret_cast<TqPchar>( m_matWorldToCamera[ 3 ] ), sizeof( m_matWorldToCamera[ 0 ][ 0 ] ) * 4 );

			file.read( reinterpret_cast<TqPchar>( m_matWorldToScreen[ 0 ] ), sizeof( m_matWorldToScreen[ 0 ][ 0 ] ) * 4 );
			file.read( reinterpret_cast<TqPchar>( m_matWorldToScreen[ 1 ] ), sizeof( m_matWorldToScreen[ 0 ][ 0 ] ) * 4 );
			file.read( reinterpret_cast<TqPchar>( m_matWorldToScreen[ 2 ] ), sizeof( m_matWorldToScreen[ 0 ][ 0 ] ) * 4 );
			file.read( reinterpret_cast<TqPchar>( m_matWorldToScreen[ 3 ] ), sizeof( m_matWorldToScreen[ 0 ][ 0 ] ) * 4 );

			// Now output the depth values
			AllocateMap( m_XRes, m_YRes );
			file.read( reinterpret_cast<TqPchar>( m_apSegments[ 0 ] ->pBufferData() ), sizeof( TqFloat ) * ( m_XRes * m_YRes ) );

			// Set the matrixes to general, not Identity as default.
			m_matWorldToCamera.SetfIdentity( TqFalse );
			m_matWorldToScreen.SetfIdentity( TqFalse );
		}
		else
		{
			CqString strErr( "Shadow map not found " );
			strErr += m_strName;
			CqBasicError( ErrorID_FileNotFound, Severity_Normal, strErr.c_str() );
		}
	}
}


//----------------------------------------------------------------------
/** Save the shadowmap data in system specifirc image format.
 */

void CqShadowMap::SaveShadowMap( const CqString& strShadowName )
{
	TqChar version[ 80 ];

	// Save the shadowmap to a binary file.
	if ( m_strName.compare( "" ) != 0 )
	{
		if ( m_apSegments.size() != 0 )
		{
			TIFF * pshadow = TIFFOpen( strShadowName.c_str(), "w" );
			TIFFCreateDirectory( pshadow );

			// Write the transform matrices.
			TqFloat	matWorldToCamera[ 16 ];
			TqFloat	matWorldToScreen[ 16 ];
			TqInt r, c;
			for ( r = 0; r < 4; r++ )
			{
				for ( c = 0; c < 4; c++ )
				{
					matWorldToCamera[ ( r * 4 ) + c ] = m_matWorldToCamera[ r ][ c ];
					matWorldToScreen[ ( r * 4 ) + c ] = m_matWorldToScreen[ r ][ c ];
				}
			}
#if defined(AQSIS_SYSTEM_WIN32) || defined(AQSIS_SYSTEM_MACOSX)
			sprintf( version, "%s %s", STRNAME, VERSION_STR );
#else
			sprintf( version, "%s %s", STRNAME, VERSION );
#endif
			TIFFSetField( pshadow, TIFFTAG_SOFTWARE, ( uint32 ) version );
			TIFFSetField( pshadow, TIFFTAG_PIXAR_MATRIX_WORLDTOCAMERA, matWorldToCamera );
			TIFFSetField( pshadow, TIFFTAG_PIXAR_MATRIX_WORLDTOSCREEN, matWorldToScreen );
			TIFFSetField( pshadow, TIFFTAG_PIXAR_TEXTUREFORMAT, SHADOWMAP_HEADER );
			TIFFSetField( pshadow, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK );

			// Write the floating point image to the directory.
			TqFloat *depths = ( TqFloat * ) m_apSegments[ 0 ] ->pBufferData();
			WriteTileImage( pshadow, depths, XRes(), YRes(), 32, 32, 1, m_Compression, m_Quality );
			TIFFClose( pshadow );
		}
	}
}


//----------------------------------------------------------------------
/** Read the matrices out of the tiff file.
 */

void CqShadowMap::ReadMatrices()
{
	// Read the transform matrices.
	TqFloat * matWorldToCamera;
	TqFloat*	matWorldToScreen;
	TqInt reta = TIFFGetField( m_pImage, TIFFTAG_PIXAR_MATRIX_WORLDTOCAMERA, &matWorldToCamera );
	TqInt retb = TIFFGetField( m_pImage, TIFFTAG_PIXAR_MATRIX_WORLDTOSCREEN, &matWorldToScreen );
	if ( !reta || !retb )
		SetInvalid();
	else
	{
		TqInt r, c;
		for ( r = 0; r < 4; r++ )
		{
			for ( c = 0; c < 4; c++ )
			{
				m_matWorldToCamera[ r ][ c ] = matWorldToCamera[ ( r * 4 ) + c ];
				m_matWorldToScreen[ r ][ c ] = matWorldToScreen[ ( r * 4 ) + c ];
			}
		}
	}
	// Set the matrixes to general, not Identity as default.
	m_matWorldToCamera.SetfIdentity( TqFalse );
	m_matWorldToScreen.SetfIdentity( TqFalse );
}


//----------------------------------------------------------------------
/** Write an image to an open TIFF file in the current directory as tiled storage.
 * as TqFloat values
 */

void CqTextureMap::WriteTileImage( TIFF* ptex, TqFloat *raster, TqUlong width, TqUlong length, TqUlong twidth, TqUlong tlength, TqInt samples, TqInt compression, TqInt quality )
{
	//TIFFCreateDirectory(ptex);
	TqChar version[ 80 ];
#if defined(AQSIS_SYSTEM_WIN32) || defined(AQSIS_SYSTEM_MACOSX)
	sprintf( version, "%s %s", STRNAME, VERSION_STR );
#else
	sprintf( version, "%s %s", STRNAME, VERSION );
#endif
	TIFFSetField( ptex, TIFFTAG_SOFTWARE, ( uint32 ) version );
	TIFFSetField( ptex, TIFFTAG_IMAGEWIDTH, width );
	TIFFSetField( ptex, TIFFTAG_IMAGELENGTH, length );
	TIFFSetField( ptex, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG );
	TIFFSetField( ptex, TIFFTAG_BITSPERSAMPLE, 32 );
	TIFFSetField( ptex, TIFFTAG_SAMPLESPERPIXEL, samples );
	TIFFSetField( ptex, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
	TIFFSetField( ptex, TIFFTAG_TILEWIDTH, twidth );
	TIFFSetField( ptex, TIFFTAG_TILELENGTH, tlength );
	TIFFSetField( ptex, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP );
	TIFFSetField( ptex, TIFFTAG_COMPRESSION, compression );


	TqInt tsize = twidth * tlength;
	TqInt tperrow = ( width + twidth - 1 ) / twidth;
	TqFloat* ptile = static_cast<TqFloat*>( _TIFFmalloc( tsize * samples * sizeof( TqFloat ) ) );

	if ( ptile != NULL )
	{
		TqInt ctiles = tperrow * ( ( length + tlength - 1 ) / tlength );
		TqInt itile;
		for ( itile = 0; itile < ctiles; itile++ )
		{
			TqInt x = ( itile % tperrow ) * twidth;
			TqInt y = ( itile / tperrow ) * tlength;
			TqFloat* ptdata = raster + ( ( y * width ) + x ) * samples;
			// Clear the tile to black.
			memset( ptile, 0, tsize * samples * sizeof( TqFloat ) );
			for ( TqUlong i = 0; i < tlength; i++ )
			{
				for ( TqUlong j = 0; j < twidth; j++ )
				{
					if ( ( x + j ) < width && ( y + i ) < length )
					{
						TqInt ii;
						for ( ii = 0; ii < samples; ii++ )
							ptile[ ( i * twidth * samples ) + ( ( ( j * samples ) + ii ) ) ] = ptdata[ ( ( j * samples ) + ii ) ];
					}
				}
				ptdata += ( width * samples );
			}
			TIFFWriteTile( ptex, ptile, x, y, 0, 0 );
		}
		TIFFWriteDirectory( ptex );

	}
}
//----------------------------------------------------------------------
/** Write an image to an open TIFF file in the current directory as straight storage.
 * as TqFloat values
 */


void CqTextureMap::WriteImage( TIFF* ptex, TqFloat *raster, TqUlong width, TqUlong length, TqInt samples, TqInt compression, TqInt quality )
{
	TqChar version[ 80 ];
	TIFFCreateDirectory( ptex );

#if defined(AQSIS_SYSTEM_WIN32) || defined(AQSIS_SYSTEM_MACOSX)
	sprintf( version, "%s %s", STRNAME, VERSION_STR );
#else
	sprintf( version, "%s %s", STRNAME, VERSION );
#endif
	TIFFSetField( ptex, TIFFTAG_SOFTWARE, ( uint32 ) version );
	TIFFSetField( ptex, TIFFTAG_IMAGEWIDTH, width );
	TIFFSetField( ptex, TIFFTAG_IMAGELENGTH, length );
	TIFFSetField( ptex, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG );
	TIFFSetField( ptex, TIFFTAG_BITSPERSAMPLE, 32 );
	TIFFSetField( ptex, TIFFTAG_SAMPLESPERPIXEL, samples );
	TIFFSetField( ptex, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
	TIFFSetField( ptex, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP );
	TIFFSetField( ptex, TIFFTAG_COMPRESSION, compression ); /* COMPRESSION_DEFLATE */
	TIFFSetField( ptex, TIFFTAG_ROWSPERSTRIP, 1 );
	TIFFSetField( ptex, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB );

	TqFloat *pdata = raster;
	for ( TqUlong i = 0; i < length; i++ )
	{
		TIFFWriteScanline( ptex, pdata, i );
		pdata += ( width * samples );
	}
	TIFFWriteDirectory( ptex );
}
//----------------------------------------------------------------------
/** Write an image to an open TIFF file in the current directory as tiled storage.
 * as unsigned char values
 */

void CqTextureMap::WriteTileImage( TIFF* ptex, TqPuchar raster, TqUlong width, TqUlong length, TqUlong twidth, TqUlong tlength, TqInt samples, TqInt compression, TqInt quality )
{
	TqChar version[ 80 ];
#if defined(AQSIS_SYSTEM_WIN32) || defined(AQSIS_SYSTEM_MACOSX)
	sprintf( version, "%s %s", STRNAME, VERSION_STR );
#else
	sprintf( version, "%s %s", STRNAME, VERSION );
#endif
	TIFFSetField( ptex, TIFFTAG_SOFTWARE, ( uint32 ) version );
	TIFFSetField( ptex, TIFFTAG_IMAGEWIDTH, width );
	TIFFSetField( ptex, TIFFTAG_IMAGELENGTH, length );
	TIFFSetField( ptex, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG );
	TIFFSetField( ptex, TIFFTAG_BITSPERSAMPLE, 8 );
	TIFFSetField( ptex, TIFFTAG_SAMPLESPERPIXEL, samples );
	TIFFSetField( ptex, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
	TIFFSetField( ptex, TIFFTAG_TILEWIDTH, twidth );
	TIFFSetField( ptex, TIFFTAG_TILELENGTH, tlength );
	TIFFSetField( ptex, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT );
	TIFFSetField( ptex, TIFFTAG_COMPRESSION, compression );
	TIFFSetField( ptex, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB );

	TqInt tsize = twidth * tlength;
	TqInt tperrow = ( width + twidth - 1 ) / twidth;
	TqPuchar ptile = static_cast<TqPuchar>( _TIFFmalloc( tsize * samples ) );

	if ( ptile != NULL )
	{
		TqInt ctiles = tperrow * ( ( length + tlength - 1 ) / tlength );
		TqInt itile;
		for ( itile = 0; itile < ctiles; itile++ )
		{
			TqInt x = ( itile % tperrow ) * twidth;
			TqInt y = ( itile / tperrow ) * tlength;
			TqPuchar ptdata = raster + ( ( y * width ) + x ) * samples;
			// Clear the tile to black.
			memset( ptile, 0, tsize * samples );
			for ( TqUlong i = 0; i < tlength; i++ )
			{
				for ( TqUlong j = 0; j < twidth; j++ )
				{
					if ( ( x + j ) < width && ( y + i ) < length )
					{
						TqInt ii;
						for ( ii = 0; ii < samples; ii++ )
							ptile[ ( i * twidth * samples ) + ( ( ( j * samples ) + ii ) ) ] = ptdata[ ( ( j * samples ) + ii ) ];
					}
				}
				ptdata += ( width * samples );
			}
			TIFFWriteTile( ptex, ptile, x, y, 0, 0 );
		}
		TIFFWriteDirectory( ptex );
	}
}

//----------------------------------------------------------------------
/** Write an image to an open TIFF file in the current directory as straight storage.
 * as unsigned char values
 */


void CqTextureMap::WriteImage( TIFF* ptex, TqPuchar raster, TqUlong width, TqUlong length, TqInt samples, TqInt compression, TqInt quality )
{
	TqChar version[ 80 ];
	TIFFCreateDirectory( ptex );

#if defined(AQSIS_SYSTEM_WIN32) || defined(AQSIS_SYSTEM_MACOSX)
	sprintf( version, "%s %s", STRNAME, VERSION_STR );
#else
	sprintf( version, "%s %s", STRNAME, VERSION );
#endif
	TIFFSetField( ptex, TIFFTAG_SOFTWARE, ( uint32 ) version );
	TIFFSetField( ptex, TIFFTAG_IMAGEWIDTH, width );
	TIFFSetField( ptex, TIFFTAG_IMAGELENGTH, length );
	TIFFSetField( ptex, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG );
	TIFFSetField( ptex, TIFFTAG_BITSPERSAMPLE, 8 );
	TIFFSetField( ptex, TIFFTAG_SAMPLESPERPIXEL, samples );
	TIFFSetField( ptex, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
	TIFFSetField( ptex, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT );
	TIFFSetField( ptex, TIFFTAG_COMPRESSION, compression );
	TIFFSetField( ptex, TIFFTAG_ROWSPERSTRIP, 1 );

	unsigned char *pdata = raster;
	for ( TqUlong i = 0; i < length; i++ )
	{
		TIFFWriteScanline( ptex, pdata, i );
		pdata += ( width * samples );
	}
	TIFFWriteDirectory( ptex );
}


END_NAMESPACE( Aqsis )

