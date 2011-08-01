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

/**
 * \file
 *
 * \brief Declare array class which holds data as tiles.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 */

#ifndef TILEARRAY_H_INCLUDED
#define TILEARRAY_H_INCLUDED

#include <aqsis/aqsis.h>

#include <vector>

#include <boost/intrusive_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <boost/noncopyable.hpp>

//#include <aqsis/util/memorysentry.h>
#include <aqsis/tex/io/itiledtexinputfile.h>
#include <aqsis/tex/buffers/texturebuffer.h>
#include "randomtable.h"
#include <aqsis/util/smartptr.h>

namespace Aqsis {

/** \class PixelIteratorConcept
 *
 * FIXME Docs
 */

/** \class FilterableArrayConcept
 *
 * FIXME Docs
 */

/** \class StochasticFilterableArrayConcept
 *
 * FIXME Docs
 */

template<typename>
class CqTextureTile;

//------------------------------------------------------------------------------
/** \brief 2D array interface holding tiled texture data, model of
 * FilterableArrayConcept
 *
 * This interface holds a texture as an array of tiles, which in turn hold the
 * underlying pixels.  The main part of the interface is an efficient pixel
 * iterator mechanism for traversing all pixels within a given region.  This
 * allows for efficient filtering to be performed over the texture, without
 * worrying about the underlying tiled structure.
 */
template<typename T>
class CqTileArray : boost::noncopyable //, public CqMemoryMonitored
{
	private:
		typedef CqTextureTile<CqTextureBuffer<T> > TqTile;
	public:
		class CqIterator;
		class CqStochasticIterator;

		typedef CqIterator TqIterator;
		typedef CqStochasticIterator TqStochasticIterator;
		typedef typename TqTile::TqSampleVector TqSampleVector;

		/** \brief Construct a tiled texture array connected to a file
		 *
		 * \brief inFile - File to take tile data from
		 * \brief subImageIdx - Index of the subimage in "inFile" which the
		 *                      data should be read from.
		 */
		CqTileArray(const boost::shared_ptr<IqTiledTexInputFile>& inFile,
				TqInt subImageIdx);

		//--------------------------------------------------
		/// \name Access to buffer dimensions & metadata
		//@{
		/// Get array width
		TqInt width() const;
		/// Get array height
		TqInt height() const;
		/// Get the number of samples per pixel
		TqInt numChannels() const;
		//@}

		//--------------------------------------------------
		/// \name Pixel access
		//@{
		/** \brief 2D Indexing operator - floating point pixel interface.
		 *
		 * The returned vector is a lightweight view onto the underlying pixel,
		 * which presents the data as floating point values.
		 *
		 * Note that this function is not be very efficient, since the correct
		 * tile has to be deduced for each invocation, which involves two
		 * integer divisions.
		 *
		 * \param x - pixel index in width direction (column index)
		 * \param y - pixel index in height direction (row index)
		 * \return a lightweight vector holding a reference to the channels data
		 */
		const TqSampleVector operator()(const TqInt x, const TqInt y) const;
		/** \brief Access to pixels through a pixel iterator
		 *
		 * The pixel iterator will iterate through all the pixels the provided
		 * support.
		 *
		 * \param support - support to iterate over
		 */
		TqIterator begin(const SqFilterSupport& support) const;
		/** \brief Stochastic iterator access to pixels in the given support.
		 *
		 * A stochastic support iterator aims to choose a fixed number of
		 * samples from the support randomly.  The randomness choices should
		 * ideally be spread evenly over the support with minimal bunching.
		 *
		 * \param support - support to iterate over
		 * \param numSamples - number of samples to choose in the support.
		 */
		TqStochasticIterator beginStochastic(const SqFilterSupport& support,
				TqInt numSamples) const;
		//@}
	private:
		/** \brief Access to the underlying tiles
		 *
		 * \return The tile holding the underlying data at the given indices.
		 */
		boost::intrusive_ptr<TqTile> getTile(const TqInt x, const TqInt y) const;

		/// Underlying texture file.
		boost::shared_ptr<IqTiledTexInputFile> m_inFile;
		/// Index for which subimage in the input file the pixel data comes from
		TqInt m_subImageIdx;
		/// Width of the array
		TqInt m_width;
		/// Height of the array
		TqInt m_height;
		/// Number of channels per pixel
		TqInt m_numChannels;
		/// Width of the tiles making up the array
		TqInt m_tileWidth;
		/// Height of tiles making up the the array
		TqInt m_tileHeight;
		/// Width of the array
		TqInt m_widthInTiles;
		/// Height of the array
		TqInt m_heightInTiles;
		/// "2D" array of tiles.  Tiles may be founnd in O(1) time using this array.
		boost::scoped_array<boost::intrusive_ptr<TqTile> > m_tiles;
};


//------------------------------------------------------------------------------
/** \brief Pixel iterator for data held by CqTileArray, models PixelIteratorConcept.
 *
 * This iterator class encapsulates an efficient means for iterating over
 * pixels held in a CqTileArray.  A provided filter support for the region to
 * be iterated over is first decomposed into non-overlapping pieces.  Each
 * piece covers the required part of exactly one underlying tile, and the
 * iterators for these tiles are then used to traverse each tile in turn.
 */
template<typename T>
class CqTileArray<T>::CqIterator
{
	public:
		/// Move to the next pixel in the support.
		CqIterator& operator++();

		/// Check whether the iterator still lies inside the support region.
		bool inSupport() const;

		/// x-coordinate of the currently referenced pixel
		TqInt x() const;
		/// y-coordinate of the currently referenced pixel
		TqInt y() const;
		/// Return a vector of the current pixel channels.
		const typename TqTile::TqSampleVector operator*();

	private:
		/** \brief Construct a pixel iterator with the given underlying array
		 * and support region.
		 *
		 * (Private constructor, since we only want CqTileArray to be able to
		 * construct pixel iterators.)
		 *
		 * \param tileArray - array to obtain tiles from
		 * \param support - region to iterate over
		 */
		CqIterator(const CqTileArray<T>& tileArray, const SqFilterSupport& support);

		/// Advance to the next tile in the support.
		void nextTile();

		/// Iterator type for the underlying tiles
		typedef typename TqTile::TqIterator TqBaseIter;

		/// Support region to iterate over.
		SqFilterSupport m_support;
		/// Parent array to obtain tiles from.
		const CqTileArray<T>* m_tileArray;
		/// Starting x-coordinate (in tile coordinates with 0 being the top-left)
		const TqInt m_tileX0;
		/// One greater than the last valid tile x-coordinate
		const TqInt m_tileXEnd;
		/// One greater than the last valid tile y-coordinate
		const TqInt m_tileYEnd;
		/// Current tile x-coordinate
		TqInt m_tileX;
		/// Current tile y-coordinate
		TqInt m_tileY;

		/// Current position in the underlying tiles.
		TqBaseIter m_currPos;

		friend class CqTileArray<T>;
};


//------------------------------------------------------------------------------
/** \brief Stochastic pixel iterator for data held by CqTileArray, models
 * StochasticPixelIteratorConcept.
 *
 * This iterator class encapsulates an efficient means for iterating over
 * pixels held in a CqTileArray.  A given filter support for the region to be
 * iterated over is first decomposed into non-overlapping pieces.  Each piece
 * covers the required part of exactly one underlying tile, and stochastic
 * iterators for these tiles are then used to traverse each tile in turn.
 *
 * The total number of samples is divided between the tiles to maximize
 * stratification; every tile gets a number of samples which is proportional to
 * the filter area which overlaps it.
 */
template<typename T>
class CqTileArray<T>::CqStochasticIterator
{
	public:
		/// Move to the next pixel in the support.
		CqStochasticIterator& operator++();

		/// Check whether the iterator still lies inside the support region.
		bool inSupport() const;

		/// x-coordinate of the currently referenced pixel
		TqInt x() const;
		/// y-coordinate of the currently referenced pixel
		TqInt y() const;
		/// Return a vector of the current pixel channels.
		const typename TqTile::TqSampleVector operator*();

	private:
		/** \brief Construct a pixel iterator with the given underlying array
		 * and support region.
		 *
		 * (Private constructor, since we only want CqTileArray to be able to
		 * construct this.)
		 *
		 * \param tileArray - array to obtain tiles from
		 * \param support - region to iterate over
		 */
		CqStochasticIterator(const CqTileArray<T>& tileArray,
				const SqFilterSupport& support, TqInt numSamps);

		/// Advance to the next tile in the support.
		void nextTile();

		/// Iterator type for the underlying tiles
		typedef typename TqTile::TqStochasticIterator TqBaseIter;

		/// Random number stream for partitioning samples into tiles (see nextTile)
		static CqRandom m_random;

		/// Support region to iterate over.
		SqFilterSupport m_support;
		/// Parent array to obtain tiles from.
		const CqTileArray<T>* m_tileArray;
		/// Starting x-coordinate (in tile coordinates with 0 being the top-left)
		const TqInt m_tileX0;
		/// One greater than the last valid tile x-coordinate
		const TqInt m_tileXEnd;
		/// One greater than the last valid tile y-coordinate
		const TqInt m_tileYEnd;
		/// Current tile x-coordinate
		TqInt m_tileX;
		/// Current tile y-coordinate
		TqInt m_tileY;
		/// Filter area remaining for tiles yet to be filtered over.
		TqFloat m_remainingArea;
		/// Number of samples remaining for tiles yet to be filtered over.
		TqInt m_remainingSamples;
		/// Current position in the underlying tiles.
		TqBaseIter m_currPos;

		friend class CqTileArray<T>;
};


//==============================================================================
// Implementation details
//==============================================================================
/** \brief A texture tile buffer to be held by CqTileArray.
 *
 * This class is a lightweight wrapper around an array type, ArrayT which holds
 * the actual pixels.  ArrayT should be a model of FilterableArrayConcept to
 * provide pixel iterators to iterate over the contained pixels.
 *
 * The wrapper adds two things to the underlying array:
 *   - Adjust the origin of the array to some point (x0, y0)
 *   - Facilities to enable being held by a tiled array (intrusive reference
 *     counting, and support for determining recent usage for tile cache
 *     rejection (FIXME coming soon) )
 */
template<typename ArrayT>
class CqTextureTile : public CqIntrusivePtrCounted
{
	private:
		/// Underlying array of pixels
		boost::scoped_ptr<ArrayT> m_pixels;
		/// x-coordinate of origin (top left of array)
		TqInt m_x0;
		/// y-coordinate of origin (top left of array)
		TqInt m_y0;
	public:
		/// Pixel iterator for CqTextureTile
		template<typename> class CqIterator;

		/// Iterator types
		typedef CqIterator<typename ArrayT::TqIterator> TqIterator;
		typedef CqIterator<typename ArrayT::TqStochasticIterator> TqStochasticIterator;
		/// Type of samples returned from dereferenceing the pixel iterator
		typedef typename ArrayT::TqSampleVector TqSampleVector;

		/// Construct a texture tile with the given origin (x0,y0)
		CqTextureTile(TqInt x0, TqInt y0)
			: m_pixels(new ArrayT()),
			m_x0(x0),
			m_y0(y0)
		{ }

		/// Return the underlying array holding the actual pixel data
		ArrayT& pixels()
		{
			return *m_pixels;
		}

		/** \brief 2D Indexing operator - floating point pixel interface.
		 *
		 * The returned vector is a lightweight view onto the underlying pixel,
		 * which presents the data as floating point values.
		 *
		 * \param x - pixel index in width direction (column index)
		 * \param y - pixel index in height direction (row index)
		 * \return a lightweight vector holding a reference to the channels data
		 */
		const TqSampleVector operator()(const TqInt x, const TqInt y) const
		{
			return (*m_pixels)(x-m_x0, y-m_y0);
		}
		/// Return a pixel iterator for the given support region
		TqIterator begin(const SqFilterSupport& support) const
		{
			return TqIterator( m_x0, m_y0, m_pixels->begin(
						SqFilterSupport(support.sx.start - m_x0, support.sx.end - m_x0,
						support.sy.start - m_y0, support.sy.end - m_y0)) );
		}
		/// Return a stochastic pixel iterator for the given support region
		TqStochasticIterator beginStochastic(const SqFilterSupport& support,
				TqInt numSamps) const
		{
			return TqStochasticIterator( m_x0, m_y0, m_pixels->beginStochastic(
						SqFilterSupport(support.sx.start - m_x0, support.sx.end - m_x0,
						support.sy.start - m_y0, support.sy.end - m_y0), numSamps) );
		}
};


//------------------------------------------------------------------------------
/** \brief A pixel iterator adjusting an underlying iterator's origin.
 */
template<typename ArrayT>
template<typename BaseIterT>
class CqTextureTile<ArrayT>::CqIterator
{
	private:
		friend class CqTextureTile<ArrayT>;

		/** \brief Construct a pixel iterator with offset origin.
		 *
		 * \param x0 - x-coordinate of origin for offset
		 * \param y0 - y-coordinate of origin for offset
		 * \param baseIter - iterator for base tile.
		 */
		CqIterator(const TqInt x0, const TqInt y0, const BaseIterT& baseIter)
			: m_currPos(baseIter),
			m_x0(x0),
			m_y0(y0)
		{ }

		/// Current position in underlying buffer
		BaseIterT m_currPos;
		/// x-coordinate of origin for offset
		TqInt m_x0;
		/// y-coordinate of origin for offset
		TqInt m_y0;
	public:
		/// Construct a null iterator (derferencing the result is an error)
		CqIterator()
			: m_currPos(),
			m_x0(0),
			m_y0(0)
		{ }
		/// Move to the next pixel position
		CqIterator& operator++()
		{
			++m_currPos;
			return *this;
		}

		/// Check whether the iterator is still in the desired region
		bool inSupport() const
		{
			return m_currPos.inSupport();
		}

		/// x-coordinate of the currently referenced pixel
		TqInt x() const
		{
			return m_currPos.x() + m_x0;
		}
		/// y-coordinate of the currently referenced pixel
		TqInt y() const
		{
			return m_currPos.y() + m_y0;
		}
		/// Return a vector of the current pixel channels.
		const typename CqTextureTile<ArrayT>::TqSampleVector operator*()
		{
			return *m_currPos;
		}
};


//------------------------------------------------------------------------------
// CqTileArray Implementation
template<typename T>
CqTileArray<T>::CqTileArray(const boost::shared_ptr<IqTiledTexInputFile>& inFile,
				TqInt subImageIdx)
	: m_inFile(inFile),
	m_subImageIdx(subImageIdx),
	m_width(inFile->width(subImageIdx)),
	m_height(inFile->height(subImageIdx)),
	m_numChannels(inFile->header().channelList().numChannels()),
	m_tileWidth(inFile->tileInfo().width),
	m_tileHeight(inFile->tileInfo().height),
	m_widthInTiles((m_width-1)/m_tileWidth + 1), // "ceil(m_width/m_tileWidth)"
	m_heightInTiles((m_height-1)/m_tileHeight + 1),
	m_tiles(new boost::intrusive_ptr<TqTile>[m_widthInTiles*m_heightInTiles])
{ }

template<typename T>
inline TqInt CqTileArray<T>::width() const
{
	return m_width;
}

template<typename T>
inline TqInt CqTileArray<T>::height() const
{
	return m_height;
}

template<typename T>
inline TqInt CqTileArray<T>::numChannels() const
{
	return m_numChannels;
}

template<typename T>
const typename CqTileArray<T>::TqSampleVector
CqTileArray<T>::operator()(const TqInt x, const TqInt y) const
{
	return (*getTile(x/m_tileWidth, y/m_tileHeight))(x,y);
}

template<typename T>
inline typename CqTileArray<T>::TqIterator CqTileArray<T>::begin(
		const SqFilterSupport& support) const
{
	return TqIterator(*this, intersect(support,
				SqFilterSupport(0,m_width, 0,m_height)));
}

template<typename T>
inline typename CqTileArray<T>::TqStochasticIterator CqTileArray<T>::beginStochastic(
		const SqFilterSupport& support, TqInt numSamples) const
{
	return TqStochasticIterator(*this, intersect(support,
				SqFilterSupport(0,m_width, 0,m_height)), numSamples);
}

template<typename T>
boost::intrusive_ptr<typename CqTileArray<T>::TqTile> CqTileArray<T>::getTile(
		const TqInt x, const TqInt y) const
{
	assert(x < m_widthInTiles);
	assert(y < m_heightInTiles);
	boost::intrusive_ptr<TqTile>& tilePtr = m_tiles[y*m_widthInTiles + x];
	if(!tilePtr)
	{
		tilePtr = boost::intrusive_ptr<TqTile>(
				new TqTile(x*m_tileWidth, y*m_tileHeight));
		m_inFile->readTile(tilePtr->pixels(), x, y, m_subImageIdx);
	}
	return tilePtr;
}


//------------------------------------------------------------------------------
// CqTileArray::CqIterator implementation
template<typename T>
inline typename CqTileArray<T>::CqIterator&
CqTileArray<T>::CqIterator::operator++()
{
	// Iterate over the current tile.
	++m_currPos;
	if(!m_currPos.inSupport())
		nextTile();
	return *this;
}

// Optimization note: It's important to have nextTile() as a separate function
// (at least for gcc-4.1), otherwise operator++ isn't inlined.  Failing to
// inline operator++() has somewhat severe performance implications.  Since
// nextTile doesn't get called very often it's not necessary to inline it.
template<typename T>
void CqTileArray<T>::CqIterator::nextTile()
{
	++m_tileX;
	if(m_tileX >= m_tileXEnd)
	{
		m_tileX = m_tileX0;
		++m_tileY;
	}
	if(inSupport())
	{
		// Grab the next tile as long as we're within the overall
		// filter support.
		m_currPos = m_tileArray->getTile(m_tileX,m_tileY)->begin(m_support);
	}
}

template<typename T>
inline bool CqTileArray<T>::CqIterator::inSupport() const
{
	return m_tileY < m_tileYEnd;
}

template<typename T>
inline TqInt CqTileArray<T>::CqIterator::x() const
{
	return m_currPos.x();
}

template<typename T>
inline TqInt CqTileArray<T>::CqIterator::y() const
{
	return m_currPos.y();
}

template<typename T>
const typename CqTileArray<T>::TqTile::TqSampleVector
inline CqTileArray<T>::CqIterator::operator*()
{
	return *m_currPos;
}

template<typename T>
CqTileArray<T>::CqIterator::CqIterator(const CqTileArray<T>& tileArray,
		const SqFilterSupport& support)
	: m_support(support),
	m_tileArray(&tileArray),
	m_tileX0(support.sx.start/tileArray.m_tileWidth),
	m_tileXEnd((support.sx.end-1)/tileArray.m_tileWidth + 1),
	m_tileYEnd((support.sy.end-1)/tileArray.m_tileHeight + 1),
	m_tileX(m_tileX0),
	m_tileY(support.sy.start/tileArray.m_tileHeight),
	// Check support.sx.empty() etc in order to make sure the tile
	// index is still valid when the support is outside the buffer
	m_currPos(m_tileArray->getTile(support.sx.isEmpty() ? 0 : m_tileX,
				support.sy.isEmpty() ? 0 : m_tileY)->begin(m_support))
{
	// Make sure that inSupport() works correctly when the support is empty.
	if(support.isEmpty())
		m_tileY = m_tileYEnd;
}


//------------------------------------------------------------------------------
// CqTileArray<T>::CqStochasticIterator implementation
template<typename T>
CqRandom CqTileArray<T>::CqStochasticIterator::m_random;

template<typename T>
inline typename CqTileArray<T>::CqStochasticIterator&
CqTileArray<T>::CqStochasticIterator::operator++()
{
	// Go to the next position in the underlying iterator for the current tile.
	++m_currPos;
	// When this fails, we move on to the next tile instead.
	if(!m_currPos.inSupport())
		nextTile();
	return *this;
}

// Optimization note: It's important to have nextTile() as a separate function
// from operator++ otherwise operator++ may not be inlined.  Failing to inline
// operator++() has somewhat severe performance implications.
template<typename T>
void CqTileArray<T>::CqStochasticIterator::nextTile()
{
	if(m_remainingSamples == 0)
	{
		m_tileY = m_tileYEnd;
		return;
	}
	TqInt numSamples = 0;
	while(numSamples == 0)
	{
		// Find position of next tile.
		++m_tileX;
		if(m_tileX >= m_tileXEnd)
		{
			m_tileX = m_tileX0;
			++m_tileY;
		}
		// Compute desired number of samples for the current tile.  This
		// consists of two parts:
		// 1) The tile gets a number of samples proportional to the area of the
		//    support which crosses itself, divided by the total area of
		//    the remaining filter support which is yet to be covered.
		TqInt area = intersect(m_support,
				SqFilterSupport( m_tileX*m_tileArray->m_tileWidth,
					(m_tileX+1)*m_tileArray->m_tileWidth,
					m_tileY*m_tileArray->m_tileHeight,
					(m_tileY+1)*m_tileArray->m_tileHeight)).area();
		TqFloat desiredSamples = m_remainingSamples*TqFloat(area)/m_remainingArea;
		numSamples = lfloor(desiredSamples);
		// 2) For any fractional part of the desired samples which remains, we
		//    accept an extra sample with probability proportional to the
		//    fractional part.
		// TODO: Investigate the performance impact of using RandomFloat() here.
		numSamples += m_random.RandomFloat() < desiredSamples-numSamples;
		// Note that this scheme is actually biased toward tiles which are
		// found later in the support in the case that a very small number of
		// samples is used.  This may not matter in practise...
		m_remainingArea -= area;
	}
	// Grab the underlying iterator for the next tile
	m_currPos = m_tileArray->getTile(m_tileX,m_tileY)->beginStochastic(m_support,
			numSamples);
	m_remainingSamples -= numSamples;
}

template<typename T>
inline bool CqTileArray<T>::CqStochasticIterator::inSupport() const
{
	return m_tileY < m_tileYEnd;
}

template<typename T>
inline TqInt CqTileArray<T>::CqStochasticIterator::x() const
{
	return m_currPos.x();
}

template<typename T>
inline TqInt CqTileArray<T>::CqStochasticIterator::y() const
{
	return m_currPos.y();
}

template<typename T>
const typename CqTileArray<T>::TqTile::TqSampleVector
inline CqTileArray<T>::CqStochasticIterator::operator*()
{
	return *m_currPos;
}

template<typename T>
CqTileArray<T>::CqStochasticIterator::CqStochasticIterator(const CqTileArray<T>& tileArray,
		const SqFilterSupport& support, TqInt numSamps)
	: m_support(support),
	m_tileArray(&tileArray),
	m_tileX0(support.sx.start/tileArray.m_tileWidth),
	m_tileXEnd((support.sx.end-1)/tileArray.m_tileWidth + 1),
	m_tileYEnd((support.sy.end-1)/tileArray.m_tileHeight + 1),
	m_tileX(m_tileX0),
	m_tileY(support.sy.start/tileArray.m_tileHeight),
	m_remainingArea(support.area()),
	m_remainingSamples(numSamps),
	m_currPos()
{
	// Make sure that inSupport() works correctly when the support region is
	// empty.
	if(support.isEmpty())
		m_tileY = m_tileYEnd;
	else
	{
		// Back off the iterator by one tile and invoke nextTile() to
		// properly initialize the underlying iterator.
		m_tileX -= 1;
		nextTile();
	}
}


//------------------------------------------------------------------------------
} // namespace Aqsis

#endif // TILEARRAY_H_INCLUDED
