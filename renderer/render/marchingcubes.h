// Aqsis
// Copyright © 2006, Paul C. Gregory
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

#include "aqsis.h"
#include "renderer.h"

START_NAMESPACE( Aqsis )

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
  inline const TqInt nverts() const { return _nverts ; }
  /** accesses the number of triangles of the generated mesh */
  inline const TqInt ntrigs() const { return _ntrigs ; }
  /** accesses a specific vertex of the generated mesh */
  inline Vertex   * vert( const TqInt i ) const { if( i < 0  || i >= _nverts ) return ( Vertex *)NULL ; return _vertices  + i ; }
  /** accesses a specific triangle of the generated mesh */
  inline Triangle * trig( const TqInt i ) const { if( i < 0  || i >= _ntrigs ) return (Triangle*)NULL ; return _triangles + i ; }

  /** accesses the vertex buffer of the generated mesh */
  inline Vertex   *vertices () { return _vertices  ; }
  /** accesses the triangle buffer of the generated mesh */
  inline Triangle *triangles() { return _triangles ; }

  /**  accesses the width  of the grid */
  inline const TqInt size_x() const { return _size_x ; }
  /**  accesses the depth  of the grid */
  inline const TqInt size_y() const { return _size_y ; }
  /**  accesses the height of the grid */
  inline const TqInt size_z() const { return _size_z ; }

  /**
   * changes the size of the grid
   * \param size_x width  of the grid
   * \param size_y depth  of the grid
   * \param size_z height of the grid
   */
  inline void set_resolution( const TqInt size_x, const TqInt size_y, const TqInt size_z ) { _size_x = size_x ;  _size_y = size_y ;  _size_z = size_z ; }
  /**
   * selects wether the algorithm will use the enhanced topologically controlled lookup table or the original MarchingCubes
   * \param originalMC TqTrue for the original Marching Cubes
   */
  inline void set_method    ( const TqBool originalMC = TqFalse ) { _originalMC = originalMC ; }

  // Data access
  /**
   * accesses a specific cube of the grid
   * \param i abscisse of the cube
   * \param j ordinate of the cube
   * \param k height of the cube
   */
  inline const TqFloat get_data  ( const TqInt i, const TqInt j, const TqInt k ) const { return _data[ i + j*_size_x + k*_size_x*_size_y] ; }
  /**
   * sets a specific cube of the grid
   * \param val new value for the cube
   * \param i abscisse of the cube
   * \param j ordinate of the cube
   * \param k height of the cube
   */
  inline void  set_data  ( const TqFloat val, const TqInt i, const TqInt j, const TqInt k ) { _data[ i + j*_size_x + k*_size_x*_size_y] = val ; }

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
   * \param bin if TqTrue, the GTS will be written in binary mode
   */
  void write( const TqChar *fn, TqBool bin = TqFalse ) ;


//-----------------------------------------------------------------------------
// Algorithm
public :
  /** Main algorithm : must be called after init_all */
  void run() ;

protected :
  /** tesselates one cube */
  void process_cube ()             ;
  /** tests if the components of the tesselation of the cube should be connected by the interior of an ambiguous face */
  TqBool test_face    ( TqChar face ) ;
  /** tests if the components of the tesselation of the cube should be connected through the interior of the cube */
  TqBool test_interior( TqChar s )    ;


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
  inline TqInt   get_x_vert( const TqInt i, const TqInt j, const TqInt k ) const { return _x_verts[ i + j*_size_x + k*_size_x*_size_y] ; }
  /**
   * accesses the pre-computed vertex index on the lower longitudinal edge of a specific cube
   * \param i abscisse of the cube
   * \param j ordinate of the cube
   * \param k height of the cube
   */
  inline TqInt   get_y_vert( const TqInt i, const TqInt j, const TqInt k ) const { return _y_verts[ i + j*_size_x + k*_size_x*_size_y] ; }
  /**
   * accesses the pre-computed vertex index on the lower vertical edge of a specific cube
   * \param i abscisse of the cube
   * \param j ordinate of the cube
   * \param k height of the cube
   */
  inline TqInt   get_z_vert( const TqInt i, const TqInt j, const TqInt k ) const { return _z_verts[ i + j*_size_x + k*_size_x*_size_y] ; }

  /**
   * sets the pre-computed vertex index on the lower horizontal edge of a specific cube
   * \param val the index of the new vertex
   * \param i abscisse of the cube
   * \param j ordinate of the cube
   * \param k height of the cube
   */
  inline void  set_x_vert( const TqInt val, const TqInt i, const TqInt j, const TqInt k ) { _x_verts[ i + j*_size_x + k*_size_x*_size_y] = val ; }
  /**
   * sets the pre-computed vertex index on the lower longitudinal edge of a specific cube
   * \param val the index of the new vertex
   * \param i abscisse of the cube
   * \param j ordinate of the cube
   * \param k height of the cube
   */
  inline void  set_y_vert( const TqInt val, const TqInt i, const TqInt j, const TqInt k ) { _y_verts[ i + j*_size_x + k*_size_x*_size_y] = val ; }
  /**
   * sets the pre-computed vertex index on the lower vertical edge of a specific cube
   * \param val the index of the new vertex
   * \param i abscisse of the cube
   * \param j ordinate of the cube
   * \param k height of the cube
   */
  inline void  set_z_vert( const TqInt val, const TqInt i, const TqInt j, const TqInt k ) { _z_verts[ i + j*_size_x + k*_size_x*_size_y] = val ; }

  /** prints cube for debug */
  void print_cube() ;

//-----------------------------------------------------------------------------
// Elements
protected :
  TqInt       _originalMC ;   /**< selects wether the algorithm will use the enhanced topologically controlled lookup table or the original MarchingCubes */
  TqInt       _N[15]      ;   /**< counts the occurence of each case for debug */

  TqInt       _size_x     ;  /**< width  of the grid */
  TqInt       _size_y     ;  /**< depth  of the grid */
  TqInt       _size_z     ;  /**< height of the grid */
  TqFloat    *_data       ;  /**< implicit function values sampled on the grid */

  TqInt      *_x_verts    ;  /**< pre-computed vertex indices on the lower horizontal   edge of each cube */
  TqInt      *_y_verts    ;  /**< pre-computed vertex indices on the lower longitudinal edge of each cube */
  TqInt      *_z_verts    ;  /**< pre-computed vertex indices on the lower vertical     edge of each cube */

  TqInt       _nverts     ;  /**< number of allocated vertices  in the vertex   buffer */
  TqInt       _ntrigs     ;  /**< number of allocated triangles in the triangle buffer */
  TqInt       _Nverts     ;  /**< size of the vertex   buffer */
  TqInt       _Ntrigs     ;  /**< size of the triangle buffer */
  Vertex   *_vertices   ;  /**< vertex   buffer */
  Triangle *_triangles  ;  /**< triangle buffer */

  TqInt       _i          ;  /**< abscisse of the active cube */
  TqInt       _j          ;  /**< height of the active cube */
  TqInt       _k          ;  /**< ordinate of the active cube */

  TqFloat     _cube[8]    ;  /**< values of the implicit function on the active cube */
  TqUchar     _lut_entry  ;  /**< cube sign representation in [0..255] */
  TqUchar     _case       ;  /**< case of the active cube in [0..15] */
  TqUchar     _config     ;  /**< configuration of the active cube */
  TqUchar     _subconfig  ;  /**< subconfiguration of the active cube */
};
//_____________________________________________________________________________

//---------------------------------------------------------------------
END_NAMESPACE( Aqsis )

#endif // _MARCHINGCUBES_H_

