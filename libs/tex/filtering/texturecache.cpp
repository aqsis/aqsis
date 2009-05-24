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
 * \brief Texture cache implementation.
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#include "texturecache.h"

#include <aqsis/util/exception.h>
#include <aqsis/util/file.h>
#include <aqsis/tex/filtering/ienvironmentsampler.h>
#include <aqsis/tex/filtering/iocclusionsampler.h>
#include <aqsis/tex/filtering/ishadowsampler.h>
#include <aqsis/tex/io/itiledtexinputfile.h>
#include <aqsis/tex/filtering/itexturesampler.h>
#include <aqsis/util/logging.h>
#include <aqsis/util/sstring.h>
#include <aqsis/tex/texexception.h>

namespace Aqsis {

//------------------------------------------------------------------------------
// IqTextureCache creation function.

boost::shared_ptr<IqTextureCache> IqTextureCache::create(
		TqSearchPathCallback searchPathCallback)
{
	return boost::shared_ptr<IqTextureCache>(
			new CqTextureCache(searchPathCallback));
}

//------------------------------------------------------------------------------
// CqTextureCache

CqTextureCache::CqTextureCache(TqSearchPathCallback searchPathCallback)
	: m_textureCache(),
	m_environmentCache(),
	m_shadowCache(),
	m_occlusionCache(),
	m_texFileCache(),
	m_currToWorld(),
	m_searchPathCallback(searchPathCallback)
{ }

IqTextureSampler& CqTextureCache::findTextureSampler(const char* name)
{
	return findSampler(m_textureCache, name);
}

IqEnvironmentSampler& CqTextureCache::findEnvironmentSampler(const char* name)
{
	return findSampler(m_environmentCache, name);
}

IqShadowSampler& CqTextureCache::findShadowSampler(const char* name)
{
	return findSampler(m_shadowCache, name);
}

IqOcclusionSampler& CqTextureCache::findOcclusionSampler(const char* name)
{
	return findSampler(m_occlusionCache, name);
}

void CqTextureCache::flush()
{
	m_textureCache.clear();
	m_environmentCache.clear();
	m_shadowCache.clear();
	m_occlusionCache.clear();
	m_texFileCache.clear();
}

const CqTexFileHeader* CqTextureCache::textureInfo(const char* name)
{
	boost::shared_ptr<IqTiledTexInputFile> file;
	try
	{
		file = getTextureFile(name);
		return &(file->header());
	}
	catch(XqInvalidFile& /*e*/)
	{
		return 0;
	}
}

void CqTextureCache::setCurrToWorldMatrix(const CqMatrix& currToWorld)
{
	m_currToWorld = currToWorld;
}

//--------------------------------------------------
// Private methods
template<typename SamplerT>
SamplerT& CqTextureCache::findSampler(
		std::map<TqUlong, boost::shared_ptr<SamplerT> >& samplerMap,
		const char* name)
{
	TqUlong hash = CqString::hash(name);
	typename std::map<TqUlong, boost::shared_ptr<SamplerT> >::const_iterator
		texIter = samplerMap.find(hash);
	if(texIter != samplerMap.end())
	{
		// The desired texture sampler is already created - return it.
		return *(texIter->second);
	}
	else
	{
		// Couldn't find in the currently open texture samplers - create a new
		// instance.
		boost::shared_ptr<SamplerT> newTex;
		try
		{
			// Find the file in the current file cache.
			newTex = newSamplerFromFile<SamplerT>(getTextureFile(name));
		}
		catch(XqInvalidFile& e)
		{
			Aqsis::log() << error
				<< "Invalid texture file - " << e.what() << "\n";
			newTex = SamplerT::createDummy();
		}
		catch(XqBadTexture& e)
		{
			Aqsis::log() << error
				<< "Bad texture file - " << e.what() << "\n";
			newTex = SamplerT::createDummy();
		}
		samplerMap[CqString::hash(name)] = newTex;
		return *newTex;
	}
}

boost::shared_ptr<IqTiledTexInputFile> CqTextureCache::getTextureFile(
		const char* name)
{
	TqUlong hash = CqString::hash(name);
	std::map<TqUlong, boost::shared_ptr<IqTiledTexInputFile> >::const_iterator
		fileIter = m_texFileCache.find(hash);
	if(fileIter != m_texFileCache.end())
		// File exists in the cache; return it.
		return fileIter->second;
	// Else try to open the file and store it in the cache before returning it.
	boostfs::path fullName = findFile(name, m_searchPathCallback());
	boost::shared_ptr<IqTiledTexInputFile> file;
	try
	{
		file = IqTiledTexInputFile::open(fullName);
	}
	catch(XqBadTexture& e)
	{
		file = IqTiledTexInputFile::openAny(fullName);
		/// \todo Make sure this warning doesn't apply to files used only for
		/// the textureInfo() function...
		Aqsis::log() << warning << "Could not open file as a tiled texture: "
			<< e.what() << ".  Rendering will continue, but may be slower.\n";
	}
	m_texFileCache[hash] = file;
	return file;
}

template<typename SamplerT>
boost::shared_ptr<SamplerT> CqTextureCache::newSamplerFromFile(
		const boost::shared_ptr<IqTiledTexInputFile>& file)
{
	return SamplerT::create(file);
}

// Special case of newSamplerFromFile() for shadow and occlusion maps - they
// need access to the camera->world transformation matrix.
template<>
boost::shared_ptr<IqShadowSampler>
CqTextureCache::newSamplerFromFile(const boost::shared_ptr<IqTiledTexInputFile>& file)
{
	return IqShadowSampler::create(file, m_currToWorld);
}
template<>
boost::shared_ptr<IqOcclusionSampler>
CqTextureCache::newSamplerFromFile(const boost::shared_ptr<IqTiledTexInputFile>& file)
{
	return IqOcclusionSampler::create(file, m_currToWorld);
}

} // namespace Aqsis
