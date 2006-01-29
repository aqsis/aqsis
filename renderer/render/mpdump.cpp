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

#include "mpdump.h"

#ifdef DEBUG_MPDUMP

START_NAMESPACE( Aqsis )


// Constructor
CqMPDump::CqMPDump()
		: out(NULL), mpcount(0)
{}
;


// Destructor
CqMPDump::~CqMPDump()
{
	close();
};

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
		std::cout<<"Creating '"<<filename<<"'"<<std::endl;
		fwrite((void*)&sf, sizeof(int), 1, out);
	}
	else
		Aqsis::log()<<"Could not create '"<<filename<<"'"<<std::endl;
}

// Close the dump file
void CqMPDump::close()
{
	if (out!=NULL)
	{
		fclose(out);
		out=NULL;
		std::cout<<mpcount<<" micro polygons dumped"<<std::endl;
	}
}

// Dump global information about the image
void CqMPDump::dumpImageInfo()
{
	short id = 3;

	if (out==NULL)
	{
		Aqsis::log()<<"No dump file opened!"<<std::endl;
		return;
	}

	int width = QGetRenderContext()->pImage()->iXRes();
	int height = QGetRenderContext()->pImage()->iYRes();
	fwrite((void*)&id, sizeof(short), 1, out);
	fwrite((void*)&width, sizeof(int), 1, out);
	fwrite((void*)&height, sizeof(int), 1, out);
}

// Dump all pixel samples of the current bucket
void CqMPDump::dumpPixelSamples()
{
	CqImageBuffer* img =  QGetRenderContext()->pImage();

	for(int i=0; i<img->BucketSize().y(); i++)
	{
		for(int j=0; j<img->BucketSize().x(); j++)
		{
			CqImagePixel* pie;
			int ix = j+img->BucketPosition().x();
			int iy = i+img->BucketPosition().y();
			CqBucket::ImageElement(ix, iy, pie);
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
		Aqsis::log()<<"No dump file opened!"<<std::endl;
		return;
	}

	fwrite((void*)&id, sizeof(short), 1, out);
	fwrite((void*)&x, sizeof(int), 1, out);
	fwrite((void*)&y, sizeof(int), 1, out);
	fwrite((void*)&idx, sizeof(int), 1, out);
	f = sd.m_Position.x();
	fwrite((void*)&f, sizeof(TqFloat), 1, out);
	f = sd.m_Position.y();
	fwrite((void*)&f, sizeof(TqFloat), 1, out);
}

// Dump a micro polygon
void CqMPDump::dump(const CqMicroPolygon& mp)
{
	CqVector3D v;
	CqColor c;
	short id = 1;

	if (out==NULL)
	{
		Aqsis::log()<<"No dump file opened!"<<std::endl;
		return;
	}

	mpcount++;
	fwrite((void*)&id, sizeof(short), 1, out);

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

	fwrite((void*)&x, sizeof(TqFloat), 1, out);
	fwrite((void*)&y, sizeof(TqFloat), 1, out);
	fwrite((void*)&z, sizeof(TqFloat), 1, out);
}

// Dump a color
void CqMPDump::dumpCol(const CqColor& c)
{
	TqFloat r = c.fRed();
	TqFloat g = c.fGreen();
	TqFloat b = c.fBlue();

	fwrite((void*)&r, sizeof(TqFloat), 1, out);
	fwrite((void*)&g, sizeof(TqFloat), 1, out);
	fwrite((void*)&b, sizeof(TqFloat), 1, out);
}


/// Global dump object
CqMPDump mpdump;

END_NAMESPACE( Aqsis )

#endif

