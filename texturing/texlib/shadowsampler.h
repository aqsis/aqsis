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

#include "aqsis.h"

#include "ishadowsampler.h"
#include "matrix.h"
#include "texturesampleoptions.h"

namespace Aqsis
{

template<typename>
class CqTextureBuffer;

class CqShadowSampler : public IqShadowSampler
{
	public:
		CqShadowSampler(const boost::shared_ptr<IqTexInputFile>& file,
				const CqMatrix& camToWorld);

		// From IqShadowSampler
		virtual void sample(const Sq3DSampleQuad& sampleQuad,
				const CqShadowSampleOptions& sampleOpts, TqFloat* outSamps) const;

		virtual const CqShadowSampleOptions& defaultSampleOptions() const;
	private:
		/// transformation: camera -> light coordinates
		CqMatrix m_camToLight;
		/// transformation: camera -> shadow map raster coordinates
		CqMatrix m_camToLightRaster;
		/// Pixel data for shadow map.
		boost::shared_ptr<CqTextureBuffer<TqFloat> > m_pixelBuf;
		/// Default shadow sampling options.
		CqShadowSampleOptions m_defaultSampleOptions;
};


} // namespace Aqsis

#endif // SHADOWSAMPLER_H_INCLUDED
