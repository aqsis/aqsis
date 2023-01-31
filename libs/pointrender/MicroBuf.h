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

#ifndef MICROBUF_H_
#define MICROBUF_H_


#include <cfloat>
#include <cmath>
#include <cstring>

#include <boost/scoped_array.hpp>

#include <Imath/ImathMath.h>
#include <Imath/ImathVec.h>
#include <Imath/ImathColor.h>


namespace Aqsis {


/**
 * \brief A MicroBuf object is an axis-aligned cube environment buffer.
 *
 * Each face has a coordinate system where the centres of the boundary pixels
 * lie just _inside_ the boundary.  That is, the positions of the pixel point
 * samples are at x_i = (1/2 + i/N) for i = 0 to N-1.
 *
 * For example, with 3x3 pixel faces, each face looks like:
 *
 * <pre>
 *
 *    +-----------+
 *    | x   x   x |
 *    |           |
 *    | x   x   x |
 *    |           |
 *    | x   x   x |
 *    +-----------+
 *
 * </pre>
 *
 * The x's represent the positions at which point sampling will occur.
 * The orientation of the faces is chosen so that they have a consistent
 * coordinate system when laid out in the following unfolded net of faces,
 * viewed from the inside of the cube.  (NB that for other possible nets the
 * coordinates of neighbouring faces won't be consistent.)
 *
 * <pre>
 *
 *               +---+
 *    ^          |+y |
 *    |  +---+---+---+---+
 *   v|  |-z |-x |+z |+x |
 *    |  +---+---+---+---+
 *    |          |-y |
 *    |          +---+
 *    |     u
 *    +-------->
 *
 * </pre>
 *
 * With the following enum indexes:
 *
 * <pre>
 *               +---+
 *    ^          | 1 |
 *    |  +---+---+---+---+
 *   v|  | 5 | 3 | 2 | 0 |
 *    |  +---+---+---+---+
 *    |          | 4 |
 *    |          +---+
 *    |     u
 *    +-------->
 *
 * </pre>
 *
 *
 */

class MicroBuf {

public:

	static const Imath::V3f Normal;

private:

	// Square face resolution
	int m_res;
	// Number of channels per pixel
	int m_nchans;
	// Number of floats needed to store a face
	int m_faceSize;
	// Pixel face storage
	boost::scoped_array<float> m_pixels;
	// Pixel face storage of the default pixels (black).
	boost::scoped_array<float> m_defaultPixels;
	// Storage for pixel ray directions
	boost::scoped_array<Imath::V3f> m_directions;
	// Pixels on a unit cube are not all equal in angular size
	boost::scoped_array<float> m_pixelSizes;

public:

	/**
	 * The identifiers for each cube face direction
	 */
	enum Face {
		Face_xp, //< x+   0
		Face_yp, //< y+   1
		Face_zp, //< z+   2
		Face_xn, //< x-   3
		Face_yn, //< y-   4
		Face_zn, //< z-   5
		Face_end,
		Face_begin = Face_xp
	};


	/**
	 * Constructor of the MicroBuf object.
	 *
	 * @param faceRes
	 * 			The resolution of the faces, faces are square.
	 * @param nchans
	 * 			The number of channels in each pixel.
	 * @param defaultPix
	 * 			A pointer to an array of values of the channels which will be used when MicroBuf::reset() is called.
	 */
	MicroBuf(int faceRes, int nchans, const float* defaultPix);

	/**
	 * Constructor used to make a copy of another MicroBuf.
	 *
	 * @param microbuf
	 * 			The reference to the other MicroBuf.
	 */
	MicroBuf(MicroBuf& microbuf);

	/**
	 * Reset the buffer to the passed values in pixels.
	 *
	 * @param pixels
	 * 			A pointer to an array of the new values of the channels.
	 */
	void reset(float* pixels);

	/**
	 *  Reset the buffer to the default (non-rendered) state, i.e. to the values passed in defaultPix during the construction of MicroBuf.
	 */
	void reset();

	/**
	 * Get the raw data of the channels for face
	 *
	 * @param which
	 * 			The index indicating which face to pass the data for, see MicroBuf::Face.
	 * @result The pointer to the raw data of the channels.
	 */
	float* face(Face which);

	/**
	 * Get the raw data of the channels for face
	 *
	 * @param which
	 * 			The index indicating which face to pass the data for, see MicroBuf::Face.
	 * @result The pointer to the raw data of the channels.
	 */
	const float* face(Face which) const;

	/**
	 * Get index of the face that is intersected by direction p.
	 *
	 * @param p
	 * 			The direction for which the index of the face should be returned.
	 * @result The face for direction p.
	 */
	static Face faceIndex(Imath::V3f p);

	/**
	 * Get a neighbouring face in the 'u' direction.
	 *
	 * <pre>
	 *     +---+---+---+  +---+---+---+  +---+---+---+
	 *     |+z |+x |-z |  |-x |+y |+x |  |-x |+z |+x |
	 *     +---+---+---+  +---+---+---+  +---+---+---+
	 *
	 *     +---+---+---+  +---+---+---+  +---+---+---+
	 *     |-z |-x |+z |  |-x |-y |+x |  |+x |-z |-x |
	 *     +---+---+---+  +---+---+---+  +---+---+---+
	 *
	 * </pre>
	 *
	 * @param face
	 * 		The current face.
	 * @param side
	 * 		The sude to look for (0 == left, 1 == right)
	 * @result
	 * 		The neighboring face in the direction of the 'u' axis.
	 */
	static Face neighbourU(Face face, int side);


	/**
	  * Get a neighbouring face in the 'v' direction.
	  *
	  * <pre>
	  *     +---+   +---+   +---+   +---+   +---+   +---+
	  *     |+y |   |-z |   |+y |   |+y |   |+z |   |+y |
	  *     +---+   +---+   +---+   +---+   +---+   +---+
	  *		|+x |   |+y |   |+z |   |-x |   |-y |   |-z |
	  *     +---+   +---+   +---+   +---+   +---+   +---+
	  *     |-y |   |+z |   |-y |   |-y |   |-z |   |-y |
	  *     +---+   +---+   +---+   +---+   +---+   +---+
	  *
	  * </pre>
	  *
	  * @param face
	  * 		The current face.
	  * @param side
	  * 		The sude to look for (0 == bottom, 1 == top)
	  * @result
	  * 		The neighboring face in the direction of the 'v' axis.
	  */
	 static Face neighbourV(Face face, int side);



	/**
	 * Get the coordinates (u,v) on face with faceIdx for the direction p.
	 *
	 * The coordinates are in the range -1 <= u,v <= 1, if faceIdx is
	 * obtained using the faceIndex function.  Coordinates outside this
	 * range are legal, as long as p has nonzero component in the
	 * direction of the normal of the face.
	 *
	 * @param face
	 * 			The face on which the coordinates should be located.
	 * @param p
	 * 			The position (may lie outside cone of current face).
	 * @param u
	 * 			The resulting location on the 'u' axis.
	 * @param v
	 * 			The resulting location on the 'v' axis.
	 */
	static void faceCoords(Face face, Imath::V3f p, float& u, float& v);

	/**
	 * Compute the dot product of the vector vec with the face normal of the face indicated by faceIdx.
	 *
	 * @param face
	 * 			The face from which the normal for the vector product should be taken.
	 * @param vec
	 * 			The vector that should be used for the vector product with the face normal.
	 * @result The vector product of the vector vec with the normal of the face indicated by faceIdx.
	 */
	static float dotFaceNormal(Face face, Imath::V3f vec);

	 /**
	  * Compute the normal of the face.
	  *
	  * @param face
	  * 		The face for which the normal needs to be calculated.
	  * @result The normal of the face.
	  */
	static Imath::V3f faceNormal(Face face);

	/**
	 * Get the direction vector for a pixel on a given face.
	 *
	 * @param face
	 * 			The face on which the pixel lies.
	 * @param u
	 * 			The position of the pixel on the 'u' axis.
	 * @param v
	 * 			The position of the pixel on the 'v' axis.
	 * @result The direction vector to the pixel.
	 */
	Imath::V3f rayDirection(Face face, int u, int v) const;


	/**
	 * Return the relative size of pixel.
	 *
	 * Compared to a pixel in the middle of the cube face, pixels in the
	 * corners of the cube have a smaller angular size.  We must take
	 * this into account when integrating the radiosity.
	 *
	 * @param u
	 * 			The position of the pixel on the 'u' axis.
	 * @param v
	 * 			The position of the pixel on the 'v' axis.
	 *
	 * @result The relative size of the pixel, compared to a pixel in the center of the face.
	 */
	float pixelSize(int u, int v) const;

	/**
	 * Reorder vector components into "canonical face coordinates".
	 *
	 * The canonical coordinates correspond to the coordinates on the +z face.
	 * If we let the returned vector be q then (q.x, q.y) corresponds to the
	 * face (u, v) coordinates, and q.z corresponds to the signed depth out
	 * from the face.
	 *
	 * @param faceIdx
	 * 			The index of the face in the direction of p.
	 * @param p
	 * 			The direction for which to retrieve the canonicalFaceCoords.
	 * @result The canonical face coordinates.
	 */
	static Imath::V3f canonicalFaceCoords(Face face, Imath::V3f p);

	/**
	 * Get a pointer to the raw data of the channels of the MicroBuf object.
	 *
	 * @result A pointer to the raw data of the channels of the MicroBuf object.
	 */
	float* getRawData();

	/**
	 * Get the resolution of the faces, the number of pixels on one side.
	 *
	 * @result The number of pixels on one side of a face.
	 */
	int getFaceResolution() const;

	/**
	 * The number of channels of this MicroBuf object.
	 *
	 * @result The number of channels for each pixel.
	 */
	int getNChans() const;

	/**
	 * Get the total amount of pixels of this MicroBuf object.
	 *
	 * @result The total amount of pixels of this MicroBuf object.
	 */
	int size() const;

	/**
	 * Destroy the MicroBuf object.
	 */
	virtual ~MicroBuf();

private:
	/**
	 * Get the direction vector for a position on a given face.
	 * Roughly speaking, this is the opposite of the faceCoords function.
	 *
	 * @param face
	 * 			The face on which the position lies.
	 * @param u
	 * 			The position on the 'u' axis.
	 * @param v
	 * 			The position on the 'v' axis.
	 * @result The direction to the position on the face.
	 */
	static Imath::V3f direction(Face face, float u, float v);

};

// a used inline helper function for the dot product.
inline float dot(Imath::V3f a, Imath::V3f b) {
	return a ^ b;
}


}
#endif /* MICROBUF_H_ */
