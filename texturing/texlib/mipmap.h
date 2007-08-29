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
 * \brief Classes and functions for creating mipmaps
 *
 * \author Chris Foster
 */

#ifndef MIPMAP_H_INCLUDED
#define MIPMAP_H_INCLUDED

#include "aqsis.h"

#include <vector>

namespace Aqsis
{

//------------------------------------------------------------------------------
/** \brief Determine the mipmap level sizes for a given image
 *
 * A mipmap consists of successive downscalings of the original file by
 * a factor of two.  The smallest mipmap level consists of a single
 * pixel (this is the convention taken in the OpenExr spec.)
 *
 * \param width  - image width
 * \param height - image height
 * \param levelWidths  - output vector of mipmap level widths
 * \param levelHeights - output vector of mipmap level heights
 */
void mipmapLevelSizes(TqUint width, TqUint height,
		std::vector<TqUint>& levelWidths, std::vector<TqUint>& levelHeights);


//------------------------------------------------------------------------------
/** \brief A class which knows how to sample and filter a mipmap level.
 *
 */
#if 0
class CqMipmapLevel
{
	public:
		CqMipmapLevel(TIFF* tiffFile, sOffset, sMult, tOffset, tMult);
		inline void filter(std::vector<TqFloat>& sBox, std::vector<TqFloat>& tBox, std::vector<TqFloat>& output);
		virtual void sampleBilinear(TqFloat s, TqFloat t, std::vector<TqFloat>& output);
	protected:
		virtual void connectToTiff(TIFF* tiffFile) = 0;
		virtual void filterEWA(TqFloat s, TqFloat t, std::vector<TqFloat>& output) = 0;
		virtual void filterMCI(TqFloat s, TqFloat t, std::vector<TqFloat>& output) = 0;
	private:
		CqFloat m_sOffset;
		CqFloat m_sMult;
		CqFloat m_tOffset;
		CqFloat m_tMult;
};


//------------------------------------------------------------------------------
/** \brief Template class for implementation of mipmap stuff.
 */
template<typename T>
class CqMipmapLevelImpl : CqMipmapLevel
{
	public:
		CqMipmapLevelImpl(TIFF* tiffFile, sOffset, sMult, tOffset, tMult);
		inline void filter(std::vector<TqFloat>& sBox, std::vector<TqFloat>& tBox, std::vector<TqFloat>& output);
		virtual void sampleBilinear(TqFloat s, TqFloat t, std::vector<TqFloat>& output);
	protected:
		virtual void connectToTiff(TIFF* tiffFile);
		inline void filterEWA(std::vector<TqFloat>& sBox, std::vector<TqFloat>& tBox, std::vector<TqFloat>& output);
		inline void filterMCI(std::vector<TqFloat>& sBox, std::vector<TqFloat>& tBox, std::vector<TqFloat>& output);
		inline void texToRasterCoords(TqFloat s, TqFloat t, TqFloat& sOut, TqFloat& tOut);
		/// May be better put in the CqTextureTileArray class.
		inline bool putWithinBounds(TqInt& iStart, TqInt& iStop, TqInt& jStart, TqInt& jStop);
		inline CqMatrix2D estimateJacobian(std::vector<TqFloat>& sBox, std::vector<TqFloat>& tBox);
	private:
		CqTextureTileArray<T> m_imageData;
};
#endif

//------------------------------------------------------------------------------
/** \brief A minimal 2D matrix class for use in texture warping.
 *
 * Don't develop this further until it's found out exactly how much it's needed...
 */
#if 0
class CqMatrix2D
{
	public:
		/** Construct a 2D matrix
		 */
		CqMatrix2D(a,b,c,d);
		/** \brief Factory function to make identity matrix.
		 */
		inline static CqMatrix2D identity();
		inline TqFloat determinant();
		inline CqMatrix2D operator*(CqMatrix2D& rhs);
		inline CqMatrix2D operator+(CqMatrix2D& rhs);
		inline TqFloat operator()(TqInt i, TqInt j);
		CqMatrix2D inverse();
	private:
		TqFloat m_a;
		TqFloat m_b;
		TqFloat m_c;
		TqFloat m_d;
};
#endif

//------------------------------------------------------------------------------
} // namespace Aqsis

#endif // MIPMAP_H_INCLUDED
