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

#include "aqsis.h"

#include <valarray>

#include "tilearray.h"
#include "aqsismath.h"
#include "samplequad.h"
#include "texturesampleoptions.h"

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief A class which knows how to sample texture buffers.
 *
 */
class IqTextureSampler
{
	public:
		/** \brief Sample the texture with the provided sample options.
		 *
		 * \param sampleQuad - quadrilateral region to sample over
		 * \param sampleOpts - options to the sampler, including filter widths etc.
		 * \param output - the output samples will be placed here.  
		 */
		virtual void filter(const SqSampleQuad& sampleQuad,
				const SqTextureSampleOptions& sampleOpts, TqFloat* output) const = 0;
		/** \brief Create and return a IqTextureSampler derived class
		 *
		 * The returned class is a CqTextureSamplerImpl<T> where T is a type
		 * appropriate to the pixel type held in the file.
		 *
		 * \param file - tiled texture file which the sampler should be connected to.
		 */
		static boost::shared_ptr<IqTextureSampler> create(
				const boost::shared_ptr<IqTiledTexInputFile>& file);
		virtual ~IqTextureSampler() {}
};

//------------------------------------------------------------------------------
/** \brief Implementation of texture buffer samplers
 */
template<typename T>
class CqTextureSamplerImpl : public IqTextureSampler
{
	public:
		CqTextureSamplerImpl(const boost::shared_ptr<CqTileArray<T> >& texData);
		virtual void filter(const SqSampleQuad& sampleQuad,
				const SqTextureSampleOptions& sampleOpts, TqFloat* output) const;
	private:
		/*
		inline void sampleBilinear(TqFloat s, TqFloat t, std::valarray<TqFloat>& output);
		void filterEWA(
		void filterMC(
		*/
		inline void texToRasterCoords(TqFloat s, TqFloat t, TqInt& sOut, TqInt& tOut) const;
		void filterNearestNeighbour(const SqSampleQuad& sampleQuad,
				const SqTextureSampleOptions& sampleOpts, TqFloat* output) const;
		/// May be better put in the CqTextureTileArray class.
		//inline bool putWithinBounds(TqInt& iStart, TqInt& iStop, TqInt& jStart, TqInt& jStop);
		//inline CqMatrix2D estimateJacobian(const SqSampleQuad& sampleQuad);
		TqFloat dummyGridTex(TqInt s, TqInt t) const;

	private:
		// instance data
		boost::shared_ptr<CqTileArray<T> > m_texData;
		TqFloat m_sMult;
		TqFloat m_tMult;
		/// (Analyse performance+complexity/quality tradeoff before including offsets):
		//TqFloat m_tOffset;
		//TqFloat m_sOffset;
};


//==============================================================================
// Implementation details
//==============================================================================

template<typename T>
inline CqTextureSamplerImpl<T>::CqTextureSamplerImpl(
		const boost::shared_ptr<CqTileArray<T> >& texData)
	: m_texData(texData),
	m_sMult(511),
	m_tMult(511)
{ }

template<typename T>
void CqTextureSamplerImpl<T>::filter(const SqSampleQuad& sampleQuad, const SqTextureSampleOptions& sampleOpts, TqFloat* output) const
{
	// \todo add switch based on filter algorithm
	filterNearestNeighbour(sampleQuad, sampleOpts, output);
}

template<typename T>
inline void CqTextureSamplerImpl<T>::texToRasterCoords(TqFloat s, TqFloat t, TqInt& sOut, TqInt& tOut) const
{
	// \todo handle edge clip mode.  For now just clamp.
	sOut = lfloor(m_sMult*clamp<TqFloat>(s, 0.0f, 1.0f)+0.5);
	tOut = lfloor(m_tMult*clamp<TqFloat>(t, 0.0f, 1.0f)+0.5);
}

template<typename T>
void CqTextureSamplerImpl<T>::filterNearestNeighbour( const SqSampleQuad& sQuad,
		const SqTextureSampleOptions& sampleOpts, TqFloat* output) const
{
	// \todo implementation
	TqInt s = 0;
	TqInt t = 0;
	texToRasterCoords((sQuad.v1.x() + sQuad.v2.x() + sQuad.v3.x() + sQuad.v4.x())/4,
			(sQuad.v1.y() + sQuad.v2.y() + sQuad.v3.y() + sQuad.v4.y())/4,
			s, t);
	TqFloat texVal = dummyGridTex(s, t);
	for(TqInt i = 1; i > 0; --i, ++output)
		*output = texVal;
}

template<typename T>
TqFloat CqTextureSamplerImpl<T>::dummyGridTex(TqInt s, TqInt t) const
{
	const TqInt gridSize = 8;
	const TqInt lineWidth = 1;
	TqFloat outVal = 1;
	//if((s / gridSize) % 2 == 0 | (t / gridSize) % 2 == 0) // checkered
	if((s % gridSize) < lineWidth || (t % gridSize) < lineWidth) // grid
		outVal = 0;
	return outVal;
}

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
