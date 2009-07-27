#include <iostream>
#include <cmath>
#include <cassert>

//
// This piece of code implements a method for finding the index for the 2
// dimensional kd-tree leaf node which contains a given sample point.  The
// indices correspond to the storage location when the kd-tree is stored in an
// array.
//
// The spatial subdivision scheme is as follows:
//
//   The root node is level 1.
//
//   Even levels correspond to subdiving the x-axis.
//   Odd levels correspond to subdiving the y-axis.
//
// Here is an example binary spatial subdivision, with indices assigned to leaf
// nodes which are associated with the boxes.
//
//
// +---------------+-0.0f
// |   |   |   |   |  |
// | 7 | 8 | 11| 12|  |       0
// |   |   |   |   |  |
// |-------|-------|  |      int coords of subdiv number.
// |   |   |   |   |  |
// | 9 | 10| 13| 14|  |       1
// |   |   |   |   |  v
// +---------------+-1.0f
// |               |
// 0.0f  ----->    1.0f
//
// integer coords of subdivison number
//   0   1   2   3
//

#define OUT(x) std::cout << #x << " = " << (x) << "\n"

struct Point
{
	float x;
	float y;
	Point(float x, float y) : x(x), y(y) {}
};

//------------------------------------------------------------------------------
/** Return the tree index for leaf node containing the given point.
 *
 * treeDepth - depth of the tree, (root node has treeDepth == 1).
 *
 * p - a point which we'd like the index for.  We assume that the possible
 *     points lie in the box [0,1) x [0,1)
 *
 * Returns an index for a 0-based array.
 */
int treeIndexForPoint_fast(int treeDepth, const Point p)
{
	assert(treeDepth > 0);
	assert(p.x >= 0 && p.x < 1);
	assert(p.y >= 0 && p.y < 1);

	const int numXSubdivisions = treeDepth / 2;
	const int numYSubdivisions = (treeDepth-1) / 2;
	// true if the last subdivison was along the x-direction.
	const bool lastSubdivisionInX = (treeDepth % 2 == 0);

	// integer coordinates of the point in terms of the subdivison which it
	// falls into, staring from 0 in the top left.
	unsigned int x = static_cast<int>(floor(p.x * (1 << numXSubdivisions)));
	unsigned int y = static_cast<int>(floor(p.y * (1 << numYSubdivisions)));

	// This is the base coordinate for the first leaf in a tree of depth "treeDepth".
	unsigned int index = 1 << (treeDepth-1);
	if(lastSubdivisionInX)
	{
		// Every second bit of the index (starting with the LSB) should make up
		// the coordinate x, so distribute x into index in that fashion.
		//
		// Similarly for y, except alternating with the bits of x (starting
		// from the bit up from the LSB.)
		for(int i = 0; i < numXSubdivisions; ++i)
		{
			index |= (x & (1 << i)) << i
				| (y & (1 << i)) << (i+1);
		}
	}
	else
	{
		// This is the opposite of the above: x and y are interlaced as before,
		// but now the LSB of y rather than x goes into the LSB of index.
		for(int i = 0; i < numYSubdivisions; ++i)
		{
			index |= (y & (1 << i)) << i
				| (x & (1 << i)) << (i+1);
		}
	}
	return index - 1;
}


//------------------------------------------------------------------------------
struct Box
{
	Point topLeft;
	Point bottomRight;

	Box(Point p1, Point p2) : topLeft(p1), bottomRight(p2) {}
};

/** Recursive implementation of the above.  Here we simply walk down the tree
 * according to the subdivison scheme defined above.
 * 
 * treeDepth - total depth of tree (root = depth 1)
 * p - point to find the index for.
 * currNodeIndex - index for the currently considered tree node
 * currDepth - index for the depth of the current node
 * bounds - spatial bounds which define the box which the current node encompasses.
 */
int treeIndexForPoint_recursive(int treeDepth, Point p,
		int currNodeIndex, int currDepth, Box bounds)
{
	if(currDepth == treeDepth)
	{
		return currNodeIndex;
	}
	if((currDepth+1) % 2 == 0)
	{
		// subdivide x-axis.
		float xCenter = (bounds.topLeft.x + bounds.bottomRight.x)/2;
		if(p.x < xCenter)
		{
			return treeIndexForPoint_recursive(treeDepth, p,
					2*currNodeIndex + 1, currDepth + 1,
					Box(bounds.topLeft, Point(xCenter, bounds.bottomRight.y)) );
		}
		else
		{
			return treeIndexForPoint_recursive(treeDepth, p,
					2*currNodeIndex + 2, currDepth + 1,
					Box(Point(xCenter, bounds.topLeft.y), bounds.bottomRight) );
		}
	}
	else
	{
		// subdivide y-axis.
		float yCenter = (bounds.topLeft.y + bounds.bottomRight.y)/2;
		if(p.y < yCenter)
		{
			return treeIndexForPoint_recursive(treeDepth, p,
					2*currNodeIndex + 1, currDepth + 1,
					Box(bounds.topLeft, Point(bounds.bottomRight.x, yCenter)) );
		}
		else
		{
			return treeIndexForPoint_recursive(treeDepth, p,
					2*currNodeIndex + 2, currDepth + 1,
					Box(Point(bounds.topLeft.x, yCenter), bounds.bottomRight) );
		}
	}
}

// Driver function for the recursive implementation above.
int treeIndexForPoint_simple(int treeDepth, Point p)
{
	return treeIndexForPoint_recursive(treeDepth, p, 0, 1, Box(Point(0,0), Point(1,1)));
}

void unitTests()
{
	// Unit tests for treeIndexForPoint_fast:
	assert(treeIndexForPoint_fast(1, Point(0.5,0.5)) == 0);
	assert(treeIndexForPoint_fast(2, Point(0.5,0.5)) == 2);
	assert(treeIndexForPoint_fast(4, Point(0.1,0.1)) == 7);
	assert(treeIndexForPoint_fast(5, Point(0.6,0.1)) == 23);
	assert(treeIndexForPoint_fast(5, Point(0.6,0.3)) == 24);

	// Unit tests for treeIndexForPoint_simple:
	assert(treeIndexForPoint_simple(1, Point(0.5,0.5)) == 0);
	assert(treeIndexForPoint_simple(2, Point(0.5,0.5)) == 2);
	assert(treeIndexForPoint_simple(4, Point(0.1,0.1)) == 7);
	assert(treeIndexForPoint_simple(5, Point(0.6,0.1)) == 23);
	assert(treeIndexForPoint_simple(5, Point(0.6,0.3)) == 24);

	// Check the implementations against each other for some large values...
	assert( treeIndexForPoint_simple(10, Point(0.6,0.3)) 
			== treeIndexForPoint_fast(10, Point(0.6,0.3)) );
	assert( treeIndexForPoint_simple(11, Point(0.9,0.5)) 
			== treeIndexForPoint_fast(11, Point(0.9,0.5)) );
	assert( treeIndexForPoint_simple(12, Point(0.1,0.99)) 
			== treeIndexForPoint_fast(12, Point(0.1,0.99)) );
}

int main()
{
	// Produce all the indices in the example figure, left to right, top to
	// bottom.
	const int treeDepth = 4;
	const int numXSubdivs = 4;
	const int numYSubdivs = 2;

	std::cout << "The following indices should correspond to the example figure in the source\n";
	for(int y = 0; y < numYSubdivs; ++y)
	{
		for(int x = 0; x < numXSubdivs; ++x)
		{
			Point p = Point((x+0.5)/numXSubdivs,(y+0.5)/numYSubdivs);
			std::cout << treeIndexForPoint_fast(treeDepth, p) << ", ";
		}
		std::cout << "\n";
	}

	unitTests();

	return 0;
}
