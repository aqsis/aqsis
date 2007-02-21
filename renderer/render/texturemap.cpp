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
		\brief Implements texture map handling and cacheing classes.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	"MultiTimer.h"

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
#include	"logging.h"

#ifndef		AQSIS_SYSTEM_WIN32
#include	"unistd.h"
#else
#include	"io.h"
#endif		// AQSIS_SYSTEM_WIN32

#if defined(AQSIS_SYSTEM_MACOSX)
#include "Carbon/Carbon.h"
#endif

START_NAMESPACE( Aqsis )

// Local Constants

#define MEG1                8192*1024
#define RDMAX 128

#undef  ALLOCSEGMENTSTATUS

#define ONETHIRD (1.0f/3.0f)
#define TWOTHIRD (2.0f/3.0f)
#define ONEHALF  (1.0f/2.0f)


//
// #define WITHFILTER 1 if you want filtering even when you are doing with area sampling;
//     this is slow since we RiSincFilter (without any form of catch) on top of the area sampling.
//
#define WITHFILTER       1

//
// #define FASTLOG2 if you want to experiment with integer/float multiplication
// replacement for log2() see fastlog2()
//

// Local Constants
typedef enum {
    PX,
    NX,
    PY,
    NY,
    PZ,
    NZ,
} ESide;

typedef enum {
    XYZ,
    XZY,
    YXZ,
    YZX,
    ZXY,
    ZYX
} EOrder;

//
// Local Variables
//
static TqBool m_critical = TqFalse;

static TqFloat sides[6][2]    =  {
                                     {0.0f,0.0f}, {0.0f, ONEHALF}, {ONETHIRD, 0.0f}, {ONETHIRD,ONEHALF},
                                     {TWOTHIRD, 0.0f}, {TWOTHIRD, ONEHALF}
                                 };

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


//----------------------------------------------------------------------
/** CalculateNoise() Return pseudorandom value for sampling texture.
*
*/

static void CalculateNoise(TqFloat &du, TqFloat &dv, TqInt which)
{
	static TqInt   i_RdIx = -1;
	static TqFloat RD[RDMAX][2];

	// Crucial to have some speed here;
	// I initialize 128 x,y random value which
	// will be used by GetSampleWithoutBlur()
	if (i_RdIx == -1)
	{
		TqInt i;
		CqRandom rd;

		for (i =0; i < RDMAX; i ++)
		{
			RD[i][0] = rd.RandomFloat();
			RD[i][1] = rd.RandomFloat();
			//Aqsis::log() << warning << RD[i][0] << " " << RD[i][1] << std::endl;
		}

		i_RdIx = 0;

	}

	if (which != 0)
	{
		du = RD[i_RdIx][0];
		dv = RD[i_RdIx][1];
		i_RdIx ++;
		i_RdIx %= RDMAX;
	}
	else
	{
		dv = du = 0.5;
	}
}
//----------------------------------------------------------------------
/** IsVerbose() Is it adequate to printout the level of mipmap ?
 * Option "statistics" "int renderinfo" 1 by default it returns TqFalse
 *
 */
static TqBool IsVerbose()
{
	static TqInt bVerbose = -1;


	if (bVerbose == -1)
	{
		const TqInt* poptVerbose = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "statistics", "renderinfo" );

		bVerbose = 0;
		if (poptVerbose && *poptVerbose)
			bVerbose = 1;
	}

	return (bVerbose == 1);
}

//----------------------------------------------------------------------
/** CalculateFilter() Figure out which filter is applied to this mipmap.
*
*/
static RtFilterFunc CalculateFilter(CqString filter)
{
	RtFilterFunc filterfunc = RiBoxFilter;

	if ( filter == "gaussian" )
		filterfunc = RiGaussianFilter;
	if ( filter == "mitchell" )
		filterfunc = RiMitchellFilter;
	if ( filter == "box" )
		filterfunc = RiBoxFilter;
	if ( filter == "triangle" )
		filterfunc = RiTriangleFilter;
	if ( filter == "catmull-rom" )
		filterfunc = RiCatmullRomFilter;
	if ( filter == "sinc" )
		filterfunc = RiSincFilter;
	if ( filter == "disk" )
		filterfunc = RiDiskFilter;
	if ( filter == "bessel" )
		filterfunc = RiBesselFilter;

	return filterfunc;
}

//---------------------------------------------------------------------
/** fast log2() implementation not very precise but for NT good enough
*/
static TqFloat fastlog2(TqFloat a)
{
#ifdef FASTLOG2
	register TqFloat x,y;
	x = *(int*)&a;
	x*= 1.19209e-007; /* pow(2.0, -23) */
	x = x - 127.0f;


	y = x - floor(x);
	y = (y - y *y) * 0.346607f;
	return x+y;
#else
static TqFloat invLog2 =  1.0f/::log(2.0f);

    return ::log(a) * invLog2;
#endif
}

//---------------------------------------------------------------------
/** Allocate a cache segment to hold the specified image tile.
 */
TqPuchar CqTextureMapBuffer::AllocSegment( TqUlong width, TqUlong height, TqInt samples, TqBool fProt )
{
	static TqInt limit = -1;
	static TqInt report = 1;
	TqInt demand = width * height * ElemSize();

#ifdef ALLOCSEGMENTSTATUS
	alloc_cnt ++;
#endif

	if ( limit == -1 )
	{
		const TqInt * poptMem = QGetRenderContextI() ->GetIntegerOption( "limits", "texturememory" );
		limit = MEG1;
		if ( poptMem )
		{
			if ( poptMem[0] < INT_MAX/1024)
				limit = poptMem[ 0 ] * 1024;
			else  limit = INT_MAX;
		}
		Aqsis::log() << info << "Set the cache limit to be " << limit << std::endl;
	}

	TqInt more = QGetRenderContext() ->Stats().GetTextureMemory() + demand;


	if ( ( more > limit ) && !fProt )
	{

		// Critical level of memory will be reached;
		// I'm better starting the cleanup cache memory buffer
		// We need to zap the cache we are exceeding the required texturememory demand
		// For now, we will just warn the user the texturememory's demand will exceed the
		// request number.

		if ( report )
		{
			Aqsis::log() << warning << "Exceeding allocated texture memory by " << more - limit << std::endl;
		}

		report = 0;
		m_critical = TqTrue;
	}

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
	// Suspicious if this does not have an extension.
	if (strName.rfind(".") == std::string::npos)
		return 0;

	const CqString extension = strName.substr(strName.rfind(".")).substr(1);

#if defined(AQSIS_SYSTEM_POSIX)

	CqString plugin_path = DEFAULT_PLUGIN_PATH "/lib" + extension + "2tif.so";
	// Check for lib<ext>2tif.so; it it is existing than let the converter to
	// be called.
	// I assume on MacOSX and Linux the file we are looking will be something:
	//     libjpg2tif.so, libtga2tif.so, lib...2tif.so or
	//     libjpg2tif.dylib, libtga2tif.dylib, ...2tif.dylib
	if (access(plugin_path.c_str(), F_OK) != 0)
	{
		plugin_path = DEFAULT_PLUGIN_PATH "/lib" + extension + "2tif.dylib";
		if (access(plugin_path.c_str(), F_OK) != 0)
			return 0;
	}

#elif defined(AQSIS_SYSTEM_WIN32)
	char acPath[255];

	if ( GetModuleFileName( NULL, acPath, 256 ) != 0)
	{
		// guaranteed file name of at least one character after path
		*( strrchr( acPath, '\\' ) + 1 ) = '\0';
	}
	else
		return 0;

	CqString plugin_path = acPath;
	plugin_path.append( CqString("/" + extension + "2tif.dll") );
#endif

	const CqString plugin_function = extension + "2tif";

	TqInt result = 0;
	CqConverter* const plug = new CqConverter( "plugin", const_cast<char*>(plugin_path.c_str()), const_cast<char*>(plugin_function.c_str()) );
	char * ( *convert ) ( const char * );
	if ( ( convert = ( char * ( * ) (const  char * s ) ) plug->Function() ) != NULL )
	{
		char* tiff = 0;
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
  * It zaps the m_apFlat for this TextureMap object completely.
  * The idea here is to erase any "GetBuffer()" memory to respect the directive
  * It looks into the big m_TextureMap_Cache release every things
  **/
void CqTextureMap::CriticalMeasure()
{
	TqInt current, now; 
	static TqInt limit = -1;
	std::vector<CqTextureMap*>::iterator j;
	std::list<CqTextureMapBuffer*>::iterator i;
	std::list<CqTextureMapBuffer*>::iterator e;

	if (limit == -1)
	{
		limit = MEG1;
		const TqInt * poptMem = QGetRenderContextI() ->GetIntegerOption( "limits", "texturememory" );
		if ( poptMem )
			limit = poptMem[ 0 ] * 1024;
	}

	IsVerbose();


	now = QGetRenderContext() ->Stats().GetTextureMemory();
	TqBool getout = TqTrue;

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
		for ( j = m_TextureMap_Cache.begin(); j != m_TextureMap_Cache.end(); j++ )
		{
			Aqsis::log() << info << "Texture cache: freeing memory used by \"" << (*j)->getName().c_str() << "\"" << std::endl;
			// the original segments (flat tiles)  are the first to go 
			i = (*j)->m_apFlat.begin(); 
			e = (*j)->m_apFlat.end(); 
			for ( ; i != e; i++ )
			{
				if (*i) delete(*i);
			}	
			(*j)->m_apFlat.resize(0);
			(*j)->m_apLast[0] = NULL;

			// All MipMaps segments (flat tiles)  are the first to go 
			for ( TqInt k = 0; (k <  256) && (getout == TqFalse); k++)
			{
				i = (*j)->m_apMipMaps[k].begin(); 
				e = (*j)->m_apMipMaps[k].end(); 
				for ( ; i != e; i++ )
				{
					if (*i) delete(*i);
				}	
				(*j)->m_apLast[k] = NULL;
				(*j)->m_apMipMaps[k].resize(0);
				current = QGetRenderContext() ->Stats().GetTextureMemory();
				if ( ( now - current ) > ( limit / 4 ) ) {
					getout = TqTrue;
					break;
				}
			}
			current = QGetRenderContext() ->Stats().GetTextureMemory();
			if ( ( now - current ) > ( limit / 4 ) ) {
				getout = TqTrue;
				break;
			}

		}
	}
	current = QGetRenderContext() ->Stats().GetTextureMemory();

	m_critical = TqFalse;

#ifdef _DEBUG

	if ( now - current )
	{
		///! \todo Review this debug message
		Aqsis::log() << info << "I was forced to zap the tile segment buffers for " << (int)( now - current ) / 1024 << "K" << std::endl;
	}
#endif

}


//---------------------------------------------------------------------
/** If properly opened, close the TIFF file.
 */
void CqTextureMap::Close()
{

	if ( m_pImage != 0 )
		TIFFClose( m_pImage );
	m_pImage = 0;

}



//---------------------------------------------------------------------
/** Create a mipmap usable for the texturemapping.
 */
TqBool CqTextureMap::CreateMIPMAP( TqBool fProtectBuffers )
{

	if ( m_pImage != 0 )
	{
		// Check if the format is normal scanline, otherwise we are unable to MIPMAP it yet.
		uint32 tsx;
		TqInt ret = TIFFGetField( m_pImage, TIFFTAG_TILEWIDTH, &tsx );
		if( ret )
		{
			Aqsis::log() << error << "Cannot MIPMAP a tiled image \"" << m_strName.c_str() << "\"" << std::endl;
			return( TqFalse );
		}
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
						ImageFilterVal( pBuffer, x, y, directory, m_xres, m_yres, accum );

						for ( TqInt sample = 0; sample < m_SamplesPerPixel; sample++ )
							pTMB->SetValue( x, y, sample, accum[ sample ] );
					}
				}
				m_apMipMaps[directory%256].push_back( pTMB );
				m_apLast[directory%256] = pTMB;
			}

			m_xres /= 2;
			m_yres /= 2;
			directory++;

		}
		while ( ( m_xres > 2 ) && ( m_yres > 2 ) ) ;
	}
	return( TqTrue );
}



//---------------------------------------------------------------------
/** Destructor.
 */
CqTextureMap::~CqTextureMap()
{
	// Close the file.
	Close();
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
	std::list<CqTextureMapBuffer*>::iterator l;
	std::list<CqTextureMapBuffer*>::iterator e;

	// First into the flat segments partitions
	l = m_apFlat.begin();
	e = m_apFlat.end();
	for ( ; l != e; l++ )
	{
		if (*l) 
		{
			delete (*l);
		}
	}
	m_apFlat.resize( 0 );
	m_apLast[0] = NULL;

	// Second into the Mipmaps segments partitions
	for (TqInt k = 0; k < 256; k++)
	{
		l = m_apMipMaps[k].begin(); 
		e = m_apMipMaps[k].end(); 
		for ( ; l != e; l++ )
		{
			if (*l) 
			{
				delete (*l);
			}
		}	
		m_apLast[k] = NULL;
		m_apMipMaps[k].resize(0);
	}



#ifdef ALLOCSEGMENTSTATUS

	{
		// We count each allocation/free at the end they should match
		Aqsis::log() << "alloc/free " << alloc_cnt << " " << free_cnt << " - Memory usage " << QGetRenderContext() ->Stats().GetTextureMemory() << std::endl;
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
        //TIMER_START("GetBuffer");
	QGetRenderContext() ->Stats().IncTextureMisses( 4 );

	CqTextureMapBuffer *lastcache = m_apLast[directory%256];

	if (lastcache && lastcache->IsValid(s, t, directory))
	{
		//TIMER_STOP("GetBuffer");
		QGetRenderContext() ->Stats().IncTextureHits( 0, 4 );
		return lastcache;
	}

	// Search already cached segments first.
	std::list<CqTextureMapBuffer*>::iterator i = m_apMipMaps[directory%256].begin(); 
	std::list<CqTextureMapBuffer*>::iterator e = m_apMipMaps[directory%256].end(); 
	for ( ; i != e; i++ )
	{
		if ( ( *i ) ->IsValid( s, t, directory ) )
		{
			QGetRenderContext() ->Stats().IncTextureHits( 1, 4 );
			CqTextureMapBuffer* pbuffer = *i;
			m_apLast[directory%256] = pbuffer;
			//TIMER_STOP("GetBuffer");
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
			Aqsis::log() << error << "Cannot open texture file \"" << m_strName.c_str() << "\"" << std::endl;
			//TIMER_STOP("GetBuffer");
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
		}
		// Put this segment on the top of the list, so that next time it is found first. This
		// allows us to take advantage of likely spatial coherence during shading.
		m_apMipMaps[directory%256].push_front( pTMB );
		m_apLast[directory%256] = pTMB;
	}
	//TIMER_STOP("GetBuffer");
	return ( pTMB );
}

//----------------------------------------------------------------------
/** CalculateLevel() determined with mipmap level is required.
* based u,v,u1,v1,u2,v2 (area of the sampling)
* the original size m_XRes, m_YRes, umapsize, vmapsize, and level.
* m_Directory will contain the lastest value of level too.
* \param u mean between u1, u2
* \param v mean between v1, v2
* \param u1, u2, v1, v2 the corners
* \param level will receive which mipmap level
* \param interp will receive the contribution of level versus level+1
* \param umapsize, vmapsize the width, height at level.
*/
void CqTextureMap::CalculateLevel(TqFloat ds, TqFloat dt)
{
	// Calculate the appropriate mipmap level from the area subtended by the sample.

	// We already computed the appropriate level don't do anything
	// since m_ds, m_dt are identical.
	if ((ds == m_ds) && (dt == m_dt))
		return;

	TqFloat UVArea;

	m_umapsize = m_XRes;
	m_vmapsize = m_YRes;
	m_interp = 0.0;
	m_level = 0;

	if ((Format() != TexFormat_MIPMAP ) && (Format() != TexFormat_Plain ))
		return;

	// Based on the area of u2-u1,v2-v2 in s,t space
	// but the area could be 0 sometimes.
	UVArea = (ds * m_XRes * dt * m_YRes);

	if (UVArea < 0.0)
		UVArea *= -1.0f;

	TqFloat l = (TqFloat) fastlog2(UVArea) / 2.0f;
	l = MAX(l, 0.0);

	TqInt id = (TqInt) FLOOR(l);

	m_interp =  l - id;
	m_interp = MIN(m_interp, 1.0);


	if (m_Directory && m_Directory < id)
		id = m_Directory;

	for (m_level =0; m_level < id; m_level ++)
	{
		m_umapsize >>= 1;
		m_vmapsize >>= 1;
		if (m_umapsize < 8 )
			break;
		if (m_vmapsize < 8 )
			break;
	}

	if (m_level)
	{
		m_Directory = m_level;
	}
	m_ds = ds;
	m_dt = dt;

}

//----------------------------------------------------------------------
/** Bilinear sample of any directory/u/v Color the result is saved
 * directly into m_color.
 * This is the implementation for the release >= 1.0
 * \param u is average of sample positions.
 * \param v is average of sample positions.
 * \param umapsize the mapsize at the directory id
 * \param vmapsize the mapsize at the direction id
 * \param id     the directory in the tiff 0...n
 * \param param m_color the result will be stored in m_color.
 */
TqBool CqTextureMap::BiLinear(TqFloat u, TqFloat v, TqInt umapsize, TqInt vmapsize,
                              TqInt id, std::valarray<TqFloat >	&m_color)
{
	TqUint umapsize1 = umapsize-1;
	TqUint vmapsize1 = vmapsize-1;

	TqUint iu = FLOOR( u * umapsize1);
	TqDouble ru = (u * umapsize1) - iu;
	TqUint iu_n = FLOOR( (u * umapsize1) + 1.0);

	TqUint iv = FLOOR( v * vmapsize1 );
	TqDouble rv = (v * vmapsize1) - iv;
	TqUint iv_n = FLOOR( (v * vmapsize1) + 1.0);

	iu = CLAMP(iu, 0,umapsize1);
	iu_n = CLAMP(iu_n, 0,umapsize1);
	iv = CLAMP(iv, 0,vmapsize1);
	iv_n = CLAMP(iv_n, 0,vmapsize1);

	// Read in the relevant texture tiles.
	register TqInt c;

	// Read in the relevant texture tiles.
	CqTextureMapBuffer* pTMBa = GetBuffer( iu, iv, id );	// Val00
	CqTextureMapBuffer* pTMBb = GetBuffer( iu_n, iv, id );	// Val01
	CqTextureMapBuffer* pTMBc = GetBuffer( iu, iv_n, id );  // Val10
	CqTextureMapBuffer* pTMBd = GetBuffer( iu_n, iv_n, id );// Val11


	/* cannot find anything than goodbye */
	if ( !pTMBa || !pTMBb || !pTMBc || !pTMBd )
	{
		for ( c = 0; c < m_SamplesPerPixel; c++ )
		{
			m_color[ c ] = 1.0f;
		}

		Aqsis::log() << error << "Cannot find value for either pTMPB[a,b,c,d]" << std::endl;
		Open();
		return TqFalse;
	}

	TqUint x1, y1, x2, y2, x3, y3, x4, y4;

	TqFloat Val00, Val01, Val10, Val11;

	x1 = iu - pTMBa->sOrigin();
	y1 = iv - pTMBa->tOrigin();
	x2 = iu_n - pTMBb->sOrigin();
	y2 = iv - pTMBb->tOrigin();
	x3 = iu - pTMBc->sOrigin();
	y3 = iv_n - pTMBc->tOrigin();
	x4 = iu_n - pTMBd->sOrigin();
	y4 = iv_n - pTMBd->tOrigin();
	//Aqsis::log() << warning << "ru, rv" << ru << " " << rv << std::endl;

	// Bilinear interpolate the values at the corners of the sample.
	for ( c = 0; c < m_SamplesPerPixel; c++ )
	{
		Val00 = pTMBa->GetValue( x1, y1, c );
		Val01 = pTMBb->GetValue( x2, y2, c );
		Val10 = pTMBc->GetValue( x3, y3, c );
		Val11 = pTMBd->GetValue( x4, y4, c );
		m_color[c] = LERP(rv, LERP(ru, Val00, Val01), LERP(ru, Val10, Val11));
	}

	return TqTrue;
}

//----------------------------------------------------------------------
/* CqTextureMap::GetSampleWithoutBlur( ) is the lastest incarnation it
 * supports, filter func on the fly, multiple samples, without blur.
 * \param u1,v2 is top/right bottom/left of sample positions.
 * \param v1,v2.
 * \param val the result will be stored.
 */
void CqTextureMap::GetSampleWithoutBlur( TqFloat u1, TqFloat v1, TqFloat u2, TqFloat v2, std::valarray<TqFloat>& val )
{
	register TqInt c;

	TIME_SCOPE("Texture mapping")

	// u/v is provided to the function now; they are the average of
	// sample positions (u1,v1), (u2, v2).
	TqFloat u = (u2+u1)/2.0;
	TqFloat v = (v2+v1)/2.0;

	// Calculate the appropriate mipmap level from the area subtended by the sample.
	CalculateLevel((u2-u1), (v2-v1));

	TqFloat contrib = 0.0f;
	TqFloat du, dv, mul;

	m_accum_color = 0.0f;

	// Textures are filtered with either bilinear or trilinear mip-mapping.
	// Trilinear gives higher image quality but takes more time.  To disable trilinear
	// mip-mapping for the entire scene:
	//        Option "texture" "float lerp" 0.0 or
	// provide to texture() call in the shader.
	if (m_lerp == -1.0)
	{
		const TqInt* pLerp = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "texture", "lerp" );

		m_lerp = 0.0f;
		if (pLerp && (*pLerp > 0.0f))
			m_lerp = 1.0f;

	}
	TqBool bLerp = (m_lerp == 1.0);


	// Assuming this will also include the pixel at u,v multiple samplings interval
	for (TqInt i = 0; i <= m_samples; i ++)
	{
		// return random values into du, dv between 0..1.0
		// but when i == 0; du = dv = 0.5 so at the minimum
		// a pixel will be read at (u,v)
		CalculateNoise(du, dv, i);

		mul = (*m_FilterFunc)(du-0.5,dv-0.5 , 1.0, 1.0);

		if (mul < m_pixelvariance)
			continue;

		u = LERP(dv, u1, LERP(du, u1, u2));
		v = LERP(dv, v1, LERP(du, v1, v2));

		BiLinear(u, v, m_umapsize, m_vmapsize, m_level, m_pixel_variance);
		if (bLerp)
			BiLinear(u, v, m_umapsize/2, m_vmapsize/2, m_level+1, m_pixel_sublevel);

		contrib += mul;

		if (bLerp)
		{
			for (c = 0; c < m_SamplesPerPixel; c++)
			{
				m_accum_color[c] += mul * LERP(m_interp, m_pixel_variance[c], m_pixel_sublevel[c]);

			}
		}
		else
		{
			for (c = 0; c < m_SamplesPerPixel; c++)
			{
				m_accum_color[c] += mul * m_pixel_variance[c];
			}
		}
	}
	for (c = 0; c < m_SamplesPerPixel; c++)
		val[c] = m_accum_color[c] / contrib;
}

//----------------------------------------------------------------------
/* CqTextureMap::GetSampleWithBlur( ) is
 * This is the implementation based on classical pyramid integration for mipmap ;
 * I found it the equivalent in multiple litterature about mipmaps; specially it is
 * implemanted very similar to pixie but without any Random to 
 * vary the u1...u2, v1..v2 interval a bit. I decide to not put the random() in place
 * since the user is more than willing to add with the "blur", "sblur" "tblur" options in first place.
 *
 * Even when ShadingRate is high this will behave; or if the user uses "blur". But if the area of 
 * micropolygon once corrected to screen coordinate is still small it might bleed texture.
 * If this module is compiled with WITHFILTER '1' it will filter using RiSincFilter the result 
 * without WITHFILTER it is equivalent to RiBoxFilter 1 1 
 * \param u1,v2 is top/right bottom/left of sample positions.
 * \param v1,v2.
 * \param val the result will be stored.
 */
void CqTextureMap::GetSampleWithBlur( TqFloat u1, TqFloat v1, TqFloat u2, TqFloat v2, std::valarray<TqFloat>& val )
{
	register TqInt c;

	TIME_SCOPE("Texture mapping")

	// u/v is provided to the function now; they are the average of
	// sample positions (u1,v1), (u2, v2).
	TqFloat u = (u1 + u2) * 0.5;
	TqFloat v = (v1 + v2) * 0.5;

	// Calculate the appropriate mipmap level from the area subtended by the sample.
	CalculateLevel(u2 - u1, v2 - v1);

	// Will it may make some sense to add some random value between 0..1.0 ?
	// For now I just put a delta based on the texel deplacement.
	TqFloat mul, div;

	// Area integration; it will be the ideal placed to put to the contribution
	// of each texel the filter factor.
	TqFloat cu, cv;

	m_accum_color = 0;
	div = 0.0;
	TqFloat deltau;
	TqFloat deltav;

	deltau = 1.0/(m_pswidth * m_umapsize);
	deltav = 1.0/(m_ptwidth * m_vmapsize);

	for (cu = u1; cu <= u2; cu += deltau)
	{
		for (cv = v1; cv <= v2; cv += deltav)
		{
			/* Yes we use Disk filter */
#ifdef WITHFILTER

			mul = (*m_FilterFunc)((cu-u), (cv-v), 2.0 * u, 2.0 * v);
#else

			mul = 1.0;
#endif

			if (mul < m_pixelvariance)
				continue;
			BiLinear(cu, cv, m_umapsize, m_vmapsize, m_level, m_pixel_variance);
			div += mul;
			for ( c = 0; c < m_SamplesPerPixel; c++ )
				m_accum_color[c] += mul * m_pixel_variance[c];
		}
	}

	for (c = 0; c < m_SamplesPerPixel; c++ )
		val[c] = m_accum_color[c] / div;
}

//----------------------------------------------------------------------
/* CqTextureMap::GetSample( ) is calling either GetSampleArea() or
 * GetSampleSgle() depending of the level of blur 
 */
void CqTextureMap::GetSample( TqFloat u1, TqFloat v1, TqFloat u2, TqFloat v2, std::valarray<TqFloat>& val)
{
	// Work out the width and height
	TqFloat uu1, uu2, vv1, vv2;

	uu1 = MIN( u1, u2);
	vv1 = MIN( v1, v2);
	uu2 = MAX( u1, u2);
	vv2 = MAX( v1, v2);

	if ( m_sblur || m_tblur)
	{
		GetSampleWithBlur( uu1, vv1, uu2, vv2, val );
	}
	else
	{
		// without blur it is not necessary to do GetSampleArea().
		GetSampleWithoutBlur( uu1, vv1, uu2, vv2, val);
	}
}

//----------------------------------------------------------------------
/** Check if a texture map exists in the cache, return a pointer to it if so, else
 * load it if possible..
 */
CqTextureMap* CqTextureMap::GetTextureMap( const CqString& strName )
{
	QGetRenderContext() ->Stats().IncTextureMisses( 0 );

	// First search the texture map cache
	for ( std::vector<CqTextureMap*>::iterator i = m_TextureMap_Cache.begin(); i != m_TextureMap_Cache.end(); i++ )
	{
		if ( ( *i ) ->m_strName == strName )
		{
			if ( ( *i ) ->Type() == MapType_Texture )
			{
				QGetRenderContext() ->Stats().IncTextureHits( 1, 0 );
				return ( *i );
			} else
			{
				return NULL;
			}
		}
	}

	QGetRenderContext() ->Stats().IncTextureHits( 0, 0 );

	// If we got here, it doesn't exist yet, so we must create and load it.
	CqTextureMap* pNew = new CqTextureMap( strName );
	pNew->Open();

	// Ensure that it is in the correct format
	if ( pNew->Format() != TexFormat_MIPMAP )
	{
		if( !pNew->CreateMIPMAP( TqTrue ) )
			pNew->SetInvalid();
		pNew->Close();
	}

	m_TextureMap_Cache.push_back( pNew );
	return ( pNew );
}



//----------------------------------------------------------------------
/** this is used for re-intrepreted the filter/wrap mode when using
 *  RiMakeTextureV() for downsampling/filter the tif file
 *
 **/
void CqTextureMap::Interpreted( TqPchar mode )
{
	const char* filter = "", *smode = "", *tmode = "";
	const char* sep = ", \t";

	// Take a copy of the string before processing it.
	char* string = new char[strlen(mode)+1];
	strcpy( string, mode );

	const char* token;
	token = strtok( string, sep );
	if( NULL != token )
	{
		smode = token;
		token = strtok( NULL, sep );
		if( NULL != token )
		{
			tmode = token;
			token = strtok( NULL, sep );
			if( NULL != token )
			{
				filter = token;
				token = strtok( NULL, sep );
				if( NULL != token )
				{
					m_swidth = atof( token );
					token = strtok( NULL, sep );
					if( NULL != token )
					{
						m_twidth = atof( token );
						token = strtok( NULL, sep );
					}
				}
			}
		}
	}

	//sscanf( mode, "%s %s %s %f %f", smode, tmode, filter, &m_swidth, &m_twidth );

	CqString sFilter = filter;
	m_FilterFunc = CalculateFilter(sFilter);

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

	delete[](string);
}

//----------------------------------------------------------------------
/** this is used for downsampling the texture at lower resolution
  *   
  * it will use the filtervalues. breakdown rgba values in floats. 
  * Accumulate the floating value rgba and ponderate the sum with the filter values;
  * and convert back to uint32 the rgba floating values.
  * The values of the current filterfunc/swrap/twrap are used ; if ever swrap or twrap is equal to
  * 1.0 than the filterfunc will be executed (at lower resolutions) using the points 
  *   (X1,Y1) and (X2,Y2) (-0.5, -0.5) ... (0.5, 0.5) giving 9 samples -0.5, 0.0, 0.5 in X and in Y.
  *
  **/
void CqTextureMap::ImageFilterVal( CqTextureMapBuffer* pData, TqInt x, TqInt y, TqInt directory,  TqInt m_xres, TqInt m_yres, std::vector<TqFloat>& accum )
{
	RtFilterFunc pFilter = m_FilterFunc;

	TqInt delta = ( 1 << directory );
	TqFloat div = 0.0;
	TqFloat mul;
	TqFloat fx, fy;
	register TqInt isample;
	TqInt xdelta = MAX(FLOOR(m_swidth) * (delta/2), 1);
	TqInt ydelta = MAX(FLOOR(m_twidth) * (delta/2), 1);
	TqInt xdelta2 = xdelta * 2;
	TqInt ydelta2 = ydelta * 2;


	// Increase the precision after the middle (0.5 and up make sure we will hit the borner at 1.0)
	fx = (TqFloat) (x)/ (TqFloat) (m_xres - 1);


	// Increase the precision after the middle (0.5 and up make sure we will hit the borner at 1.0)
	fy = (TqFloat) (y)/ (TqFloat) (m_yres - 1 );


	// Clear the accumulator
	accum.assign( SamplesPerPixel(), 0.0f );

	if ( directory )
	{
		TqInt i, j;

		for ( isample = 0; isample < SamplesPerPixel(); isample++ )
			accum[ isample ]= 0.0;

		/* From -twidth to +twidth */
		for ( j = - ydelta; j <= ydelta; j++ )
		{
			/* From -swidth to +swidth */
			for ( i = -xdelta; i <= xdelta; i++)
			{
				/* find the filter value */
				mul = ( *pFilter ) ( (TqFloat) i, (TqFloat) j, (TqFloat) xdelta2, (TqFloat) ydelta2 );
				if (mul == 0.0)
					continue;

				/* find the value in the original image */
				TqInt ypos = (TqInt) (fy*m_YRes-1) + j;
				TqInt xpos = (TqInt) (fx*m_XRes-1) + i;
				if (ypos < 0)
					continue;
				if (xpos < 0)
					continue;
				if (ypos > (TqInt) m_YRes - 1)
					continue;
				if (xpos > (TqInt) m_XRes - 1)
					continue;
				/* ponderate the value */
				for ( isample = 0; isample < SamplesPerPixel(); isample++ )
					accum[ isample ] += (pData->GetValue( xpos, ypos, isample ) * mul);

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
			accum[ isample ] = ( pData->GetValue( x, y, isample ) );

	}

}

//---------------------------------------------------------------------
/** Open a named texture map.
 */
void CqTextureMap::Open()
{
	TqInt wasconverted = 0;

	m_IsValid = TqFalse;


	// Find the file required.
	CqRiFile	fileImage( m_strName.c_str(), "texture" );
	if ( !fileImage.IsValid() )
	{
		Aqsis::log() << error << "Cannot open texture file \"" << m_strName.c_str() << "\"" << std::endl;
		return ;
	}
	CqString strRealName( fileImage.strRealName() );
	fileImage.Close();

	// Now try to converted first to tif file
	// if the plugin is not existant than goes straight to TIFFOpen()
	wasconverted = Convert( strRealName );
	if ( wasconverted )
	{
		CqString * strnew = new CqString( strRealName );
		m_ConvertString_Cache.push_back( strnew );
		// Now open it as a tiff file.
		m_pImage = TIFFOpen( strRealName.c_str(), "r" );
	}
	else
	{
		m_pImage = TIFFOpen( strRealName.c_str(), "r" );
	}


	if ( m_pImage )
	{
		Aqsis::log() << info << "TextureMap: \"" << strRealName.c_str() << "\" is open" << std::endl;
		TqPchar pFormat = 0;
		TqPchar pModes = 0;

		TIFFGetField( m_pImage, TIFFTAG_IMAGEWIDTH, &m_XRes );
		TIFFGetField( m_pImage, TIFFTAG_IMAGELENGTH, &m_YRes );

		uint16 planarconfig;
		TIFFGetField( m_pImage, TIFFTAG_PLANARCONFIG, &planarconfig );
		m_PlanarConfig = planarconfig;
		uint16 samplesperpixel = 1;
		TIFFGetField( m_pImage, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel );
		m_SamplesPerPixel = samplesperpixel;
		uint16 sampleformat;
		TIFFGetFieldDefaulted( m_pImage, TIFFTAG_SAMPLEFORMAT, &sampleformat );
		m_SampleFormat = sampleformat;

		uint16 bitspersample;
		TIFFGetFieldDefaulted( m_pImage, TIFFTAG_BITSPERSAMPLE, &bitspersample );
		m_BitsPerSample = bitspersample;

		TIFFGetField( m_pImage, TIFFTAG_PIXAR_TEXTUREFORMAT, &pFormat );
		TIFFGetField( m_pImage, TIFFTAG_PIXAR_WRAPMODES, &pModes );

		// Resize the temporary storage values to the new depth.
		m_pixel_variance.resize( m_SamplesPerPixel );
		m_pixel_sublevel.resize( m_SamplesPerPixel );
		m_accum_color.resize( m_SamplesPerPixel );

		/* Aqsis supports a slighty different scheme for MipMap tiff file;
		 * its filtering is kept as a string in 
		 * Texture Wrap Modes: "periodic periodic box 1.000000 1.000000"
		 * where AIR, 3Delight, BMRT, RDC use very basic texture wrap mode description eg.
		 * Texture Wrap Modes: "black,black"
		 * therefore I initialized the value for filtering to be black, black, box, 1.0, 1.0
		 * 
		 */
		if ( pModes )
		{
			Interpreted( pModes );
		}
		uint32 tsx;

		/* First tests; is it stored using tiles ? */
		TqInt bMipMap = TIFFGetField( m_pImage, TIFFTAG_TILEWIDTH, &tsx );
		bMipMap &= TIFFGetField( m_pImage, TIFFTAG_TILELENGTH, &tsx );

		/* Second test; is it containing enough directories for us */
		TqInt min = MIN(m_XRes, m_YRes );
		TqInt directory = static_cast<TqInt>(fastlog2(static_cast<TqFloat> (min)));
		if (TIFFSetDirectory(m_pImage, directory - 1) == TqFalse)
		   bMipMap &= TIFFSetDirectory(m_pImage, directory - 2);


		TIFFSetDirectory(m_pImage, 0 );


		/* Support for 3delight, AIR, BMRT, RDC, PIXIE MipMap files.
		 * Aqsis is not bound to have exact multiples of 2 on height, length.
		 * The Format of 3Delight, AIR, BMRT and RDC is more "Plain Texture"/MipMap.
		 * What is preventing us to load their files was the format description file as 
		 * MipMap differ from our format description not the way they store their information.
		 * A better way is to ask the direct question if if the image is stored as MipMap via
		 * TIFFTAG_TILEWIDTH, TIFFTAG_TILELENGTH and checking if the texture contains enough 
		 * directory/pages.
		 */

		if ( bMipMap )
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
	m_Directory = 0;
	for (TqInt k=0; k < 256; k++)
	{
		m_apLast[k] = NULL;
		m_apMipMaps[k].resize(0);
	}
	m_apFlat.resize(0);
}

//---------------------------------------------------------------------
/** Figure out the value passed by the user in the ribfile:
 *  blur, widths and samples (please not the m_samples is read but it 
 *  is never used only in texture(), environment(), shadow(); it is simply used for now.
 *  m_sblur = m_tblur = 0.0, m_pswidth = m_ptwidth = 1.0 and m_samples = 16.0 (for shadow) 
 *  by default.
 */
void CqTextureMap::PrepareSampleOptions( std::map<std::string, IqShaderData*>& paramMap )
{
	m_sblur = 0.0f;   // TurnOff the blur per texture(), environment() or shadow() by default
	m_tblur = 0.0f;
	m_pswidth = 1.0f; // In case of sampling
	m_ptwidth = 1.0f;
	m_samples = 16.0f; // The shadow required to be init. at 16.0 by default

	if (Type() != MapType_Shadow)
		m_samples = 8.0f;
	if (Type() != MapType_Environment)
		m_samples = 8.0f;

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
			{
				paramMap[ "sblur" ] ->GetFloat( m_sblur );
			}
			if ( paramMap.find( "tblur" ) != paramMap.end() )
			{
				paramMap[ "tblur" ] ->GetFloat( m_tblur );
			}
		}

		if ( paramMap.find( "samples" ) != paramMap.end() )
		{
			paramMap[ "samples" ] ->GetFloat( m_samples );
		}

		if ( paramMap.find( "filter" ) != paramMap.end() )
		{
			CqString filter;

			paramMap[ "filter" ] ->GetString( filter );
			//Aqsis::log() << warning << "filter will be " << filter << std::endl;

			m_FilterFunc = CalculateFilter(filter);

		}

		if ( paramMap.find( "pixelvariance" ) != paramMap.end() )
		{
			paramMap[ "pixelvariance" ] ->GetFloat( m_pixelvariance );
		}

	}
}

//---------------------------------------------------------------------
/** Most of the texturemapping passed by this function
 *    (libshadeops->texture1(), environment1()...)
 *  blur, width, samples and filter are effective; even the filter 
 * is taken care. 
 *  m_sblur = m_tblur = 0.0, m_pswidth = m_ptwidth = 1.0 and m_samples = 16 
 *  by default.
 */
void CqTextureMap::SampleMap( TqFloat s1, TqFloat t1, TqFloat swidth, TqFloat twidth, std::valarray<TqFloat>& val)
{
	// Check the memory and make sure we don't abuse it
	CriticalMeasure();

	if ( !IsValid() )
		return ;

	swidth *= m_pswidth;
	twidth *= m_ptwidth;

	// T(s2,t2)-T(s2,t1)-T(s1,t2)+T(s1,t1)
	val.resize( m_SamplesPerPixel );
	val = 0.0f;

	TqFloat ss1, ss2, tt1, tt2;

	if ( m_smode == WrapMode_Periodic )
	{
		s1 = fmod( s1, 1.0f );
		if ( s1 < 0.0 )
			s1 += 1.0f;
	}
	if ( m_tmode == WrapMode_Periodic )
	{
		t1 = fmod( t1, 1.0f );
		if ( t1 < 0.0 )
			t1 += 1.0f;
	}

	if ( m_smode == WrapMode_Black )
	{
		if ( ( s1 < 0.0f ) ||
		        ( s1 > 1.0f ) )
			return ;
	}
	if ( m_tmode == WrapMode_Black )
	{
		if ( ( t1 < 0.0f ) ||
		        ( t1 > 1.0f ) )
			return ;
	}

	if ( m_smode == WrapMode_Clamp || Type() == MapType_Environment )
	{
		s1 = CLAMP( s1, 0.0f, 1.0f );
	}
	if ( m_tmode == WrapMode_Clamp || Type() == MapType_Environment )
	{
		t1 = CLAMP( t1, 0.0f, 1.0f );
	}
	ss1 = s1 - swidth - ( m_sblur * 0.5f );
	tt1 = t1 - twidth - ( m_tblur * 0.5f );
	ss1 = CLAMP(ss1, 0.0f, 1.0f);
	tt1 = CLAMP(tt1, 0.0f, 1.0f );

	ss2 = s1 + swidth + ( m_sblur * 0.5f );
	tt2 = t1 + twidth + ( m_tblur * 0.5f );
	ss2 = CLAMP(ss2, 0.0f, 1.0f);
	tt2 = CLAMP(tt2, 0.0f, 1.0f );

	/* make ss1 is always less or equal to ss2
	* tt1 is always less or equal to tt2
	*/
	TqFloat tmp;
	tmp = ss1;
	ss1 = MIN(ss1, ss2);
	ss2 = MAX(tmp, ss2);
	tmp = tt1;
	tt1 = MIN(tt1, tt2);
	tt2 = MAX(tmp, tt2);

	GetSample( ss1, tt1, ss2, tt2, val);
}


//----------------------------------------------------------------------
/** Retrieve a sample from the MIP MAP over the area specified by the four vertices
 */

void CqTextureMap::SampleMap( TqFloat s1, TqFloat t1, TqFloat s2, TqFloat t2, TqFloat s3, TqFloat t3, TqFloat s4, TqFloat t4,
                              std::valarray<TqFloat>& val)
{
	val.resize( m_SamplesPerPixel );
	val = 0.0f;

	// Work out the width and height
	TqFloat ss1, tt1, ss2, tt2;
	ss1 = MIN( MIN( MIN( s1, s2 ), s3 ), s4 );
	tt1 = MIN( MIN( MIN( t1, t2 ), t3 ), t4 );
	ss2 = MAX( MAX( MAX( s1, s2 ), s3 ), s4 );
	tt2 = MAX( MAX( MAX( t1, t2 ), t3 ), t4 );

	// By definition the area sampling is requested.
	// Primary used by shadow() calls
	GetSample( ss1, tt1, ss2, tt2, val);
}


//----------------------------------------------------------------------
/** Write an image to an open TIFF file in the current directory as straight storage.
 * as unsigned char values
 */
void CqTextureMap::WriteImage( TIFF* ptex, TqPuchar raster, TqUlong width, TqUlong length, TqInt samples, TqInt compression, TqInt quality )
{
	// First check if we can support the requested compression format.
	if(!TIFFIsCODECConfigured(compression))
	{
		Aqsis::log() << error << "Compression type " << compression << " not supported by the libtiff implementation" << std::endl;
		return;
	}
	TqChar version[ 80 ];
	TIFFCreateDirectory( ptex );

	sprintf( version, "%s %s", STRNAME, VERSION_STR );
	TIFFSetField( ptex, TIFFTAG_SOFTWARE, ( char* ) version );
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
	// First check if we can support the requested compression format.
	if(!TIFFIsCODECConfigured(compression))
	{
		Aqsis::log() << error << "Compression type " << compression << " not supported by the libtiff implementation" << std::endl;
		return;
	}
	TqChar version[ 80 ];
	TIFFCreateDirectory( ptex );

	sprintf( version, "%s %s", STRNAME, VERSION_STR );
	TIFFSetField( ptex, TIFFTAG_SOFTWARE, ( char* ) version );
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
/** Write an image to an open TIFF file in the current directory as straight storage.
 * as 16 bit int values
 */
void CqTextureMap::WriteImage( TIFF* ptex, TqUshort *raster, TqUlong width, TqUlong length, TqInt samples, TqInt compression, TqInt quality )
{
	// First check if we can support the requested compression format.
	if(!TIFFIsCODECConfigured(compression))
	{
		Aqsis::log() << error << "Compression type " << compression << " not supported by the libtiff implementation" << std::endl;
		return;
	}
	TqChar version[ 80 ];
	TIFFCreateDirectory( ptex );

	sprintf( version, "%s %s", STRNAME, VERSION_STR );
	TIFFSetField( ptex, TIFFTAG_SOFTWARE, ( char* ) version );
	TIFFSetField( ptex, TIFFTAG_IMAGEWIDTH, width );
	TIFFSetField( ptex, TIFFTAG_IMAGELENGTH, length );
	TIFFSetField( ptex, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG );
	TIFFSetField( ptex, TIFFTAG_BITSPERSAMPLE, 16 );
	TIFFSetField( ptex, TIFFTAG_SAMPLESPERPIXEL, samples );
	TIFFSetField( ptex, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
	TIFFSetField( ptex, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT );
	TIFFSetField( ptex, TIFFTAG_COMPRESSION, compression ); /* COMPRESSION_DEFLATE */
	TIFFSetField( ptex, TIFFTAG_ROWSPERSTRIP, 1 );
	TIFFSetField( ptex, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB );

	TqUshort *pdata = raster;
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

			case BufferType_Int16:
			{
				WriteImage( ptex, static_cast<TqUshort*>( pBuffer->pVoidBufferData() ), pBuffer->Width(), pBuffer->Height(), pBuffer->Samples(), compression, quality );
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

			case BufferType_Int16:
			{
				WriteTileImage( ptex, static_cast<TqUshort*>( pBuffer->pVoidBufferData() ), pBuffer->Width(), pBuffer->Height(), twidth, theight, pBuffer->Samples(), compression, quality );
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
	// First check if we can support the requested compression format.
	if(!TIFFIsCODECConfigured(compression))
	{
		Aqsis::log() << error << "Compression type " << compression << " not supported by the libtiff implementation" << std::endl;
		return;
	}
	//TIFFCreateDirectory(ptex);
	std::ostringstream version;
	version << STRNAME << " " << VERSION_STR << std::ends;
	TIFFSetField( ptex, TIFFTAG_SOFTWARE, version.str ().c_str () );
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
						{
							TqFloat value = ptdata[ ( ( j * samples ) + ii ) ];
							ptile[ ( i * twidth * samples ) + ( ( ( j * samples ) + ii ) ) ] = value;
						}
					}
				}
				ptdata += ( width * samples );
			}
			TIFFWriteTile( ptex, ptile, x, y, 0, 0 );
		}
		TIFFWriteDirectory( ptex );
		_TIFFfree( ptile );
	}
}


//----------------------------------------------------------------------
/** Write an image to an open TIFF file in the current directory as tiled storage.
 * as 16 bit int values
 */
void CqTextureMap::WriteTileImage( TIFF* ptex, TqUshort *raster, TqUlong width, TqUlong length, TqUlong twidth, TqUlong tlength, TqInt samples, TqInt compression, TqInt quality )
{
	// First check if we can support the requested compression format.
	if(!TIFFIsCODECConfigured(compression))
	{
		Aqsis::log() << error << "Compression type " << compression << " not supported by the libtiff implementation" << std::endl;
		return;
	}
	//TIFFCreateDirectory(ptex);
	std::ostringstream version;
	version << STRNAME << " " << VERSION_STR << std::ends;
	TIFFSetField( ptex, TIFFTAG_SOFTWARE, version.str ().c_str () );
	TIFFSetField( ptex, TIFFTAG_IMAGEWIDTH, width );
	TIFFSetField( ptex, TIFFTAG_IMAGELENGTH, length );
	TIFFSetField( ptex, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG );
	TIFFSetField( ptex, TIFFTAG_BITSPERSAMPLE, 16 );
	TIFFSetField( ptex, TIFFTAG_SAMPLESPERPIXEL, samples );
	TIFFSetField( ptex, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
	TIFFSetField( ptex, TIFFTAG_TILEWIDTH, twidth );
	TIFFSetField( ptex, TIFFTAG_TILELENGTH, tlength );
	TIFFSetField( ptex, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT );
	TIFFSetField( ptex, TIFFTAG_COMPRESSION, compression );


	TqInt tsize = twidth * tlength;
	TqInt tperrow = ( width + twidth - 1 ) / twidth;
	TqUshort* ptile = static_cast<TqUshort*>( _TIFFmalloc( tsize * samples * sizeof( TqUshort ) ) );

	if ( ptile != NULL )
	{
		TqInt ctiles = tperrow * ( ( length + tlength - 1 ) / tlength );
		TqInt itile;
		for ( itile = 0; itile < ctiles; itile++ )
		{
			TqInt x = ( itile % tperrow ) * twidth;
			TqInt y = ( itile / tperrow ) * tlength;
			TqUshort* ptdata = raster + ( ( y * width ) + x ) * samples;
			// Clear the tile to black.
			memset( ptile, 0, tsize * samples * sizeof( TqUshort ) );
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
		_TIFFfree( ptile );
	}
}


//----------------------------------------------------------------------
/** Write an image to an open TIFF file in the current directory as tiled storage.
 * as unsigned char values
 */
void CqTextureMap::WriteTileImage( TIFF* ptex, TqPuchar raster, TqUlong width, TqUlong length, TqUlong twidth, TqUlong tlength, TqInt samples, TqInt compression, TqInt quality )
{
	// First check if we can support the requested compression format.
	if(!TIFFIsCODECConfigured(compression))
	{
		Aqsis::log() << error << "Compression type " << compression << " not supported by the libtiff implementation" << std::endl;
		return;
	}
	std::ostringstream version;
	version << STRNAME << " " << VERSION_STR << std::ends;
	TIFFSetField( ptex, TIFFTAG_SOFTWARE, version.str ().c_str () );
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
		_TIFFfree( ptile );
	}
}


//----------------------------------------------------------------------
/** Retrieve a sample from the environment map using R as the reflection vector.
 */

#define COMP_X 0
#define COMP_Y 1
#define COMP_Z 2

void CqEnvironmentMap::SampleMap( CqVector3D& R1,
                                  CqVector3D& R2, CqVector3D& R3, CqVector3D& R4,
                                  std::valarray<TqFloat>& val, TqInt index,
                                  TqFloat* average_depth, TqFloat* shadow_depth )
{
	CqVector3D D;
	TqFloat x,y;
	TqFloat contrib, mul, t, u, v;
	TqInt i;
	EOrder order;
	TqFloat side[2];
	register TqInt c;

        u = v = 0.0f;

	TIME_SCOPE("Environment mapping")

	// Textures are filtered with either bilinear or trilinear mip-mapping.
	// Trilinear gives higher image quality but takes more time.  To disable trilinear
	// mip-mapping for the entire scene:
	//        Option "texture" "float lerp" 0.0 or
	// provide to texture() call in the shader.
	if (m_lerp == -1.0)
	{
		const TqInt* pLerp = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "texture", "lerp" );

		m_lerp = 0.0f;
		if (pLerp && (*pLerp > 0.0f))
			m_lerp = 1.0f;
	}
	TqBool bLerp = (m_lerp == 1.0);

	if ( m_pImage != 0 )
	{
		val.resize( m_SamplesPerPixel );
		val = 0.0f;
		m_accum_color = 0.0f;
		contrib = 0.0f;

		if ((R1 * R1) == 0)
			return;

		TqFloat dfovu = fabs(1.0f - m_fov)/(TqFloat) (m_XRes);
		TqFloat dfovv = fabs(1.0f - m_fov)/(TqFloat) (m_YRes);
		for (i=0; i < m_samples; i++)
		{
			CalculateNoise(x, y, i);

			D = LERP(y, LERP(x, R1, R2), LERP(x, R3, R4));

			mul = (*m_FilterFunc)(x-0.5, y-0.5, 1.0, 1.0);
			if (mul < m_pixelvariance)
				continue;

			contrib += mul;

			// Find the side of the cube that we're looking at
			if (fabs(D[COMP_Y]) > fabs(D[COMP_X]))
			{
				if (fabs(D[COMP_Z]) > fabs(D[COMP_Y]))
				{
					order   =   ZYX;
				}
				else
				{
					if (fabs(D[COMP_Z]) > fabs(D[COMP_X]))
						order   =   YZX;
					else
						order   =   YXZ;
				}
			}
			else if (fabs(D[COMP_Z]) > fabs(D[COMP_Y]))
			{
				if (fabs(D[COMP_Z]) > fabs(D[COMP_X]))
					order   =   ZXY;
				else
					order   =   XZY;
			}
			else
			{
				order =   XYZ;
			}

			switch(order)
			{
					case XYZ:
					case XZY:
					if (D[COMP_X] > 0)
					{
						memcpy(side,   &sides[PX][0], sizeof(TqFloat) * 2);
						t =   1 / D[COMP_X];
						u =   (-D[COMP_Z]*t+1)*(float) 0.5;
						v =   (-D[COMP_Y]*t+1)*(float) 0.5;
					}
					else
					{
						memcpy(side,   &sides[NX][0], sizeof(TqFloat) * 2);
						t =   -1 / D[COMP_X];
						u =   (D[COMP_Z]*t+1)*(float) 0.5;
						v =   (-D[COMP_Y]*t+1)*(float) 0.5;
					}
					break;
					case YXZ:
					case YZX:
					if (D[COMP_Y] > 0)
					{
						memcpy(side, &sides[PY][0], sizeof(TqFloat) * 2);
						t =   1 / D[COMP_Y];
						u =   (D[COMP_X]*t+1)*(float) 0.5;
						v =   (D[COMP_Z]*t+1)*(float) 0.5;
					}
					else
					{
						memcpy(side,   &sides[NY][0], sizeof(TqFloat) * 2);
						t =   -1 / D[COMP_Y];
						u =   (D[COMP_X]*t+1)*(float) 0.5;
						v =   (-D[COMP_Z]*t+1)*(float) 0.5;
					}
					break;
					case ZXY:
					case ZYX:
					if (D[COMP_Z] > 0)
					{
						memcpy(side,   &sides[PZ][0], sizeof(TqFloat) * 2);
						t =   1 / D[COMP_Z];
						u =   (D[COMP_X]*t+1)*(float) 0.5;
						v =   (-D[COMP_Y]*t+1)*(float) 0.5;
					}
					else
					{
						memcpy(side,   &sides[NZ][0], sizeof(TqFloat) * 2);
						t =   -1 / D[COMP_Z];
						u =   (-D[COMP_X]*t+1)*(float) 0.5;
						v =   (-D[COMP_Y]*t+1)*(float) 0.5;
					}
					break;
			}

			/* At this point: u,v are between 0..1.0
			* They must be remapped to their correct 
			* position 
			*/

			u = CLAMP(u, dfovu, 1.0f );
			v = CLAMP(v, dfovv, 1.0f );

			u = side[0] + u*ONETHIRD;
			v = side[1] + v*ONEHALF;
			u = CLAMP(u, 0.0f, 1.0f);
			v = CLAMP(v, 0.0f, 1.0f);
			CalculateLevel(u, v);

			BiLinear(u, v, m_umapsize, m_vmapsize, m_level, m_pixel_variance);
			if (bLerp)
				BiLinear(u, v, m_umapsize/2, m_vmapsize/2, m_level+1, m_pixel_sublevel);


			if (bLerp)
			{
				for (c = 0; c < m_SamplesPerPixel; c++)
				{
					m_accum_color[c] += mul * LERP(m_interp, m_pixel_variance[c], m_pixel_sublevel[c]);

				}
			}
			else
			{
				for (c = 0; c < m_SamplesPerPixel; c++)
				{
					m_accum_color[c] += mul * m_pixel_variance[c];
				}
			}
		}

		for (c = 0; c < m_SamplesPerPixel; c++)
			val[c] = m_accum_color[c] / contrib;

	}
}

END_NAMESPACE( Aqsis )

