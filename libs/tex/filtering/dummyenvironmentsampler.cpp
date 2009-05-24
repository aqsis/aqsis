// Aqsis
// Copyright (C) 1997 - 2007, Paul C. Gregory
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
 *
 * \brief Dummy environment sampler for missing files - implementation.
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#include "dummyenvironmentsampler.h"

namespace Aqsis {

namespace {

// return one when inside an "x"-shaped glyph, zero otherwise.
//
// The x is underlined to break the symmetry, ensuring the orientation is
// clear.
TqFloat xGlyph(TqFloat x, TqFloat y)
{
	TqFloat x1 = 0.8191520*x + 0.5735764*y;
	TqFloat y1 = -0.8191520*x + 0.5735764*y;

	// x
	if( std::fabs(x) < 0.2 && std::fabs(y) < 0.2
		&& (
			std::fabs(y1) < 0.035 || std::fabs(x1) < 0.035
			)
		)
		return 1;
	// underline
	else if( std::fabs(x) < 0.19 && std::fabs(y+0.27) < 0.02 )
		return 1;
	return 0;
}

// return one when inside a "y"-shaped glyph, zero otherwise.
TqFloat yGlyph(TqFloat x, TqFloat y)
{
	if( std::fabs(x) < 0.15 && std::fabs(y) < 0.25
		&& (
			(std::fabs(x) > 0.08 || y < 0.035)
			&&
			(x > 0.08 || y < -0.18 || y > -0.035)
			)
		)
		return 1;
	return 0;
}

// return one when inside a "z"-shaped glyph, zero otherwise.
TqFloat zGlyph(TqFloat x, TqFloat y)
{
	TqFloat x1 = 0.707106*(x - y);
	if( std::fabs(x) < 0.15 && std::fabs(y) < 0.2
		&& (
			std::fabs(y) > 0.14
			|| std::fabs(x1) < 0.035
			)
		)
		return 1;
	return 0;
}

// return one when inside a "+"-shaped glyph, zero otherwise.
TqFloat plusGlyph(TqFloat x, TqFloat y)
{
	if(std::fabs(x) < 0.02 && std::fabs(y) < 0.1
			|| std::fabs(x) < 0.1 && std::fabs(y) < 0.02 )
		return 1;
	return 0;
}

// return one when inside a "-"-shaped glyph, zero otherwise.
TqFloat minusGlyph(TqFloat x, TqFloat y)
{
	if(std::fabs(x) < 0.1 && std::fabs(y) < 0.02)
		return 1;
	return 0;
}

} // anon. namespace

void CqDummyEnvironmentSampler::sample(const Sq3DSamplePllgram& samplePllgram,
		const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const
{
	TqFloat cx = samplePllgram.c.x();
	TqFloat cy = samplePllgram.c.y();
	TqFloat cz = samplePllgram.c.z();
	TqFloat absCx = std::fabs(cx);
	TqFloat absCy = std::fabs(cy);
	TqFloat absCz = std::fabs(cz);

	TqFloat intens = 0;

	bool usePlus = true;
	TqFloat glyphX = 0;
	TqFloat glyphY = 0;

	// Decide which of the six faces of the cube we're pointing at and insert a
	// distinguishing glyph as appropriate.
	if(absCx >= absCy && absCx >= absCz)
	{
		// x-axis
		glyphX = -cz/cx;
		glyphY = cy/absCx;
		intens = xGlyph(glyphX, glyphY);
		usePlus = cx > 0;
	}
	else if(absCy >= absCx && absCy >= absCz)
	{
		// y-axis
		glyphX = cx/absCy;
		glyphY = -cz/cy;
		intens = yGlyph(glyphX, glyphY);
		usePlus = cy > 0;
	}
	else
	{
		// z-axis
		glyphX = cx/cz;
		glyphY = cy/absCz;
		intens = zGlyph(glyphX, glyphY);
		usePlus = cz > 0;
	}

	// Indicate the direction of the axis is by inserting + or - glyphs 
	if(usePlus)
		intens = max(intens, plusGlyph(glyphX+0.3, glyphY));
	else
		intens = max(intens, minusGlyph(glyphX+0.3, glyphY));

	// Insert borders between faces of the cube.
	if(std::fabs(glyphX) > 0.95 || std::fabs(glyphY) > 0.95)
		intens = 0.5;

	for(TqInt i = 0; i < sampleOpts.numChannels(); ++i)
		outSamps[i] = intens;
}

} // namespace Aqsis
