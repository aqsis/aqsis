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

namespace Aqsis
{

//----------------------------------------------------------------------
/** \brief Defines the various modes of handling texture access outside of the
 * normal range.
 */
enum	EqWrapMode
{
    WrapMode_Black,		///< Return black.
    WrapMode_Periodic,	///< Wrap around to the opposite side.
    WrapMode_Clamp,		///< Clamp to the edge of the range.
};


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
 * This struct holds many of the sampling parameters which may be passed to the
 * texture() and environment() RSL calls.  Note that blur and width are left
 * out; the more specific sblur,tblur, are used instead.
 */
struct SqTextureSampleOptions
{
	/// Additional texture blurs in the s and t directions.  
	TqFloat sblur;
	TqFloat tblur;
	/// Filter widths as a multiple of the given filter box.
	TqFloat swidth;
	TqFloat twidth;
	/// Type of filter - represents both the filter weight function and filtering algorithm.
	EqTextureFilter filterType;
	/// Value to fill a channel with if the associated texture doesn't contain sufficiently many channels.
	TqFloat fill;
	/// Index of the starting channel (the first channel has index 0)
	TqInt startChannel;
	/// Number of channels to sample from the texture.
	TqInt numChannels;
	/// Number of samples to take when using a stochastic sampler
	TqInt numSamples;
	/// Wrap modes specifying what the sampler should do with (s,t) coordinates lying off the image edge.
	EqWrapMode sWrapMode;
	EqWrapMode tWrapMode;

	/// Trivial constructor.
	inline SqTextureSampleOptions(TqFloat sblur, TqFloat tblur, TqFloat swidth,
			TqFloat twidth, EqTextureFilter filterType, TqFloat fill,
			TqInt startChannel, TqInt numChannels, TqInt numSamples,
			EqWrapMode sWrapMode, EqWrapMode tWrapMode);
	/** Default constructor
	 *
	 * Set all numerical quantities to 0 except for numSamples and numChannels
	 * which are set to 1.  The filter is set to gaussian, and wrap modes to
	 * black.
	 */
	inline SqTextureSampleOptions();
};


struct SqShadowSampleOptions : private SqTextureSampleOptions
{
	/// \todo: Flesh this out when we eventually get around to redoing the shadow maps.
	// (The inherited members need to be pulled in with "using")
	TqFloat biasLow;
	TqFloat biasHigh;
};


//==============================================================================
// Implementation details.
//==============================================================================

// SqTextureSampleOptions implementation

inline SqTextureSampleOptions::SqTextureSampleOptions( TqFloat sblur, TqFloat tblur,
		TqFloat swidth, TqFloat twidth, EqTextureFilter filterType, TqFloat fill,
		TqInt startChannel, TqInt numChannels, TqInt numSamples,
		EqWrapMode sWrapMode, EqWrapMode tWrapMode)
	: sblur(sblur),
	tblur(tblur),
	swidth(swidth),
	twidth(twidth),
	filterType(filterType),
	fill(fill),
	startChannel(startChannel),
	numChannels(numChannels),
	numSamples(numSamples),
	sWrapMode(sWrapMode),
	tWrapMode(tWrapMode)
{ }

inline SqTextureSampleOptions::SqTextureSampleOptions()
	: sblur(0),
	tblur(0),
	swidth(0),
	twidth(0),
	filterType(TextureFilter_Gaussian),
	fill(0),
	startChannel(0),
	numChannels(1),
	numSamples(1),
	sWrapMode(WrapMode_Black),
	tWrapMode(WrapMode_Black)
{ }

} // namespace Aqsis

#endif // TEXTURESAMPLEOPTIONS_H_INCLUDED
