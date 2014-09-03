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

#include <aqsis/util/logging.h>

#include "RadiosityIntegrator.h"

namespace Aqsis {

using Imath::V3f;
using Imath::C3f;
using Aqsis::MicroBuf;

RadiosityIntegrator::RadiosityIntegrator(int faceRes):
		m_buf(faceRes, 5, defaultPixel()),
		m_face(0),
		m_currRadiosity(0) {
	clear();
}

RadiosityIntegrator::RadiosityIntegrator(MicroBuf& microbuffer):
		m_buf(microbuffer),
		m_face(0),
		m_currRadiosity(0) {
}

V3f RadiosityIntegrator::rayDirection(MicroBuf::Face face, int u, int v) {
	return m_buf.rayDirection(face, u, v);
}

const MicroBuf& RadiosityIntegrator::microBuf() {
	return m_buf;
}

int RadiosityIntegrator::res() const {
	return m_buf.getFaceResolution();
}

void RadiosityIntegrator::clear() {
	m_buf.reset();
}

void RadiosityIntegrator::addSample(int u, int v, float distance, float coverage) {
	float* pix = m_face + (v * m_buf.getFaceResolution() + u) * m_buf.getNChans();
	// TODO: Eventually remove dist if not needed
	float& currDist = pix[0];
	float& currCover = pix[1];
	C3f & radiosity = *reinterpret_cast<C3f*> (pix + 2);
	if (distance < currDist)
		currDist = distance;
	if (currCover < 1) {
		if (currCover + coverage <= 1) {
			radiosity += coverage * m_currRadiosity;
			currCover += coverage;
		} else {
			radiosity += (1 - currCover) * m_currRadiosity;
			currCover = 1;
		}
	}
}

void RadiosityIntegrator::setFace(MicroBuf::Face face) {
	m_face = m_buf.face(face);
}

void RadiosityIntegrator::setPointData(const float * radiosity) {
	m_currRadiosity = C3f(radiosity[0], radiosity[1], radiosity[2]);
}

C3f RadiosityIntegrator::radiosity(V3f N, float coneAngle, float* occlusion) const {
	// Integrate incoming light with cosine weighting to get outgoing
	// radiosity
	C3f rad(0);
	float totWeight = 0;
	float cosConeAngle = std::cos(coneAngle);
	float occ = 0;
	for (int iface = MicroBuf::Face_begin; iface < MicroBuf::Face_end; ++iface) {
		MicroBuf::Face face = static_cast<MicroBuf::Face>(iface);
		const float* facep = m_buf.face(face);
		for (int iv = 0; iv < m_buf.getFaceResolution(); ++iv)
			for (int iu = 0; iu < m_buf.getFaceResolution(); ++iu, facep += m_buf.getNChans()) {
				float d = dot(m_buf.rayDirection(face, iu, iv), N) - cosConeAngle;
				if (d > 0) {
					d *= m_buf.pixelSize(iu, iv);
					C3f & radiosity = *(C3f*) (facep + 2);
					rad += d * radiosity;
					occ += d * facep[1];
					totWeight += d;
				}
			}
	}
	if (totWeight != 0) {
		occ /= totWeight;
		rad = (1.0f / totWeight) * rad;
	}
	if (occlusion)
		*occlusion = occ;
	return rad;
}


RadiosityIntegrator::~RadiosityIntegrator() {

}

}
