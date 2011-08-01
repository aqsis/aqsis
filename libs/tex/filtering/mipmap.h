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
 * \brief Declare a class for holding and sampling a collection of mipmap levels.
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef MIPMAP_H_INCLUDED
#define MIPMAP_H_INCLUDED

#include <aqsis/aqsis.h>

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include <aqsis/util/autobuffer.h>
#include <aqsis/util/exception.h>
#include <aqsis/tex/filtering/filtertexture.h>
#include <aqsis/tex/io/itiledtexinputfile.h>
#include <aqsis/util/logging.h>
#include <aqsis/tex/filtering/sampleaccum.h>
#include <aqsis/tex/texexception.h>
#include <aqsis/tex/filtering/texturesampleoptions.h>

namespace Aqsis
{

struct SqLevelTrans;

//------------------------------------------------------------------------------
/** \brief Mipmap level container and sampler class.
 *
 * This class uses a set of power-of-two downsampled images (a typical
 * mipmap) to make texture filtering efficient.  When filtering, a mipmap level
 * is chosen based on the extent of the filter region.  We choose the mipmap
 * level to be as small as possible, subject to the filter falling across a
 * sufficient number of pixels.
 *
 * In addition to holding and sampling a set of mipmap levels, this class also
 * caches the default sampling options, as determined from attributes of the
 * texture file.  When these aren't present, we attempt to choose sensible
 * defaults.
 */
template<typename TextureBufferT>
class CqMipmap
{
	public:
		/** \brief Construct the cache from an open file.
		 *
		 * \param file - read texture data from here.
		 */
		CqMipmap(const boost::shared_ptr<IqTiledTexInputFile>& file);
		/// Get the width of the level0 image in the mipmap.
		TqInt width0() const;
		/// Get the height of the level0 image in the mipmap.
		TqInt height0() const;
		/** \brief Get the default sample options associated with the texture file.
		 *
		 * \return The default sample options - these include options which
		 * were used when the texture was created (things like the texutre wrap
		 * mode).
		 */
		const CqTextureSampleOptions& defaultSampleOptions();
		/// Returns the underlying texture file attribute header
		const CqTexFileHeader& header() const;

		/** \brief Apply the given filter to the mipmap.
		 *
		 * This function is the main workhorse of the class.  It first decides
		 * on which mipmap level to use, by taking into account the filter
		 * width and the amount of blur.  The level is then filtered with a
		 * filter function obtained from the filter factory provided.  Further
		 * filtering on the next smaller mipmap level is then performed if
		 * requested or necessary, and the results interpolated between the
		 * levels.
		 *
		 * \param filterFactory - A factory function which can create filters
		 *            specialized to a particular scaling of the level 0 image.
		 *            CqEwaFactory is a model of the required concept with the
		 *            methods filterFactory.minorAxisWidth() and
		 *            filterFactory.createFilter().
		 * \param sampleOpts - Sample options structure.
		 * \param outSamps - Output variable - filtered samples will be placed
		 *            here.
		 */
		template<typename FilterFactoryT>
		void applyFilter(const FilterFactoryT& filterFactory,
				const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps);

	private:
		/// Initialize all mipmap levels
		void initLevels();

		/** \brief Filter the given mipmap level into a sample array.
		 *
		 * \param level - mipmap level to filter over.
		 * \param filterFactory - factory containing parameters for creating
		 *            filter weights.
		 * \param sampleOpts - sample options structure
		 * \param outSamps - destination array for filtered samples.
		 */
		template<typename FilterFactoryT>
		void filterLevel(TqInt level, const FilterFactoryT& filterFactory,
				const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const;

		/** \brief Get the buffer for a given level.
		 *
		 * \param levelNum - mipmap level to grab
		 */
		const TextureBufferT& getLevel(TqInt levelNum) const;

		/** \brief Get the basetex-relative transformation for a mipmap level.
		 */
		const SqLevelTrans& levelTrans(TqInt levelNum) const;

		/// Get the number of levels in this mipmap.
		TqInt numLevels() const;

		/// Texture file to retrieve all data from
		boost::shared_ptr<IqTiledTexInputFile> m_texFile;
		/** \brief List of samplers for mipmap levels.
		 *
		 * The pointers to these may be NULL since they are created only on
		 * demand.  Mutable so that levels can be added on demand.
		 */
		mutable std::vector<boost::shared_ptr<TextureBufferT> > m_levels;
		/// Transformation information for each level.
		std::vector<SqLevelTrans> m_levelTransforms;
		/// Width of the first mipmap level
		TqInt m_width0;
		/// Height of the first mipmap level
		TqInt m_height0;
		/// Default texture sampling options for the set of mipmap levels.
		CqTextureSampleOptions m_defaultSampleOptions;
};


//==============================================================================
// Implementation details
//==============================================================================

/** \brief Raster coordinate transformation coefficients
 *
 * Raster coordinates held by a filter need to be modified if the filter is
 * built for the level zero mipmap.  A simple scaling and translation of the
 * coordinates is sufficient.  Example transformation:
 *
 *   xNew = xScale * (xOld + xOffset)
 */
struct SqLevelTrans
{
	TqFloat xScale;
	TqFloat xOffset;
	TqFloat yScale;
	TqFloat yOffset;

	/// Default constructor: set scale factors to 1 and offsets to 0
	SqLevelTrans();
	/// Trivial constructor
	SqLevelTrans(TqFloat xScale, TqFloat xOffset,
			TqFloat yScale, TqFloat yOffset);
};


//------------------------------------------------------------------------------
// SqLevelTrans
inline SqLevelTrans::SqLevelTrans()
	: xScale(1),
	xOffset(0),
	yScale(1),
	yOffset(0)
{ }

inline SqLevelTrans::SqLevelTrans(TqFloat xScale, TqFloat xOffset,
		TqFloat yScale, TqFloat yOffset)
	: xScale(xScale),
	xOffset(xOffset),
	yScale(yScale),
	yOffset(yOffset)
{ }


//------------------------------------------------------------------------------
// CqMipmap
template<typename TextureBufferT>
CqMipmap<TextureBufferT>::CqMipmap(
			const boost::shared_ptr<IqTiledTexInputFile>& file)
	: m_texFile(file),
	m_levels(),
	m_levelTransforms(),
	m_width0(0),
	m_height0(0),
	m_defaultSampleOptions()
{
	assert(m_texFile);
	initLevels();
	m_defaultSampleOptions.fillFromFileHeader(m_texFile->header());
}

template<typename TextureBufferT>
inline TqInt CqMipmap<TextureBufferT>::width0() const
{
	return m_width0;
}

template<typename TextureBufferT>
inline TqInt CqMipmap<TextureBufferT>::height0() const
{
	return m_height0;
}

template<typename TextureBufferT>
inline const CqTextureSampleOptions& CqMipmap<TextureBufferT>::defaultSampleOptions()
{
	return m_defaultSampleOptions;
}

template<typename TextureBufferT>
const CqTexFileHeader& CqMipmap<TextureBufferT>::header() const
{
	return m_texFile->header();
}

template<typename TextureBufferT>
template<typename FilterFactoryT>
void CqMipmap<TextureBufferT>::applyFilter(const FilterFactoryT& filterFactory,
		const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps)
{
	// Select mipmap level to use.
	//
	// The minimum filter width is the minimum number of pixels over which the
	// shortest length scale of the filter should extend.
	TqFloat minFilterWidth = sampleOpts.minWidth();
	// Blur ratio ranges from 0 at no blur to 1 for a "lot" of blur.
	TqFloat blurRatio = 0;
	if(sampleOpts.lerp() == Lerp_Auto && (sampleOpts.sBlur() != 0 || sampleOpts.tBlur() != 0))
	{
		// When using blur, the minimum filter width needs to be increased.
		//
		// Experiments show that for large blur factors minFilterWidth should
		// be about 4 for good results.
		TqFloat maxBlur = max(sampleOpts.sBlur()*m_width0,
				sampleOpts.tBlur()*m_height0);
		// To estimate how much to increase the blur, we take the ratio of the
		// the blur to the computed width of the minor axis of the filter.
		// This should be near 0 for blur which doesn't effect the filtering
		// much, and a asymptote to a positive constant when the blur is the
		// dominant factor.
		blurRatio = clamp(2*maxBlur/filterFactory.minorAxisWidth(), 0.0f, 1.0f);
		minFilterWidth += 2*blurRatio;
	}
	TqFloat levelCts = log2(filterFactory.minorAxisWidth()/minFilterWidth);
	TqInt level = clamp<TqInt>(lfloor(levelCts), 0, numLevels()-1);

	filterLevel(level, filterFactory, sampleOpts, outSamps);

	// Sometimes we might want to interpolate between the filtered result
	// already computed above and the next lower mipmap level.  We do that now
	// if necessary.
	if( ( sampleOpts.lerp() == Lerp_Always
		|| (sampleOpts.lerp() == Lerp_Auto && blurRatio > 0.2) )
		&& level < numLevels()-1 && levelCts > 0)
	{
		// Use interpolation between the results of filtering on two different
		// mipmap levels.  This should only be necessary if using filter blur,
		// however the user can also turn it on explicitly using the "lerp"
		// option.
		//
		// Experiments with large amounts of blurring show that some form of
		// interpolation near level transitions is necessary to ensure that
		// they're smooth and invisible.
		//
		// Such interpolation is mainly necessary when large regions of the
		// output image arise from filtering over a small part of a high mipmap
		// level - something which only occurs with artifically large filter
		// widths such as those arising from lots of blur.
		//
		// Since this extra interpolation isn't really needed for small amounts
		// of blur, we only do the interpolation when the blur ratio is large
		// enough to make it worthwhile.

		// Filter second level into tmpSamps.
		CqAutoBuffer<TqFloat, 16> tmpSamps(sampleOpts.numChannels());
		filterLevel(level+1, filterFactory, sampleOpts, tmpSamps.get());

		// Mix outSamps and tmpSamps.
		TqFloat levelInterp = levelCts - level;
		// We square levelInterp here in order to bias the interpolation toward
		// the higher resolution mipmap level, since the filtered result on the
		// higher level is more accurate.
		levelInterp *= levelInterp;
		for(TqInt i = 0; i < sampleOpts.numChannels(); ++i)
			outSamps[i] = (1-levelInterp) * outSamps[i] + levelInterp*tmpSamps[i];
	}
	// Debug - colourise mipmap level selection.
	// outSamps[level%sampleOpts.numCahnnels()] += 0.1;
}

template<typename TextureBufferT>
const TextureBufferT& CqMipmap<TextureBufferT>::getLevel(TqInt levelNum) const
{
	assert(levelNum < static_cast<TqInt>(m_levels.size()));
	assert(levelNum >= 0);
	if(!m_levels[levelNum])
	{
		// read in requested level if it's not loaded yet.
		m_levels[levelNum].reset(new TextureBufferT(m_texFile, levelNum));
		Aqsis::log() << debug << "initialized subtexture " << levelNum
			<< " [" << m_levels[levelNum]->width() << "x"
			<< m_levels[levelNum]->width() << "] " 
			<< "from texture " << m_texFile->fileName() << "\n";
	}
	return *m_levels[levelNum];
}

template<typename TextureBufferT>
inline const SqLevelTrans& CqMipmap<TextureBufferT>::levelTrans(
		TqInt levelNum) const
{
	assert(levelNum < static_cast<TqInt>(m_levelTransforms.size()));
	assert(levelNum >= 0);
	return m_levelTransforms[levelNum];
}

template<typename TextureBufferT>
inline TqInt CqMipmap<TextureBufferT>::numLevels() const
{
	return m_levels.size();
}

template<typename TextureBufferT>
void CqMipmap<TextureBufferT>::initLevels()
{
	TqInt numLevels = m_texFile->numSubImages();
	m_levels.resize(numLevels);
	m_levelTransforms.reserve(m_texFile->numSubImages());
	m_levelTransforms.push_back(SqLevelTrans());
	// Calculated level sizes (init with base texture dimensions.)
	TqInt levelWidth = m_texFile->width(0);
	TqInt levelHeight = m_texFile->height(0);
	m_width0 = levelWidth;
	m_height0 = levelHeight;
	// level offsets (in base-texture raster coordinates)
	TqFloat xOffset = 0;
	TqFloat yOffset = 0;
	for(TqInt i = 1; i < numLevels; ++i)
	{
		if(levelWidth == 1 && levelHeight == 1)
		{
			m_levels.resize(i);
			break;
		}
		// Update offsets for the current level.
		if(levelWidth % 2 == 0)
		{
			// Previous level has an even width; add mipmap offset
			xOffset += 0.5*(1 << (i-1));
		}
		if(levelHeight % 2 == 0)
		{
			// Previous level has an even height; add mipmap offset
			yOffset += 0.5*(1 << (i-1));
		}
		// compute expected level dimensions
		levelWidth = max((levelWidth+1)/2, 1);
		levelHeight = max((levelHeight+1)/2, 1);
		// check expected dimensions against actual dimensions.
		if(levelWidth != m_texFile->width(i) || levelHeight != m_texFile->height(i))
		{
			AQSIS_THROW_XQERROR(XqBadTexture, EqE_BadFile,
				"Mipmap level has incorrect size");
		}
		// set up scaling and offset transformation for this level.
		TqFloat levelScale = 1.0/(1 << i);
		m_levelTransforms.push_back( SqLevelTrans(
				levelScale, -xOffset,
				levelScale, -yOffset) );
	}
	// Check that we have the expected number of mipmap levels.
	if(levelWidth != 1 || levelHeight != 1)
	{
		Aqsis::log() << warning << "Texture \"" << m_texFile->fileName() << "\" "
			<< "has less than the expected number of mipmap levels. "
			<< "(smallest level: " << levelWidth << "x" << levelHeight << ")\n";
	}
}

template<typename TextureBufferT>
template<typename FilterFactoryT>
void CqMipmap<TextureBufferT>::filterLevel(
		TqInt level, const FilterFactoryT& filterFactory,
		const CqTextureSampleOptions& sampleOpts, TqFloat* outSamps) const
{
	// Create filter weights for chosen level.
	const SqLevelTrans& trans = levelTrans(level);
	CqEwaFilter weights = filterFactory.createFilter(
		trans.xScale, trans.xOffset,
		trans.yScale, trans.yOffset
	);
	// Create an accumulator for the samples.
	CqSampleAccum<CqEwaFilter> accumulator(
		weights,
		sampleOpts.startChannel(),
		sampleOpts.numChannels(),
		outSamps,
		sampleOpts.fill()
	);
	SqFilterSupport support = weights.support();
	if(level == numLevels() - 1)
	{
		// Truncate the support to a maximum size of 20x20 if we're on the
		// highest mipmap level.  If we don't do this, the support can
		// occasionally be very large, resulting in very long filter times.
		TqInt cx = (support.sx.start + support.sx.end)/2;
		TqInt cy = (support.sy.start + support.sy.end)/2;
		support = intersect(support, SqFilterSupport(cx-10, cx+11, cy-10, cy+11));
	}
	// filter the texture
	filterTexture(
		accumulator,
		getLevel(level),
		support,
		SqWrapModes(sampleOpts.sWrapMode(), sampleOpts.tWrapMode())
	);
}

} // namespace Aqsis

#endif // MIPMAP_H_INCLUDED
