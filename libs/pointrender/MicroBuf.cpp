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

#include <Imath/ImathVec.h>
#include <Imath/ImathMath.h>
#include <Imath/ImathFun.h>

#include "MicroBuf.h"

namespace Aqsis {

using Imath::V3f;
using Imath::C3f;

const V3f MicroBuf::Normal = Imath::V3f(0,0,-1);

MicroBuf::MicroBuf(int faceRes, int nchans, const float* defaultPix) :
	m_res(faceRes), m_nchans(nchans), m_faceSize(nchans * faceRes * faceRes),
			m_pixels() {

	// Create the arrays.
	m_pixels.reset(new float[m_faceSize * Face_end]);
	m_defaultPixels.reset(new float[m_faceSize * Face_end]);
	m_directions.reset(new V3f[Face_end * faceRes * faceRes]);
	m_pixelSizes.reset(new float[m_faceSize]);

	// Cache direction vectors
	for (int iface = Face_begin; iface < Face_end; ++iface) {
		Face face = static_cast<Face>(iface);
		for (int iv = 0; iv < m_res; ++iv)
			for (int iu = 0; iu < m_res; ++iu) {
				// directions of pixels go through pixel centers
				float u = (0.5f + iu) / faceRes * 2.0f - 1.0f;
				float v = (0.5f + iv) / faceRes * 2.0f - 1.0f;
				m_directions[(face * m_res + iv) * m_res + iu] = direction(
						face, u, v);
			}
	}

	// Calculate the solid angle for each pixel.
	for (int iv = 0; iv < m_res; ++iv) {
		for (int iu = 0; iu < m_res; ++iu) {
			float u = (0.5f + iu) / faceRes * 2.0f - 1.0f;
			float v = (0.5f + iv) / faceRes * 2.0f - 1.0f;
			float d2 = V3f(u, v, 1).length2(); // squared length to the pixel center
			float cosFac = dotFaceNormal(Face_zp, rayDirection(Face_zp, iu, iv)); // cos between facenormal and dir
			float area = (2.f/faceRes)*(2.f/faceRes); // area of a pixel
			m_pixelSizes[iv * m_res + iu] =  ((area*cosFac)/d2);
			// m_pixelSizes[iv * m_res + iu] =  1.f/d2; // legacy val
		}
	}

	// Set the default channel values for the pixels.
	float* pix = m_defaultPixels.get();
	for (int i = 0, iend = size(); i < iend; ++i, pix += m_nchans)
		for (int c = 0; c < m_nchans; ++c)
			pix[c] = defaultPix[c];
}

MicroBuf::MicroBuf(MicroBuf& microbuf) :
			m_res(microbuf.m_res),
			m_nchans(microbuf.m_nchans),
			m_faceSize(microbuf.m_faceSize),
			m_pixels(microbuf.getRawData()) {
}

void MicroBuf::reset(float* pixels) {
	m_pixels.reset(pixels);
}

void MicroBuf::reset() {
	memcpy(m_pixels.get(), m_defaultPixels.get(), sizeof(float) * size()
			* m_nchans);
}

float * MicroBuf::getRawData() {
	return &m_pixels[0];
}

float* MicroBuf::face(Face which) {
	assert(which >= Face_begin && which < Face_end);
	return &m_pixels[0] + which * m_faceSize;
}

const float* MicroBuf::face(Face which) const {
	assert(which >= Face_begin && which < Face_end);
	return &m_pixels[0] + which * m_faceSize;
}

MicroBuf::Face MicroBuf::faceIndex(V3f p) {
	V3f absp = V3f(fabs(p.x), fabs(p.y), fabs(p.z));
	if (absp.x >= absp.y && absp.x >= absp.z)
		return (p.x > 0) ? Face_xp : Face_xn;
	else if (absp.y >= absp.x && absp.y >= absp.z)
		return (p.y > 0) ? Face_yp : Face_yn;
	else {
		assert(absp.z >= absp.x && absp.z >= absp.y);
		return (p.z > 0) ? Face_zp : Face_zn;
	}
}

MicroBuf::Face MicroBuf::neighbourU(Face face, int side) {
	static Face neighbourArray[6][2] = { { Face_zp, Face_zn }, { Face_xn,
			Face_xp }, { Face_xn, Face_xp }, { Face_zn, Face_zp }, { Face_xn,
			Face_xp }, { Face_xp, Face_xn } };
	return neighbourArray[face][side];
}

MicroBuf::Face MicroBuf::neighbourV(Face face, int side) {
	static Face neighbourArray[6][2] = { { MicroBuf::Face_yn, Face_yp }, {
			Face_zp, Face_zn }, { Face_yn, Face_yp }, { Face_yn, Face_yp }, {
			Face_zn, Face_zp }, { Face_yn, Face_yp } };
	return neighbourArray[face][side];
}

void MicroBuf::faceCoords(Face face, V3f p, float& u, float& v) {
	p = canonicalFaceCoords(face, p);
	assert(p.z != 0);
	float zinv = 1.0 / p.z;
	u = p.x * zinv;
	v = p.y * zinv;
}

float MicroBuf::dotFaceNormal(Face face, V3f vec) {
	assert(face < Face_end && face >= Face_begin);
	return (face < 3) ? vec[face] : -vec[face - 3];
}

V3f MicroBuf::faceNormal(Face face) {
	static V3f normals[6] = { V3f(1, 0, 0), V3f(0, 1, 0), V3f(0, 0, 1), V3f(-1,
			0, 0), V3f(0, -1, 0), V3f(0, 0, -1) };
	return normals[face];
}

V3f MicroBuf::rayDirection(Face face, int u, int v) const {
	return m_directions[(face * m_res + v) * m_res + u];
}

float MicroBuf::pixelSize(int u, int v) const {
	return m_pixelSizes[m_res * v + u];
}

V3f MicroBuf::canonicalFaceCoords(Face face, V3f p) {
	switch (face) {
	case Face_xp:
		return V3f(-p.z, p.y, p.x);
	case Face_xn:
		return V3f(-p.z, -p.y, p.x);
	case Face_yp:
		return V3f(p.x, -p.z, p.y);
	case Face_yn:
		return V3f(-p.x, -p.z, p.y);
	case Face_zp:
		return V3f(p.x, p.y, p.z);
	case Face_zn:
		return V3f(p.x, -p.y, p.z);
	default:
		assert(0 && "invalid face");
		return V3f();
	}
}

int MicroBuf::getFaceResolution() const {
	return m_res;
}

int MicroBuf::getNChans() const {
	return m_nchans;
}

int MicroBuf::size() const {
	return Face_end * m_res * m_res;
}

V3f MicroBuf::direction(Face face, float u, float v) {
	switch (face) {
	case Face_xp:
		return V3f(1, v, -u).normalized();
	case Face_yp:
		return V3f(u, 1, -v).normalized();
	case Face_zp:
		return V3f(u, v, 1).normalized();
	case Face_xn:
		return V3f(-1, v, u).normalized();
	case Face_yn:
		return V3f(u, -1, v).normalized();
	case Face_zn:
		return V3f(-u, v, -1).normalized();
	default:
		assert(0 && "unknown face");
		return V3f();
	}
}

MicroBuf::~MicroBuf() {
	// The MicroBuf object uses scoped arrays, no destruction necessary.
}

}
