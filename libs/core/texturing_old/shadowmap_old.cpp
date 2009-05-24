// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
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
		\brief Implements shadowmap handling.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	<aqsis/aqsis.h>

#include	<cstring>
#include	<iostream>
#include	<fstream>
#include	<boost/shared_array.hpp>

#include	"texturemap_old.h"
#include	<aqsis/math/random.h>
#include	<aqsis/version.h>
#include	"renderer.h"
#include	<aqsis/util/logging.h>
#include	"stats.h"

#ifndef		AQSIS_SYSTEM_WIN32
#include	"unistd.h"
#endif

namespace Aqsis {

// Local Constants

#define	MinSize		3.0f
#define	NumSamples	16
#define	MinSamples	3

// Local Variables

static TqInt    m_rand_index = -1;
static TqFloat  m_aRand_no[ 1024 ];
static CqRandom random( 42 );

//---------------------------------------------------------------------
/** Constructor.
 */

CqShadowMapOld::CqShadowMapOld( const CqString& strName ) :
		CqTextureMapOld( strName )
{

	// Initialise the random table the first time it is needed.
	if( m_rand_index < 0 )
	{
		TqInt i;
		for ( i = 0; i < 1024; i++ )
			m_aRand_no[ i ] = random.RandomFloat(2.0);
		m_rand_index = 0;
	}
	for (TqInt k=0; k < 256; k++)
		m_apLast[k] = NULL;
	m_LastPoint = CqVector2D(-1, -1);
	m_Val = 0.0f;
	m_Depth = 0.0f;
	m_Average = 0.0f;
}

//---------------------------------------------------------------------
/** Return an index based on the spatial layer 32x32 in case of regular
 *  shadowmap to make the search for GetBuffer() more efficient.
 */
TqInt CqShadowMapOld::PseudoMipMaps( TqUlong s, TqInt index )
{
	TqInt idx = index;
	if (NumPages() == 1)
	{
		idx = s / 32;
	}
	return idx;
}

//---------------------------------------------------------------------
/** Allocate the memory required by the depthmap.
 */

void CqShadowMapOld::AllocateMap( TqInt XRes, TqInt YRes )
{
	std::list<CqTextureMapBuffer*>::iterator s;
	for ( s = m_apFlat.begin(); s != m_apFlat.end(); s++ )
		delete( *s );

	m_XRes = XRes;
	m_YRes = YRes;
	m_apFlat.push_back( CreateBuffer( 0, 0, m_XRes, m_YRes, 1 ) );
}



//----------------------------------------------------------------------
/** Check if a texture map exists in the cache, return a pointer to it if so, else
 * load it if possible..
 */

IqTextureMapOld* CqTextureMapOld::GetShadowMap( const CqString& strName )
{
	QGetRenderContext() ->Stats().IncTextureMisses( 3 );

	//TqUlong hash = CqString::hash(strName.c_str());

	// First search the texture map cache
	for ( std::vector<CqTextureMapOld*>::iterator i = m_TextureMap_Cache.begin(); i != m_TextureMap_Cache.end(); i++ )
	{
		if ( (*i)->getName() == strName )
		{
			if ( ( *i ) ->Type() == MapType_Shadow )
			{
				QGetRenderContext() ->Stats().IncTextureHits( 1, 3 );
				return ( *i );
			} else
			{
				return NULL;
			}
		}
	}

	QGetRenderContext() ->Stats().IncTextureHits( 0, 3 );

	// If we got here, it doesn't exist yet, so we must create and load it.
	CqShadowMapOld* pNew = new CqShadowMapOld( strName );
	m_TextureMap_Cache.push_back( pNew );
	pNew->Open();

	TqPchar ptexfmt;
	if ( pNew->m_pImage == 0 ||
	        TIFFGetField( pNew->m_pImage, TIFFTAG_PIXAR_TEXTUREFORMAT, &ptexfmt ) != 1 ||
	        strcmp( ptexfmt, SHADOWMAP_HEADER ) != 0 )
	{
		static bool done = false;
		if (!done)
		{
			Aqsis::log() << error << "Map \"" << strName.c_str() << "\" is not a valid shadow map, use RiMakeShadow" << std::endl;
			done = true;
		}
		pNew->SetInvalid();
	}
   	else 
   	{
		pNew->ReadMatrices();
   	}
	return ( pNew );
}


//----------------------------------------------------------------------
/** Load the shadowmap data.
 */

void CqShadowMapOld::LoadZFile()
{
	// Load the shadowmap from a binary file.
	if ( m_strName != "" )
	{
		std::ifstream file( m_strName.c_str(), std::ios::in | std::ios::binary );

		if ( file != NULL )
		{
			// Save a file type and version marker
			TqPchar origHeader = tokenCast(ZFILE_HEADER);
			TqInt headerLength = strlen( ZFILE_HEADER );
			boost::shared_array<TqChar> strHeader( new TqChar[ headerLength ] );
			file.read( strHeader.get(), headerLength );
			// Check validity of shadow map.
			if ( strncmp( strHeader.get(), origHeader, headerLength ) != 0 )
			{
				Aqsis::log() << error << "Invalid shadow map format \"" << m_strName.c_str() << "\"" << " : \"" << strHeader.get() << "\"[" << origHeader << "]"<< std::endl;
				return ;
			}

			// Save the xres and yres.
			file.read( reinterpret_cast<TqPchar >( &m_XRes ), sizeof( m_XRes ) );
			file.read( reinterpret_cast<TqPchar >( &m_YRes ), sizeof( m_YRes ) );

			// Save the transformation matrices.
			m_WorldToScreenMatrices.resize(1);
			m_WorldToCameraMatrices.resize(1);
			m_MinZ.resize(1);
			m_MinZ[0] = RI_FLOATMAX;
			m_NumberOfMaps = 0;
			file.read( reinterpret_cast<TqPchar>( matWorldToCamera()[ 0 ] ), sizeof( matWorldToCamera()[ 0 ][ 0 ] ) * 4 );
			file.read( reinterpret_cast<TqPchar>( matWorldToCamera()[ 1 ] ), sizeof( matWorldToCamera()[ 0 ][ 0 ] ) * 4 );
			file.read( reinterpret_cast<TqPchar>( matWorldToCamera()[ 2 ] ), sizeof( matWorldToCamera()[ 0 ][ 0 ] ) * 4 );
			file.read( reinterpret_cast<TqPchar>( matWorldToCamera()[ 3 ] ), sizeof( matWorldToCamera()[ 0 ][ 0 ] ) * 4 );

			file.read( reinterpret_cast<TqPchar>( matWorldToScreen()[ 0 ] ), sizeof( matWorldToScreen()[ 0 ][ 0 ] ) * 4 );
			file.read( reinterpret_cast<TqPchar>( matWorldToScreen()[ 1 ] ), sizeof( matWorldToScreen()[ 0 ][ 0 ] ) * 4 );
			file.read( reinterpret_cast<TqPchar>( matWorldToScreen()[ 2 ] ), sizeof( matWorldToScreen()[ 0 ][ 0 ] ) * 4 );
			file.read( reinterpret_cast<TqPchar>( matWorldToScreen()[ 3 ] ), sizeof( matWorldToScreen()[ 0 ][ 0 ] ) * 4 );

			// Now output the depth values
			AllocateMap( m_XRes, m_YRes );
			file.read( reinterpret_cast<TqPchar>( m_apFlat.front() ->pVoidBufferData() ), sizeof( TqFloat ) * ( m_XRes * m_YRes ) );

			// Set the matrixes to general, not Identity as default.
			matWorldToCamera().SetfIdentity( false );
			matWorldToScreen().SetfIdentity( false );
			m_Directory = 0;
		}
		else
		{
			Aqsis::log() << error << "Shadow map \"" << m_strName.c_str() << "\" not found" << std::endl;
		}
	}
}

//----------------------------------------------------------------------
/** Read the matrices out of the tiff file.
 */

void CqShadowMapOld::ReadMatrices()
{
	// Read the transform matrices.
	TqFloat*	WToC;
	TqFloat*	WToS;
	CqMatrix	matWToC, matWToS;

	// Set the number of shadow maps initially to 0.
	m_NumberOfMaps = 0;

	CqMatrix matCToW;
	QGetRenderContextI() ->matSpaceToSpace( "camera", "world", NULL, NULL, QGetRenderContextI()->Time(), matCToW );

	TqDouble minz;

	while(1)
	{
		TqInt reta = TIFFGetField( m_pImage, TIFFTAG_PIXAR_MATRIX_WORLDTOCAMERA, &WToC );
		TqInt retb = TIFFGetField( m_pImage, TIFFTAG_PIXAR_MATRIX_WORLDTOSCREEN, &WToS );
		if ( !reta || !retb )
			SetInvalid();
		else
		{
			TqInt r, c;
			for ( r = 0; r < 4; r++ )
			{
				for ( c = 0; c < 4; c++ )
				{
					matWToC[ r ][ c ] = WToC[ ( r * 4 ) + c ];
					matWToS[ r ][ c ] = WToS[ ( r * 4 ) + c ];
				}
			}
		}
		// Set the matrixes to general, not Identity as default.
		matWToC.SetfIdentity( false );
		matWToS.SetfIdentity( false );

		matWToC *= matCToW;
		matWToS *= matCToW;

		// Generate normal conversion matrices to save time.
		CqMatrix matITTCToL = matWToC;
		matITTCToL[ 3 ][ 0 ] = matITTCToL[ 3 ][ 1 ] = matITTCToL[ 3 ][ 2 ] = matITTCToL[ 0 ][ 3 ] = matITTCToL[ 1 ][ 3 ] = matITTCToL[ 2 ][ 3 ] = 0.0;
		matITTCToL[ 3 ][ 3 ] = 1.0;
		matITTCToL.Inverse();
		matITTCToL.Transpose();

		m_WorldToCameraMatrices.push_back( matWToC );
		m_WorldToScreenMatrices.push_back( matWToS );
		m_ITTCameraToLightMatrices.push_back( matITTCToL );

		if (TIFFGetField( m_pImage, TIFFTAG_SMINSAMPLEVALUE, &minz ))
		{
			m_MinZ.push_back( minz );
		}
    
		m_NumberOfMaps++;	// Increment the number of maps.

		if( TIFFReadDirectory( m_pImage ) == 0 )
			break;
	}
}


void CqShadowMapOld::PrepareSampleOptions( std::map<std::string, IqShaderData*>& paramMap )
{
	CqTextureMapOld::PrepareSampleOptions( paramMap );

	// Extend the shadow() call to accept bias, if set, override global bias
	m_minBias = 0.0f;
	TqFloat maxBias = 0.0f;
	m_biasRange = 0.0f;

	if ( ( !paramMap.empty() ) && ( paramMap.find( "bias" ) != paramMap.end() ) )
	{
		paramMap[ "bias" ] ->GetFloat( m_minBias );
		maxBias = m_minBias;
	}
	if ( ( !paramMap.empty() ) && ( paramMap.find( "bias0" ) != paramMap.end() ) )
	{
		paramMap[ "bias0" ] ->GetFloat( m_minBias );
		maxBias = m_minBias;
	}
	if ( ( !paramMap.empty() ) && ( paramMap.find( "bias1" ) != paramMap.end() ) )
	{
		paramMap[ "bias1" ] ->GetFloat( maxBias );
	}
	
	// Sanity check: make sure min biases is less than max bias.
	if(m_minBias > maxBias)
	{
		TqFloat tmp = m_minBias;
		m_minBias = maxBias;
		maxBias = tmp;
	}

	m_biasRange = maxBias - m_minBias;
}

//---------------------------------------------------------------------
/** Sample the shadow map data to see if the point vecPoint is in shadow.
 */

void CqShadowMapOld::SampleMap( CqVector3D& vecPoint, CqVector3D& swidth, CqVector3D& twidth, std::valarray<TqFloat>& val, TqInt index, TqFloat* average_depth, TqFloat* shadow_depth )
{
	if ( m_pImage != 0 )
	{
		SampleMap( vecPoint, vecPoint, vecPoint, vecPoint, val, index, average_depth, shadow_depth );
	}
	else
	{
		// If no map defined, not in shadow.
		val.resize( 1 );
		val[ 0 ] = 0.0f;
	}
}

//---------------------------------------------------------------------
/** Sample the shadow map data using R1, R2, R3, R4.
 */

void	CqShadowMapOld::SampleMap( CqVector3D& R1, CqVector3D& R2, CqVector3D& R3, CqVector3D& R4, std::valarray<TqFloat>& val, TqInt index, TqFloat* average_depth, TqFloat* shadow_depth )
{
	// Check the memory and make sure we don't abuse it
	if (index == 0)
		CriticalMeasure();


	// If no map defined, not in shadow.
	val.resize( 1 );
	val[ 0 ] = 0.0f;

	CqVector3D	vecR1l;
	CqVector3D	vecR1m, vecR2m, vecR3m, vecR4m;


	// Generate a matrix to transform points from camera space into the space of the light source used in the
	// definition of the shadow map.
	CqMatrix& matCameraToLight = matWorldToCamera( index );
	// Generate a matrix to transform points from camera space into the space of the shadow map.
	CqMatrix& matCameraToMap = matWorldToScreen( index );

	bool fouridenticals = ((R1 == R2 ) && (R2 == R3) && (R3 == R4));

	if (fouridenticals)
		vecR1l = matCameraToLight * R1;
	else
		vecR1l = matCameraToLight * ( ( R1 + R2 + R3 + R4 ) * 0.25f );

	TqFloat z = vecR1l.z();

	// If point is behind light, call it not in shadow.
	if (z <= 0.0f)
		return;

	vecR1m = matCameraToMap * R1;
	if (fouridenticals)
	{
		vecR2m = vecR3m = vecR4m = vecR1m;
	}
 	else
 	{
 		vecR2m = matCameraToMap * R2;
 		vecR3m = matCameraToMap * R3;
 		vecR4m = matCameraToMap * R4;
 	}

	TqFloat xro2 = (m_XRes - 1) * 0.5f;
	TqFloat yro2 = (m_YRes - 1) * 0.5f;

	TqFloat sbo2 = m_sblur * xro2;
	TqFloat tbo2 = m_tblur * yro2;


	TqFloat s1 = vecR1m.x() * xro2 + xro2;
	TqFloat t1 = m_YRes - ( vecR1m.y() * yro2 + yro2 ) - 1;
	TqFloat s2 = vecR2m.x() * xro2 + xro2;
	TqFloat t2 = m_YRes - ( vecR2m.y() * yro2 + yro2 ) - 1;
	TqFloat s3 = vecR3m.x() * xro2 + xro2;
	TqFloat t3 = m_YRes - ( vecR3m.y() * yro2 + yro2 ) - 1;
	TqFloat s4 = vecR4m.x() * xro2 + xro2;
	TqFloat t4 = m_YRes - ( vecR4m.y() * yro2 + yro2 ) - 1;

	TqFloat smin = ( s1 < s2 ) ? s1 : ( s2 < s3 ) ? s2 : ( s3 < s4 ) ? s3 : s4;
	TqFloat smax = ( s1 > s2 ) ? s1 : ( s2 > s3 ) ? s2 : ( s3 > s4 ) ? s3 : s4;
	TqFloat tmin = ( t1 < t2 ) ? t1 : ( t2 < t3 ) ? t2 : ( t3 < t4 ) ? t3 : t4;
	TqFloat tmax = ( t1 > t2 ) ? t1 : ( t2 > t3 ) ? t2 : ( t3 > t4 ) ? t3 : t4;

	// Cull if outside bounding box.
	TqUint lu = lfloor(smin - sbo2);
	TqUint hu = lceil(smax + sbo2);
	TqUint lv = lfloor(tmin - tbo2);
	TqUint hv = lceil(tmax + tbo2);

	if ( lu >= m_XRes || hu < 0 || lv >= m_YRes || hv < 0 )
		return ;

	lu = max<TqUint>(0,lu);
	lv = max<TqUint>(0,lv);
	hu = min(m_XRes - 1,hu);
	hv = min(m_YRes - 1,hv);

	TqFloat sres = (1.0 + m_pswidth/2.0) * (hu - lu);
	TqFloat tres = (1.0 + m_ptwidth/2.0) * (hv - lv);


	if (sres < MinSize)
		sres = MinSize;
	if (tres < MinSize)
		tres = MinSize;

	if (sres > m_XRes/2)
		sres = m_XRes/2;
	if (tres > m_YRes/2)
		tres = m_YRes/2;

	// Calculate no. of samples.
	TqUint nt, ns;

	if ( m_samples > 0 )
	{

		nt = ns =  3 * lceil(sqrt(m_samples));
	}
	else
	{
		if ( sres * tres * 4.0 < NumSamples )
		{
			ns = static_cast<TqInt>( sres * 2.0 + 0.5 );
			ns = ( ns < MinSamples ? MinSamples : ( ns > NumSamples ? NumSamples : ns ) );
			nt = static_cast<TqInt>( tres * 2.0 + 0.5 );
			nt = ( nt < MinSamples ? MinSamples : ( nt > NumSamples ? NumSamples : nt ) );
		}
		else
		{
			nt = static_cast<TqInt>( sqrt( tres * NumSamples / sres ) + 0.5 );
			nt = ( nt < MinSamples ? MinSamples : ( nt > NumSamples ? NumSamples : nt ) );
			ns = static_cast<TqInt>( static_cast<TqFloat>( NumSamples ) / nt + 0.5 );
			ns = ( ns < MinSamples ? MinSamples : ( ns > NumSamples ? NumSamples : ns ) );
		}
	}


	// Is this shadowmap an occlusion map (NumPages() > 1) ?
	// NumPages() contains the number of shadowmaps which were used 
	// to create this occlusion map.
	// if ns, nt is computed assuming only one shadow map; let try divide 
	// both numbers by the sqrt() of the number of maps. 
	//
	// eg: assuming I have 256 shadowmaps (256x256) in one occlusion map
	// and ns = nt = 16 (depends solely of the du,dv,blur)
	// Than the number of computations will be at the worst case 
	// (re-visit each Z values on each shadowmaps).  
	// 256 shadows * ns * nt * 256 * 256. or the 4G operations. 
	// 
	// We need to reduce the number of operations by one or two order 
	// of magnitude then.
	// An solution is to divide the numbers ns,nt by the sqrt(number of shadows). 
	// In the examples something in range of 
	// 256x256x256 x (16 x 16) / (8 x 8)  or 256 x 256 x 256 x 2 x 2 or 64M 
	// operations good start; but not enough.  
	// But what happens if you only have 4 shadows ?
	// 256x256x256 x (16 x 16) /( 2 x 2) or 256x256x256x64 about 1G 
	// it is worst than with 256 maps. So I chose another solution is to do 
	// following let recompute ns, nt so the number of operations will 
	// never reach more 256k. 
	// 1) Bigger the number of shadowmap smaller ns, nt will be required; 
	// 2) Bigger the shadowmaps than smaller ns, nt;

	if (NumPages() > 1) {
		TqInt samples = lfloor(sqrt(m_samples));
		TqInt occl = (256 * 1024) / (NumPages() * XRes() * YRes());
		occl = lceil(sqrt(static_cast<TqFloat>(occl)));
		occl = max(2, occl);
		// Samples could overwrite after all the magic number!!
		occl = max(samples, occl); 
		ns = nt = occl;
	}

	// Setup jitter variables
	TqFloat ds = sres / ns;
	TqFloat dt = tres / nt;
	TqFloat js = ds * 0.5f;
	TqFloat jt = dt * 0.5f;

	// Test the samples.
	TqInt inshadow = 0;
	TqUint i;
	TqFloat avz = 0.0f;
	TqFloat sample_z = 0.0f; // How deep we're in the shadow

	// I've really no idea what rbias does, but in the process of fixing the
	// bias behaviour quickly (prior to the texture refactor which will replace
	// much of this), I'm leaving it with the same value which it previously
	// contained - CJF.
	TqFloat rbias = m_minBias + 0.5*m_biasRange;

  	// A conservative z value is the worst case scenario
	// for the high bias value will be between 0..2.0 * rbias
        TqDouble minz = MinZ(index);

	if ((minz != RI_FLOATMAX) && ((z + 2.0 * rbias) < minz))
		return;

	index = PseudoMipMaps( lu , index );

	CqTextureMapBuffer * pTMBa = GetBuffer( lu, lv, index );

	bool valid =  pTMBa  && pTMBa->IsValid (hu, hv, index );

	// Don't do the area computing of the shadow if the conservative z 
	// value is either lower than the current minz value of the tile or 
	// even if conservative z value is higher than the maxz value 
	// (assuming the maxz is not infinite)
  	// A conservative z value is the worst case scenario
	// for the high bias value will be between 0..2.0 * rbias

	if ( (NumPages() > 1) && valid )
        {
		TqFloat minz, maxz;

		pTMBa->MinMax(minz, maxz, 0);
		
		if ((z + 2.0 * rbias) < minz ) 
		{
			return;
		}
		if (maxz != RI_FLOATMAX) 
		{
			if ((z - 2.0 * rbias) > maxz  )
			{
				val[0] = 1.0;
				return;
			}
		}
	}

	TqFloat sdelta = (sres - static_cast<TqFloat>(hu - lu) ) / 2.0;
	TqFloat tdelta = (tres - static_cast<TqFloat>(hv - lv) ) / 2.0;
	TqFloat s = lu - sdelta;
	TqFloat t = lv - tdelta;

	// Speedup for the case of normal shadowmap; if we ever recompute around the same point
	// we will return the previous value.
	CqVector2D vecPoint(s,t);
	if ((NumPages() == 1) && (vecPoint.x() == m_LastPoint.x()) && (vecPoint.y() == m_LastPoint.y()))
	{
		val[0] = m_Val;
		if (average_depth)
			*average_depth = m_Average;
		if (shadow_depth)
			*shadow_depth = m_Depth;
		return;
	}

	for ( i = 0; i < ns; i++, s += ds )
	{
		t = lv - tdelta;
		TqUint j;

		for ( j = 0; j < nt; j++, t += dt )
		{
			// Jitter s and t
			m_rand_index = ( m_rand_index + 1 ) & 1023;
			TqInt iu = static_cast<TqUint>( s + m_aRand_no[ m_rand_index ] * js );
			m_rand_index = ( m_rand_index + 1 ) & 1023;
			TqInt iv = static_cast<TqUint>( t + m_aRand_no[ m_rand_index ] * jt );

			if( iu < 0 || iu >= (TqInt) m_XRes || iv < 0 || iv >= (TqInt) m_YRes )
			{
				continue;
			}

			if( ( pTMBa == NULL )  || !pTMBa->IsValid( iu, iv, index ) )
			{
				pTMBa = GetBuffer( iu, iv, index );
			}

			if( ( pTMBa != NULL) && ( pTMBa->pVoidBufferData() != 0 ) )
			{
				iu -= pTMBa->sOrigin();
				iv -= pTMBa->tOrigin();

				TqFloat mapz = pTMBa->GetValue( iu, iv, 0 );

				m_rand_index = ( m_rand_index + 1 ) & 1023;
				TqFloat bias = m_biasRange*m_aRand_no[m_rand_index] + m_minBias;

				if ( z > mapz + bias)
				{
					inshadow ++;
					sample_z = z - mapz;
				}
				avz += mapz;
			}
		}
	}

	if( NULL != average_depth )
	{
		avz /= ( ns * nt );
		*average_depth = avz;
	}

	if( NULL != shadow_depth )
	{
		sample_z /= ( ns * nt );

		*shadow_depth = sample_z;
	}

	val[ 0 ] = ( static_cast<TqFloat>( inshadow ) / ( ns * nt ) );

	// Keep track of computed values it might be usefull later in the next iteration
	if (NumPages() == 1)
	{
		m_LastPoint = vecPoint;
		m_Val = val[0];
		m_Depth = sample_z;
		m_Average = avz;
	}
}

//----------------------------------------------------------------------
/** Save the shadowmap data in system specifirc image format.
 */

void CqShadowMapOld::SaveShadowMapOld( const CqString& strShadowName, bool append )
{
	const char* mode = (append)? "a" : "w";

	// Save the shadowmap to a binary file.
	if ( m_strName.compare( "" ) != 0 )
	{
		if ( ! m_apFlat.empty() )
		{
			TIFF * pshadow = TIFFOpen( strShadowName.c_str(), mode );
			TIFFCreateDirectory( pshadow );

			// Write the transform matrices.
			TqFloat	matWToC[ 16 ];
			TqFloat	matWToS[ 16 ];
			TqInt r, c;
			for ( r = 0; r < 4; r++ )
			{
				for ( c = 0; c < 4; c++ )
				{
					matWToC[ ( r * 4 ) + c ] = matWorldToCamera()[ r ][ c ];
					matWToS[ ( r * 4 ) + c ] = matWorldToScreen()[ r ][ c ];
				}
			}
			TIFFSetField( pshadow, TIFFTAG_PIXAR_MATRIX_WORLDTOCAMERA, matWToC );
			TIFFSetField( pshadow, TIFFTAG_PIXAR_MATRIX_WORLDTOSCREEN, matWToS );
			TIFFSetField( pshadow, TIFFTAG_PIXAR_TEXTUREFORMAT, SHADOWMAP_HEADER );
			TIFFSetField( pshadow, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK );

			// Write the floating point image to the directory.
			TqDouble minz = RI_FLOATMAX;
			TqFloat *depths = reinterpret_cast<TqFloat*>( m_apFlat.front() ->pVoidBufferData() );
			for (TqUint y =0; y < YRes(); y++)
				for (TqUint x = 0; x < XRes(); x++)
					minz = min(minz, (TqDouble)depths[y*XRes() + x]);
			TIFFSetField( pshadow, TIFFTAG_SMINSAMPLEVALUE, minz );
			WriteTileImage( pshadow, depths, XRes(), YRes(), 32, 32, 1, m_Compression, m_Quality );
			TIFFClose( pshadow );
		}
	}
}

//----------------------------------------------------------------------
/** Save the shadowmap data in system specifirc image format.
 */

void CqShadowMapOld::SaveZFile()
{
	// Save the shadowmap to a binary file.
	if ( m_strName != "" )
	{
		std::ofstream ofile( m_strName.c_str(), std::ios::out | std::ios::binary );
		if ( ofile.is_open() )
		{
			// Save a file type and version marker
			ofile << ZFILE_HEADER;

			// Save the xres and yres.
			ofile.write( reinterpret_cast<TqPchar >( &m_XRes ), sizeof( m_XRes ) );
			ofile.write( reinterpret_cast<TqPchar >( &m_YRes ), sizeof( m_XRes ) );

			// Save the transformation matrices.
			ofile.write( reinterpret_cast<TqPchar>( matWorldToCamera()[ 0 ] ), sizeof( matWorldToCamera()[ 0 ][ 0 ] ) * 4 );
			ofile.write( reinterpret_cast<TqPchar>( matWorldToCamera()[ 1 ] ), sizeof( matWorldToCamera()[ 0 ][ 0 ] ) * 4 );
			ofile.write( reinterpret_cast<TqPchar>( matWorldToCamera()[ 2 ] ), sizeof( matWorldToCamera()[ 0 ][ 0 ] ) * 4 );
			ofile.write( reinterpret_cast<TqPchar>( matWorldToCamera()[ 3 ] ), sizeof( matWorldToCamera()[ 0 ][ 0 ] ) * 4 );

			ofile.write( reinterpret_cast<TqPchar>( matWorldToScreen()[ 0 ] ), sizeof( matWorldToScreen()[ 0 ][ 0 ] ) * 4 );
			ofile.write( reinterpret_cast<TqPchar>( matWorldToScreen()[ 1 ] ), sizeof( matWorldToScreen()[ 0 ][ 0 ] ) * 4 );
			ofile.write( reinterpret_cast<TqPchar>( matWorldToScreen()[ 2 ] ), sizeof( matWorldToScreen()[ 0 ][ 0 ] ) * 4 );
			ofile.write( reinterpret_cast<TqPchar>( matWorldToScreen()[ 3 ] ), sizeof( matWorldToScreen()[ 0 ][ 0 ] ) * 4 );

			// Now output the depth values
			ofile.write( reinterpret_cast<TqPchar>( m_apFlat.front() ->pVoidBufferData() ), sizeof( TqFloat ) * ( m_XRes * m_YRes ) );
			ofile.close();
		}
	}
}





} // namespace Aqsis



