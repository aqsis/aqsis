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


#include "mpdump.h"
#include "micropolygon.h"
#include "bucketprocessor.h"

namespace Aqsis {

// Constructor
CqMPDump::CqMPDump()
	: m_outFile(NULL),
	m_mpcount(0)
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
	m_mpcount = 0;
	m_outFile = fopen(filename, "wb");
	if (m_outFile!=NULL)
	{
		Aqsis::log() << info << "Creating '" << filename << "'" << std::endl;
		size_t len_written = fwrite((void*)&sf, sizeof(int), 1, m_outFile);
		if(len_written != 1)
			AQSIS_THROW_XQERROR(XqInvalidFile, EqE_System,
				"Error writing mpdump file");
	}
	else
		Aqsis::log() << error << "Could not create '" << filename << "'" << std::endl;
}

// Close the dump file
void CqMPDump::close()
{
	if (m_outFile!=NULL)
	{
		fclose(m_outFile);
		m_outFile=NULL;
		Aqsis::log() << info << m_mpcount << " micro polygons dumped" << std::endl;
	}
}

// Dump global information about the image
void CqMPDump::dumpImageInfo()
{
	short id = 3;

	if (m_outFile==NULL)
	{
		Aqsis::log() << error << "Attempted to write to unopened mpdump file." << std::endl;
		return;
	}

	int width = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "System", "Resolution" ) [ 0 ];
	int height = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "System", "Resolution" ) [ 1 ];
	size_t len_written = fwrite((void*)&id, sizeof(short), 1, m_outFile);
	len_written += fwrite((void*)&width, sizeof(int), 1, m_outFile);
	len_written += fwrite((void*)&height, sizeof(int), 1, m_outFile);
	if(len_written != 3)
		AQSIS_THROW_XQERROR(XqInvalidFile, EqE_System,
				"Error writing mpdump file");
}

// Dump all pixel samples of the current bucket
void CqMPDump::dumpPixelSamples(const CqBucketProcessor& bp)
{
	const std::vector<CqImagePixelPtr>& pixels = bp.pixels();
	for(std::vector<CqImagePixelPtr>::const_iterator p = pixels.begin(),
			e = pixels.end(); p != e; ++p)
	{
		const CqImagePixel& pixel = **p;
		for(int i = 0, numSamples = pixel.numSamples(); i < numSamples; ++i)
		{
			CqVector2D pos = pixel.SampleData(i).position;
			if(!(  pos.x() <= bp.SampleRegion().xMin()
				|| pos.x() > bp.SampleRegion().xMax()
				|| pos.y() <= bp.SampleRegion().yMin()
				|| pos.y() > bp.SampleRegion().yMax() ) )
			{
				// Only dump samples which are inside bp.SampleRegion()
				// this means that only samples which are actually computed for
				// the current bucket will be considered.
				dump(lfloor(pos.x()), lfloor(pos.y()), i, pos);
			}
		}
	}
}

// Dump a pixel sample
void CqMPDump::dump(int x, int y, int idx, const CqVector2D& pos)
{
	short id = 2;
	TqFloat f;

	if (m_outFile==NULL)
	{
		Aqsis::log() << error << "Attempted to write to unopened mpdump file." << std::endl;
		return;
	}

	size_t len_written = fwrite((void*)&id, sizeof(short), 1, m_outFile);
	len_written += fwrite((void*)&x, sizeof(int), 1, m_outFile);
	len_written += fwrite((void*)&y, sizeof(int), 1, m_outFile);
	len_written += fwrite((void*)&idx, sizeof(int), 1, m_outFile);
	f = pos.x();
	len_written += fwrite((void*)&f, sizeof(TqFloat), 1, m_outFile);
	f = pos.y();
	len_written += fwrite((void*)&f, sizeof(TqFloat), 1, m_outFile);
	if(len_written != 6)
		AQSIS_THROW_XQERROR(XqInvalidFile, EqE_System,
				"Error writing mpdump file");
}

// Dump a micro polygon
void CqMPDump::dump(const CqMicroPolygon& mp)
{
	CqColor c;
	short id = 1;

	if (m_outFile==NULL)
	{
		Aqsis::log() << error << "Attempted to write to unopened mpdump file." << std::endl;
		return;
	}

	m_mpcount++;
	size_t len_written = fwrite((void*)&id, sizeof(short), 1, m_outFile);
	if(len_written != 1)
		AQSIS_THROW_XQERROR(XqInvalidFile, EqE_System,
				"Error writing mpdump file");

	// Dump vertices in a funny circular order for backward-compatibility
	// rather than in the usual bilinear patch type order.
	CqVector3D P[4];
	mp.GetVertices(P);
	dumpVec3(P[0]);
	dumpVec3(P[1]);
	dumpVec3(P[3]);
	dumpVec3(P[2]);
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

	size_t len_written = fwrite((void*)&x, sizeof(TqFloat), 1, m_outFile);
	len_written += fwrite((void*)&y, sizeof(TqFloat), 1, m_outFile);
	len_written += fwrite((void*)&z, sizeof(TqFloat), 1, m_outFile);
	if(len_written != 3)
		AQSIS_THROW_XQERROR(XqInvalidFile, EqE_System,
				"Error writing mpdump file");
}

// Dump a color
void CqMPDump::dumpCol(const CqColor& c)
{
	TqFloat r = c.r();
	TqFloat g = c.g();
	TqFloat b = c.b();

	size_t len_written = fwrite((void*)&r, sizeof(TqFloat), 1, m_outFile);
	len_written += fwrite((void*)&g, sizeof(TqFloat), 1, m_outFile);
	len_written += fwrite((void*)&b, sizeof(TqFloat), 1, m_outFile);
	if(len_written != 3)
		AQSIS_THROW_XQERROR(XqInvalidFile, EqE_System,
				"Error writing mpdump file");
}


/// Global dump object
CqMPDump mpdump;

} // namespace Aqsis

