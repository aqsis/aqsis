
// A C++ Implicit Surface Polygonizer
// Copyright 2002-2006, Romain Behar <romainbehar@yahoo.com>
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

#ifndef JULES_BLOOMENTHAL_H_INCLUDED
#define JULES_BLOOMENTHAL_H_INCLUDED 1

#include <map>
#include <stack>
#include <vector>

#include <aqsis/ri/ri.h>
#include <aqsis/math/vector3d.h>


namespace Aqsis {


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


#include <aqsis/inttype.h>

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
		Location(const TqInt I = 0, const TqInt J = 0, const TqInt K = 0) :
				i(I),
				j(J),
				k(K)
		{}

		inline friend bool operator == (const Location& a, const Location& b)
		{
			return (a.i == b.i) && (a.j == b.j) && (a.k == b.k);
		}
		inline friend Location operator + (const Location& a, const Location& b)
		{
			return Location(a.i + b.i, a.j + b.j, a.k + b.k);
		}
		inline friend bool operator <= (const Location& a, const Location& b)
		{
			return (a.i <= b.i && a.j <= b.j && a.k <= b.k);
		}
		inline friend bool operator < (const Location& a, const Location& b)
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

		TqInt i;
		TqInt j;
		TqInt k;
};

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
			TqInt key = loc.i + loc.j + loc.k;
			m_table[key].push_back(std::pair<Location, type_t>(loc, item));
		}

		bool get
			(const Location& loc, type_t& out)
		{
			TqInt key = loc.i + loc.j + loc.k;
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
		std::map<unsigned long, std::vector< std::pair<Location, type_t> > > m_table;
};

// bloomenthal_polygonizer implementation
class bloomenthal_polygonizer
{
	public:
		typedef enum
		{
		    MARCHINGCUBES,
		    TETRAHEDRAL
	} polygonization_t;

		bloomenthal_polygonizer(
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
		    std::vector<std::vector<TqInt> >& surface_polygons);

		~bloomenthal_polygonizer();

		bool polygonize_from_inside_point(const CqVector3D& startingpoint);

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
				TqDouble value;

				Corner(const Location& L) :
						l(L)
				{}

				inline friend bool operator == (const Corner& c1, const Corner& c2)
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
					for(TqInt i = 0; i < 8; i++)
						corners[i] = 0;
				}
		};

		class Edge
		{
			public:
				Edge(const Location& L1, const Location& L2, const TqInt VID = -1) :
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

				inline friend bool operator == (const Edge& e1, const Edge& e2)
				{
					return (e1.l1 == e2.l1) && (e1.l2 == e2.l2) && (e1.vid == e2.vid);
				}

				Location l1;
				Location l2;
				TqInt vid;
		};

		class EdgeHash
		{
			private:
				static const TqInt HashBit;
				static const TqInt Mask;
				static const TqInt HashSize;

				inline TqInt HashFunc(const Location& l)
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
					TqInt index = HashFunc(Value.l1) + HashFunc(Value.l2);
					edges[index].push_back(Value);
				}

				TqInt GetValue(const Edge& Value)
				{
					TqInt index = HashFunc(Value.l1) + HashFunc(Value.l2);
					for(TqInt n = 0; n < static_cast<TqInt>(edges[index].size()); n++)
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

		// Polygonization type
		polygonization_t m_Decomposition;
		// Width of the partitioning cube
		TqDouble m_VoxelSize;
		// Threshold value (defining the equipotential surface)
		TqDouble m_Threshold;
		// Grid limit corners (left-bottom-near and right-top-far)
		Location m_MinCorner;
		Location m_MaxCorner;
		bool m_keep_within_limits;
		// Grid center ( Location(0, 0, 0) )
		CqVector3D m_GridOrigin;
		// Implicit function
		implicit_functor& m_FieldFunctor;
		// Surface storage
		std::vector<CqVector3D>& m_Vertices;
		std::vector<CqVector3D>& m_Normals;
		std::vector<std::vector<TqInt> >& m_Polygons;

		/// Temp storage

		// Active cubes
		std::stack<Cube> m_active_cubes;

		// Centers hash
		LocationMap<bool> m_centers;
		// Return true if already set, otherwise set and return false
		bool mark_center(const Location& l)
		{
			bool out;
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

		// Build fast Marching Cube tables
		std::vector< std::vector< std::vector<TqInt> > > m_CubeTable;

		// Convert between vertex and Location
		CqVector3D location_vertex(const Location& l);
		Location nearest_location(const CqVector3D& p);

		void PolygonizeSurface(const Location& startinglocation);

		// Inline functions
		inline TqInt bit_value(TqInt number, TqInt bit_number)
		{
			return (number >> bit_number) & 1;
		}
		inline TqInt invert_bit(TqInt i, TqInt bit)
		{
			return i ^ (1 << bit);
		}

		CqVector3D normal(const CqVector3D& Point);

		bool SurfaceLocation(Location& startinglocation);

		// Tetrahedral Polygonization
		void TriangulateTet(const Cube& cube1, TqInt c1, TqInt c2, TqInt c3, TqInt c4);

		// Cubical Polygonization
		void MakeCubeTable();
		void MarchingCube(const Cube& cube1);

		void TestFace(const Location& facelocation, Cube& old, TqInt face, TqInt c1, TqInt c2, TqInt c3, TqInt c4);

		TqInt VerticeId(Corner *c1, Corner *c2);
		void Converge(const CqVector3D& p1, const CqVector3D& p2, double v, CqVector3D& p);

		void SaveTriangle(TqInt u, TqInt v, TqInt w);
};

//-----------------------------------------------------------------------

} // namespace Aqsis

#endif // JULES_BLOOMENTHAL_H_INCLUDED


