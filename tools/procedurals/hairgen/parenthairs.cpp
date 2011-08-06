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

#include "parenthairs.h"

#include <cmath>
#include <iostream>
#include <algorithm>

//------------------------------------------------------------------------------
// HairModifiers implementation
bool HairModifiers::parseParam(const std::string& name, std::istream& in)
{
	if(name == "end_rough")
	{
		in >> std::boolalpha >> endRough;
		return true;
	}
	else if(name == "root_index")
	{
		in >> rootIndex;
		return true;
	}
	else if(name == "clump")
	{
		in >> clump;
		return true;
	}
	else if(name == "clump_shape")
	{
		in >> clumpShape;
		return true;
	}
	return false;
}


//------------------------------------------------------------------------------
// ParentHairs implementation
ParentHairs::ParentHairs(bool linear,
		const Ri::IntArray& numVerts,
		const boost::shared_ptr<PrimVars>& primVars,
		const HairModifiers& modifiers)
	: m_linear(linear),
	m_modifiers(modifiers),
	m_vertsPerCurve(numVerts[0]),
	m_primVars(primVars),
	m_storageCounts(),
	m_baseP(),
	m_lookupTree()
{
	if(m_modifiers.rootIndex < 0)
	{
		// Modify root index for a sensible default.  For hairs which
		// interpolate the control points this should be 0.  Otherwise it
		// should be something else.
		//
		// Currently we guess 1 for cubic curves, since this corresponds to the
		// way MOSAIC works (using catmull-rom splines)
		m_modifiers.rootIndex = m_linear ? 0 : 1;
	}
	// Check that we have enough parents for interpolation scheme
	if(static_cast<int>(numVerts.size()) < m_parentsPerChild)
		throw std::runtime_error("number of parent hairs must be >= 4");
	// iterate over the curves, finding the base point for each.
	for(int i = 0, numNumVerts = numVerts.size(); i < numNumVerts; ++i)
	{
		// check that all parent curves have the same length.
		if(numVerts[i] != m_vertsPerCurve)
		{
			throw std::runtime_error("number of vertices per parent hair"
					"must be constant");
		}
	}
	// initialize the per-child storage counts
	perChildStorage(*primVars, numVerts.size(), m_storageCounts);
	// initialize the child --> parent hair lookup scheme.
	const FloatArray& P = m_primVars->find(Aqsis::CqPrimvarToken(
				Aqsis::class_vertex, Aqsis::type_point, 1, "P"));
	initLookup(P, numVerts.size());
}

bool ParentHairs::linear() const
{
	return m_linear;
}

int ParentHairs::vertsPerCurve() const
{
	return m_vertsPerCurve;
}

void ParentHairs::childInterp(PrimVars& childVars) const
{
	const FloatArray& P_emit = childVars.find("P_emit");

	int numChildren = P_emit.size()/3;

	typedef std::vector<FloatArray*> PrimVarValueVec;
	// vector of value vectors for new child primvars, used during interpolation.
	PrimVarValueVec newPrimvars;
	// allocate child storage.
	for(int i = 0, end = m_primVars->size(); i < end; ++i)
	{
		const TokFloatValPair& parentVar = (*m_primVars)[i];
		// child hairs have the same tokens as parents
		childVars.append(parentVar.token);
		FloatArray& childValues = *childVars.back().value;
		if(parentVar.token.Class() == Aqsis::class_constant)
		{
			// for class constant, no interpolation is required, so we may
			// fill in the constant child primvars here.
			childValues.assign(parentVar.value->begin(), parentVar.value->end());
		}
		else
		{
			// else allocate enough storage to hold the interpolated values.
			childValues.assign(numChildren*m_storageCounts[i], 0);
		}
		newPrimvars.push_back(&childValues);
	}

	std::vector<int> closestParent;
	if(m_modifiers.clump != 0)
		closestParent.resize(numChildren);
	// loop over all child curves
	for(int curveNum = 0; curveNum < numChildren; ++curveNum)
	{
		// Get weights and indices of parent hairs for current child.
		int parentIdx[m_parentsPerChild];
		float weights[m_parentsPerChild];
		Vec3 currP(&P_emit[3*curveNum]);
		getParents(currP, parentIdx, weights);
		if(m_modifiers.clump != 0)
			closestParent[curveNum] = parentIdx[0];

		// loop over all primvars of parent curves
		int storageIndex = 0;
		PrimVarValueVec::iterator destVar = newPrimvars.begin();
		for(PrimVars::const_iterator srcVar = m_primVars->begin(),
				end = m_primVars->end(); srcVar != end;
				++srcVar, ++storageIndex, ++destVar)
		{
			switch(srcVar->token.Class())
			{
				case Aqsis::class_uniform:
				case Aqsis::class_varying:
				case Aqsis::class_vertex:
					break;
				default:
					continue;
			}

			int storageStride = m_storageCounts[storageIndex];

			// interpolate vertex class values
			const float* src[m_parentsPerChild];
			for(int i = 0; i < m_parentsPerChild; ++i)
				src[i] = &(*srcVar->value)[storageStride*parentIdx[i]];

			float* dest = &(**destVar)[storageStride*curveNum];
			for(int k = 0; k < storageStride; ++k, ++dest)
			{
				*dest = 0;
				for(int i = 0; i < m_parentsPerChild; ++i)
				{
					*dest += *src[i] * weights[i];
					++src[i];
				}
			}
		}
	}

	FloatArray& P_child = childVars.find("P");
	// Apply corrections to interpolation scheme for variables of class
	// "point".  This is necessary since the desired base point of the hair
	// (stored in P_emit) isn't stationary under the interpolation scheme.
	int storageIndex = 0;
	PrimVarValueVec::iterator destVar = newPrimvars.begin();
	for(PrimVars::const_iterator srcVar = m_primVars->begin(),
			end = m_primVars->end(); srcVar != end;
			++srcVar, ++storageIndex, ++destVar)
	{
		switch(srcVar->token.Class())
		{
			default:
				continue;
			case Aqsis::class_uniform:
			case Aqsis::class_varying:
			case Aqsis::class_vertex:
				break;
		}
		if(srcVar->token.type() != Aqsis::type_point)
			continue;
		// If we get here, we have a point class primvar which has been
		// interpolated from parent hairs, and we need to apply the correction
		// term to it.
		int storageStride = m_storageCounts[storageIndex];
		FloatArray& value = **destVar;
		for(int curveNum = 0; curveNum < numChildren; ++curveNum)
		{
			Vec3 deltaP = Vec3(&P_emit[3*curveNum])
				- Vec3(&P_child[storageStride*curveNum+3*m_modifiers.rootIndex]);
			for(int k = curveNum*storageStride, kEnd = (curveNum+1)*storageStride;
					k < kEnd; k += 3)
			{
				value[k] += deltaP.x();
				value[k+1] += deltaP.y();
				value[k+2] += deltaP.z();
			}
		}
	}

	if(m_modifiers.clump != 0)
	{
		// Apply clumping to P after correction factor.
		const FloatArray& P_parent = m_primVars->find("P");
		std::vector<float> weights;
		computeClumpWeights(weights);
		for(int curveNum = 0; curveNum < numChildren; ++curveNum)
		{
			const float* parentP = &P_parent[3*m_vertsPerCurve*closestParent[curveNum]];
			float* childP = &P_child[3*m_vertsPerCurve*curveNum];
			for(int k = 0; k < m_vertsPerCurve; ++k)
			{
				int i = 3*k;
				childP[i] = (1-weights[k])*childP[i] + weights[k]*parentP[i];
				childP[i+1] = (1-weights[k])*childP[i+1] + weights[k]*parentP[i+1];
				childP[i+2] = (1-weights[k])*childP[i+2] + weights[k]*parentP[i+2];
			}
		}
	}

	if(m_modifiers.endRough)
	{
		// Add random vectors to help with end rough.
		childVars.append(Aqsis::CqPrimvarToken(Aqsis::class_uniform,
					Aqsis::type_vector, 1, "endRoughRand"));
		FloatArray& rand1 = *childVars.back().value;
		rand1.reserve(3*numChildren);
		for(int curveNum = 0; curveNum < numChildren; ++curveNum)
		{
			rand1.push_back(2*uRand()-1);
			rand1.push_back(2*uRand()-1);
			rand1.push_back(2*uRand()-1);
		}
	}
}

/** Compute weights to use in hair clumping
 *
 * A clump weight is the amount the hair is weighted toward the parent hair.  A
 * value of zero indicates no clumping, while a value of one indicates that
 * every child hair would lie exactly on top of the corresponding parent.
 *
 * \param clumpWeights - vector of computed clump weights (output)
 */
void ParentHairs::computeClumpWeights(std::vector<float>& clumpWeights) const
{
	clumpWeights.resize(m_vertsPerCurve);
	// Convert the clumpShape parameter into an exponent in the same way that
	// blender does.
	float clumpPow = 0;
	if(m_modifiers.clumpShape < 0)
		clumpPow = 1 + m_modifiers.clumpShape;
	else
		clumpPow = 1 + 9*m_modifiers.clumpShape;
	// compute weights along the curve.
	for(int i = 0; i < m_vertsPerCurve; ++i)
	{
		float v = float(i)/(m_vertsPerCurve-1);
		if(m_modifiers.clump < 0)
		{
			// for negative values of clump, clump hair root rather than tips.
			v = 1-v;
		}
		clumpWeights[i] = std::fabs(m_modifiers.clump)
			* std::pow(v, clumpPow);
	}
}

/** Get parent particle weigths and indices for a given child position.
 *
 * The indicies of the parents are returned in order from the closest to
 * the most distant parent from the child position.
 *
 * \param pos - child particle position
 * \param ind - indices of parent particles (output parameter).
 * \param weights - weights for parent particles (output parameter).
 */
void ParentHairs::getParents(const Vec3& pos, int ind[m_parentsPerChild],
		float weights[m_parentsPerChild]) const
{
	// Search for closest parents using a kd-tree.  The kdtree2 interface kinda
	// sucks, so we need to allocate a std::vector here.
	std::vector<float> childPos(3);
	childPos[0] = pos.x();
	childPos[1] = pos.y();
	childPos[2] = pos.z();

	kdtree::kdtree2_result_vector neighbours;

	m_lookupTree->n_nearest(childPos, m_parentsPerChild, neighbours);
	// sort so that nearest neighbours come first.
	std::sort(neighbours.begin(), neighbours.end());

	// Compute weights
	float totWeight = 0;
	float maxDis = neighbours.back().dis;
	for(int i = 0; i < m_parentsPerChild; ++i)
	{
		ind[i] = neighbours[i].idx;
		// This is the same weighting function for parent hairs as
		// blender uses - just an exponential decay with distance.
//				float w = std::pow(2, -6*dist[i]/maxDist);
		// Interpolation quality is better if the weights are slightly modified
		// when the number of parent particles is higher.  Here's one for 5 or
		// 6 parent particles.
		float w = std::pow(2, -10*std::sqrt(neighbours[i].dis/maxDis));
		weights[i] = w;
		totWeight += w;
	}
	// normalize weights
	for(int i = 0; i < m_parentsPerChild; ++i)
		weights[i] /= totWeight;
}

/** Compute per-child storage counts for interpolated primvars
 *
 * The per-child storage count for a primvar is the number of floats required
 * per child hair to store that primvar.
 *
 * \param primVars - parent primvars
 * \param numParents - number of parent hairs
 * \param storageCounts - per child storage counts (output)
 */
void ParentHairs::perChildStorage(const PrimVars& primVars, int numParents,
		std::vector<int>& storageCounts)
{
	storageCounts.clear();
	storageCounts.reserve(primVars.size());
	for(PrimVars::const_iterator var = primVars.begin(), end = primVars.end();
			var != end; ++var)
	{
		if(var->token.Class() == Aqsis::class_constant)
		{
			// constant is a special case, for which we need zero per-child
			// storage, only a fixed amount per RiCurves
			storageCounts.push_back(0);
		}
		else
		{
			// In all other cases, the parent hair storage count should be
			// divisible by the total number of curves.  We check that this is
			// so and if it is, presume that everything is ok.
			int totParentCount = var->value->size();
			if(totParentCount % numParents != 0)
			{
				throw std::runtime_error("parent hair storage counts must be "
						"a multiple of the number of parent hairs");
			}
			storageCounts.push_back(totParentCount/numParents);
		}
	}
}

/** Initialize the kdtree for finding parent particles for a child.
 *
 * \param P - Positions array for parent curves.  Every m_vertsPerCurve'th
 *            position represents the start vector for a curve.
 * \param numParents - total number of parent particles.
 */
void ParentHairs::initLookup(const FloatArray& P, int numParents)
{
	m_baseP.resize(boost::extents[numParents][3]);
	for(int i = 0, end = P.size()/(3*m_vertsPerCurve); i < end; ++i)
	{
		int baseIdx = 3*(m_vertsPerCurve*i + m_modifiers.rootIndex);
		m_baseP[i][0] = P[baseIdx];
		m_baseP[i][1] = P[baseIdx+1];
		m_baseP[i][2] = P[baseIdx+2];
	}
	m_lookupTree.reset(new kdtree::kdtree2(m_baseP, false));
}
