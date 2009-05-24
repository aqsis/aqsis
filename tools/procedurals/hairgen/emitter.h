// hairgen procedural
// Copyright (C) 2008 Christopher J. Foster [chris42f (at) gmail (d0t) com]
//
// This software is licensed under the GPLv2 - see the file COPYING for details.

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
		EmitterMesh(const IntArray& nverts, const IntArray& verts,
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
		void createFaceList(const IntArray& nverts,
				const IntArray& verts, FaceVec& faces) const;

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
