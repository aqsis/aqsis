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
		\brief Simple example display device manager.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/


#include	"aqsis.h"

#include	"renderer.h"
#include	"ddmsimple.h"
#include	"imagebuffer.h"
#include	"file.h"
#include	"tiffio.h"

START_NAMESPACE(Aqsis)


//---------------------------------------------------------------------
/** Initialise the device manager.
 */

TqInt CqDDManagerSimple::Initialise()
{
	return(0);
}

TqInt CqDDManagerSimple::Shutdown()
{
	return(0);
}


TqInt CqDDManagerSimple::AddDisplay(const TqChar* name, const TqChar* type, const TqChar* mode)
{
	m_aDisplayRequests.push_back(SqDDevice(name, type, mode));
	return(0);
}

TqInt CqDDManagerSimple::ClearDisplays()
{
	m_aDisplayRequests.clear();
	return(0);
}

TqInt CqDDManagerSimple::OpenDisplays()
{
	std::vector<SqDDevice>::iterator i;
	for(i=m_aDisplayRequests.begin(); i!=m_aDisplayRequests.end(); i++)
	{
		i->m_XRes=QGetRenderContext()->pImage()->iXRes();
		i->m_YRes=QGetRenderContext()->pImage()->iYRes();
		RtInt mode=0;
		if(strstr(i->m_strMode.c_str(), RI_RGB)!=NULL)
			mode|=ModeRGB;
		if(strstr(i->m_strMode.c_str(), RI_A)!=NULL)
			mode|=ModeA;
		if(strstr(i->m_strMode.c_str(), RI_Z)!=NULL)
			mode|=ModeZ;
		TqInt SamplesPerElement=mode&ModeRGB?3:0;
			  SamplesPerElement+=mode&ModeA?1:0;
			  SamplesPerElement=mode&ModeZ?1:SamplesPerElement;
		i->m_SamplesPerElement=SamplesPerElement;

		// Create a buffer big enough to hold a row of buckets.
		i->m_pData=new unsigned char[i->m_XRes*i->m_YRes*i->m_SamplesPerElement];
	}
	return(0);
}

TqInt CqDDManagerSimple::CloseDisplays()
{
	std::vector<SqDDevice>::iterator i;
	for(i=m_aDisplayRequests.begin(); i!=m_aDisplayRequests.end(); i++)
	{
		uint16 photometric = PHOTOMETRIC_RGB;
		uint16 config = PLANARCONFIG_CONTIG;
		uint16 compression = COMPRESSION_NONE;

		TIFF* pOut = TIFFOpen(i->m_strName.c_str(), "w");

		if(pOut)
		{
			// Write the image to a tiff file.
			
			int ExtraSamplesTypes[1]={EXTRASAMPLE_ASSOCALPHA};
			
			TIFFSetField(pOut, TIFFTAG_IMAGEWIDTH, (uint32)i->m_XRes);
			TIFFSetField(pOut, TIFFTAG_IMAGELENGTH, (uint32)i->m_YRes);
			TIFFSetField(pOut, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
			TIFFSetField(pOut, TIFFTAG_SAMPLESPERPIXEL, i->m_SamplesPerElement);
			TIFFSetField(pOut, TIFFTAG_BITSPERSAMPLE, 8);
			TIFFSetField(pOut, TIFFTAG_PLANARCONFIG, config);			
			TIFFSetField(pOut, TIFFTAG_COMPRESSION, compression);
			TIFFSetField(pOut, TIFFTAG_PHOTOMETRIC, photometric);
			TIFFSetField(pOut, TIFFTAG_ROWSPERSTRIP,	TIFFDefaultStripSize(pOut, -1));
			
			if(i->m_SamplesPerElement==4)
				TIFFSetField(pOut, TIFFTAG_EXTRASAMPLES,	1, ExtraSamplesTypes);

			// Set the position tages in case we aer dealing with a cropped image.
			//TIFFSetField(pOut, TIFFTAG_XPOSITION, (float)CWXMin);
			//TIFFSetField(pOut, TIFFTAG_YPOSITION, (float)CWYMin);

			TqInt	linelen=i->m_XRes*i->m_SamplesPerElement;
			TqInt row;
			for(row=0; row<i->m_YRes; row++)
			{
				if(TIFFWriteScanline(pOut, i->m_pData+(row*linelen), row, 0)<0)
					break;
			}
			TIFFClose(pOut);
		}
	}
	return(0);
}



TqInt CqDDManagerSimple::DisplayBucket(IqBucket* pBucket)
{
	std::vector<SqDDevice>::iterator i;
	for(i=m_aDisplayRequests.begin(); i!=m_aDisplayRequests.end(); i++)
	{
		TqInt		xmin=pBucket->XOrigin();
		TqInt		ymin=pBucket->YOrigin();
		TqInt		xsize=pBucket->XSize();
		TqInt		ysize=pBucket->YSize();
		TqInt		xmaxplus1=xmin+xsize;
		TqInt		ymaxplus1=ymin+ysize;

		for(std::vector<SqDDevice>::iterator i=m_aDisplayRequests.begin(); i!=m_aDisplayRequests.end(); i++)
		{
			TqInt		samples=i->m_SamplesPerElement;
			TqInt		linelen=i->m_XRes*samples;

			RtInt mode=0;
			if(strstr(i->m_strMode.c_str(), RI_RGB)!=NULL)
				mode|=ModeRGB;
			if(strstr(i->m_strMode.c_str(), RI_A)!=NULL)
				mode|=ModeA;
			if(strstr(i->m_strMode.c_str(), RI_Z)!=NULL)
				mode|=ModeZ;

			SqImageValue val;
			TqInt y;
			for(y=0; y<ysize; y++)
			{
				TqInt sy=y+ymin;
				TqInt x;
				for(x=0; x<xsize; x++)
				{
					TqInt sx=x+xmin;
					TqInt so=(sy*linelen)+(sx*samples);
					// If outputting a zfile, use the midpoint method.
					/// \todo Should really be generalising this section to use specif Filter/Expose/Quantize functions.
					if(mode&ModeZ)
					{
						i->m_pData[so]=pBucket->Depth(sx,sy);
					}
					else
					{
						if(samples>=3)
						{
							CqColor col=pBucket->Color(sx,sy);
							i->m_pData[so+0]=col.fRed();
							i->m_pData[so+1]=col.fGreen();
							i->m_pData[so+2]=col.fBlue();
							if(samples==4)
								i->m_pData[so+3]=pBucket->Coverage(sx,sy);
						}
						else if(samples==1)
							i->m_pData[so+0]=pBucket->Coverage(sx,sy);
					}
				}
			}
		}
	}
	return(0);
}



END_NAMESPACE(Aqsis)
