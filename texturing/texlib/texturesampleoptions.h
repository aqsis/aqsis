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
 * \brief Define texture sampling options structures and associated enums
 *
 * \author Chris Foster [chris42f (at) gmail (d0t) com]
 */

#ifndef TEXTURESAMPLEOPTIONS_H_INCLUDED
#define TEXTURESAMPLEOPTIONS_H_INCLUDED

#include "aqsis.h"

#include "samplequad.h"
#include "wrapmode.h"

namespace Aqsis
{

class CqTexFileHeader;

//----------------------------------------------------------------------
/** \brief Texture filter types
 *
 * The texture filters define the weighting functions for texture filtering.
 *
 * EWA (Elliptical Weighted Average or gaussian) filtering is very high
 * quality, so it's expected that most people will want this.
 *
 * Other filter types are evaluated by Monte Carlo integration of a bilinearly
 * reconstructed texture over the filter support, while EWA is analytical, (so
 * has no sampling noise).
 */
enum EqTextureFilter
{
	TextureFilter_Box,		///< box filtering (Monte Carlo)
	TextureFilter_Gaussian, ///< implies EWA filtering
	TextureFilter_None,		///< No filtering, only do bilinear interpolation.
	TextureFilter_Unknown	///< Unknown filter type.
};


/** \brief Return a filter type given a descriptive string.
 *
 * \param filterName - The name string is the lowercase suffix (the bit after
 * the underscore) of the associated enum name.
 *
 * \return The EqTextureFilter associated with a name string.  If the string
 * doesn't describe a known filter, return TextureFilter_Unknown.
 */
EqTextureFilter texFilterTypeFromString(const char* filterName);


//----------------------------------------------------------------------
/** \brief Contain the standard renderman texture/environment sampling options
 *
 * This class holds many of the sampling parameters which may be passed to the
 * texture() and environment() RSL calls.  Note that blur and width are left
 * out; the more specific sBlur,tBlur, etc are used instead.
 */
class AQSISTEX_SHARE CqTextureSampleOptions
{
	public:
		/// Trivial constructor.
		inline CqTextureSampleOptions(TqFloat sBlur, TqFloat tBlur, TqFloat sWidth,
				TqFloat tWidth, EqTextureFilter filterType, TqFloat fill,
				TqInt startChannel, TqInt numChannels,
				TqInt numSamples, EqWrapMode sWrapMode, EqWrapMode tWrapMode);
		/** Default constructor
		 *
		 * Set all numerical quantities to 0 except for sWidth, tWidth,
		 * numSamples and numChannels which are set to 1.  The filter is set to
		 * gaussian, and wrap modes to black.
		 */
		inline CqTextureSampleOptions();

		//--------------------------------------------------
		/// \name Accessors for relevant texture sample options.
		//@{
		/// Get the blur in the s-direction
		inline TqFloat sBlur() const;
		/// Get the blur in the t-direction
		inline TqFloat tBlur() const;
		/// Get the width multiplier in the s-direction
		inline TqFloat sWidth() const;
		/// Get the width multiplier in the t-direction
		inline TqFloat tWidth() const;
		/// Get the filter type
		inline EqTextureFilter filterType() const;
		/// Get the fill value for texture channels indices outside the available range
		inline TqFloat fill() const;
		/// Get the start channel index where channels will be read from.
		inline TqInt startChannel() const;
		/// Get the number of channels to sample
		inline TqInt numChannels() const;
		/// Get the number of samples used by stochastic sampling methods.
		inline TqInt numSamples() const;
		/// Get the wrap mode in the s-direction
		inline EqWrapMode sWrapMode() const;
		/// Get the wrap mode in the t-direction
		inline EqWrapMode tWrapMode() const;
		//@}

		//--------------------------------------------------
		/// \name Modifiers for texture sampling options.
		//@{
		/// Set the blur in both directions
		inline void setBlur(TqFloat blur);
		/// Set the blur in the s-direction
		inline void setSBlur(TqFloat sBlur);
		/// Set the blur in the t-direction
		inline void setTBlur(TqFloat tBlur);
		/// Set the width multiplier in all directions.
		inline void setWidth(TqFloat width);
		/// Set the width multiplier in the s-direction
		inline void setSWidth(TqFloat sWidth);
		/// Set the width multiplier in the t-direction
		inline void setTWidth(TqFloat tWidth);
		/// Set the filter type
		inline void setFilterType(EqTextureFilter type);
		/// Set the fill value for texture channels indices outside the available range
		inline void setFill(TqFloat fill);
		/// Set the start channel index where channels will be read from.
		inline void setStartChannel(TqInt startChannel);
		/// Set the number of channels to sample
		inline void setNumChannels(TqInt numChans);
		/// Set the number of samples used by stochastic sampling methods.
		inline void setNumSamples(TqInt numSamples);
		/// Set the wrap mode in both directions
		inline void setWrapMode(EqWrapMode wrapMode);
		/// Set the wrap mode in the s-direction
		inline void setSWrapMode(EqWrapMode sWrapMode);
		/// Set the wrap mode in the t-direction
		inline void setTWrapMode(EqWrapMode tWrapMode);
		//@}

		/** \brief Extract sample options from a texture file header.
		 *
		 * As many relevant texture file attributes are extracted from the
		 * header as possible and used to fill the fields of the sample options
		 * object.
		 *
		 * \param header - extract options from here.
		 */
		void fillFromFileHeader(const CqTexFileHeader& header);

		//--------------------------------------------------
		/** \brief Adjust a sample quad to take into account the width
		 * parameter.
		 *
		 * The width parameter effects the quad in a simple way:  All the
		 * vertices are contracted toward or exapanded away from the quad
		 * center point by multiplying by the width in the appropriate
		 * direction.  
		 *
		 * \param quad - sample quadrilateral to adjust.
		 */
		void adjustSampleQuad(Tq2DSampleQuad& quad) const;

	protected:
		/** \brief Check that the blur and filter settings are compatible.
		 *
		 * This function checks that the filter is gaussian if we're attempting
		 * to apply a nonzero blur to the texture.  If it's not gaussian, then
		 * a warning is generated.
		 */
		void checkBlurAndFilter();

		/// Texture blur amount in the s and t directions.  
		TqFloat m_sBlur;
		TqFloat m_tBlur;
		/// Filter widths as a multiple of the given filter box.
		TqFloat m_sWidth;
		TqFloat m_tWidth;
		/// Type of filter - represents both the filter weight function and filtering algorithm.
		EqTextureFilter m_filterType;
		/// Value to fill a channel with if the associated texture doesn't contain sufficiently many channels.
		TqFloat m_fill;
		/// Index of the starting channel (the first channel has index 0)
		TqInt m_startChannel;
		/// Number of channels to sample from the texture.
		TqInt m_numChannels;
		/// Number of samples to take when using a stochastic sampler
		TqInt m_numSamples;
		/// Wrap modes specifying what the sampler should do with (s,t) coordinates
		/// lying off the image edge.
		EqWrapMode m_sWrapMode;
		EqWrapMode m_tWrapMode;
};


class AQSISTEX_SHARE CqShadowSampleOptions : private CqTextureSampleOptions
{
	public:
		// Accessors from CqTextureSampleOptions
		CqTextureSampleOptions::sBlur;
		CqTextureSampleOptions::tBlur;
		CqTextureSampleOptions::sWidth;
		CqTextureSampleOptions::tWidth;
		CqTextureSampleOptions::filterType;
		CqTextureSampleOptions::fill;
		CqTextureSampleOptions::startChannel;
		CqTextureSampleOptions::numChannels;
		CqTextureSampleOptions::numSamples;
		CqTextureSampleOptions::sWrapMode;
		CqTextureSampleOptions::tWrapMode;

		CqTextureSampleOptions::setBlur;
		CqTextureSampleOptions::setSBlur;
		CqTextureSampleOptions::setTBlur;
		CqTextureSampleOptions::setWidth;
		CqTextureSampleOptions::setSWidth;
		CqTextureSampleOptions::setTWidth;
		CqTextureSampleOptions::setFilterType;
		CqTextureSampleOptions::setFill;
		CqTextureSampleOptions::setStartChannel;
		CqTextureSampleOptions::setNumChannels;
		CqTextureSampleOptions::setNumSamples;
		CqTextureSampleOptions::setWrapMode;
		CqTextureSampleOptions::setSWrapMode;
		CqTextureSampleOptions::setTWrapMode;

		//--------------------------------------------------
		// Shadow-specific sample options
		/// Get the low shadow bias
		TqFloat biasLow() const;
		/// Get the high shadow bias
		TqFloat biasHigh() const;

		/// Set the shadow bias
		void setBias(TqFloat bias);
		/// Set the low and high shadow biases
		void setBias(TqFloat biasLow, TqFloat biasHigh);
	protected:
		TqFloat m_biasLow;
		TqFloat m_biasHigh;
};


//==============================================================================
// Implementation details.
//==============================================================================

// CqTextureSampleOptions implementation

inline CqTextureSampleOptions::CqTextureSampleOptions( TqFloat sBlur, TqFloat tBlur,
		TqFloat sWidth, TqFloat tWidth, EqTextureFilter filterType, TqFloat fill,
		TqInt startChannel, TqInt numChannels, TqInt numSamples,
		EqWrapMode sWrapMode, EqWrapMode tWrapMode)
	: m_sBlur(sBlur),
	m_tBlur(tBlur),
	m_sWidth(sWidth),
	m_tWidth(tWidth),
	m_filterType(filterType),
	m_fill(fill),
	m_startChannel(startChannel),
	m_numChannels(numChannels),
	m_numSamples(numSamples),
	m_sWrapMode(sWrapMode),
	m_tWrapMode(tWrapMode)
{ }

inline CqTextureSampleOptions::CqTextureSampleOptions()
	: m_sBlur(0),
	m_tBlur(0),
	m_sWidth(1),
	m_tWidth(1),
	m_filterType(TextureFilter_Gaussian),
	m_fill(0),
	m_startChannel(0),
	m_numChannels(1),
	m_numSamples(1),
	m_sWrapMode(WrapMode_Black),
	m_tWrapMode(WrapMode_Black)
{ }


inline TqFloat CqTextureSampleOptions::sBlur() const
{
	return m_sBlur;
}

inline TqFloat CqTextureSampleOptions::tBlur() const
{
	return m_tBlur;
}

inline TqFloat CqTextureSampleOptions::sWidth() const
{
	return m_sWidth;
}

inline TqFloat CqTextureSampleOptions::tWidth() const
{
	return m_tWidth;
}

inline EqTextureFilter CqTextureSampleOptions::filterType() const
{
	return m_filterType;
}

inline TqFloat CqTextureSampleOptions::fill() const
{
	return m_fill;
}

inline TqInt CqTextureSampleOptions::startChannel() const
{
	return m_startChannel;
}

inline TqInt CqTextureSampleOptions::numChannels() const
{
	return m_numChannels;
}

inline TqInt CqTextureSampleOptions::numSamples() const
{
	return m_numSamples;
}

inline EqWrapMode CqTextureSampleOptions::sWrapMode() const
{
	return m_sWrapMode;
}

inline EqWrapMode CqTextureSampleOptions::tWrapMode() const
{
	return m_tWrapMode;
}


inline void CqTextureSampleOptions::setBlur(TqFloat blur)
{
	setSBlur(blur);
	setTBlur(blur);
}

inline void CqTextureSampleOptions::setSBlur(TqFloat sBlur)
{
	assert(sBlur >= 0);
	m_sBlur = sBlur;
	checkBlurAndFilter();
}

inline void CqTextureSampleOptions::setTBlur(TqFloat tBlur)
{
	assert(tBlur >= 0);
	m_tBlur = tBlur;
	checkBlurAndFilter();
}

inline void CqTextureSampleOptions::setWidth(TqFloat width)
{
	setSWidth(width);
	setTWidth(width);
}

inline void CqTextureSampleOptions::setSWidth(TqFloat sWidth)
{
	assert(sWidth >= 0);
	m_sWidth = sWidth;
}

inline void CqTextureSampleOptions::setTWidth(TqFloat tWidth)
{
	assert(tWidth >= 0);
	m_tWidth = tWidth;
}

inline void CqTextureSampleOptions::setFilterType(EqTextureFilter type)
{
	assert(type != TextureFilter_Unknown);
	m_filterType = type;
	checkBlurAndFilter();
}

inline void CqTextureSampleOptions::setFill(TqFloat fill)
{
	m_fill = fill;
}

inline void CqTextureSampleOptions::setStartChannel(TqInt startChannel)
{
	assert(startChannel >= 0);
	m_startChannel = startChannel;
}

inline void CqTextureSampleOptions::setNumChannels(TqInt numChans)
{
	assert(numChans >= 0);
	m_numChannels = numChans;
}

inline void CqTextureSampleOptions::setNumSamples(TqInt numSamples)
{
	assert(numSamples >= 0);
	m_numSamples = numSamples;
}

inline void CqTextureSampleOptions::setWrapMode(EqWrapMode wrapMode)
{
	setSWrapMode(wrapMode);
	setTWrapMode(wrapMode);
}

inline void CqTextureSampleOptions::setSWrapMode(EqWrapMode sWrapMode)
{
	m_sWrapMode = sWrapMode;
}

inline void CqTextureSampleOptions::setTWrapMode(EqWrapMode tWrapMode)
{
	m_tWrapMode = tWrapMode;
}

} // namespace Aqsis

#endif // TEXTURESAMPLEOPTIONS_H_INCLUDED
