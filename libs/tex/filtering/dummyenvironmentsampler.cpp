// Aqsis
// Copyright (C) 2001, Paul C. Gregory and the other authors and contributors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the software's owners nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// (This is the New BSD license)

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
