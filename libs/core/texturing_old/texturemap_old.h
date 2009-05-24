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
		\brief Declares texture map handling and cacheing classes.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef TEXTUREMAP_H_INCLUDED
#define TEXTUREMAP_H_INCLUDED 1


#include	<vector>
#include	<list>
#include	<valarray>

#include	<aqsis/aqsis.h>

#include	"tiffio.h"

#include	<aqsis/util/sstring.h>
#include	<aqsis/math/color.h>
#include	<aqsis/math/matrix.h>
#include	"itexturemap_old.h"
#include	<aqsis/ri/ri.h>
#include	<aqsis/shadervm/ishaderexecenv.h>
#include	<aqsis/math/lowdiscrep.h>

namespace Aqsis {

struct IqShader;
struct IqShaderData;

#define	ZFILE_HEADER		"Aqsis ZFile" AQSIS_VERSION_STR
#define	LATLONG_HEADER		"LatLong Environment"
#define	CUBEENVMAP_HEADER	"CubeFace Environment"
#define	SHADOWMAP_HEADER	"Shadow"
#define	MIPMAP_HEADER		"Aqsis MIP MAP"


enum EqBufferType
{
    BufferType_RGBA = 0,
    BufferType_Float,
    BufferType_Int16,
};


//----------------------------------------------------------------------
/** \class CqTextureMapBuffer
 *  \brief A container for a segment of a texture map. 
 *  
 *  Where a texture map is stored as a tiled image, i.e. when it has been processed 
 *  using teqser to produce a mip-mapped image, each tile will be loaded on demand
 *  into an object of this base type.
 *
 *  The base class represents data as 8bit per channel integer colour data,
 *  the base is specialised to provide buffer storage for different colour
 *  formats.
 *
 *  The buffer does not actually store the data, but instead contains a reference
 *  into the global texture cache area, a central memory area for all texture
 *  data.
 *
 *  Texture buffer segments are managed by the cache manager, and will be released
 *  if the cache usage exceeds a predefined amount. This behaviour can be controlled
 *  by marking the buffer as protected using the fProt argument to Init, or the
 *  SetfProtected function.
 */

class CqTextureMapBuffer
{
	public:
		/** \brief Default constructor.
		 *
		 *  Initialises the buffer to represent and empty, unprotected segment.
		 */
		CqTextureMapBuffer() :
				m_pBufferData( 0 ),
				m_sOrigin( 0 ),
				m_tOrigin( 0 ),
				m_Width( 0 ),
				m_Height( 0 ),
				m_Samples( 0 ),
				m_Directory( 0 ),
				m_fProtected( false )
		{}
		/** \brief Destructor.
		 *
		 *  Automatically releases the buffer data in the cache.
		 */
		virtual	~CqTextureMapBuffer()
		{
			Release();
		}

		/** Initialise the buffer reference to the specified format.
		 * \param xorigin Integer origin within the texture map.
		 * \param yorigin Integer origin within the texture map.
		 * \param width Width of the buffer segment.
		 * \param height Height of the buffer segment.
		 * \param samples Number of samples per pixel.
		 * \param directory The directory within the TIFF image map, used for multi image formats, i.e. cubeface environment map.
		 * \param fProt A flag indicating the buffer should be protected from removal by the cache management system.
		 */
		void	Init( TqUlong xorigin, TqUlong yorigin, TqUlong width, TqUlong height, TqInt samples, TqInt directory = 0, bool fProt = false )
		{
			Release();
			m_sOrigin = xorigin;
			m_tOrigin = yorigin;
			m_Width = width;
			m_Height = height;
			m_Samples = samples;
			m_Directory = directory;
			m_fProtected = fProt;

			m_pBufferData = AllocSegment( width, height, samples, m_fProtected );
		}
		/** Release the area of the cache memory referenced by this buffer object..
		 */
		void	Release()
		{
			if ( m_pBufferData != 0 )
				FreeSegment( m_pBufferData, m_Width, m_Height, m_Samples );
			m_pBufferData = 0;
		}

		/** Determine if the specified sample point and directory index is within this buffer segment.
		 * \param s Horizontal sample position.
		 * \param t Vertical sample position.
		 * \param directory TIFF directory index.
		 * \return Boolean indicating sample is within this buffer.
		 */
		bool	IsValid( TqUlong s, TqUlong t, TqInt directory = 0 )
		{

			return (
			           (s >= m_sOrigin) &&
			           (t >= m_tOrigin) &&
			           (s < m_sOrigin + m_Width) &&
			           (t < m_tOrigin + m_Height) &&
			           directory == m_Directory );
		}

		/** Get a pointer to the data for this buffer segment.
		 */
		TqPuchar	pBufferData()
		{
			return ( m_pBufferData );
		}
		/** Get a pointer to the data for this buffer segment.
		 */
		void*	pVoidBufferData()
		{
			return ( m_pBufferData );
		}
		/** Get the size of a single element
		 */
		virtual TqInt	ElemSize()
		{
			return( m_Samples * sizeof(TqUchar) );
		}
		/** Get the type of the data in the buffer
		 */
		virtual EqBufferType	BufferType()
		{
			return( BufferType_RGBA );
		}

		/** Get the float value at the specified pixel/element (0.0 --> 1.0)
		 *  \param x Horizontal sample position, 0.0 --> 1.0
		 *  \param y Vertical sample position, 0.0 --> 1.0
		 *  \param sample Index of the element to read from the sample pixel.
		 */
		virtual TqFloat	GetValue(TqInt x, TqInt y, TqInt sample)
		{
			TqInt iv = y * ( m_Width * ElemSize() );
			TqInt iu = x * ElemSize();
			return ( m_pBufferData[ iv + iu + sample ] / 255.0f );
		}
		/** Set the float value at the specified pixel/element (0.0 --> 1.0)
		 *  \param x Horizontal sample position, 0.0 --> 1.0
		 *  \param y Vertical sample position, 0.0 --> 1.0
		 *  \param sample Index of the element to modify within the chosen pixel.
		 *  \param value The new value to assign to the element.
		 */
		virtual void	SetValue(TqInt x, TqInt y, TqInt sample, TqFloat value)
		{
			TqInt iv = y * ( m_Width * ElemSize() );
			TqInt iu = x * ElemSize();
			m_pBufferData[ iv + iu + sample ] = static_cast<TqUchar>( value * 255.0f );
		}

		/** Get the min, max value of one tile/buffer
		 *  \note Use only with shadowmap
		 */
		virtual void MinMax(TqFloat &minz, TqFloat &maxz, TqInt sample) {}

		/** Get the origin of this buffer segment.
		 *  \return The horizontal origin of this segment within the overall image space.
		 */
		TqUlong sOrigin() const
		{
			return ( m_sOrigin );
		}
		/** Get the origin of this buffer segment.
		 *  \return The vertical origin of this segment within the overall image space.
		 */
		TqUlong tOrigin() const
		{
			return ( m_tOrigin );
		}
		/** Get the width of this buffer segment.
		 *  \return The width of this segment within the overall image space.
		 */
		TqUlong Width() const
		{
			return ( m_Width );
		}
		/** Get the height of this buffer segment.
		 *  \return The height of this segment within the overall image space.
		 */
		TqUlong Height() const
		{
			return ( m_Height );
		}
		/** Get the directory index of this buffer segment.
		 *  \return The directory index within a multi image texture map that this segment comes from.
		 */
		TqInt	Directory() const
		{
			return ( m_Directory );
		}
		/** Get the number of samples per element.
		 *  \return The number of samples that each pixel contains.
		 */
		TqInt	Samples() const
		{
			return ( m_Samples );
		}
		/** Get the status of the protected flag
		 *  \return true if the data within the texture cache for this buffer is protected from being flushed.
		 */
		bool	fProtected() const
		{
			return( m_fProtected );
		}
		/** Set this buffer as protected or not.
		 *  \param fProt Set to true to protect the data in the cache for this buffer from being flushed during
		 *               routine cache management.
		 */
		void	SetfProtected( bool fProt = true )
		{
			m_fProtected = fProt;
		}


		TqPuchar AllocSegment( TqUlong width, TqUlong height, TqInt samples, bool fProt = false );
		void	FreeSegment( TqPuchar pBufferData, TqUlong width, TqUlong height, TqInt samples );

	protected:
		TqPuchar	m_pBufferData;	///< Pointer to the image data.
		TqUlong	m_sOrigin;		///< Horizontal segment origin.
		TqUlong	m_tOrigin;		///< Vertical segment origin.
		TqUlong	m_Width;		///< Width of segment.
		TqUlong	m_Height;		///< Height of segment.
		TqInt	m_Samples;		///< Number of samples per pixel.
		TqInt	m_Directory;	///< TIFF directory index. Used for multi image textures, i.e. cubeface environment.
		bool	m_fProtected;	///< Flag indicating if this buffer is protected from being automatically delted by the cache.

}
;


//----------------------------------------------------------------------
/** \class CqFloatTextureMapBuffer
 * Class referencing a buffer in the image map cache in floating point format. 
 */

class CqFloatTextureMapBuffer : public CqTextureMapBuffer
{
	public:
		CqFloatTextureMapBuffer() : CqTextureMapBuffer()
		{}
		virtual	~CqFloatTextureMapBuffer()
		{}

		virtual TqFloat	GetValue(TqInt x, TqInt y, TqInt sample)
		{
			TqInt iv = y * ( m_Width * ElemSize() );
			TqInt iu = x * ElemSize();
			return ( (reinterpret_cast<TqFloat*>(&m_pBufferData[ iv + iu ]))[sample] );
		}
		virtual void	SetValue(TqInt x, TqInt y, TqInt sample, TqFloat value)
		{
			TqInt iv = y * ( m_Width * ElemSize() );
			TqInt iu = x * ElemSize();
			(reinterpret_cast<TqFloat*>(&m_pBufferData[ iv + iu ]))[sample] = value;
		}
		virtual TqInt	ElemSize()
		{
			return( m_Samples * sizeof(TqFloat) );
		}

		/** Get the type of the data in the buffer
		 */
		virtual EqBufferType	BufferType()
		{
			return( BufferType_Float );
		}
		
}
;


//----------------------------------------------------------------------
/** \class CqFloatTextureMapBuffer
 * Class referencing a buffer in the image map cache in floating point format. 
 */

class Cq16bitTextureMapBuffer : public CqTextureMapBuffer
{
	public:
		Cq16bitTextureMapBuffer() : CqTextureMapBuffer()
		{}
		virtual	~Cq16bitTextureMapBuffer()
		{}

		virtual TqFloat	GetValue(TqInt x, TqInt y, TqInt sample)
		{
			TqInt iv = y * ( m_Width * ElemSize() );
			TqInt iu = x * ElemSize();
			return ( (reinterpret_cast<TqUshort*>(&m_pBufferData[ iv + iu ]))[sample] / 65535.0f );
		}
		virtual void	SetValue(TqInt x, TqInt y, TqInt sample, TqFloat value)
		{
			TqInt iv = y * ( m_Width * ElemSize() );
			TqInt iu = x * ElemSize();
			(reinterpret_cast<TqUshort*>(&m_pBufferData[ iv + iu ]))[sample] = static_cast<TqUshort>( value * 65535.0f );
		}
		virtual TqInt	ElemSize()
		{
			return( m_Samples * sizeof(TqUshort) );
		}

		/** Get the type of the data in the buffer
		 */
		virtual EqBufferType	BufferType()
		{
			return( BufferType_Int16 );
		}

}
;

//----------------------------------------------------------------------
/** \class CqShadowMapBuffer
 * Class referencing a depth buffer in the image map cache. 
 */

class CqShadowMapBuffer : public CqTextureMapBuffer
{
	public:
		CqShadowMapBuffer() : CqTextureMapBuffer(), m_computed(false)
		{}
		virtual	~CqShadowMapBuffer()
		{}

		virtual TqFloat	GetValue(TqInt x, TqInt y, TqInt sample)
		{
			TqInt iv = y * ( m_Width * m_Samples );
			TqInt iu = x * m_Samples;
			return ( reinterpret_cast<TqFloat*>(m_pBufferData)[ iv + iu + sample] );
		}
		virtual void	SetValue(TqInt x, TqInt y, TqInt sample, TqFloat value)
		{
			TqInt iv = y * ( m_Width * m_Samples );
			TqInt iu = x * m_Samples;
			reinterpret_cast<TqFloat*>(m_pBufferData)[ iv + iu + sample] = value;
		}
		virtual TqInt	ElemSize()
		{
			return( m_Samples * sizeof(TqFloat) );
		}
		/** Get the type of the data in the buffer
		 */
		virtual EqBufferType	BufferType()
		{
			return( BufferType_Float );
		}
	
		
		/** Find the min and max z value for this tile; re-use the 
		 * previous computed values if necessary (m_computed)
		 */
		virtual void MinMax(TqFloat &minz, TqFloat &maxz, TqInt sample)
		{
			
			if (m_computed)
			{
				minz = m_minz;
				maxz = m_maxz;
			} else 
			{
				TqInt multiplier = m_Width * m_Samples;
				minz =  RI_FLOATMAX;
				maxz =  -RI_FLOATMAX;
				for (TqUint y = 0; y < m_Height; y++)
					for (TqUint x = 0; x < m_Width; x++)
					{
						TqInt iv = y * multiplier;
						TqInt iu = x * m_Samples;
						TqFloat val = reinterpret_cast<TqFloat*>(m_pBufferData)[ iv + iu + sample];
						
						minz = min(val, minz);
						maxz = max(val, maxz);
					}
				m_computed = true;
				m_minz = minz;
				m_maxz = maxz;
			}
			
		}
	private:
		TqFloat m_maxz;
		TqFloat m_minz;
		bool  m_computed;

}
;


//----------------------------------------------------------------------
/** \class CqTextureMapOld
 * Base class from which all texture maps are derived.
 */

class CqTextureMapOld : public IqTextureMapOld
{
	public:
		CqTextureMapOld( const CqString& strName ) :
				m_Compression( COMPRESSION_NONE ),
				m_Quality( 70 ),
				m_MinZ( RI_FLOATMAX ),
				m_XRes( 0 ),
				m_YRes( 0 ),
				m_PlanarConfig( PLANARCONFIG_CONTIG ),
				m_SamplesPerPixel( 3 ),
				m_Format( TexFormat_Plain ),
				m_strName( strName ),
				m_pImage( 0 ),
				m_IsValid( true ),
				m_smode( WrapMode_Black ),
				m_tmode( WrapMode_Black ),
				m_FilterFunc( RiBoxFilter ),
				m_lerp(-1.0),
				m_pixelvariance(0.001f),
				m_umapsize(0),
				m_vmapsize(0),
				m_interp(0.0),
				m_swidth( 1.0 ), m_twidth( 1.0 ),
				m_ds(-1.0),
				m_dt(-1.0),
				m_level(0)

		{
			m_pixel_variance.resize( m_SamplesPerPixel );
			m_pixel_sublevel.resize( m_SamplesPerPixel );
			m_accum_color.resize( m_SamplesPerPixel );
			m_hash = CqString::hash(strName.c_str());
		}
		virtual	~CqTextureMapOld();

		/** Get/Set the mininum depth this texture (for any surfaces using it)
		 */
		TqFloat	MinZ() const
		{
			return ( m_MinZ );
		}
		void	SetMinZ( TqFloat minz )
		{
			if ( minz <= m_MinZ )
				m_MinZ = minz;
		}

		/** Get the horizontal resolution of this image.
		 */
		virtual	TqUint	XRes() const
		{
			return ( m_XRes );
		}
		/** Get the vertical resolution of this image.
		 */
		virtual	TqUint	YRes() const
		{
			return ( m_YRes );
		}
		/** Get the number of samples per pixel.
		 */
		virtual	TqInt	SamplesPerPixel() const
		{
			return ( m_SamplesPerPixel );
		}
		/** Get the storage format of this image.
		 */
		virtual	EqTexFormat	Format() const
		{
			return ( m_Format );
		}

		virtual	TqInt Compression() const
		{
			return ( m_Compression );
		}

		virtual	void SetCompression( TqInt Compression )
		{
			m_Compression = Compression;
		}

		virtual	TqInt Quality() const
		{
			return ( m_Quality );
		}

		virtual	void SetQuality( TqInt Quality )
		{
			m_Quality = Quality;
		}

		virtual CqMatrix& GetMatrix( TqInt which, TqInt index = 0 )
		{
			return ( m_matWorldToScreen );
		}
		virtual const CqString& getName() const
		{
			return ( m_strName );
		}

		/** Get the image type.
		 */
		virtual	EqMapType	Type() const
		{
			return ( IsValid() ? MapType_Texture : MapType_Invalid );
		}
		/** Open this image ready for reading.
		                         */
		virtual	void	Open();
		/** Close this image file.
		 */
		virtual	void	Close();

		/** Determine if this image file is valid, i.e. has been found and opened successfully.
		 */
		bool	IsValid() const
		{
			return ( m_IsValid );
		}
		/** Set the flag indicating that this image has not been successfully opened.
		 */
		void	SetInvalid()
		{
			m_IsValid = false;
		}

		virtual	CqTextureMapBuffer*	GetBuffer( TqUlong s, TqUlong t, TqInt directory = 0, bool fProt = false );
		bool	CreateMIPMAP( bool fProtectBuffers = false );
		virtual	CqTextureMapBuffer* CreateBuffer( TqUlong xorigin, TqUlong yorigin, TqUlong width, TqUlong height, TqInt directory = 0, bool fProt = false )
		{
			CqTextureMapBuffer* pRes;
			switch( m_SampleFormat )
			{
					case SAMPLEFORMAT_IEEEFP:
					pRes = new CqFloatTextureMapBuffer();
					break;

					case SAMPLEFORMAT_UINT:
					default:
					{
						switch( m_BitsPerSample )
						{
								case 16:
								pRes = new Cq16bitTextureMapBuffer();
								break;
								case 8:
								default:
								pRes = new CqTextureMapBuffer();
								break;
						}
					}
					break;
			}
			pRes->Init( xorigin, yorigin, width, height, m_SamplesPerPixel, directory, fProt );
			return( pRes );
		}

		virtual void	PrepareSampleOptions(std::map<std::string, IqShaderData*>& paramMap );

		virtual	void	SampleMap( TqFloat s1, TqFloat t1, TqFloat swidth, TqFloat twidth, std::valarray<TqFloat>& val);
		virtual	void	SampleMap( TqFloat s1, TqFloat t1, TqFloat s2, TqFloat t2, TqFloat s3, TqFloat t3, TqFloat s4, TqFloat t4,
		                        std::valarray<TqFloat>& val);
		virtual	void	SampleMap( CqVector3D& R, CqVector3D& swidth, CqVector3D& twidth,
		                        std::valarray<TqFloat>& val, TqInt index = 0, TqFloat* average_depth = NULL, TqFloat* shadow_depth = NULL)
	{}
		virtual	void	SampleMap( CqVector3D& R1, CqVector3D& R2, CqVector3D& R3, CqVector3D& R4,
		                        std::valarray<TqFloat>& val, TqInt index = 0, TqFloat* average_depth = NULL, TqFloat* shadow_depth = NULL)
		{}

		virtual	void	GetSample( TqFloat ss1, TqFloat tt1, TqFloat ss2, TqFloat tt2, std::valarray<TqFloat>& val);
		virtual	void	GetSampleWithBlur( TqFloat ss1, TqFloat tt1, TqFloat ss2, TqFloat tt2, std::valarray<TqFloat>& val );
		virtual	void	GetSampleWithoutBlur( TqFloat ss1, TqFloat tt1, TqFloat ss2, TqFloat tt2, std::valarray<TqFloat>& val);


		virtual	TqInt	NumPages() const
		{
			return(1);
		}

		static	IqTextureMapOld* GetTextureMap( const CqString& strName );
		static	IqTextureMapOld* GetEnvironmentMap( const CqString& strName );
		static	IqTextureMapOld* GetShadowMap( const CqString& strName );
		static	IqTextureMapOld* GetLatLongMap( const CqString& strName );

		//void	ImageFilterVal( CqTextureMapBuffer* pData, TqInt x, TqInt y, TqInt directory, TqInt m_xres, TqInt m_yres, std::vector<TqFloat>& accum );

		void Interpreted( TqPchar mode );

		/** Clear the cache of texture maps.
		 */
		static void FlushCache();

		void CriticalMeasure();

		static void WriteTileImage( TIFF* ptex, CqTextureMapBuffer* pBuffer, TqUlong twidth, TqUlong theight, TqInt compression, TqInt quality );
		static void WriteTileImage( TIFF* ptex, TqFloat *raster, TqUlong width, TqUlong length, TqUlong twidth, TqUlong tlength, TqInt samples, TqInt compression, TqInt quality );
		static void WriteTileImage( TIFF* ptex, TqPuchar raster, TqUlong width, TqUlong length, TqUlong twidth, TqUlong tlength, TqInt samples, TqInt compression, TqInt quality );
		static void WriteTileImage( TIFF* ptex, TqUshort* raster, TqUlong width, TqUlong length, TqUlong twidth, TqUlong tlength, TqInt samples, TqInt compression, TqInt quality );

		static void WriteImage( TIFF* ptex, CqTextureMapBuffer* pBuffer, TqInt compression, TqInt quality );
		static void WriteImage( TIFF* ptex, TqFloat *raster, TqUlong width, TqUlong length, TqInt samples, TqInt compression, TqInt quality );
		static void WriteImage( TIFF* ptex, TqPuchar raster, TqUlong width, TqUlong length, TqInt samples, TqInt compression, TqInt quality );
		static void WriteImage( TIFF* ptex, TqUshort* raster, TqUlong width, TqUlong length, TqInt samples, TqInt compression, TqInt quality );

		TIFF*	pImage()
		{
			return( m_pImage );
		}
		const TIFF*	pImage() const
		{
			return( m_pImage );
		}

		bool BiLinear (TqFloat u, TqFloat v, TqInt umapsize, TqInt vmapsize, TqInt id, std::valarray<TqFloat > &m_color);
		void   CalculateLevel(TqFloat ds, TqFloat dt);

	protected:
		static std::vector<CqTextureMapOld*> m_TextureMap_Cache;	///< Static array of loaded textures.
		static std::vector<CqString*>	m_ConvertString_Cache; ///< Static array of filename (after conversion)

		TqInt m_Compression;            ///< TIFF Compression model
		TqInt m_Quality;                ///< If Jpeg compression is used than its overall quality

		TqFloat m_MinZ;                 ///< Minimum Depth
		TqUint	m_XRes;			///< Horizontal resolution.
		TqUint	m_YRes;			///< Vertical resolution.
		TqInt	m_PlanarConfig;		///< TIFF planar configuration type.
		TqInt	m_SamplesPerPixel;	///< Number of samples per pixel.
		TqInt	m_SampleFormat;		///< Format of the sample elements, i.e. RGBA, or IEEE
		TqInt	m_BitsPerSample;	///< Number of bits per sample element, 8 or 16.

		EqTexFormat	m_Format;	///< Image storage format type.

		CqString	m_strName;	///< Name of the image.
		TIFF*	m_pImage;		///< Pointer to an opened TIFF image.
		bool	m_IsValid;		///< Indicate whether this image has been successfully opened.
		enum EqWrapMode m_smode;        ///< Periodic, black, clamp
		enum EqWrapMode m_tmode;        ///< Periodic, black, clamp
		RtFilterFunc m_FilterFunc;      ///< Catmull-Rom, sinc, disk, ... pixelfilter
		std::list<CqTextureMapBuffer*> m_apFlat;///< Array of segments but for non mipmaps files  
		std::list<CqTextureMapBuffer*> m_apMipMaps[256];///< Arrays of segments per pages/directories
		CqTextureMapBuffer *m_apLast[256];	///< vector of last segments per pages/directories

		CqMatrix	m_matWorldToScreen;	///< Matrix to convert points from world space to screen space.

		TqFloat		m_sblur;
		TqFloat		m_tblur;
		TqFloat		m_pswidth;
		TqFloat		m_ptwidth;          
		TqFloat		m_samples;	///< How many samplings
		TqFloat     	m_lerp; 	///< Enable TriLinear
		TqFloat		m_pixelvariance;      ///< Smallest Difference between two distinct samples
		TqInt		m_umapsize;           ///< Umapsize for m_level of mipmap
		TqInt		m_vmapsize;           ///< Vmapsize for m_level of mipmap
		TqFloat		m_interp;             ///< Difference between m_level and m_level+1 MipMap (for TriLinear sampling)
		TqFloat 	m_swidth, m_twidth;   ///< for the pixel's filter
		TqFloat		m_ds;                 ///< delta (u2-u1) 
		TqFloat		m_dt;                 ///< delta (v2-v1)
		TqInt		m_level;              ///< Which level of mipmap (from m_ds, m_dt)

		// Temporary values used during BiLinear and/or TriLinear sampling.
		std::valarray<TqFloat>	m_pixel_variance;
		std::valarray<TqFloat>	m_pixel_sublevel;
		std::valarray<TqFloat>	m_accum_color;

		// To correlate all the microgrids together; keep track
		// of the last (non-zero) directory (typically level) so later we could
		// re-use the same level of mipmap in GetSampleArea(), GetSampleSgle().
		TqInt   m_Directory;
		TqUlong m_hash;
}
;


//----------------------------------------------------------------------
/** \brief Class to handle downsampling of images by a factor of 2 for mipmap
 * generation.
 */
class CqImageDownsampler
{
	public:
		/** \brief Construct an image downsampler 
		 *
		 * \param sWidth  filter width in s direction
		 * \param tWidth  filter width in t direction
		 * \param filterFunc  function returning filter coefficients
		 * \param sWrapMode  texture wrapping mode in s direction
		 * \param tWrapMode  texture wrapping mode in t direction
		 */
		CqImageDownsampler(TqFloat sWidth, TqFloat tWidth, RtFilterFunc filterFunc, EqWrapMode sWrapMode, EqWrapMode tWrapMode);

		/** \brief Downsample an image by a factor of two.
		 */
		CqTextureMapBuffer* downsample(CqTextureMapBuffer* inBuf, CqTextureMapOld& texMap, TqInt directory, bool protectBuffer);
	private:
		/** \brief Compute and cache filter kernel values for the given filter function.
		 */
		void computeFilterKernel(TqFloat sWidth, TqFloat tWidth, RtFilterFunc filterFunc, bool evenFilterS, bool evenFilterT);
		/** \brief Wrap a position at the image edges according to current wrapmode
		 */
		inline TqInt edgeWrap(TqInt pos, TqInt posMax, EqWrapMode mode);

		// Filter parameters for the convolution kernel.
		TqInt m_sNumPts;
		TqInt m_tNumPts;
		TqInt m_sStartOffset;
		TqInt m_tStartOffset;
		std::vector<TqFloat> m_weights;
		// Other members
		TqFloat m_sWidth;
		TqFloat m_tWidth;
		RtFilterFunc m_filterFunc;
		EqWrapMode m_sWrapMode;
		EqWrapMode m_tWrapMode;
};


//----------------------------------------------------------------------
/** \class CqEnvironmentMapOld
 * Environment map, derives from texture map and handles converting reflection
 * vector to s,t coordinates.
 */

class CqEnvironmentMapOld : public CqTextureMapOld
{
	public:
		CqEnvironmentMapOld( const CqString& strName ) :
				CqTextureMapOld( strName )
		{}
		virtual	~CqEnvironmentMapOld()
		{}

		virtual	EqMapType	Type() const
		{
			return ( IsValid() ? MapType_Environment : MapType_Invalid );
		}

		virtual	void	SampleMap( CqVector3D& R, CqVector3D& swidth, CqVector3D& twidth,
		                        std::valarray<TqFloat>& val, TqInt index = 0, TqFloat* average_depth = NULL, TqFloat* shadow_depth = NULL);
		virtual	void	SampleMap( CqVector3D& R1, CqVector3D& R2, CqVector3D& R3, CqVector3D& R4,
		                        std::valarray<TqFloat>& val, TqInt index = 0, TqFloat* average_depth = NULL, TqFloat* shadow_depth = NULL);

		virtual CqMatrix& GetMatrix( TqInt which, TqInt index = 0 )
		{

			return ( m_matWorldToScreen );
		}
		virtual void SetFov(TqFloat f)
		{		
			m_fov = f;
		}


	private:
		void	Getst( CqVector3D& R, TqUlong fullwidth, TqUlong fulllength, TqFloat& s, TqFloat& t );
		CqMatrix	m_matWorldToScreen;		///< Matrix to convert points from world space to screen space.
		TqFloat m_fov;                  ///< cotangent()

}
;

//----------------------------------------------------------------------
/** \class CqLatLongMapOld
 * Environment map, derives from texture map and handles converting reflection
 * vector to s,t coordinates.
 */

class CqLatLongMapOld : public CqEnvironmentMapOld
{
	public:
		CqLatLongMapOld( const CqString& strName ) :
				CqEnvironmentMapOld( strName )
		{}
		virtual	~CqLatLongMapOld()
		{}

		virtual	EqMapType	Type() const
		{
			return ( IsValid() ? MapType_LatLong : MapType_Invalid );
		}


};


//----------------------------------------------------------------------
/** \class CqShadowMapOld
 * Shadow map, derives from texture map.
 */

class CqShadowMapOld : public CqTextureMapOld
{
	public:
		CqShadowMapOld( const CqString& strName );
		virtual	~CqShadowMapOld()
		{}

		virtual	EqMapType	Type() const
		{
			return ( IsValid() ? MapType_Shadow : MapType_Invalid );
		}

		/** Get the matrix used to convert points from work into camera space.
		 */
		virtual CqMatrix&	matWorldToCamera(TqInt index = 0)
		{
			assert( index < static_cast<TqInt>(m_WorldToCameraMatrices.size()) );
			return ( m_WorldToCameraMatrices[index] );
		}
		/** Get the matrix used to convert points from work into screen space.
		 */
		virtual CqMatrix&	matWorldToScreen(TqInt index = 0)
		{
			assert( index < static_cast<TqInt>(m_WorldToScreenMatrices.size()) );
			return ( m_WorldToScreenMatrices[index] );
		}



		void	AllocateMap( TqInt XRes, TqInt YRes );
		TqFloat	Sample( const CqVector3D&	vecPoint );
		void	SaveZFile();
		void	LoadZFile();
		void	SaveShadowMapOld( const CqString& strShadowName, bool append = false );
		void	ReadMatrices();
		TqInt   PseudoMipMaps( TqUlong s, TqInt index );

		virtual	CqTextureMapBuffer* CreateBuffer( TqUlong xorigin, TqUlong yorigin, TqUlong width, TqUlong height, TqInt directory = 0, bool fProt = false )
		{
			CqTextureMapBuffer* pRes = new CqShadowMapBuffer();
			pRes->Init( xorigin, yorigin, width, height, m_SamplesPerPixel, directory, fProt );
			return( pRes );
		}

		virtual void	PrepareSampleOptions(std::map<std::string, IqShaderData*>& paramMap );

		virtual	void	SampleMap( CqVector3D& R, CqVector3D& swidth, CqVector3D& twidth, std::valarray<TqFloat>& val, TqInt index = 0, TqFloat* average_depth = NULL, TqFloat* shadow_depth = NULL);
		virtual	void	SampleMap( CqVector3D& R1, CqVector3D& R2, CqVector3D& R3, CqVector3D& R4, std::valarray<TqFloat>& val, TqInt index = 0, TqFloat* average_depth = NULL, TqFloat* shadow_depth = NULL);
		virtual TqDouble MinZ( TqInt index = 0 )
                {
 			if (m_MinZ.size() > 0)
				return m_MinZ[index];
			else
				return RI_FLOATMAX;

                }
		virtual CqMatrix& GetMatrix( TqInt which, TqInt index = 0 )
		{
			if ( which == 0 )
				return matWorldToCamera(index);
			else if ( which == 1 )
				return matWorldToScreen(index);
			else if ( which == 2 )
				return m_ITTCameraToLightMatrices[index];
			return ( matWorldToCamera(index) );
		}
		virtual	TqInt	NumPages() const
		{
			return(m_NumberOfMaps);
		}


	private:
		static	CqLowDiscrepancy	m_LowDiscrep;			///< Low discrepancy point generator.

		TqFloat m_minBias;
		TqFloat m_biasRange;

		std::vector<CqMatrix>	m_WorldToCameraMatrices;		///< Matrix to convert points from world space to light space.
		std::vector<CqMatrix>	m_WorldToScreenMatrices;		///< Matrix to convert points from world space to screen space.
		std::vector<CqMatrix>	m_ITTCameraToLightMatrices;
		std::vector<TqDouble>	m_MinZ;	///< The minimum Z value for a shadow map
		TqInt			m_NumberOfMaps; ///< Number of occlusion maps; regular shadowmap m_NumberOfMaps == 1
		CqVector2D		m_LastPoint;	///< The last iu,iv coords; to minimize the computation over the same point
		TqFloat			m_Val;		///< Its value at the last iu,iv
		TqFloat			m_Depth; 	///< Its depth at the last iu,iv
		TqFloat			m_Average; 	///< Its average z at the last iu,iv
}
;


//-----------------------------------------------------------------------

} // namespace Aqsis

#endif	// !TEXTUREMAP_H_INCLUDED

