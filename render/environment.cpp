// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
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
		\brief Implements latlong/cubic environment handling.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	"aqsis.h"

#include	<math.h>
#include	<iostream>
#include	<fstream>

#include	"texturemap.h"
#include	"random.h"
#include	"version.h"
#include	"renderer.h"

#ifndef		AQSIS_SYSTEM_WIN32
#include	"unistd.h"
#endif

START_NAMESPACE( Aqsis )


// Local Constants

#define	max_no	30

// Face and edge numberings. see fig 6.12 page 192 [watt]
#define	pz	1
#define	px	2
#define	py	4
#define	nx	8
#define	ny	16
#define	nz	32

#define	edge01	3
#define	edge02	5
#define	edge03	9
#define	edge04	17
#define	edge12	6
#define	edge23	12
#define	edge34	24
#define	edge41	18
#define	edge51	34
#define	edge52	36
#define	edge53	40
#define	edge54	48

// Local variables

static CqVector3D	cube[ max_no ];	// Stores the projection of the reflected beam onto the cube.
static TqInt	cube_no; 		// Stores the number of points making up the projection.
static TqFloat	uv[ max_no ][ 2 ];	// Stores the values of this projection for a given face.

// Forward definition for local functions
static void get_face_intersection( CqVector3D *normal, CqVector3D *pt, TqInt* face );
static void get_edge_intersection( CqVector3D* n1, CqVector3D* n2, TqInt edge, CqVector3D* pt );
static void project( TqInt face );


//----------------------------------------------------------------------
/** Check if a texture map exists in the cache, return a pointer to it if so, else
 * load it if possible..
 */

CqTextureMap* CqTextureMap::GetLatLongMap( const CqString& strName )
{
	static int size = -1;
	static CqTextureMap *previous = NULL;
	QGetRenderContext() ->Stats().IncTextureMisses( 2 );

	if ( size == static_cast<int>( m_TextureMap_Cache.size() ) )
		if ( previous && ( previous->m_strName == strName ) )
		{
			QGetRenderContext() ->Stats().IncTextureHits( 0, 2 );
			return previous;
		}



	// First search the texture map cache
	for ( std::vector<CqTextureMap*>::iterator i = m_TextureMap_Cache.begin(); i != m_TextureMap_Cache.end(); i++ )
	{
		if ( ( *i ) ->m_strName == strName )
		{
			if ( ( *i ) ->Type() == MapType_LatLong )
			{
				previous = *i;
				size = m_TextureMap_Cache.size();
				QGetRenderContext() ->Stats().IncTextureHits( 1, 2 );
				return ( *i );
			}
			else
			{
				return ( NULL );
			}
		}
	}
	// If we got here, it doesn't exist yet, so we must create and load it.
	CqTextureMap* pNew = new CqLatLongMap( strName );
	m_TextureMap_Cache.push_back( pNew );
	pNew->Open();

	TqPchar ptexfmt;

	// Invalid only if this is not a LatLong Env. map file
	if ( pNew->m_pImage == 0 ||
	        TIFFGetField( pNew->m_pImage, TIFFTAG_PIXAR_TEXTUREFORMAT, &ptexfmt ) != 1 ||
	        strcmp( ptexfmt, LATLONG_HEADER ) != 0 )
	{
		//CqString strError( strName );
		//strError += " not an environment map, use RiMakeLatLongEnvironment";
		//CqBasicError( 0, Severity_Normal, strError.c_str() );
		QGetRenderContextI() ->Logger() ->error( "Map \"%s\" is not an environment map, use RiMakeLatLongEnvironment", strName.c_str() );
		
		pNew->SetInvalid();
	}

	previous = pNew;
	size = m_TextureMap_Cache.size();
	return ( pNew );
}

//----------------------------------------------------------------------
/** Check if a texture map exists in the cache, return a pointer to it if so, else
 * load it if possible..
 */

CqTextureMap* CqTextureMap::GetEnvironmentMap( const CqString& strName )
{
	static int size = -1;
	static CqTextureMap *previous = NULL;

	QGetRenderContext() ->Stats().IncTextureMisses( 1 );

	/* look if the last item return by this function was ok */
	if ( size == static_cast<int>( m_TextureMap_Cache.size() ) )
		if ( ( previous ) && ( previous->m_strName == strName ) )
		{
			QGetRenderContext() ->Stats().IncTextureHits( 0, 1 );
			return previous;
		}



	// First search the texture map cache
	for ( std::vector<CqTextureMap*>::iterator i = m_TextureMap_Cache.begin(); i != m_TextureMap_Cache.end(); i++ )
	{
		if ( ( *i ) ->m_strName == strName )
		{
			if ( ( *i ) ->Type() == MapType_Environment )
			{
				previous = *i;
				size = m_TextureMap_Cache.size();
				QGetRenderContext() ->Stats().IncTextureHits( 1, 1 );
				return ( *i );
			}
			else
			{
				return ( NULL );
			}
		}
	}
	// If we got here, it doesn't exist yet, so we must create and load it.
	CqTextureMap* pNew = new CqEnvironmentMap( strName );
	m_TextureMap_Cache.push_back( pNew );
	pNew->Open();

	TqPchar ptexfmt = 0;

	// Invalid if the m_pImage is not there or it is not cube or latlong env. map file
	if ( pNew->m_pImage == 0 ||
	        TIFFGetField( pNew->m_pImage, TIFFTAG_PIXAR_TEXTUREFORMAT, &ptexfmt ) != 1 ||
	        ( strcmp( ptexfmt, CUBEENVMAP_HEADER ) != 0 ) && ( strcmp( ptexfmt, LATLONG_HEADER ) != 0 ) )
	{
		//CqString strError( strName );
		//strError += " not an environment map, use RiMakeCubeFaceEnvironment";
		//CqBasicError( 0, Severity_Normal, strError.c_str() );
		QGetRenderContextI() ->Logger() ->error( "Map \"%s\" is not an environment map, use RiMakeCubeFaceEnvironment", strName.c_str() );
		pNew->SetInvalid();
		delete pNew;
		pNew = NULL;

	}

	// remove from the list a LatLong env. map since in shadeops.cpp we will cope with it.
	if ( ptexfmt && strcmp( ptexfmt, LATLONG_HEADER ) == 0 )
	{
		pNew->SetInvalid();
		delete pNew;
		pNew = NULL;
	}

	previous = pNew;
	size = m_TextureMap_Cache.size();
	return ( pNew );
}

//----------------------------------------------------------------------
/** Retrieve a color sample from the environment map using R as the reflection vector.
 * Filtering is done using swidth, twidth and nsamples.
 */

void CqEnvironmentMap::SampleMap( CqVector3D& R1, CqVector3D& swidth, CqVector3D& twidth,
                                  std::valarray<TqFloat>& val, std::map<std::string, IqShaderData*>& paramMap, TqInt index, TqFloat* average_depth )
{
	// Check the memory and make sure we don't abuse it
	CriticalMeasure();

	if ( m_pImage != 0 )
	{
		if ( Type() != MapType_LatLong )
		{
			CqVector3D	R2, R3, R4;
			R2 = R1 + swidth;
			R3 = R1 + twidth;
			R4 = R1 + swidth + twidth;

			SampleMap( R1, R2, R3, R4, val, paramMap );
		}
		else if ( Type() == MapType_LatLong )
		{
			CqVector3D V = R1;
			V.Unit();
			TqFloat ss1, tt1;
			TqFloat sswidth = swidth.Magnitude();
			TqFloat stwidth = twidth.Magnitude();

			ss1 = atan2( V.y(), V.x() ) / ( 2.0 * RI_PI );  /* -.5 -> .5 */
			ss1 = ss1 + 0.5; /* remaps to 0 -> 1 */
			tt1 = acos( -V.z() ) / RI_PI;

			CqTextureMap::SampleMap( ss1, tt1, sswidth, stwidth, val, paramMap );
		}
	}
}


//----------------------------------------------------------------------
/** Retrieve a sample from the environment map using R as the reflection vector.
 */

void CqEnvironmentMap::SampleMap( CqVector3D& R1, CqVector3D& R2, CqVector3D& R3, CqVector3D& R4,
                                  std::valarray<TqFloat>& val, std::map<std::string, IqShaderData*>& paramMap, TqInt index, TqFloat* average_depth )
{
	if ( m_pImage != 0 )
	{

		CqVector3D	last_R, R, pt;
		TqFloat	texture_area, total_area;
		TqInt	i, j;
		TqInt	projection_bit;		// Stores all the faces the reflected beam projects onto.
		TqInt	edge, last_face, current_face, project_face;

		CqVector3D	vertex_list[ 4 ] =
		    {
		        R1,
		        R2,
		        R3,
		        R4
		    };

		val.resize( m_SamplesPerPixel );

		vertex_list[ 0 ].Unit();
		vertex_list[ 1 ].Unit();
		vertex_list[ 2 ].Unit();
		vertex_list[ 3 ].Unit();

		cube_no = 0;
		projection_bit = 0;
		total_area = 0.0;

		// Find intersection the reflected vector from the last vertex in the list makes with the cube.
		last_R = vertex_list[ 3 ];

		get_face_intersection( &last_R, &cube[ cube_no ], &last_face );
		cube_no++;
		projection_bit |= last_face;

		for ( i = 0; i < 4; i++ )
		{
			R = vertex_list[ i ];
			get_face_intersection( &R, &pt, &current_face );

			// If the last reflected ray intersected a face different from the current
			// ray, we must find the intersection the beam makes with the corresponding edge.
			if ( current_face != last_face )
			{
				edge = current_face | last_face;
				get_edge_intersection( &last_R, &R, edge, &cube[ cube_no ] );
				cube_no++;
				projection_bit |= current_face;
			}
			cube[ cube_no ] = pt;
			cube_no++;
			last_face = current_face;
			last_R = R;
		}

		std::valarray<TqFloat>	run_val;
		run_val.resize( m_SamplesPerPixel );
		run_val = 0.0f;
		val = 0.0f;

		TqFloat facewidth = 1.0f / 3.0f;
		TqFloat faceheight = 1.0f * 0.5f;

		for ( i = 0, project_face = 1; i < 6; i++, project_face += project_face )
		{
			if ( project_face & projection_bit )
			{
				// Get the projection in UVSpace on the project_face
				project( project_face );
				TqFloat s1, t1, s2, t2;
				s1 = s2 = uv[ 0 ][ 0 ];
				t1 = t2 = uv[ 0 ][ 1 ];
				texture_area = 0.0;

				for ( j = 1; j < cube_no; j++ )
				{
					if ( uv[ j ][ 0 ] < s1 ) s1 = uv[ j ][ 0 ];
					if ( uv[ j ][ 1 ] < t1 ) t1 = uv[ j ][ 1 ];
					if ( uv[ j ][ 0 ] > s2 ) s2 = uv[ j ][ 0 ];
					if ( uv[ j ][ 1 ] > t2 ) t2 = uv[ j ][ 1 ];
				}
				texture_area = ( s2 - s1 ) * ( t2 - t1 );
				if ( texture_area <= 0.0 ) texture_area = 1.0f;

				// Adjust for the appropriate section of the cube map.
				// Clamp to the cubemap face, this is OK, as the wrap over a boundary is handled
				// by the preceeding code which gets multiple samples for the cube if a sample crosses
				// edges.
				s1 = CLAMP( s1, 0.0f, 1.0f );
				s2 = CLAMP( s2, 0.0f, 1.0f );
				t1 = CLAMP( t1, 0.0f, 1.0f );
				t2 = CLAMP( t2, 0.0f, 1.0f );
				s1 = ( s1 * facewidth ) + ( facewidth * ( i % 3 ) );
				s2 = ( s2 * facewidth ) + ( facewidth * ( i % 3 ) );
				t1 = ( t1 * faceheight ) + ( faceheight * ( i / 3 ) );
				t2 = ( t2 * faceheight ) + ( faceheight * ( i / 3 ) );

				GetSample( s1, t1, s2, t2, run_val );

				// Add the color contribution weighted by the area
				val += run_val * texture_area;
				total_area += texture_area;
			}
		}
		// Normalize the weightings
		val /= total_area;
	}

}

// Local Functions/Routines


static void get_face_intersection( CqVector3D *normal, CqVector3D *pt, TqInt* face )
{
	CqVector3D n = *normal;
	TqFloat t;

	// Test nz direction
	if ( n.z() < 0 )        	// Test intersection with nz
	{
		t = -0.5 / n.z();
		pt->x( n.x() * t );
		pt->y( n.y() * t );
		pt->z( n.z() * t );
		if ( fabs( pt->x() ) < 0.5 && fabs( pt->y() ) < 0.5 )
		{
			*face = nz;
			return ;
		}
	}
	else if ( n.z() > 0 )        	// Test intersection with pz
	{
		t = 0.5 / n.z();
		pt->x( n.x() * t );
		pt->y( n.y() * t );
		pt->z( n.z() * t );
		if ( fabs( pt->x() ) < 0.5 && fabs( pt->y() ) < 0.5 )
		{
			*face = pz;
			return ;
		}
	}


	// Test ny direction
	if ( n.y() < 0 )        	// Test intersection with ny
	{
		t = -0.5 / n.y();
		pt->x( n.x() * t );
		pt->y( n.y() * t );
		pt->z( n.z() * t );
		if ( fabs( pt->x() ) < 0.5 && fabs( pt->z() ) < 0.5 )
		{
			*face = ny;
			return ;
		}
	}
	else if ( n.y() > 0 )        	// Test intersection with py
	{
		t = 0.5 / n.y();
		pt->x( n.x() * t );
		pt->y( n.y() * t );
		pt->z( n.z() * t );
		if ( fabs( pt->x() ) < 0.5 && fabs( pt->z() ) < 0.5 )
		{
			*face = py;
			return ;
		}
	}

	// Test nx direction
	if ( n.x() < 0 )        	// Test intersection with nx
	{
		t = -0.5 / n.x();
		pt->x( n.x() * t );
		pt->y( n.y() * t );
		pt->z( n.z() * t );
		if ( fabs( pt->y() ) < 0.5 && fabs( pt->z() ) < 0.5 )
		{
			*face = nx;
			return ;
		}
	}
	else if ( n.x() > 0 )        	// Test intersection with px
	{
		t = 0.5 / n.x();
		pt->x( n.x() * t );
		pt->y( n.y() * t );
		pt->z( n.z() * t );
		if ( fabs( pt->y() ) < 0.5 && fabs( pt->z() ) < 0.5 )
		{
			*face = px;
			return ;
		}
	}
}

static void get_edge_intersection( CqVector3D* n1, CqVector3D* n2, TqInt edge, CqVector3D* pt )
{
	TqFloat a, b, c;
	TqFloat x0, y0, z0, f, g, h;
	TqFloat denom, t;

	// Get plane eqn: ax+by+cz=0 from two normals n1 and n2
	a = n1->y() * n2->z() - n1->z() * n2->y();
	b = n1->z() * n2->x() - n1->x() * n2->z();
	c = n1->x() * n2->y() - n1->y() * n2->x();

	// Set up line equation of edge.
	x0 = y0 = z0 = 0.0;
	f = g = h = 0.0;
	switch ( edge )
	{
			case edge01: x0 = z0 = 0.5; g = 1; break;
			case edge02: y0 = z0 = 0.5; f = 1; break;
			case edge03: x0 = -0.5; z0 = 0.5; g = 1; break;
			case edge04: y0 = -0.5; z0 = 0.5; f = 1; break;
			case edge12: x0 = y0 = 0.5; h = 1; break;
			case edge23: x0 = -0.5; y0 = 0.5; h = 1; break;
			case edge34: x0 = y0 = -0.5; h = 1; break;
			case edge41: x0 = 0.5; y0 = -0.5; h = 1; break;
			case edge51: x0 = 0.5; z0 = -0.5; g = 1; break;
			case edge52: y0 = 0.5; z0 = -0.5; f = 1; break;
			case edge53: x0 = z0 = -0.5; g = 1; break;
			case edge54: y0 = z0 = -0.5; f = 1; break;
	}

	// Return the intersection of the plane and edge
	denom = a * f + b * g + c * h;
	t = -( a * x0 + b * y0 + c * z0 ) / denom;
	pt->x( x0 + f * t );
	pt->y( y0 + g * t );
	pt->z( z0 + h * t );
}


static void project( TqInt face )
{
	TqInt i;
	switch ( face )
	{
			case pz:
			for ( i = 0; i < cube_no; i++ )
			{
				uv[ i ][ 0 ] = cube[ i ].x() + 0.5;
				uv[ i ][ 1 ] = -cube[ i ].y() + 0.5;
			}
			break;

			case px:
			for ( i = 0; i < cube_no; i++ )
			{
				uv[ i ][ 0 ] = -cube[ i ].z() + 0.5;
				uv[ i ][ 1 ] = -cube[ i ].y() + 0.5;
			}
			break;

			case py:
			for ( i = 0; i < cube_no; i++ )
			{
				uv[ i ][ 0 ] = cube[ i ].x() + 0.5;
				uv[ i ][ 1 ] = cube[ i ].z() + 0.5;
			}
			break;

			case nx:
			for ( i = 0; i < cube_no; i++ )
			{
				uv[ i ][ 0 ] = cube[ i ].z() + 0.5;
				uv[ i ][ 1 ] = -cube[ i ].y() + 0.5;
			}
			break;

			case ny:
			for ( i = 0; i < cube_no; i++ )
			{
				uv[ i ][ 0 ] = cube[ i ].x() + 0.5;
				uv[ i ][ 1 ] = -cube[ i ].z() + 0.5;
			}
			break;

			case nz:
			for ( i = 0; i < cube_no; i++ )
			{
				uv[ i ][ 0 ] = -cube[ i ].x() + 0.5;
				uv[ i ][ 1 ] = -cube[ i ].y() + 0.5;
			}
			break;
	}
}




END_NAMESPACE( Aqsis )

