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
	m_baseP()
{
	const Aqsis::TqRiFloatArray& P = m_primVars->find(Aqsis::CqPrimvarToken(
				Aqsis::class_vertex, Aqsis::type_point, 1, "P"));
	if(static_cast<int>(numVerts.size()) < numParents)
		throw std::runtime_error("number of parent hairs must be >= 4");
	// iterate over the curves, finding the base point for each.
	int vertsPerCurve = numVerts[0];
	for(int i = 0, numNumVerts = numVerts.size(); i < numNumVerts; ++i)
	{
		m_baseP.push_back(Vec3(&P[vertsPerCurve*3*i]));
		if(numVerts[i] != vertsPerCurve)
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

	std::vector<int> storageCounts;
	typedef std::vector<Aqsis::TqRiFloatArray*> PrimVarValueVec;
	PrimVarValueVec newPrimvars;
	for(PrimVars::const_iterator i = m_primVars->begin(), end = m_primVars->end();
			i != end; ++i)
	{
		// Child hairs have the same tokens as parents
		childVars.append(i->token);

		// Get amount of storage necessary.
		int perChildStorage = 0;
		switch(i->token.Class())
		{
			case Aqsis::class_constant:
				perChildStorage = 0;
				break;
			case Aqsis::class_uniform:
				perChildStorage = 1;
				break;
			case Aqsis::class_varying:
				if(m_linear)
					perChildStorage = m_vertsPerCurve;
				else
				{
					// TODO: Fix this properly.
					// varying count for cubic bezier splines.
					// perChildStorage = 2*(m_vertsPerCurve-1)/3;
					// varying count for cubic b-splines
					perChildStorage = m_vertsPerCurve-2;
				}
				break;
			case Aqsis::class_vertex:
				perChildStorage = m_vertsPerCurve;
				break;
			default:
				g_errStream << "unimplemented interpolation class "
					<< i->token.Class() << "\n";
				break;
		}
		// Allocate storage
		int floatsPerChild = perChildStorage*i->token.storageCount();
		storageCounts.push_back(floatsPerChild);
		if(floatsPerChild > 0)
			childVars.back().value->assign(numChildren*floatsPerChild, 0);
		else
		{
			// this special case is for class constant
			childVars.back().value->assign(i->value->begin(), i->value->end());
		}
		newPrimvars.push_back(&*childVars.back().value);
	}

	// loop over all child curves
	for(int curveNum = 0; curveNum < numChildren; ++curveNum)
	{
		// Get weights and indices of parent hairs for current child.
		int parentIdx[numParents];
		float weights[numParents];
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

			int storageStride = storageCounts[storageIndex];

			// interpolate vertex class values
			const float* src[numParents];
			for(int i = 0; i < numParents; ++i)
				src[i] = &(*srcVar->value)[storageStride*parentIdx[i]];

			float* dest = &(**destVar)[storageStride*curveNum];
			for(int k = 0; k < storageStride; ++k, ++dest)
			{
				*dest = 0;
				for(int i = 0; i < numParents; ++i)
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
		int storageStride = storageCounts[storageIndex];
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
 * \param dist - array of numParents distances.
 * \return index such that dist[index] is the maximum in the array.
 */
inline int ParentHairs::findMaxDistIndex(const float dist[numParents])
{
	// compute max index and distance
	int maxIdx = 0;
	float maxDist = dist[maxIdx];
	for(int i = 1; i < numParents; ++i)
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
void ParentHairs::getParents(const Vec3& pos, int ind[numParents],
		float weights[numParents]) const
{
	float dist[numParents] = {0,0,0,0};

	// inital indices and distances are the first four.
	for(int i = 0; i < numParents; ++i)
	{
		ind[i] = i;
		dist[i] = (m_baseP[i] - pos).Magnitude();
	}

	// index of the furtherest of the closest parent base points to pos.
	int maxIdx = findMaxDistIndex(dist);
	float maxDist = dist[maxIdx];

	// Find the closest four points to pos from the parent hair base points.
	for(int i = numParents, end = m_baseP.size(); i < end; ++i)
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
	for(int i = 0; i < numParents; ++i)
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
	for(int i = 0; i < numParents; ++i)
		weights[i] /= totWeight;
}
