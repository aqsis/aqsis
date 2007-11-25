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

#include "ishaderdata.h"  /// \todo remove when removing the wrapper.
#include "logging.h"

namespace Aqsis
{

//------------------------------------------------------------------------------
// Implementation of CqTextureMap2

CqTextureMap2::CqTextureMap2(const std::string& fileName)
	: m_fileName(fileName),
	m_mipLevels(),
	m_defaultSampleOptions()
{
	// \todo decide how many mipmap levels are needed.
	m_mipLevels.resize(1);
	m_mipLevels[0] = IqTextureSampler::create(
			boost::shared_ptr<IqTiledTexInputFile>());
	// \todo: Init default sampling opts from texture file
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
	m_realMap(fileName),
	m_sampleOptions()
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
	m_sampleOptions = SqTextureSampleOptions(0,0, 1,1, TextureFilter_Gaussian,
			0, 0, SamplesPerPixel(), 4, WrapMode_Black, WrapMode_Black);
	if(paramMap.size() != 0)
	{
		std::map<std::string, IqShaderData*>::const_iterator i;
		std::map<std::string, IqShaderData*>::const_iterator end = paramMap.end();
		// Load filter widths
		i = paramMap.find("width");
		if(i != end)
		{
			TqFloat width = 1;
			i->second->GetFloat(width);
			m_sampleOptions.swidth = width;
			m_sampleOptions.twidth = width;
		}
		else
		{
			i = paramMap.find("swidth");
			if(i != end)
				i->second->GetFloat(m_sampleOptions.swidth);
			i = paramMap.find("twidth");
			if(i != end)
				i->second->GetFloat(m_sampleOptions.twidth);
		}
		// Load filter blurs
		i = paramMap.find("blur");
		if(i != end)
		{
			TqFloat blur = 0;
			i->second->GetFloat(blur);
			m_sampleOptions.sblur = blur;
			m_sampleOptions.tblur = blur;
		}
		else
		{
			i = paramMap.find("sblur");
			if(i != end)
				i->second->GetFloat(m_sampleOptions.sblur);
			i = paramMap.find("tblur");
			if(i != end)
				i->second->GetFloat(m_sampleOptions.tblur);
		}
		// number of samples for stochastic filters
		i = paramMap.find("samples");
		if(i != end)
		{
			TqFloat samples = 0;
			i->second->GetFloat(samples);
			m_sampleOptions.numSamples = lfloor(samples);
		}
		/// \todo pixelvariance ?
		// Filter type
		i = paramMap.find("filter");
		if(i != end)
		{
			CqString filterName;
			i->second->GetString(filterName);
			if(filterName == "box")
				m_sampleOptions.filterType = TextureFilter_Box;
			else if(filterName == "none")
				m_sampleOptions.filterType = TextureFilter_None;
			else
				m_sampleOptions.filterType = TextureFilter_Gaussian;
		}
	}
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
	m_realMap.sampleMap(sQuad, m_sampleOptions, &val[0]);
}

inline CqMatrix& CqTextureMap2Wrapper::GetMatrix(TqInt which, TqInt index)
{
	// This matrix isn't incredibly meaningful for normal texture maps.
	// However some of the "stupid RAT tricks" have used it IIRC.
	static CqMatrix unused;
	return unused;
}

} // namespace Aqsis
