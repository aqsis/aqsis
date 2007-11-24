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
 * \brief Texture buffer sampling machinery.
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef TEXTURESAMPLER_H_INCLUDED
#define TEXTURESAMPLER_H_INCLUDED

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief A class which knows how to sample texture buffers.
 *
 */
class CqTextureSampler
{
	public:
		inline void filter(std::vector<TqFloat>& sBox, std::vector<TqFloat>& tBox, std::vector<TqFloat>& output);
		static boost::shared_ptr<CqTextureSampler> create(const boost::shared_ptr<IqTiledTexInputFile>& file);
		virtual ~CqTextureSampler();
	protected:
		/**
		 */
		virtual void filterClosest(TqFloat s, TqFloat t, std::vector<TqFloat>& output) = 0;
		/** \breif Elliptical Weighted Average (EWA) filter
		 */
		virtual void filterEWA(TqFloat s, TqFloat t, std::vector<TqFloat>& output) = 0;
		/**
		 */
		virtual void filterMCI(TqFloat s, TqFloat t, std::vector<TqFloat>& output) = 0;
	private:
};

//------------------------------------------------------------------------------
/** \brief Implementation of texture buffer samplers
 */
template<typename T>
class CqTextureSamplerImpl : public CqTextureSampler
{
	public:
		CqTextureSamplerImpl(const boost::shared_ptr<CqTextureSampler>& tiffFile, sOffset, sMult, tOffset, tMult);
		inline void filter(std::vector<TqFloat>& sBox, std::vector<TqFloat>& tBox, std::vector<TqFloat>& output);
	protected:
		inline void sampleBilinear(TqFloat s, TqFloat t, std::vector<TqFloat>& output);
		inline void filterEWA(std::vector<TqFloat>& sBox, std::vector<TqFloat>& tBox, std::vector<TqFloat>& output);
		inline void filterMCI(std::vector<TqFloat>& sBox, std::vector<TqFloat>& tBox, std::vector<TqFloat>& output);
		inline void texToRasterCoords(TqFloat s, TqFloat t, TqFloat& sOut, TqFloat& tOut);
		/// May be better put in the CqTextureTileArray class.
		inline bool putWithinBounds(TqInt& iStart, TqInt& iStop, TqInt& jStart, TqInt& jStop);
		inline CqMatrix2D estimateJacobian(std::vector<TqFloat>& sBox, std::vector<TqFloat>& tBox);
	private:
		CqTileArray<T> m_imageData;
		TqFloat m_sOffset;
		TqFloat m_sMult;
		TqFloat m_tOffset;
		TqFloat m_tMult;
};


//==============================================================================
// Implementation details
//==============================================================================

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

} // namespace Aqsis

#endif // TEXTURESAMPLER_H_INCLUDED
