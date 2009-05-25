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
 * \brief Shadow texture sampler.
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef SHADOWSAMPLER_H_INCLUDED
#define SHADOWSAMPLER_H_INCLUDED

#include <aqsis/aqsis.h>

#include <aqsis/tex/filtering/ishadowsampler.h>
#include <aqsis/math/matrix.h>
#include <aqsis/tex/filtering/texturesampleoptions.h>

namespace Aqsis
{

template<typename>
class CqTileArray;

//------------------------------------------------------------------------------
/** \brief A sampler for shadow maps, implementing percentage closer filtering.
 */
class AQSIS_TEX_SHARE CqShadowSampler : public IqShadowSampler
{
	public:
		/** \brief Construct a shadow sampler with data from the provided file.
		 *
		 * \param file - file to obtain the shadow map data from.
		 * \param currToWorld - a matrix transforming the "current" coordinate
		 *                      system to the world coordinate system.  Sample
		 *                      quads are assumed to be passed to the sample()
		 *                      function represented in the "current"
		 *                      coordinate system.
		 */
		CqShadowSampler(const boost::shared_ptr<IqTiledTexInputFile>& file,
				const CqMatrix& currToWorld);

		// inherited
		virtual void sample(const Sq3DSampleQuad& sampleQuad,
				const CqShadowSampleOptions& sampleOpts, TqFloat* outSamps) const;
		virtual const CqShadowSampleOptions& defaultSampleOptions() const;
	private:
		/// transformation: current -> light coordinates
		CqMatrix m_currToLight;
		/// transformation: current -> raster coordinates ( [0,1]x[0,1] )
		CqMatrix m_currToTexture;
		/// Pixel data for shadow map.
		boost::shared_ptr<CqTileArray<TqFloat> > m_pixelBuf;
		/// Default shadow sampling options.
		CqShadowSampleOptions m_defaultSampleOptions;
};


} // namespace Aqsis

#endif // SHADOWSAMPLER_H_INCLUDED
