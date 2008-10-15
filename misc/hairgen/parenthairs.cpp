// hairgen procedural
// Copyright (C) 2008 Christopher J. Foster [chris42f (at) gmail (d0t) com]
//
// This software is licensed under the GPLv2 - see the file COPYING for details.

#include "parenthairs.h"

ParentHairs::ParentHairs(bool linear,
		const Aqsis::TqRiIntArray& numVerts,
		const boost::shared_ptr<PrimVars>& primVars)
	: m_linear(linear),
	m_vertsPerCurve(numVerts[0]),
	m_primVars(primVars),
	m_storageCounts(),
	m_baseP()
{
	const Aqsis::TqRiFloatArray& P = m_primVars->find(Aqsis::CqPrimvarToken(
				Aqsis::class_vertex, Aqsis::type_point, 1, "P"));
	// Check that we have enough parents for interpolation scheme
	if(static_cast<int>(numVerts.size()) < m_parentsPerChild)
		throw std::runtime_error("number of parent hairs must be >= 4");
	// initialize the per-child storage counts
	perChildStorage(*primVars, numVerts.size(), m_storageCounts);
	// iterate over the curves, finding the base point for each.
	for(int i = 0, numNumVerts = numVerts.size(); i < numNumVerts; ++i)
	{
		m_baseP.push_back(Vec3(&P[m_vertsPerCurve*3*i]));
		// check that all parent curves have the same length.
		if(numVerts[i] != m_vertsPerCurve)
		{
			throw std::runtime_error("number of vertices per parent hair"
					"must be constant");
		}
	}
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
	const Aqsis::TqRiFloatArray& P_emit = childVars.find("P_emit");

	int numChildren = P_emit.size()/3;

	typedef std::vector<Aqsis::TqRiFloatArray*> PrimVarValueVec;
	// vector of value vectors for new child primvars, used during interpolation.
	PrimVarValueVec newPrimvars;
	// allocate child storage.
	for(int i = 0, end = m_primVars->size(); i < end; ++i)
	{
		const TokFloatValPair& parentVar = (*m_primVars)[i];
		// child hairs have the same tokens as parents
		childVars.append(parentVar.token);
		Aqsis::TqRiFloatArray& childValues = *childVars.back().value;
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

	// loop over all child curves
	for(int curveNum = 0; curveNum < numChildren; ++curveNum)
	{
		// Get weights and indices of parent hairs for current child.
		int parentIdx[m_parentsPerChild];
		float weights[m_parentsPerChild];
		Vec3 currP(&P_emit[3*curveNum]);
		getParents(currP, parentIdx, weights);

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

	const Aqsis::TqRiFloatArray& P_child = childVars.find("P");
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
		Aqsis::TqRiFloatArray& value = **destVar;
		for(int curveNum = 0; curveNum < numChildren; ++curveNum)
		{
			Vec3 deltaP = Vec3(&P_emit[3*curveNum])
				- Vec3(&P_child[storageStride*curveNum]);
			for(int k = curveNum*storageStride, kEnd = (curveNum+1)*storageStride;
					k < kEnd; k += 3)
			{
				value[k] += deltaP.x();
				value[k+1] += deltaP.y();
				value[k+2] += deltaP.z();
			}
		}
	}
}

/** Find the index of the maximum element of the array.
 *
 * \param dist - array of m_parentsPerChild distances.
 * \return index such that dist[index] is the maximum in the array.
 */
inline int ParentHairs::findMaxDistIndex(const float dist[m_parentsPerChild])
{
	// compute max index and distance
	int maxIdx = 0;
	float maxDist = dist[maxIdx];
	for(int i = 1; i < m_parentsPerChild; ++i)
	{
		if(dist[i] > maxDist)
		{
			maxDist = dist[i];
			maxIdx = i;
		}
	}
	return maxIdx;
}

/** Get parent particle weigths and indices for a given child position.
 *
 * \param pos - child particle position
 * \param ind - indices of parent particles (output parameter).
 * \param weights - weights for parent particles (output parameter).
 */
void ParentHairs::getParents(const Vec3& pos, int ind[m_parentsPerChild],
		float weights[m_parentsPerChild]) const
{
	float dist[m_parentsPerChild] = {0,0,0,0};

	// inital indices and distances are the first four.
	for(int i = 0; i < m_parentsPerChild; ++i)
	{
		ind[i] = i;
		dist[i] = (m_baseP[i] - pos).Magnitude();
	}

	// index of the furtherest of the closest parent base points to pos.
	int maxIdx = findMaxDistIndex(dist);
	float maxDist = dist[maxIdx];

	// Find the closest four points to pos from the parent hair base points.
	for(int i = m_parentsPerChild, end = m_baseP.size(); i < end; ++i)
	{
		float d = (m_baseP[i] - pos).Magnitude();
		if(d < maxDist)
		{
			ind[maxIdx] = i;
			dist[maxIdx] = d;
			maxIdx = findMaxDistIndex(dist);
			maxDist = dist[maxIdx];
		}
	}

	// Compute weights
	float totWeight = 0;
	for(int i = 0; i < m_parentsPerChild; ++i)
	{
		// This is the same weighting function for parent hairs as
		// blender uses - just an exponential decay with distance.
//				float w = std::pow(2, -6*dist[i]/maxDist);
		// Interpolation quality is better if modified the weights are slightly
		// modified when the number of parent particles is higher.  Here's one
		// for 5 or 6 parent particles.
		float w = std::pow(2, -10*dist[i]/maxDist);
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

