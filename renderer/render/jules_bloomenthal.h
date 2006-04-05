// A C++ Implicit Surface Polygonizer
// Copyright 2002-2005, Romain Behar <romainbehar@yahoo.com>
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
		\brief Declares bloomenthal_polygonizer, an implicit surface polygonizer
		\author Romain Behar (romainbehar@yahoo.com)
*/

//? Is .h included already?
#ifndef JULES_BLOOMENTHAL_H_INCLUDED
#define JULES_BLOOMENTHAL_H_INCLUDED 1

#include "ri.h"
#include "vector3d.h"

#include <map>
#include <stack>
#include <vector>

START_NAMESPACE( Aqsis )

// It is based on Jules Bloomenthal's work :
//
// C code from the article
// "An Implicit Surface Polygonizer"
// by Jules Bloomenthal, jbloom@beauty.gmu.edu
// in "Graphics Gems IV", Academic Press, 1994
//
// implicit.c
//     an implicit surface polygonizer, translated from Mesa
//     applications should call polygonize()
//
// Authored by Jules Bloomenthal, Xerox PARC.
// Copyright (c) Xerox Corporation, 1991.  All rights reserved.
// Permission is granted to reproduce, use and distribute this code for
// any and all purposes, provided that this notice appears in all copies.

#include "aqsis_types.h"

class implicit_functor
{
	public:
		virtual ~implicit_functor()
		{}

		virtual TqFloat implicit_value(const CqVector3D& point) = 0;
};


// Lattice position (centered on (0, 0, 0), signed values)
class Location
{
	public:
		Location(const int I = 0, const int J = 0, const int K = 0) :
				i(I),
				j(J),
				k(K)
		{}

		inline friend TqBool operator == (const Location& a, const Location& b)
		{
			return (a.i == b.i) && (a.j == b.j) && (a.k == b.k);
		}
		inline friend Location operator + (const Location& a, const Location& b)
		{
			return Location(a.i + b.i, a.j + b.j, a.k + b.k);
		}
		inline friend TqBool operator <= (const Location& a, const Location& b)
		{
			return (a.i <= b.i && a.j <= b.j && a.k <= b.k);
		}
		inline friend TqBool operator < (const Location& a, const Location& b)
		{
			return (a.i < b.i && a.j < b.j && a.k < b.k);
		}

		friend std::ostream& operator << (std::ostream& Stream, const Location& RHS)
		{
			Stream << RHS.i << " " << RHS.j << " " << RHS.k;
			return Stream;
		}

		Location Left()
		{
			return Location(i-1, j, k);
		}
		Location Right()
		{
			return Location(i+1, j, k);
		}
		Location Bottom()
		{
			return Location(i, j-1, k);
		}
		Location Top()
		{
			return Location(i, j+1, k);
		}
		Location Near()
		{
			return Location(i, j, k-1);
		}
		Location Far()
		{
			return Location(i, j, k+1);
		}

		int i;
		int j;
		int k;
};

/*
class LocationHash
{
public:
	inline int HashFunc(const Location& Value)
	{
		static const int HashBit = 5;
		static const int Mask = (1 << HashBit) - 1;
		return ((Value.i & Mask) << (HashBit*2)) | ((Value.j & Mask) << HashBit) | (Value.k & Mask);
	}
};
*/

template<typename type_t>
class LocationMap
{
	public:
		typedef std::vector< std::pair<Location, type_t> > table_t;

		LocationMap()
		{}
		~LocationMap()
		{}

		void insert(const Location& loc, const type_t item)
		{
			int key = loc.i + loc.j + loc.k;
			m_table[key].push_back(std::pair<Location, type_t>(loc, item));
		}

		TqBool get
			(const Location& loc, type_t& out)
		{
			int key = loc.i + loc.j + loc.k;
			table_t& table = m_table[key];
			for(typename table_t::const_iterator t = table.begin(); t != table.end(); t++)
				if(t->first == loc)
				{
					out = t->second;
					return true;
				}

			return false;
		}

	private:
		std::map<TqInt, std::vector< std::pair<Location, type_t> > > m_table;
};

// bloomenthal_polygonizer implementation
class bloomenthal_polygonizer
{
	public:
		bloomenthal_polygonizer(
		    const TqFloat voxel_size,
		    const TqFloat threshold,
		    const TqInt xmin, const TqInt xmax,
		    const TqInt ymin, const TqInt ymax,
		    const TqInt zmin, const TqInt zmax,
		    const CqVector3D& origin,
		    implicit_functor& functor,
		    std::vector<CqVector3D>& surface_vertices,
		    std::vector<CqVector3D>& surface_normals,
		    std::vector<std::vector<TqInt> >& surface_polygons);

		~bloomenthal_polygonizer();

		TqBool polygonize_from_inside_point(const CqVector3D& startingpoint);

		void polygonize_whole_grid();

		void cross_limits()
		{
			m_keep_within_limits = false;
		}

		// Cube corner
		class Corner
		{
			public:
				Location l;
				CqVector3D p;
				TqFloat value;

				Corner(const Location& L) :
						l(L)
				{}

				inline friend TqBool operator == (const Corner& c1, const Corner& c2)
				{
					return (c1.l == c2.l) && (c1.p == c2.p) && (c1.value == c2.value);
				}
		};

		// Partitioning cell
		class Cube
		{
			public:
				Location l;
				Corner* corners[8];

				Cube(const Location& L) :
						l(L)
				{
					for(int i = 0; i < 8; i++)
						corners[i] = 0;
				}
		};

		void MarchingCube(const Cube& cube1);
		void MakeCubeTable();
		std::vector< std::vector< std::vector<TqInt> > > m_CubeTable;

		class Edge
		{
			public:
				Edge(const Location& L1, const Location& L2, const int VID = -1) :
						vid(VID)
				{
					if(L1.i > L2.i || (L1.i == L2.i && (L1.j > L2.j || (L1.j == L2.j && L1.k > L2.k))))
					{
						l1 = L2;
						l2 = L1;
					}
					else
					{
						l1 = L1;
						l2 = L2;
					}
				}

				inline friend TqBool operator == (const Edge& e1, const Edge& e2)
				{
					return (e1.l1 == e2.l1) && (e1.l2 == e2.l2) && (e1.vid == e2.vid);
				}

				Location l1;
				Location l2;
				int vid;
		};

		class EdgeHash
		{
			private:
				static const int HashBit;
				static const int Mask;
				static const int HashSize;

				inline int HashFunc(const Location& l)
				{
					return ((((l.i & Mask) << HashBit) | (l.j & Mask)) << HashBit) | (l.k & Mask);
				}

			public:
				EdgeHash()
				{
					edges.resize(HashSize*2);
				}

				void push_back(const Edge& Value)
				{
					int index = HashFunc(Value.l1) + HashFunc(Value.l2);
					edges[index].push_back(Value);
				}

				int GetValue(const Edge& Value)
				{
					int index = HashFunc(Value.l1) + HashFunc(Value.l2);
					for(unsigned int n = 0; n < edges[index].size(); n++)
					{
						if(edges[index][n].l1 == Value.l1 && edges[index][n].l2 == Value.l2)
							return edges[index][n].vid;
					}

					return -1;
				}

			protected:
				std::vector< std::vector<Edge> > edges;
		};

	private:
		/// Polygonizer parameters

		// Width of the partitioning cube
		TqFloat m_VoxelSize;
		// Threshold value (defining the equipotential surface)
		TqFloat m_Threshold;
		// Grid limit corners (left-bottom-near and right-top-far)
		Location m_MinCorner;
		Location m_MaxCorner;
		TqBool m_keep_within_limits;
		// Grid center ( Location(0, 0, 0) )
		CqVector3D m_GridOrigin;
		// Implicit function
		implicit_functor& m_FieldFunctor;
		// Surface storage
		std::vector<CqVector3D>& m_Vertices;
		std::vector<CqVector3D>& m_normals;
		std::vector<std::vector<TqInt> >& m_Polygons;

		/// Temp storage

		// Active cubes
		std::stack<Cube> m_active_cubes;

		// Centers hash
		LocationMap<TqBool> m_centers;
		// Return true if already set, otherwise set and return false
		TqBool mark_center(const Location& l)
		{
			TqBool out;
			if(m_centers.get(l, out))
				return true;

			m_centers.insert(l, true);
			return false;
		}

		// Corners hash
		LocationMap<Corner*> m_Corners;
		// Return corner if found, else return 0
		Corner* get_corner(const Location& l)
		{
			Corner* out;
			if(m_Corners.get(l, out))
				return out;

			return 0;
		}

		Corner* get_cached_corner(const Location& l);

		// Edge hash
		EdgeHash m_Edges;

		// Convert between vertex and Location
		CqVector3D location_vertex(const Location& l);
		Location nearest_location(const CqVector3D& p);

		void PolygonizeSurface(const Location& startinglocation);

		// Inline functions
		inline int bit_value(int number, int bit_number)
		{
			return (number >> bit_number) & 1;
		}
		inline int invert_bit(int i, int bit)
		{
			return i ^ (1 << bit);
		}

		CqVector3D normal(const CqVector3D& Point);

		TqBool SurfaceLocation(Location& startinglocation);

		// Tetrahedral Polygonization
		void TriangulateTet(const Cube& cube1, int c1, int c2, int c3, int c4);

		void TestFace(const Location& facelocation, Cube& old, int face, int c1, int c2, int c3, int c4);

		int VerticeId(Corner *c1, Corner *c2);
		void Converge(const CqVector3D& p1, const CqVector3D& p2, TqFloat v, CqVector3D& p);

		void SaveTriangle(TqInt u, TqInt v, TqInt w);
};

//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

// End of #ifdef JULES_BLOOMENTHAL_H_INCLUDED
#endif

