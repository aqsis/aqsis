// Aqsis
// Copyright (C) 2006, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA



/** \file
    \brief MarchingCubes Algorithm
    \author Thomas Lewiner <thomas.lewiner@polytechnique.org>
    \author Math Dept, PUC-Rio
    \version 0.2
    \date    12/08/2002
*/

#ifndef _MARCHINGCUBES_H_
#define _MARCHINGCUBES_H_

#ifndef AQSIS_SYSTEM_WIN32
#pragma interface
#endif //AQSIS_SYSTEM_WIN32

#include <aqsis/aqsis.h>
#include "renderer.h"

namespace Aqsis {

// types

//-----------------------------------------------------------------------------
// Vertex structure
/** \struct Vertex "MarchingCubes.h" MarchingCubes
 * Position and normal of a vertex
 * \brief vertex structure
 * \param x X coordinate
 * \param y Y coordinate
 * \param z Z coordinate
 * \param nx X component of the normal
 * \param ny Y component of the normal
 * \param nz Z component of the normal
 */
typedef struct
{
  TqFloat  x,  y,  z ;  /**< Vertex coordinates */
#ifdef COMPUTE_NORMALS
  TqFloat nx, ny, nz ;  /**< Vertex normal */
#endif
} Vertex ;

//-----------------------------------------------------------------------------
// Triangle structure
/** \struct Triangle "MarchingCubes.h" MarchingCubes
 * Indices of the oriented triange vertices
 * \brief triangle structure
 * \param v1 First vertex index
 * \param v2 Second vertex index
 * \param v3 Third vertex index
 */
typedef struct
{
  TqInt v1,v2,v3 ;  /**< Triangle vertices */
} Triangle ;
//_____________________________________________________________________________



//_____________________________________________________________________________
/** Marching Cubes algorithm wrapper */
class MarchingCubes
//-----------------------------------------------------------------------------
{
// Constructors
public :
  /**
   * Main and default constructor
   * \brief constructor
   * \param size_x width  of the grid
   * \param size_y depth  of the grid
   * \param size_z height of the grid
   */
  MarchingCubes ( const TqInt size_x = -1, const TqInt size_y = -1, const TqInt size_z = -1 ) ;
  /** Destructor */
  ~MarchingCubes() ;

//-----------------------------------------------------------------------------
// Accessors
public :
  /** accesses the number of vertices of the generated mesh */
  const TqInt nverts() const { return i_nverts ; }
  /** accesses the number of triangles of the generated mesh */
  const TqInt ntrigs() const { return i_ntrigs ; }
  /** accesses a specific vertex of the generated mesh */
  Vertex   * vert( const TqInt i ) const { if( i < 0  || i >= i_nverts ) return ( Vertex *)NULL ; return i_vertices  + i ; }
  /** accesses a specific triangle of the generated mesh */
  Triangle * trig( const TqInt i ) const { if( i < 0  || i >= i_ntrigs ) return (Triangle*)NULL ; return i_triangles + i ; }

  /** accesses the vertex buffer of the generated mesh */
  Vertex   *vertices () { return i_vertices  ; }
  /** accesses the triangle buffer of the generated mesh */
  Triangle *triangles() { return i_triangles ; }

  /**  accesses the width  of the grid */
  const TqInt size_x() const { return i_size_x ; }
  /**  accesses the depth  of the grid */
  const TqInt size_y() const { return i_size_y ; }
  /**  accesses the height of the grid */
  const TqInt size_z() const { return i_size_z ; }

  /**
   * changes the size of the grid
   * \param size_x width  of the grid
   * \param size_y depth  of the grid
   * \param size_z height of the grid
   */
  void set_resolution( const TqInt size_x, const TqInt size_y, const TqInt size_z ) { i_size_x = size_x ;  i_size_y = size_y ;  i_size_z = size_z ; }
  /**
   * selects wether the algorithm will use the enhanced topologically controlled lookup table or the original MarchingCubes
   * \param originalMC true for the original Marching Cubes
   */
  void set_method    ( const bool originalMC = false ) { i_originalMC = originalMC ; }

  // Data access
  /**
   * accesses a specific cube of the grid
   * \param i abscisse of the cube
   * \param j ordinate of the cube
   * \param k height of the cube
   */
  const TqFloat get_data  ( const TqInt i, const TqInt j, const TqInt k ) const { return i_data[ i + j*i_size_x + k*i_size_x*i_size_y] ; }
  /**
   * sets a specific cube of the grid
   * \param val new value for the cube
   * \param i abscisse of the cube
   * \param j ordinate of the cube
   * \param k height of the cube
   */
  void  set_data  ( const TqFloat val, const TqInt i, const TqInt j, const TqInt k ) { i_data[ i + j*i_size_x + k*i_size_x*i_size_y] = val ; }

  // Data initialization
  /** inits temporary structures (must set sizes before call) : the grid and the vertex index per cube */
  void init_temps () ;
  /** inits all structures (must set sizes before call) : the temporary structures and the mesh buffers */
  void init_all   () ;
  /** clears temporary structures : the grid and the main */
  void clean_temps() ;
  /** clears all structures : the temporary structures and the mesh buffers */
  void clean_all  () ;


//-----------------------------------------------------------------------------
// Exportation
public :
  /**
   * GTS exportation of the generated mesh
   * \param fn  name of the GTS file to create
   * \param bin if true, the GTS will be written in binary mode
   */
  void write( const TqChar *fn, bool bin = false ) ;


//-----------------------------------------------------------------------------
// Algorithm
public :
  /** Main algorithm : must be called after init_all */
  void run() ;

protected :
  /** tesselates one cube */
  void process_cube ()             ;
  /** tests if the components of the tesselation of the cube should be connected by the interior of an ambiguous face */
  bool test_face    ( TqChar face ) ;
  /** tests if the components of the tesselation of the cube should be connected through the interior of the cube */
  bool test_interior( TqChar s )    ;


//-----------------------------------------------------------------------------
// Operations
protected :
  /** computes almost all the vertices of the mesh by interpolation along the cubes edges */
  void compute_intersection_points() ;

  /**
   * routine to add a triangle to the mesh
   * \param trig the code for the triangle as a sequence of edges index
   * \param n    the number of triangles to produce
   * \param v12  the index of the interior vertex to use, if necessary
   */
  void add_triangle ( const TqInt* trig, TqChar n, TqInt v12 = -1 ) ;

  /** tests and eventually doubles the vertex buffer capacity for a new vertex insertion */
  void test_vertex_addition() ;
  /** adds a vertex on the current horizontal edge */
  TqInt add_x_vertex() ;
  /** adds a vertex on the current longitudinal edge */
  TqInt add_y_vertex() ;
  /** adds a vertex on the current vertical edge */
  TqInt add_z_vertex() ;
  /** adds a vertex inside the current cube */
  TqInt add_c_vertex() ;

  /**
   * interpolates the horizontal gradient of the implicit function at the lower vertex of the specified cube
   * \param i abscisse of the cube
   * \param j ordinate of the cube
   * \param k height of the cube
   */
  TqFloat get_x_grad( const TqInt i, const TqInt j, const TqInt k ) const ;
  /**
   * interpolates the longitudinal gradient of the implicit function at the lower vertex of the specified cube
   * \param i abscisse of the cube
   * \param j ordinate of the cube
   * \param k height of the cube
   */
  TqFloat get_y_grad( const TqInt i, const TqInt j, const TqInt k ) const ;
  /**
   * interpolates the vertical gradient of the implicit function at the lower vertex of the specified cube
   * \param i abscisse of the cube
   * \param j ordinate of the cube
   * \param k height of the cube
   */
  TqFloat get_z_grad( const TqInt i, const TqInt j, const TqInt k ) const ;

  /**
   * accesses the pre-computed vertex index on the lower horizontal edge of a specific cube
   * \param i abscisse of the cube
   * \param j ordinate of the cube
   * \param k height of the cube
   */
  TqInt   get_x_vert( const TqInt i, const TqInt j, const TqInt k ) const { return i_x_verts[ i + j*i_size_x + k*i_size_x*i_size_y] ; }
  /**
   * accesses the pre-computed vertex index on the lower longitudinal edge of a specific cube
   * \param i abscisse of the cube
   * \param j ordinate of the cube
   * \param k height of the cube
   */
  TqInt   get_y_vert( const TqInt i, const TqInt j, const TqInt k ) const { return i_y_verts[ i + j*i_size_x + k*i_size_x*i_size_y] ; }
  /**
   * accesses the pre-computed vertex index on the lower vertical edge of a specific cube
   * \param i abscisse of the cube
   * \param j ordinate of the cube
   * \param k height of the cube
   */
  TqInt   get_z_vert( const TqInt i, const TqInt j, const TqInt k ) const { return i_z_verts[ i + j*i_size_x + k*i_size_x*i_size_y] ; }

  /**
   * sets the pre-computed vertex index on the lower horizontal edge of a specific cube
   * \param val the index of the new vertex
   * \param i abscisse of the cube
   * \param j ordinate of the cube
   * \param k height of the cube
   */
  void  set_x_vert( const TqInt val, const TqInt i, const TqInt j, const TqInt k ) { i_x_verts[ i + j*i_size_x + k*i_size_x*i_size_y] = val ; }
  /**
   * sets the pre-computed vertex index on the lower longitudinal edge of a specific cube
   * \param val the index of the new vertex
   * \param i abscisse of the cube
   * \param j ordinate of the cube
   * \param k height of the cube
   */
  void  set_y_vert( const TqInt val, const TqInt i, const TqInt j, const TqInt k ) { i_y_verts[ i + j*i_size_x + k*i_size_x*i_size_y] = val ; }
  /**
   * sets the pre-computed vertex index on the lower vertical edge of a specific cube
   * \param val the index of the new vertex
   * \param i abscisse of the cube
   * \param j ordinate of the cube
   * \param k height of the cube
   */
  void  set_z_vert( const TqInt val, const TqInt i, const TqInt j, const TqInt k ) { i_z_verts[ i + j*i_size_x + k*i_size_x*i_size_y] = val ; }

  /** prints cube for debug */
  void print_cube() ;

//-----------------------------------------------------------------------------
// Elements
protected :
  TqInt       i_originalMC ;   /**< selects wether the algorithm will use the enhanced topologically controlled lookup table or the original MarchingCubes */
  TqInt       i_N[15]      ;   /**< counts the occurence of each case for debug */

  TqInt       i_size_x     ;  /**< width  of the grid */
  TqInt       i_size_y     ;  /**< depth  of the grid */
  TqInt       i_size_z     ;  /**< height of the grid */
  TqFloat    *i_data       ;  /**< implicit function values sampled on the grid */

  TqInt      *i_x_verts    ;  /**< pre-computed vertex indices on the lower horizontal   edge of each cube */
  TqInt      *i_y_verts    ;  /**< pre-computed vertex indices on the lower longitudinal edge of each cube */
  TqInt      *i_z_verts    ;  /**< pre-computed vertex indices on the lower vertical     edge of each cube */

  TqInt       i_nverts     ;  /**< number of allocated vertices  in the vertex   buffer */
  TqInt       i_ntrigs     ;  /**< number of allocated triangles in the triangle buffer */
  TqInt       i_Nverts     ;  /**< size of the vertex   buffer */
  TqInt       i_Ntrigs     ;  /**< size of the triangle buffer */
  Vertex     *i_vertices   ;  /**< vertex   buffer */
  Triangle   *i_triangles  ;  /**< triangle buffer */

  TqInt       i_i          ;  /**< abscisse of the active cube */
  TqInt       i_j          ;  /**< height of the active cube */
  TqInt       i_k          ;  /**< ordinate of the active cube */

  TqFloat     i_cube[8]    ;  /**< values of the implicit function on the active cube */
  TqUchar     i_lut_entry  ;  /**< cube sign representation in [0..255] */
  TqUchar     i_case       ;  /**< case of the active cube in [0..15] */
  TqUchar     i_config     ;  /**< configuration of the active cube */
  TqUchar     i_subconfig  ;  /**< subconfiguration of the active cube */
};
//_____________________________________________________________________________

//---------------------------------------------------------------------
} // namespace Aqsis

#endif // _MARCHINGCUBES_H_

