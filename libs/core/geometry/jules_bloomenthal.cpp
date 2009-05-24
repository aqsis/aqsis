// A C++ Implicit Surface Polygonizer
// Copyright 2002-2004, Romain Behar <romainbehar@yahoo.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

/** \file
		\brief Implements bloomenthal_polygonizer
		\author Romain Behar (romainbehar@yahoo.com)
		\author Michel Joron (joron@sympatico.ca)
 
*/

#include <iostream>
#include <cmath>
#include <math.h>
#include <vector>
#include <list>
#include <limits>
#include <algorithm>

#include <aqsis/aqsis.h>
#include <aqsis/ri/ri.h>
#include <aqsis/math/vector4d.h>
#include <aqsis/math/matrix.h>
#include "jules_bloomenthal.h"
#include <aqsis/util/logging.h>

namespace Aqsis {

// Number of iterations (convergence)
const TqInt RES = 10;

const TqInt bloomenthal_polygonizer::EdgeHash::HashBit = 5;
const TqInt bloomenthal_polygonizer::EdgeHash::Mask = (1 << HashBit) - 1;
const TqInt bloomenthal_polygonizer::EdgeHash::HashSize = 1 << (3 * HashBit);

// Directions
const TqInt L = 0;	// left:	-x, -i
const TqInt R = 1;	// right:	+x, +i
const TqInt B = 2;	// bottom:	-y, -j
const TqInt T = 3;	// top:		+y, +j
const TqInt N = 4;	// near:	-z, -k
const TqInt F = 5;	// far:		+z, +k

// Corners
const TqInt LBN = 0;	// left bottom near
const TqInt LBF = 1;	// left bottom far
const TqInt LTN = 2;	// left top near
const TqInt LTF = 3;	// left top far
const TqInt RBN = 4;	// right bottom near
const TqInt RBF = 5;	// right bottom far
const TqInt RTN = 6;	// right top near
const TqInt RTF = 7;	// right top far



bloomenthal_polygonizer::bloomenthal_polygonizer(
    const polygonization_t polygonization_type,
    const TqDouble voxel_size,
    const TqDouble threshold,
    const TqInt xmin, const TqInt xmax,
    const TqInt ymin, const TqInt ymax,
    const TqInt zmin, const TqInt zmax,
    const CqVector3D& origin,
    implicit_functor& functor,
    std::vector<CqVector3D>& surface_vertices,
    std::vector<CqVector3D>& surface_normals,
    std::vector<std::vector<TqInt> >& surface_polygons) :
		m_Decomposition(polygonization_type),
		m_VoxelSize(voxel_size),
		m_Threshold(threshold),
		m_MinCorner(Location(xmin, ymin, zmin)),
		m_MaxCorner(Location(xmax, ymax, zmax)),
		m_keep_within_limits(true),
		m_GridOrigin(origin),
		m_FieldFunctor(functor),
		m_Vertices(surface_vertices),
		m_Normals(surface_normals),
		m_Polygons(surface_polygons)
{
	// Sanity checks ...
	if(!(m_MinCorner <= nearest_location(m_GridOrigin) && nearest_location(m_GridOrigin) < m_MaxCorner))
	{
		Aqsis::log() << warning << "Surface Polygonizer: grid origin must be in grid, defaulting to min corner" << std::endl;
		m_GridOrigin = location_vertex(m_MinCorner);
	}

	// Init polygonizing algorithm
	if(m_Decomposition == MARCHINGCUBES)
		MakeCubeTable();
}

bloomenthal_polygonizer::~bloomenthal_polygonizer()
{
	// Delete corners
}

// Return the CqVector3D corresponding to the Location
CqVector3D bloomenthal_polygonizer::location_vertex(const Location& l)
{
	return m_GridOrigin + m_VoxelSize * CqVector3D((TqDouble)l.i, (TqDouble)l.j, (TqDouble)l.k);
}

// Return the nearest location corresponding to the CqVector3D
Location bloomenthal_polygonizer::nearest_location(const CqVector3D& point)
{
	CqVector3D vertex_position = (point - m_GridOrigin) / m_VoxelSize;

	TqInt i = static_cast<TqInt>(vertex_position[0]);
	TqInt j = static_cast<TqInt>(vertex_position[1]);
	TqInt k = static_cast<TqInt>(vertex_position[2]);

	return Location(i, j, k);
}

// Sample the whole grid and polygonize
void bloomenthal_polygonizer::polygonize_whole_grid()
{
	for(Location x = m_MinCorner; x <= m_MaxCorner; x = x.Right())
		for(Location y = x; y <= m_MaxCorner; y = y.Top())
			for(Location z = y; z <= m_MaxCorner; z = z.Far())
			{
				Corner* corner = get_cached_corner(z);
				if(corner->value < m_Threshold)
					continue;

				Location surface_location = z;
				if(SurfaceLocation(surface_location))
					PolygonizeSurface(surface_location);
			}
}

// Find surface and polygonize from a known inside point
bool bloomenthal_polygonizer::polygonize_from_inside_point(const CqVector3D& starting_point)
{
	Location starting_location = nearest_location(starting_point);

	// Make sure the point is inside a surface
	Corner* corner = get_cached_corner(starting_location);
	if(corner->value < m_Threshold)
		return false;

	// Get a Location enclosing surface
	if(!SurfaceLocation(starting_location))
		return false;

	// Surface found, polygonize it
	PolygonizeSurface(starting_location);

	return true;
}

void bloomenthal_polygonizer::PolygonizeSurface(const Location& startinglocation)
{
	// Create initial cube
	if(mark_center(startinglocation))
		return;

	Cube c(startinglocation);
	for(TqInt n = 0; n < 8; n++)
		c.corners[n] = get_cached_corner(startinglocation + Location(bit_value(n, 2), bit_value(n, 1), bit_value(n, 0)));

	// Push it on stack
	m_active_cubes.push(c);

	// Process active cubes till none left
	while(!m_active_cubes.empty())
	{
		Cube c = m_active_cubes.top();
		m_active_cubes.pop();

		// Polygonize
		switch(m_Decomposition)
		{
				case MARCHINGCUBES:
				MarchingCube(c);
				break;
				case TETRAHEDRAL:
				// Decompose into tetrahedra and polygonize
				TriangulateTet(c, LBN, LTN, RBN, LBF);
				TriangulateTet(c, RTN, LTN, LBF, RBN);
				TriangulateTet(c, RTN, LTN, LTF, LBF);
				TriangulateTet(c, RTN, RBN, LBF, RBF);
				TriangulateTet(c, RTN, LBF, LTF, RBF);
				TriangulateTet(c, RTN, LTF, RTF, RBF);
				break;
				default:
				Aqsis::log() << warning << "Unknow decomposition " << std::endl;
				MarchingCube(c);
				break;

		}

		// Test six face directions, maybe add to stack
		TestFace(c.l.Left(), c, L, LBN, LBF, LTN, LTF);
		TestFace(c.l.Right(), c, R, RBN, RBF, RTN, RTF);
		TestFace(c.l.Bottom(), c, B, LBN, LBF, RBN, RBF);
		TestFace(c.l.Top(), c, T, LTN, LTF, RTN, RTF);
		TestFace(c.l.Near(), c, N, LBN, LTN, RBN, RTN);
		TestFace(c.l.Far(), c, F, LBF, LTF, RBF, RTF);
	}
}

// Find a location enclosing surface
bool bloomenthal_polygonizer::SurfaceLocation(Location& startinglocation)
{
	Location loc2 = startinglocation;
	TqDouble value2 = m_FieldFunctor.implicit_value(location_vertex(loc2)) - m_Threshold;

	// Top
	do
	{
		Location loc1 = loc2;
		TqDouble value1 = value2;

		loc2 = loc2.Top();
		value2 = m_FieldFunctor.implicit_value(location_vertex(loc2)) - m_Threshold;

		if((value1*value2 < 0) || ((value1 == 0) && (value2 < 0)) || ((value2 == 0) && (value1 < 0)))
		{
			startinglocation = loc1;
			return true;
		}
	}
	while(loc2 <= m_MaxCorner);

	// We reached the grid boundary: check the whole grid
	return false;
}

// Triangulate the tetrahedron (b, c, d should appear clockwise when viewed from a)
void bloomenthal_polygonizer::TriangulateTet(const Cube& cube1, TqInt c1, TqInt c2, TqInt c3, TqInt c4)
{
	Corner *a = cube1.corners[c1];
	Corner *b = cube1.corners[c2];
	Corner *c = cube1.corners[c3];
	Corner *d = cube1.corners[c4];

	bool apos = (a->value >= m_Threshold);
	bool bpos = (b->value >= m_Threshold);
	bool cpos = (c->value >= m_Threshold);
	bool dpos = (d->value >= m_Threshold);

	TqInt index = 0;
	if(apos)
		index += 8;
	if(bpos)
		index += 4;
	if(cpos)
		index += 2;
	if(dpos)
		index += 1;

	// Index is now 4-bit number representing one of the 16 possible cases
	TqInt e1 = 0;
	TqInt e2 = 0;
	TqInt e3 = 0;
	TqInt e4 = 0;
	TqInt e5 = 0;
	TqInt e6 = 0;
	if(apos != bpos)
		e1 = VerticeId(a, b);
	if(apos != cpos)
		e2 = VerticeId(a, c);
	if(apos != dpos)
		e3 = VerticeId(a, d);
	if(bpos != cpos)
		e4 = VerticeId(b, c);
	if(bpos != dpos)
		e5 = VerticeId(b, d);
	if(cpos != dpos)
		e6 = VerticeId(c, d);

	// 14 productive tetrahedral cases (0000 and 1111 do not yield polygons)
	switch(index)
	{
			case 1:
			SaveTriangle(e5, e6, e3);
			break;
			case 2:
			SaveTriangle(e2, e6, e4);
			break;
			case 3:
			SaveTriangle(e3, e5, e4);
			SaveTriangle(e3, e4, e2);
			break;
			case 4:
			SaveTriangle(e1, e4, e5);
			break;
			case 5:
			SaveTriangle(e3, e1, e4);
			SaveTriangle(e3, e4, e6);
			break;
			case 6:
			SaveTriangle(e1, e2, e6);
			SaveTriangle(e1, e6, e5);
			break;
			case 7:
			SaveTriangle(e1, e2, e3);
			break;
			case 8:
			SaveTriangle(e1, e3, e2);
			break;
			case 9:
			SaveTriangle(e1, e5, e6);
			SaveTriangle(e1, e6, e2);
			break;
			case 10:
			SaveTriangle(e1, e3, e6);
			SaveTriangle(e1, e6, e4);
			break;
			case 11:
			SaveTriangle(e1, e5, e4);
			break;
			case 12:
			SaveTriangle(e3, e2, e4);
			SaveTriangle(e3, e4, e5);
			break;
			case 13:
			SaveTriangle(e6, e2, e4);
			break;
			case 14:
			SaveTriangle(e5, e3, e6);
			break;
	}
}

namespace mc
{

// Edges
const TqInt LB = 0;	// left bottom
const TqInt LT = 1;	// left top
const TqInt LN = 2;	// left near
const TqInt LF = 3;	// left far
const TqInt RB = 4;	// right bottom
const TqInt RT = 5;	// right top
const TqInt RN = 6;	// right near
const TqInt RF = 7;	// right far
const TqInt BN = 8;	// bottom near
const TqInt BF = 9;	// bottom far
const TqInt TN = 10;	// top near
const TqInt TF = 11;	// top far

// Face on left when going from corner1 to corner2
const TqInt leftface[12] =
    {
        B, L, L, F, R, T, N, R, N, B, T, F
    };
// Face on right when going from corner1 to corner2
const TqInt rightface[12] =
    {
        L, T, N, L, B, R, R, F, B, F, N, T
    };

// Return next clockwise edge from given edge around given face
TqInt next_edge(const TqInt edge, const TqInt face)
{
	switch(edge)
	{
			case LB:
			return (face == L) ? LF : BN;
			case LT:
			return (face == L) ? LN : TF;
			case LN:
			return (face == L) ? LB : TN;
			case LF:
			return (face == L) ? LT : BF;
			case RB:
			return (face == R) ? RN : BF;
			case RT:
			return (face == R) ? RF : TN;
			case RN:
			return (face == R) ? RT : BN;
			case RF:
			return (face == R) ? RB : TF;
			case BN:
			return (face == B) ? RB : LN;
			case BF:
			return (face == B) ? LB : RF;
			case TN:
			return (face == T) ? LT : RN;
			case TF:
			return (face == T) ? RT : LF;
			default:
			Aqsis::log() << warning << "Default case for next_edge()" << std::endl;
			break;
	}

	return LF;
}

// Corners
// edges:                   LB, LT, LN, LF, RB, RT, RN, RF, BN, BF, TN, TF
const TqInt corner1[12] =
    {
        LBN,LTN,LBN,LBF,RBN,RTN,RBN,RBF,LBN,LBF,LTN,LTF
    };
const TqInt corner2[12] =
    {
        LBF,LTF,LTN,LTF,RBF,RTF,RTN,RTF,RBN,RBF,RTN,RTF
    };

} // namespace mc

// Triangulate the cube directly, without decomposition
void bloomenthal_polygonizer::MarchingCube(const Cube& cube1)
{
	TqInt index = 0;
	TqInt i;
	for( i= 0; i < 8; i++)
		if(cube1.corners[i]->value >= m_Threshold)
			index += 1 << i;

	std::vector< std::vector<TqInt> > currentindex = m_CubeTable[index];
	for( i = 0; i < static_cast<TqInt>(currentindex.size()); i++)
	{
		TqInt a = 0;
		TqInt b = 0;
		TqInt count = 0;

		for(TqInt j = 0; j < static_cast<TqInt>(currentindex[i].size()); j++)
		{
			Corner* c1 = cube1.corners[mc::corner1[currentindex[i][j]]];
			Corner* c2 = cube1.corners[mc::corner2[currentindex[i][j]]];
			TqInt c = VerticeId(c1, c2);

			if(++count > 2)
				SaveTriangle(a, b, c);

			if(count < 3)
				a = b;
			b = c;
		}
	}
}

// Create the 256 entry table for cubical polygonization
void bloomenthal_polygonizer::MakeCubeTable()
{
	for(TqInt configuration = 0; configuration < 256; configuration++)
	{
		std::vector< std::vector<TqInt> > triangles;
		TqInt edge;

		TqInt bits[8];
		for(TqInt c = 0; c < 8; c++)
			bits[c] = bit_value(configuration, c);

		bool done[12];
		for(edge = 0; edge < 12; edge++)
			done[edge] = false;
		for( edge = 0; edge < 12; edge++)
			if(!done[edge] && (bits[mc::corner1[edge]] != bits[mc::corner2[edge]]))
			{
				std::vector<TqInt> triangle;

				// Get face that is to right of edge from pos to neg corner
				TqInt face = bits[mc::corner1[edge]] ? mc::rightface[edge] : mc::leftface[edge];

				TqInt startingedge = edge, currentedge = edge;
				do
				{
					currentedge = mc::next_edge(currentedge, face);
					done[currentedge] = true;

					if(bits[mc::corner1[currentedge]] != bits[mc::corner2[currentedge]])
					{
						triangle.push_back(currentedge);

						// face adjoining edge that is not the given face
						if(face == mc::leftface[currentedge])
							face = mc::rightface[currentedge];
						else
							face = mc::leftface[currentedge];
					}
				}
				while(currentedge != startingedge);

				triangles.push_back(triangle);
			}

		m_CubeTable.push_back(triangles);
	}

	for(TqInt i = 0, numI = m_CubeTable.size(); i < numI; i++)
		for(TqInt j = 0, numJ = m_CubeTable[i].size(); j < numJ; j++)
			std::reverse(m_CubeTable[i][j].begin(), m_CubeTable[i][j].end());
}

//**** Storage ****

// Given cube at lattice (i, j, k), and four corners of face,
// if surface crosses face, compute other four corners of adjacent cube
// and add new cube to cube stack

void bloomenthal_polygonizer::TestFace(const Location& facelocation, Cube& old, TqInt face, TqInt c1, TqInt c2, TqInt c3, TqInt c4)
{
	// No surface crossing?
	bool pos = old.corners[c1]->value >= m_Threshold;
	if(((old.corners[c2]->value >= m_Threshold) == pos) &&
	        ((old.corners[c3]->value >= m_Threshold) == pos) &&
	        ((old.corners[c4]->value >= m_Threshold) == pos))
		return;

	// Out of bounds?
	if(m_keep_within_limits && !(m_MinCorner <= facelocation && facelocation < m_MaxCorner))
		return;

	// Already visited?
	if(mark_center(facelocation))
		return;

	// Create new cube and add it to top of stack
	Cube newc(facelocation);

	const TqInt facebit[6] =
	    {
	        2, 2, 1, 1, 0, 0
	    };
	TqInt bit = facebit[face];
	newc.corners[invert_bit(c1, bit)] = old.corners[c1];
	newc.corners[invert_bit(c2, bit)] = old.corners[c2];
	newc.corners[invert_bit(c3, bit)] = old.corners[c3];
	newc.corners[invert_bit(c4, bit)] = old.corners[c4];

	for(TqInt n = 0; n < 8; n++)
		if(!newc.corners[n])
			newc.corners[n] = get_cached_corner(facelocation + Location(bit_value(n, 2), bit_value(n, 1), bit_value(n, 0)));

	m_active_cubes.push(newc);
}

// Return the gradient at Location l
CqVector3D bloomenthal_polygonizer::normal(const CqVector3D& Point)
{
	TqDouble delta = m_VoxelSize / static_cast<TqDouble>(RES*RES);

	TqDouble f = m_FieldFunctor.implicit_value(Point);
	TqDouble gx = m_FieldFunctor.implicit_value(Point + CqVector3D(delta, 0, 0)) - f;
	TqDouble gy = m_FieldFunctor.implicit_value(Point + CqVector3D(0, delta, 0)) - f;
	TqDouble gz = m_FieldFunctor.implicit_value(Point + CqVector3D(0, 0, delta)) - f;
	f = sqrt(gx*gx + gy*gy + gz*gz);
	if(f != 0)
	{
		gx /= f;
		gy /= f;
		gz /= f;
	}

	return CqVector3D(gx, gy, gz);
}

// Return cached corner with the given lattice Location
bloomenthal_polygonizer::Corner* bloomenthal_polygonizer::get_cached_corner(const Location& L)
{
	Corner* c = get_corner(L);
	if(!c)
	{
		c = new Corner(L);
		c->p = location_vertex(L);
		c->value = m_FieldFunctor.implicit_value(c->p);

		m_Corners.insert(L, c);
	}

	return c;
}

// Save a triangle
void bloomenthal_polygonizer::SaveTriangle(TqInt u, TqInt v, TqInt w)
{
	std::vector<TqInt> triangle;
	triangle.push_back(u);
	triangle.push_back(v);
	triangle.push_back(w);

	m_Polygons.push_back(triangle);
}

// Return index for vertex on edge
TqInt bloomenthal_polygonizer::VerticeId(Corner *c1, Corner *c2)
// c1->value and c2->value are presumed one on each side of the equipotential surface
{
	TqInt vid = m_Edges.GetValue(Edge(c1->l, c2->l));
	if(vid != -1)
	{
		// Has been previously computed, return saved index
		return vid;
	}

	// Compute index, save and return it
	CqVector3D p;
	Converge(c1->p, c2->p, c1->value, p);
	m_Vertices.push_back(p);
	m_Normals.push_back(normal(p));

	vid = m_Vertices.size() - 1;
	m_Edges.push_back(Edge(c1->l, c2->l, vid));

	return vid;
}

// From two points of differing sign, converge to zero crossing
void bloomenthal_polygonizer::Converge(const CqVector3D& p1, const CqVector3D& p2, TqDouble v, CqVector3D& point)
{
	CqVector3D pos = p1;
	CqVector3D neg = p2;

	if(v < m_Threshold)
		std::swap(pos, neg);

	point = 0.5 * (pos + neg);

	for(TqInt iter = 0; iter < RES; iter++)
	{
		if(m_FieldFunctor.implicit_value(point) >= m_Threshold)
			pos = point;
		else
			neg = point;

		point = 0.5 * (pos + neg);
	}
}



} // namespace Aqsis
//---------------------------------------------------------------------

