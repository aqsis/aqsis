// Menger Sponge primitive as renderman ProcDynamicLoad procedural geometry.
//
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

// Author: Chris Foster [chris42f (at) gmail (d0t) com]

#include <ri.h>

#include <sstream>
#include <iostream>
#include <cstring>

extern "C" AQSIS_EXPORT RtPointer ConvertParameters(char* initialdata);
extern "C" AQSIS_EXPORT void Subdivide(RtPointer blinddata, RtFloat detailsize);
extern "C" AQSIS_EXPORT void Free(RtPointer blinddata);


/** A simple vector class with overloaded operators
 */
struct Vec3
{
	RtFloat x;
	RtFloat y;
	RtFloat z;

	Vec3(RtFloat x, RtFloat y, RtFloat z) : x(x), y(y), z(z) {}
	Vec3(RtFloat a) : x(a), y(a), z(a) {}

	Vec3 operator+(const Vec3& rhs) const { return Vec3(x + rhs.x, y + rhs.y, z + rhs.z); }
	Vec3 operator-(const Vec3& rhs) const { return Vec3(x - rhs.x, y - rhs.y, z - rhs.z); }
	Vec3 operator*(float a) const { return Vec3(a*x, a*y, a*z); }
};

Vec3 operator*(float a, const Vec3& v) { return v*a; }


/** Draw an axis-aligned cube, defined by the the minimum and maximum
 * coordinates of the corners.  Only draw faces for which the face index in
 * the drawFace array is true.
 */
void drawCube(Vec3 min, Vec3 max, const bool drawFace[6])
{
	// Reference verts array we'd use if all faces are to be drawn
	RtInt vertsAll[] = {
		0, 3, 7, 4, // -x
		1, 5, 6, 2, // +x
		0, 1, 2, 3, // -y
		4, 7, 6, 5, // +y
		0, 4, 5, 1, // -z
		3, 2, 6, 7  // +z
	};
	// Fill in actual verts array.
	RtInt verts[24] = {0};
	int nfaces = 0;
	for(int i = 0; i < 6; ++i)
	{
		if(drawFace[i])
		{
			// Copy face
			verts[4*nfaces    ] = vertsAll[4*i    ];
			verts[4*nfaces + 1] = vertsAll[4*i + 1];
			verts[4*nfaces + 2] = vertsAll[4*i + 2];
			verts[4*nfaces + 3] = vertsAll[4*i + 3];
			++nfaces;
		}
	}
	// Number of vertices per face.  Technically should be of length nfaces,
	// but this will do since nfaces <= 6 always:
	RtInt nverts[] = {4, 4, 4, 4, 4, 4};
	// Array containing all faces
	RtFloat P[] = {
		min.x, min.y, min.z,
		max.x, min.y, min.z,
		max.x, min.y, max.z,
		min.x, min.y, max.z,
		min.x, max.y, min.z,
		max.x, max.y, min.z,
		max.x, max.y, max.z,
		min.x, max.y, max.z
	};
	RiPointsPolygons(nfaces, nverts, verts, "P", P, RI_NULL);
}


/// RiProcedural representing a genralized menger sponge fractal.
///
/// The usual menger sponge is generated recursively by breaking the current
/// cube up into 3x3x3 child cubes, and removing the centre child cubes on each
/// face as well as the central child cube.  A generalized version is created
/// by allowing the pattern of child cube removals to be specified arbitrarily.
/// In practise we can do this using a 3x3x3 array of booleans, with 1's
/// indicating a child cube should be created.  In this format, the traditional
/// menger sponge has the motif
///
/// 1 1 1   1 0 1   1 1 1
/// 1 0 1   0 0 0   1 0 1
/// 1 1 1   1 0 1   1 1 1
///
class MengerSponge
{
	private:
		// Level to which this sponge should be subdivided.  0 == no subdivision.
		int m_level;
		// minimum of axis-aligned box bounding the sponge
		Vec3 m_min;
		// maximum of axis-aligned box bounding the sponge
		Vec3 m_max;
		// Flags determining whether to draw faces.  Face order: -x +x -y +y -z +z
		bool m_drawFace[6];
		// The 3x3x3 pattern of on/off subcells
		bool m_motif[27];

	public:
		MengerSponge(int level, const Vec3& min, const Vec3& max,
					 const bool drawFace[6], const bool motif[27])
			: m_level(level),
			m_min(min),
			m_max(max)
		{
			memcpy(m_drawFace, drawFace, 6*sizeof(bool));
			memcpy(m_motif, motif, 27*sizeof(bool));
		}
		/** Extract sponge parameters from a string
		 *
		 * initString is in the form "level  x1 x2  y1 y1  z2 z2  motif"
		 *
		 * the motif is an array of 27 ones and zeros, as described above.
		 */
		MengerSponge(const char* initString)
			: m_level(1),
			m_min(-1),
			m_max(1)
		{
			// Set initial outside faces to be closed.
			for(int i = 0; i < 6; ++i)
				m_drawFace[i] = true;
			std::istringstream iss(initString);
			iss >> m_level
				>> m_min.x >> m_max.x
				>> m_min.y >> m_max.y
				>> m_min.z >> m_max.z;
			// Input 27 ones or zeros as the motif.
			for(int i = 0; i < 27; ++i)
				iss >> m_motif[i];
			if(!iss)
			{
				// Oops, input failure; use the standard motif instead.
				bool stdMotif[] = {
					1,1,1, 1,0,1, 1,1,1,
					1,0,1, 0,0,0, 1,0,1,
					1,1,1, 1,0,1, 1,1,1,
				};
				memcpy(m_motif, stdMotif, 27*sizeof(bool));
			}
		}

		/** Fill a renderman bound array with the bound for the current
		 * primitive.
		 */
		void bound(RtBound bnd) const
		{
			bnd[0] = m_min.x;
			bnd[1] = m_max.x;
			bnd[2] = m_min.y;
			bnd[3] = m_max.y;
			bnd[4] = m_min.z;
			bnd[5] = m_max.z;
		}

		/// Utility to return index modulo 3.  Assumes i >= -3.
		static inline int wrap3(int i) { return (i + 3) % 3; };

		/// Return an element of the 3x3 subdivision motif.
		bool motif(int ix, int iy, int iz) const
		{
			return m_motif[3*(3*wrap3(iz) + wrap3(iy)) + wrap3(ix)];
		}

		/** Split the procedural up according to the fractal subdivision rules.
		 *
		 * We iterate through all of the set of 3x3x3 sub-cubes, deciding which
		 * ones to draw based on the fractal recursion relation.  Each drawn
		 * subcube is subdivided in the same way until leaf nodes of the
		 * subdivision tree are reached.
		 */
		void subdivide() const
		{
			if(m_level <= 0) // || m_level/4.0 < std::rand()/float(RAND_MAX))
				drawCube(m_min, m_max, m_drawFace);
			else
			{
				Vec3 diag = (1/3.0)*(m_max - m_min);
				// For each child in 3x3 grid of children...
				for(int iz = 0; iz < 3; ++iz)
				for(int iy = 0; iy < 3; ++iy)
				for(int ix = 0; ix < 3; ++ix)
				{
					if(motif(ix, iy, iz))
					{
						// Figure out which faces need to be drawn for the
						// child cube.  This depends on which boundary of the
						// current cube the child touches, and whether a new
						// face will be created due to a neighbouring child
						// being turned off by the motif.
						bool drawFace[6] = {
							(ix == 0 && m_drawFace[0]) || motif(ix-1, iy, iz) == 0,
							(ix == 2 && m_drawFace[1]) || motif(ix+1, iy, iz) == 0,
							(iy == 0 && m_drawFace[2]) || motif(ix, iy-1, iz) == 0,
							(iy == 2 && m_drawFace[3]) || motif(ix, iy+1, iz) == 0,
							(iz == 0 && m_drawFace[4]) || motif(ix, iy, iz-1) == 0,
							(iz == 2 && m_drawFace[5]) || motif(ix, iy, iz+1) == 0,
						};
						// Create a child of the current sponge.  ix, iy
						// and iz are the integer coordinates of the child
						// within an even 3x3x3 subdivision of the current
						// bounding box.  RiProcedural() is called recursively
						// to add the subdivided child primitives to the render
						// pipeline.
						Vec3 newMin = m_min + Vec3(ix*diag.x, iy*diag.y,
												   iz*diag.z);
						MengerSponge* ms = new MengerSponge(m_level-1, newMin,
															newMin + diag,
															drawFace, m_motif);
						RtBound bnd;
						ms->bound(bnd);
						RiProcedural(ms, bnd, Subdivide, Free);
					}
				}
			}
		}
};


//------------------------------------------------------------------------------
/**
 * Any procedural which will be accessed via the RiProcDynamicLoad procedural
 * type must provide three functions: ConvertParameters, Subdivide, and Free;
 * here they are.
 */

/** ConvertParameters()
 *
 * Converts a string of initialization data for the procedural into whatever
 * internal data the procedural needs.  This data is sent back to the renderer
 * as an opaque pointer, and will be passed onto the Subdivide() and Free()
 * functions.
 */
extern "C" RtPointer ConvertParameters(char* initialdata)
{
	MengerSponge* params = new MengerSponge(initialdata);
	return reinterpret_cast<RtPointer>(params);
}

/** Subdivide()
 *
 * Splits the procedural into smaller primitives which are inserted
 * into the renderer pipeline via the RI.  These can be any primitive type
 * supported by the renderer, including further procedurals.
 */
extern "C" void Subdivide(RtPointer blinddata, RtFloat detailsize)
{
	const MengerSponge* p = reinterpret_cast<MengerSponge*>(blinddata);
	p->subdivide();
}

/** Free()
 *
 * Frees the data pointed to by the handle which was allocated inside
 * ConvertParameters().
 */
extern "C" void Free(RtPointer blinddata)
{
	delete reinterpret_cast<MengerSponge*>(blinddata);
}

