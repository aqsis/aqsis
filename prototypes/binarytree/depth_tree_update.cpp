/** A prototype for updating the depths in an array-based occlusion tree.
 *
 * This compares a recursive traversal of the tree with a simple iterative
 * update made possible by the array structure.
 */

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <vector>

float urand()
{
	return static_cast<float>(std::rand())/RAND_MAX;
}

inline float max(const float a, const float b)
{
	return (a < b) ? b : a;
}

// Example: binary tree with 3 levels, stored in an array.
//
// level |   layout with node indices
//       |
//   0   |             0
//       |            / \
//       |           /   \
//       |          /     \
//   1   |         1       2
//       |        / \     / \
//   2   |       3   4   5   6
//
// index of first node of i'th level is  pow(2, level) - 1

class DepthTree
{
	private:
		// depth data
		std::vector<float> m_depths;
		// Number of levels in the tree.
		int m_numLevels;
		// to speed up recursive depth propagation.
		int m_firstLeafIndex;

		// Recursive depth update function.
		float propagateDepthsRecursive(int index)
		{
			// Using m_firstLeafIndex here gives *big* speedups for small
			// trees.  For large trees where the computation is cache-bound the
			// improvements aren't so apparent.
			if(index >= m_firstLeafIndex)
				return m_depths[index];

			m_depths[index] = max(
					propagateDepthsRecursive(2*index + 1),
					propagateDepthsRecursive(2*index + 2)
				);
			return m_depths[index];
		}

	public:
		DepthTree(int numLevels)
			: m_depths(static_cast<int>(std::pow(2.0,numLevels))-1, 0),
			m_numLevels(numLevels)
		{ }

		//--------------------------------------------------
		// Depth update functions

		// Fix the tree so that each node records the maximum depth of all the
		// children below it.
		void propagateDepths()
		{
			// Iterate over each level of the tree in turn, starting at one
			// level below the leaf nodes, and ending at the root.  This
			// algorithm is cache-coherent and simple.
			for(int i = static_cast<int>(std::pow(2.0, m_numLevels-1)) - 2; i >= 0; --i)
				m_depths[i] = max(m_depths[2*i+1], m_depths[2*i+2]);
		}
		// Recursive version of the above.
		void propagateDepthsRecursive()
		{
			m_firstLeafIndex = static_cast<int>(std::pow(2.0, m_numLevels-1)) - 1;
			propagateDepthsRecursive(0);
		}


		//--------------------------------------------------
		// Test stuff.

		// Fill with some random numbers.
		void randomize()
		{
			for(int i = 0, end = m_depths.size(); i < end; ++i)
				m_depths[i] = urand();
		}

		// Check whether two trees contain the same depth data.
		bool operator==(const DepthTree& rhs)
		{
			if(rhs.m_depths.size() != m_depths.size())
				return false;
			for(int i = 0, end = m_depths.size(); i < end; ++i)
			{
				if(m_depths[i] != rhs.m_depths[i])
					return false;
			}
			return true;
		}

		// Print the tree
		friend std::ostream& operator<<(std::ostream& out, const DepthTree& t)
		{
			for(int level = 0; level < t.m_numLevels; ++level)
			{
				for(int i = static_cast<int>(std::pow(2.0, level))-1,
						end = static_cast<int>(std::pow(2.0, level+1))-1;
						i < end; ++i)
				{
					out << std::setprecision(2) << t.m_depths[i] << " ";
				}
				out << "\n";
			}
			return out;
		}

		// Test the depth propagation.
		static void unitTest()
		{
			DepthTree t1(5);
			t1.randomize();
			DepthTree t2 = t1;

			// Check depth proagation algorithms against each other.
			t1.propagateDepths();
			t2.propagateDepthsRecursive();
			assert(t1 == t2);

			// Check that the root node contains the largest depth.
			float largest = t1.m_depths[0];
			for(int i = 1, end = t1.m_depths.size(); i < end; ++i)
				assert(t1.m_depths[i] <= largest);

			std::cout << t1;
		}
};


int main()
{
	DepthTree::unitTest();

	// The size of the tree is critical for the relative efficiencies of the
	// two algorithms, probably since the iterative version is more cache
	// friendly.
	DepthTree tree(13);
	tree.randomize();

	for(int i = 0; i < 10000; ++i)
	{
		tree.propagateDepths();
//		tree.propagateDepthsRecursive();
	}

}

// Example timings (linux i686, g++-4.1.2, Pentium4 2.60GHz)
//
// propagateDepths() - 0.551s
// propagateDepthsRecursive() - 1.174s
