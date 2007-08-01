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


#if ENABLE_MPDUMP

#include "mpdump.h"
#include "micropolygon.h"
#include "imagebuffer.h"

START_NAMESPACE( Aqsis )


// Constructor
CqMPDump::CqMPDump()
		: out(NULL), mpcount(0)
{}



// Destructor
CqMPDump::~CqMPDump()
{
	close();
}

// Open the dump file
void CqMPDump::open()
{
	char filename[20] = "mpdump.mp";
	int sf = sizeof(TqFloat);

	close();
	mpcount = 0;
	out = fopen(filename, "wb");
	if (out!=NULL)
	{
		Aqsis::log() << info << "Creating '" << filename << "'" << std::endl;
		size_t len_written = fwrite((void*)&sf, sizeof(int), 1, out);
		if(len_written != 1)
			throw(XqException("Error writing mpdump file"));
	}
	else
		Aqsis::log() << error << "Could not create '" << filename << "'" << std::endl;
}

// Close the dump file
void CqMPDump::close()
{
	if (out!=NULL)
	{
		fclose(out);
		out=NULL;
		Aqsis::log() << info << mpcount << " micro polygons dumped" << std::endl;
	}
}

// Dump global information about the image
void CqMPDump::dumpImageInfo()
{
	short id = 3;

	if (out==NULL)
	{
		Aqsis::log() << error << "Attempted to write to unopened mpdump file." << std::endl;
		return;
	}

	int width = QGetRenderContext()->pImage()->iXRes();
	int height = QGetRenderContext()->pImage()->iYRes();
	size_t len_written = fwrite((void*)&id, sizeof(short), 1, out);
	len_written += fwrite((void*)&width, sizeof(int), 1, out);
	len_written += fwrite((void*)&height, sizeof(int), 1, out);
	if(len_written != 3)
		throw(XqException("Error writing mpdump file"));
}

// Dump all pixel samples of the current bucket
void CqMPDump::dumpPixelSamples(TqInt bucketCol, TqInt bucketRow, const CqBucket* currentBucket)
{
	CqImageBuffer* img =  QGetRenderContext()->pImage();

	for(int i=0; i<img->BucketSize(bucketCol, bucketRow).y(); i++)
	{
		for(int j=0; j<img->BucketSize(bucketCol, bucketRow).x(); j++)
		{
			CqImagePixel* pie;
			int ix = static_cast<int>(j+img->BucketPosition(bucketCol, bucketRow).x());
			int iy = static_cast<int>(i+img->BucketPosition(bucketCol, bucketRow).y());
			currentBucket->ImageElement(ix, iy, pie);
			for(int k=0; k<pie->XSamples()*pie->YSamples(); k++)
			{
				SqSampleData sd = pie->SampleData(k);
				dump(ix, iy, k, sd);
			}
		}
	}
}

// Dump a pixel sample
void CqMPDump::dump(int x, int y, int idx, const SqSampleData& sd)
{
	short id = 2;
	TqFloat f;

	if (out==NULL)
	{
		Aqsis::log() << error << "Attempted to write to unopened mpdump file." << std::endl;
		return;
	}

	size_t len_written = fwrite((void*)&id, sizeof(short), 1, out);
	len_written += fwrite((void*)&x, sizeof(int), 1, out);
	len_written += fwrite((void*)&y, sizeof(int), 1, out);
	len_written += fwrite((void*)&idx, sizeof(int), 1, out);
	f = sd.m_Position.x();
	len_written += fwrite((void*)&f, sizeof(TqFloat), 1, out);
	f = sd.m_Position.y();
	len_written += fwrite((void*)&f, sizeof(TqFloat), 1, out);
	if(len_written != 6)
		throw(XqException("Error writing mpdump file"));
}

// Dump a micro polygon
void CqMPDump::dump(const CqMicroPolygon& mp)
{
	CqVector3D v;
	CqColor c;
	short id = 1;

	if (out==NULL)
	{
		Aqsis::log() << error << "Attempted to write to unopened mpdump file." << std::endl;
		return;
	}

	mpcount++;
	size_t len_written = fwrite((void*)&id, sizeof(short), 1, out);
	if(len_written != 1)
		throw(XqException("Error writing mpdump file"));

	v = mp.PointA();
	dumpVec3(v);
	v = mp.PointB();
	dumpVec3(v);
	v = mp.PointC();
	dumpVec3(v);
	v = mp.PointD();
	dumpVec3(v);
	if (mp.pGrid()->pVar(EnvVars_Ci)!=NULL)
		c = *mp.colColor();
	else
		c = CqColor(0.9,0.9,1);
	dumpCol(c);
	if (mp.pGrid()->pVar(EnvVars_Oi)!=NULL)
		c = *mp.colOpacity();
	else
		c = CqColor(0.9,0.9,1);
	dumpCol(c);
}

// Dump a 3d vector
void CqMPDump::dumpVec3(const CqVector3D& v)
{
	TqFloat x = v.x();
	TqFloat y = v.y();
	TqFloat z = v.z();

	size_t len_written = fwrite((void*)&x, sizeof(TqFloat), 1, out);
	len_written += fwrite((void*)&y, sizeof(TqFloat), 1, out);
	len_written += fwrite((void*)&z, sizeof(TqFloat), 1, out);
	if(len_written != 3)
		throw(XqException("Error writing mpdump file"));
}

// Dump a color
void CqMPDump::dumpCol(const CqColor& c)
{
	TqFloat r = c.fRed();
	TqFloat g = c.fGreen();
	TqFloat b = c.fBlue();

	size_t len_written = fwrite((void*)&r, sizeof(TqFloat), 1, out);
	len_written += fwrite((void*)&g, sizeof(TqFloat), 1, out);
	len_written += fwrite((void*)&b, sizeof(TqFloat), 1, out);
	if(len_written != 3)
		throw(XqException("Error writing mpdump file"));
}


/// Global dump object
CqMPDump mpdump;

END_NAMESPACE( Aqsis )

#endif

