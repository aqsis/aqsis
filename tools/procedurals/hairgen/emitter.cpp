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

#include "emitter.h"

//------------------------------------------------------------------------------
// EmitterMesh Implementation

EmitterMesh::EmitterMesh(
		const Ri::IntArray& nverts, const Ri::IntArray& verts,
		boost::shared_ptr<PrimVars> primVars, int totParticles)
	: m_faces(),
	m_P(),
	m_primVars(primVars),
	m_totParticles(totParticles),
	m_lowDiscrep(2)
{
	// initialize vertex positions
	const FloatArray* P = primVars->findPtr(
			Aqsis::CqPrimvarToken(Aqsis::class_vertex, Aqsis::type_point, 1, "P") );
	if(!P)
		throw std::runtime_error("\"vertex point[1] P\" must be present"
				"in parameter list for mesh");
	m_P.reserve(P->size()/3);
	for(int j = 0, endP = P->size(); j+2 < endP; j += 3)
		m_P.push_back(Vec3((*P)[j], (*P)[j+1], (*P)[j+2]));

	// Finally, create the list of faces
	// TODO: Ugly, since it uses m_P internally!
	createFaceList(nverts, verts, m_faces);
}

int EmitterMesh::numFaces() const
{
	return m_faces.size();
}

boost::shared_ptr<PrimVars> EmitterMesh::particlesOnFace(int faceIdx)
{
	const MeshFace& face = m_faces[faceIdx];

	boost::shared_ptr<PrimVars> interpVars(new PrimVars());

	float numParticlesCts = face.weight*m_totParticles;
	int numParticles = Aqsis::lfloor(face.weight*m_totParticles);
	if(numParticlesCts - numParticles > uRand())
		++numParticles;
	if(numParticles == 0)
		return boost::shared_ptr<PrimVars>();
	std::vector<int> storageCounts;
	// Create storage for all interpolated output parameters.
	for(PrimVars::const_iterator i = m_primVars->begin(), end = m_primVars->end();
			i != end; ++i)
	{
		if(i->token.Class() == Aqsis::class_constant
				|| i->token.Class() == Aqsis::class_uniform)
		{
			storageCounts.push_back(0);
			// uniform and constant primvars on the mesh interpolate to
			// constant primvars on the curves
			interpVars->append(Aqsis::CqPrimvarToken(Aqsis::class_constant,
				i->token.type(), i->token.count(), i->token.name() + "_emit"));
			// We can just copy over constant/uniform data; no interpolation needed.
			if(i->token.Class() == Aqsis::class_constant)
				*interpVars->back().value = *i->value;
			else
			{
				int stride = i->token.storageCount();
				interpVars->back().value->assign(
						i->value->begin() + stride*faceIdx,
						i->value->begin() + stride*(faceIdx+1));
			}
		}
		else
		{
			storageCounts.push_back(i->token.storageCount());
			// varying, vertex, facevarying and facevertex primvars interpolate
			// to uniform primvars on the curves
			interpVars->append(Aqsis::CqPrimvarToken(Aqsis::class_uniform,
				i->token.type(), i->token.count(), i->token.name() + "_emit"));
			// Allocate storage
			interpVars->back().value->assign(numParticles*storageCounts.back(), 0);
		}
	}

	// Float offsets for randomized quasi Monte-Carlo distribution
	float uOffset = float(std::rand())/RAND_MAX;
	float vOffset = float(std::rand())/RAND_MAX;
	// loop over child particles
	for(int particleNum = 0; particleNum < numParticles; ++particleNum)
	{
		// get random weights for the vertices of the current face.
		float u = uOffset + m_lowDiscrep.Generate(0, particleNum);
		if(u > 1)
			u -= 1;
		float v = vOffset + m_lowDiscrep.Generate(1, particleNum);
		if(v > 1)
			v -= 1;
		float weights[4];
		if(face.numVerts == 3)
		{
			if(u + v > 1)
			{
				u = 1-u;
				v = 1-v;
			}
			weights[0] = 1 - u - v;
			weights[1] = u;
			weights[2] = v;
		}
		else
		{
			weights[0] = (1-u)*(1-v);
			weights[1] = (1-u)*v;
			weights[2] = u*v;
			weights[3] = u*(1-v);
		}

		// loop over primitive variables.  Each varying/vertex/facevarying
		// /facevertex primvar is interpolated from the parent mesh to the
		// current child particle.
		int storageIndex = 0;
		PrimVars::iterator destVar = interpVars->begin();
		for(PrimVars::const_iterator srcVar = m_primVars->begin(),
				end = m_primVars->end(); srcVar != end;
				++srcVar, ++storageIndex, ++destVar)
		{
			int storageStride = storageCounts[storageIndex];
			// Get pointers to source parameters for the vertices
			const float* src[4] = {0,0,0,0};
			switch(srcVar->token.Class())
			{
				case Aqsis::class_varying:
				case Aqsis::class_vertex:
					for(int i = 0; i < face.numVerts; ++i)
						src[i] = &(*srcVar->value)[storageStride*face.v[i]];
					break;
				case Aqsis::class_facevarying:
				case Aqsis::class_facevertex:
					for(int i = 0; i < face.numVerts; ++i)
						src[i] = &(*srcVar->value)[
							storageStride*(face.faceVaryingIndex+i) ];
					break;
				default:
					// Other classes don't need any interpolation, so we just
					// go to the next primvar in m_primVars
					continue;
			}

			// Interpolate the primvar pointed to by srcVar to the current
			// particle position.  This is just a a weighted average of values
			// attached to vertices of the current face.
			float* dest = &(*destVar->value)[storageStride*particleNum];
			for(int k = 0; k < storageStride; ++k, ++dest)
			{
				*dest = 0;
				for(int i = 0; i < face.numVerts; ++i)
				{
					*dest += *src[i] * weights[i];
					++src[i];
				}
			}
		}
	}

	// Finally, add extra face-constant parameters.
	Vec3 Ng_emitVec = faceNormal(face);
	float Ng_emit[] = {Ng_emitVec.x(), Ng_emitVec.y(), Ng_emitVec.z()};
	interpVars->append(Aqsis::CqPrimvarToken(Aqsis::class_constant, Aqsis::type_normal,
				1, "Ng_emit"), FloatArray(Ng_emit, Ng_emit+3));

	return interpVars;
}

/// Get the area for a triangle
float EmitterMesh::triangleArea(const int* v) const
{
	Vec3 edge1 = m_P[v[0]] - m_P[v[1]];
	Vec3 edge2 = m_P[v[1]] - m_P[v[2]];

	return (edge1 % edge2).Magnitude()/2;
}

/// Get the area for a face
float EmitterMesh::faceArea(const MeshFace& face) const
{
	float area = 0;
	for(int i = 3; i <= face.numVerts; ++i)
		area += triangleArea(&face.v[0] + i-3);
	return area;
}

/// Get the normal for a face
Vec3 EmitterMesh::faceNormal(const MeshFace& face) const
{
	return ((m_P[face.v[1]] - m_P[face.v[0]]) %
		(m_P[face.v[2]] - m_P[face.v[1]])).Unit();
}

/** Initialise the list of faces for the mesh
 *
 * \param nverts - number of vertices per face
 * \param verts - concatenated array of vertex indices into the primvar arrays.
 * \param faces - newly initialised faces go here.
 */
void EmitterMesh::createFaceList(const Ri::IntArray& nverts,
		const Ri::IntArray& verts,
		FaceVec& faces) const
{
	// Create face list
	int faceStart = 0;
	float totWeight = 0;
	int sizeNVerts = nverts.size();
	int totVerts = 0;
	faces.reserve(sizeNVerts);
	for(int i = 0; i < sizeNVerts; ++i)
	{
		if(nverts[i] != 3 && nverts[i] != 4)
		{
			assert(0 && "emitter mesh can only deal with 3 and 4-sided faces");
			continue;
		}
		faces.push_back(MeshFace(&verts[0]+faceStart, totVerts, nverts[i]));
		faceStart += nverts[i];
		// Get weight for face
		float w = faceArea(faces.back());
		faces.back().weight = w;
		totWeight += w;
		totVerts += nverts[i];
	}
	// normalized areas so that total area = 1.
	float scale = 1/totWeight;
	for(int i = 0; i < sizeNVerts; ++i)
		faces[i].weight *= scale;
}

