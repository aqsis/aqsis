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

#include "aqsismath.h"
#include "texturesampleoptions.h"

namespace Aqsis
{

//------------------------------------------------------------------------------
/** \brief Downsample an image to the next smaller mipmap size.
 *
 * The size of the new image is ceil(width/2) x ceil(height/2).  This is one
 * possible choice, the other being to take the floor.  Neither of these is
 * obviously preferable; the essential problem is that non-power of two
 * textures aren't perfect for mipmapping, though they do a fairly good job.
 *
 * \param
 */
template<typename FilterFunctorT, typename PixelT>
boost::shared_ptr<CqTextureBuffer<PixelT> > mipmapDownsample(
		const CqTextureBuffer<PixelT>& inputBuf,
		TqFloat sWidth, TqFloat tWidth,
		const FilterFunctorT& filterWeights,
		EqWrapMode sWrapMode, EqWrapMode tWrapMode);

template<typename FilterFunctorT, typename PixelT>
boost::shared_ptr<CqTextureBuffer<PixelT> > mipmapDownsampleNonseperable(
		const CqTextureBuffer<PixelT>& srcBuf,
		const CqCachedFilter& filterWeights,
		EqWrapMode sWrapMode, EqWrapMode tWrapMode);

//template<typename FilterFunctorT, typename PixelT>
//boost::shared_ptr<CqTextureBuffer<PixelT> > mipmapDownsampleSeperable(
//		const CqTextureBuffer<PixelT>& srcBuf,
//		const std::vector<TqFloat>& filterWeights,
//		EqWrapMode sWrapMode, EqWrapMode tWrapMode);



//==============================================================================
// Implementation details

template<typename FilterFunctorT>
boost::shared_ptr<CqTextureBuffer> mipmapDownsample(
		const CqTextureBuffer& inputBuf,
		TqFloat sWidth, TqFloat tWidth,
		const FilterFunctorT& filterWeights,
		EqWrapMode sWrapMode, EqWrapMode tWrapMode)
{
	// Amount to scale the image by.  Fixed at a factor of 2 for now.
	TqFloat scale = 0.5f;

	// Get the size of the lattice in the source buffer over which the filter
	// needs to be evaluated.  For even-sized images, our downsampled points
	// will lie *between* samples of the source, so we want to straddle the
	// zero point of the filter rather than include the zero point.  This leads
	// to the two different cases.
	TqInt sSupportSize =
		(inputBuf.width() % 2 == 0)
		? max(2*static_cast<TqInt>(0.5*(sWidth/scale-1)), 2)
		: max(2*static_cast<TqInt>(0.5*sWidth/scale), 3);
	TqInt tSupportSize =
		(inputBuf.height() % 2 == 0)
		? max(2*static_cast<TqInt>(0.5*(tWidth/scale-1)), 2)
		: max(2*static_cast<TqInt>(0.5*tWidth/scale), 3);
	/// \todo Optimize for seperable filters.  This should also give some idea
	/// about how to optimize the analogous filtering operations in the core renderer.
	/// if(FilterFunctorT::isSeperable) ...

	// General case: Non-seperable filter.
	CqCachedFilter weights(CqSincFilter(sWidth*scale, tWidth*scale),
			sSupportSize, tSupportSize, scale);
	return mipmapDownsampleNonseperable(srcBuf, sWrapMode, tWrapMode);
}


namespace detail
{

class CqImageDownsampler
{
	public:
		/** \brief Construct an image downsampler 
		 *
		 * \param sWidth  filter width in s direction
		 * \param tWidth  filter width in t direction
		 * \param filterFunc  function returning filter coefficients
		 * \param sWrapMode  texture wrapping mode in s direction
		 * \param tWrapMode  texture wrapping mode in t direction
		 */
		CqImageDownsampler(TqFloat sWidth, TqFloat tWidth, RtFilterFunc filterFunc, EqWrapMode sWrapMode, EqWrapMode tWrapMode);

		/** \brief Downsample an image by a factor of two.
		 */
		CqTextureMapBuffer* downsample(CqTextureMapBuffer* inBuf, CqTextureMap& texMap, TqInt directory, bool protectBuffer);
	private:
		/** \brief Compute and cache filter kernel values for the given filter function.
		 */
		void computeFilterKernel(TqFloat sWidth, TqFloat tWidth, RtFilterFunc filterFunc, bool evenFilterS, bool evenFilterT);
		/** \brief Wrap a position at the image edges according to current wrapmode
		 */
		inline TqInt edgeWrap(TqInt pos, TqInt posMax, EqWrapMode mode);

		// Filter parameters for the convolution kernel.
		TqInt m_sNumPts;
		TqInt m_tNumPts;
		TqInt m_sStartOffset;
		TqInt m_tStartOffset;
		std::vector<TqFloat> m_weights;
		// Other members
		TqFloat m_sWidth;
		TqFloat m_tWidth;
		RtFilterFunc m_filterFunc;
		EqWrapMode m_sWrapMode;
		EqWrapMode m_tWrapMode;
};

template<typename FilterFunctorT>
CqCachedFilter computeFilterWeights(TqFloat sWidth, TqFloat tWidth,
		const FilterFunctorT& weightFxn, bool evenFilterS, bool evenFilterT)
{
	// set up filter sizes & offsets
	if(evenFilterS) // for even-sized images in s
		m_sNumPts = std::max(2*static_cast<TqInt>((sWidth+1)/2), 2);
	else // for odd-sized images in s
		m_sNumPts = std::max(2*static_cast<TqInt>(sWidth/2) + 1, 3);

	if(evenFilterT) // for even-sized images in t
		m_tNumPts = std::max(2*static_cast<TqInt>((tWidth+1)/2), 2);
	else // for odd-sized images in t
		m_tNumPts = std::max(2*static_cast<TqInt>(tWidth/2) + 1, 3);

	m_sStartOffset = -(m_sNumPts-1)/2;
	m_tStartOffset = -(m_tNumPts-1)/2;

	// set up filter weights
	m_weights.resize(m_tNumPts * m_sNumPts);
	TqUint weightOffset = 0;
	TqFloat sum = 0;
	for(TqInt j = 0; j < m_tNumPts; j++)
	{
		// overall division by 2 is to downsample the image by a factor of 2.
		TqFloat t = (-(m_tNumPts-1)/2.0 + j)/2;
		for(TqInt i = 0; i < m_sNumPts; i++)
		{
			TqFloat s = (-(m_sNumPts-1)/2.0 + i)/2;
			m_weights[weightOffset] = (*filterFunc) (s, t, sWidth/2, tWidth/2);
			sum += m_weights[weightOffset];
			weightOffset++;
		}
	}
	// normalise the filter
	for(std::vector<TqFloat>::iterator i = m_weights.begin(), end = m_weights.end(); i != end; i++)
		*i /= sum;

	// print the filter kernel to the log at debug priority
//	weightOffset = 0;
//	Aqsis::log() << debug << "filter Kernel =\n";
//	for(TqInt j = 0; j < m_tNumPts; j++)
//	{
//		Aqsis::log() << debug << "[";
//		for(TqInt i = 0; i < m_sNumPts; i++)
//		{
//			Aqsis::log() << debug << m_weights[weightOffset++] << ", "; 
//		}
//		Aqsis::log() << debug << "]\n";
//	}
//	Aqsis::log() << debug << "\n";
}


inline TqInt CqImageDownsampler::edgeWrap(TqInt pos, TqInt posMax, EqWrapMode mode)
{
	switch(mode)
	{
		case WrapMode_Clamp:
		return clamp(pos, 0, posMax-1);
		break;
		case WrapMode_Periodic:
		return pos = (pos + posMax) % posMax;
		break;
		case WrapMode_Black:
		default:
		return pos;
	}
}

//----------------------------------------------------------------------
CqTextureMapBuffer* CqImageDownsampler::downsample(CqTextureMapBuffer* inBuf, CqTextureMap& texMap, TqInt directory, bool protectBuffer)
{
	TqInt imWidth = inBuf->Width();
	TqInt imHeight = inBuf->Height();
	TqInt newWidth = (imWidth+1)/2;
	TqInt newHeight = (imHeight+1)/2;
	TqInt samplesPerPixel = inBuf->Samples();
	bool imEvenS = !(imWidth % 2);
	bool imEvenT = !(imHeight % 2);
	if(m_weights.empty() || !((m_sNumPts % 2) ^ imEvenS) || !((m_tNumPts % 2) ^ imEvenT))
	{
		// recalculate filter kernel if cached one isn't the right size.
		computeFilterKernel(m_sWidth, m_tWidth, m_filterFunc, imEvenS, imEvenT);
	}
	// Make a new buffer to store the downsampled image in.
	CqTextureMapBuffer* outBuf = texMap.CreateBuffer(0, 0, newWidth, newHeight, directory, protectBuffer);
	if(outBuf->pVoidBufferData() == NULL)
		throw XqException("Cannot create buffer for downsampled image");
	std::vector<TqFloat> accum(samplesPerPixel);
	for(TqInt y = 0; y < newHeight; y++)
	{
		for(TqInt x = 0; x < newWidth; x++)
		{
			// s ~ x ~ inner loop ~ 1st var in TMB.GetValue ~ width
			// t ~ y ~ outer loop ~ 2nd var in TMB.GetValue ~ height
			TqInt weightOffset = 0;
			accum.assign(samplesPerPixel, 0);
			for(TqInt j = 0; j < m_tNumPts; j++)
			{
				TqInt ypos = edgeWrap(2*y + m_tStartOffset + j, imHeight, m_tWrapMode);
				for(TqInt i = 0; i < m_sNumPts; i++)
				{
					TqInt xpos = edgeWrap(2*x + m_sStartOffset + i, imWidth, m_sWrapMode);
					if(!((m_tWrapMode == WrapMode_Black && (ypos < 0 || ypos >= imHeight))
							|| (m_sWrapMode == WrapMode_Black && (xpos < 0 || xpos >= imWidth))))
					{
						TqFloat weight = m_weights[weightOffset];
						for(TqInt sample = 0; sample < samplesPerPixel; sample++)
							accum[sample] += weight * inBuf->GetValue(xpos, ypos, sample);
					}
					weightOffset++;
				}
			}
			for(TqInt sample = 0; sample < samplesPerPixel; sample++)
				outBuf->SetValue(x, y, sample, CLAMP(accum[sample], 0.0, 1.0));
		}
	}
	return outBuf;
}

} // namespace detail

boost::shared_ptr<CqTextureBuffer> mipmapDownsampleNonseperable(
		const CqTextureBuffer& srcBuf,
		const CqCachedFilter& filterWeights,
		EqWrapMode sWrapMode, EqWrapMode tWrapMode)
{
	TqInt newWidth = lceil(srcBuf.width()/2.0f);
	TqInt newHeight = lceil(srcBuf.height()/2.0f);
	boost::shared_ptr<CqTextureBuffer> destBuf(new CqTextureBuffer());
}

} // namespace Aqsis

#endif // MIPMAP_H_INCLUDED
