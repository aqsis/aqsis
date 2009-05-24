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

#ifndef _WIN32
#pragma implementation
#endif // AQSIS_SYSTEM_WIN32

#include <aqsis/aqsis.h>

#include <cstring>  // for memcpy, memset
#include <math.h>
#include <time.h>
#include <memory.h>
#include <float.h>
#include <stdio.h>

#include "renderer.h"
#include <aqsis/util/logging.h>

// Compute normals
#undef COMPUTE_NORMALS

#include "marchingcubes.h"
#include "lookuptable.h"

// step size of the arrays of vertices and triangles
#define ALLOC_SIZE 65536

namespace Aqsis {

//_____________________________________________________________________________
// print cube for debug
void MarchingCubes::print_cube()
{
	//Aqsis::log() << warning << i_cube[0] << " " <<  i_cube[1] << " " <<  i_cube[2] << " " <<  i_cube[3] << " " <<  i_cube[4] << " " <<  i_cube[5] << " " <<  i_cube[6] << " " <<  i_cube[7]) << std::endl;
}

//_____________________________________________________________________________
// Constructor
MarchingCubes::MarchingCubes( const TqInt x /*= -1*/, const TqInt y /*= -1*/, const TqInt z /*= -1*/ ) :
		//-----------------------------------------------------------------------------
		i_originalMC(false),
		i_size_x    (x),
		i_size_y    (y),
		i_size_z    (z),
		i_data      ((float*)NULL),
		i_x_verts   (( TqInt *)NULL),
		i_y_verts   (( TqInt *)NULL),
		i_z_verts   (( TqInt *)NULL),
		i_nverts    (0),
		i_ntrigs    (0),
		i_Nverts    (0),
		i_Ntrigs    (0),
		i_vertices  (( Vertex *)NULL),
		i_triangles ((Triangle*)NULL)
{}
//_____________________________________________________________________________



//_____________________________________________________________________________
// Destructor
MarchingCubes::~MarchingCubes()
//-----------------------------------------------------------------------------
{
	clean_all() ;
}
//_____________________________________________________________________________



//_____________________________________________________________________________
// main algorithm
void MarchingCubes::run()
//-----------------------------------------------------------------------------
{
/*
  	TqLong tick = clock();
	TqDouble perclocks = 1.0/(double)CLOCKS_PER_SEC;
*/

	compute_intersection_points( ) ;

	for( i_k = 0 ; i_k < i_size_z-1 ; i_k++ )
		for( i_j = 0 ; i_j < i_size_y-1 ; i_j++ )
			for( i_i = 0 ; i_i < i_size_x-1 ; i_i++ )
			{
				i_lut_entry = 0 ;
				for( TqInt p = 0 ; p < 8 ; ++p )
				{
					i_cube[p] = get_data( i_i+((p^(p>>1))&1), i_j+((p>>1)&1), i_k+((p>>2)&1) ) ;
					if( fabs( i_cube[p] ) < FLT_EPSILON )
						i_cube[p] = FLT_EPSILON ;
					if( i_cube[p] > 0 )
						i_lut_entry += 1 << p ;
				}
				/*
				    if( ( i_cube[0] = get_data( i_i , i_j , i_k ) ) > 0 ) i_lut_entry +=   1 ;
				    if( ( i_cube[1] = get_data(i_i+1, i_j , i_k ) ) > 0 ) i_lut_entry +=   2 ;
				    if( ( i_cube[2] = get_data(i_i+1,i_j+1, i_k ) ) > 0 ) i_lut_entry +=   4 ;
				    if( ( i_cube[3] = get_data( i_i ,i_j+1, i_k ) ) > 0 ) i_lut_entry +=   8 ;
				    if( ( i_cube[4] = get_data( i_i , i_j ,i_k+1) ) > 0 ) i_lut_entry +=  16 ;
				    if( ( i_cube[5] = get_data(i_i+1, i_j ,i_k+1) ) > 0 ) i_lut_entry +=  32 ;
				    if( ( i_cube[6] = get_data(i_i+1,i_j+1,i_k+1) ) > 0 ) i_lut_entry +=  64 ;
				    if( ( i_cube[7] = get_data(i_i ,i_j+1,i_k+1) ) > 0 ) i_lut_entry += 128 ;
				*/
				process_cube( ) ;
			}

/*
	  Aqsis::log() << info << "the cpu tooks " << perclocks * (TqDouble) (clock() - tick ) << " secs." << std::endl;

	  for( i_i = 0 ; i_i < 15 ; i_i++ )
	  {
	  	Aqsis::log() << info << i_N[i_i] << " cases " << i_i << std::endl;
	  }
*/
}
//_____________________________________________________________________________



//_____________________________________________________________________________
// init temporary structures (must set sizes before call)
void MarchingCubes::init_temps()
//-----------------------------------------------------------------------------
{
	TqLong howmany = i_size_x * i_size_y * i_size_z;
	i_data    = new TqFloat[ howmany ];
	i_x_verts = new TqInt  [ howmany ];
	i_y_verts = new TqInt  [ howmany ];
	i_z_verts = new TqInt  [ howmany ];

	while (!i_x_verts || !i_y_verts || !i_z_verts)
	{
		clean_temps();
		i_size_x /= 2;
		i_size_y /= 2;
		i_size_z /= 2;
		howmany = i_size_x * i_size_y * i_size_z;

		i_data    = new TqFloat[ howmany ];
		i_x_verts = new TqInt  [ howmany ];
		i_y_verts = new TqInt  [ howmany ];
		i_z_verts = new TqInt  [ howmany ];

	}
	memset( i_x_verts, -1, howmany * sizeof( TqInt ) ) ;
	memset( i_y_verts, -1, howmany * sizeof( TqInt ) ) ;
	memset( i_z_verts, -1, howmany * sizeof( TqInt ) ) ;

	memset( i_N, 0, 15 * sizeof(TqInt) ) ;
}
//_____________________________________________________________________________



//_____________________________________________________________________________
// init all structures (must set sizes before call)
void MarchingCubes::init_all ()
//-----------------------------------------------------------------------------
{
	init_temps() ;

	i_nverts = i_ntrigs = 0 ;
	i_Nverts = i_Ntrigs = ALLOC_SIZE ;
	i_vertices  = new Vertex  [i_Nverts] ;
	i_triangles = new Triangle[i_Ntrigs] ;
}
//_____________________________________________________________________________



//_____________________________________________________________________________
// clean temporary structures
void MarchingCubes::clean_temps()
//-----------------------------------------------------------------------------
{
	if (i_data)
		delete [] i_data;
	if (i_x_verts)
		delete [] i_x_verts;
	if (i_y_verts)
		delete [] i_y_verts;
	if (i_z_verts)
		delete [] i_z_verts;

	i_data     = (float*)NULL ;
	i_x_verts  = (int*)NULL ;
	i_y_verts  = (int*)NULL ;
	i_z_verts  = (int*)NULL ;
}
//_____________________________________________________________________________



//_____________________________________________________________________________
// clean all structures
void MarchingCubes::clean_all()
//-----------------------------------------------------------------------------
{
	clean_temps() ;
	delete [] i_vertices  ;
	delete [] i_triangles ;
	i_vertices  = (Vertex   *)NULL ;
	i_triangles = (Triangle *)NULL ;
	i_nverts = i_ntrigs = 0 ;
	i_Nverts = i_Ntrigs = 0 ;

	i_size_x = i_size_y = i_size_z = -1 ;
}
//_____________________________________________________________________________



//_____________________________________________________________________________
//_____________________________________________________________________________


//_____________________________________________________________________________
// Compute the intersection points
void MarchingCubes::compute_intersection_points( )
//-----------------------------------------------------------------------------
{
	for( i_k = 0 ; i_k < i_size_z ; i_k++ )
		for( i_j = 0 ; i_j < i_size_y ; i_j++ )
			for( i_i = 0 ; i_i < i_size_x ; i_i++ )
			{
				i_cube[0] = get_data( i_i, i_j, i_k ) ;
				if( i_i < i_size_x - 1 )
					i_cube[1] = get_data(i_i+1, i_j , i_k ) ;
				else
					i_cube[1] = i_cube[0] ;

				if( i_j < i_size_y - 1 )
					i_cube[3] = get_data( i_i ,i_j+1, i_k ) ;
				else
					i_cube[3] = i_cube[0] ;

				if( i_k < i_size_z - 1 )
					i_cube[4] = get_data( i_i , i_j ,i_k+1) ;
				else
					i_cube[4] = i_cube[0] ;

				if( fabs( i_cube[0] ) < FLT_EPSILON )
					i_cube[0] = FLT_EPSILON ;
				if( fabs( i_cube[1] ) < FLT_EPSILON )
					i_cube[1] = FLT_EPSILON ;
				if( fabs( i_cube[3] ) < FLT_EPSILON )
					i_cube[3] = FLT_EPSILON ;
				if( fabs( i_cube[4] ) < FLT_EPSILON )
					i_cube[4] = FLT_EPSILON ;

				if( i_cube[0] < 0 )
				{
					if( i_cube[1] > 0 )
						set_x_vert( add_x_vertex( ), i_i,i_j,i_k ) ;
					if( i_cube[3] > 0 )
						set_y_vert( add_y_vertex( ), i_i,i_j,i_k ) ;
					if( i_cube[4] > 0 )
						set_z_vert( add_z_vertex( ), i_i,i_j,i_k ) ;
				}
				else
				{
					if( i_cube[1] < 0 )
						set_x_vert( add_x_vertex( ), i_i,i_j,i_k ) ;
					if( i_cube[3] < 0 )
						set_y_vert( add_y_vertex( ), i_i,i_j,i_k ) ;
					if( i_cube[4] < 0 )
						set_z_vert( add_z_vertex( ), i_i,i_j,i_k ) ;
				}
			}
}
//_____________________________________________________________________________





//_____________________________________________________________________________
// Test a face
// if face>0 return true if the face contains a part of the surface
bool MarchingCubes::test_face( TqChar face )
//-----------------------------------------------------------------------------
{
	TqFloat A,B,C,D ;

	switch( face )
	{
			case -1 :
			case 1 :
			A = i_cube[0] ;
			B = i_cube[4] ;
			C = i_cube[5] ;
			D = i_cube[1] ;
			break ;
			case -2 :
			case 2 :
			A = i_cube[1] ;
			B = i_cube[5] ;
			C = i_cube[6] ;
			D = i_cube[2] ;
			break ;
			case -3 :
			case 3 :
			A = i_cube[2] ;
			B = i_cube[6] ;
			C = i_cube[7] ;
			D = i_cube[3] ;
			break ;
			case -4 :
			case 4 :
			A = i_cube[3] ;
			B = i_cube[7] ;
			C = i_cube[4] ;
			D = i_cube[0] ;
			break ;
			case -5 :
			case 5 :
			A = i_cube[0] ;
			B = i_cube[3] ;
			C = i_cube[2] ;
			D = i_cube[1] ;
			break ;
			case -6 :
			case 6 :
			A = i_cube[4] ;
			B = i_cube[7] ;
			C = i_cube[6] ;
			D = i_cube[5] ;
			break ;
			default :
			Aqsis::log() << warning << "Invalid face code " << face << std::endl;
			print_cube() ;
			A = B = C = D = 0 ;
	};

	return face * A * ( A*C - B*D ) >= 0  ;  // face and A invert signs
}
//_____________________________________________________________________________





//_____________________________________________________________________________
// Test the interior of a cube
// if s == 7, return true  if the interior is empty
// if s ==-7, return false if the interior is empty
bool MarchingCubes::test_interior( TqChar s )
//-----------------------------------------------------------------------------
{
	TqFloat t, At=0, Bt=0, Ct=0, Dt=0, a, b ;
	TqChar  test =  0 ;
	TqChar  edge = -1 ; // reference edge of the triangulation

	switch( i_case )
	{
			case  4 :
			case 10 :
			a = ( i_cube[4] - i_cube[0] ) * ( i_cube[6] - i_cube[2] ) - ( i_cube[7] - i_cube[3] ) * ( i_cube[5] - i_cube[1] ) ;
			b =  i_cube[2] * ( i_cube[4] - i_cube[0] ) + i_cube[0] * ( i_cube[6] - i_cube[2] )
			     - i_cube[1] * ( i_cube[7] - i_cube[3] ) - i_cube[3] * ( i_cube[5] - i_cube[1] ) ;
			t = - b / (2*a) ;
			if( t<0 || t>1 )
				return s>0 ;

			At = i_cube[0] + ( i_cube[4] - i_cube[0] ) * t ;
			Bt = i_cube[3] + ( i_cube[7] - i_cube[3] ) * t ;
			Ct = i_cube[2] + ( i_cube[6] - i_cube[2] ) * t ;
			Dt = i_cube[1] + ( i_cube[5] - i_cube[1] ) * t ;
			break ;

			case  6 :
			case  7 :
			case 12 :
			case 13 :
			switch( i_case )
			{
					case  6 :
					edge = test6 [i_config][2] ;
					break ;
					case  7 :
					edge = test7 [i_config][4] ;
					break ;
					case 12 :
					edge = test12[i_config][3] ;
					break ;
					case 13 :
					edge = tiling13_5_1[i_config][i_subconfig][0] ;
					break ;
			}
			switch( edge )
			{
					case  0 :
					t  = i_cube[0] / ( i_cube[0] - i_cube[1] ) ;
					At = 0 ;
					Bt = i_cube[3] + ( i_cube[2] - i_cube[3] ) * t ;
					Ct = i_cube[7] + ( i_cube[6] - i_cube[7] ) * t ;
					Dt = i_cube[4] + ( i_cube[5] - i_cube[4] ) * t ;
					break ;
					case  1 :
					t  = i_cube[1] / ( i_cube[1] - i_cube[2] ) ;
					At = 0 ;
					Bt = i_cube[0] + ( i_cube[3] - i_cube[0] ) * t ;
					Ct = i_cube[4] + ( i_cube[7] - i_cube[4] ) * t ;
					Dt = i_cube[5] + ( i_cube[6] - i_cube[5] ) * t ;
					break ;
					case  2 :
					t  = i_cube[2] / ( i_cube[2] - i_cube[3] ) ;
					At = 0 ;
					Bt = i_cube[1] + ( i_cube[0] - i_cube[1] ) * t ;
					Ct = i_cube[5] + ( i_cube[4] - i_cube[5] ) * t ;
					Dt = i_cube[6] + ( i_cube[7] - i_cube[6] ) * t ;
					break ;
					case  3 :
					t  = i_cube[3] / ( i_cube[3] - i_cube[0] ) ;
					At = 0 ;
					Bt = i_cube[2] + ( i_cube[1] - i_cube[2] ) * t ;
					Ct = i_cube[6] + ( i_cube[5] - i_cube[6] ) * t ;
					Dt = i_cube[7] + ( i_cube[4] - i_cube[7] ) * t ;
					break ;
					case  4 :
					t  = i_cube[4] / ( i_cube[4] - i_cube[5] ) ;
					At = 0 ;
					Bt = i_cube[7] + ( i_cube[6] - i_cube[7] ) * t ;
					Ct = i_cube[3] + ( i_cube[2] - i_cube[3] ) * t ;
					Dt = i_cube[0] + ( i_cube[1] - i_cube[0] ) * t ;
					break ;
					case  5 :
					t  = i_cube[5] / ( i_cube[5] - i_cube[6] ) ;
					At = 0 ;
					Bt = i_cube[4] + ( i_cube[7] - i_cube[4] ) * t ;
					Ct = i_cube[0] + ( i_cube[3] - i_cube[0] ) * t ;
					Dt = i_cube[1] + ( i_cube[2] - i_cube[1] ) * t ;
					break ;
					case  6 :
					t  = i_cube[6] / ( i_cube[6] - i_cube[7] ) ;
					At = 0 ;
					Bt = i_cube[5] + ( i_cube[4] - i_cube[5] ) * t ;
					Ct = i_cube[1] + ( i_cube[0] - i_cube[1] ) * t ;
					Dt = i_cube[2] + ( i_cube[3] - i_cube[2] ) * t ;
					break ;
					case  7 :
					t  = i_cube[7] / ( i_cube[7] - i_cube[4] ) ;
					At = 0 ;
					Bt = i_cube[6] + ( i_cube[5] - i_cube[6] ) * t ;
					Ct = i_cube[2] + ( i_cube[1] - i_cube[2] ) * t ;
					Dt = i_cube[3] + ( i_cube[0] - i_cube[3] ) * t ;
					break ;
					case  8 :
					t  = i_cube[0] / ( i_cube[0] - i_cube[4] ) ;
					At = 0 ;
					Bt = i_cube[3] + ( i_cube[7] - i_cube[3] ) * t ;
					Ct = i_cube[2] + ( i_cube[6] - i_cube[2] ) * t ;
					Dt = i_cube[1] + ( i_cube[5] - i_cube[1] ) * t ;
					break ;
					case  9 :
					t  = i_cube[1] / ( i_cube[1] - i_cube[5] ) ;
					At = 0 ;
					Bt = i_cube[0] + ( i_cube[4] - i_cube[0] ) * t ;
					Ct = i_cube[3] + ( i_cube[7] - i_cube[3] ) * t ;
					Dt = i_cube[2] + ( i_cube[6] - i_cube[2] ) * t ;
					break ;
					case 10 :
					t  = i_cube[2] / ( i_cube[2] - i_cube[6] ) ;
					At = 0 ;
					Bt = i_cube[1] + ( i_cube[5] - i_cube[1] ) * t ;
					Ct = i_cube[0] + ( i_cube[4] - i_cube[0] ) * t ;
					Dt = i_cube[3] + ( i_cube[7] - i_cube[3] ) * t ;
					break ;
					case 11 :
					t  = i_cube[3] / ( i_cube[3] - i_cube[7] ) ;
					At = 0 ;
					Bt = i_cube[2] + ( i_cube[6] - i_cube[2] ) * t ;
					Ct = i_cube[1] + ( i_cube[5] - i_cube[1] ) * t ;
					Dt = i_cube[0] + ( i_cube[4] - i_cube[0] ) * t ;
					break ;
					default :
					Aqsis::log() << warning << "Invalid edge " << edge << std::endl;
					print_cube() ;
					break ;
			}
			break ;

			default :
			Aqsis::log() << warning << "invalid ambiguous case " << i_case << std::endl;
			print_cube() ;
			break ;
	}

	if( At >= 0 )
		test ++ ;
	if( Bt >= 0 )
		test += 2 ;
	if( Ct >= 0 )
		test += 4 ;
	if( Dt >= 0 )
		test += 8 ;
	switch( test )
	{
			case  0 :
			return s>0 ;
			case  1 :
			return s>0 ;
			case  2 :
			return s>0 ;
			case  3 :
			return s>0 ;
			case  4 :
			return s>0 ;
			case  5 :
			if( At * Ct <  Bt * Dt )
				return s>0 ;
			break ;
			case  6 :
			return s>0 ;
			case  7 :
			return s<0 ;
			case  8 :
			return s>0 ;
			case  9 :
			return s>0 ;
			case 10 :
			if( At * Ct >= Bt * Dt )
				return s>0 ;
			break ;
			case 11 :
			return s<0 ;
			case 12 :
			return s>0 ;
			case 13 :
			return s<0 ;
			case 14 :
			return s<0 ;
			case 15 :
			return s<0 ;
	}

	return s<0 ;
}
//_____________________________________________________________________________




//_____________________________________________________________________________
// Process a unit cube
void MarchingCubes::process_cube( )
//-----------------------------------------------------------------------------
{
	if( i_originalMC )
	{
		TqChar nt = 0 ;
		while( casesClassic[i_lut_entry][3*nt] != -1 )
			nt++ ;
		add_triangle( casesClassic[i_lut_entry], nt ) ;
		return ;
	}

	TqInt   v12 = -1 ;
	i_case   = cases[i_lut_entry][0] ;
	i_config = cases[i_lut_entry][1] ;
	i_subconfig = 0 ;

	i_N[i_case]++ ;

	switch( i_case )
	{
			case  0 :
			break ;

			case  1 :
			add_triangle( tiling1[i_config], 1 ) ;
			break ;

			case  2 :
			add_triangle( tiling2[i_config], 2 ) ;
			break ;

			case  3 :
			if( test_face( test3[i_config]) )
				add_triangle( tiling3_2[i_config], 4 ) ; // 3.2
			else
				add_triangle( tiling3_1[i_config], 2 ) ; // 3.1
			break ;

			case  4 :
			if( test_interior( test4[i_config]) )
				add_triangle( tiling4_1[i_config], 2 ) ; // 4.1.1
			else
				add_triangle( tiling4_2[i_config], 6 ) ; // 4.1.2
			break ;

			case  5 :
			add_triangle( tiling5[i_config], 3 ) ;
			break ;

			case  6 :
			if( test_face( test6[i_config][0]) )
				add_triangle( tiling6_2[i_config], 5 ) ; // 6.2
			else
			{
				if( test_interior( test6[i_config][1]) )
					add_triangle( tiling6_1_1[i_config], 3 ) ; // 6.1.1
				else
					add_triangle( tiling6_1_2[i_config], 7 ) ; // 6.1.2
			}
			break ;

			case  7 :
			if( test_face( test7[i_config][0] ) )
				i_subconfig +=  1 ;
			if( test_face( test7[i_config][1] ) )
				i_subconfig +=  2 ;
			if( test_face( test7[i_config][2] ) )
				i_subconfig +=  4 ;
			switch( i_subconfig )
			{
					case 0 :
					add_triangle( tiling7_1[i_config], 3 ) ;
					break ;
					case 1 :
					add_triangle( tiling7_2[i_config][0], 5 ) ;
					break ;
					case 2 :
					add_triangle( tiling7_2[i_config][1], 5 ) ;
					break ;
					case 3 :
					v12 = add_c_vertex() ;
					add_triangle( tiling7_3[i_config][0], 9, v12 ) ;
					break ;
					case 4 :
					add_triangle( tiling7_2[i_config][2], 5 ) ;
					break ;
					case 5 :
					v12 = add_c_vertex() ;
					add_triangle( tiling7_3[i_config][1], 9, v12 ) ;
					break ;
					case 6 :
					v12 = add_c_vertex() ;
					add_triangle( tiling7_3[i_config][2], 9, v12 ) ;
					break ;
					case 7 :
					if( test_interior( test7[i_config][3]) )
						add_triangle( tiling7_4_2[i_config], 9 ) ;
					else
						add_triangle( tiling7_4_1[i_config], 5 ) ;
					break ;
			};
			break ;

			case  8 :
			add_triangle( tiling8[i_config], 2 ) ;
			break ;

			case  9 :
			add_triangle( tiling9[i_config], 4 ) ;
			break ;

			case 10 :
			if( test_face( test10[i_config][0]) )
			{
				if( test_face( test10[i_config][1]) )
					add_triangle( tiling10_1_1_[i_config], 4 ) ; // 10.1.1
				else
				{
					v12 = add_c_vertex() ;
					add_triangle( tiling10_2[i_config], 8, v12 ) ; // 10.2
				}
			}
			else
			{
				if( test_face( test10[i_config][1]) )
				{
					v12 = add_c_vertex() ;
					add_triangle( tiling10_2_[i_config], 8, v12 ) ; // 10.2
				}
				else
				{
					if( test_interior( test10[i_config][2]) )
						add_triangle( tiling10_1_1[i_config], 4 ) ; // 10.1.1
					else
						add_triangle( tiling10_1_2[i_config], 8 ) ; // 10.1.2
				}
			}
			break ;

			case 11 :
			add_triangle( tiling11[i_config], 4 ) ;
			break ;

			case 12 :
			if( test_face( test12[i_config][0]) )
			{
				if( test_face( test12[i_config][1]) )
					add_triangle( tiling12_1_1_[i_config], 4 ) ; // 12.1.1
				else
				{
					v12 = add_c_vertex() ;
					add_triangle( tiling12_2[i_config], 8, v12 ) ; // 12.2
				}
			}
			else
			{
				if( test_face( test12[i_config][1]) )
				{
					v12 = add_c_vertex() ;
					add_triangle( tiling12_2_[i_config], 8, v12 ) ; // 12.2
				}
				else
				{
					if( test_interior( test12[i_config][2]) )
						add_triangle( tiling12_1_1[i_config], 4 ) ; // 12.1.1
					else
						add_triangle( tiling12_1_2[i_config], 8 ) ; // 12.1.2
				}
			}
			break ;

			case 13 :
			if( test_face( test13[i_config][0] ) )
				i_subconfig +=  1 ;
			if( test_face( test13[i_config][1] ) )
				i_subconfig +=  2 ;
			if( test_face( test13[i_config][2] ) )
				i_subconfig +=  4 ;
			if( test_face( test13[i_config][3] ) )
				i_subconfig +=  8 ;
			if( test_face( test13[i_config][4] ) )
				i_subconfig += 16 ;
			if( test_face( test13[i_config][5] ) )
				i_subconfig += 32 ;
			switch( subconfig13[i_subconfig] )
			{
					case 0 :/* 13.1 */
					add_triangle( tiling13_1[i_config], 4 ) ;
					break ;

					case 1 :/* 13.2 */
					add_triangle( tiling13_2[i_config][0], 6 ) ;
					break ;
					case 2 :/* 13.2 */
					add_triangle( tiling13_2[i_config][1], 6 ) ;
					break ;
					case 3 :/* 13.2 */
					add_triangle( tiling13_2[i_config][2], 6 ) ;
					break ;
					case 4 :/* 13.2 */
					add_triangle( tiling13_2[i_config][3], 6 ) ;
					break ;
					case 5 :/* 13.2 */
					add_triangle( tiling13_2[i_config][4], 6 ) ;
					break ;
					case 6 :/* 13.2 */
					add_triangle( tiling13_2[i_config][5], 6 ) ;
					break ;

					case 7 :/* 13.3 */
					v12 = add_c_vertex() ;
					add_triangle( tiling13_3[i_config][0], 10, v12 ) ;
					break ;
					case 8 :/* 13.3 */
					v12 = add_c_vertex() ;
					add_triangle( tiling13_3[i_config][1], 10, v12 ) ;
					break ;
					case 9 :/* 13.3 */
					v12 = add_c_vertex() ;
					add_triangle( tiling13_3[i_config][2], 10, v12 ) ;
					break ;
					case 10 :/* 13.3 */
					v12 = add_c_vertex() ;
					add_triangle( tiling13_3[i_config][3], 10, v12 ) ;
					break ;
					case 11 :/* 13.3 */
					v12 = add_c_vertex() ;
					add_triangle( tiling13_3[i_config][4], 10, v12 ) ;
					break ;
					case 12 :/* 13.3 */
					v12 = add_c_vertex() ;
					add_triangle( tiling13_3[i_config][5], 10, v12 ) ;
					break ;
					case 13 :/* 13.3 */
					v12 = add_c_vertex() ;
					add_triangle( tiling13_3[i_config][6], 10, v12 ) ;
					break ;
					case 14 :/* 13.3 */
					v12 = add_c_vertex() ;
					add_triangle( tiling13_3[i_config][7], 10, v12 ) ;
					break ;
					case 15 :/* 13.3 */
					v12 = add_c_vertex() ;
					add_triangle( tiling13_3[i_config][8], 10, v12 ) ;
					break ;
					case 16 :/* 13.3 */
					v12 = add_c_vertex() ;
					add_triangle( tiling13_3[i_config][9], 10, v12 ) ;
					break ;
					case 17 :/* 13.3 */
					v12 = add_c_vertex() ;
					add_triangle( tiling13_3[i_config][10], 10, v12 ) ;
					break ;
					case 18 :/* 13.3 */
					v12 = add_c_vertex() ;
					add_triangle( tiling13_3[i_config][11], 10, v12 ) ;
					break ;

					case 19 :/* 13.4 */
					v12 = add_c_vertex() ;
					add_triangle( tiling13_4[i_config][0], 12, v12 ) ;
					break ;
					case 20 :/* 13.4 */
					v12 = add_c_vertex() ;
					add_triangle( tiling13_4[i_config][1], 12, v12 ) ;
					break ;
					case 21 :/* 13.4 */
					v12 = add_c_vertex() ;
					add_triangle( tiling13_4[i_config][2], 12, v12 ) ;
					break ;
					case 22 :/* 13.4 */
					v12 = add_c_vertex() ;
					add_triangle( tiling13_4[i_config][3], 12, v12 ) ;
					break ;

					case 23 :/* 13.5 */
					i_subconfig = 0 ;
					if( test_interior( test13[i_config][6] ) )
						add_triangle( tiling13_5_1[i_config][0], 6 ) ;
					else
						add_triangle( tiling13_5_2[i_config][0], 10 ) ;
					break ;
					case 24 :/* 13.5 */
					i_subconfig = 1 ;
					if( test_interior( test13[i_config][6] ) )
						add_triangle( tiling13_5_1[i_config][1], 6 ) ;
					else
						add_triangle( tiling13_5_2[i_config][1], 10 ) ;
					break ;
					case 25 :/* 13.5 */
					i_subconfig = 2 ;
					if( test_interior( test13[i_config][6] ) )
						add_triangle( tiling13_5_1[i_config][2], 6 ) ;
					else
						add_triangle( tiling13_5_2[i_config][2], 10 ) ;
					break ;
					case 26 :/* 13.5 */
					i_subconfig = 3 ;
					if( test_interior( test13[i_config][6] ) )
						add_triangle( tiling13_5_1[i_config][3], 6 ) ;
					else
						add_triangle( tiling13_5_2[i_config][3], 10 ) ;
					break ;

					case 27 :/* 13.3 */
					v12 = add_c_vertex() ;
					add_triangle( tiling13_3_[i_config][0], 10, v12 ) ;
					break ;
					case 28 :/* 13.3 */
					v12 = add_c_vertex() ;
					add_triangle( tiling13_3_[i_config][1], 10, v12 ) ;
					break ;
					case 29 :/* 13.3 */
					v12 = add_c_vertex() ;
					add_triangle( tiling13_3_[i_config][2], 10, v12 ) ;
					break ;
					case 30 :/* 13.3 */
					v12 = add_c_vertex() ;
					add_triangle( tiling13_3_[i_config][3], 10, v12 ) ;
					break ;
					case 31 :/* 13.3 */
					v12 = add_c_vertex() ;
					add_triangle( tiling13_3_[i_config][4], 10, v12 ) ;
					break ;
					case 32 :/* 13.3 */
					v12 = add_c_vertex() ;
					add_triangle( tiling13_3_[i_config][5], 10, v12 ) ;
					break ;
					case 33 :/* 13.3 */
					v12 = add_c_vertex() ;
					add_triangle( tiling13_3_[i_config][6], 10, v12 ) ;
					break ;
					case 34 :/* 13.3 */
					v12 = add_c_vertex() ;
					add_triangle( tiling13_3_[i_config][7], 10, v12 ) ;
					break ;
					case 35 :/* 13.3 */
					v12 = add_c_vertex() ;
					add_triangle( tiling13_3_[i_config][8], 10, v12 ) ;
					break ;
					case 36 :/* 13.3 */
					v12 = add_c_vertex() ;
					add_triangle( tiling13_3_[i_config][9], 10, v12 ) ;
					break ;
					case 37 :/* 13.3 */
					v12 = add_c_vertex() ;
					add_triangle( tiling13_3_[i_config][10], 10, v12 ) ;
					break ;
					case 38 :/* 13.3 */
					v12 = add_c_vertex() ;
					add_triangle( tiling13_3_[i_config][11], 10, v12 ) ;
					break ;

					case 39 :/* 13.2 */
					add_triangle( tiling13_2_[i_config][0], 6 ) ;
					break ;
					case 40 :/* 13.2 */
					add_triangle( tiling13_2_[i_config][1], 6 ) ;
					break ;
					case 41 :/* 13.2 */
					add_triangle( tiling13_2_[i_config][2], 6 ) ;
					break ;
					case 42 :/* 13.2 */
					add_triangle( tiling13_2_[i_config][3], 6 ) ;
					break ;
					case 43 :/* 13.2 */
					add_triangle( tiling13_2_[i_config][4], 6 ) ;
					break ;
					case 44 :/* 13.2 */
					add_triangle( tiling13_2_[i_config][5], 6 ) ;
					break ;

					case 45 :/* 13.1 */
					add_triangle( tiling13_1_[i_config], 4 ) ;
					break ;

					default :
					Aqsis::log() << warning << "Impossible case 13 ?" << std::endl;
					print_cube() ;
			}
			break ;

			case 14 :
			add_triangle( tiling14[i_config], 4 ) ;
			break ;
	};
}
//_____________________________________________________________________________



//_____________________________________________________________________________
// Adding triangles
void MarchingCubes::add_triangle( const TqInt* trig, TqChar n, TqInt v12 )
//-----------------------------------------------------------------------------
{
	TqInt    tv[3] ;

	for( register TqInt t = 0 ; t < 3*n ; t++ )
	{
		register TqInt t3 = t % 3;
		switch( trig[t] )
		{
				case  0 :
				tv[ t3 ] = get_x_vert( i_i , i_j , i_k ) ;
				break ;
				case  1 :
				tv[ t3 ] = get_y_vert(i_i+1, i_j , i_k ) ;
				break ;
				case  2 :
				tv[ t3 ] = get_x_vert( i_i ,i_j+1, i_k ) ;
				break ;
				case  3 :
				tv[ t3 ] = get_y_vert( i_i , i_j , i_k ) ;
				break ;
				case  4 :
				tv[ t3 ] = get_x_vert( i_i , i_j ,i_k+1) ;
				break ;
				case  5 :
				tv[ t3 ] = get_y_vert(i_i+1, i_j ,i_k+1) ;
				break ;
				case  6 :
				tv[ t3 ] = get_x_vert( i_i ,i_j+1,i_k+1) ;
				break ;
				case  7 :
				tv[ t3 ] = get_y_vert( i_i , i_j ,i_k+1) ;
				break ;
				case  8 :
				tv[ t3 ] = get_z_vert( i_i , i_j , i_k ) ;
				break ;
				case  9 :
				tv[ t3 ] = get_z_vert(i_i+1, i_j , i_k ) ;
				break ;
				case 10 :
				tv[ t3 ] = get_z_vert(i_i+1,i_j+1, i_k ) ;
				break ;
				case 11 :
				tv[ t3 ] = get_z_vert( i_i ,i_j+1, i_k ) ;
				break ;
				case 12 :
				tv[ t3 ] = v12 ;
				break ;
				default :
				break ;
		}

		if( tv[t3] == -1 )
		{
			Aqsis::log() << warning << "Invalid triangle " << i_ntrigs << std::endl;
			print_cube() ;
		}

		if( t3 == 2 )
		{
			if( i_ntrigs >= i_Ntrigs )
			{
				Triangle *temp = i_triangles ;
				i_triangles = new Triangle[ i_ntrigs + 1024] ;
				memcpy( i_triangles, temp, i_Ntrigs*sizeof(Triangle) ) ;
				delete[] temp ;
/*
				Aqsis::log() << warning << "allocated triangles " << i_Ntrigs << std::endl;
*/
				i_Ntrigs = i_ntrigs + 1024 ;
			}

			Triangle *T = i_triangles + i_ntrigs++ ;
			T->v1    = tv[0] ;
			T->v2    = tv[1] ;
			T->v3    = tv[2] ;
		}
	}
}
//_____________________________________________________________________________



//_____________________________________________________________________________
// Calculating gradient

float MarchingCubes::get_x_grad( const TqInt i, const TqInt j, const TqInt k ) const
//-----------------------------------------------------------------------------
{
	if( i > 0 )
	{
		if ( i < i_size_x - 1 )
			return ( get_data( i+1, j, k ) - get_data( i-1, j, k ) ) / 2 ;
		else
			return get_data( i, j, k ) - get_data( i-1, j, k ) ;
	}
	else
		return get_data( i+1, j, k ) - get_data( i, j, k ) ;
}
//-----------------------------------------------------------------------------

float MarchingCubes::get_y_grad( const TqInt i, const TqInt j, const TqInt k ) const
//-----------------------------------------------------------------------------
{
	if( j > 0 )
	{
		if ( j < i_size_y - 1 )
			return ( get_data( i, j+1, k ) - get_data( i, j-1, k ) ) / 2 ;
		else
			return get_data( i, j, k ) - get_data( i, j-1, k ) ;
	}
	else
		return get_data( i, j+1, k ) - get_data( i, j, k ) ;
}
//-----------------------------------------------------------------------------

float MarchingCubes::get_z_grad( const TqInt i, const TqInt j, const TqInt k ) const
//-----------------------------------------------------------------------------
{
	if( k > 0 )
	{
		if ( k < i_size_z - 1 )
			return ( get_data( i, j, k+1 ) - get_data( i, j, k-1 ) ) / 2 ;
		else
			return get_data( i, j, k ) - get_data( i, j, k-1 ) ;
	}
	else
		return get_data( i, j, k+1 ) - get_data( i, j, k ) ;
}
//_____________________________________________________________________________


//_____________________________________________________________________________
// Adding vertices

void MarchingCubes::test_vertex_addition()
{
	if( i_nverts >= i_Nverts )
	{
		Vertex *temp = i_vertices ;
		i_vertices = new Vertex[ i_nverts  + 1024] ;
		memcpy( i_vertices, temp, i_Nverts*sizeof(Vertex) ) ;
		delete[] temp ;
/*
		Aqsis::log() << warning << "allocated vertices " << i_Nverts << std::endl;
*/
		i_Nverts = i_nverts + 1024 ;
	}
}


TqInt MarchingCubes::add_x_vertex( )
//-----------------------------------------------------------------------------
{
	test_vertex_addition() ;
	Vertex *vert = i_vertices + i_nverts++ ;

	TqFloat   u = ( i_cube[0] ) / ( i_cube[0] - i_cube[1] ) ;

	vert->x      = (float)i_i+u;
	vert->y      = (float) i_j ;
	vert->z      = (float) i_k ;

#ifdef COMPUTE_NORMALS
	vert->nx = (1-u)*get_x_grad(i_i,i_j,i_k) + u*get_x_grad(i_i+1,i_j,i_k) ;
	vert->ny = (1-u)*get_y_grad(i_i,i_j,i_k) + u*get_y_grad(i_i+1,i_j,i_k) ;
	vert->nz = (1-u)*get_z_grad(i_i,i_j,i_k) + u*get_z_grad(i_i+1,i_j,i_k) ;

	u = (float) sqrt( vert->nx * vert->nx + vert->ny * vert->ny +vert->nz * vert->nz ) ;
	if( u > 0 )
	{
		vert->nx /= u ;
		vert->ny /= u ;
		vert->nz /= u ;
	}
#endif

	return i_nverts-1 ;
}
//-----------------------------------------------------------------------------

TqInt MarchingCubes::add_y_vertex( )
//-----------------------------------------------------------------------------
{
	test_vertex_addition() ;
	Vertex *vert = i_vertices + i_nverts++ ;

	TqFloat   u = ( i_cube[0] ) / ( i_cube[0] - i_cube[3] ) ;

	vert->x      = (float) i_i ;
	vert->y      = (float)i_j+u;
	vert->z      = (float) i_k ;

#ifdef COMPUTE_NORMALS
	vert->nx = (1-u)*get_x_grad(i_i,i_j,i_k) + u*get_x_grad(i_i,i_j+1,i_k) ;
	vert->ny = (1-u)*get_y_grad(i_i,i_j,i_k) + u*get_y_grad(i_i,i_j+1,i_k) ;
	vert->nz = (1-u)*get_z_grad(i_i,i_j,i_k) + u*get_z_grad(i_i,i_j+1,i_k) ;

	u = (float) sqrt( vert->nx * vert->nx + vert->ny * vert->ny +vert->nz * vert->nz ) ;
	if( u > 0 )
	{
		vert->nx /= u ;
		vert->ny /= u ;
		vert->nz /= u ;
	}
#endif

	return i_nverts-1 ;
}
//-----------------------------------------------------------------------------

TqInt MarchingCubes::add_z_vertex( )
//-----------------------------------------------------------------------------
{
	test_vertex_addition() ;
	Vertex *vert = i_vertices + i_nverts++ ;

	TqFloat   u = ( i_cube[0] ) / ( i_cube[0] - i_cube[4] ) ;

	vert->x      = (float) i_i ;
	vert->y      = (float) i_j ;
	vert->z      = (float)i_k+u;

#ifdef COMPUTE_NORMALS
	vert->nx = (1-u)*get_x_grad(i_i,i_j,i_k) + u*get_x_grad(i_i,i_j,i_k+1) ;
	vert->ny = (1-u)*get_y_grad(i_i,i_j,i_k) + u*get_y_grad(i_i,i_j,i_k+1) ;
	vert->nz = (1-u)*get_z_grad(i_i,i_j,i_k) + u*get_z_grad(i_i,i_j,i_k+1) ;

	u = (float) sqrt( vert->nx * vert->nx + vert->ny * vert->ny +vert->nz * vert->nz ) ;
	if( u > 0 )
	{
		vert->nx /= u ;
		vert->ny /= u ;
		vert->nz /= u ;
	}
#endif

	return i_nverts-1 ;
}


TqInt MarchingCubes::add_c_vertex( )
//-----------------------------------------------------------------------------
{
	test_vertex_addition() ;
	Vertex *vert = i_vertices + i_nverts++ ;

	TqFloat  u = 0 ;
	TqInt   vid ;

	vert->x = vert->y = vert->z =  0;
#ifdef COMPUTE_NORMALS
        vert->nx = vert->ny = vert->nz = 0 ;
#endif

	// Computes the average of the intersection points of the cube
	vid = get_x_vert( i_i , i_j , i_k ) ;
	if( vid != -1 )
	{
		++u ;
		const Vertex &v = i_vertices[vid] ;
		vert->x += v.x ;
		vert->y += v.y ;
		vert->z += v.z ;
#ifdef COMPUTE_NORMALS
		vert->nx += v.nx ;
		vert->ny += v.ny ;
		vert->nz += v.nz ;
#endif
	}
	vid = get_y_vert(i_i+1, i_j , i_k ) ;
	if( vid != -1 )
	{
		++u ;
		const Vertex &v = i_vertices[vid] ;
		vert->x += v.x ;
		vert->y += v.y ;
		vert->z += v.z ;
#ifdef COMPUTE_NORMALS
		vert->nx += v.nx ;
		vert->ny += v.ny ;
		vert->nz += v.nz ;
#endif
	}
	vid = get_x_vert( i_i ,i_j+1, i_k ) ;
	if( vid != -1 )
	{
		++u ;
		const Vertex &v = i_vertices[vid] ;
		vert->x += v.x ;
		vert->y += v.y ;
		vert->z += v.z ;
#ifdef COMPUTE_NORMALS
		vert->nx += v.nx ;
		vert->ny += v.ny ;
		vert->nz += v.nz ;
#endif
	}
	vid = get_y_vert( i_i , i_j , i_k ) ;
	if( vid != -1 )
	{
		++u ;
		const Vertex &v = i_vertices[vid] ;
		vert->x += v.x ;
		vert->y += v.y ;
		vert->z += v.z ;
#ifdef COMPUTE_NORMALS
		vert->nx += v.nx ;
		vert->ny += v.ny ;
		vert->nz += v.nz ;
#endif
	}
	vid = get_x_vert( i_i , i_j ,i_k+1) ;
	if( vid != -1 )
	{
		++u ;
		const Vertex &v = i_vertices[vid] ;
		vert->x += v.x ;
		vert->y += v.y ;
		vert->z += v.z ;
#ifdef COMPUTE_NORMALS
		vert->nx += v.nx ;
		vert->ny += v.ny ;
		vert->nz += v.nz ;
#endif
	}
	vid = get_y_vert(i_i+1, i_j ,i_k+1) ;
	if( vid != -1 )
	{
		++u ;
		const Vertex &v = i_vertices[vid] ;
		vert->x += v.x ;
		vert->y += v.y ;
		vert->z += v.z ;
#ifdef COMPUTE_NORMALS
		vert->nx += v.nx ;
		vert->ny += v.ny ;
		vert->nz += v.nz ;
#endif
	}
	vid = get_x_vert( i_i ,i_j+1,i_k+1) ;
	if( vid != -1 )
	{
		++u ;
		const Vertex &v = i_vertices[vid] ;
		vert->x += v.x ;
		vert->y += v.y ;
		vert->z += v.z ;
#ifdef COMPUTE_NORMALS
		vert->nx += v.nx ;
		vert->ny += v.ny ;
		vert->nz += v.nz ;
#endif
	}
	vid = get_y_vert( i_i , i_j ,i_k+1) ;
	if( vid != -1 )
	{
		++u ;
		const Vertex &v = i_vertices[vid] ;
		vert->x += v.x ;
		vert->y += v.y ;
		vert->z += v.z ;
#ifdef COMPUTE_NORMALS
		vert->nx += v.nx ;
		vert->ny += v.ny ;
		vert->nz += v.nz ;
#endif
	}
	vid = get_z_vert( i_i , i_j , i_k ) ;
	if( vid != -1 )
	{
		++u ;
		const Vertex &v = i_vertices[vid] ;
		vert->x += v.x ;
		vert->y += v.y ;
		vert->z += v.z ;
#ifdef COMPUTE_NORMALS
		vert->nx += v.nx ;
		vert->ny += v.ny ;
		vert->nz += v.nz ;
#endif
	}
	vid = get_z_vert(i_i+1, i_j , i_k ) ;
	if( vid != -1 )
	{
		++u ;
		const Vertex &v = i_vertices[vid] ;
		vert->x += v.x ;
		vert->y += v.y ;
		vert->z += v.z ;
#ifdef COMPUTE_NORMALS
		vert->nx += v.nx ;
		vert->ny += v.ny ;
		vert->nz += v.nz ;
#endif
	}
	vid = get_z_vert(i_i+1,i_j+1, i_k ) ;
	if( vid != -1 )
	{
		++u ;
		const Vertex &v = i_vertices[vid] ;
		vert->x += v.x ;
		vert->y += v.y ;
		vert->z += v.z ;
#ifdef COMPUTE_NORMALS
		vert->nx += v.nx ;
		vert->ny += v.ny ;
		vert->nz += v.nz ;
#endif
	}
	vid = get_z_vert( i_i ,i_j+1, i_k ) ;
	if( vid != -1 )
	{
		++u ;
		const Vertex &v = i_vertices[vid] ;
		vert->x += v.x ;
		vert->y += v.y ;
		vert->z += v.z ;
#ifdef COMPUTE_NORMALS
		vert->nx += v.nx ;
		vert->ny += v.ny ;
		vert->nz += v.nz ;
#endif
	}

	vert->x  /= u ;
	vert->y  /= u ;
	vert->z  /= u ;

#ifdef COMPUTE_NORMALS
	u = (float) sqrt( vert->nx * vert->nx + vert->ny * vert->ny +vert->nz * vert->nz ) ;
	if( u > 0 )
	{
		vert->nx /= u ;
		vert->ny /= u ;
		vert->nz /= u ;
	}
#endif

	return i_nverts-1 ;
}
//_____________________________________________________________________________



//_____________________________________________________________________________
//_____________________________________________________________________________




void MarchingCubes::write(const TqChar *fn, bool bin )
//-----------------------------------------------------------------------------
{
	FILE       *fp = fopen( fn, "w" );
	fprintf(fp, "%d %d\n", i_nverts, i_ntrigs);

	TqInt          i ;

	for ( i = 0; i < i_nverts; i++ )
		fprintf(fp, "%f %f %f\n", i_vertices[i].x, i_vertices[i].y, i_vertices[i].z);

	for ( i = 0; i < i_ntrigs; i++ )
	{
		fprintf(fp, "%d %d %d \n", i_triangles[i].v1, i_triangles[i].v2, i_triangles[i].v3);
	}

	fclose( fp ) ;
}
//_____________________________________________________________________________


//---------------------------------------------------------------------
} // namespace Aqsis
