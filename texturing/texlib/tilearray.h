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

/**
 * \file
 *
 * \brief Declare array class which holds data as tiles.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 */

#ifndef TILEARRAY_H_INCLUDED
#define TILEARRAY_H_INCLUDED

#include "aqsis.h"

#include <vector>

#include <boost/intrusive_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <boost/noncopyable.hpp>

//#include "memorysentry.h"
#include "itiledtexinputfile.h"
#include "texturebuffer.h"
//#include "tilebuffer.new.h"
#include "smartptr.h"

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
		/** \brief Access to pixels through a pixel iterator
		 *
		 * The pixel iterator will iterate through all the pixels the provided
		 * support.
		 *
		 * \param support - support to iterate over
		 */
		CqIterator begin(const SqFilterSupport& support) const;
		/** \brief Stochastic iterator access to pixels in the given support.
		 *
		 * A stochastic support iterator aims to choose a fixed number of
		 * samples from the support randomly.  The randomness choices should
		 * ideally be spread evenly over the support with minimal bunching.
		 *
		 * \param support - support to iterate over
		 * \param numSamples - number of samples to choose in the support.
		 */
//		CqStochasticIterator beginStochastic(const SqFilterSupport& support,
//				TqInt numSamples) const;
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

		/// Iterator type for the underlying tiles
		typedef typename TqTile::CqIterator TqBaseIter;
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


//template<typename T>
//class CqTileArray<T>::CqStocahsticIterator
//{
//};

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
		class CqIterator;
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

		/// Return a pixel iterator for the given support region
		CqIterator begin(const SqFilterSupport& support) const
		{
			return CqIterator( m_x0, m_y0, m_pixels->begin(
						SqFilterSupport(support.sx.start - m_x0, support.sx.end - m_x0,
						support.sy.start - m_y0, support.sy.end - m_y0)) );
		}
};


//------------------------------------------------------------------------------
/** \brief A pixel iterator adjusting an underlying iterator's origin.
 */
template<typename ArrayT>
class CqTextureTile<ArrayT>::CqIterator
{
	private:
		friend class CqTextureTile<ArrayT>;
		/// Type of the base iterator
		typedef typename ArrayT::CqIterator TqBaseIter;

		/** \brief Construct a pixel iterator with offset origin.
		 *
		 * \param x0 - x-coordinate of origin for offset
		 * \param y0 - y-coordinate of origin for offset
		 * \param baseIter - iterator for base tile.
		 */
		CqIterator(const TqInt x0, const TqInt y0, const TqBaseIter& baseIter)
			: m_currPos(baseIter),
			m_x0(x0),
			m_y0(y0)
		{ }

		/// Current position in underlying buffer
		TqBaseIter m_currPos;
		/// x-coordinate of origin for offset
		TqInt m_x0;
		/// y-coordinate of origin for offset
		TqInt m_y0;
	public:
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
typename CqTileArray<T>::CqIterator CqTileArray<T>::begin(
		const SqFilterSupport& support) const
{
	return CqIterator(*this, intersect(support,
				SqFilterSupport(0,m_width, 0,m_height)));
}

//template<typename T>
//CqTileArray<T>::CqStochasticIterator CqTileArray<T>::beginStochastic(
//		const SqFilterSupport& support, TqInt numSamples) const
//{
//}

template<typename T>
inline boost::intrusive_ptr<typename CqTileArray<T>::TqTile> CqTileArray<T>::getTile(
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
	{
		// If we've passed outside the support of the current tile, we
		// advance to the next one.
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
	return *this;
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
	if(support.sx.isEmpty())
		m_tileY = m_tileYEnd;
}

//------------------------------------------------------------------------------
} // namespace Aqsis

#endif // TILEARRAY_H_INCLUDED
