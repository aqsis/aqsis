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


#ifndef RADIOSITYINTEGRATOR_H_
#define RADIOSITYINTEGRATOR_H_

#include "MicroBuf.h"


namespace Aqsis {

/**
 * This integrator calculates the indirect radiosity.
 *
 * The job of an integrator class is to save the microrasterized data
 * somewhere (presumably in a microbuffer) and integrate it at the end of the
 * rasterization to give the final illumination value.
 *
 */
class RadiosityIntegrator {

private:

	/**
	 * This method returns the default (empty) pixel values for RadiosityIntegrators.
	 *
	 * @return 5 channel float values, the first channel represents the depth (MAX),
	 * the second channel represents the coverage, the last three channels represent
	 * the RGB radiosity.
	 */
	static float* defaultPixel() {
		// depth, foreground_coverage, foreground_rgb,
		static float def[] = {FLT_MAX, 0, 0, 0, 0};
		return def;
	}

    MicroBuf m_buf; //< The inner microbuffer of this integrator.
    float* m_face; //< A reference to the inner data of the microbuffer.
    Imath::C3f m_currRadiosity; //< The value of the radiosity of the current sample.


public:


	/**
	 * Create an RadiosityIntegrator with an inner microbuffer that has a face
	 * resolution of \p faceres.
	 *
	 * @param faceRes
	 * 			The face resolution of the inner microbuffer (@see MicroBuf::getFaceRes()).
	 */
        RadiosityIntegrator(int faceRes);

        /**
         * Create the integrator from a given microbuffer.
         *
         * @param microbuffer
         * 			The inner microbuffer to be used for this RadiosityIntegrator.
         */
        RadiosityIntegrator(MicroBuf& microbuffer);

        /**
		 * @see MicroBuf::rayDirection(MicroBuf::Face, int, int)
		 */
        Imath::V3f rayDirection(MicroBuf::Face face, int u, int v);

	/**
	 * Get a reference to the underlying microbuffer.
	 *
	 * @return The inner MicroBuf object.
	 */
	const MicroBuf& microBuf();

	/**
	 * @see MicroBuf::getFaceRes()
	 */
	int res() const;

	/**
	 * Reset the integrator to it's empty initial state.
	 */
	void clear();

	/**
	 * Set the face to which subsequent calls of addSample will apply, @see addSample().
	 *
	 * @param face
	 * 			The face to set.
	 */
	void setFace(MicroBuf::Face face);

	/**
		 * This method indicates that a rasterized sample covers a certain pixel of the microbuffer,
		 * on the face indicated by a previous call of @see setFace()
		 *
		 * @param u
		 * 			The position of the pixel on the 'u' axis.
		 * @param v
		 * 			The position of the pixel on the 'v' axis.
		 * @param distance
		 * 			The distance to the sample.
		 * @param coverage
		 * 			The coverage of the pixel by the sample.
		 */
		void addSample(int u, int v, float distance, float coverage);


		/**
		 * Set the data for the current sample, @see addSample().
		 *
		 * This will be a pointer to three float values, indicating the RGB radiosity.
		 *
		 * @param data
		 * 			A reference to a float array.
		 */
        void setPointData(const float* radiosity);

        /**
         * Integrate the radiosity based on the previously sampled scene.
         *
         * @param N
         * 			The shading normal, the normal of the surface on which this
         * 			integrator is located.
         * @param coneAngle
         * 			The angle over which to consider the occlusion in radians. If
         * 			coneAngle is not PI/2 radians, the usual cos(theta) weighting is
         * 			adjusted to (cos(theta) - cos(coneAngle)) so that it falls
         * 			continuously to zero when theta == coneAngle
         * @param occlusion
         * 			If this value is non-null, the amount of occlusion will also be
         * 			computed and stored.
         * @return
         */
        Imath::C3f radiosity(Imath::V3f N, float coneAngle, float* occlusion = 0) const;

        /**
         * Delete the RadiosityIntegrator.
         */
        virtual ~RadiosityIntegrator();

};

}
#endif /* RADIOSITYINTEGRATOR_H_ */
