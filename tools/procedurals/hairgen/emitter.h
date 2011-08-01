// Aqsis
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

#ifndef EMITTER_H_INCLUDED
#define EMITTER_H_INCLUDED

#include <aqsis/math/lowdiscrep.h>

#include "primvar.h"
#include "util.h"


//------------------------------------------------------------------------------
/** Emitter for particles at random positions on a mesh.
 *
 * Particles are generated on the faces of the mesh so that the distribution is
 * as even as possible (the number of particles on a face is proportional to
 * the area of the face).  For each face, particles are distributed according
 * to a randomized quasi-random sequence, which makes them pretty even.
 *
 * After choosing a point on a face, interpolation coefficients for each of the
 * vertices are determined 
 */
class EmitterMesh
{
	public:
		/** Create an emitting mesh 
		 *
		 * \param nverts - Vector of number of verts per face.  The length of
		 *                 this is the number of faces.  Currently this class
		 *                 can only handle faces with 3 or 4 vertices.
		 * \param verts - Indicies into primvars of class vertex specifying
		 *                which primvars go with which face.
		 * \param primVars - Primitive variables attached to the mesh.  This
		 *                must contain at least the primvar "P" or a
		 *                std::runtime_error is thrown.
		 * \param totParticles - Total number of particles to generate on the mesh.
		 */
		EmitterMesh(const Ri::IntArray& nverts, const Ri::IntArray& verts,
				boost::shared_ptr<PrimVars> primVars, int totParticles);

		/// Get the number of faces in the mesh
		int numFaces() const;

		/** Randomly generate particle positions on a face, and interpolate
		 * primvars from the mesh.
		 *
		 * The number of particles generated on the face is the total number of
		 * particles scaled by the area of the face so that the distribution is
		 * even over the whole mesh.
		 *
		 * \param faceIdx - index of the face in the mesh
		 * \return A set of interpolated primvars for random positions on the face.
		 */
		boost::shared_ptr<PrimVars> particlesOnFace(int faceIdx);

	private:
		struct MeshFace;
		typedef std::vector<MeshFace> FaceVec;

		float triangleArea(const int* v) const;
		float faceArea(const MeshFace& face) const;
		Vec3 faceNormal(const MeshFace& face) const;
		void createFaceList(const Ri::IntArray& nverts,
				const Ri::IntArray& verts, FaceVec& faces) const;

		/// Array of faces.
		FaceVec m_faces;
		/// Array of 3D vertex positions.
		std::vector<Vec3> m_P;
		boost::shared_ptr<PrimVars> m_primVars;
		int m_totParticles;
		Aqsis::CqLowDiscrepancy m_lowDiscrep;
};



//==============================================================================
// Implementation details
//==============================================================================
/// Hold information about a face in a mesh.
struct EmitterMesh::MeshFace
{
	int v[4];  ///< vertex indices for face.
	int faceVaryingIndex;    ///< first index into facevarying arrays
	int numVerts;            ///< number of verts for face
	float weight;            ///< face area / total mesh area

	MeshFace(const int* verts, int faceVaryingIndex,
			int numVerts, float weight = 0)
		: faceVaryingIndex(faceVaryingIndex),
		numVerts(numVerts),
		weight(weight)
	{
		assert(numVerts >= 3);
		v[0] = v[1] = v[2] = v[3] = 0;
		assert(numVerts <= 4);
		std::copy(verts, verts+numVerts, v);
	}
};


#endif // EMITTER_H_INCLUDED
