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
#include	"version.h"
#include	"renderer.h"
#include	"converter.h"

#ifndef		AQSIS_SYSTEM_WIN32
#include	"unistd.h"
#endif

START_NAMESPACE( Aqsis )

// Local Constants

#define MEG1    8192*1024
#undef ALLOCSEGMENTSTATUS

// Local Variables

static TqBool m_critical = TqFalse;

//---------------------------------------------------------------------
/** Static array of cached texture maps.
 */
std::vector<CqTextureMap*>	CqTextureMap::m_TextureMap_Cache;
std::vector<CqString*> CqTextureMap::m_ConvertString_Cache;

#ifdef ALLOCSEGMENTSTATUS
static TqInt alloc_cnt = 0;
static TqInt free_cnt = 0;
#endif 

// Forward definition for local functions

// Forward Forward definition for global functions
IqRenderer* QGetRenderContextI();

//---------------------------------------------------------------------
/** Allocate a cache segment to hold the specified image tile.
 */

TqPuchar CqTextureMapBuffer::AllocSegment( TqUlong width, TqUlong height, TqInt samples, TqBool fProt )
{
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


	if ( ( more > limit ) && !fProt )
	{

		// Critical level of memory will be reached;
		// I'm better starting the cleanup cache memory buffer
		// We need to zap the cache we are exceeding the required texturememory demand
		// For now, we will just warn the user the texturememory's demand will exceed the
		// request number.

		//sprintf( warnings, "Exceeding the allocated texturememory by %d",
		//         more - limit );
		if ( report )
		{
			//RiErrorPrint( 0, 1, warnings );
			QGetRenderContextI() ->Logger() ->warn( "Exceeding allocated texture memory by %d", more - limit );
		}
		
		report = 0;
		m_critical = TqTrue;
	}

#ifdef _DEBUG
	if ( ( more > MEG1 ) && ( ( more / ( 1024 * 1024 ) ) > megs ) )
	{
		QGetRenderContextI() ->Logger() ->debug( "Texturememory is more than %d megs", megs );
		megs += 10;
	}
#endif
	QGetRenderContext() ->Stats().IncTextureMemory( demand );

	// just do a malloc since the texturemapping will set individual member of the allocated buffer
	// a calloc is wastefull operation in this case.
	return ( static_cast<TqPuchar>( malloc( demand ) ) );
}
//--------------------------------------------------------------------------
/** Support for plugins mainly converter from any bitmap format to .tif file.
 */
TqInt CqTextureMap::Convert( CqString &strName )
{
	TqInt result = 0;
	TqInt lenght = 0;
	TqChar library[ 1024 ];
	TqChar function[ 1024 ];
	char * ( *convert ) ( const char * );
	char *ext = NULL;
	char *tiff = NULL;

	convert = NULL;

	char *aqsis_home = getenv( "AQSIS_BASE_PATH" );

	if ( aqsis_home == NULL )
#if defined(AQSIS_SYSTEM_POSIX) && !defined(AQSIS_SYSTEM_MACOSX)
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
	CqConverter *plug = new CqConverter( "", library, function );
	if ( ( convert = ( char * ( * ) (const  char * s ) ) plug->Function() ) != NULL )
	{

#ifdef DEBUG
		std::cout << "strName: " << strName.c_str() << std::endl;
#endif
		if ( ( tiff = convert (  strName.c_str() ) ) != NULL )
		{
			strName = tiff;
			result = 1; // success
		}
		plug->Close();
	}
	delete plug;
	return result;
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
	TqInt current, limit, now;
	std::vector<CqTextureMap*>::iterator i;
	std::list<CqTextureMapBuffer*>::iterator j;

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
				// Only release if not protected.
				if ( !( *j ) ->fProtected() )
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

	if ( now - current )
	{
		///! \todo Review this debug message
		QGetRenderContextI() ->Logger() ->info( "I was forced to zap the tile segment buffers for %dK", ( now - current ) / 1024 );
	}
#endif

}


//---------------------------------------------------------------------
/** If properly opened, close the TIFF file.
 */

void CqTextureMap::Close()
{

	if ( m_pImage != 0 ) TIFFClose( m_pImage );
	m_pImage = 0;

}




void CqTextureMap::CreateMIPMAP( TqBool fProtectBuffers )
{
	if ( m_pImage != 0 )
	{
		// Read the whole image into a buffer.
		CqTextureMapBuffer * pBuffer = GetBuffer( 0, 0, 0, fProtectBuffers );

		TqInt m_xres = m_XRes;
		TqInt m_yres = m_YRes;
		TqInt directory = 0;

		do
		{
			CqTextureMapBuffer* pTMB = CreateBuffer( 0, 0, m_xres, m_yres, directory, fProtectBuffers );

			if ( pTMB->pVoidBufferData() != NULL )
			{
				for ( TqInt y = 0; y < m_yres; y++ )
				{
					//unsigned char accum[ 4 ];
					std::vector<TqFloat> accum;
					for ( TqInt x = 0; x < m_xres; x++ )
					{
						ImageFilterVal( pBuffer, x, y, directory, accum );
						for ( TqInt sample = 0; sample < m_SamplesPerPixel; sample++ )
							pTMB->SetValue( x, y, sample, accum[ sample ] );
					}
				}
				m_apSegments.push_back( pTMB );
			}

			m_xres /= 2;
			m_yres /= 2;
			directory++;

		}
		while ( ( m_xres > 2 ) && ( m_yres > 2 ) ) ;
	}
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
	std::list<CqTextureMapBuffer*>::iterator s;
	for ( s = m_apSegments.begin(); s != m_apSegments.end(); s++ )
		delete( *s );

	m_apSegments.resize( 0 );


#ifdef ALLOCSEGMENTSTATUS
	{
		// We count each allocation/free at the end they should match
		//TqChar report[ 200 ];

		//sprintf( report, "alloc/free %d %d\n Memory Usage %d", alloc_cnt, free_cnt, QGetRenderContext() ->Stats().GetTextureMemory() );
		//RiErrorPrint( 0, 1, report );
		QGetRenderContextI() ->Logger() ->info( "alloc/free %d %d\n Memory usage %d", alloc_cnt, free_cnt, QGetRenderContext() ->Stats().GetTextureMemory() );
	}
#endif
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

//----------------------------------------------------------------------
/** Get a pointer to the cache buffer segment which contains the specifed sample point.
 * \param s Horizontal sample position.
 * \param t Vertical sample position.
 * \param directory TIFF directory index.
 * \param fProt A boolean value, true if the buffer should be protected from removal by the cache management system.
 */

CqTextureMapBuffer* CqTextureMap::GetBuffer( TqUlong s, TqUlong t, TqInt directory, TqBool fProt )
{
	QGetRenderContext() ->Stats().IncTextureMisses( 4 );

	if ( m_apSegments.front() ->IsValid( s, t, directory ) )
	{
		QGetRenderContext() ->Stats().IncTextureHits( 1, 4 );
		return( m_apSegments.front() );
	}

	// Search already cached segments first.
	for ( std::list<CqTextureMapBuffer*>::iterator i = m_apSegments.begin(); i != m_apSegments.end(); i++ )
	{
		if ( ( *i ) ->IsValid( s, t, directory ) )
		{
			QGetRenderContext() ->Stats().IncTextureHits( 1, 4 );
			// Move this segment to the top of the list, so that next time it is found first. This
			// allows us to take advantage of likely spatial coherence during shading.
			CqTextureMapBuffer* pbuffer = *i;
			m_apSegments.erase(i);
			m_apSegments.push_front(pbuffer);
			return ( pbuffer );
		}
	}

	// If we got here, segment is not currently loaded, so load the correct segement and store it in the cache.
	CqTextureMapBuffer* pTMB = 0;

	if ( !m_pImage )
	{
		CqRiFile	fileImage( m_strName.c_str(), "texture" );
		if ( !fileImage.IsValid() )
		{
			QGetRenderContextI() ->Logger() ->error( "Cannot open texture file \"%s\"", m_strName.c_str() );
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
			pTMB = CreateBuffer( ox, oy, tsx, tsy, directory, fProt );

			TIFFSetDirectory( m_pImage, directory );

			void* pData = pTMB->pVoidBufferData();
			TIFFReadTile( m_pImage, pData, s, t, 0, 0 );
			// Put this segment on the top of the list, so that next time it is found first. This
			// allows us to take advantage of likely spatial coherence during shading.
			m_apSegments.push_front( pTMB );
		}
		else
		{
			// Create a storage buffer
			pTMB = CreateBuffer( 0, 0, m_XRes, m_YRes, directory, TqTrue );

			TIFFSetDirectory( m_pImage, directory );
			void* pdata = pTMB->pVoidBufferData();
			TqUint i;
			for ( i = 0; i < m_YRes; i++ )
			{
				TIFFReadScanline( m_pImage, pdata, i );
				pdata = reinterpret_cast<void*>( reinterpret_cast<char*>( pdata ) + m_XRes * pTMB->ElemSize() );
			}
			// Put this segment on the top of the list, so that next time it is found first. This
			// allows us to take advantage of likely spatial coherence during shading.
			m_apSegments.push_front( pTMB );
		}
	}
	return ( pTMB );
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
	if ( iv != iv_n )
	{
		pTMBb = GetBuffer( iu, iv_n, id );	// Val01
	}

	CqTextureMapBuffer* pTMBc = pTMBa;
	if ( iu_n != iu )
	{
		pTMBc = GetBuffer( iu_n, iv, id );	// Val10
	}
	CqTextureMapBuffer* pTMBd = NULL;
	if ( iv == iv_n ) pTMBd = pTMBc;
	else if ( iu == iu_n ) pTMBd = pTMBb;
	else
		pTMBd = GetBuffer( iu_n, iv_n, id );	// Val11

	TqInt c;
	/* cannot find anything than goodbye */
	if ( !pTMBa || !pTMBb || !pTMBc || !pTMBd )
	{
		//TqChar warnings[ 400 ];
		for ( c = 0; c < m_SamplesPerPixel; c++ )
			val[ c ] = 1.0f;

		//sprintf( warnings, "Cannot find value for either pTMPB[a,b,c,d]" );
		//RiErrorPrint( 0, 1, warnings );
		///! \todo what do we want to say here?
		QGetRenderContextI() ->Logger() ->error( "Cannot find value for either pTMPB[a,b,c,d]" );
		return ;
	}

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
		TqFloat Val00 = pTMBa->GetValue( iu, iv, c );
		TqFloat Val01 = pTMBb->GetValue( iu, iv_n, c );
		TqFloat Val10 = pTMBc->GetValue( iu_n, iv, c );
		TqFloat Val11 = pTMBd->GetValue( iu_n, iv_n, c );
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
		if ( iv != iv_n )
		{
			pTMBb = GetBuffer( iu, iv_n, id + 1 );	// Val01
		}

		CqTextureMapBuffer* pTMBc = pTMBa;
		if ( iu_n != iu )
		{
			pTMBc = GetBuffer( iu_n, iv, id + 1 );	// Val10
		}
		CqTextureMapBuffer* pTMBd = NULL;
		if ( iv == iv_n ) pTMBd = pTMBc;
		else if ( iu == iu_n ) pTMBd = pTMBb;
		else
			pTMBd = GetBuffer( iu_n, iv_n, id + 1 );	// Val11

		/* cannot find anything than goodbye */
		if ( !pTMBa || !pTMBb || !pTMBc || !pTMBd )
		{
			//TqChar warnings[ 400 ];

			val = m_low_color;
			//sprintf( warnings, "Cannot find value for either pTMPB[a,b,c,d]" );
			//RiErrorPrint( 0, 1, warnings );
			///! \todo what do we want to say here?
			QGetRenderContextI() ->Logger() ->error( "Cannot find value for either pTMPB[a,b,c,d]" );
			return ;
		}

		// Bilinear intepolate the values at the corners of the sample.
		iu -= pTMBa->sOrigin();
		iu_n -= pTMBc->sOrigin();
		iv -= pTMBa->tOrigin();
		iv_n -= pTMBb->tOrigin();

		TqInt c;
		for ( c = 0; c < m_SamplesPerPixel; c++ )
		{
			TqFloat Val00 = pTMBa->GetValue( iu, iv, c );
			TqFloat Val01 = pTMBb->GetValue( iu, iv_n, c );
			TqFloat Val10 = pTMBc->GetValue( iu_n, iv, c );
			TqFloat Val11 = pTMBd->GetValue( iu_n, iv_n, c );
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
/** Check if a texture map exists in the cache, return a pointer to it if so, else
 * load it if possible..
 */

CqTextureMap* CqTextureMap::GetTextureMap( const CqString& strName )
{
	static int size = -1;
	static CqTextureMap *previous = NULL;

	QGetRenderContext() ->Stats().IncTextureMisses( 0 );

	/* look if the last item return by this function was ok */
	if ( size == static_cast<int>( m_TextureMap_Cache.size() ) )
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
		pNew->CreateMIPMAP( TqTrue );

#if 0
		TqFloat swidth = pNew->m_swidth;
		TqFloat twidth = pNew->m_twidth;

		enum EqWrapMode smode = pNew->m_smode;
		char* swrap, *twrap;
		if ( smode == WrapMode_Periodic )
			swrap = RI_PERIODIC;
		else if ( smode == WrapMode_Clamp )
			swrap = RI_CLAMP;
		else if ( smode == WrapMode_Black )
			swrap = RI_BLACK;

		enum EqWrapMode tmode = pNew->m_tmode;
		if ( tmode == WrapMode_Periodic )
			twrap = RI_PERIODIC;
		else if ( tmode == WrapMode_Clamp )
			twrap = RI_CLAMP;
		else if ( tmode == WrapMode_Black )
			twrap = RI_BLACK;

		char modes[ 1024 ];
		sprintf( modes, "%s %s %s %f %f", swrap, twrap, "box", swidth, twidth );
		if ( pNew->m_FilterFunc == RiGaussianFilter )
			sprintf( modes, "%s %s %s %f %f", swrap, twrap, "gaussian", swidth, twidth );
		if ( pNew->m_FilterFunc == RiBoxFilter )
			sprintf( modes, "%s %s %s %f %f", swrap, twrap, "box", swidth, twidth );
		if ( pNew->m_FilterFunc == RiTriangleFilter )
			sprintf( modes, "%s %s %s %f %f", swrap, twrap, "triangle", swidth, twidth );
		if ( pNew->m_FilterFunc == RiCatmullRomFilter )
			sprintf( modes, "%s %s %s %f %f", swrap, twrap, "catmull-rom", swidth, twidth );
		if ( pNew->m_FilterFunc == RiSincFilter )
			sprintf( modes, "%s %s %s %f %f", swrap, twrap, "sinc", swidth, twidth );
		if ( pNew->m_FilterFunc == RiDiskFilter )
			sprintf( modes, "%s %s %s %f %f", swrap, twrap, "disk", swidth, twidth );
		if ( pNew->m_FilterFunc == RiBesselFilter )
			sprintf( modes, "%s %s %s %f %f", swrap, twrap, "bessel", swidth, twidth );

		TIFF* ptex = TIFFOpen( "test.tex", "w" );

		TIFFCreateDirectory( ptex );
		TIFFSetField( ptex, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB );
		TIFFSetField( ptex, TIFFTAG_PIXAR_TEXTUREFORMAT, MIPMAP_HEADER );
		TIFFSetField( ptex, TIFFTAG_PIXAR_WRAPMODES, modes );
		TIFFSetField( ptex, TIFFTAG_SAMPLESPERPIXEL, pNew->SamplesPerPixel() );
		TIFFSetField( ptex, TIFFTAG_BITSPERSAMPLE, 8 );
		TIFFSetField( ptex, TIFFTAG_COMPRESSION, pNew->Compression() ); /* COMPRESSION_DEFLATE */
		int log2 = MIN( pNew->XRes(), pNew->YRes() );
		log2 = ( int ) ( log( log2 ) / log( 2.0 ) );


		for ( int i = 0; i < log2; i ++ )
		{
			// Write the floating point image to the directory.
			CqTextureMapBuffer* pBuffer = pNew->GetBuffer( 0, 0, i );
			if ( !pBuffer ) break;
			pNew->WriteTileImage( ptex, pBuffer, 64, 64, pNew->Compression(), pNew->Quality() );
		}
		TIFFClose( ptex );
#endif

		pNew->Close();
	}

	previous = pNew;
	size = m_TextureMap_Cache.size();
	return ( pNew );
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
void CqTextureMap::ImageFilterVal( CqTextureMapBuffer* pData, TqInt x, TqInt y, TqInt directory, std::vector<TqFloat>& accum )
{
	RtFilterFunc pFilter = m_FilterFunc;

	TqInt ydelta = ( 1 << directory );
	TqInt xdelta = ( 1 << directory );
	TqFloat div = 0.0;
	TqFloat mul;
	TqInt isample;

	// Clear the accumulator
	accum.assign( SamplesPerPixel(), 0.0f );

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
				TqInt ypos = ( ( y * ydelta ) + j );
				TqInt xpos = ( ( x * xdelta ) + i );

				/* ponderate the value */
				for ( isample = 0; isample < SamplesPerPixel(); isample++ )
					accum[ isample ] += ( pData->GetValue( xpos, ypos, isample ) ) * mul;

				/* accumulate the ponderation factor */
				div += mul;
			}
		}

		/* use the accumulated ponderation factor */
		for ( isample = 0; isample < SamplesPerPixel(); isample++ )
			accum[ isample ] /= static_cast<TqFloat>( div );
	}
	else
	{
		/* copy the byte don't bother much */
		for ( isample = 0; isample < SamplesPerPixel(); isample++ )
			accum[ isample ] = ( pData->GetValue( x, ( m_YRes - y - 1 ), isample ) );

	}
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
		//CqString strErr( "Cannot open texture file : " );
		//strErr += m_strName;
		//CqBasicError( 1, Severity_Fatal, strErr.c_str() );
		QGetRenderContextI() ->Logger() ->error( "Cannot open texture file \"%s\"", m_strName.c_str() );
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

		uint16 planarconfig;
		TIFFGetField( m_pImage, TIFFTAG_PLANARCONFIG, &planarconfig );
		m_PlanarConfig = planarconfig;
		uint16 samplesperpixel;
		TIFFGetField( m_pImage, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel );
		m_SamplesPerPixel = samplesperpixel;
		uint16 sampleformat;
		TIFFGetField( m_pImage, TIFFTAG_SAMPLEFORMAT, &sampleformat );
		m_SampleFormat = sampleformat;

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


void CqTextureMap::PrepareSampleOptions( std::map<std::string, IqShaderData*>& paramMap )
{
	m_sblur = 0.0f;
	m_tblur = 0.0f;
	m_pswidth = 1.0f;
	m_ptwidth = 1.0f;
	m_samples = 0.0f;

	// Get parameters out of the map.
	if ( paramMap.size() != 0 )
	{
		if ( paramMap.find( "width" ) != paramMap.end() )
		{
			paramMap[ "width" ] ->GetFloat( m_pswidth );
			m_ptwidth = m_pswidth;
		}
		else
		{
			if ( paramMap.find( "swidth" ) != paramMap.end() )
				paramMap[ "swidth" ] ->GetFloat( m_pswidth );
			if ( paramMap.find( "twidth" ) != paramMap.end() )
				paramMap[ "twidth" ] ->GetFloat( m_ptwidth );
		}
		if ( paramMap.find( "blur" ) != paramMap.end() )
		{
			paramMap[ "blur" ] ->GetFloat( m_sblur );
			m_tblur = m_sblur;
		}
		else
		{
			if ( paramMap.find( "sblur" ) != paramMap.end() )
				paramMap[ "sblur" ] ->GetFloat( m_sblur );
			if ( paramMap.find( "tblur" ) != paramMap.end() )
				paramMap[ "tblur" ] ->GetFloat( m_tblur );
		}
		if ( paramMap.find( "samples" ) != paramMap.end() )
			paramMap[ "samples" ] ->GetFloat( m_samples );
	}
}


void CqTextureMap::SampleMap( TqFloat s1, TqFloat t1, TqFloat swidth, TqFloat twidth, std::valarray<TqFloat>& val )
{
	// Check the memory and make sure we don't abuse it
	CriticalMeasure();

	if ( !IsValid() ) return ;

	swidth *= m_pswidth;
	twidth *= m_ptwidth;

	// T(s2,t2)-T(s2,t1)-T(s1,t2)+T(s1,t1)
	val.resize( m_SamplesPerPixel );
	val = 0.0f;

	TqFloat ss1 = s1 - swidth - ( m_sblur * 0.5f );
	TqFloat tt1 = t1 - twidth - ( m_tblur * 0.5f );
	TqFloat ss2 = s1 + swidth + ( m_sblur * 0.5f );
	TqFloat tt2 = t1 + twidth + ( m_tblur * 0.5f );

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
	//      TqInt i;
	//	for ( i = 0; i < m_SamplesPerPixel; i++ )
	//		val[ i ] = CLAMP( val[ i ], 0.0f, 1.0f );
}


//----------------------------------------------------------------------
/** Retrieve a sample from the MIP MAP over the area specified by the four vertices
 */

void CqTextureMap::SampleMap( TqFloat s1, TqFloat t1, TqFloat s2, TqFloat t2, TqFloat s3, TqFloat t3, TqFloat s4, TqFloat t4,
                              std::valarray<TqFloat>& val )
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

	SampleMap( ss1, tt1, swidth, twidth, val );
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
 * determine the size and type from the buffer.
 */

void CqTextureMap::WriteImage( TIFF* ptex, CqTextureMapBuffer* pBuffer, TqInt compression, TqInt quality )
{
	switch ( pBuffer->BufferType() )
	{
			case BufferType_RGBA:
			{
				WriteImage( ptex, static_cast<TqPuchar>( pBuffer->pVoidBufferData() ), pBuffer->Width(), pBuffer->Height(), pBuffer->Samples(), compression, quality );
				break;
			}

			case BufferType_Float:
			{
				WriteImage( ptex, static_cast<TqFloat*>( pBuffer->pVoidBufferData() ), pBuffer->Width(), pBuffer->Height(), pBuffer->Samples(), compression, quality );
				break;
			}
	}
}

//----------------------------------------------------------------------
/** Write an image to an open TIFF file in the current directory as tiled storage.
 * determine the size and type from the buffer.
 */

void CqTextureMap::WriteTileImage( TIFF* ptex, CqTextureMapBuffer* pBuffer, TqUlong twidth, TqUlong theight, TqInt compression, TqInt quality )
{
	switch ( pBuffer->BufferType() )
	{
			case BufferType_RGBA:
			{
				WriteTileImage( ptex, static_cast<TqPuchar>( pBuffer->pVoidBufferData() ), pBuffer->Width(), pBuffer->Height(), twidth, theight, pBuffer->Samples(), compression, quality );
				break;
			}

			case BufferType_Float:
			{
				WriteTileImage( ptex, static_cast<TqFloat*>( pBuffer->pVoidBufferData() ), pBuffer->Width(), pBuffer->Height(), twidth, theight, pBuffer->Samples(), compression, quality );
				break;
			}
	}
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



END_NAMESPACE( Aqsis )

