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

#include	<math.h>
#include	<iostream>
#include	<fstream>

#include	"aqsis.h"
#include	"texturemap.h"
#include	"rifile.h"
#include	"exception.h"
#include	"renderer.h"

START_NAMESPACE(Aqsis)


static void get_face_intersection(CqVector3D *normal, CqVector3D *pt, int* face);
static void get_edge_intersection(CqVector3D* n1, CqVector3D* n2, int edge, CqVector3D* pt);
static void project(int face);

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


CqVector3D	cube[max_no];	// Stores the projection of the reflected beam onto the cube.
int		cube_no; 		// Stores the number of points making up the projection.
TqFloat	uv[max_no][2];	// Stores the values of this projection for a given face.

TqInt		CqShadowMap::m_rand_index=0;
TqFloat		CqShadowMap::m_aRand_no[256];

//---------------------------------------------------------------------
/** Static array of cached texture maps.
 */

std::vector<CqTextureMap*>	CqTextureMap::m_TextureMap_Cache;


//---------------------------------------------------------------------
/** Allocate a cache segment to hold the specified image tile.
 */

float* CqTextureMapBuffer::AllocSegment(unsigned long width, unsigned long height, int samples)
{
	// TODO: Implement proper global cache handling.
    QGetRenderContext()->Stats().IncTextureMemory( width * height * samples * sizeof(float));
	return(static_cast<float*>(calloc((width*height)*samples, sizeof(float))));
}

//---------------------------------------------------------------------
/** Allocate a cache segment to hold the specified image tile.
 */

unsigned char* CqTextureMapBuffer::AllocSegmentB(unsigned long width, unsigned long height, int samples)
{
	// TODO: Implement proper global cache handling.
    QGetRenderContext()->Stats().IncTextureMemory( width * height * samples);
	return(static_cast<unsigned char*>(calloc((width*height)*samples, 1)));
}


//---------------------------------------------------------------------
/** Static array of cached texture maps.
 */

void	CqTextureMapBuffer::FreeSegment(void* pBufferData, unsigned long width, unsigned long height, int samples)
{
	// TODO: Implement proper global cache handling.
	//QGetRenderContext()->Stats().IncTextureMemory( -width * height * samples * sizeof(float));
	free(pBufferData);
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqTextureMap::~CqTextureMap()
{
	// Search for it in the cache and remove the reference.
	std::vector<CqTextureMap*>::iterator i;
	for(i=m_TextureMap_Cache.begin(); i!=m_TextureMap_Cache.end(); i++)
	{
		if((*i)==this)
		{
			m_TextureMap_Cache.erase(i);
			break;
		}
	}

	// Delete any held cache buffer segments.
	std::vector<CqTextureMapBuffer*>::iterator s;
	for(s=m_apSegments.begin(); s!=m_apSegments.end(); s++)
		delete(*s);
}


//---------------------------------------------------------------------
/** Open a named texture map.
 */

void CqTextureMap::Open()
{
	char swrap[80], twrap[80], filterfunc[80];
	float swidth, twidth;

	m_IsValid=TqFalse;
	// Find the file required.
	CqRiFile	fileImage(m_strName.c_str(),"texture");
	if(!fileImage.IsValid())
	{
		CqString strErr("Cannot open texture file : ");
		strErr+=m_strName;
		CqBasicError(1,Severity_Fatal,strErr.c_str());
		return;
	}
	CqString strRealName(fileImage.strRealName());
	fileImage.Close();

	// Now open it as a tiff file.
	m_pImage=TIFFOpen(strRealName.c_str(),"r");
	if(m_pImage)
	{
		char*pFormat=0;
		char*pModes=0;

		TIFFGetField(m_pImage, TIFFTAG_IMAGEWIDTH, &m_XRes);
		TIFFGetField(m_pImage, TIFFTAG_IMAGELENGTH, &m_YRes);
		TIFFGetField(m_pImage, TIFFTAG_PLANARCONFIG, &m_PlanarConfig);
		TIFFGetField(m_pImage, TIFFTAG_SAMPLESPERPIXEL, &m_SamplesPerPixel);
		TIFFGetField(m_pImage,  TIFFTAG_PIXAR_TEXTUREFORMAT, &pFormat);
		TIFFGetField(m_pImage,  TIFFTAG_PIXAR_WRAPMODES, &pModes);

		if (pModes) 
		{
          	  sscanf(pModes, "%s %s %s %f %f", swrap, twrap, filterfunc, &swidth, &twidth);

		  /// smode
		  if (strcmp(swrap, RI_PERIODIC) == 0)
		  {
			  m_smode = WrapMode_Periodic;
		  } else if (strcmp(swrap, RI_BLACK) == 0)
		  {
			  m_smode = WrapMode_Black;
		  } else if (strcmp(swrap, RI_CLAMP) == 0)
		  {
			  m_smode = WrapMode_Clamp;
		  }

		  /// t mode
		  if (strcmp(twrap, RI_PERIODIC) == 0)
		  {
			  m_tmode = WrapMode_Periodic;
		  } else if (strcmp(twrap, RI_BLACK) == 0)
		  {
			  m_tmode = WrapMode_Black;
		  } else if (strcmp(twrap, RI_CLAMP) == 0)
		  {
			  m_tmode = WrapMode_Clamp;
		  }
		  /// Pixel's Filter
		  if (strcmp(filterfunc, "gaussian") == 0)
		  {
			  m_FilterFunc = RiGaussianFilter;
		  } else if (strcmp(filterfunc, "box") == 0)
		  {
			  m_FilterFunc = RiBoxFilter;
		  } else if (strcmp(filterfunc, "triangle") == 0)
		  {
			  m_FilterFunc = RiTriangleFilter;
		  } else if (strcmp(filterfunc, "catmull-rom") == 0)
		  {
			  m_FilterFunc = RiCatmullRomFilter;
		  } else if (strcmp(filterfunc, "sinc") == 0)
		  {
			  m_FilterFunc = RiSincFilter;
		  } else if (strcmp(filterfunc, "disk") == 0)
		  {
			  m_FilterFunc = RiDiskFilter;
		  } else if (strcmp(filterfunc, "bessel") == 0)
		  {
			  m_FilterFunc = RiBesselFilter;
		  }

		  /// Pixel's Filter x,y 
		  m_swrap = swidth;
		  m_twrap = twidth;
		}


		if(pFormat && strcmp(pFormat, SATMAP_HEADER)==0 ||
		   pFormat && strcmp(pFormat, CUBEENVMAP_HEADER)==0)
		{
			m_Format=TexFormat_SAT;
			m_IsValid=TqTrue;
		}
		else
		{
			m_Format=TexFormat_Plain;
			m_IsValid=TqTrue;
		}
	}
}


//---------------------------------------------------------------------
/** If properly opened, close the TIFF file.
 */

void CqTextureMap::Close()
{
	
	if(m_pImage!=0)	TIFFClose(m_pImage);
	m_pImage=0;
	
}


//----------------------------------------------------------------------
/** Check if a texture map exists in the cache, return a pointer to it if so, else
 * load it if possible..
 */

CqTextureMap* CqTextureMap::GetTextureMap(const char* strName)
{
	// First search the texture map cache
	for(std::vector<CqTextureMap*>::iterator i=m_TextureMap_Cache.begin(); i!=m_TextureMap_Cache.end(); i++)
	{
		if((*i)->m_strName==strName)
		{ 
			if((*i)->Type()==MapType_Texture)
				return(*i);
			else
				return(NULL);
		}
	}
	// If we got here, it doesn't exist yet, so we must create and load it.
	CqTextureMap* pNew=new CqTextureMap(strName);
	m_TextureMap_Cache.push_back(pNew);
	pNew->Open();

	// Ensure that it is in the correct format
	if(pNew->Format()!=TexFormat_SAT)
	{
		pNew->CreateSATMap();
		pNew->Close();
	}

	return(pNew);
}


//----------------------------------------------------------------------
/** Check if a texture map exists in the cache, return a pointer to it if so, else
 * load it if possible..
 */

CqTextureMap* CqTextureMap::GetEnvironmentMap(const char* strName)
{
	// First search the texture map cache
	for(std::vector<CqTextureMap*>::iterator i=m_TextureMap_Cache.begin(); i!=m_TextureMap_Cache.end(); i++)
	{
		if((*i)->m_strName==strName)
		{
			if((*i)->Type()==MapType_Environment)
				return(*i);
			else
				return(NULL);
		}
	}
	// If we got here, it doesn't exist yet, so we must create and load it.
	CqTextureMap* pNew=new CqEnvironmentMap(strName);
	m_TextureMap_Cache.push_back(pNew);
	pNew->Open();

	char* ptexfmt;
	if(pNew->m_pImage==0 ||
	   TIFFGetField(pNew->m_pImage, TIFFTAG_PIXAR_TEXTUREFORMAT, &ptexfmt)!=1 ||
	   strcmp(ptexfmt,CUBEENVMAP_HEADER)!=0)
	{
		CqString strError(strName);
		strError+=" not an environment map, use RiMakeCubeFaceEnvironment";
		CqBasicError(0,Severity_Normal,strError.c_str());
		pNew->SetInvalid();
	}
	return(pNew);
}


//----------------------------------------------------------------------
/** Check if a texture map exists in the cache, return a pointer to it if so, else
 * load it if possible..
 */

CqTextureMap* CqTextureMap::GetShadowMap(const char* strName)
{
	// First search the texture map cache
	for(std::vector<CqTextureMap*>::iterator i=m_TextureMap_Cache.begin(); i!=m_TextureMap_Cache.end(); i++)
	{
		if((*i)->m_strName==strName)
		{
			if((*i)->Type()==MapType_Shadow)
				return(*i);
			else
				return(NULL);
		}
	}
	// If we got here, it doesn't exist yet, so we must create and load it.
	CqShadowMap* pNew=new CqShadowMap(strName);
	m_TextureMap_Cache.push_back(pNew);
	pNew->Open();

	char* ptexfmt;
	if(pNew->m_pImage==0 ||
	   TIFFGetField(pNew->m_pImage, TIFFTAG_PIXAR_TEXTUREFORMAT, &ptexfmt)!=1 ||
	   strcmp(ptexfmt,SHADOWMAP_HEADER)!=0)
	{
		CqString strError(strName);
		strError+=" not a shadow map, use RiMakeShadow";
		CqBasicError(0,Severity_Normal,strError.c_str());
		pNew->SetInvalid();
	}
	else
		pNew->ReadMatrices();

	return(pNew);
}


//----------------------------------------------------------------------
/** Get the buffer segment from the cache which conmtains the requested s,t coordinates, read from the file if not
 * already cached.
 */

CqTextureMapBuffer* CqTextureMap::GetBuffer(unsigned long s, unsigned long t, int directory)
{
	// Search already cached segments first.
	for(std::vector<CqTextureMapBuffer*>::iterator i=m_apSegments.begin(); i!=m_apSegments.end(); i++)
		if((*i)->IsValid(s,t,directory))	return(*i);

	// If we got here, segment is not currently loaded, so load the correct segement and store it in the cache.
	CqTextureMapBuffer* pTMB=0;

	if(!m_pImage){
		CqRiFile	fileImage(m_strName.c_str(),"texture");
		if(!fileImage.IsValid())
		{
			CqString strErr("Cannot open texture file : ");
			strErr+=m_strName;
			CqBasicError(1,Severity_Fatal,strErr.c_str());
			return pTMB;
		}
		CqString strRealName(fileImage.strRealName());
		fileImage.Close();

		// Now open it as a tiff file.
		m_pImage=TIFFOpen(strRealName.c_str(),"r");
	}

	if(m_pImage)
	{
		uint32 tsx,tsy;
		int ret=TIFFGetField(m_pImage,TIFFTAG_TILEWIDTH,&tsx);
		TIFFGetField(m_pImage,TIFFTAG_TILELENGTH,&tsy);
		// If a tiled image, read the appropriate tile.
		if(ret)
		{
			// Work out the coordinates of this tile.
			unsigned long ox=(s/tsx)*tsx;
			unsigned long oy=(t/tsy)*tsy;
			
			if (Type() == MapType_Shadow)
				pTMB=new CqTextureMapBuffer(ox,oy,tsx,tsy,sizeof(float) * m_SamplesPerPixel,directory);
			else
				pTMB=new CqTextureMapBuffer(ox,oy,tsx,tsy,m_SamplesPerPixel,directory);

			TIFFSetDirectory(m_pImage, directory);
            
			unsigned char* pData=pTMB->pBufferData();
			TIFFReadTile(m_pImage,pData,s,t,0,0);
			m_apSegments.push_back(pTMB);
		}
		else
		{
			if (Type() == MapType_Shadow)
				pTMB=new CqTextureMapBuffer(0,0,sizeof(float) * m_XRes,m_YRes,m_SamplesPerPixel,directory);
			else
				pTMB=new CqTextureMapBuffer(0,0,m_XRes,m_YRes,m_SamplesPerPixel,directory);

			TIFFSetDirectory(m_pImage, directory);
			unsigned char* pdata=pTMB->pBufferData();
			TqUint i;
			for(i=0; i<m_YRes; i++)
			{
				TIFFReadScanline(m_pImage,pdata,i);
				if (Type() == MapType_Shadow)
					pdata+=m_XRes*m_SamplesPerPixel*sizeof(float);
				else
					pdata+=m_XRes*m_SamplesPerPixel;
			}
			m_apSegments.push_back(pTMB);
		}
	}	
	return(pTMB);
}

//----------------------------------------------------------------------
/** this is used for re-intrepreted the filter/wrap mode when using 
 *  RiMakeTextureV() for downsampling/filter the tif file
 *
 **/
void CqTextureMap::Interpreted(char *mode)
{
	char filter[80];
	char smode[80];
	char tmode[80];

	sscanf(mode, "%s %s %s %f %f", smode, tmode, filter, &m_swrap, &m_twrap);

	m_FilterFunc = RiBoxFilter;
	if (strcmp(filter,"gaussian") == 0) m_FilterFunc = RiGaussianFilter;
	if (strcmp(filter,"box") == 0) m_FilterFunc = RiBoxFilter;
	if (strcmp(filter,"triangle") == 0) m_FilterFunc = RiTriangleFilter;
	if (strcmp(filter,"catmull-rom") == 0) m_FilterFunc = RiCatmullRomFilter;
	if (strcmp(filter,"sinc") == 0) m_FilterFunc = RiSincFilter;
	if (strcmp(filter,"disk") == 0) m_FilterFunc = RiDiskFilter;
	if (strcmp(filter,"bessel") == 0) m_FilterFunc = RiBesselFilter;

	m_smode = m_tmode=WrapMode_Clamp;
	if(strcmp(smode,RI_PERIODIC)==0)
		m_smode=WrapMode_Periodic;
	else if(strcmp(smode,RI_CLAMP)==0)
		m_smode=WrapMode_Clamp;
	else if(strcmp(smode,RI_BLACK)==0)
		m_smode=WrapMode_Black;

	if(strcmp(tmode,RI_PERIODIC)==0)
		m_tmode=WrapMode_Periodic;
	else if(strcmp(tmode,RI_CLAMP)==0)
		m_tmode=WrapMode_Clamp;
	else if(strcmp(tmode,RI_BLACK)==0)
		m_tmode=WrapMode_Black;
	
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
uint32 CqTextureMap::ImageFilterVal(uint32* p, int x, int y,  int directory)
{
	uint32 val = 0;
	RtFilterFunc pFilter = m_FilterFunc;
	
	float ydelta =  (1 << directory);
	float xdelta =  (1 << directory);
    float div = 0.0;
	float mul;
	float accum[4];
	float dx, dy;
	int ox, oy;
		
	accum[0] = accum[1] = accum[2] = accum[3] = 0.0;

	if (directory) {
		/* downsampling */
		dx = -m_swrap /2.0;
		dy = -m_twrap /2.0;

		 	
		
		for (int j=(int) dy; j <  m_twrap/2; j ++, dy += 1.0) {
			for (int i=(int) dx; i < m_swrap/2; i ++, dx += 1.0) {
	
				ox = i;
				oy = j;
				
				/* clamp the offset to be inside the row,col 
				 * of the original image 
				 */
				if ((x * xdelta) + i < 0.0) 
				{
					ox = 0;
				}
				if ((y * ydelta) + j < 0.0)
				{
					oy = 0;
				}

				if (((float)x * xdelta) + i >= (float) m_XRes ) 
				{
					ox = 0;
				}
				if (((float)y * ydelta) + j >= (float) m_YRes ) 
				{
					oy = 0;
				}

				/* find the value in the original image */
				int pos = m_YRes - (int) ((float)y * ydelta +  oy) - 1;
				pos *= m_XRes;
				pos += (int) ((float)x * xdelta) + ox;
				val = p[pos];

				/* find the filter value */
				mul = (*pFilter)(dx, dy, m_swrap, m_twrap);

				/* ponderate the value */
				accum[0]+=(TIFFGetR(val)/255.0) * mul;
				accum[1]+=(TIFFGetG(val)/255.0) * mul;
				accum[2]+=(TIFFGetB(val)/255.0) * mul;
				accum[3]+=(TIFFGetA(val)/255.0) * mul;  

				/* accumulate the ponderation factor */
				div += mul;
				
			}
		}

		if ((m_swrap <= 0.0) || m_twrap <= 0.0) 
		{
			/* The user provides an swidth or twidth == 0.0
			 * if swidth/twidth equal to 0.0 than turn off the filterfunction.
			 */
			div = 1.0;
			val = p[((m_YRes-(int) (ydelta*y)-1)*m_XRes)+(int) (x * xdelta)];
			accum[0]=TIFFGetR(val)/255.0;
			accum[1]=TIFFGetG(val)/255.0;
			accum[2]=TIFFGetB(val)/255.0;
			accum[3]=TIFFGetA(val)/255.0;  

		}
	
	
		/* use the accumulated ponderation factor */
		accum[0]/=(float)div;
		accum[1]/=(float)div;
		accum[2]/=(float)div;
		accum[3]/=(float)div;

		/* restore the byte from the floating values RGB */
    	/* this is assuming tiff decoding is using shifting operations */
		val = ((unsigned int) (accum[0] * 255.0) & 0xff) +  /* R */
				(((unsigned int)(accum[1] * 255.0) << 8) & 0x0ff00) + /* G */
				(((unsigned int)(accum[2] * 255.0) << 16) & 0x0ff0000) + /* B */
				(((unsigned int)(accum[3] * 255.0) << 24) & 0x0ff000000); /* A */
	

	} else  {
		/* copy the byte don't bother much */
		val = p[((m_YRes-(int) y-1)*m_XRes)+(int) x];
		
	}

	
	return val;
}

void CqTextureMap::CreateSATMap()
{
    if(m_pImage!=0)
    {
	uint32* pImage=static_cast<uint32*>(_TIFFmalloc(m_XRes*m_YRes*sizeof(uint32)));
	TIFFReadRGBAImage(m_pImage, m_XRes, m_YRes, pImage, 0);
	int m_xres = m_XRes;
	int m_yres = m_YRes;
	int directory = 0;
	
    do {
		
		CqTextureMapBuffer* pTMB=new CqTextureMapBuffer();
		pTMB->Init(0,0,m_xres,m_yres,m_SamplesPerPixel, directory);

		if(pTMB->pBufferData() != NULL)	{
			unsigned char* pSATMap=pTMB->pBufferData();
			long rowlen=m_xres*m_SamplesPerPixel;

			if(pImage!=NULL)
			{
				for(TqInt y=0; y<m_yres; y++)
				{
					unsigned char accum[4];
					
					
					for(TqInt x=0; x<m_xres; x++) 
					{
					uint32 val=ImageFilterVal(pImage, x, y, directory);
					
						accum[0]=TIFFGetR(val);
						accum[1]=TIFFGetG(val);
						accum[2]=TIFFGetB(val);
						accum[3]=TIFFGetA(val);   
						
						for(TqInt sample=0; sample<m_SamplesPerPixel; sample++) 
							pSATMap[(y*rowlen)+(x*m_SamplesPerPixel)+sample]=accum[sample];
					}
				}
					
			}
			
			m_apSegments.push_back(pTMB);

		}		

		m_xres /= 2;
		m_yres /= 2;
		directory++;
	
	} while ( (m_xres > 2) && (m_yres > 2) ) ;
	
	_TIFFfree(pImage);
	
	}
}


void CqTextureMap::SampleSATMap(float s1, float t1, float swidth, float twidth, float sblur, float tblur, 
								std::valarray<float>& val, int directory)
{
	// T(s2,t2)-T(s2,t1)-T(s1,t2)+T(s1,t1)

	
	int i;

	
	if(!IsValid())	return;

	val.resize(m_SamplesPerPixel);

	float m_xres, m_yres;

	m_xres = m_XRes/(1<<directory);
	m_yres = m_YRes/(1<<directory);

	float swo2=swidth*0.5f;
	float two2=twidth*0.5f;
	float sbo2=(sblur*0.5f)*m_xres;
	float tbo2=(tblur*0.5f)*m_yres;
	
	long ss1=static_cast<long>(FLOOR((s1-swo2)*(m_xres-1.0))-sbo2);
	long tt1=static_cast<long>(FLOOR((t1-two2)*(m_yres-1.0))-tbo2);
	long ss2=static_cast<long>(CEIL ((s1+swo2)*(m_xres-1.0))+sbo2);
	long tt2=static_cast<long>(CEIL ((t1+two2)*(m_yres-1.0))+sbo2);

	bool fss=ss2-ss1==0;
	bool ftt=tt2-tt1==0;

	std::valarray<float> val1;
	std::valarray<float> val2;
	std::valarray<float> val3;
	std::valarray<float> val4;
	val1.resize(m_SamplesPerPixel);
	val2.resize(m_SamplesPerPixel);
	val3.resize(m_SamplesPerPixel);
	val4.resize(m_SamplesPerPixel);
	val1 = 0.0f;
	val2 = 0.0f;
	val3 = 0.0f;
	val4 = 0.0f;

	

	if (m_smode == WrapMode_Periodic) 
	{
		while(ss1<0)	ss1+=m_xres;
		while(ss1>static_cast<long>(m_xres-1.0))	ss1-=m_xres;
		while(ss2<0)	ss2+=m_xres;
		while(ss2>static_cast<long>(m_xres-1.0))	ss2-=m_xres;
	} 
	if (m_tmode == WrapMode_Periodic) {
		while(tt1<0)	tt1+=m_yres;
		while(tt1>static_cast<long>(m_yres-1.0))	tt1-=m_yres;
		while(tt2<0)	tt2+=m_yres;
		while(tt2>static_cast<long>(m_yres-1.0))	tt2-=m_yres;
	}
	if(Type()==MapType_Environment)
	{
		if(ss1<0)	ss1=0;
		if(tt1<0)	tt1=0;
		if(ss2>static_cast<long>(m_XRes-1))	ss2=(m_XRes-1);
		if(tt2>static_cast<long>(m_YRes-1))	tt2=(m_YRes-1);
	}

	// TODO: This is effectively 'clamp' mode, by default
	if (m_smode == WrapMode_Clamp) {
		if (ss1<0) ss1=0;
		if (ss2<0) ss2=0;
		if(ss2>static_cast<long>(m_xres-1.0))	ss2=(m_xres-1.0);
		if(ss1>static_cast<long>(m_xres-1.0))	ss1=(m_xres-1.0);
	}
	if (m_tmode == WrapMode_Clamp) {
		if (tt1<0) tt1=0;
		if (tt2<0) tt2=0;
		if(tt2>static_cast<long>(m_yres-1.0))	tt2=(m_yres-1.0);
		if(tt1>static_cast<long>(m_yres-1.0))	tt1=(m_yres-1.0);
	}

	// If no boundaries are crossed, just do a single sample (the most common case)
	if((ss1<ss2) && (tt1<tt2))
	{
		GetSample(ss1,tt1,ss2,tt2,val,fss,ftt,directory);
		
	}
	// If it crosses only the s boundary, we need to get two samples.
	else if((ss1>ss2) && (tt1<tt2))
	{
		GetSample(0,tt1,ss2,tt2,val1,fss, ftt, directory);
		GetSample(ss1,tt1,m_xres-1,tt2,val2,fss,ftt,directory);
		val=(val1+val2);
		val*=0.5f;
		
	}
	// If it crosses only the t boundary, we need to get two samples.
	else if((ss1<ss2) && (tt1>tt2))
	{
		GetSample(ss1,0,ss2,tt2,val1,fss,ftt,directory);
		GetSample(ss1,tt1,ss2,m_yres-1,val2,fss,ftt,directory);
		val=(val1+val2);
		val*=0.5f;
		
	}
	// If it crosses the s and t boundary, we need to get four samples.
	else
	{
		GetSample(0,0,ss2,tt2,val1,fss,ftt,directory);
		GetSample(ss1,0,m_xres-1,tt2,val2,fss,ftt,directory);
		GetSample(0,tt1,ss2,m_yres-1,val3,fss,ftt,directory);
		GetSample(ss1,tt1,m_xres-1,m_yres-1,val4,fss,ftt,directory);
		val=(val1+val2+val3+val4);
		val*=0.25f;
		
	}

    
	// Clamp and smoothstep() the result 
	for (i=0; i< m_SamplesPerPixel; i++) {
		if   (val[i] > 1.0)
			val[i] = 1.0; 
		else if (val[i] < 0.0)
			val[i] = 0.0;
		else if ( directory == 0 /* 1 */ ) {
			/* Should we smoothstep the result since we already use the 
			 * filterfunc better ? 
			 * Not entirely sure about that for now.
			 * It is a lowpass filter, it may produce darker image
			 * (or in fact image with stronger contrast).
			 */
			val[i]=val[i]*val[i]*(3.0-2.0*val[i]);
		}
	}

}


void CqTextureMap::GetSample(long ss1, long tt1, long ss2, long tt2, std::valarray<float>& val, bool fss, bool ftt, int directory)
{
	// Read in the relevant texture tiles.
	CqTextureMapBuffer* pTMBa=GetBuffer(ss1,tt1,directory);
	CqTextureMapBuffer* pTMBb=GetBuffer(ss2,tt1,directory);
	CqTextureMapBuffer* pTMBc=GetBuffer(ss1,tt2,directory);
	CqTextureMapBuffer* pTMBd=GetBuffer(ss2,tt2,directory);
    

        /* cannot find anything than goodbye */
        if (!pTMBa || !pTMBb || !pTMBc || !pTMBd) return;

	// all the tile are using the same size therefore the number is ok 
	long rowlen=  pTMBa->Width()*m_SamplesPerPixel; 
	

	TqFloat ds=ss2-ss1;
	TqFloat dt=tt2-tt1;
	TqFloat d=fabs(ds*dt);

	// Adjust the offsets according to the buffer offset and size.

	ss1-=pTMBa->sOrigin();
	tt1-=pTMBa->tOrigin();
	ss2-=pTMBb->sOrigin();
	tt2-=pTMBc->tOrigin();

	ss1*=m_SamplesPerPixel;
	tt1*=rowlen;
	ss2*=m_SamplesPerPixel;
	tt2*=rowlen;


	int c;
	TqFloat ratio = 4.0;

	for(c=0; c<m_SamplesPerPixel; c++)
	{
		 val[c]=pTMBd->pBufferData()[tt2+ss2+c]/255.0;

         if(!ftt) {
              val[c]+=pTMBb->pBufferData()[tt1+ss2+c]/255.0;
		 } else ratio -= 1.0;

         if(!fss)
               val[c]+=pTMBc->pBufferData()[tt2+ss1+c]/255.0;
		 else ratio -= 1.0;

         val[c]+=pTMBa->pBufferData()[tt1+ss1+c]/255.0;
		 val[c] /= ratio;
	}
}

//----------------------------------------------------------------------
/** Retrieve a sample from the SAT map over the area specified by the four vertices
 */

void CqTextureMap::SampleSATMap(float s1, float t1, float s2, float t2, float s3, float t3, float s4, float t4, 
									float sblur, float tblur, 
									std::valarray<float>& val, int directory)
{
	// Work out the width and height
	float ss1,tt1,ss2,tt2;
	ss1=MIN(MIN(MIN(s1,s2),s3),s4);
	tt1=MIN(MIN(MIN(t1,t2),t3),t4);
	ss2=MAX(MAX(MAX(s1,s2),s3),s4);
	tt2=MAX(MAX(MAX(t1,t2),t3),t4);

	float swidth=ss2-ss1;
	float twidth=tt2-tt1;
	ss1=ss1+(swidth*0.5f);
	tt1=tt1+(twidth*0.5f);

	SampleSATMap(ss1,tt1,swidth,twidth,sblur,tblur,val,directory);
}


//----------------------------------------------------------------------
/** Retrieve a color sample from the environment map using R as the reflection vector. 
 * Filtering is done using swidth, twidth and nsamples.
 */

void CqEnvironmentMap::SampleSATMap(CqVector3D& R1, CqVector3D& swidth, CqVector3D& twidth, float sblur, float tblur, 
									std::valarray<float>& val)
{
	if(m_pImage!=0)
	{
		CqVector3D	R2,R3,R4;
		R2=R1+swidth;
		R3=R1+twidth;
		R4=R1+swidth+twidth;

		SampleSATMap(R1,R2,R3,R4,sblur,tblur,val);
	}
}


//----------------------------------------------------------------------
/** Retrieve a sample from the environment map using R as the reflection vector. 
 */

void CqEnvironmentMap::SampleSATMap(CqVector3D& R1, CqVector3D& R2, CqVector3D& R3, CqVector3D& R4, float sblur, float tblur, 
									std::valarray<float>& val)
{
	if(m_pImage!=0)
	{
		CqVector3D	last_R,R,pt;
		TqFloat	texture_area,total_area;
		int		i,j;
		int		projection_bit;		// Stores all the faces the reflected beam projects onto.
		int		edge,last_face,current_face,project_face;

		CqVector3D	vertex_list[4]=
		{
			R1,
			R2,
			R3,
			R4
		};

		val.resize(m_SamplesPerPixel);

		vertex_list[0].Unit();
		vertex_list[1].Unit();
		vertex_list[2].Unit();
		vertex_list[3].Unit();

		cube_no=0;
		projection_bit=0;
		total_area=0.0;

		// Find intersection the reflected vector from the last vertex in the list makes with the cube.
		last_R=vertex_list[3];
		
		get_face_intersection(&last_R, &cube[cube_no], &last_face);
		cube_no++;
		projection_bit|=last_face;

		for(i=0; i<4; i++)
		{
			R=vertex_list[i];
			get_face_intersection(&R, &pt, &current_face);

			// If the last reflected ray intersected a face different from the current
			// ray, we must find the intersection the beam makes with the corresponding edge.
			if(current_face != last_face)
			{
				edge=current_face|last_face;
				get_edge_intersection(&last_R, &R, edge, &cube[cube_no]);
				cube_no++;
				projection_bit|=current_face;
			}
			cube[cube_no]=pt;
			cube_no++;
			last_face=current_face;
			last_R=R;
		}

		std::valarray<float>	run_val;
		run_val.resize(m_SamplesPerPixel);
		run_val=0.0f;
		val=0.0f;
		for(i=0,project_face=1; i<6; i++,project_face+=project_face)
		{
			if(project_face & projection_bit)
			{
				// Get the projection in UVSpace on the project_face
				project(project_face);
				float s1,t1,s2,t2;
				s1=s2=uv[0][0];
				t1=t2=uv[0][1];
				texture_area=0.0;

				for(j=1; j<cube_no; j++)
				{
					if(uv[j][0]<s1)	s1=uv[j][0];
					if(uv[j][1]<t1)	t1=uv[j][1];
					if(uv[j][0]>s2)	s2=uv[j][0];
					if(uv[j][1]>t2)	t2=uv[j][1];
				}
				texture_area=(s2-s1)*(t2-t1);
				if(texture_area<=0.0)	texture_area=1.0f;

				// Calculate the widths and offset the origin before passing onto standard sampling routine.
				float swidth=(s2-s1);
				float twidth=(s2-s1);
				s1+=(swidth*0.5f);
				t1+=(twidth*0.5f);

				CqTextureMap::SampleSATMap(s1,t1,swidth,twidth,sblur,tblur,run_val,i);

				// Add the color contribution weighted by the area
				val+=run_val*texture_area;
				total_area += texture_area;
			}
		}
		// Normalize the weightings
		val/=total_area;
	}
	
}


static void get_face_intersection(CqVector3D *normal, CqVector3D *pt, int* face)
{
	CqVector3D n=*normal;
	TqFloat t;

	// Test nz direction
	if(n.z()<0)	// Test intersection with nz
	{
		t=-0.5/n.z();
		pt->x(n.x()*t);
		pt->y(n.y()*t);
		pt->z(n.z()*t);
		if(fabs(pt->x())<0.5 && fabs(pt->y())<0.5)
		{
			*face=nz;
			return;
		}
	}
	else if(n.z()>0)	// Test intersection with pz
	{
		t=0.5/n.z();
		pt->x(n.x()*t);
		pt->y(n.y()*t);
		pt->z(n.z()*t);
		if(fabs(pt->x())<0.5 && fabs(pt->y())<0.5)
		{
			*face=pz;
			return;
		}
	}


	// Test ny direction
	if(n.y()<0)	// Test intersection with ny
	{
		t=-0.5/n.y();
		pt->x(n.x()*t);
		pt->y(n.y()*t);
		pt->z(n.z()*t);
		if(fabs(pt->x())<0.5 && fabs(pt->z())<0.5)
		{
			*face=ny;
			return;
		}
	}
	else if(n.y()>0)	// Test intersection with py
	{
		t=0.5/n.y();
		pt->x(n.x()*t);
		pt->y(n.y()*t);
		pt->z(n.z()*t);
		if(fabs(pt->x())<0.5 && fabs(pt->z())<0.5)
		{
			*face=py;
			return;
		}
	}

	// Test nx direction
	if(n.x()<0)	// Test intersection with nx
	{
		t=-0.5/n.x();
		pt->x(n.x()*t);
		pt->y(n.y()*t);
		pt->z(n.z()*t);
		if(fabs(pt->y())<0.5 && fabs(pt->z())<0.5)
		{
			*face=nx;
			return;
		}
	}
	else if(n.x()>0)	// Test intersection with px
	{
		t=0.5/n.x();
		pt->x(n.x()*t);
		pt->y(n.y()*t);
		pt->z(n.z()*t);
		if(fabs(pt->y())<0.5 && fabs(pt->z())<0.5)
		{
			*face=px;
			return;
		}
	}
}

static void get_edge_intersection(CqVector3D* n1, CqVector3D* n2, int edge, CqVector3D* pt)
{
	TqFloat a,b,c;
	TqFloat x0,y0,z0,f,g,h;
	TqFloat denom,t;

	// Get plane eqn: ax+by+cz=0 from two normals n1 and n2
	a=n1->y()*n2->z() - n1->z()*n2->y();
	b=n1->z()*n2->x() - n1->x()*n2->z();
	c=n1->x()*n2->y() - n1->y()*n2->x();

	// Set up line equation of edge.
	x0=y0=z0=0.0;
	f=g=h=0.0;
	switch(edge)
	{
		case edge01:	x0=z0=0.5; g=1; break;
		case edge02:	y0=z0=0.5; f=1; break;
		case edge03:	x0=-0.5; z0=0.5; g=1; break;
		case edge04:	y0=-0.5; z0=0.5; f=1; break;
		case edge12:	x0=y0=0.5; h=1; break;
		case edge23:	x0=-0.5; y0=0.5; h=1; break;
		case edge34:	x0=y0=-0.5; h=1; break;
		case edge41:	x0=0.5; y0=-0.5; h=1; break;
		case edge51:	x0=0.5; z0=-0.5; g=1; break;
		case edge52:	y0=0.5; z0=-0.5; f=1; break;
		case edge53:	x0=z0=-0.5; g=1; break;
		case edge54:	y0=z0=-0.5; f=1; break;
	}

	// Return the intersection of the plane and edge
	denom=a*f+b*g+c*h;
	t=-(a*x0 + b*y0 + c*z0)/denom;
	pt->x(x0 + f*t);
	pt->y(y0 + g*t);
	pt->z(z0 + h*t);
}


static void project(int face)
{
	int i;
	switch(face)
	{
		case pz:
			for(i=0; i<cube_no; i++)
			{
				uv[i][0]=  cube[i].x()+0.5;
				uv[i][1]= -cube[i].y()+0.5;
			}
		break;

		case px:
			for(i=0; i<cube_no; i++)
			{
				uv[i][0]= -cube[i].z()+0.5;
				uv[i][1]= -cube[i].y()+0.5;
			}
		break;

		case py:
			for(i=0; i<cube_no; i++)
			{
				uv[i][0]=  cube[i].x()+0.5;
				uv[i][1]=  cube[i].z()+0.5;
			}
		break;

		case nx:
			for(i=0; i<cube_no; i++)
			{
				uv[i][0]=  cube[i].z()+0.5;
				uv[i][1]= -cube[i].y()+0.5;
			}
		break;

		case ny:
			for(i=0; i<cube_no; i++)
			{
				uv[i][0]=  cube[i].x()+0.5;
				uv[i][1]= -cube[i].z()+0.5;
			}
		break;

		case nz:
			for(i=0; i<cube_no; i++)
			{
				uv[i][0]= -cube[i].x()+0.5;
				uv[i][1]= -cube[i].y()+0.5;
			}
		break;
	}
}


#define	dsres		1.0f
#define	dtres		1.0f
#define	ResFactor	1.0f
#define	MinSize		0.0f
#define	NumSamples	16
#define	MinSamples	1

//---------------------------------------------------------------------
/** Sample the shadow map data to see if the point vecPoint is in shadow.
 */

void CqShadowMap::SampleMap(const CqVector3D& vecPoint, const CqVector3D& swidth, const CqVector3D& twidth, float sblur, float tblur, float& val)
{
	if(m_pImage!=0)
	{
		CqVector3D	R1,R2,R3,R4;
		R1=vecPoint-(swidth/2.0f)-(twidth/2.0f);
		R2=vecPoint+(swidth/2.0f)-(twidth/2.0f);
		R3=vecPoint-(swidth/2.0f)+(twidth/2.0f);
		R4=vecPoint+(swidth/2.0f)+(twidth/2.0f);

		SampleMap(R1,R2,R3,R4,sblur,tblur,val);
	}
}


void	CqShadowMap::SampleMap(const CqVector3D& R1, const CqVector3D& R2,const CqVector3D& R3,const CqVector3D& R4, float sblur, float tblur, float& val)
{
	float depth;
	float previousdepth;
	float coverage;

	// get coverage and average depth
	SampleMap(R1,R2,R3,R4,sblur,tblur,coverage,depth);
	previousdepth = depth;

	if(sblur!=0 || tblur!=0)
	{
		int maxiterations=5; // cap the no of times we go round in case we get stuck in a loop
		int iterations=0; 
		while (depth!=0.0 && iterations<maxiterations)
		{ 
			// resize the filter
			sblur *= 1 - depth;
			tblur *= 1 - depth;

			// get coverage and average depth again
			SampleMap(R1,R2,R3,R4,sblur,tblur,coverage,depth);

			// stop if we get roughly the same answer twice
			if(fabs(depth - previousdepth) < 0.05)
				break;

			previousdepth = depth;
			iterations++;
		}
		val = coverage;		
	}
	else
	{
		// get coverage and average depth again
		SampleMap(R1,R2,R3,R4,sblur,tblur,val,depth);
	}
}


void	CqShadowMap::SampleMap(const CqVector3D& R1, const CqVector3D& R2,const CqVector3D& R3,const CqVector3D& R4, float sblur, float tblur, float& val, float& depth)
{
	// If no map defined, not in shadow.
	val=0.0f;
	depth=0.0f;
	
	CqVector3D	vecR1l,vecR2l,vecR3l,vecR4l;
	CqVector3D	vecR1m,vecR2m,vecR3m,vecR4m;

	// Add in the bias at this point in camera coordinates.
	TqFloat bias=0.225f;
	const TqFloat* poptBias=QGetRenderContext()->optCurrent().GetFloatOption("shadow","bias");
	if(poptBias!=0)
		bias=poptBias[0];

	CqVector3D vecBias(0,0,bias);
	// Generate a matrix to transform points from camera space into the space of the light source used in the 
	// definition of the shadow map.
	CqMatrix matCameraToLight=m_matWorldToCamera*QGetRenderContext()->matSpaceToSpace("camera","world");
	// Generate a matrix to transform points from camera space into the space of the shadow map.
	CqMatrix matCameraToMap=m_matWorldToScreen*QGetRenderContext()->matSpaceToSpace("camera","world");

	vecR1l=matCameraToLight*(R1-vecBias);
	vecR2l=matCameraToLight*(R2-vecBias);
	vecR3l=matCameraToLight*(R3-vecBias);
	vecR4l=matCameraToLight*(R4-vecBias);

	vecR1m=matCameraToMap*(R1-vecBias);
	vecR2m=matCameraToMap*(R2-vecBias);
	vecR3m=matCameraToMap*(R3-vecBias);
	vecR4m=matCameraToMap*(R4-vecBias);

	TqFloat z1=vecR1l.z();
	TqFloat z2=vecR2l.z();
	TqFloat z3=vecR3l.z();
	TqFloat z4=vecR4l.z();
	TqFloat z=(z1+z2+z3+z4)*0.25;

	float sbo2=(sblur*0.5f)*m_XRes;
	float tbo2=(tblur*0.5f)*m_YRes;

	// If point is behind light, call it not in shadow.
	//if(z1<0.0)	return;
	
	TqFloat xro2=m_XRes*0.5;
	TqFloat yro2=m_YRes*0.5;
	
	TqFloat s1=vecR1m.x()*xro2+xro2;
	TqFloat t1=m_YRes-(vecR1m.y()*yro2+yro2);
	TqFloat s2=vecR2m.x()*xro2+xro2;
	TqFloat t2=m_YRes-(vecR2m.y()*yro2+yro2);
	TqFloat s3=vecR3m.x()*xro2+xro2;
	TqFloat t3=m_YRes-(vecR3m.y()*yro2+yro2);
	TqFloat s4=vecR4m.x()*xro2+xro2;
	TqFloat t4=m_YRes-(vecR4m.y()*yro2+yro2);

	TqFloat smin=(s1<s2)?s1:(s2<s3)?s2:(s3<s4)?s3:s4;
	TqFloat smax=(s1>s2)?s1:(s2>s3)?s2:(s3>s4)?s3:s4;
	TqFloat tmin=(t1<t2)?t1:(t2<t3)?t2:(t3<t4)?t3:t4;
	TqFloat tmax=(t1>t2)?t1:(t2>t3)?t2:(t3>t4)?t3:t4;
	
	// Cull if outside bounding box.
	TqUint lu=static_cast<TqInt>(FLOOR(smin));
	TqUint hu=static_cast<TqInt>(CEIL (smax));
	TqUint lv=static_cast<TqInt>(FLOOR(tmin));
	TqUint hv=static_cast<TqInt>(CEIL (tmax));

	lu-=static_cast<TqInt>(sbo2);
	lv-=static_cast<TqInt>(tbo2);
	hu+=static_cast<TqInt>(sbo2);
	hv+=static_cast<TqInt>(tbo2);

	if(lu>m_XRes || hu<0 || lv>m_YRes || hv<0)
		return;

	TqFloat sres=hu-lu;
	TqFloat tres=hv-lv;

	// Calculate no. of samples.
	TqInt nt,ns;
	if(sres*tres*4.0 < NumSamples)
	{
		ns=static_cast<TqInt>(sres*2.0+0.5);
		ns=(ns<MinSamples?MinSamples:(ns>NumSamples?NumSamples:ns));
		nt=static_cast<TqInt>(tres*2.0+0.5);
		nt=(nt<MinSamples?MinSamples:(nt>NumSamples?NumSamples:nt));
	}
	else
	{
		nt=static_cast<TqInt>(sqrt(tres*NumSamples/sres)+0.5);
		nt=(nt<MinSamples?MinSamples:(nt>NumSamples?NumSamples:nt));
		ns=static_cast<TqInt>(static_cast<TqFloat>(NumSamples)/nt+0.5);
		ns=(ns<MinSamples?MinSamples:(ns>NumSamples?NumSamples:ns));
	}

	// Setup jitter variables
	TqFloat ds=/*2.0f**/sres/ns;
	TqFloat dt=/*2.0f**/tres/nt;
	TqFloat js=ds*0.5f;
	TqFloat jt=dt*0.5f;

	// Test the samples.
	TqInt inshadow=0;

	TqFloat s=lu;
	TqInt i;
	for(i=0; i<ns; i++)
	{
		TqFloat t=lv;
		TqInt j;
		for(j=0; j<nt; j++)
		{
			// Jitter s and t
			m_rand_index=(m_rand_index+1)&255;
			TqUint iu=static_cast<TqUint>(s+m_aRand_no[m_rand_index]*js);
			m_rand_index=(m_rand_index+1)&255;
			TqUint iv=static_cast<TqUint>(t+m_aRand_no[m_rand_index]*jt);
			// Clip to bounding box.
			if(iu>=0 && iu<m_XRes &&
			   iv>=0 && iv<m_YRes)
			{
				CqTextureMapBuffer* pTMBa=GetBuffer(iu,iv,0);
				if( pTMBa!=0 && pTMBa->pBufferData()!=0)
				{
					iu-=pTMBa->sOrigin();
					iv-=pTMBa->tOrigin();
					TqInt rowlen=pTMBa->Width();
					float *depths = (float *) pTMBa->pBufferData();
					if(z>depths[(iv*rowlen)+iu])
					{
						inshadow+=1;
						depth+=depths[(iv*rowlen)+iu];
					}
				}
			}
			t=t+dt;
		}
		s=s+ds;
	}

	val=(static_cast<TqFloat>(inshadow)/(ns*nt));
	
	// get the average depth of occluded samples
	float lightdistance = MAX(MAX(MAX(vecR1l.Magnitude(), vecR2l.Magnitude()), vecR3l.Magnitude()), vecR4l.Magnitude());
	depth = (depth/inshadow) / lightdistance;
}


//---------------------------------------------------------------------
/** Allocate the memory required by the depthmap.
 */

void CqShadowMap::AllocateMap(TqInt XRes, TqInt YRes)
{
	static CqRandom rand;

	std::vector<CqTextureMapBuffer*>::iterator s;
	for(s=m_apSegments.begin(); s!=m_apSegments.end(); s++)
		delete(*s);

	m_XRes=XRes;
	m_YRes=YRes;
	m_apSegments.push_back(new	CqTextureMapBuffer(0,0,sizeof(float)*m_XRes,m_YRes,1));

	TqInt i;
	for(i=0; i<256; i++)
		m_aRand_no[i]=(rand.RandomFloat(2.0f)-1.0f);
}


//----------------------------------------------------------------------
/** Save the shadowmap data in system specifirc image format.
 */

void CqShadowMap::SaveZFile()
{
	// Save the shadowmap to a binary file.
	if(m_strName!="")
	{
		std::ofstream ofile(m_strName.c_str(), std::ios::binary);
		if(ofile.is_open())
		{
			// Save a file type and version marker
			ofile << ZFILE_HEADER;

			// Save the xres and yres.
			ofile.write(reinterpret_cast<char* >(&m_XRes), sizeof(m_XRes));
			ofile.write(reinterpret_cast<char* >(&m_YRes), sizeof(m_XRes));

			// Save the transformation matrices.
			ofile.write(reinterpret_cast<char*>(m_matWorldToCamera[0]),sizeof(m_matWorldToCamera[0][0])*4);
			ofile.write(reinterpret_cast<char*>(m_matWorldToCamera[1]),sizeof(m_matWorldToCamera[0][0])*4);
			ofile.write(reinterpret_cast<char*>(m_matWorldToCamera[2]),sizeof(m_matWorldToCamera[0][0])*4);
			ofile.write(reinterpret_cast<char*>(m_matWorldToCamera[3]),sizeof(m_matWorldToCamera[0][0])*4);

			ofile.write(reinterpret_cast<char*>(m_matWorldToScreen[0]),sizeof(m_matWorldToScreen[0][0])*4);
			ofile.write(reinterpret_cast<char*>(m_matWorldToScreen[1]),sizeof(m_matWorldToScreen[0][0])*4);
			ofile.write(reinterpret_cast<char*>(m_matWorldToScreen[2]),sizeof(m_matWorldToScreen[0][0])*4);
			ofile.write(reinterpret_cast<char*>(m_matWorldToScreen[3]),sizeof(m_matWorldToScreen[0][0])*4);

			// Now output the depth values
			ofile.write(reinterpret_cast<char*>(m_apSegments[0]->pBufferData()),sizeof(float)*(m_XRes*m_YRes));
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
	if(m_strName!="")
	{
		std::ifstream file(m_strName.c_str(),std::ios::binary);
		if(file!=NULL)
		{
			// Save a file type and version marker
			char* strHeader=new char[strlen(ZFILE_HEADER)];
            char* origHeader=ZFILE_HEADER;
			file.read(strHeader, strlen(ZFILE_HEADER));
			// Check validity of shadow map.
			if(strncmp(strHeader,origHeader,strlen(origHeader))!=0)
			{
				CqString strErr("Error : Invalid shadowmap format - ");
				strErr+=m_strName;
				CqBasicError(ErrorID_InvalidShadowMap,Severity_Normal,strErr.c_str());
				return;
			}

			// Save the xres and yres.
			file.read(reinterpret_cast<char* >(&m_XRes), sizeof(m_XRes));
			file.read(reinterpret_cast<char* >(&m_YRes), sizeof(m_YRes));

			// Save the transformation matrices.
			file.read(reinterpret_cast<char*>(m_matWorldToCamera[0]),sizeof(m_matWorldToCamera[0][0])*4);
			file.read(reinterpret_cast<char*>(m_matWorldToCamera[1]),sizeof(m_matWorldToCamera[0][0])*4);
			file.read(reinterpret_cast<char*>(m_matWorldToCamera[2]),sizeof(m_matWorldToCamera[0][0])*4);
			file.read(reinterpret_cast<char*>(m_matWorldToCamera[3]),sizeof(m_matWorldToCamera[0][0])*4);

			file.read(reinterpret_cast<char*>(m_matWorldToScreen[0]),sizeof(m_matWorldToScreen[0][0])*4);
			file.read(reinterpret_cast<char*>(m_matWorldToScreen[1]),sizeof(m_matWorldToScreen[0][0])*4);
			file.read(reinterpret_cast<char*>(m_matWorldToScreen[2]),sizeof(m_matWorldToScreen[0][0])*4);
			file.read(reinterpret_cast<char*>(m_matWorldToScreen[3]),sizeof(m_matWorldToScreen[0][0])*4);

			// Now output the depth values
			AllocateMap(m_XRes,m_YRes);
			file.read(reinterpret_cast<char*>(m_apSegments[0]->pBufferData()),sizeof(TqFloat)*(m_XRes*m_YRes));

			// Set the matrixes to general, not Identity as default.
			m_matWorldToCamera.SetfIdentity(TqFalse);
			m_matWorldToScreen.SetfIdentity(TqFalse);
		}	
		else
		{
			CqString strErr("Shadow map not found ");
			strErr+=m_strName;
			CqBasicError(ErrorID_FileNotFound,Severity_Normal,strErr.c_str());
		}
	}
}


//----------------------------------------------------------------------
/** Save the shadowmap data in system specifirc image format.
 */

void CqShadowMap::SaveShadowMap(const char* strShadowName)
{
	char version[80];

	// Save the shadowmap to a binary file.
	if(m_strName!="")
	{
		if(m_apSegments.size()!=0)
		{
			TIFF* pshadow=TIFFOpen(strShadowName,"w");
			TIFFCreateDirectory(pshadow);

			// Write the transform matrices.
			float	matWorldToCamera[16];
			float	matWorldToScreen[16];
			TqInt r,c;
			for(r=0; r<4; r++)
			{
				for(c=0; c<4; c++)
				{
					matWorldToCamera[(r*4)+c]=m_matWorldToCamera[r][c];
					matWorldToScreen[(r*4)+c]=m_matWorldToScreen[r][c];
				}
			}
#ifdef  AQSIS_SYSTEM_WIN32
			sprintf(version, "%s %s",STRNAME, VERSION_STR);
#else
			sprintf(version, "%s %s",STRNAME, VERSION);
#endif	
			TIFFSetField(pshadow,TIFFTAG_SOFTWARE, (uint32)version);
			TIFFSetField(pshadow,TIFFTAG_PIXAR_MATRIX_WORLDTOCAMERA, matWorldToCamera);
			TIFFSetField(pshadow,TIFFTAG_PIXAR_MATRIX_WORLDTOSCREEN, matWorldToScreen);
			TIFFSetField(pshadow,TIFFTAG_PIXAR_TEXTUREFORMAT, SHADOWMAP_HEADER);
			TIFFSetField(pshadow,TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);

			// Write the floating point image to the directory.
			float *depths = (float *)m_apSegments[0]->pBufferData();
			WriteTileImage(pshadow,depths,XRes(),YRes(),32,32,1);
			TIFFClose(pshadow);
		}
	}
}


//----------------------------------------------------------------------
/** Read the matrices out of the tiff file.
 */

void CqShadowMap::ReadMatrices()
{
	// Read the transform matrices.
	float*  matWorldToCamera;
	float*	matWorldToScreen;
	int reta=TIFFGetField(m_pImage,TIFFTAG_PIXAR_MATRIX_WORLDTOCAMERA, &matWorldToCamera);
	int retb=TIFFGetField(m_pImage,TIFFTAG_PIXAR_MATRIX_WORLDTOSCREEN, &matWorldToScreen);
	if(!reta || !retb)
		SetInvalid();
	else
	{
		TqInt r,c;
		for(r=0; r<4; r++)
		{
			for(c=0; c<4; c++)
			{
				m_matWorldToCamera[r][c]=matWorldToCamera[(r*4)+c];
				m_matWorldToScreen[r][c]=matWorldToScreen[(r*4)+c];
			}
		}
	}
	// Set the matrixes to general, not Identity as default.
	m_matWorldToCamera.SetfIdentity(TqFalse);
	m_matWorldToScreen.SetfIdentity(TqFalse);
}


//----------------------------------------------------------------------
/** Write an image to an open TIFF file in the current directory as tiled storage.
 * as float values
 */

void CqTextureMap::WriteTileImage(TIFF* ptex, float *raster, unsigned long width, unsigned long length, unsigned long twidth, unsigned long tlength, int samples)
{ 
	//TIFFCreateDirectory(ptex);
	char version[80];
#ifdef  AQSIS_SYSTEM_WIN32
	sprintf(version, "%s %s",STRNAME, VERSION_STR);
#else
	sprintf(version, "%s %s",STRNAME, VERSION);
#endif
	TIFFSetField(ptex,TIFFTAG_SOFTWARE, (uint32)version);
	TIFFSetField(ptex,TIFFTAG_IMAGEWIDTH, width);
	TIFFSetField(ptex,TIFFTAG_IMAGELENGTH, length);
	TIFFSetField(ptex,TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(ptex,TIFFTAG_BITSPERSAMPLE, 32);
	TIFFSetField(ptex,TIFFTAG_SAMPLESPERPIXEL, samples);
	TIFFSetField(ptex,TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	TIFFSetField(ptex,TIFFTAG_TILEWIDTH, twidth);
	TIFFSetField(ptex,TIFFTAG_TILELENGTH, tlength);
	TIFFSetField(ptex,TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);

	int tsize=twidth*tlength;
	int tperrow=(width+twidth-1)/twidth;
	float* ptile=static_cast<float*>(_TIFFmalloc(tsize*samples*sizeof(float)));
	
	if(ptile!=NULL)
	{
		int ctiles=tperrow*((length+tlength-1)/tlength);
		int itile;
		for(itile=0; itile<ctiles; itile++)
		{
			int x=(itile%tperrow)*twidth;
			int y=(itile/tperrow)*tlength;
			float* ptdata=raster+((y*width)+x)*samples;
			// Clear the tile to black.
			memset(ptile,0,tsize*samples*sizeof(float));
			for(unsigned long i=0; i<tlength; i++)
			{
				for(unsigned long j=0; j<twidth; j++)
				{
					if((x+j)<width && (y+i)<length)
					{
						int ii;
						for(ii=0; ii<samples; ii++)
							ptile[(i*twidth*samples)+(((j*samples)+ii))]=ptdata[((j*samples)+ii)];
					}
				}
				ptdata+=(width*samples);
			}
			TIFFWriteTile(ptex,ptile,x,y,0,0);
		}
		TIFFWriteDirectory(ptex);
		
	}
}
//----------------------------------------------------------------------
/** Write an image to an open TIFF file in the current directory as straight storage.
 * as float values
 */


void CqTextureMap::WriteImage(TIFF* ptex, float *raster, unsigned long width, unsigned long length, int samples)
{ 	
	char version[80];
	TIFFCreateDirectory(ptex);

#ifdef  AQSIS_SYSTEM_WIN32
	sprintf(version, "%s %s",STRNAME, VERSION_STR);
#else
	sprintf(version, "%s %s",STRNAME, VERSION);
#endif
	TIFFSetField(ptex,TIFFTAG_SOFTWARE, (uint32)version);
	TIFFSetField(ptex,TIFFTAG_IMAGEWIDTH, width);
	TIFFSetField(ptex,TIFFTAG_IMAGELENGTH, length);
	TIFFSetField(ptex,TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(ptex,TIFFTAG_BITSPERSAMPLE, 32);
	TIFFSetField(ptex,TIFFTAG_SAMPLESPERPIXEL, samples);
	TIFFSetField(ptex,TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	TIFFSetField(ptex,TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
	TIFFSetField(ptex,TIFFTAG_COMPRESSION, COMPRESSION_DEFLATE);
	TIFFSetField(ptex,TIFFTAG_ROWSPERSTRIP, 1);

	float *pdata=raster;
	for(unsigned long i=0; i<length; i++)
	{
		TIFFWriteScanline(ptex,pdata,i);
		pdata+=(width*samples);
	}
	TIFFWriteDirectory(ptex);
}
//----------------------------------------------------------------------
/** Write an image to an open TIFF file in the current directory as tiled storage.
 * as unsigned char values
 */

void CqTextureMap::WriteTileImage(TIFF* ptex, unsigned char *raster, unsigned long width, unsigned long length, unsigned long twidth, unsigned long tlength, int samples)
{ 
	char version[80];
#ifdef  AQSIS_SYSTEM_WIN32
	sprintf(version, "%s %s",STRNAME, VERSION_STR);
#else
	sprintf(version, "%s %s",STRNAME, VERSION);
#endif
	TIFFSetField(ptex,TIFFTAG_SOFTWARE, (uint32)version);
	TIFFSetField(ptex,TIFFTAG_IMAGEWIDTH, width);
	TIFFSetField(ptex,TIFFTAG_IMAGELENGTH, length);
	TIFFSetField(ptex,TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(ptex,TIFFTAG_BITSPERSAMPLE, 8);
	TIFFSetField(ptex,TIFFTAG_SAMPLESPERPIXEL, samples);
	TIFFSetField(ptex,TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	TIFFSetField(ptex,TIFFTAG_TILEWIDTH, twidth);
	TIFFSetField(ptex,TIFFTAG_TILELENGTH, tlength);
	TIFFSetField(ptex,TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
	TIFFSetField(ptex, TIFFTAG_COMPRESSION, COMPRESSION_PACKBITS);

	int tsize=twidth*tlength;
	int tperrow=(width+twidth-1)/twidth;
	unsigned char * ptile=static_cast<unsigned char*>(_TIFFmalloc(tsize*samples));
	
	if(ptile!=NULL)
	{
		int ctiles=tperrow*((length+tlength-1)/tlength);
		int itile;
		for(itile=0; itile<ctiles; itile++)
		{
			int x=(itile%tperrow)*twidth;
			int y=(itile/tperrow)*tlength;
			unsigned char* ptdata=raster+((y*width)+x)*samples;
			// Clear the tile to black.
			memset(ptile,0,tsize*samples);
			for(unsigned long i=0; i<tlength; i++)
			{
				for(unsigned long j=0; j<twidth; j++)
				{
					if((x+j)<width && (y+i)<length)
					{
						int ii;
						for(ii=0; ii<samples; ii++)
							ptile[(i*twidth*samples)+(((j*samples)+ii))]=ptdata[((j*samples)+ii)];
					}
				}
				ptdata+=(width*samples);
			}
			TIFFWriteTile(ptex,ptile,x,y,0,0);
		}
		TIFFWriteDirectory(ptex);
	}
}

//----------------------------------------------------------------------
/** Write an image to an open TIFF file in the current directory as straight storage.
 * as unsigned char values
 */

void CqTextureMap::WriteImage(TIFF* ptex, unsigned char *raster, unsigned long width, unsigned long length, int samples)
{ 	
	char version[80];
	TIFFCreateDirectory(ptex);

#ifdef  AQSIS_SYSTEM_WIN32
	sprintf(version, "%s %s",STRNAME, VERSION_STR);
#else
	sprintf(version, "%s %s",STRNAME, VERSION);
#endif
	TIFFSetField(ptex,TIFFTAG_SOFTWARE, (uint32)version);
	TIFFSetField(ptex,TIFFTAG_IMAGEWIDTH, width);
	TIFFSetField(ptex,TIFFTAG_IMAGELENGTH, length);
	TIFFSetField(ptex,TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(ptex,TIFFTAG_BITSPERSAMPLE, 8);
	TIFFSetField(ptex,TIFFTAG_SAMPLESPERPIXEL, samples);
	TIFFSetField(ptex,TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	TIFFSetField(ptex,TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
	TIFFSetField(ptex,TIFFTAG_COMPRESSION, COMPRESSION_PACKBITS);
	TIFFSetField(ptex,TIFFTAG_ROWSPERSTRIP, 1);

	unsigned char *pdata=raster;
	for(unsigned long i=0; i<length; i++)
	{
		TIFFWriteScanline(ptex,pdata,i);
		pdata+=(width*samples);
	}
	TIFFWriteDirectory(ptex);
}


END_NAMESPACE(Aqsis)
//---------------------------------------------------------------------
