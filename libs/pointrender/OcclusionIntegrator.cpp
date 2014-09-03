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

#include <cfloat>
#include <cmath>
#include <cstring>

#include <boost/scoped_array.hpp>

#include "OcclusionIntegrator.h"

namespace Aqsis {

using Imath::V3f;
using Imath::C3f;
using Aqsis::MicroBuf;


OcclusionIntegrator::OcclusionIntegrator(int faceRes) :
	m_buf(faceRes, 1, defaultPixel()), m_face(0) {
	clear();
}

V3f OcclusionIntegrator::rayDirection(MicroBuf::Face face, int u, int v) {
	return m_buf.rayDirection(face, u, v);
}

const MicroBuf& OcclusionIntegrator::microBuf() {
	return m_buf;
}

int OcclusionIntegrator::res() {
	return m_buf.getFaceResolution();
}

void OcclusionIntegrator::clear() {
	m_buf.reset();
}

void OcclusionIntegrator::setFace(MicroBuf::Face face) {
	m_face = m_buf.face(face);
}

void OcclusionIntegrator::addSample(int u, int v, float distance,
		float coverage) {
	float* pix = m_face + (v * m_buf.getFaceResolution() + u) * m_buf.getNChans();
	// There's more than one way to combine the coverage.
	//
	// 1) The usual method of compositing.  This assumes that
	// successive layers of geometry are uncorrellated so that each
	// attenuates the layer before, but a bunch of semi-covered layers
	// never result in full opacity.
	//
	// 1 - (1 - o1)*(1 - o2);
	//
	// 2) Add the opacities (and clamp to 1 at the end).  This is more
	// appropriate if we assume that we have adjacent non-overlapping
	// surfels.
	pix[0] += coverage;
}

float OcclusionIntegrator::occlusion(V3f N, float coneAngle) const {
	// Integrate over face to get occlusion.
	float occ = 0;
	float totWeight = 0;
	float cosConeAngle = std::cos(coneAngle);
	for (int iface = MicroBuf::Face_begin; iface < MicroBuf::Face_end; ++iface) {
		MicroBuf::Face face = static_cast<MicroBuf::Face>(iface);
		const float* facep = m_buf.face(face);
		for (int iv = 0; iv < m_buf.getFaceResolution(); ++iv)
			for (int iu = 0; iu < m_buf.getFaceResolution(); ++iu, facep += m_buf.getNChans()) {
				float d = dot(m_buf.rayDirection(face, iu, iv), N) - cosConeAngle;
				if (d > 0) {
					d *= m_buf.pixelSize(iu, iv);
					// Accumulate light coming from infinity.
					occ += d * std::min(1.0f, facep[0]);
					totWeight += d;
				}
			}
	}
	if (totWeight != 0)
		occ /= totWeight;
	return occ;
}

OcclusionIntegrator::~OcclusionIntegrator() {

}

}
