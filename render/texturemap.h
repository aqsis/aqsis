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
		\brief Declares texture map handling and cacheing classes.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef TEXTUREMAP_H_INCLUDED
#define TEXTUREMAP_H_INCLUDED 1

#undef		min
#undef		max

#include	<vector>
#include	<valarray>

#include	"tiffio.h"
//#include	"tiffiop.h"

#include	"sstring.h"
#include	"color.h"
#include	"lights.h"

#include	"specific.h"	// Needed for namespace macros.

#define		_qShareName	CORE
#include	"share.h"

START_NAMESPACE(Aqsis)

#define	ZFILE_HEADER		"Aqsis ZFile" VERSION_STR
#define	CUBEENVMAP_HEADER	"Aqsis CubeFace Environment"
#define	SHADOWMAP_HEADER	"Aqsis Shadow Map"
#define	SATMAP_HEADER		"Aqsis SAT Map"

void WriteImage(TIFF* ptex, float *raster, unsigned long width, unsigned long length, int samples);
void WriteTileImage(TIFF* ptex, float *raster, unsigned long width, unsigned long length, unsigned long twidth, unsigned long tlength, int samples);

//----------------------------------------------------------------------
/** \enum EqMapType
 * Enum defining the various image map types.
 */

enum	EqMapType
{
	MapType_Invalid=0,

	MapType_Texture=1,		///< Plain texture map.
	MapType_Environment,	///< Cube face environment map.
	MapType_Bump,			///< Bump map (not used).
	MapType_Shadow,			///< Shadow map.
};


/** \enum EqWrapMode
 * Enum defining the various modes of handling texture access outside of the normal range.
 */
enum	EqWrapMode
{
	WrapMode_Black=0,		///< Return black.
	WrapMode_Periodic,		///< Wrap round to the opposite side.
	WrapMode_Clamp,			///< Clamp to in range.
};


/** \enum EqTexFormat
 * Enum defining the possible storage types for image maps.
 */
enum EqTexFormat
{
	TexFormat_Plain=0,		///< Plain TIFF image.
	TexFormat_SAT=1,		///< Aqsis Summed Area Table format.
};


//----------------------------------------------------------------------
/** \class CqTextureMapBuffer
 * Class referencing a buffer in the image map cache. 
 */

class _qShareC CqTextureMapBuffer
{
	public:
	_qShareM		CqTextureMapBuffer() : m_sOrigin(0), m_tOrigin(0), m_Width(0), m_Height(0), m_Samples(0), m_pBufferData(0), m_Directory(0)	{}
					/** Constructor taking buffer information.
					 * \param xorigin Integer origin within the texture map.
					 * \param yorigin Integer origin within the texture map.
					 * \param width Width of the buffer segment.
					 * \param height Height of the buffer segment.
					 * \param samples Number of samples per pixel.
					 * \param directory The directory within the TIFF image map, used for multi image formats, i.e. cubeface environment map.
					 */
	_qShareM		CqTextureMapBuffer(unsigned long xorigin, unsigned long yorigin, unsigned long width, unsigned long height, int samples, int directory=0) : 
						m_sOrigin(xorigin), m_tOrigin(yorigin), m_Width(width), m_Height(height), m_Samples(samples), m_pBufferData(0), m_Directory(directory)
					{
						m_pBufferData=AllocSegment(width,height,samples);
					}
	_qShareM		~CqTextureMapBuffer()
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
					 */
	_qShareM		void	Init(unsigned long xorigin, unsigned long yorigin, unsigned long width, unsigned long height, int samples, int directory=0)
					{
						Release();
						m_sOrigin=xorigin;
						m_tOrigin=yorigin; 
						m_Width=width;
						m_Height=height; 
						m_Samples=samples;
						m_Directory=directory;
						m_pBufferData=AllocSegment(width,height,samples);
					}
					/** Release this reference to the cache.
					 */
	_qShareM		void	Release()
					{
						if(m_pBufferData!=0)	FreeSegment(m_pBufferData,m_Width,m_Height,m_Samples);
						m_pBufferData=0;
					}

					/** Determine if the specified sample point and directory index is within this buffer segment.
					 * \param s Horizontal sample position.
					 * \param t Vertical sample position.
					 * \param directory TIFF directory index.
					 * \return Boolean indicating sample is within this buffer.
					 */
	_qShareM		TqBool	IsValid(unsigned long s, unsigned long t, int directory=0)
					{
						return(s>=m_sOrigin && t>=m_tOrigin && s<m_sOrigin+m_Width && t<m_tOrigin+m_Height && directory==m_Directory);
					}

					/** Get a pointer to the data for this buffer segment.
					 */
	_qShareM		float*	pBufferData()			{return(m_pBufferData);}
					/** Get the origin of this buffer segment.
					 */
	_qShareM		unsigned long sOrigin() const	{return(m_sOrigin);}
					/** Get the origin of this buffer segment.
					 */
	_qShareM		unsigned long tOrigin() const	{return(m_tOrigin);}
					/** Get the width of this buffer segment.
					 */
	_qShareM		unsigned long Width() const		{return(m_Width);}
					/** Get the height of this buffer segment.
					 */
	_qShareM		unsigned long Height() const	{return(m_Height);}
					/** Get the directory index of this buffer segment.
					 */
	_qShareM		int			  Directory() const	{return(m_Directory);}

	
	_qShareM	static 	float* AllocSegment(unsigned long width, unsigned long height, int samples);
	_qShareM	static 	void	FreeSegment(float* pBufferData, unsigned long width, unsigned long height, int samples);

	private:
			float*			m_pBufferData;	///< Pointer to the image data.
			unsigned long	m_sOrigin;		///< Horizontal segment origin.
			unsigned long	m_tOrigin;		///< Vertical segment origin.
			unsigned long	m_Width;		///< Width of segment.
			unsigned long	m_Height;		///< Height of segment.
			int				m_Samples;		///< Number of samples per pixel.
			int				m_Directory;	///< TIFF directory index. Used for multi image textures, i.e. cubeface environment.
};

//----------------------------------------------------------------------
/** \class CqTextureMap
 * Base class from which all texture maps are derived.
 */

class _qShareC CqTextureMap
{
	public:
	_qShareM 		CqTextureMap(const char* strName)	:
									m_strName(strName),
									m_pImage(0),
									m_XRes(0),
									m_YRes(0),
									m_PlanarConfig(PLANARCONFIG_CONTIG),
									m_SamplesPerPixel(3),
									m_Format(TexFormat_Plain),
									m_IsValid(TqTrue)
									{}
	_qShareM	virtual	~CqTextureMap();

										/** Get the horizontal resolution of this image.
										 */
	_qShareM			unsigned long	XRes() const		{return(m_XRes);}
										/** Get the vertical resolution of this image.
										 */
	_qShareM			unsigned long	YRes() const		{return(m_YRes);}
										/** Get the number of samples per pixel.
										 */
	_qShareM			int				SamplesPerPixel() const	{return(m_SamplesPerPixel);}
										/** Get the storage format of this image.
										 */
	_qShareM			EqTexFormat		Format() const		{return(m_Format);}

									/** Get the image type.
									 */
	_qShareM	virtual	EqMapType	Type() const			{return(IsValid()?MapType_Texture:MapType_Invalid);}
									/** Open this image ready for reading.
									 */
	_qShareM	virtual	void		Open();
									/** Close this image file.
									 */
	_qShareM	virtual	void		Close();

									/** Determine if this image file is valid, i.e. has been found and opened successfully.
									 */
	_qShareM			bool		IsValid() const			{return(m_IsValid);}
									/** Set the flag indicating that this image has not been successfully opened.
									 */
	_qShareM			void		SetInvalid()			{m_IsValid=TqFalse;}

									/** Get a pointer to the cache buffer segment which contains the specifed sample point.
									 * \param s Horizontal sample position.
									 * \param t Vertical sample position.
									 * \param directory TIFF directory index.
									 */
	_qShareM	virtual	CqTextureMapBuffer*	GetBuffer(unsigned long s, unsigned long t, int directory=0);
						void		CreateSATMap();

	_qShareM	virtual	void		SampleSATMap(float s1, float t1, float swidth, float twidth, float sblur, float tblur, 
												 std::valarray<float>& val, int directory=0);
	_qShareM	virtual	void		SampleSATMap(float s1, float t1, float s2, float t2, float s3, float t3, float s4, float t4, 
												 float sblur, float tblur, 
												 std::valarray<float>& val, int directory=0);
	_qShareM	virtual	void		SampleSATMap(CqVector3D& R, CqVector3D& swidth, CqVector3D& twidth, float sblur, float tblur, 
												 std::valarray<float>& val)	{}
	_qShareM	virtual	void		SampleSATMap(CqVector3D& R1, CqVector3D& R2, CqVector3D& R3, CqVector3D& R4, 
												 float sblur, float tblur, 
												 std::valarray<float>& val)	{}

	_qShareM	virtual	void		GetSample(long ss1, long tt1, long ss2, long tt2, std::valarray<float>& val, bool fss, bool ftt, int directory);

	_qShareM	static	CqTextureMap* GetTextureMap(const char* strName);
	_qShareM	static	CqTextureMap* GetEnvironmentMap(const char* strName);
	_qShareM	static	CqTextureMap* GetShadowMap(const char* strName);

									/** Clear the cache of texture maps.
									 */
	_qShareM	static	void		FlushCache()			{
																int i;
																for(i=0; i<m_TextureMap_Cache.size(); i++)
																	delete(m_TextureMap_Cache[i]);

																m_TextureMap_Cache.clear();
															}

	protected:
static	std::vector<CqTextureMap*>	m_TextureMap_Cache;	///< Static array of loaded textures.

				TqInt	m_XRes;					///< Horizontal resolution.
				TqInt	m_YRes;					///< Vertical resolution.
				TqInt	m_PlanarConfig;			///< TIFF planar configuration type.
				TqInt	m_SamplesPerPixel;		///< Number of samples per pixel.

				EqTexFormat	m_Format;			///< Image storage format type.

				CqString	m_strName;			///< Name of the image.
				TIFF*		m_pImage;			///< Pointer to an opened TIFF image.
				TqBool		m_IsValid;			///< Indicate whether this image has been successfully opened.

				std::vector<CqTextureMapBuffer*>	m_apSegments;	///< Array of cache segments related to this image.
};


//----------------------------------------------------------------------
/** \class CqEnvironmentMap
 * Environment map, derives from texture map and handles converting reflection
 * vector to s,t coordinates.
 */

class _qShareC CqEnvironmentMap : public CqTextureMap
{
	public:
	_qShareM 		CqEnvironmentMap(const char* strName)	:
									CqTextureMap(strName)
									{}
	_qShareM	virtual	~CqEnvironmentMap()	{}
	
	_qShareM	virtual	EqMapType	Type() const			{return(IsValid()?MapType_Environment:MapType_Invalid);}

	_qShareM	virtual	void		SampleSATMap(CqVector3D& R, CqVector3D& swidth, CqVector3D& twidth, float sblur, float tblur, 
												 std::valarray<float>& val);
	_qShareM	virtual	void		SampleSATMap(CqVector3D& R1, CqVector3D& R2, CqVector3D& R3, CqVector3D& R4, 
												 float sblur, float tblur, 
												 std::valarray<float>& val);

	private:
						void		Getst(CqVector3D& R, unsigned long fullwidth, unsigned long fulllength, float& s, float& t);
};


//----------------------------------------------------------------------
/** \class CqShadowMap
 * Shadow map, derives from texture map.
 */

class _qShareC CqShadowMap : public CqTextureMap, public CqImageBuffer
{
	public:
	_qShareM 		CqShadowMap(const char* strName)	:
									CqImageBuffer(),
									CqTextureMap(strName)
									{}
	_qShareM	virtual	~CqShadowMap()	{}
	
	_qShareM	virtual	EqMapType	Type() const			{return(IsValid()?MapType_Shadow:MapType_Invalid);}

							/** Get the matrix used to convert points from work into camera space.
							 */
	_qShareM	CqMatrix&	matWorldToCamera()		{return(m_matWorldToCamera);}
							/** Get the matrix used to convert points from work into screen space.
							 */
	_qShareM	CqMatrix&	matWorldToScreen()		{return(m_matWorldToScreen);}

	_qShareM	TqInt		XRes() const			{return(m_XRes);}
	_qShareM	TqInt		YRes() const			{return(m_YRes);}
		
	_qShareM	void		AllocateMap(TqInt XRes, TqInt YRes);
	_qShareM	TqFloat		Sample(const CqVector3D&	vecPoint);
	_qShareM	void		SaveZFile();
	_qShareM	void		LoadZFile();
	_qShareM	void		SaveShadowMap(const char* strShadowName);
	_qShareM	void		ReadMatrices();

	// Overrides from CqImageBuffer
	_qShareM	virtual	void	SetImage();
	_qShareM	virtual	void	BucketComplete(TqInt iBucket);
	_qShareM	virtual	void	ImageComplete();

	_qShareM	virtual	void	SampleMap(const CqVector3D& R, const CqVector3D& swidth, const CqVector3D& twidth, float sblur, float tblur, float& val);
	_qShareM	virtual	void	SampleMap(const CqVector3D& R1, const CqVector3D& R2,const CqVector3D& R3,const CqVector3D& R4, float sblur, float tblur, float& val);

	private:
static	TqInt		m_rand_index;			///< Static random number table index.
static	TqFloat		m_aRand_no[256];		///< Random no. table used for jittering the shadow sampling.

		CqMatrix	m_matWorldToCamera;		///< Matrix to convert points from world space to light space.
		CqMatrix	m_matWorldToScreen;		///< Matrix to convert points from world space to screen space.
};


//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif	// !TEXTUREMAP_H_INCLUDED
