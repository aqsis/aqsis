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
 * \brief Declare a filtered texture mapping class
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#include "texturemap2.h"

#include "vector3d.h"
#include "matrix.h"
#include "texturesampler.h"

namespace Aqsis
{

//------------------------------------------------------------------------------
// Implementation of CqTextureMap2

CqTextureMap2::CqTextureMap2(const std::string& fileName)
	: m_fileName(fileName),
	m_mipLevels()
{
	// \todo decide how many mipmap levels are needed.
	m_mipLevels.resize(1);
	m_mipLevels[0] = IqTextureSampler::create(
			boost::shared_ptr<IqTiledTexInputFile>());
}

TqInt CqTextureMap2::numSamples() const
{
	/// \todo implementation
	return 1;
}

const std::string& CqTextureMap2::name() const
{
	return m_fileName;
}

void CqTextureMap2::sampleMap(const SqSampleQuad& sampleQuad,
		const SqTextureSampleOptions& sampleOpts, TqFloat* outSamples) const
{
	m_mipLevels[0]->filter(sampleQuad, sampleOpts, outSamples);
}


//------------------------------------------------------------------------------
// Implementation of CqTextureMap2Wrapper

CqTextureMap2Wrapper::CqTextureMap2Wrapper(const std::string& fileName)
	: m_fileName(),
	m_realMap(fileName)
{ }

void CqTextureMap2Wrapper::Open()
{
	// \todo implementation
}

void CqTextureMap2Wrapper::Close()
{
	// \todo implementation
}

void CqTextureMap2Wrapper::PrepareSampleOptions(std::map<std::string, IqShaderData*>& paramMap )
{
	// \todo implementation
}

void CqTextureMap2Wrapper::SampleMap(TqFloat s1, TqFloat t1, TqFloat swidth, TqFloat twidth,
		std::valarray<TqFloat>& val)
{
	// \todo implementation
	//
	// In fact, we really don't want to implement this - it's a generally
	// broken way of accessing a texture.
	//assert(0);
	SampleMap(s1-swidth/2, t1-twidth/2,  s1+swidth/2, t1-twidth/2,
			s1-swidth/2, t1+twidth/2,  s1+swidth/2, t1+twidth/2,
			val);
}

void CqTextureMap2Wrapper::SampleMap(TqFloat s1, TqFloat t1, TqFloat s2, TqFloat t2,
		TqFloat s3, TqFloat t3, TqFloat s4, TqFloat t4,
		std::valarray<TqFloat>& val )
{
	// \todo implementation
	val.resize(SamplesPerPixel());
	const SqSampleQuad sQuad(s1,t1, s2,t2, s3,t3, s4,t4);
	const SqTextureSampleOptions sampleOpts(0,0, 1,1, TextureFilter_Gaussian,
			0, 0, 1);
	m_realMap.sampleMap(sQuad, sampleOpts, &val[0]);
}

inline CqMatrix& CqTextureMap2Wrapper::GetMatrix(TqInt which, TqInt index)
{
	// This matrix isn't incredibly meaningful for normal texture maps.
	// However some of the "stupid RAT tricks" have used it IIRC.
	static CqMatrix unused;
	return unused;
}

} // namespace Aqsis
