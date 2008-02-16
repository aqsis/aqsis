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

#include "texturemap.h"

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
// Implementation of CqTextureMapWrapper

CqTextureMapWrapper::CqTextureMapWrapper(const std::string& texName)
	: m_texName(texName),
	m_realMap(IqTextureSampler::create(texName.c_str()))
{ }

void CqTextureMapWrapper::Open()
{
	// \todo implementation
}

void CqTextureMapWrapper::Close()
{
	// \todo implementation
}

void CqTextureMapWrapper::PrepareSampleOptions(std::map<std::string, IqShaderData*>& paramMap )
{
	m_sampleOpts.setBlur(0);
	m_sampleOpts.setWidth(1);
	m_sampleOpts.setFilterType(TextureFilter_Gaussian);
	m_sampleOpts.setFill(0);
	m_sampleOpts.setStartChannel(0);
	m_sampleOpts.setNumChannels(SamplesPerPixel());
	m_sampleOpts.setNumSamples(4);
	m_sampleOpts.setWrapMode(WrapMode_Periodic);
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
			m_sampleOpts.setWidth(width);
		}
		else
		{
			i = paramMap.find("swidth");
			if(i != end)
			{
				TqFloat tmp = 1;
				i->second->GetFloat(tmp);
				m_sampleOpts.setSWidth(tmp);
			}
			i = paramMap.find("twidth");
			if(i != end)
			{
				TqFloat tmp = 1;
				i->second->GetFloat(tmp);
				m_sampleOpts.setTWidth(tmp);
			}
		}
		// Load filter blurs
		i = paramMap.find("blur");
		if(i != end)
		{
			TqFloat blur = 0;
			i->second->GetFloat(blur);
			m_sampleOpts.setBlur(blur);
		}
		else
		{
			i = paramMap.find("sblur");
			if(i != end)
			{
				TqFloat tmp = 0;
				i->second->GetFloat(tmp);
				m_sampleOpts.setSBlur(tmp);
			}
			i = paramMap.find("tblur");
			if(i != end)
			{
				TqFloat tmp = 0;
				i->second->GetFloat(tmp);
				m_sampleOpts.setTBlur(tmp);
			}
		}
		// number of samples for stochastic filters
		i = paramMap.find("samples");
		if(i != end)
		{
			TqFloat samples = 0;
			i->second->GetFloat(samples);
			m_sampleOpts.setNumSamples(lfloor(samples));
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
			m_sampleOpts.setFilterType(filterType);
		}
	}
}

void CqTextureMapWrapper::SampleMap(TqFloat s1, TqFloat t1, TqFloat swidth, TqFloat twidth,
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

void CqTextureMapWrapper::SampleMap(TqFloat s1, TqFloat t1, TqFloat s2, TqFloat t2,
		TqFloat s3, TqFloat t3, TqFloat s4, TqFloat t4,
		std::valarray<TqFloat>& val )
{
	val.resize(SamplesPerPixel());
	const SqSampleQuad sQuad(CqVector2D(s1,t1), CqVector2D(s2,t2),
			CqVector2D(s3,t3), CqVector2D(s4,t4));
	m_realMap->sample(sQuad, m_sampleOpts, &val[0]);
}

inline CqMatrix& CqTextureMapWrapper::GetMatrix(TqInt which, TqInt index)
{
	// This matrix isn't incredibly meaningful for normal texture maps.
	// However some of the "stupid RAT tricks" have used it IIRC.
	static CqMatrix unused;
	return unused;
}

} // namespace Aqsis
