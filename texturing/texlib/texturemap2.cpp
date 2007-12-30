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
#include "itexturesampler.h"
#include "mipmaptexturesampler.h"

#include "ishaderdata.h"  /// \todo remove when removing the wrapper.
#include "logging.h"
#include "itexturesampler.h"
#include "itexinputfile.h"

namespace Aqsis
{

//------------------------------------------------------------------------------
// Implementation of CqTextureMap2

CqTextureMap2::CqTextureMap2(const boost::shared_ptr<IqTextureSampler>& sampler)
	: m_sampler(sampler),
	m_sampleOptions(sampler->defaultSampleOptions())
{
	/// \todo Proper error handling if null...
	assert(m_sampler);
}

TqInt CqTextureMap2::numSamples() const
{
	/// \todo implementation
	return 3;
}

void CqTextureMap2::sampleMap(const SqSampleQuad& sampleQuad, TqFloat* outSamples) const
{
	/// \todo Add timing statistics?
	m_sampler->filter(sampleQuad, m_sampleOptions, outSamples);
}


//------------------------------------------------------------------------------
// Implementation of CqTextureMap2Wrapper

CqTextureMap2Wrapper::CqTextureMap2Wrapper(const std::string& texName)
	: m_texName(texName),
	m_realMap(newWrappedTexture(texName.c_str()))
{ }

boost::shared_ptr<IqTextureSampler> CqTextureMap2Wrapper::newWrappedTexture(const char* texName)
{
	Aqsis::log() << texName << "\n";
	/*
	boost::shared_ptr<IqTexInputFile> file = IqTexInputFile::open(texName);
	return IqTextureSampler::create(file);
	*/
	/// \todo Yuck.  remove this cast!
	boost::shared_ptr<IqMultiTexInputFile> file =
		boost::dynamic_pointer_cast<IqMultiTexInputFile,IqTexInputFile>(
				IqTexInputFile::open(texName));
	boost::shared_ptr<CqLevelSamplerCache<CqTextureBuffer<TqUchar> > > levels(
			new CqLevelSamplerCache<CqTextureBuffer<TqUchar> >(file));
	boost::shared_ptr<IqTextureSampler> sampler(
			new CqMipmapTextureSampler<TqUchar>(levels));
	return sampler;
}

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
	CqTextureSampleOptions& opts = m_realMap.sampleOptions();
	opts.setBlur(0);
	opts.setWidth(1);
	opts.setFilterType(TextureFilter_Gaussian);
	opts.setFill(0);
	opts.setStartChannel(0);
	opts.setNumChannels(SamplesPerPixel());
	opts.setNumSamples(4);
	opts.setWrapMode(WrapMode_Periodic);
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
			opts.setWidth(width);
		}
		else
		{
			i = paramMap.find("swidth");
			if(i != end)
			{
				TqFloat tmp = 1;
				i->second->GetFloat(tmp);
				opts.setSWidth(tmp);
			}
			i = paramMap.find("twidth");
			if(i != end)
			{
				TqFloat tmp = 1;
				i->second->GetFloat(tmp);
				opts.setTWidth(tmp);
			}
		}
		// Load filter blurs
		i = paramMap.find("blur");
		if(i != end)
		{
			TqFloat blur = 0;
			i->second->GetFloat(blur);
			opts.setBlur(blur);
		}
		else
		{
			i = paramMap.find("sblur");
			if(i != end)
			{
				TqFloat tmp = 0;
				i->second->GetFloat(tmp);
				opts.setSBlur(tmp);
			}
			i = paramMap.find("tblur");
			if(i != end)
			{
				TqFloat tmp = 0;
				i->second->GetFloat(tmp);
				opts.setTBlur(tmp);
			}
		}
		// number of samples for stochastic filters
		i = paramMap.find("samples");
		if(i != end)
		{
			TqFloat samples = 0;
			i->second->GetFloat(samples);
			opts.setNumSamples(lfloor(samples));
		}
		/// \todo pixelvariance ?
		// Filter type
		i = paramMap.find("filter");
		if(i != end)
		{
			CqString filterName;
			i->second->GetString(filterName);
			EqTextureFilter filterType = texFilterTypeFromString(filterName.c_str());
			if(filterType == TextureFilter_Unknown)
			{
				/// \todo Do this check at shader compilation time if possible.
				Aqsis::log() << warning << "Unknown texture filter \""
					<< filterName << "\".  Using gaussian\n";
				filterType = TextureFilter_Gaussian;
			}
			opts.setFilterType(filterType);
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
	assert(0);
	SampleMap(s1-swidth/2, t1-twidth/2,  s1+swidth/2, t1-twidth/2,
			s1-swidth/2, t1+twidth/2,  s1+swidth/2, t1+twidth/2,
			val);
}

void CqTextureMap2Wrapper::SampleMap(TqFloat s1, TqFloat t1, TqFloat s2, TqFloat t2,
		TqFloat s3, TqFloat t3, TqFloat s4, TqFloat t4,
		std::valarray<TqFloat>& val )
{
	val.resize(SamplesPerPixel());
	const SqSampleQuad sQuad(s1,t1, s2,t2, s3,t3, s4,t4);
	m_realMap.sampleMap(sQuad, &val[0]);
}

inline CqMatrix& CqTextureMap2Wrapper::GetMatrix(TqInt which, TqInt index)
{
	// This matrix isn't incredibly meaningful for normal texture maps.
	// However some of the "stupid RAT tricks" have used it IIRC.
	static CqMatrix unused;
	return unused;
}

} // namespace Aqsis
