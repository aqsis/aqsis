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
#include	"file.h"
#include	"exception.h"
#include	"irenderer.h"

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
int		cube_no;		// Stores the number of points making up the projection.
TqFloat	uv[max_no][2];	// Stores the values of this projection fro a given face.

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
	return(static_cast<float*>(malloc((width*height)*samples*sizeof(float))));
}


//---------------------------------------------------------------------
/** Static array of cached texture maps.
 */

void	CqTextureMapBuffer::FreeSegment(float* pBufferData, unsigned long width, unsigned long height, int samples)
{
	// TODO: Implement proper global cache handling.
	free(pBufferData);
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqTextureMap::~CqTextureMap()
{
	// Search for it in the cache and remove the reference.
	int i;
	for(i=0; i<m_TextureMap_Cache.size(); i++)
	{
		if(m_TextureMap_Cache[i]==this)
		{
			m_TextureMap_Cache.erase(m_TextureMap_Cache.begin()+i);
			break;
		}
	}

	// Delete any held cache buffer segments.
	for(i=0; i<m_apSegments.size(); i++)
	{
		delete(m_apSegments[i]);
	}
}


//---------------------------------------------------------------------
/** Open a named texture map.
 */

void CqTextureMap::Open()
{
	m_IsValid=TqFalse;
	// Find the file required.
	CqFile	fileImage(m_strName.c_str(),"texture");
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

		TIFFGetField(m_pImage, TIFFTAG_IMAGEWIDTH, &m_XRes);
		TIFFGetField(m_pImage, TIFFTAG_IMAGELENGTH, &m_YRes);
		TIFFGetField(m_pImage, TIFFTAG_PLANARCONFIG, &m_PlanarConfig);
		TIFFGetField(m_pImage, TIFFTAG_SAMPLESPERPIXEL, &m_SamplesPerPixel);
		TIFFGetField(m_pImage, TIFFTAG_IMAGEDESCRIPTION, &pFormat);

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
	int i;
	for(i=0; i<m_TextureMap_Cache.size(); i++)
	{
		if(m_TextureMap_Cache[i]->m_strName==strName)
		{ 
			if(m_TextureMap_Cache[i]->Type()==MapType_Texture)
				return(m_TextureMap_Cache[i]);
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
	int i;
	for(i=0; i<m_TextureMap_Cache.size(); i++)
	{
		if(m_TextureMap_Cache[i]->m_strName==strName)
		{
			if(m_TextureMap_Cache[i]->Type()==MapType_Environment)
				return(m_TextureMap_Cache[i]);
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
	int i;
	for(i=0; i<m_TextureMap_Cache.size(); i++)
	{
		if(m_TextureMap_Cache[i]->m_strName==strName)
		{
			if(m_TextureMap_Cache[i]->Type()==MapType_Shadow)
				return(m_TextureMap_Cache[i]);
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
	int i;
	for(i=0; i<m_apSegments.size(); i++)
		if(m_apSegments[i]->IsValid(s,t,directory))	return(m_apSegments[i]);

	// If we got here, segment is not currently loaded, so load the correct segement and store it in the cache.
	CqTextureMapBuffer* pTMB=0;

	if(m_pImage)
	{
		uint32 tsx,tsy;
		int ret=TIFFGetField(m_pImage,TIFFTAG_TILEWIDTH,&tsx);
		TIFFGetField(m_pImage,TIFFTAG_TILELENGTH,&tsy);
		tsize_t tsize=TIFFTileSize(m_pImage);
		ttile_t ctiles=TIFFNumberOfTiles(m_pImage);
		// If a tiled image, read the appropriate tile.
		if(ret)
		{
			// Work out the coordinates of this tile.
			unsigned long ox=(s/tsx)*tsx;
			unsigned long oy=(t/tsy)*tsy;
						
			pTMB=new CqTextureMapBuffer(ox,oy,tsx,tsy,m_SamplesPerPixel,directory);

			int res=TIFFSetDirectory(m_pImage, directory);
			float* pData=pTMB->pBufferData();
			TIFFReadTile(m_pImage,pData,s,t,0,0);
			m_apSegments.push_back(pTMB);
		}
		else
		{
			size_t npixels;
			npixels = m_XRes * m_YRes;

			pTMB=new CqTextureMapBuffer(0,0,m_XRes,m_YRes,m_SamplesPerPixel,directory);
			
			int res=TIFFSetDirectory(m_pImage, directory);
			float* pdata=pTMB->pBufferData();
			for(unsigned long i=0; i<m_YRes; i++)
			{
				TIFFReadScanline(m_pImage,pdata,i);
				pdata+=m_XRes*m_SamplesPerPixel;
			}
			m_apSegments.push_back(pTMB);
		}
	}	
	return(pTMB);
}


void CqTextureMap::CreateSATMap()
{
	if(m_pImage!=0)
	{
		CqTextureMapBuffer* pTMB=new CqTextureMapBuffer(0,0,m_XRes,m_YRes,m_SamplesPerPixel);

		if(pTMB->pBufferData() != NULL)
		{
			m_apSegments.push_back(pTMB);

			float* pSATMap=pTMB->pBufferData();
			long rowlen=m_XRes*m_SamplesPerPixel;

			size_t npixels;
			npixels = m_XRes * m_YRes;

			uint32* pImage=static_cast<uint32*>(_TIFFmalloc(m_XRes*m_YRes*sizeof(uint32)));
			TIFFReadRGBAImage(m_pImage, m_XRes, m_YRes, pImage, 0);

			if(pImage!=NULL)
			{
				for(uint32 y=0; y<m_YRes; y++)
				{
					float raccum=0;
					float gaccum=0;
					float baccum=0;
					for(uint32 x=0; x<m_XRes; x++)
					{
						uint32 val=pImage[((m_YRes-y-1)*m_XRes)+x];

						float r=TIFFGetR(val);
						float g=TIFFGetG(val);
						float b=TIFFGetB(val);

						r/=255.0;
						g/=255.0;
						b/=255.0;

						raccum+=r;
						gaccum+=g;
						baccum+=b;

						if(y==0)
						{
							pSATMap[(x*m_SamplesPerPixel)  ]=raccum;
							pSATMap[(x*m_SamplesPerPixel)+1]=gaccum;
							pSATMap[(x*m_SamplesPerPixel)+2]=baccum;
						}
						else
						{
							pSATMap[(y*rowlen)+(x*m_SamplesPerPixel)  ]=raccum+pSATMap[((y-1)*rowlen)+(x*m_SamplesPerPixel)  ];
							pSATMap[(y*rowlen)+(x*m_SamplesPerPixel)+1]=gaccum+pSATMap[((y-1)*rowlen)+(x*m_SamplesPerPixel)+1];
							pSATMap[(y*rowlen)+(x*m_SamplesPerPixel)+2]=baccum+pSATMap[((y-1)*rowlen)+(x*m_SamplesPerPixel)+2];
						}
					}		
				}
				_TIFFfree(pImage);
			}
		}
	}
}


void CqTextureMap::SampleSATMap(float s1, float t1, float swidth, float twidth, float sblur, float tblur, 
								std::valarray<float>& val, int directory)
{
	// T(s2,t2)-T(s2,t1)-T(s1,t2)+T(s1,t1)

	if(!IsValid())	return;

	val.resize(m_SamplesPerPixel);

	float swo2=swidth*0.5f;
	float two2=twidth*0.5f;
	float sbo2=(sblur*0.5f)*m_XRes;
	float tbo2=(tblur*0.5f)*m_YRes;
	
	long ss1=static_cast<long>(floor((s1-swo2)*(m_XRes-1))-sbo2);
	long tt1=static_cast<long>(floor((t1-two2)*(m_YRes-1))-tbo2);
	long ss2=static_cast<long>(ceil ((s1+swo2)*(m_XRes-1))+sbo2);
	long tt2=static_cast<long>(ceil ((t1+two2)*(m_YRes-1))+sbo2);

	bool fss=ss2-ss1==0;
	bool ftt=tt2-tt1==0;

	// TODO: This is effectively 'clamp' mode, should implement this as modes, and set the mode as clamp in the env map.
	if(Type()==MapType_Environment)
	{
		if(ss1<0)	ss1=0;
		if(tt1<0)	tt1=0;
		if(ss2>(m_XRes-1))	ss2=(m_XRes-1);
		if(tt2>(m_YRes-1))	tt2=(m_YRes-1);
	}
	else
	{
		while(ss1<0)	ss1+=m_XRes;
		while(tt1<0)	tt1+=m_YRes;
		while(ss2<0)	ss2+=m_XRes;
		while(tt2<0)	tt2+=m_YRes;
		while(ss1>(m_XRes-1))	ss1-=m_XRes;
		while(tt1>(m_YRes-1))	tt1-=m_YRes;
		while(ss2>(m_XRes-1))	ss2-=m_XRes;
		while(tt2>(m_YRes-1))	tt2-=m_YRes;
	}

	// If no boundaries are crossed, just do a single sample (the most common case)
	if((ss1<ss2) && (tt1<tt2))
	{
		GetSample(ss1,tt1,ss2,tt2,val,fss,ftt,directory);
	}
	// If it crosses only the s boundary, we need to get two samples.
	else if((ss1>ss2) && (tt1<tt2))
	{
		std::valarray<float> val1, val2;
		val1.resize(m_SamplesPerPixel);
		val2.resize(m_SamplesPerPixel);

		GetSample(0,tt1,ss2,tt2,val1,fss, ftt, directory);
		GetSample(ss1,tt1,m_XRes-1,tt2,val2,fss,ftt,directory);
		val=(val1+val2);
		val*=0.5f;
	}
	// If it crosses only the t boundary, we need to get two samples.
	else if((ss1<ss2) && (tt1>tt2))
	{
		std::valarray<float> val1, val2;
		val1.resize(m_SamplesPerPixel);
		val2.resize(m_SamplesPerPixel);

		GetSample(ss1,0,ss2,tt2,val1,fss,ftt,directory);
		GetSample(ss1,tt1,ss2,m_YRes-1,val2,fss,ftt,directory);
		val=(val1+val2);
		val*=0.5f;
	}
	// If it crosses the s and t boundary, we need to get four samples.
	else
	{
		std::valarray<float> val1, val2, val3, val4;
		val1.resize(m_SamplesPerPixel);
		val2.resize(m_SamplesPerPixel);
		val3.resize(m_SamplesPerPixel);
		val4.resize(m_SamplesPerPixel);

		GetSample(0,0,ss2,tt2,val1,fss,ftt,directory);
		GetSample(ss1,0,m_XRes-1,tt2,val2,fss,ftt,directory);
		GetSample(0,tt1,ss2,m_YRes-1,val3,fss,ftt,directory);
		GetSample(ss1,tt1,m_XRes-1,m_YRes-1,val4,fss,ftt,directory);
		val=(val1+val2+val3+val4);
		val*=0.25f;
	}
}


void CqTextureMap::GetSample(long ss1, long tt1, long ss2, long tt2, std::valarray<float>& val, bool fss, bool ftt, int directory)
{
	// Read in the relevant texture tiles.
	CqTextureMapBuffer* pTMBa=GetBuffer(ss1,tt1,directory);
	CqTextureMapBuffer* pTMBb=GetBuffer(ss2,tt1,directory);
	CqTextureMapBuffer* pTMBc=GetBuffer(ss1,tt2,directory);
	CqTextureMapBuffer* pTMBd=GetBuffer(ss2,tt2,directory);

	long rowlen=pTMBa->Width()*m_SamplesPerPixel;

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
	for(c=0; c<m_SamplesPerPixel; c++)
	{
		val[c]=pTMBd->pBufferData()[tt2+ss2+c];

		if(!ftt)
			val[c]-=pTMBb->pBufferData()[tt1+ss2+c];

		if(!fss)
			val[c]-=pTMBc->pBufferData()[tt2+ss1+c];

		val[c]+=pTMBa->pBufferData()[tt1+ss1+c];
	}
	val/=d;
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
	// If no map defined, not in shadow.
	val=0.0f;

	CqVector3D	vecR1l,vecR2l,vecR3l,vecR4l;
	CqVector3D	vecR1s,vecR2s,vecR3s,vecR4s;

	CqMatrix matCameraToScreen=m_matWorldToScreen*pCurrentRenderer()->matSpaceToSpace("camera","world");
	CqMatrix matCameraToLight=m_matWorldToCamera*pCurrentRenderer()->matSpaceToSpace("camera","world");

	vecR1l=matCameraToLight*R1;
	vecR2l=matCameraToLight*R2;
	vecR3l=matCameraToLight*R3;
	vecR4l=matCameraToLight*R4;

	vecR1s=matCameraToScreen*R1;
	vecR2s=matCameraToScreen*R2;
	vecR3s=matCameraToScreen*R3;
	vecR4s=matCameraToScreen*R4;

	TqFloat z1=vecR1l.z();
	TqFloat z2=vecR2l.z();
	TqFloat z3=vecR3l.z();
	TqFloat z4=vecR4l.z();
	TqFloat z=(z1+z2+z3+z4)*0.25;

	float sbo2=(sblur*0.5f)*m_XRes;
	float tbo2=(tblur*0.5f)*m_YRes;

	// If point is behind light, call it not in shadow.
	//if(z1<0.0)	return;
	
	TqFloat s1=vecR1s.x();
	TqFloat t1=vecR1s.y();
	TqFloat s2=vecR2s.x();
	TqFloat t2=vecR2s.y();
	TqFloat s3=vecR3s.x();
	TqFloat t3=vecR3s.y();
	TqFloat s4=vecR4s.x();
	TqFloat t4=vecR4s.y();

	TqFloat smin=(s1<s2)?s1:(s2<s3)?s2:(s3<s4)?s3:s4;
	TqFloat smax=(s1>s2)?s1:(s2>s3)?s2:(s3>s4)?s3:s4;
	TqFloat tmin=(t1<t2)?t1:(t2<t3)?t2:(t3<t4)?t3:t4;
	TqFloat tmax=(t1>t2)?t1:(t2>t3)?t2:(t3>t4)?t3:t4;
	
	// Cull if outside bounding box.
	TqInt lu=static_cast<TqInt>(floor(smin));
	TqInt hu=static_cast<TqInt>(ceil(smax));
	TqInt lv=static_cast<TqInt>(floor(tmin));
	TqInt hv=static_cast<TqInt>(ceil(tmax));

	lu-=sbo2;
	lv-=tbo2;
	hu+=sbo2;
	hv+=tbo2;

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
	//smin=smin+js;
	//tmin=tmin+jt;

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
			TqInt iu=static_cast<TqInt>(s+m_aRand_no[m_rand_index]*js);
			m_rand_index=(m_rand_index+1)&255;
			TqInt iv=static_cast<TqInt>(t+m_aRand_no[m_rand_index]*jt);
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
					if(z>pTMBa->pBufferData()[(iv*rowlen)+iu])
						inshadow+=1;
				}
			}
			t=t+dt;
		}
		s=s+ds;
	}

	val=(static_cast<TqFloat>(inshadow)/(ns*nt));
}


//---------------------------------------------------------------------
/** Set the reolution of the image buffer.
 */

void CqShadowMap::SetImage()
{
	// Call through to the standard image buffer function.
	CqImageBuffer::SetImage();
	// Allocate the map.
	AllocateMap((CropWindowXMax()-CropWindowXMin()),(CropWindowYMax()-CropWindowYMin()));

	// Set up the transformation matrices.
	m_matWorldToCamera=pCurrentRenderer()->matSpaceToSpace("world","camera");
	m_matWorldToScreen=pCurrentRenderer()->matSpaceToSpace("world","raster");
}


//---------------------------------------------------------------------
/** Bucket is complete so store it in our depth buffer.
 */

void CqShadowMap::BucketComplete(TqInt iBucket)
{
	// Copy the bucket to the display buffer.
	CqVector2D	vecA=Position(iBucket);
	CqVector2D	vecB=Size(iBucket);

	// Check if this bucket is outside the crop window.
	if((vecA.x()+vecB.x())<CropWindowXMin() ||
	   (vecA.y()+vecB.y())<CropWindowYMin() ||
	   (vecA.x())>CropWindowXMax() ||
	   (vecA.y())>CropWindowYMax())
		return;

	TqInt y;
	for(y=0; y<vecB.y(); y++)
	{
		TqInt sy=y+static_cast<TqInt>(vecA.y());
		TqInt x;
		for(x=0; x<vecB.x(); x++)
		{
			TqInt sx=x+static_cast<TqInt>(vecA.x());

			TqInt isx=sx-CropWindowXMin();
			TqInt isy=sy-CropWindowYMin();
			if(isx<0 || isy<0 || isx>=m_XRes || isy>=m_YRes)
				continue;

			CqImageElement* pie;
			if(Pixel(sx,sy,iBucket,pie))
			{
				std::vector<SqImageValue>& aValues=pie->Values(0,0);
	
				TqFloat Depth;
				if(aValues.size()<=1)
					Depth=FLT_MAX;
				else
				{
					TqFloat Diff=(aValues[1].m_Depth-aValues[0].m_Depth);
					Depth=(Diff/2.0)+aValues[0].m_Depth;
				}
				
				TqInt so=(isy*m_XRes)+isx;
				m_apSegments[0]->pBufferData()[so]=Depth;
			}
		}
	}
}


//----------------------------------------------------------------------
/** Called by the renderer when the image is complete.
 */

void CqShadowMap::ImageComplete()
{
	SaveZFile();
}


//---------------------------------------------------------------------
/** Allocate the memory required by the depthmap.
 */

void CqShadowMap::AllocateMap(TqInt XRes, TqInt YRes)
{
	static CqRandom rand;

	if(m_apSegments.size()>0)	delete(m_apSegments[0]);
	
	m_XRes=XRes;
	m_YRes=YRes;
	m_apSegments.push_back(new	CqTextureMapBuffer(0,0,m_XRes,m_YRes,1));

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
		CqFile fileShad(m_strName.c_str());
		std::istream* pfile=fileShad;
		if(pfile!=NULL)
		{
			// Save a file type and version marker
			char* strHeader=new char[strlen(ZFILE_HEADER)];
			pfile->read(strHeader, strlen(ZFILE_HEADER));
			// Check validity of shadow map.
			if(strncmp(strHeader,ZFILE_HEADER,strlen(ZFILE_HEADER))!=0)
			{
				CqString strErr("Error : Invalid shadowmap format - ");
				strErr+=m_strName;
				CqBasicError(ErrorID_InvalidShadowMap,Severity_Normal,strErr.c_str());
				return;
			}

			// Save the xres and yres.
			pfile->read(reinterpret_cast<char* >(&m_XRes), sizeof(m_XRes));
			pfile->read(reinterpret_cast<char* >(&m_YRes), sizeof(m_XRes));

			// Save the transformation matrices.
			pfile->read(reinterpret_cast<char*>(m_matWorldToCamera[0]),sizeof(m_matWorldToCamera[0][0])*4);
			pfile->read(reinterpret_cast<char*>(m_matWorldToCamera[1]),sizeof(m_matWorldToCamera[0][0])*4);
			pfile->read(reinterpret_cast<char*>(m_matWorldToCamera[2]),sizeof(m_matWorldToCamera[0][0])*4);
			pfile->read(reinterpret_cast<char*>(m_matWorldToCamera[3]),sizeof(m_matWorldToCamera[0][0])*4);

			pfile->read(reinterpret_cast<char*>(m_matWorldToScreen[0]),sizeof(m_matWorldToScreen[0][0])*4);
			pfile->read(reinterpret_cast<char*>(m_matWorldToScreen[1]),sizeof(m_matWorldToScreen[0][0])*4);
			pfile->read(reinterpret_cast<char*>(m_matWorldToScreen[2]),sizeof(m_matWorldToScreen[0][0])*4);
			pfile->read(reinterpret_cast<char*>(m_matWorldToScreen[3]),sizeof(m_matWorldToScreen[0][0])*4);

			// Now output the depth values
			AllocateMap(m_XRes,m_YRes);
			pfile->read(reinterpret_cast<char*>(m_apSegments[0]->pBufferData()),sizeof(float)*(m_XRes*m_YRes));

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
	// Save the shadowmap to a binary file.
	if(m_strName!="")
	{
		if(m_apSegments.size()!=0)
		{
			TIFF* pshadow=TIFFOpen(strShadowName,"w");
//			TIFFCreateDirectory(pshadow);

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
			TIFFSetField(pshadow,TIFFTAG_PIXAR_MATRIX_WORLDTOCAMERA, matWorldToCamera);
			TIFFSetField(pshadow,TIFFTAG_PIXAR_MATRIX_WORLDTOSCREEN, matWorldToScreen);
			TIFFSetField(pshadow,TIFFTAG_PIXAR_TEXTUREFORMAT, SHADOWMAP_HEADER);
			TIFFSetField(pshadow,TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);

			// Write the floating point image to the directory.
			WriteTileImage(pshadow,m_apSegments[0]->pBufferData(),XRes(),YRes(),64,64,1);
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
 */

void WriteTileImage(TIFF* ptex, float *raster, unsigned long width, unsigned long length, unsigned long twidth, unsigned long tlength, int samples)
{ 
//	TIFFCreateDirectory(ptex);
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


void WriteImage(TIFF* ptex, float *raster, unsigned long width, unsigned long length, int samples)
{ 
//	TIFFCreateDirectory(ptex);
	TIFFSetField(ptex,TIFFTAG_IMAGEWIDTH, width);
	TIFFSetField(ptex,TIFFTAG_IMAGELENGTH, length);
	TIFFSetField(ptex,TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(ptex,TIFFTAG_BITSPERSAMPLE, 32);
	TIFFSetField(ptex,TIFFTAG_SAMPLESPERPIXEL, samples);
	TIFFSetField(ptex,TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	TIFFSetField(ptex,TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
	TIFFSetField(ptex,TIFFTAG_COMPRESSION, COMPRESSION_DEFLATE);
	TIFFSetField(ptex,TIFFTAG_ROWSPERSTRIP, 1);

//	float *pdata=raster+((length-1)*width*samples);
	float *pdata=raster;
	for(unsigned long i=0; i<length; i++)
	{
		TIFFWriteScanline(ptex,pdata,i);
		pdata+=(width*samples);
	}
	TIFFWriteDirectory(ptex);
}

END_NAMESPACE(Aqsis)
//---------------------------------------------------------------------
