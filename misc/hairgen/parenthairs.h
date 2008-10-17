// hairgen procedural
// Copyright (C) 2008 Christopher J. Foster [chris42f (at) gmail (d0t) com]
//
// This software is licensed under the GPLv2 - see the file COPYING for details.

#ifndef PARENTHAIRS_H_INCLUDED
#define PARENTHAIRS_H_INCLUDED

#include <vector>

#include <boost/scoped_ptr.hpp>
#include <aqsis/ribparser.h>

#include "kdtree/kdtree2.hpp"
#include "primvar.h"
#include "util.h"

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
		ParentHairs(bool linear, const Aqsis::TqRiIntArray& numVerts,
				const boost::shared_ptr<PrimVars>& primVars);

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
		void getParents(const Vec3& pos, int ind[m_parentsPerChild],
				float weights[m_parentsPerChild]) const;

		static void perChildStorage(const PrimVars& primVars, int numParents,
				std::vector<int>& storageCounts);

		void initLookup(const Aqsis::TqRiFloatArray& P, int numParents);

		//--------------------------------------------------
		/// flag for linear/cubic hairs
		bool m_linear;
		/// number of vertex-class values per hair.
		int m_vertsPerCurve;
		/// hair primitive variables
		boost::shared_ptr<PrimVars> m_primVars;
		std::vector<int> m_storageCounts;
		/// base points of hairs.
//		 std::vector<Vec3> m_baseP;  // old implementation.
		// implementation via kdtree lookup
		kdtree::kdtree2_array m_baseP;
		/// search tree
		boost::scoped_ptr<kdtree::kdtree2> m_lookupTree;
};

#endif // PARENTHAIRS_H_INCLUDED
