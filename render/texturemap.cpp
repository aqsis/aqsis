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
#include	"logging.h"

#ifndef		AQSIS_SYSTEM_WIN32
#include	"unistd.h"
#endif

START_NAMESPACE( Aqsis )

// Local Constants

#define MEG1    8192*1024
#undef ALLOCSEGMENTSTATUS


//
// #define WITHFILTER 1 if you want filtering even when you are doing with trilinear sampling;
//     this is slow since we RiSincFilter (without any form of catch) on top of the area sampling.
//
// #define CORRECTEDVARIOUS if you want filtering various idx across different microgrid size.
//
#define CORRECTEDVARIOUS 1
#define WITHFILTER       1

#define INTERPOLATE1(A,B,C) (A + (B-A) * C)
#define INTERPOLATE2(A,B,C,D,U,V) INTERPOLATE1(INTERPOLATE1(A,B,U), INTERPOLATE1(C,D,U), V)

//
// Local Variables
//
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

//----------------------------------------------------------------------
// ForceCorrect() force to correct multiple level of mipmap to be the same.
// Option "limits" "integer texturecorrect" 1 this is only applicable with GetSampleTri()
//

static TqInt ForceCorrect()
{

    static TqInt correct = -1;
    if (correct == -1)
    {
        const TqInt* pCorrect =  QGetRenderContextI()->GetIntegerOption("limits","texturecorrect");

        correct = 0;
        if (pCorrect) {
            correct = pCorrect[0];
        }
    }
    return correct;
}

//----------------------------------------------------------------------
// FindBlurRatio() find a better ratio
// Option "limits" "float textureblur" 1.2 by default it is 1.2
//

static TqFloat FindBlurRatio()
{

    static TqFloat sqr = -1.0;
    if (sqr < 0.0)
    {
        const TqFloat* pCorrect =  QGetRenderContextI()->GetFloatOption("limits","textureblur");

        sqr = 1.2;
        if (pCorrect) {
            sqr = MAX(0.1 , pCorrect[0]);
        }
    }
    return sqr;
}


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
            std::cerr << warning << "Exceeding allocated texture memory by " << more - limit << std::endl;
        }

        report = 0;
        m_critical = TqTrue;
    }

#ifdef _DEBUG
    if ( ( more > MEG1 ) && ( ( more / ( 1024 * 1024 ) ) > megs ) )
    {
        std::cerr << debug << "Texturememory is more than " << megs << " megs" << std::endl;
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
    // Suspicious if this does not have an extension.
    if (strName.rfind(".") == -1) return 0; 

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

#elif	AQSIS_SYSTEM_WIN32
    char acPath[255];

    if ( GetModuleFileName( NULL, acPath, 256 ) != 0) 
    {
	// guaranteed file name of at least one character after path
	*( strrchr( acPath, '\\' ) + 1 ) = '\0';
    } else return 0;

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
        std::cerr << info << "I was forced to zap the tile segment buffers for " << (int)( now - current ) / 1024 << "K" << std::endl;
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
            std::cerr << error << "Cannot MIPMAP a tiled image \"" << m_strName.c_str() << "\"" << std::endl;
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
                m_apSegments.push_back( pTMB );
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
        std::cerr << "alloc/free " << alloc_cnt << " " << free_cnt << " - Memory usage " << QGetRenderContext() ->Stats().GetTextureMemory() << std::endl;
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

    if ( m_apSegments.size() > 0 && m_apSegments.front() ->IsValid( s, t, directory ) )
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
            std::cerr << error << "Cannot open texture file \"" << m_strName.c_str() << "\"" << std::endl;
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



//----------------------------------------------------------------------
/** Bilinear sample of any directory/u/v Color the result is saved into
 * either m_low_color, or m_high_color depending of the param m_color. 
 * This is the implementation for the releases < 0.9.2
 * \param u is average of sample positions.
 * \param v is average of sample positions.
 * \param umapsize the mapsize at the directory id
 * \param vmapsize the mapsize at the direction id
 * \param id     the directory in the tiff 0...n
 * \param param m_color the result will be stored in m_low_color or m_high_color.
 */

TqBool CqTextureMap::BiLinear(TqFloat u, TqFloat v, TqInt umapsize, TqInt vmapsize,
                              TqInt id, std::valarray<TqFloat >	&m_color)
{
	TqUint umapsize1 = umapsize-1; 
	TqUint vmapsize1 = vmapsize-1;

    TqUint iu = FLOOR( u * umapsize1);
    TqDouble ru = u * umapsize1 - iu;
    TqUint iu_n = FLOOR( u * umapsize1 + 1.0);
    iu = iu % umapsize;		/// \todo This is wrap mode periodic.
    iu_n = iu_n % umapsize;	/// \todo This is wrap mode periodic.

    TqUint iv = FLOOR( v * vmapsize1 );
    TqDouble rv = v * vmapsize1 - iv;
    TqUint iv_n = FLOOR( v * vmapsize1 + 1.0);
    iv = iv % vmapsize;		/// \todo This is wrap mode periodic.
    iv_n = iv_n % vmapsize;	/// \todo This is wrap mode periodic.

    // Read in the relevant texture tiles.
    register TqInt c;

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

    /* cannot find anything than goodbye */
    if ( !pTMBa || !pTMBb || !pTMBc || !pTMBd )
    {
        for ( c = 0; c < m_SamplesPerPixel; c++ )
        {
            m_color[ c ] = 1.0f;
        }

        std::cerr << error << "Cannot find value for either pTMPB[a,b,c,d]" << std::endl;
        return TqFalse;
    }


    // Bilinear interpolate the values at the corners of the sample.
    iu -= pTMBa->sOrigin();
    iu_n -= pTMBc->sOrigin();
    iv -= pTMBa->tOrigin();
    iv_n -= pTMBb->tOrigin();

    for ( c = 0; c < m_SamplesPerPixel; c++ )
    {
        TqFloat Val00 = pTMBa->GetValue( iu, iv, c );
        TqFloat Val01 = pTMBc->GetValue( iu_n, iv, c );
        TqFloat Val10 = pTMBb->GetValue( iu, iv_n, c );
        TqFloat Val11 = pTMBd->GetValue( iu_n, iv_n, c );
        m_color[ c ] = INTERPOLATE2(Val00, Val01, Val10, Val11, ru , rv );
    }
    return TqTrue;
    }

//----------------------------------------------------------------------
/* CqTextureMap::GetSampleSgle( ) is
 * This is the implementation based on classical pyramid integration for mipmap;
 * I don't see wrong with it but is only doing a one shot sampling which is prone to give some 
 * artifacts when the texture() required some blur or ShadingRate is high. This could not be used as soon as you have some blur or sampling over an area
 * in case of shadowing per examples. See SampleMap()s for when to use or not use this method.
 * \param u1,v2 is top/right bottom/left of sample positions.
 * \param v1,v2.
 * \param val the result will be stored.
 */
void CqTextureMap::GetSampleSgle( TqFloat u1, TqFloat v1, TqFloat u2, TqFloat v2, std::valarray<TqFloat>& val )
{
    register TqInt c;

    QGetRenderContext() ->Stats().TextureMapTimer().Start();

    // u/v is provided to the function now; they are the average of
    // sample positions (u1,v1), (u2, v2).

    // Calculate the appropriate mipmap level from the area subtended by the sample.
    TqFloat UVArea = sqrt( fabs(( u2 - u1 ) * ( v2 - v1 )));
    TqFloat u = (u2+u1)/2.0;
    TqFloat v = (v2+v1)/2.0;

    // Find out which two layers the of the pyramid d lies between.
    TqFloat d = MIN(UVArea, 1.0f);

    // Adjust u and v for the pyramid level.
    TqUint size = MIN(m_XRes, m_YRes);
    TqUint id = (TqUint) FLOOR( d * size);

    // If we are on the edge of the texture only achive singlelevel mipmap.
    TqBool singlelevel = ( id == 0) ||  (id == size);

    TqInt umapsize = m_XRes;
    TqInt vmapsize = m_YRes;
    TqInt level = 0;

    while(id > 1) {
        id >>= 1;
        umapsize >>= 1;
        vmapsize >>= 1;
        level++;
		
        if (umapsize < 8 ) break;
        if (vmapsize < 8 ) break;
    }

    BiLinear(u, v, umapsize, vmapsize, level, m_low_color);
    
    if (singlelevel) {
        for (c = 0; c < m_SamplesPerPixel; c++) {
            val[c] = m_low_color[c];
    }
    } else {
        size = MIN(umapsize, vmapsize);
        TqFloat a = 1.0/(TqFloat)size;
        TqFloat b = 1.0/(TqFloat)(size/2);
        TqFloat interp = (d - a) / (b - a);
        umapsize >>= 1;
        vmapsize >>= 1;
        level ++;


        BiLinear(u, v, umapsize, vmapsize, level, m_high_color);

        for (c = 0; c < m_SamplesPerPixel; c++) {
            val[c] = INTERPOLATE1(m_low_color[c], m_high_color[c], interp);
        }
        }

    QGetRenderContext() ->Stats().TextureMapTimer().Stop();
        }

//----------------------------------------------------------------------
/* CqTextureMap::GetSampleTri( ) is
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
void CqTextureMap::GetSampleTri( TqFloat u1, TqFloat v1, TqFloat u2, TqFloat v2, std::valarray<TqFloat>& val )
        {
    register TqInt c;

    QGetRenderContext() ->Stats().TextureMapTimer().Start();

    TqFloat cs = (u1 + u2) * 0.5;
    TqFloat ct = (v1 + v2) * 0.5;
    TqFloat ds,dt,diag;
#ifdef WITHFILTER 
    RtFilterFunc pFilter = RiSincFilter;
#endif

    ds = u1 - cs;
    dt = v1 - ct;
    diag = (ds *ds * m_XRes * m_XRes) +  (dt * dt * m_YRes * m_YRes);

    ds = u2 - cs;
    dt = v2 - ct;
    diag = MIN(diag, (ds *ds * m_XRes * m_XRes) +  (dt * dt * m_YRes * m_YRes));

    TqFloat l = log(diag) / (2.0 * log(2.0));
    l = MAX(l, 0.0);
    TqInt id = (int) floor(l);
    TqFloat offset = l - id;
    offset = MIN(offset, 1.0);
    TqInt umapsize = m_XRes;
    TqInt vmapsize = m_YRes;
    TqBool singlelevel = TqFalse;

    TqInt idx;

#ifdef CORRECTEDVARIOUS
    // Important note: if ever the texturesampling determine a certain
    // directory could be used other than what directory was already used for any microgrids.
    // We will prefer to chose the minimum between the current number and the existant; until
    // the texture is zapped. It eliminates texture bleeding at the edge of the primitive
    // or dissimilitudes between two different and adjacent microgrids.
    if (m_Directory && ForceCorrect()) {
        if (id > m_Directory) {
            id = m_Directory;
        }
        }
#endif

    /* With id lower down the umapsize and vmapsize until idx matches id and
     * umapsize and vmapsize is bigger than 2.
     */
    for (idx=0; (idx < id) && (umapsize > 8) && (vmapsize > 8); idx++, umapsize >>= 1, vmapsize >>= 1);

    /*
     * if idx is different than id 
     * than set id to be idx since we could not met all the conditions
     */
    if (idx != id) id = idx;
    m_low_color = 0;
    m_Directory = id;
    if ((umapsize < 3) || (vmapsize < 3)) singlelevel = 1;
    if ((cs == 0.0) || (cs == 1.0)) singlelevel = 1;
    if ((ct == 0.0) || (ct == 1.0)) singlelevel = 1;


    // Will it may make some sense to add some random value between 0..1.0 ?
    // For now I just put a delta based on the texel deplacement.
    TqFloat mul, div;

    // Area integration; it will be the ideal placed to put to the contribution
    // of each texel the filter factor.
    TqFloat u, v;

    m_low_color = 0;
    div = 0.0;
    TqFloat deltau;
    TqFloat deltav;

    deltau = 1.0/(m_pswidth * umapsize);
    deltav = 1.0/(m_ptwidth * vmapsize);

    for (u = u1; u <= u2; u += deltau) {
        for (v = v1; v <= v2; v += deltav)
        {
            BiLinear(u, v, umapsize, vmapsize, id, m_tempval1);

            /* Yes we use Disk filter */
#ifdef WITHFILTER
            mul = (*pFilter)((u-cs), (v-ct), 2.0 * cs, 2.0 * ct);
#else
            mul = 1.0;
#endif
            if (mul < FLT_EPSILON) continue;
            div += mul;
        for ( c = 0; c < m_SamplesPerPixel; c++ )
                m_low_color[c] += mul * m_tempval1[c];
        }
        }

    for (c = 0; c < m_SamplesPerPixel; c++ )
        m_low_color[c] /= div;


    // This is never the case since even at the border a valid texel will be provided
    // but do singlelevel = 1 to debug the supersampling, filtering
    if (singlelevel)
    {
        /*
        *  we will store the value back to val if we are in singlelevel mode
        *  
        */
        for ( c = 0; c < m_SamplesPerPixel; c++ )
            val[ c ] = m_low_color[ c ];

    } else {


        //
        // umapsize and vmapsize must be at least 2 pixels here; right they are garantee to
        // at least bigger than 64 (size of unique tile).
        //
        umapsize >>= 1;
        vmapsize >>= 1;
        id ++;

        m_high_color = 0;
        div = 0.0;
        deltau = 1.0/(m_pswidth * umapsize);
        deltav = 1.0/(m_ptwidth * vmapsize);

        // Recompute the delta based on the new pyramid level; but
        // this is also the place to put as contribution factor the filter correction.
        for (u = u1; u <= u2; u += deltau){
            for (v = v1; v <= v2; v += deltav)
            {
                BiLinear(u, v, umapsize, vmapsize, id, m_tempval1);
#ifdef WITHFILTER
                mul = (*pFilter)((u-cs), (v-ct), 2.0 * cs, 2.0 * ct);
#else
                mul = 1.0;
#endif
                if (mul < FLT_EPSILON) continue;
                div += mul;
                for (c = 0; c < m_SamplesPerPixel; c++ )
                    m_high_color[c] += mul * m_tempval1[c];
            }
    }

        for (c = 0; c < m_SamplesPerPixel; c++ )
            m_high_color[c] /= div;


        // Linearly interpolate between low_color and high_color by dinterp.
        // Please note the m_low_color value is the best value; and m_high_color
        // is less precise.

        for (c = 0; c < m_SamplesPerPixel; c++ )
            val[c] =  ((1.0 - offset) * m_low_color[c]) + ((offset) * m_high_color[c]);

    }
    QGetRenderContext() ->Stats().TextureMapTimer().Stop(); 
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
    pNew->Open();

    // Ensure that it is in the correct format
    if ( pNew->Format() != TexFormat_MIPMAP )
    {
        if( !pNew->CreateMIPMAP( TqTrue ) )
            pNew->SetInvalid();
        pNew->Close();
    }

    m_TextureMap_Cache.push_back( pNew );
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
                if (mul == 0.0) continue;

                /* find the value in the original image */
                TqInt ypos = (TqInt) (fy*m_YRes-1) + j;
                TqInt xpos = (TqInt) (fx*m_XRes-1) + i;
                if (ypos < 0) continue;
                if (xpos < 0) continue;
                if (ypos > (TqInt) m_YRes - 1)continue;
                if (xpos > (TqInt) m_XRes - 1) continue;
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
        std::cerr << error << "Cannot open texture file \"" << m_strName.c_str() << "\"" << std::endl;
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
    } else {
        m_pImage = TIFFOpen( strRealName.c_str(), "r" );
    }


    if ( m_pImage )
    {
	    std::cerr << info << "TextureMap: \"" << strRealName.c_str() << "\" is open" << std::endl;
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
        m_tempval1.resize( m_SamplesPerPixel );
        m_tempval2.resize( m_SamplesPerPixel );
        m_tempval3.resize( m_SamplesPerPixel );
        m_tempval4.resize( m_SamplesPerPixel );
        m_low_color.resize( m_SamplesPerPixel );
        m_high_color.resize( m_SamplesPerPixel );

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
        TqInt directory = static_cast<TqInt>(log((double) min)/log((double) 2.0));
        bMipMap &= TIFFSetDirectory(m_pImage, directory - 1);


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
    m_sblur = 0.0f;   // Turnoff the blur per texture(), environment() or shadow() by default
    m_tblur = 0.0f;
    m_pswidth = 1.0f; // In case of Trilinear sampling
    m_ptwidth = 1.0f;
    m_samples = 16.0f; // The shadow required to be init. at 16.0 by default

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
            m_sblur = m_sblur / FindBlurRatio();
            m_tblur = m_sblur;
        }
        else
        {
            if ( paramMap.find( "sblur" ) != paramMap.end() ) {
                paramMap[ "sblur" ] ->GetFloat( m_sblur );
                m_sblur = m_sblur / FindBlurRatio();
            }
            if ( paramMap.find( "tblur" ) != paramMap.end() ) {
                paramMap[ "tblur" ] ->GetFloat( m_tblur );
                m_tblur = m_tblur / FindBlurRatio();
            }
        }
        if ( paramMap.find( "samples" ) != paramMap.end() )
            paramMap[ "samples" ] ->GetFloat( m_samples );
    }
}

//----------------------------------------------------------------------
/* CqTextureMap::GetSample( ) is calling either GetSampleTri() or
 * GetSampleSgle() depending of the level of blur or if 
 * ForceTrilinear() returns TRUE;
 */
void CqTextureMap::GetSample( TqFloat u1, TqFloat v1, TqFloat u2, TqFloat v2, std::valarray<TqFloat>& val )
{
    // You could force it too
    // by defining "Options" "limit" "trilinear" 1 in your rib file
    if (m_sblur || m_tblur)
    {
        GetSampleTri( u1, v1, u2, v2, val );
    } else {
        // without blur it is not necessary to do GetSampleTri().
        GetSampleSgle( u1, v1, u2, v2, val );
    }
}

//---------------------------------------------------------------------
/** Most of the texturemapping passed by this function
 *    (libshadeops->texture1(), environment1()...)
 *  blur, widths and samples (please not the m_samples is read but it 
 *  will used only in GetSampleTri(); it is simply ingnored by GetSampleSgle().
 *  m_sblur = m_tblur = 0.0, m_pswidth = m_ptwidth = 1.0 and m_samples = 16 
 *  by default.
 */
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

    TqFloat ss1, ss2, tt1, tt2;

    if ( m_smode == WrapMode_Periodic )
    {
        s1 = fmod( s1, 1.0f );
        if ( s1 < 0.0 ) s1 += 1.0f;
    }
    if ( m_tmode == WrapMode_Periodic )
    {
        t1 = fmod( t1, 1.0f );
        if ( t1 < 0.0 ) t1 += 1.0f;
    }

    if ( m_smode == WrapMode_Black )
    {
        if ( ( s1 < 0.0f ) ||
                ( s1 > 1.0f ) ) return ;
    }
    if ( m_tmode == WrapMode_Black )
    {
        if ( ( t1 < 0.0f ) ||
                ( t1 > 1.0f ) ) return ;
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
        tt1 = CLAMP( tt1, 0.0f, 1.0f );

    ss2 = s1 + swidth + ( m_sblur * 0.5f );
    tt2 = t1 + twidth + ( m_tblur * 0.5f );
    ss2 = CLAMP(ss2, 0.0f, 1.0f);
        tt2 = CLAMP( tt2, 0.0f, 1.0f );

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

        GetSample( ss1, tt1, ss2, tt2, val );


}


//----------------------------------------------------------------------
/** Retrieve a sample from the MIP MAP over the area specified by the four vertices
 */

void CqTextureMap::SampleMap( TqFloat s1, TqFloat t1, TqFloat s2, TqFloat t2, TqFloat s3, TqFloat t3, TqFloat s4, TqFloat t4,
                              std::valarray<TqFloat>& val )
{
    val.resize( m_SamplesPerPixel );
    val = 0.0f;

    // Work out the width and height
    TqFloat ss1, tt1, ss2, tt2;
    ss1 = MIN( MIN( MIN( s1, s2 ), s3 ), s4 );
    tt1 = MIN( MIN( MIN( t1, t2 ), t3 ), t4 );
    ss2 = MAX( MAX( MAX( s1, s2 ), s3 ), s4 );
    tt2 = MAX( MAX( MAX( t1, t2 ), t3 ), t4 );

    // By definition the trilinear multiple area sampling is requested.
    // Primary used by shadow() calls
    GetSampleTri( ss1, tt1, ss2, tt2, val );
}


//----------------------------------------------------------------------
/** Write an image to an open TIFF file in the current directory as straight storage.
 * as unsigned char values
 */


void CqTextureMap::WriteImage( TIFF* ptex, TqPuchar raster, TqUlong width, TqUlong length, TqInt samples, TqInt compression, TqInt quality )
{
    TqChar version[ 80 ];
    TIFFCreateDirectory( ptex );

    sprintf( version, "%s %s", STRNAME, VERSION_STR );
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

    sprintf( version, "%s %s", STRNAME, VERSION_STR );
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
/** Write an image to an open TIFF file in the current directory as straight storage.
 * as 16 bit int values
 */

void CqTextureMap::WriteImage( TIFF* ptex, TqUshort *raster, TqUlong width, TqUlong length, TqInt samples, TqInt compression, TqInt quality )
{
    TqChar version[ 80 ];
    TIFFCreateDirectory( ptex );

    sprintf( version, "%s %s", STRNAME, VERSION_STR );
    TIFFSetField( ptex, TIFFTAG_SOFTWARE, ( uint32 ) version );
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
    //TIFFCreateDirectory(ptex);
    TqChar version[ 80 ];
    sprintf( version, "%s %s", STRNAME, VERSION_STR );
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
 * as 16 bit int values
 */

void CqTextureMap::WriteTileImage( TIFF* ptex, TqUshort *raster, TqUlong width, TqUlong length, TqUlong twidth, TqUlong tlength, TqInt samples, TqInt compression, TqInt quality )
{
    //TIFFCreateDirectory(ptex);
    TqChar version[ 80 ];
    sprintf( version, "%s %s", STRNAME, VERSION_STR );
    TIFFSetField( ptex, TIFFTAG_SOFTWARE, ( uint32 ) version );
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

    }
}


//----------------------------------------------------------------------
/** Write an image to an open TIFF file in the current directory as tiled storage.
 * as unsigned char values
 */

void CqTextureMap::WriteTileImage( TIFF* ptex, TqPuchar raster, TqUlong width, TqUlong length, TqUlong twidth, TqUlong tlength, TqInt samples, TqInt compression, TqInt quality )
{
    TqChar version[ 80 ];
    sprintf( version, "%s %s", STRNAME, VERSION_STR );
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

