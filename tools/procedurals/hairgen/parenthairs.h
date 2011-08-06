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


#ifndef PARENTHAIRS_H_INCLUDED
#define PARENTHAIRS_H_INCLUDED

#include <vector>
#include <iosfwd>

#include <boost/scoped_ptr.hpp>

#include "kdtree/kdtree2.hpp"
#include "primvar.h"
#include "util.h"


/** Holder for the set of hair modifiers.
 */
struct HairModifiers
{
	/** end_rough specifies whether or not to attach a random vector to the
	 * curves for use in displacement shaders.
	 * TODO: genralize this.
	 */
	bool endRough;
	/** root_index specifies which vertex index along the parent splines
	 * corresponds to the position of the hair root.  Negative values cause
	 * hairgen to choose a default automatically.
	 */
	int rootIndex;
	/** Clump specifies clumping behaviour in which child hairs clump toward
	 * the dominant parent.  This parameter is modelled after blender, and
	 * should lie in the interval [-1,1]:
	 * For clump > 0:
	 *   The tips of child hairs clump toward the parents.  At the tip, the
	 *   weight of the parent hair is clump, and the weight of the non-clumped
	 *   child hair is (1-clump)
	 * clump < 0:
	 *   The root of the child hairs clump toward the parents in an analogous
	 *   way to the tip for clump > 0.
	 */
	float clump;
	/** clump_shape is used in conjunction with clump, to control the blending
	 * between parent hairs and child hairs.  For surface parameter v not at
	 * the tip of the curves, the clump parameter is modified to be
	 *
	 *   clump*pow(v, clump_shape)
	 */
	float clumpShape;

	HairModifiers()
		: endRough(false),
		rootIndex(-1),
		clump(0),
		clumpShape(1)
	{ }

	/** Parse a modifier parameter name from the input stream
	 *
	 * \param name - name of the parameter
	 * \param in - stream from which to read the value.
	 *
	 * \return true if the name was recognized and processed, false otherwise.
	 */
	bool parseParam(const std::string& name, std::istream& in);
};

/** A class encapsulating an interpolation scheme based on a set of parent
 * hairs.
 *
 * The parent hairs are distributed through space in a possibly disordered
 * manner.  Given a child hair position, this class computes a small set of
 * parent hairs for the child, and associated weights.  All parent primvars are
 * then combined with a linear combination based on the weights to find
 * approximately interpolated primvars for the child particle.
 */
class ParentHairs
{
	public:
		/// Number of parent hairs per child in interpolation scheme
		static const int m_parentsPerChild = 5;

		/** Create a parent hair interpolation object.
		 *
		 * \param linear - Flag indicating whether curves are linear (true) or
		 *                 cubic (false)
		 * \param numVerts - vector of number of vertices for the curves.
		 *                   The number of vertices must be the same for all
		 *                   parent curves; if not a std::runtime_error is
		 *                   thrown.
		 * \param primVars - Set of primvars for parent curves.
		 */
		ParentHairs(bool linear, const Ri::IntArray& numVerts,
				const boost::shared_ptr<PrimVars>& primVars,
				const HairModifiers& modifiers);

		/** Create a set of interpolated child primvars based on the parent
		 * hair primvars.
		 *
		 * \param childVars - A set of primvars containing at least the child
		 *                    particle positions from the emitting mesh, P_emit.
		 */
		void childInterp(PrimVars& childVars) const;

		/// Return the curve type flag (true==linear, false==cubic)
		bool linear() const;

		/// Return the number of vertices on each parent curve.
		int vertsPerCurve() const;

	private:
		//--------------------------------------------------
		void computeClumpWeights(std::vector<float>& clumpWeights) const;

		void getParents(const Vec3& pos, int ind[m_parentsPerChild],
				float weights[m_parentsPerChild]) const;

		static void perChildStorage(const PrimVars& primVars, int numParents,
				std::vector<int>& storageCounts);

		void initLookup(const FloatArray& P, int numParents);

		//--------------------------------------------------
		/// flag for linear/cubic hairs
		bool m_linear;
		/// Set of modifiers for child hairs.
		HairModifiers m_modifiers;
		/// number of vertex-class values per hair.
		int m_vertsPerCurve;
		/// hair primitive variables
		boost::shared_ptr<PrimVars> m_primVars;
		std::vector<int> m_storageCounts;
		/// base points of hairs.
		// implementation via kdtree lookup
		kdtree::kdtree2_array m_baseP;
		/// search tree
		boost::scoped_ptr<kdtree::kdtree2> m_lookupTree;
};

#endif // PARENTHAIRS_H_INCLUDED
