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
		\brief Implements the basic shader operations.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	<math.h>

#include	"aqsis.h"
#include	"lights.h"
#include	"shadervm.h"
#include	"shaderexecenv.h"
#include	"texturemap.h"
#include	"spline.h"
#include	"surface.h"

START_NAMESPACE( Aqsis )

static	TqFloat	temp_float;

//----------------------------------------------------------------------
// SO_sprintf
// Helper function to process a string inserting variable, used in printf and format.

static	CqString	SO_sprintf( const char* str, int cParams, CqVMStackEntry** apParams, int varyingindex )
{
	CqString strRes( "" );
	CqString strTrans = str; //str.TranslateEqcapes();

	TqUint i = 0;
	TqUint ivar = 0;
	while ( i < strTrans.size() )
	{
		switch ( strTrans[ i ] )
		{
				case '%': 	// Insert variable.
				{
					i++;
					switch ( strTrans[ i ] )
					{
							case 'f':
							{
								TqFloat f;
								apParams[ ivar++ ] ->Value( f, varyingindex );
								CqString strVal;
								strVal.Format( "%f", f );
								strRes += strVal;
							}
							break;

							case 'p':
							{
								CqVector3D vec;
								apParams[ ivar++ ] ->Value( vec, varyingindex );
								CqString strVal;
								strVal.Format( "%f,%f,%f", vec.x(), vec.y(), vec.z() );
								strRes += strVal;
							}
							break;

							case 'c':
							{
								CqColor col;
								apParams[ ivar++ ] ->Value( col, varyingindex );
								CqString strVal;
								strVal.Format( "%f,%f,%f", col.fRed(), col.fGreen(), col.fBlue() );
								strRes += strVal;
							}
							break;

							case 'm':
							{
								CqMatrix mat;
								apParams[ ivar++ ] ->Value( mat, varyingindex );
								CqString strVal;
								strVal.Format( "%f,%f,%f,%f\n%f,%f,%f,%f\n%f,%f,%f,%f\n%f,%f,%f,%f",
								               mat.Element( 0, 0 ), mat.Element( 0, 1 ), mat.Element( 0, 2 ), mat.Element( 0, 3 ),
								               mat.Element( 1, 0 ), mat.Element( 1, 1 ), mat.Element( 1, 2 ), mat.Element( 1, 3 ),
								               mat.Element( 2, 0 ), mat.Element( 2, 1 ), mat.Element( 2, 2 ), mat.Element( 2, 3 ),
								               mat.Element( 3, 0 ), mat.Element( 3, 1 ), mat.Element( 3, 2 ), mat.Element( 3, 3 ) );
								strRes += strVal;
							}
							break;

							case 's':
							{
								CqString stra;
								apParams[ ivar++ ] ->Value( stra, varyingindex );
								strRes += stra;
							}
							break;

							default:
							{
								strRes += strTrans[ i ];
							}
							break;
					}
					i++;
				}
				break;

				default:
				{
					strRes += strTrans[ i ];
					i++;
				}
				break;
		}
	}
	return ( strRes );
}

//----------------------------------------------------------------------
// init_illuminance()
TqBool CqShaderExecEnv::SO_init_illuminance()
{
	m_li = -1;
	return ( SO_advance_illuminance() );
}


//----------------------------------------------------------------------
// advance_illuminance()
TqBool CqShaderExecEnv::SO_advance_illuminance()
{
	m_li++;
	while ( m_li < static_cast<TqInt>( m_pSurface->pAttributes() ->apLights().size() ) &&
	        m_pSurface->pAttributes() ->apLights() [ m_li ] ->pShader() ->fAmbient() )
	{
		m_li++;
	}
	if ( m_li < static_cast<TqInt>( m_pSurface->pAttributes() ->apLights().size() ) ) return ( TqTrue );
	else	return ( TqFalse );
}


void CqShaderExecEnv::ValidateIlluminanceCache( CqVMStackEntry& P, CqShader* pShader )
{
	// If this is the first call to illuminance this time round, call all lights and setup the Cl and L caches.
	if ( !m_IlluminanceCacheValid )
	{
		// Fist cache the shaderexecenv state for later retrieval.
		TqInt	cacheGridI = m_GridI;

		TqUint li = 0;
		while ( li < m_pSurface->pAttributes() ->apLights().size() )
		{
			CqLightsource * lp = m_pSurface->pAttributes() ->apLights() [ li ];
			// Initialise the lightsource
			lp->Initialise( uGridRes(), vGridRes() );
			m_Illuminate = 0;
			// Evaluate the lightsource
			lp->Evaluate( P );
			li++;
		}
		m_IlluminanceCacheValid = TqTrue;;

		// Restore the state prior to executing the lightsources.
		m_GridI = cacheGridI;
	}
}


STD_SOIMPL	CqShaderExecEnv::SO_radians( FLOATVAL degrees, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( degrees )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( degrees );
	Result.SetValue( i, FLOAT( degrees ) / ( 180.0f / RI_PI ) );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_degrees( FLOATVAL radians, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( radians )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( radians );
	Result.SetValue( i, ( FLOAT( radians ) / 180.0f ) * RI_PI );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_sin( FLOATVAL a, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( a )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( a );
	Result.SetValue( i, static_cast<TqFloat>( sin( FLOAT( a ) ) ) );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_asin( FLOATVAL a, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( a )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( a );
	Result.SetValue( i, static_cast<TqFloat>( asin( FLOAT( a ) ) ) );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_cos( FLOATVAL a, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( a )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( a );
	Result.SetValue( i, static_cast<TqFloat>( cos( FLOAT( a ) ) ) );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_acos( FLOATVAL a, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( a )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( a );
	Result.SetValue( i, static_cast<TqFloat>( acos( FLOAT( a ) ) ) );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_tan( FLOATVAL a, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( a )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( a );
	Result.SetValue( i, static_cast<TqFloat>( tan( FLOAT( a ) ) ) );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_atan( FLOATVAL yoverx, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( yoverx )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( yoverx );
	Result.SetValue( i, static_cast<TqFloat>( atan( FLOAT( yoverx ) ) ) );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_atan( FLOATVAL y, FLOATVAL x, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( x )
	CHECKVARY( y )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( x );
	GETFLOAT( y );
	Result.SetValue( i, static_cast<TqFloat>( atan2( FLOAT( y ), FLOAT( x ) ) ) );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_pow( FLOATVAL x, FLOATVAL y, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( x )
	CHECKVARY( y )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( x );
	GETFLOAT( y );
	Result.SetValue( i, static_cast<TqFloat>( pow( FLOAT( x ), FLOAT( y ) ) ) );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_exp( FLOATVAL x, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( x )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( x );
	Result.SetValue( i, static_cast<TqFloat>( exp( FLOAT( x ) ) ) );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_sqrt( FLOATVAL x, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( x )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( x );
	Result.SetValue( i, static_cast<TqFloat>( sqrt( FLOAT( x ) ) ) );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_log( FLOATVAL x, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( x )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( x );
	Result.SetValue( i, static_cast<TqFloat>( log( FLOAT( x ) ) ) );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_mod( FLOATVAL a, FLOATVAL b, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( a )
	CHECKVARY( b )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( a );
	GETFLOAT( b );
	TqInt n = static_cast<TqInt>( FLOAT( a ) / FLOAT( b ) );
	TqFloat a2 = FLOAT( a ) - n * FLOAT( b );
	if ( a2 < 0.0f )
		a2 += FLOAT( b );
	Result.SetValue( i, a2 );
	END_FORR
}

//----------------------------------------------------------------------
// log(x,base)
STD_SOIMPL	CqShaderExecEnv::SO_log( FLOATVAL x, FLOATVAL base, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( x )
	CHECKVARY( base )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( x );
	GETFLOAT( base );
	Result.SetValue( i, static_cast<TqFloat>( log( FLOAT( x ) ) / log( FLOAT( base ) ) ) );
	END_FORR
}


STD_SOIMPL	CqShaderExecEnv::SO_abs( FLOATVAL x, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( x )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( x );
	Result.SetValue( i, static_cast<TqFloat>( fabs( FLOAT( x ) ) ) );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_sign( FLOATVAL x, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( x )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( x );
	Result.SetValue( i, ( FLOAT( x ) < 0.0f ) ? -1.0f : 1.0f );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_min( FLOATVAL a, FLOATVAL b, DEFPARAMVARIMPL )
{
	INIT_SOR
	CHECKVARY( a )
	CHECKVARY( b )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( a );
	GETFLOAT( b );
	TqFloat fRes = MIN( FLOAT( a ), FLOAT( b ) );
	while ( cParams-- > 0 )
	{
		CqVMStackEntry& next = *apParams[ cParams ];
		GETFLOAT( next );
		fRes = MIN( fRes, FLOAT( next ) );
	}
	Result.SetValue( i, fRes );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_max( FLOATVAL a, FLOATVAL b, DEFPARAMVARIMPL )
{
	INIT_SOR
	CHECKVARY( a )
	CHECKVARY( b )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( a );
	GETFLOAT( b );
	TqFloat fRes = MAX( FLOAT( a ), FLOAT( b ) );
	while ( cParams-- > 0 )
	{
		CqVMStackEntry& next = *apParams[ cParams ];
		GETFLOAT( next );
		fRes = MAX( fRes, FLOAT( next ) );
	}
	Result.SetValue( i, fRes );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_pmin( POINTVAL a, POINTVAL b, DEFPARAMVARIMPL )
{
	INIT_SOR
	CHECKVARY( a )
	CHECKVARY( b )
	CHECKVARY( Result )
	FOR_EACHR
	GETPOINT( a );
	GETPOINT( b );
	CqVector3D res = VMIN( POINT( a ), POINT( b ) );
	while ( cParams-- > 0 )
	{
		CqVMStackEntry& next = *apParams[ cParams ];
		GETPOINT( next );
		res = VMIN( res, POINT( next ) );
	}
	Result.SetValue( i, res );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_pmax( POINTVAL a, POINTVAL b, DEFPARAMVARIMPL )
{
	INIT_SOR
	CHECKVARY( a )
	CHECKVARY( b )
	CHECKVARY( Result )
	FOR_EACHR
	GETPOINT( a );
	GETPOINT( b );
	CqVector3D res = VMAX( POINT( a ), POINT( b ) );
	while ( cParams-- > 0 )
	{
		CqVMStackEntry& next = *apParams[ cParams ];
		GETPOINT( next );
		res = VMAX( res, POINT( next ) );
	}
	Result.SetValue( i, res );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_cmin( COLORVAL a, COLORVAL b, DEFPARAMVARIMPL )
{
	INIT_SOR
	CHECKVARY( a )
	CHECKVARY( b )
	CHECKVARY( Result )
	FOR_EACHR
	GETCOLOR( a );
	GETCOLOR( b );
	CqColor res = CMIN( COLOR( a ), COLOR( b ) );
	while ( cParams-- > 0 )
	{
		CqVMStackEntry& next = *apParams[ cParams ];
		GETCOLOR( next );
		res = CMIN( res, COLOR( next ) );
	}
	Result.SetValue( i, res );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_cmax( COLORVAL a, COLORVAL b, DEFPARAMVARIMPL )
{
	INIT_SOR
	CHECKVARY( a )
	CHECKVARY( b )
	CHECKVARY( Result )
	FOR_EACHR
	GETCOLOR( a );
	GETCOLOR( b );
	CqColor res = CMAX( COLOR( a ), COLOR( b ) );
	while ( cParams-- > 0 )
	{
		CqVMStackEntry& next = *apParams[ cParams ];
		GETCOLOR( next );
		res = CMAX( res, COLOR( next ) );
	}
	Result.SetValue( i, res );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_clamp( FLOATVAL a, FLOATVAL min, FLOATVAL max, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( a )
	CHECKVARY( min )
	CHECKVARY( max )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( a );
	GETFLOAT( min );
	GETFLOAT( max );
	Result.SetValue( i, CLAMP( FLOAT( a ), FLOAT( min ), FLOAT( max ) ) );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_pclamp( POINTVAL a, POINTVAL min, POINTVAL max, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( a )
	CHECKVARY( min )
	CHECKVARY( max )
	CHECKVARY( Result )
	FOR_EACHR
	GETPOINT( a );
	GETPOINT( min );
	GETPOINT( max );
	Result.SetValue( i, VCLAMP( POINT( a ), POINT( min ), POINT( max ) ) );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_cclamp( COLORVAL a, COLORVAL min, COLORVAL max, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( a )
	CHECKVARY( min )
	CHECKVARY( max )
	CHECKVARY( Result )
	FOR_EACHR
	GETCOLOR( a );
	GETCOLOR( min );
	GETCOLOR( max );
	Result.SetValue( i, CCLAMP( COLOR( a ), COLOR( min ), COLOR( max ) ) );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_floor( FLOATVAL x, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( x )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( x );
	Result.SetValue( i, static_cast<TqFloat>( FLOOR( FLOAT( x ) ) ) );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_ceil( FLOATVAL x, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( x )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( x );
	Result.SetValue( i, static_cast<TqFloat>( CEIL( FLOAT( x ) ) ) );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_round( FLOATVAL x, DEFPARAMIMPL )
{
	double v;
	INIT_SOR
	CHECKVARY( x )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( x );
	Result.SetValue( i, ( modf( FLOAT( x ), &v ) > 0.5f ) ? static_cast<TqFloat>( v ) + 1.0f : static_cast<TqFloat>( v ) );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_step( FLOATVAL min, FLOATVAL value, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( min )
	CHECKVARY( value )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( min );
	GETFLOAT( value );
	Result.SetValue( i, ( FLOAT( value ) < FLOAT( min ) ) ? 0.0f : 1.0f );
	END_FORR
}


//----------------------------------------------------------------------
// smoothstep(min,max,value)
STD_SOIMPL	CqShaderExecEnv::SO_smoothstep( FLOATVAL min, FLOATVAL max, FLOATVAL value, DEFPARAMIMPL )
{
	// TODO: Check this against Eberts code
	INIT_SOR
	CHECKVARY( value )
	CHECKVARY( min )
	CHECKVARY( max )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( min );
	GETFLOAT( max );
	GETFLOAT( value );
	if ( FLOAT( value ) < FLOAT( min ) )
		Result.SetValue( i, 0.0f );
	else if ( FLOAT( value ) >= FLOAT( max ) )
		Result.SetValue( i, 1.0f );
	else
	{
		TqFloat v = ( FLOAT( value ) - FLOAT( min ) ) / ( FLOAT( max ) - FLOAT( min ) );
		Result.SetValue( i, v * v * ( 3.0f - 2.0f * v ) );
	}
	END_FORR
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_fspline( FLOATVAL value, DEFPARAMVARIMPL )
{
	INIT_SOR
	CHECKVARY( value )
	TqInt v;
	for ( v = 0; v < cParams; v++ ) CHECKVARY( ( *apParams[ v ] ) )
		CHECKVARY( Result )

		CqSplineCubic spline( cParams );

	FOR_EACHR
	GETFLOAT( value );
	if ( FLOAT( value ) >= 1.0f )
	{
		CqVMStackEntry& last = *apParams[ cParams - 2 ];
		GETFLOAT( last );
		Result.SetValue( i, FLOAT( last ) );
	}
	else if ( FLOAT( value ) <= 0.0f ) 
	{
		CqVMStackEntry& first = *apParams[ 1 ];
		GETFLOAT( first );
		Result.SetValue( i, FLOAT( first ) );
	}
	else
	{
		TqInt j;
		for ( j = 0; j < cParams; j++ )
		{
			CqVMStackEntry& next = *apParams[ j ];
			GETFLOAT( next );
			spline[ j ] = CqVector4D( FLOAT( next ), 0.0f, 0.0f, 1.0f );
		}

		CqVector4D	res = spline.Evaluate( FLOAT( value ) );
		Result.SetValue( i, res.x() );
	}
	END_FORR
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_cspline( FLOATVAL value, DEFPARAMVARIMPL )
{
	INIT_SOR
	CHECKVARY( value )
	TqInt v;
	for ( v = 0; v < cParams; v++ ) CHECKVARY( ( *apParams[ v ] ) )
		CHECKVARY( Result )

		CqSplineCubic spline( cParams );

	FOR_EACHR
	GETFLOAT( value );
	if ( FLOAT( value ) >= 1.0f )
	{
		CqVMStackEntry& last = *apParams[ cParams - 2 ];
		GETCOLOR( last );
		Result.SetValue( i, COLOR( last ) );
	}
	else if ( FLOAT( value ) <= 0.0f )
	{
		CqVMStackEntry& first = *apParams[ 1 ];
		GETCOLOR( first );
		Result.SetValue( i, COLOR( first ) );
	}
	else
	{
		TqInt j;
		for ( j = 0; j < cParams; j++ )
		{
			CqVMStackEntry& next = *apParams[ j ];
			GETCOLOR( next );
			spline[ j ] = CqVector4D( COLOR( next ).fRed(), COLOR( next ).fGreen(), COLOR( next ).fBlue(), 1.0f );
		}

		CqVector4D	res = spline.Evaluate( FLOAT( value ) );
		Result.SetValue( i, CqColor( res.x(), res.y(), res.z() ) );
	}
	END_FORR
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_pspline( FLOATVAL value, DEFPARAMVARIMPL )
{
	INIT_SOR
	CHECKVARY( value )
	TqInt v;
	for ( v = 0; v < cParams; v++ ) CHECKVARY( ( *apParams[ v ] ) )
		CHECKVARY( Result )

		CqSplineCubic spline( cParams );

	FOR_EACHR
	GETFLOAT( value );
	if ( FLOAT( value ) >= 1.0f )
	{
		CqVMStackEntry& last = *apParams[ cParams - 2 ];
		GETPOINT( last );
		Result.SetValue( i, POINT( last ) );
	}
	else if ( FLOAT( value ) <= 0.0f )
	{
		CqVMStackEntry& first = *apParams[ 1 ];
		GETPOINT( first );
		Result.SetValue( i, POINT( first ) );
	}
	else
	{
		TqInt j;
		for ( j = 0; j < cParams; j++ )
		{
			CqVMStackEntry& next = *apParams[ j ];
			GETPOINT( next );
			spline[ j ] = POINT( next );
		}

		CqVector3D	res = spline.Evaluate( FLOAT( value ) );
		Result.SetValue( i, res );
	}
	END_FORR
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_sfspline( STRINGVAL basis, FLOATVAL value, DEFPARAMVARIMPL )
{
	INIT_SOR
	CHECKVARY( value )
	TqInt v;
	for ( v = 0; v < cParams; v++ )
	{
		CHECKVARY( ( *apParams[ v ] ) )
	}
	CHECKVARY( Result )

	TqInt i = 0;
	CqSplineCubic spline( cParams );
	GETSTRING( basis );
	spline.SetmatBasis( STRING( basis ) );

	FOR_EACHR
	GETFLOAT( value );
	if ( FLOAT( value ) >= 1.0f )
	{
		CqVMStackEntry& last = *apParams[ cParams - 2 ];
		GETFLOAT( last );
		Result.SetValue( i, FLOAT( last ) );
	}
	else if ( FLOAT( value ) <= 0.0f )
	{
		CqVMStackEntry& first = *apParams[ 1 ];
		GETFLOAT( first );
		Result.SetValue( i, FLOAT( first ) );
	}
	else
	{
		TqInt j;
		for ( j = 0; j < cParams; j++ )
		{
			CqVMStackEntry& next = *apParams[ j ];
			GETFLOAT( next );
			spline[ j ] = CqVector4D( FLOAT( next ), 0.0f, 0.0f, 1.0f );
		}

		CqVector4D	res = spline.Evaluate( FLOAT( value ) );
		Result.SetValue( i, res.x() );
	}
	END_FORR
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_scspline( STRINGVAL basis, FLOATVAL value, DEFPARAMVARIMPL )
{
	INIT_SOR
	CHECKVARY( value )
	TqInt v;
	for ( v = 0; v < cParams; v++ )
	{
		CHECKVARY( ( *apParams[ v ] ) )
	}
	
	CHECKVARY( Result )
	CqSplineCubic spline( cParams );

	TqInt i = 0;
	GETFLOAT( basis );
	spline.SetmatBasis( STRING( basis) );

	FOR_EACHR
	GETFLOAT( value );
	if ( FLOAT( value ) >= 1.0f )
	{
		CqVMStackEntry& last = *apParams[ cParams - 2 ];
		GETCOLOR( last );
		Result.SetValue( i, COLOR( last ) );
	}
	else if ( FLOAT( value ) <= 0.0f )
	{
		CqVMStackEntry& first = *apParams[ 1 ];
		GETCOLOR( first );
		Result.SetValue( i, COLOR( first ) );
	}
	else
	{
		TqInt j;
		for ( j = 0; j < cParams; j++ )
		{
			CqVMStackEntry& next = *apParams[ j ];
			GETCOLOR( next );
			spline[ j ] = CqVector4D( COLOR( next ).fRed(), COLOR( next ).fGreen(), COLOR( next ).fBlue(), 1.0f );
		}

		CqVector4D	res = spline.Evaluate( FLOAT( value ) );
		Result.SetValue( i, CqColor( res.x(), res.y(), res.z() ) );
	}
	END_FORR
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_spspline( STRINGVAL basis, FLOATVAL value, DEFPARAMVARIMPL )
{
	INIT_SOR
	CHECKVARY( value )
	TqInt v;
	for ( v = 0; v < cParams; v++ )
	{
		CHECKVARY( ( *apParams[ v ] ) )
	}
	
	CHECKVARY( Result )
	CqSplineCubic spline( cParams );

	TqInt i = 0;
	GETSTRING( basis );
	spline.SetmatBasis( STRING( basis ) );

	FOR_EACHR
	GETFLOAT( value );
	if ( FLOAT( value ) >= 1.0f )
	{
		CqVMStackEntry& last = *apParams[ cParams - 2 ];
		GETPOINT( last );
		Result.SetValue( i, POINT( last ) );
	}
	else if ( FLOAT( value ) <= 0.0f )
	{
		CqVMStackEntry& first = *apParams[ 1 ];
		GETPOINT( first );
		Result.SetValue( i, POINT( first ) );
	}
	else
	{
		TqInt j;
		for ( j = 0; j < cParams; j++ )
		{
			CqVMStackEntry& next = *apParams[ j ];
			GETPOINT( next );
			spline[ j ] = POINT( next );
		}

		CqVector3D	res = spline.Evaluate( FLOAT( value ) );
		Result.SetValue( i, res );
	}
	END_FORR
}


STD_SOIMPL	CqShaderExecEnv::SO_fDu( FLOATVAL p, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( Result )
	FOR_EACHR
	Result.SetValue( i, SO_DuType<TqFloat>( p, i, *this ) );
	END_FORR
}


STD_SOIMPL	CqShaderExecEnv::SO_fDv( FLOATVAL p, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( Result )
	FOR_EACHR
	Result.SetValue( i, SO_DvType<TqFloat>( p, i, *this ) );
	END_FORR
}


STD_SOIMPL	CqShaderExecEnv::SO_fDeriv( FLOATVAL p, FLOATVAL den, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( den )
	CHECKVARY( Result )
	FOR_EACHR
	Result.SetValue( i, SO_DerivType<TqFloat>( p, den, i, *this ) );
	END_FORR
}


STD_SOIMPL	CqShaderExecEnv::SO_cDu( COLORVAL p, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( Result )
	FOR_EACHR
	Result.SetValue( i, SO_DuType<CqColor>( p, i, *this ) );
	END_FORR
}


STD_SOIMPL	CqShaderExecEnv::SO_cDv( COLORVAL p, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( Result )
	FOR_EACHR
	Result.SetValue( i, SO_DvType<CqColor>( p, i, *this ) );
	END_FORR
}


STD_SOIMPL	CqShaderExecEnv::SO_cDeriv( COLORVAL p, FLOATVAL den, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( den )
	CHECKVARY( Result )
	FOR_EACHR
	Result.SetValue( i, SO_DerivType<CqColor>( p, den, i, *this ) );
	END_FORR
}


STD_SOIMPL	CqShaderExecEnv::SO_pDu( POINTVAL p, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( Result )
	FOR_EACHR
	Result.SetValue( i, SO_DuType<CqVector3D>( p, i, *this ) );
	END_FORR
}


STD_SOIMPL	CqShaderExecEnv::SO_pDv( POINTVAL p, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( Result )
	FOR_EACHR
	Result.SetValue( i, SO_DvType<CqVector3D>( p, i, *this ) );
	END_FORR
}


STD_SOIMPL	CqShaderExecEnv::SO_pDeriv( POINTVAL p, FLOATVAL den, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( den )
	CHECKVARY( Result )
	FOR_EACHR
	Result.SetValue( i, SO_DerivType<CqVector3D>( p, den, i, *this ) );
	END_FORR
}


STD_SOIMPL	CqShaderExecEnv::SO_frandom( DEFPARAMIMPL )
{
	static CqRandom randomiser;

	INIT_SOR
	CHECKVARY( Result )
	FOR_EACHR
	Result.SetValue( i, m_random.RandomFloat() );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_crandom( DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( Result )
	FOR_EACHR
	Result.SetValue( i, CqColor( m_random.RandomFloat(), m_random.RandomFloat(), m_random.RandomFloat() ) );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_prandom( DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( Result )
	FOR_EACHR
	Result.SetValue( i, CqVector3D( m_random.RandomFloat(), m_random.RandomFloat(), m_random.RandomFloat() ) );
	END_FORR
}


//----------------------------------------------------------------------
// noise(v)
STD_SOIMPL	CqShaderExecEnv::SO_fnoise1( FLOATVAL v, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( v )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( v );
	Result.SetValue( i, ( m_noise.FGNoise1( FLOAT( v ) ) + 1 ) / 2.0f );
	END_FORR
}

//----------------------------------------------------------------------
// noise(u,v)
STD_SOIMPL CqShaderExecEnv::SO_fnoise2( FLOATVAL u, FLOATVAL v, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( u )
	CHECKVARY( v )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( u );
	GETFLOAT( v );
	Result.SetValue( i, ( m_noise.FGNoise2( FLOAT( u ), FLOAT( v ) ) + 1 ) / 2.0f );
	END_FORR
}

//----------------------------------------------------------------------
// noise(p)
STD_SOIMPL CqShaderExecEnv::SO_fnoise3( POINTVAL p, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( Result )
	FOR_EACHR
	GETPOINT( p );
	Result.SetValue( i, ( m_noise.FGNoise3( POINT( p ) ) + 1 ) / 2.0f );
	END_FORR
}

//----------------------------------------------------------------------
// noise(p,t)
STD_SOIMPL CqShaderExecEnv::SO_fnoise4( POINTVAL p, FLOATVAL t, DEFPARAMIMPL )
{
	// TODO: Do proper 4D noise.
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( t )
	CHECKVARY( Result )
	FOR_EACHR
	GETPOINT( p );
	GETFLOAT( t );
	Result.SetValue( i, ( m_noise.FGNoise3( POINT( p ) ) + 1 ) / 2.0f );
	END_FORR
}

//----------------------------------------------------------------------
// noise(v)
STD_SOIMPL	CqShaderExecEnv::SO_cnoise1( FLOATVAL v, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( v )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( v );
	Result.SetValue( i, ( m_noise.CGNoise1( FLOAT( v ) ) + 1 ) / 2.0f );
	END_FORR
}

//----------------------------------------------------------------------
// noise(u,v)
STD_SOIMPL CqShaderExecEnv::SO_cnoise2( FLOATVAL u, FLOATVAL v, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( u )
	CHECKVARY( v )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( u );
	GETFLOAT( v );
	Result.SetValue( i, ( m_noise.CGNoise2( FLOAT( u ), FLOAT( v ) ) + 1 ) / 2.0f );
	END_FORR
}

//----------------------------------------------------------------------
// noise(p)
STD_SOIMPL CqShaderExecEnv::SO_cnoise3( POINTVAL p, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( Result )
	FOR_EACHR
	GETPOINT( p );
	Result.SetValue( i, ( m_noise.CGNoise3( POINT( p ) ) + 1 ) / 2.0f );
	END_FORR
}

//----------------------------------------------------------------------
// noise(p,t)
STD_SOIMPL CqShaderExecEnv::SO_cnoise4( POINTVAL p, FLOATVAL t, DEFPARAMIMPL )
{
	// TODO: Do proper 4D noise.
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( t )
	CHECKVARY( Result )
	FOR_EACHR
	GETPOINT( p );
	GETFLOAT( t );
	Result.SetValue( i, ( m_noise.CGNoise3( POINT( p ) ) + 1 ) / 2.0f );
	END_FORR
}

//----------------------------------------------------------------------
// noise(v)
STD_SOIMPL CqShaderExecEnv::SO_pnoise1( FLOATVAL v, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( v )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( v );
	Result.SetValue( i, ( m_noise.PGNoise1( FLOAT( v ) ) + 1 ) / 2.0f );
	END_FORR
}

//----------------------------------------------------------------------
// noise(u,v)
STD_SOIMPL CqShaderExecEnv::SO_pnoise2( FLOATVAL u, FLOATVAL v, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( u )
	CHECKVARY( v )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( u );
	GETFLOAT( v );
	Result.SetValue( i, ( m_noise.PGNoise2( FLOAT( u ), FLOAT( v ) ) + 1 ) / 2.0f );
	END_FORR
}

//----------------------------------------------------------------------
// noise(p)
STD_SOIMPL CqShaderExecEnv::SO_pnoise3( POINTVAL p, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( Result )
	FOR_EACHR
	GETPOINT( p );
	Result.SetValue( i, ( m_noise.PGNoise3( POINT( p ) ) + 1 ) / 2.0f );
	END_FORR
}

//----------------------------------------------------------------------
// noise(p,t)
STD_SOIMPL CqShaderExecEnv::SO_pnoise4( POINTVAL p, FLOATVAL t, DEFPARAMIMPL )
{
	// TODO: Do proper 4D noise.
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( t )
	CHECKVARY( Result )
	FOR_EACHR
	GETPOINT( p );
	GETFLOAT( t );
	Result.SetValue( i, ( m_noise.PGNoise3( POINT( p ) ) + 1 ) / 2.0f );
	END_FORR
}

//----------------------------------------------------------------------
// setcomp(c,i,v)
STD_SOIMPL	CqShaderExecEnv::SO_setcomp( COLORVAL p, FLOATVAL index, FLOATVAL v, DEFVOIDPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( v )
	CHECKVARY( index )
	FOR_EACHR
	GETCOLOR( p );
	GETFLOAT( index );
	GETFLOAT( v );
	COLOR( p )[ FLOAT( index ) ] = FLOAT( v );
	p.SetValue( i, COLOR( p ) );
	END_FORR
}

//----------------------------------------------------------------------
// setxcomp(p,v)
STD_SOIMPL	CqShaderExecEnv::SO_setxcomp( POINTVAL p, FLOATVAL v, DEFVOIDPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( v )
	FOR_EACHR
	GETPOINT( p );
	GETFLOAT( v );
	POINT( p ).x( FLOAT( v ) );
	p.SetValue( i, POINT( p ) );
	END_FORR
}

//----------------------------------------------------------------------
// setycomp(p,v)
STD_SOIMPL	CqShaderExecEnv::SO_setycomp( POINTVAL p, FLOATVAL v, DEFVOIDPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( v )
	FOR_EACHR
	GETPOINT( p );
	GETFLOAT( v );
	POINT( p ).y( FLOAT( v ) );
	p.SetValue( i, POINT( p ) );
	END_FORR
}

//----------------------------------------------------------------------
// setzcomp(p,v)
STD_SOIMPL	CqShaderExecEnv::SO_setzcomp( POINTVAL p, FLOATVAL v, DEFVOIDPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( v )
	FOR_EACHR
	GETPOINT( p );
	GETFLOAT( v );
	POINT( p ).z( FLOAT( v ) );
	p.SetValue( i, POINT( p ) );
	END_FORR
}



STD_SOIMPL	CqShaderExecEnv::SO_length( VECTORVAL V, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( V )
	CHECKVARY( Result )
	FOR_EACHR
	GETVECTOR( V );
	Result.SetValue( i, VECTOR( V ).Magnitude() );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_distance( POINTVAL P1, POINTVAL P2, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( P1 )
	CHECKVARY( P2 )
	CHECKVARY( Result )
	FOR_EACHR
	GETPOINT( P1 );
	GETPOINT( P2 );
	Result.SetValue( i, ( POINT( P1 ) - POINT( P2 ) ).Magnitude() );
	END_FORR
}


//----------------------------------------------------------------------
// area(P)
STD_SOIMPL CqShaderExecEnv::SO_area( POINTVAL p, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( Result )
	FOR_EACHR
	CqVector3D	vecR;
	if ( m_pSurface )
	{
		CqVMStackEntry SEdu, SEdv;
		du().GetValue( i, SEdu );
		dv().GetValue( i, SEdv );
		GETFLOAT( SEdu );
		GETFLOAT( SEdv );
		vecR = ( SO_DuType<CqVector3D>( p, m_GridI, *this ) * FLOAT( SEdu ) ) % 
			   ( SO_DvType<CqVector3D>( p, m_GridI, *this ) * FLOAT( SEdv ) );
		Result.SetValue( i, vecR.Magnitude() );
	}

	END_FORR
}


STD_SOIMPL	CqShaderExecEnv::SO_normalize( VECTORVAL V, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( V )
	CHECKVARY( Result )
	FOR_EACHR
	GETVECTOR( V );
	VECTOR( V ).Unit();
	Result.SetValue( i, VECTOR( V ) );
	END_FORR
}


//----------------------------------------------------------------------
// faceforward(N,I,[Nref])
STD_SOIMPL CqShaderExecEnv::SO_faceforward( NORMALVAL N, VECTORVAL I, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( N )
	CHECKVARY( I )
	CHECKVARY( Result )
	FOR_EACHR
	GETNORMAL( N );
	GETVECTOR( I );
	TqFloat s = ( ( ( -VECTOR( I ) ) * NORMAL( N ) ) < 0.0f ) ? -1.0f : 1.0f;
	Result.SetValue( i, NORMAL( N ) * s );
	END_FORR
}


//----------------------------------------------------------------------
// reflect(I,N)
STD_SOIMPL CqShaderExecEnv::SO_reflect( VECTORVAL I, NORMALVAL N, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( I )
	CHECKVARY( N )
	CHECKVARY( Result )
	FOR_EACHR
	GETVECTOR( I );
	GETNORMAL( N );
	TqFloat idn = 2.0f * ( VECTOR( I ) * NORMAL( N ) );
	CqVector3D res = VECTOR( I ) - ( idn * NORMAL( N ) );
	Result.SetValue( i, res );
	END_FORR
}


//----------------------------------------------------------------------
// reftact(I,N,eta)
STD_SOIMPL CqShaderExecEnv::SO_refract( VECTORVAL I, NORMALVAL N, FLOATVAL eta, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( I )
	CHECKVARY( N )
	CHECKVARY( eta )
	CHECKVARY( Result )
	FOR_EACHR
	GETVECTOR( I );
	GETNORMAL( N );
	GETFLOAT( eta );
	TqFloat IdotN = VECTOR( I ) * NORMAL( N );
	TqFloat feta = FLOAT( eta );
	TqFloat k = 1 - feta * feta * ( 1 - IdotN * IdotN );
	Result.SetValue( i, ( k < 0.0f ) ? CqVector3D( 0, 0, 0 ) : CqVector3D( feta * VECTOR( I ) - ( feta * IdotN + sqrt( k ) ) * NORMAL( N ) ) );
	END_FORR
}


//----------------------------------------------------------------------
// fresnel(I,N,eta,Kr,Kt)
#define SQR(A)	((A)*(A))
STD_SOIMPL CqShaderExecEnv::SO_fresnel( VECTORVAL I, NORMALVAL N, FLOATVAL eta, FLOATVAL Kr, FLOATVAL Kt, DEFVOIDPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( I )
	CHECKVARY( N )
	CHECKVARY( eta )
	CHECKVARY( Kr )
	CHECKVARY( Kt )
	FOR_EACHR
	GETVECTOR( I );
	GETNORMAL( N );
	GETFLOAT( eta );
	GETFLOAT( Kr );
	GETFLOAT( Kt );
	TqFloat cos_theta = -VECTOR( I ) * NORMAL( N );
	TqFloat fuvA = SQR( 1.0f / FLOAT( eta ) ) - ( 1.0f - SQR( cos_theta ) );
	TqFloat fuvB = sqrt( SQR( fuvA ) );
	TqFloat fu2 = ( fuvA + fuvB ) / 2;
	TqFloat fv2 = ( -fuvA + fuvB ) / 2;
	TqFloat fperp2 = ( SQR( cos_theta - sqrt( fu2 ) ) + fv2 ) / ( SQR( cos_theta + sqrt( fu2 ) ) + fv2 );
	TqFloat feta = FLOAT( eta );
	TqFloat fpara2 = ( SQR( SQR( 1.0f / feta ) * cos_theta - sqrt( fu2 ) ) + SQR( -sqrt( fv2 ) ) ) /
	                 ( SQR( SQR( 1.0f / feta ) * cos_theta + sqrt( fu2 ) ) + SQR( sqrt( fv2 ) ) );

	Kr.SetValue( i, 0.5f * ( fperp2 + fpara2 ) );
	Kt.SetValue( i, 1.0f - FLOAT( Kr ) );
	END_FORR
}

//----------------------------------------------------------------------
// fresnel(I,N,eta,Kr,Kt,R,T)
STD_SOIMPL CqShaderExecEnv::SO_fresnel( VECTORVAL I, NORMALVAL N, FLOATVAL eta, FLOATVAL Kr, FLOATVAL Kt, VECTORVAL R, VECTORVAL T, DEFVOIDPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( I )
	CHECKVARY( N )
	CHECKVARY( eta )
	CHECKVARY( Kr )
	CHECKVARY( Kt )
	CHECKVARY( R )
	CHECKVARY( T )
	FOR_EACHR
	GETVECTOR( I );
	GETNORMAL( N );
	GETFLOAT( eta );
	GETFLOAT( Kr );
	GETFLOAT( Kt );
	GETVECTOR( R );
	GETVECTOR( T );
	TqFloat cos_theta = -VECTOR( I ) * NORMAL( N );
	TqFloat fuvA = SQR( 1.0f / FLOAT( eta ) ) - ( 1.0f - SQR( cos_theta ) );
	TqFloat fuvB = sqrt( SQR( fuvA ) );
	TqFloat fu2 = ( fuvA + fuvB ) / 2;
	TqFloat fv2 = ( -fuvA + fuvB ) / 2;
	TqFloat feta = FLOAT( eta );
	TqFloat fperp2 = ( SQR( cos_theta - sqrt( fu2 ) ) + fv2 ) / ( SQR( cos_theta + sqrt( fu2 ) ) + fv2 );
	TqFloat fpara2 = ( SQR( SQR( 1.0f / feta ) * cos_theta - sqrt( fu2 ) ) + SQR( -sqrt( fv2 ) ) ) /
	                 ( SQR( SQR( 1.0f / feta ) * cos_theta + sqrt( fu2 ) ) + SQR( sqrt( fv2 ) ) );
	Kr.SetValue( i, 0.5f * ( fperp2 + fpara2 ) );
	Kt.SetValue( i, 1.0f - FLOAT( Kr ) );
	END_FORR
	SO_reflect( I, N, R );
	SO_refract( I, N, eta, T );
}


//----------------------------------------------------------------------
// transform(s,s,P)
STD_SOIMPL CqShaderExecEnv::SO_transform( STRINGVAL fromspace, STRINGVAL tospace, POINTVAL p, DEFPARAMIMPL )
{
	assert( pShader != 0 );

	TqInt i = 0;
	GETSTRING( fromspace );
	GETSTRING( tospace );
	const CqMatrix& mat = QGetRenderContext() ->matSpaceToSpace( STRING( fromspace ).c_str(), STRING( tospace ).c_str(), pShader->matCurrent(), matObjectToWorld() );
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( Result )
	FOR_EACHR
	GETPOINT( p );
	Result.SetValue( i, mat * POINT( p ) );
	END_FORR
}


//----------------------------------------------------------------------
// transform(s,P)
STD_SOIMPL CqShaderExecEnv::SO_transform( STRINGVAL tospace, POINTVAL p, DEFPARAMIMPL )
{
	assert( pShader != 0 );

	TqInt i = 0;
	GETSTRING( tospace );
	const CqMatrix& mat = QGetRenderContext() ->matSpaceToSpace( "current", STRING( tospace ).c_str(), pShader->matCurrent(), matObjectToWorld() );
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( Result )
	FOR_EACHR
	GETPOINT( p );
	Result.SetValue( i, mat * POINT( p ) );
	END_FORR
}


//----------------------------------------------------------------------
// transform(m,P)
STD_SOIMPL CqShaderExecEnv::SO_transformm( MATRIXVAL tospace, POINTVAL p, DEFPARAMIMPL )
{
	assert( pShader != 0 );

	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( Result )
	FOR_EACHR
	GETMATRIX( tospace );
	GETPOINT( p );
	Result.SetValue( i, MATRIX( tospace ) * POINT( p ) );
	END_FORR
}


//----------------------------------------------------------------------
// vtransform(s,s,P)
STD_SOIMPL CqShaderExecEnv::SO_vtransform( STRINGVAL fromspace, STRINGVAL tospace, VECTORVAL p, DEFPARAMIMPL )
{
	assert( pShader != 0 );

	TqInt i = 0;
	GETSTRING( fromspace );
	GETSTRING( tospace );

	const CqMatrix& mat = QGetRenderContext() ->matVSpaceToSpace( STRING( fromspace ).c_str(), STRING( tospace ).c_str(), pShader->matCurrent(), matObjectToWorld() );
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( Result )
	FOR_EACHR
	GETVECTOR( p );
	Result.SetValue( i, mat * VECTOR( p ) );
	END_FORR
}


//----------------------------------------------------------------------
// vtransform(s,P)
STD_SOIMPL CqShaderExecEnv::SO_vtransform( STRINGVAL tospace, VECTORVAL p, DEFPARAMIMPL )
{
	assert( pShader != 0 );

	TqInt i = 0;
	GETSTRING( tospace );

	const CqMatrix& mat = QGetRenderContext() ->matVSpaceToSpace( "current", STRING( tospace ).c_str(), pShader->matCurrent(), matObjectToWorld() );
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( Result )
	FOR_EACHR
	GETVECTOR( p );
	Result.SetValue( i, mat * VECTOR( p ) );
	END_FORR
}


//----------------------------------------------------------------------
// vtransform(m,P)
STD_SOIMPL CqShaderExecEnv::SO_vtransformm( MATRIXVAL tospace, VECTORVAL p, DEFPARAMIMPL )
{
	assert( pShader != 0 );

	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( Result )
	FOR_EACHR
	GETMATRIX( tospace );
	GETVECTOR( p );
	Result.SetValue( i, MATRIX( tospace ) * VECTOR( p ) );
	END_FORR
}


//----------------------------------------------------------------------
// ntransform(s,s,P)
STD_SOIMPL CqShaderExecEnv::SO_ntransform( STRINGVAL fromspace, STRINGVAL tospace, NORMALVAL p, DEFPARAMIMPL )
{
	assert( pShader != 0 );

	TqInt i = 0;
	GETSTRING( fromspace );
	GETSTRING( tospace );

	const CqMatrix& mat = QGetRenderContext() ->matNSpaceToSpace( STRING( fromspace ).c_str(), STRING( tospace ).c_str(), pShader->matCurrent(), matObjectToWorld() );
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( Result )
	FOR_EACHR
	GETNORMAL( p );
	Result.SetValue( i, mat * NORMAL( p ) );
	END_FORR
}


//----------------------------------------------------------------------
// ntransform(s,P)
STD_SOIMPL CqShaderExecEnv::SO_ntransform( STRINGVAL tospace, NORMALVAL p, DEFPARAMIMPL )
{
	assert( pShader != 0 );

	TqInt i = 0;
	GETSTRING( tospace );

	const CqMatrix& mat = QGetRenderContext() ->matNSpaceToSpace( "current", STRING( tospace ).c_str(), pShader->matCurrent(), matObjectToWorld() );
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( Result )
	FOR_EACHR
	GETNORMAL( p );
	Result.SetValue( i, mat * NORMAL( p ) );
	END_FORR
}


//----------------------------------------------------------------------
// ntransform(m,P)
STD_SOIMPL CqShaderExecEnv::SO_ntransformm( MATRIXVAL tospace, NORMALVAL p, DEFPARAMIMPL )
{
	assert( pShader != 0 );

	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( Result )
	FOR_EACHR
	GETMATRIX( tospace );
	GETNORMAL( p );
	Result.SetValue( i, MATRIX( tospace ) * NORMAL( p ) );
	END_FORR
}


//----------------------------------------------------------------------
// depth(P)
STD_SOIMPL CqShaderExecEnv::SO_depth( POINTVAL p, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( Result )
	FOR_EACHR
	GETPOINT( p );
	TqFloat d = POINT( p ).z();
	d = ( d - QGetRenderContext() ->optCurrent().fClippingPlaneNear() ) /
	    ( QGetRenderContext() ->optCurrent().fClippingPlaneFar() - QGetRenderContext() ->optCurrent().fClippingPlaneNear() );
	Result.SetValue( i, d );
	END_FORR
}


//----------------------------------------------------------------------
// calculatenormal(P)
STD_SOIMPL CqShaderExecEnv::SO_calculatenormal( POINTVAL p, DEFPARAMIMPL )
{
	// Find out if the orientation is inverted.
	EqOrientation O = pSurface() ->pAttributes() ->eOrientation();
	float neg = 1;
	if ( O != OrientationLH ) neg = -1;

	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( Result )
	FOR_EACHR
	CqVector3D	dPdu = SO_DuType<CqVector3D>( p, m_GridI, *this );
	CqVector3D	dPdv = SO_DvType<CqVector3D>( p, m_GridI, *this );
	CqVector3D	N = dPdu % dPdv;
	N.Unit();
	N *= neg;
	Result.SetValue( i, N );
	END_FORR
}

STD_SOIMPL CqShaderExecEnv::SO_cmix( COLORVAL color0, COLORVAL color1, FLOATVAL value, DEFPARAMIMPL )
{
	//assert(value>=0.0 && value<=1.0);
	INIT_SOR
	CHECKVARY( color0 )
	CHECKVARY( color1 )
	CHECKVARY( value )
	CHECKVARY( Result )
	FOR_EACHR
	GETCOLOR( color0 );
	GETCOLOR( color1 );
	GETFLOAT( value );
	CqColor c( ( 1.0f - FLOAT( value ) ) * COLOR( color0 ) + FLOAT( value ) * COLOR( color1 ) );
	Result.SetValue( i, c );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_fmix( FLOATVAL f0, FLOATVAL f1, FLOATVAL value, DEFPARAMIMPL )
{
	//assert(value>=0.0 && value<=1.0);
	INIT_SOR
	CHECKVARY( f0 )
	CHECKVARY( f1 )
	CHECKVARY( value )
	FOR_EACHR
	GETFLOAT( f0 );
	GETFLOAT( f1 );
	GETFLOAT( value );
	TqFloat f( ( 1.0f - FLOAT( value ) ) * FLOAT( f0 ) + FLOAT( value ) * FLOAT( f1 ) );
	Result.SetValue( i, f );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_pmix( POINTVAL p0, POINTVAL p1, FLOATVAL value, DEFPARAMIMPL )
{
	//assert(value>=0.0 && value<=1.0);
	INIT_SOR
	CHECKVARY( p0 )
	CHECKVARY( p1 )
	CHECKVARY( value )
	FOR_EACHR
	GETPOINT( p0 );
	GETPOINT( p1 );
	GETFLOAT( value );
	CqVector3D p( ( 1.0f - FLOAT( value ) ) * POINT( p0 ) + FLOAT( value ) * POINT( p1 ) );
	Result.SetValue( i, p );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_vmix( VECTORVAL v0, VECTORVAL v1, FLOATVAL value, DEFPARAMIMPL )
{
	//assert(value>=0.0 && value<=1.0);
	INIT_SOR
	CHECKVARY( v0 )
	CHECKVARY( v1 )
	CHECKVARY( value )
	FOR_EACHR
	GETVECTOR( v0 );
	GETVECTOR( v1 );
	GETFLOAT( value );
	CqVector3D v( ( 1.0f - FLOAT( value ) ) * VECTOR( v0 ) + FLOAT( value ) * VECTOR( v1 ) );
	Result.SetValue( i, v );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_nmix( NORMALVAL n0, NORMALVAL n1, FLOATVAL value, DEFPARAMIMPL )
{
	//assert(value>=0.0 && value<=1.0);
	INIT_SOR
	CHECKVARY( n0 )
	CHECKVARY( n1 )
	CHECKVARY( value )
	FOR_EACHR
	GETNORMAL( n0 );
	GETNORMAL( n1 );
	GETFLOAT( value );
	CqVector3D n( ( 1.0f - FLOAT( value ) ) * NORMAL( n0 ) + FLOAT( value ) * NORMAL( n1 ) );
	Result.SetValue( i, n );
	END_FORR
}


//----------------------------------------------------------------------
// texture(S)
STD_SOIMPL CqShaderExecEnv::SO_ftexture1( STRINGVAL name, FLOATVAL channel, DEFPARAMVARIMPL )
{
	GET_TEXTURE_PARAMS;

	TqInt i = 0;
	GETSTRING( name );
	GETFLOAT( channel );
	CqTextureMap* pTMap = CqTextureMap::GetTextureMap( STRING( name ).c_str() );

	INIT_SOR
	__fVarying = TqTrue;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		CqVMStackEntry SEdu, SEdv;
		du().GetValue( 0, SEdu );
		dv().GetValue( 0, SEdv );
		TqFloat fdu = 0.0f, fdv = 0.0f;
		if( m_pSurface )	
		{
			SEdu.Value( fdu );
			SEdv.Value( fdv );
		}
		FOR_EACHR
		TqFloat swidth = 0.0f, twidth = 0.0f;
		if ( fdu != 0.0f && fdv != 0.0f )
		{
			TqFloat dsdu = SO_DuType<TqFloat>( dsdu, &s(), m_GridI, *this );
			swidth = fabs( dsdu * fdu );
			TqFloat dtdu = SO_DuType<TqFloat>( dsdu, &t(), m_GridI, *this );
			twidth = fabs( dtdu * fdu );

			TqFloat dsdv = SO_DvType<TqFloat>( dsdu, &s(), m_GridI, *this );
			swidth += fabs( dsdv * fdv );
			TqFloat dtdv = SO_DvType<TqFloat>( dsdu, &t(), m_GridI, *this );
			twidth += fabs( dtdv * fdv );
		}
		else
		{
			swidth = 1.0 / pTMap->XRes();
			twidth = 1.0 / pTMap->YRes();
		}

		swidth *= _pswidth;
		twidth *= _ptwidth;

		// Sample the texture.
		std::valarray<TqFloat> val;

		CqVMStackEntry SEs, SEt;
		s().GetValue( i, SEs );
		t().GetValue( i, SEt );
		GETFLOAT( SEs );
		GETFLOAT( SEt );
		pTMap->SampleMIPMAP( FLOAT( SEs ), FLOAT( SEt ), swidth, twidth, _psblur, _ptblur, val);

		// Grab the appropriate channel.
		float fchan = FLOAT( channel );
		if ( fchan >= val.size() )
			Result.SetValue( i, _pfill );
		else
			Result.SetValue( i, val[ static_cast<unsigned int>( fchan ) ] );
		END_FORR
	}
	else
		Result = 0.0f;
}

//----------------------------------------------------------------------
// texture(S,F,F)
STD_SOIMPL CqShaderExecEnv::SO_ftexture2( STRINGVAL name, FLOATVAL channel, FLOATVAL s, FLOATVAL t, DEFPARAMVARIMPL )
{
	GET_TEXTURE_PARAMS;

	TqInt i = 0;
	GETSTRING( name );
	GETFLOAT( channel );
	CqTextureMap* pTMap = CqTextureMap::GetTextureMap( STRING( name ).c_str() );

	INIT_SOR
	__fVarying = TqTrue;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		
		CqVMStackEntry SEdu, SEdv;
		du().GetValue( 0, SEdu );
		dv().GetValue( 0, SEdv );
		TqFloat fdu = 0.0f, fdv = 0.0f;
		if( m_pSurface )	
		{
			SEdu.Value( fdu );
			SEdv.Value( fdv );
		}
		FOR_EACHR
		TqFloat swidth = 0.0f, twidth = 0.0f;
		if ( fdu != 0.0f && fdv != 0.0f )
		{
			TqFloat dsdu = SO_DuType<TqFloat>( s, m_GridI, *this );
			swidth = fabs( dsdu * fdu );
			TqFloat dtdu = SO_DuType<TqFloat>( t, m_GridI, *this );
			twidth = fabs( dtdu * fdu );

			TqFloat dsdv = SO_DvType<TqFloat>( s, m_GridI, *this );
			swidth += fabs( dsdv * fdv );
			TqFloat dtdv = SO_DvType<TqFloat>( t, m_GridI, *this );
			twidth += fabs( dtdv * fdv );
		}
		else
		{
			swidth = 1.0 / pTMap->XRes();
			twidth = 1.0 / pTMap->YRes();
		}

		swidth *= _pswidth;
		twidth *= _ptwidth;

		// Sample the texture.
		std::valarray<TqFloat> val;

		GETFLOAT( s );
		GETFLOAT( t );
		pTMap->SampleMIPMAP( FLOAT( s ), FLOAT( t ), swidth, twidth, _psblur, _ptblur, val);

		// Grab the appropriate channel.
		float fchan = FLOAT( channel );
		if ( fchan >= val.size() )
			Result.SetValue( i, _pfill );
		else
			Result.SetValue( i, val[ static_cast<unsigned int>( fchan ) ] );
		END_FORR
	}
	else
		Result = 0.0f;
}

//----------------------------------------------------------------------
// texture(S,F,F,F,F,F,F,F,F)
STD_SOIMPL CqShaderExecEnv::SO_ftexture3( STRINGVAL name, FLOATVAL channel, FLOATVAL s1, FLOATVAL t1, FLOATVAL s2, FLOATVAL t2, FLOATVAL s3, FLOATVAL t3, FLOATVAL s4, FLOATVAL t4, DEFPARAMVARIMPL )
{
	GET_TEXTURE_PARAMS;

	TqInt i = 0;
	GETSTRING( name );
	GETFLOAT( channel );
	CqTextureMap* pTMap = CqTextureMap::GetTextureMap( STRING( name ).c_str() );
	INIT_SOR
	__fVarying = TqTrue;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		FOR_EACHR
		// Sample the texture.
		std::valarray<float> val;
		GETFLOAT( s1 );
		GETFLOAT( t1 );
		GETFLOAT( s2 );
		GETFLOAT( t2 );
		GETFLOAT( s3 );
		GETFLOAT( t3 );
		GETFLOAT( s4 );
		GETFLOAT( t4 );
		pTMap->SampleMIPMAP( FLOAT( s1 ), FLOAT( t1 ), FLOAT( s2 ), FLOAT( t2 ), FLOAT( s3 ), FLOAT( t3 ), FLOAT( s4 ), FLOAT( t4 ), _psblur, _ptblur, val );

		// Grab the appropriate channel.
		float fchan = FLOAT( channel );
		if ( fchan >= val.size() )
			Result.SetValue( i, _pfill );
		else
			Result.SetValue( i, val[ static_cast<unsigned int>( fchan ) ] );
		END_FORR
	}
	else
		Result = 0.0f;
}

//----------------------------------------------------------------------
// texture(S)
STD_SOIMPL CqShaderExecEnv::SO_ctexture1( STRINGVAL name, FLOATVAL channel, DEFPARAMVARIMPL )
{
	GET_TEXTURE_PARAMS;

	TqInt i = 0;
	GETSTRING( name );
	GETFLOAT( channel );
	CqTextureMap* pTMap = CqTextureMap::GetTextureMap( STRING( name ).c_str() );

	INIT_SOR
	__fVarying = TqTrue;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		CqVMStackEntry SEdu, SEdv;
		du().GetValue( 0, SEdu );
		dv().GetValue( 0, SEdv );
		TqFloat fdu = 0.0f, fdv = 0.0f;
		if( m_pSurface )	
		{
			SEdu.Value( fdu );
			SEdv.Value( fdv );
		}
		FOR_EACHR
		TqFloat swidth = 0.0f, twidth = 0.0f;
		if ( fdu != 0.0f && fdv != 0.0f )
		{
			TqFloat dsdu = SO_DuType<TqFloat>( dsdu, &s(), m_GridI, *this );
			swidth = fabs( dsdu * fdu );
			TqFloat dsdv = SO_DvType<TqFloat>( dsdu, &s(), m_GridI, *this );
			swidth += fabs( dsdv * fdv );

			TqFloat dtdu = SO_DuType<TqFloat>( dsdu, &t(), m_GridI, *this );
			twidth = fabs( dtdu * fdu );
			TqFloat dtdv = SO_DvType<TqFloat>( dsdu, &t(), m_GridI, *this );
			twidth += fabs( dtdv * fdv );
		}
		else
		{
			swidth = 1.0 / pTMap->XRes();
			twidth = 1.0 / pTMap->YRes();
		}

		swidth *= _pswidth;
		twidth *= _ptwidth;

		// Sample the texture.
		std::valarray<TqFloat> val;


		CqVMStackEntry SEs, SEt;
		s().GetValue( i, SEs );
		t().GetValue( i, SEt );
		GETFLOAT( SEs );
		GETFLOAT( SEt );
		pTMap->SampleMIPMAP( FLOAT( SEs ), FLOAT( SEt ), swidth, twidth, _psblur, _ptblur, val);

		// Grab the appropriate channel.
		float fchan = FLOAT( channel );
		if ( fchan + 2 >= val.size() )
			Result.SetValue( i, CqColor( _pfill, _pfill, _pfill ) );
		else
			Result.SetValue( i, CqColor( val[ static_cast<unsigned int>( fchan ) ], val[ static_cast<unsigned int>( fchan ) + 1 ], val[ static_cast<unsigned int>( fchan ) + 2 ] ) );
		END_FORR
	}
	else
		Result = CqColor( 0, 0, 0 );
}

//----------------------------------------------------------------------
// texture(S,F,F)
STD_SOIMPL CqShaderExecEnv::SO_ctexture2( STRINGVAL name, FLOATVAL channel, FLOATVAL s, FLOATVAL t, DEFPARAMVARIMPL )
{
	GET_TEXTURE_PARAMS;

	TqInt i = 0;
	GETSTRING( name );
	GETFLOAT( channel );
	CqTextureMap* pTMap = CqTextureMap::GetTextureMap( STRING( name ).c_str() );

	INIT_SOR
	__fVarying = TqTrue;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		CqVMStackEntry SEdu, SEdv;
		du().GetValue( 0, SEdu );
		dv().GetValue( 0, SEdv );
		TqFloat fdu = 0.0f, fdv = 0.0f;
		if( m_pSurface )	
		{
			SEdu.Value( fdu );
			SEdv.Value( fdv );
		}
		FOR_EACHR
		TqFloat swidth = 0.0f, twidth = 0.0f;
		if ( fdu != 0.0f && fdv != 0.0f )
		{
			TqFloat dsdu = SO_DuType<TqFloat>( s, m_GridI, *this );
			swidth = fabs( dsdu * fdu );
			TqFloat dsdv = SO_DvType<TqFloat>( s, m_GridI, *this );
			swidth += fabs( dsdv * fdv );

			TqFloat dtdu = SO_DuType<TqFloat>( t, m_GridI, *this );
			twidth = fabs( dtdu * fdu );
			TqFloat dtdv = SO_DvType<TqFloat>( t, m_GridI, *this );
			twidth += fabs( dtdv * fdv );
		}
		else
		{
			swidth = 1.0 / pTMap->XRes();
			twidth = 1.0 / pTMap->YRes();
		}

		swidth *= _pswidth;
		twidth *= _ptwidth;

		// Sample the texture.
		std::valarray<TqFloat> val;

		GETFLOAT( s );
		GETFLOAT( t );
		pTMap->SampleMIPMAP( FLOAT( s ), FLOAT( t ), swidth, twidth, _psblur, _ptblur, val);

		// Grab the appropriate channel.
		float fchan = FLOAT( channel );
		if ( fchan + 2 >= val.size() )
			Result.SetValue( i, CqColor( _pfill, _pfill, _pfill ) );
		else
			Result.SetValue( i, CqColor( val[ static_cast<unsigned int>( fchan ) ], val[ static_cast<unsigned int>( fchan ) + 1 ], val[ static_cast<unsigned int>( fchan ) + 2 ] ) );
		END_FORR
	}
	else
		Result = CqColor( 0, 0, 0 );
}

//----------------------------------------------------------------------
// texture(S,F,F,F,F,F,F,F,F)
STD_SOIMPL CqShaderExecEnv::SO_ctexture3( STRINGVAL name, FLOATVAL channel, FLOATVAL s1, FLOATVAL t1, FLOATVAL s2, FLOATVAL t2, FLOATVAL s3, FLOATVAL t3, FLOATVAL s4, FLOATVAL t4, DEFPARAMVARIMPL )
{
	GET_TEXTURE_PARAMS;

	TqInt i = 0;
	GETSTRING( name );
	GETFLOAT( channel );
	CqTextureMap* pTMap = CqTextureMap::GetTextureMap( STRING( name ).c_str() );
	INIT_SOR
	__fVarying = TqTrue;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		FOR_EACHR
		// Sample the texture.
		std::valarray<float> val;
		GETFLOAT( s1 );
		GETFLOAT( t1 );
		GETFLOAT( s2 );
		GETFLOAT( t2 );
		GETFLOAT( s3 );
		GETFLOAT( t3 );
		GETFLOAT( s4 );
		GETFLOAT( t4 );
		pTMap->SampleMIPMAP( FLOAT( s1 ), FLOAT( t1 ), FLOAT( s2 ), FLOAT( t2 ), FLOAT( s3 ), FLOAT( t3 ), FLOAT( s4 ), FLOAT( t4 ), _psblur, _ptblur, val );

		// Grab the appropriate channel.
		float fchan = FLOAT( channel );
		if ( fchan + 2 >= val.size() )
			Result.SetValue( i, CqColor( _pfill, _pfill, _pfill ) );
		else
			Result.SetValue( i, CqColor( val[ static_cast<unsigned int>( fchan ) ], val[ static_cast<unsigned int>( fchan ) + 1 ], val[ static_cast<unsigned int>( fchan ) + 2 ] ) );
		END_FORR
	}
	else
		Result = CqColor( 0, 0, 0 );
}


//----------------------------------------------------------------------
// environment(S,P)
STD_SOIMPL CqShaderExecEnv::SO_fenvironment2( STRINGVAL name, FLOATVAL channel, VECTORVAL R, DEFPARAMVARIMPL )
{
	GET_TEXTURE_PARAMS;

	TqInt i = 0;
	CqTextureMap* pTMap;
	GETSTRING( name );
	GETFLOAT( channel );
	pTMap = CqTextureMap::GetEnvironmentMap( STRING( name ).c_str() );

	// Try with LatLong map file
	if ( pTMap == 0 )
	{
		pTMap = CqTextureMap::GetLatLongMap( STRING( name ).c_str() );
	}

	INIT_SOR
	__fVarying = TqTrue;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		CqVMStackEntry SEdu, SEdv;
		du().GetValue( 0, SEdu );
		dv().GetValue( 0, SEdv );
		TqFloat fdu = 0.0f, fdv = 0.0f;
		if( m_pSurface )
		{
			SEdu.Value( fdu );
			SEdv.Value( fdv );
		}
		FOR_EACHR
		CqVector3D swidth = 0.0f, twidth = 0.0f;
		if ( fdu != 0.0f )
		{
			CqVector3D dRdu = SO_DuType<CqVector3D>( R, m_GridI, *this );
			swidth = dRdu * fdu;
		}
		if ( fdv != 0.0f )
		{
			CqVector3D dRdv = SO_DvType<CqVector3D>( R, m_GridI, *this );
			twidth = dRdv * fdv;
		}
		else
		{
			swidth = 1.0 / pTMap->XRes();
			twidth = 1.0 / pTMap->YRes();
		}

		swidth *= _pswidth;
		twidth *= _ptwidth;


		// Sample the texture.
		std::valarray<TqFloat> val;
		GETVECTOR( R );
		pTMap->SampleMIPMAP( VECTOR( R ), swidth, twidth, _psblur, _ptblur, val );

		// Grab the appropriate channel.
		float fchan = FLOAT( channel );
		if ( fchan >= val.size() )
			Result.SetValue( i, _pfill );
		else
			Result.SetValue( i, val[ static_cast<unsigned int>( fchan ) ] );
		END_FORR
	}
	else
		Result = 0.0f;
}

//----------------------------------------------------------------------
// environment(S,P,P,P,P)
STD_SOIMPL CqShaderExecEnv::SO_fenvironment3( STRINGVAL name, FLOATVAL channel, VECTORVAL R1, VECTORVAL R2, VECTORVAL R3, VECTORVAL R4, DEFPARAMVARIMPL )
{
	GET_TEXTURE_PARAMS;

	TqInt i = 0;
	CqTextureMap* pTMap;
	GETSTRING( name );
	GETFLOAT( channel );
	pTMap = CqTextureMap::GetEnvironmentMap( STRING( name ).c_str() );

	// Try with LatLong map file
	if ( pTMap == 0 )
	{
		pTMap = CqTextureMap::GetLatLongMap( STRING( name ).c_str() );
	}

	INIT_SOR
	__fVarying = TqTrue;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		FOR_EACHR
		// Sample the texture.
		std::valarray<float> val;
		GETVECTOR( R1 );
		GETVECTOR( R2 );
		GETVECTOR( R3 );
		GETVECTOR( R4 );
		pTMap->SampleMIPMAP( VECTOR( R1 ), VECTOR( R2 ), VECTOR( R3 ), VECTOR( R4 ), _psblur, _ptblur, val );

		// Grab the appropriate channel.
		float fchan = FLOAT( channel );
		if ( fchan >= val.size() )
			Result.SetValue( i, _pfill );
		else
			Result.SetValue( i, val[ static_cast<unsigned int>( fchan ) ] );
		END_FORR
	}
	else
		Result = 0.0f;
}


//----------------------------------------------------------------------
// environment(S,P)
STD_SOIMPL CqShaderExecEnv::SO_cenvironment2( STRINGVAL name, FLOATVAL channel, VECTORVAL R, DEFPARAMVARIMPL )
{
	GET_TEXTURE_PARAMS;

	TqInt i = 0;
	CqTextureMap* pTMap = NULL;
	GETSTRING( name );
	GETFLOAT( channel );
	pTMap = CqTextureMap::GetEnvironmentMap( STRING( name ).c_str() );

	// Try with LatLong map file
	if ( pTMap == 0 )
	{
		pTMap = CqTextureMap::GetLatLongMap( STRING( name ).c_str() );
	}

	INIT_SOR
	__fVarying = TqTrue;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		CqVMStackEntry SEdu, SEdv;
		du().GetValue( 0, SEdu );
		dv().GetValue( 0, SEdv );
		TqFloat fdu = 0.0f, fdv = 0.0f;
		if( m_pSurface )
		{
			SEdu.Value( fdu );
			SEdv.Value( fdv );
		}
		FOR_EACHR
		CqVector3D swidth = 0.0f, twidth = 0.0f;
		if ( fdu != 0.0f )
		{
			CqVector3D dRdu = SO_DuType<CqVector3D>( R, m_GridI, *this );
			swidth = dRdu * fdu;
		}
		if ( fdv != 0.0f )
		{
			CqVector3D dRdv = SO_DvType<CqVector3D>( R, m_GridI, *this );
			twidth = dRdv * fdv;
		}
		else
		{
			swidth = 1.0 / pTMap->XRes();
			twidth = 1.0 / pTMap->YRes();
		}

		swidth *= _pswidth;
		twidth *= _ptwidth;


		// Sample the texture.
		std::valarray<TqFloat> val;
		GETVECTOR( R );
		pTMap->SampleMIPMAP( VECTOR( R ), swidth, twidth, _psblur, _ptblur, val );


		// Grab the appropriate channel.
		float fchan = FLOAT( channel );
		if ( fchan + 2 >= val.size() )
			Result.SetValue( i, CqColor( _pfill, _pfill, _pfill ) );
		else
			Result.SetValue( i, CqColor( val[ static_cast<unsigned int>( fchan ) ], val[ static_cast<unsigned int>( fchan ) + 1 ], val[ static_cast<unsigned int>( fchan ) + 2 ] ) );
		END_FORR
	}
	else
		Result = CqColor( 1, 1, 0 );
}

//----------------------------------------------------------------------
// environment(S,P,P,P,P)
STD_SOIMPL CqShaderExecEnv::SO_cenvironment3( STRINGVAL name, FLOATVAL channel, VECTORVAL R1, VECTORVAL R2, VECTORVAL R3, VECTORVAL R4, DEFPARAMVARIMPL )
{
	GET_TEXTURE_PARAMS;

	TqInt i = 0;
	CqTextureMap* pTMap;
	GETSTRING( name );
	GETFLOAT( channel );
	pTMap = CqTextureMap::GetEnvironmentMap( STRING( name ).c_str() );

	// Try with LatLong map file
	if ( pTMap == 0 )
	{
		pTMap = CqTextureMap::GetLatLongMap( STRING( name ).c_str() );
	}

	INIT_SOR
	__fVarying = TqTrue;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		FOR_EACHR
		// Sample the texture.
		// TODO: need to get and pass width,blur etc. values.
		std::valarray<float> val;
		GETVECTOR( R1 );
		GETVECTOR( R2 );
		GETVECTOR( R3 );
		GETVECTOR( R4 );
		pTMap->SampleMIPMAP( VECTOR( R1 ), VECTOR( R2 ), VECTOR( R3 ), VECTOR( R4 ), _psblur, _ptblur, val );

		// Grab the appropriate channel.
		float fchan = FLOAT( channel );
		if ( fchan + 2 >= val.size() )
			Result.SetValue( i, CqColor( _pfill, _pfill, _pfill ) );
		else
			Result.SetValue( i, CqColor( val[ static_cast<unsigned int>( fchan ) ], val[ static_cast<unsigned int>( fchan ) + 1 ], val[ static_cast<unsigned int>( fchan ) + 2 ] ) );
		END_FORR
	}
	else
		Result = CqColor( 0, 0, 0 );
}

//----------------------------------------------------------------------
// bump(S)
STD_SOIMPL CqShaderExecEnv::SO_bump1( STRINGVAL name, FLOATVAL channel, DEFPARAMVARIMPL )
{
	INIT_SOR
	__fVarying = TqTrue;
	FOR_EACHR
	Result.SetValue( i, CqVector3D( 0, 0, 0 ) );
	END_FORR
}

//----------------------------------------------------------------------
// bump(S,F,F)
STD_SOIMPL CqShaderExecEnv::SO_bump2( STRINGVAL name, FLOATVAL channel, FLOATVAL s, FLOATVAL t, DEFPARAMVARIMPL )
{
	INIT_SOR
	__fVarying = TqTrue;
	FOR_EACHR
	Result.SetValue( i, CqVector3D( 0, 0, 0 ) );
	END_FORR
}

//----------------------------------------------------------------------
// bump(S,F,F,F,F,F,F,F,F)
STD_SOIMPL CqShaderExecEnv::SO_bump3( STRINGVAL name, FLOATVAL channel, FLOATVAL s1, FLOATVAL t1, FLOATVAL s2, FLOATVAL t2, FLOATVAL s3, FLOATVAL t3, FLOATVAL s4, FLOATVAL t4, DEFPARAMVARIMPL )
{
	INIT_SOR
	__fVarying = TqTrue;
	FOR_EACHR
	Result.SetValue( i, CqVector3D( 0, 0, 0 ) );
	END_FORR
}

//----------------------------------------------------------------------
// shadow(S,P)
STD_SOIMPL CqShaderExecEnv::SO_shadow( STRINGVAL name, FLOATVAL channel, POINTVAL P, DEFPARAMVARIMPL )
{
	static CqVMStackEntry den( 1.0f );

	GET_TEXTURE_PARAMS;

	TqInt i = 0;

	GETSTRING( name );
	GETFLOAT( channel );
	CqShadowMap* pMap = static_cast<CqShadowMap*>( CqShadowMap::GetShadowMap( STRING( name ).c_str() ) );
	INIT_SOR
	__fVarying = TqTrue;
	if ( pMap != 0 && pMap->IsValid() )
	{
		FOR_EACHR
		CqVector3D swidth = 0.0f, twidth = 0.0f;

		swidth = SO_DerivType<CqVector3D>( P, den, m_GridI, *this );
		twidth = SO_DerivType<CqVector3D>( P, den, m_GridI, *this );

		swidth *= _pswidth;
		twidth *= _ptwidth;

		float fv;
		GETPOINT( P );
		pMap->SampleMap( POINT( P ), swidth, twidth, _psblur, _ptblur, fv );
		Result.SetValue( i, fv );
		END_FORR
	}
	else
		Result.SetValue( i, 0.0f );	// Default, completely lit
}

//----------------------------------------------------------------------
// shadow(S,P,P,P,P)

STD_SOIMPL CqShaderExecEnv::SO_shadow1( STRINGVAL name, FLOATVAL channel, POINTVAL P1, POINTVAL P2, POINTVAL P3, POINTVAL P4, DEFPARAMVARIMPL )
{
	GET_TEXTURE_PARAMS;

	TqInt i = 0;
	GETSTRING( name );
	GETFLOAT( channel );
	CqShadowMap* pMap = static_cast<CqShadowMap*>( CqShadowMap::GetShadowMap( STRING( name ).c_str() ) );
	INIT_SOR
	__fVarying = TqTrue;
	if ( pMap != 0 && pMap->IsValid() )
	{
		FOR_EACHR
		float fv;
		GETPOINT( P1 );
		GETPOINT( P2 );
		GETPOINT( P3 );
		GETPOINT( P4 );
		pMap->SampleMap( POINT( P1 ), POINT( P2 ), POINT( P3 ), POINT( P4 ), _psblur, _ptblur, fv );
		Result.SetValue( i, fv );
		END_FORR
	}
	else
		Result.SetValue( i, 0.0f );	// Default, completely lit
}


//----------------------------------------------------------------------
// ambient()

STD_SOIMPL CqShaderExecEnv::SO_ambient( DEFPARAMIMPL )
{
	static CqVMStackEntry Point;
	static CqColor colTemp;

	TqInt i = 0;
	Result = CqColor( 0,0,0 );

	// Use the lightsource stack on the current surface
	if ( m_pSurface != 0 )
	{
		// If this is the first call to illuminance this time round, call all lights and setup the Cl and L caches.
		if ( !m_IlluminanceCacheValid )
		{
			ValidateIlluminanceCache( Point, pShader );
		}

		for ( TqUint light_index = 0; light_index < m_pSurface->pAttributes() ->apLights().size(); light_index++ )
		{
			INIT_SOR
			__fVarying = TqTrue;

			CqLightsource* lp = m_pSurface->pAttributes() ->apLights() [ light_index ];
			if ( lp->pShader() ->fAmbient() )
			{
				FOR_EACHR
				// Get the color from the lightsource.
				CqVMStackEntry SECl;
				lp->Cl().GetValue( i, SECl );
				GETCOLOR( SECl );

				/// Set the light color in this surface shader /note not sure this is necessary.
				//Cl().SetValue( i, CqVMStackEntry( COLOR( SECl ) ) );

				// Now Combine the color of all ambient lightsources.
				GETCOLOR( Result );
				Result.SetValue( i, COLOR( Result ) + COLOR( SECl ) );

				END_FORR
			}
		}
	}
}


//----------------------------------------------------------------------
// diffuse(N)
STD_SOIMPL CqShaderExecEnv::SO_diffuse( NORMALVAL N, DEFPARAMIMPL )
{
	static CqVMStackEntry Tempangle;
	static CqVMStackEntry Tempnsamples;
	static CqVMStackEntry Point;
	CqColor colTemp;

	Point = &P();
	// If the illuminance cache is already OK, then we don't need to bother filling in the illuminance parameters.
	if ( !m_IlluminanceCacheValid )
	{
		ValidateIlluminanceCache( Point, pShader );
	}

	Tempangle = RI_PIO2;
	Tempnsamples = 0.0f;

	// Setup the return value.
	INIT_SOR
	Result = CqColor( 0, 0, 0 );
	__fVarying = TqTrue;

	// SO_init_illuminance returns TRUE if there are any non ambient ligthsources available.
	if ( SO_init_illuminance() )
	{
		do
		{
			// SO_illuminance sets the current state to whether the lightsource illuminates the points or not.
			SO_illuminance( Point, N, Tempangle, Tempnsamples );
			
			PushState();
			GetCurrentState();
			
			FOR_EACHR
			
			// Get the light vector and color from the lightsource.
			CqVMStackEntry SEL, SECl;
			L().GetValue( i, SEL );
			Cl().GetValue( i, SECl );
			GETVECTOR( SEL );
			GETCOLOR( SECl );
			VECTOR( SEL ).Unit();

			// Combine the light color into the result
			GETCOLOR( Result );
			GETNORMAL( N );
			Result.SetValue( i, COLOR( Result ) + COLOR( SECl ) * ( NORMAL ( SEL ) * NORMAL( N ) ) );
			
			END_FORR
			PopState();
			// SO_advance_illuminance returns TRUE if there are any more non ambient lightsources.
		}
		while ( SO_advance_illuminance() );
	}
	else
	{
		INIT_SOR
		__fVarying = TqTrue;
		FOR_EACHR
		Result.SetValue( i, CqColor( 0, 0, 0 ) );
		END_FORR
	}
}


//----------------------------------------------------------------------
// specular(N,V,roughness)
STD_SOIMPL CqShaderExecEnv::SO_specular( NORMALVAL N, VECTORVAL V, FLOATVAL roughness, DEFPARAMIMPL )
{
	static CqVMStackEntry Tempangle;
	static CqVMStackEntry Tempnsamples;
	static CqVMStackEntry Point;
	CqColor colTemp;

	Point = &P();
	// If the illuminance cache is already OK, then we don't need to bother filling in the illuminance parameters.
	if ( !m_IlluminanceCacheValid )
	{
		ValidateIlluminanceCache( Point, pShader );
	}

	Tempangle = RI_PIO2;
	Tempnsamples = 0.0f;

	// Setup the return value.
	INIT_SOR
	Result = CqColor( 0, 0, 0 );
	__fVarying = TqTrue;

	// SO_init_illuminance returns TRUE if there are any non ambient ligthsources available.
	if ( SO_init_illuminance() )
	{
		do
		{
			// SO_illuminance sets the current state to whether the lightsource illuminates the points or not.
			SO_illuminance( Point, N, Tempangle, Tempnsamples );

			PushState();
			GetCurrentState();
			FOR_EACHR

			// Get the ligth vector and color from the lightsource
			CqVMStackEntry SEL, SECl;
			L().GetValue( i, SEL );
			Cl().GetValue( i, SECl );
			GETVECTOR( SEL );
			GETCOLOR( SECl );
			VECTOR( SEL ).Unit();
			GETVECTOR( V ); 
			CqVector3D	H = VECTOR( SEL ) + VECTOR( V );
			H.Unit();
			
			// Combine the color into the result.
			/// \note The (roughness/8) term emulates the BMRT behaviour for prmanspecular.
			GETCOLOR( Result );
			GETNORMAL( N );
			GETFLOAT( roughness );
			Result.SetValue( i, COLOR( Result ) + COLOR( SECl ) * pow( MAX( 0.0f, NORMAL( N ) * H ), 1.0f / ( FLOAT( roughness ) / 8.0f ) ) );

			END_FORR
			PopState();
			// SO_advance_illuminance returns TRUE if there are any more non ambient lightsources.
		}
		while ( SO_advance_illuminance() );
	}
	else
	{
		INIT_SOR
		__fVarying = TqTrue;
		FOR_EACHR
		Result.SetValue( i, CqColor( 0, 0, 0 ) );
		END_FORR
	}
}


//----------------------------------------------------------------------
// phong(N,V,size)
STD_SOIMPL CqShaderExecEnv::SO_phong( NORMALVAL N, VECTORVAL V, FLOATVAL size, DEFPARAMIMPL )
{
	static CqVMStackEntry nV;
	static CqVMStackEntry nN;
	static CqVMStackEntry R;

	static CqVMStackEntry Tempangle;
	static CqVMStackEntry Tempnsamples;
	static CqVMStackEntry Point;
	CqColor colTemp;

	SO_normalize( V, nV );
	SO_normalize( N, nN );

	INIT_SOR
	__fVarying = TqTrue;
	{
		FOR_EACHR
		GETVECTOR( nV );
		nV.SetValue( i, -VECTOR( nV ) );
		END_FORR
	}
	SO_reflect( nV, nN, R );

	Point = &P();
	// If the illuminance cache is already OK, then we don't need to bother filling in the illuminance parameters.
	if ( !m_IlluminanceCacheValid )
	{
		ValidateIlluminanceCache( Point, pShader );
	}

	Tempangle = RI_PIO2;
	Tempnsamples = 0.0f;

	// Initialise the return value
	Result = CqColor( 0, 0, 0 );

	// SO_init_illuminance returns TRUE if there are any non ambient ligthsources available.
	if ( SO_init_illuminance() )
	{
		do
		{
			// SO_illuminance sets the current state to whether the lightsource illuminates the points or not.
			SO_illuminance( Point, N, Tempangle, Tempnsamples );
			
			PushState();
			GetCurrentState();
			
			FOR_EACHR
			
			// Get the light vector and color from the loght source.
			CqVMStackEntry SEL, SECl;
			L().GetValue( i, SEL );
			Cl().GetValue( i, SECl );
			GETVECTOR( SEL );
			GETCOLOR( SECl );
			VECTOR( SEL ).Unit();	

			// Now combine the color into the result.
			GETCOLOR( Result );
			GETVECTOR( R );
			GETFLOAT( size );
			Result.SetValue( i, COLOR( Result ) + COLOR( SECl ) * pow( MAX( 0.0f, VECTOR( R ) * VECTOR( SEL ) ), FLOAT( size ) ) );

			END_FORR

			PopState();
			// SO_advance_illuminance returns TRUE if there are any more non ambient lightsources.
		}
		while ( SO_advance_illuminance() );
	}
	else
	{
		INIT_SOR
		__fVarying = TqTrue;
		FOR_EACHR
		Result.SetValue( i, CqColor( 0, 0, 0 ) );
		END_FORR
	}
}


//----------------------------------------------------------------------
// trace(P,R)
STD_SOIMPL CqShaderExecEnv::SO_trace( POINTVAL P, VECTORVAL R, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( P )
	CHECKVARY( R )
	CHECKVARY( Result )
	FOR_EACHR
	Result.SetValue( i, CqColor( 0, 0, 0 ) );
	END_FORR
}


//----------------------------------------------------------------------
// illuminance(P,nsamples)
STD_SOIMPL CqShaderExecEnv::SO_illuminance( POINTVAL P, VECTORVAL Axis, FLOATVAL Angle, FLOATVAL nsamples, DEFVOIDPARAMIMPL )
{
	// Fill in the lightsource information, and transfer the results to the shader variables,
	if ( m_pSurface != 0 )
	{
		CqLightsource * lp = m_pSurface->pAttributes() ->apLights() [ m_li ];

		INIT_SOR
		CHECKVARY( P )
		CHECKVARY( Axis )
		CHECKVARY( Angle )
		CHECKVARY( nsamples )

		CqVector3D vecTemp;
		CqColor colTemp;

		FOR_EACHR
		
		// Get the light vector and color from the loghtsource.
		CqVMStackEntry SEL, SECl;
		lp->L().GetValue( i, SEL );
		lp->Cl().GetValue( i, SECl );
		GETVECTOR( SEL );
		GETCOLOR( SECl );
		
		// Store them locally on the surface.
		L().SetValue( i, CqVMStackEntry( -( VECTOR( SEL ) ) ) );
		Cl().SetValue( i, CqVMStackEntry( COLOR( SECl ) ) );
		
		// Check if its within the cone.
		CqVector3D nL = -( VECTOR( SEL ) );	
		nL.Unit();
		GETVECTOR( Axis );
		GETFLOAT( Angle );
		TqFloat cosangle = nL * VECTOR( Axis );
		if ( acos( cosangle ) > FLOAT( Angle ) )
			m_CurrentState.SetValue( i, TqFalse );
		else
			m_CurrentState.SetValue( i, TqTrue );
		
		END_FORR
	}
}


STD_SOIMPL	CqShaderExecEnv::SO_illuminance( POINTVAL P, FLOATVAL nsamples, DEFVOIDPARAMIMPL )
{
	static CqVMStackEntry Axis;
	static CqVMStackEntry Angle;

	Axis = CqVector3D( 0, 1, 0 );
	Angle = RI_PI;

	SO_illuminance( P, Axis, Angle, nsamples );
}


//----------------------------------------------------------------------
// illuminate(P)
STD_SOIMPL CqShaderExecEnv::SO_illuminate( POINTVAL P, VECTORVAL Axis, FLOATVAL Angle, DEFVOIDPARAMIMPL )
{
	TqBool res = TqTrue;
	if ( m_Illuminate > 0 ) res = TqFalse;
	INIT_SOR
	__fVarying = TqTrue;
	FOR_EACHR
	if ( res )
	{
		// Get the point being lit and set the ligth vector.
		CqVMStackEntry SEPs;
		Ps().GetValue( i, SEPs );
		GETPOINT( SEPs );
		GETPOINT( P );
		L().SetValue( i, CqVMStackEntry( POINT( SEPs ) - POINT( P ) ) );
		
		// Check if its within the cone.
		CqVMStackEntry SEL;
		L().GetValue( i, SEL );
		GETVECTOR( SEL );
		
		VECTOR( SEL ).Unit();

		GETVECTOR( Axis );
		GETFLOAT( Angle );
		TqFloat cosangle = VECTOR( SEL ) * VECTOR( Axis );
		if ( acos( cosangle ) > FLOAT( Angle ) )
		{
			// Make sure we set the light color to zero in the areas that won't be lit.
			Cl().SetValue( i, CqVMStackEntry( CqColor( 0, 0, 0 ) ) );
			m_CurrentState.SetValue( i, TqFalse );
		}
		else
			m_CurrentState.SetValue( i, TqTrue );
	}
	END_FORR
	m_Illuminate++;
}


STD_SOIMPL	CqShaderExecEnv::SO_illuminate( POINTVAL P, DEFVOIDPARAMIMPL )
{
	static CqVMStackEntry Axis;
	static CqVMStackEntry Angle;

	Axis = CqVector3D( 0, 1, 0 );
	Angle = RI_PI;

	SO_illuminate( P, Axis, Angle, pShader );
}


//----------------------------------------------------------------------
// solar()
STD_SOIMPL CqShaderExecEnv::SO_solar( VECTORVAL Axis, FLOATVAL Angle, DEFVOIDPARAMIMPL )
{
	TqBool res = TqTrue;
	if ( m_Illuminate > 0 ) res = TqFalse;
	// TODO: Check light cone, and exclude points outside.
	INIT_SOR
	__fVarying = TqTrue;
	FOR_EACHR
	if ( res )
	{
		GETVECTOR( Axis );
		L().SetValue( i, CqVMStackEntry( VECTOR( Axis ) ) );
		m_CurrentState.SetValue( i, TqTrue );
	}
	END_FORR
	m_Illuminate++;
}


STD_SOIMPL	CqShaderExecEnv::SO_solar( DEFVOIDPARAMIMPL )
{
	static CqVMStackEntry Axis;
	static CqVMStackEntry Angle;

	Axis = CqVector3D( 0, 0, 0 );
	Angle = ( TqFloat ) - 1.0f;

	SO_solar( Axis, Angle, pShader );
}


//----------------------------------------------------------------------
// printf(s,...)

STD_SOIMPL	CqShaderExecEnv::SO_printf( STRINGVAL str, DEFVOIDPARAMVARIMPL )
{
	INIT_SOR
	CHECKVARY( str )
	int ii;
	for ( ii = 0; ii < cParams; ii++ ) CHECKVARY( *apParams[ ii ] );
	FOR_EACHR
	GETSTRING( str );
	CqString strA = SO_sprintf( STRING( str ).c_str(), cParams, apParams, i );
	QGetRenderContext() ->PrintMessage( SqMessage( 0, 0, strA.c_str() ) );
	END_FORR
}


//----------------------------------------------------------------------
// format(s,...)

STD_SOIMPL	CqShaderExecEnv::SO_format( STRINGVAL str, DEFPARAMVARIMPL )
{
	INIT_SOR
	CHECKVARY( str )
	int ii;
	for ( ii = 0; ii < cParams; ii++ ) CHECKVARY( *apParams[ ii ] );
	CHECKVARY( Result );
	FOR_EACHR
	GETSTRING( str );
	CqString strA = SO_sprintf( STRING( str ).c_str(), cParams, apParams, i );
	Result.SetValue( i, strA );
	END_FORR
}


//----------------------------------------------------------------------
// concat(s,s,...)

STD_SOIMPL	CqShaderExecEnv::SO_concat( STRINGVAL stra, STRINGVAL strb, DEFPARAMVARIMPL )
{
	INIT_SOR
	CHECKVARY( stra )
	CHECKVARY( strb )
	int ii;
	for ( ii = 0; ii < cParams; ii++ ) CHECKVARY( *apParams[ ii ] );
	CHECKVARY( Result );
	FOR_EACHR
	GETSTRING( stra );
	CqString strRes = STRING( stra );
	GETSTRING( strb );
	strRes += STRING( strb );
	for ( ii = 0; ii < cParams; ii++ )
	{
		CqVMStackEntry& next = *apParams[ ii ];
		GETSTRING( next );
		strRes += STRING( next );
	}
	Result.SetValue( i, strRes );
	END_FORR
}


//----------------------------------------------------------------------
// noise(v)
STD_SOIMPL CqShaderExecEnv::SO_fcellnoise1( FLOATVAL v, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( v )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( v );
	Result.SetValue( i, m_cellnoise.FCellNoise1( FLOAT( v ) ) );
	END_FORR
}
STD_SOIMPL CqShaderExecEnv::SO_ccellnoise1( FLOATVAL v, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( v )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( v );
	Result.SetValue( i, CqColor( m_cellnoise.PCellNoise1( FLOAT( v ) ) ) );
	END_FORR
}
STD_SOIMPL CqShaderExecEnv::SO_pcellnoise1( FLOATVAL v, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( v )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( v );
	Result.SetValue( i, m_cellnoise.PCellNoise1( FLOAT( v ) ) );
	END_FORR
}

//----------------------------------------------------------------------
// noise(u,v)
STD_SOIMPL CqShaderExecEnv::SO_fcellnoise2( FLOATVAL u, FLOATVAL v, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( u )
	CHECKVARY( v )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( u );
	GETFLOAT( v );
	Result.SetValue( i, m_cellnoise.FCellNoise2( FLOAT( u ), FLOAT( v ) ) );
	END_FORR
}
STD_SOIMPL CqShaderExecEnv::SO_ccellnoise2( FLOATVAL u, FLOATVAL v, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( u )
	CHECKVARY( v )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( u );
	GETFLOAT( v );
	Result.SetValue( i, CqColor( m_cellnoise.PCellNoise2( FLOAT( u ), FLOAT( v ) ) ) );
	END_FORR
}
STD_SOIMPL CqShaderExecEnv::SO_pcellnoise2( FLOATVAL u, FLOATVAL v, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( u )
	CHECKVARY( v )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( u );
	GETFLOAT( v );
	Result.SetValue( i, m_cellnoise.PCellNoise2( FLOAT( u ), FLOAT( v ) ) );
	END_FORR
}

//----------------------------------------------------------------------
// noise(p)
STD_SOIMPL CqShaderExecEnv::SO_fcellnoise3( POINTVAL p, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( Result )
	FOR_EACHR
	GETPOINT( p );
	Result.SetValue( i, m_cellnoise.FCellNoise3( POINT( p ) ) );
	END_FORR
}
STD_SOIMPL CqShaderExecEnv::SO_ccellnoise3( POINTVAL p, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( Result )
	FOR_EACHR
	GETPOINT( p );
	Result.SetValue( i, CqColor( m_cellnoise.PCellNoise3( POINT( p ) ) ) );
	END_FORR
}
STD_SOIMPL CqShaderExecEnv::SO_pcellnoise3( POINTVAL p, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( Result )
	FOR_EACHR
	GETPOINT( p );
	Result.SetValue( i, m_cellnoise.PCellNoise3( POINT( p ) ) );
	END_FORR
}

//----------------------------------------------------------------------
// noise(p,f)
STD_SOIMPL CqShaderExecEnv::SO_fcellnoise4( POINTVAL p, FLOATVAL v, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( v )
	CHECKVARY( Result )
	FOR_EACHR
	GETPOINT( p );
	GETFLOAT( v );
	Result.SetValue( i, m_cellnoise.FCellNoise4( POINT( p ), FLOAT( v ) ) );
	END_FORR
}
STD_SOIMPL CqShaderExecEnv::SO_ccellnoise4( POINTVAL p, FLOATVAL v, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( v )
	CHECKVARY( Result )
	FOR_EACHR
	GETPOINT( p );
	GETFLOAT( v );
	Result.SetValue( i, CqColor( m_cellnoise.PCellNoise4( POINT( p ), FLOAT( v ) ) ) );
	END_FORR
}
STD_SOIMPL CqShaderExecEnv::SO_pcellnoise4( POINTVAL p, FLOATVAL v, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( v )
	CHECKVARY( Result )
	FOR_EACHR
	GETPOINT( p );
	GETFLOAT( v );
	Result.SetValue( i, m_cellnoise.PCellNoise4( POINT( p ), FLOAT( v ) ) );
	END_FORR
}



//----------------------------------------------------------------------
// atmosphere
//

STD_SOIMPL CqShaderExecEnv::SO_atmosphere( STRINGVAL name, IqShaderVariable* pV, DEFPARAMIMPL )
{
	CqShader * pAtmosphere = m_pSurface->pAttributes() ->pshadAtmosphere();
	TqInt i = 0;
	GETSTRING( name );
	if ( pAtmosphere ) Result.SetValue( 0, pAtmosphere->GetValue( STRING( name ).c_str(), pV ) ? 1.0f : 0.0f );
	else	Result.SetValue( 0, 0.0f );
}


//----------------------------------------------------------------------
// displacement
//

STD_SOIMPL CqShaderExecEnv::SO_displacement( STRINGVAL name, IqShaderVariable* pV, DEFPARAMIMPL )
{
	CqShader * pDisplacement = m_pSurface->pAttributes() ->pshadDisplacement();
	TqInt i = 0;
	GETSTRING( name );
	if ( pDisplacement ) Result.SetValue( 0, pDisplacement->GetValue( STRING( name ).c_str(), pV ) ? 1.0f : 0.0f );
	else	Result.SetValue( 0, 0.0f );
}


//----------------------------------------------------------------------
// lightsource
//

STD_SOIMPL CqShaderExecEnv::SO_lightsource( STRINGVAL name, IqShaderVariable* pV, DEFPARAMIMPL )
{
	// This should only be called within an Illuminance construct, so m_li should be valid.
	TqInt i = 0;
	CqShader* pLightsource = 0;
	GETSTRING( name );
	if ( m_li < static_cast<TqInt>( m_pSurface->pAttributes() ->apLights().size() ) )
		pLightsource = m_pSurface->pAttributes() ->apLights() [ m_li ] ->pShader();
	if ( pLightsource ) Result.SetValue( 0, pLightsource->GetValue( STRING( name ).c_str(), pV ) ? 1.0f : 0.0f );
	else	Result.SetValue( 0, 0.0f );
}


//----------------------------------------------------------------------
// surface
//

STD_SOIMPL CqShaderExecEnv::SO_surface( STRINGVAL name, IqShaderVariable* pV, DEFPARAMIMPL )
{
	CqShader * pSurface = m_pSurface->pAttributes() ->pshadSurface();
	TqInt i = 0;
	GETSTRING( name );
	if ( pSurface ) Result.SetValue( 0, pSurface->GetValue( STRING( name ).c_str(), pV ) ? 1.0f : 0.0f );
	else	Result.SetValue( 0, 0.0f );
}


//----------------------------------------------------------------------
// attribute
//

STD_SOIMPL CqShaderExecEnv::SO_attribute( STRINGVAL name, IqShaderVariable* pV, DEFPARAMIMPL )
{
	//Find out if it is a specific attribute request
	TqInt i = 0;
	GETSTRING( name );
	TqFloat Ret = 0.0f;

	if ( STRING( name ).compare( "ShadingRate" ) == 0 )
	{
		if ( pV->Type() == type_float )
		{
			CqVMStackEntry SE( static_cast<TqInt>( m_pSurface->pAttributes() ->fEffectiveShadingRate() ) );
			pV->SetValue( SE );
			Ret = 1.0f;
		}
	}
	else if ( STRING( name ).compare( "Sides" ) == 0 )
	{
		if ( pV->Type() == type_float )
		{
			CqVMStackEntry SE( m_pSurface->pAttributes() ->iNumberOfSides() );
			pV->SetValue( SE );
			Ret = 1.0f;
		}
	}
	else if ( STRING( name ).compare( "Matte" ) == 0 )
	{
		if ( pV->Type() == type_float )
		{
			CqVMStackEntry SE( m_pSurface->pAttributes() ->bMatteSurfaceFlag() );
			pV->SetValue( SE );
			Ret = 1.0f;
		}
	}
	else
	{
		int iColon = STRING( name ).find_first_of( ':' );
		if ( iColon >= 0 )
		{
			CqString strParam = STRING( name ).substr( iColon + 1, STRING( name ).size() - iColon - 1 );
			STRING( name ) = STRING( name ).substr( 0, iColon );
			const CqParameter* pParam = m_pSurface->pAttributes() ->pParameter( STRING( name ).c_str(), strParam.c_str() );
			if ( pParam != 0 )
			{
				// Should only be able to query uniform parameters here, varying ones should be handled
				// by passing as shader paramters.
				// Types must match, although storage doesn't have to
				if ( pParam->Class() == class_uniform &&
				     pParam->Type() == pV->Type() )
				{
					CqVMStackEntry se;
					switch ( pParam->Type() )
					{
							case type_integer:
							{
								se = static_cast<TqFloat>( *( static_cast<const CqParameterTyped<TqInt>*>( pParam ) ->pValue() ) );
								break;
							}

							case type_float:
							{
								se = *( static_cast<const CqParameterTyped<TqFloat>*>( pParam ) ->pValue() );
								break;
							}

							case type_string:
							{
								se = *( static_cast<const CqParameterTyped<CqString>*>( pParam ) ->pValue() );
								break;
							}

							case type_point:
							case type_vector:
							case type_normal:
							{
								se = *( static_cast<const CqParameterTyped<CqVector3D>*>( pParam ) ->pValue() );
								break;
							}

							case type_color:
							{
								se = *( static_cast<const CqParameterTyped<CqColor>*>( pParam ) ->pValue() );
								break;
							}

							case type_matrix:
							{
								se = *( static_cast<const CqParameterTyped<CqMatrix>*>( pParam ) ->pValue() );
								break;
							}
					}
					pV->SetValue( se );
					Ret = 1.0f;
				}
			}
		}
	}

	Result.SetValue( 0, Ret );
}


//----------------------------------------------------------------------
// option
//

STD_SOIMPL CqShaderExecEnv::SO_option( STRINGVAL name, IqShaderVariable* pV, DEFPARAMIMPL )
{
	//Find out if it is a specific option request
	TqInt i = 0;
	GETSTRING( name );
	TqFloat Ret = 0.0f;
	CqVMStackEntry se;

	if ( STRING( name ).compare( "Format" ) == 0 )
	{
		if ( pV->Type() == type_float &&
		     pV->ArrayLength() > 0 )
		{
			CqShaderVariableArray * paV = static_cast<CqShaderVariableArray*>( pV );
			if ( paV->ArrayLength() >= 3 )
			{
				se = static_cast<TqFloat>( QGetRenderContext() ->optCurrent().iXResolution() );
				( *paV ) [ 0 ] ->SetValue( se );
				se = static_cast<TqFloat>( QGetRenderContext() ->optCurrent().iYResolution() );
				( *paV ) [ 1 ] ->SetValue( se );
				se = static_cast<TqFloat>( QGetRenderContext() ->optCurrent().fPixelAspectRatio() );
				( *paV ) [ 2 ] ->SetValue( se );
				Ret = 1.0f;
			}
		}
	}
	else if ( STRING( name ).compare( "CropWindow" ) == 0 )
	{
		if ( pV->Type() == type_float &&
		     pV->ArrayLength() > 0 )
		{
			CqShaderVariableArray * paV = static_cast<CqShaderVariableArray*>( pV );
			if ( paV->ArrayLength() >= 4 )
			{
				se = static_cast<TqFloat>( QGetRenderContext() ->optCurrent().fCropWindowXMin() );
				( *paV ) [ 0 ] ->SetValue( se );
				se = static_cast<TqFloat>( QGetRenderContext() ->optCurrent().fCropWindowXMax() );
				( *paV ) [ 1 ] ->SetValue( se );
				se = static_cast<TqFloat>( QGetRenderContext() ->optCurrent().fCropWindowYMin() );
				( *paV ) [ 2 ] ->SetValue( se );
				se = static_cast<TqFloat>( QGetRenderContext() ->optCurrent().fCropWindowYMax() );
				( *paV ) [ 3 ] ->SetValue( se );
				Ret = 1.0f;
			}
		}
	}
	else if ( STRING( name ).compare( "FrameAspectRatio" ) == 0 )
	{
		if ( pV->Type() == type_float )
		{
			se = static_cast<TqFloat>( QGetRenderContext() ->optCurrent().fFrameAspectRatio() );
			pV->SetValue( se );
			Ret = 1.0f;
		}
	}
	else if ( STRING( name ).compare( "DepthOfField" ) == 0 )
	{
		if ( pV->Type() == type_float &&
		     pV->ArrayLength() > 0 )
		{
			CqShaderVariableArray * paV = static_cast<CqShaderVariableArray*>( pV );
			if ( paV->ArrayLength() >= 3 )
			{
				se = static_cast<TqFloat>( QGetRenderContext() ->optCurrent().ffStop() );
				( *paV ) [ 0 ] ->SetValue( se );
				se = static_cast<TqFloat>( QGetRenderContext() ->optCurrent().fFocalLength() );
				( *paV ) [ 1 ] ->SetValue( se );
				se = static_cast<TqFloat>( QGetRenderContext() ->optCurrent().fFocalDistance() );
				( *paV ) [ 2 ] ->SetValue( se );
				Ret = 1.0f;
			}
		}
	}
	else if ( STRING( name ).compare( "Shutter" ) == 0 )
	{
		if ( pV->Type() == type_float &&
		     pV->ArrayLength() > 0 )
		{
			CqShaderVariableArray * paV = static_cast<CqShaderVariableArray*>( pV );
			if ( paV->ArrayLength() >= 2 )
			{
				se = static_cast<TqFloat>( QGetRenderContext() ->optCurrent().fShutterOpen() );
				( *paV ) [ 0 ] ->SetValue( se );
				se = static_cast<TqFloat>( QGetRenderContext() ->optCurrent().fShutterClose() );
				( *paV ) [ 1 ] ->SetValue( se );
				Ret = 1.0f;
			}
		}
	}
	else if ( STRING( name ).compare( "Clipping" ) == 0 )
	{
		if ( pV->Type() == type_float &&
		     pV->ArrayLength() > 0 )
		{
			CqShaderVariableArray * paV = static_cast<CqShaderVariableArray*>( pV );
			if ( paV->ArrayLength() >= 2 )
			{
				se = static_cast<TqFloat>( QGetRenderContext() ->optCurrent().fClippingPlaneNear() );
				( *paV ) [ 0 ] ->SetValue( se );
				se = static_cast<TqFloat>( QGetRenderContext() ->optCurrent().fClippingPlaneFar() );
				( *paV ) [ 1 ] ->SetValue( se );
				Ret = 1.0f;
			}
		}
	}
	else
	{
		int iColon = STRING( name ).find_first_of( ':' );
		if ( iColon >= 0 )
		{
			CqString strParam = STRING( name ).substr( iColon + 1, STRING( name ).size() - iColon - 1 );
			STRING( name ) = STRING( name ).substr( 0, iColon );
			const CqParameter* pParam = QGetRenderContext() ->optCurrent().pParameter( STRING( name ).c_str(), strParam.c_str() );
			if ( pParam != 0 )
			{
				// Should only be able to query uniform parameters here, varying ones should be handled
				// by passing as shader paramters.
				// Types must match, although storage doesn't have to
				if ( pParam->Class() == class_uniform &&
				     pParam->Type() == pV->Type() )
				{
					switch ( pParam->Type() )
					{
							case type_integer:
							{
								se = static_cast<TqFloat>( *( static_cast<const CqParameterTyped<TqInt>*>( pParam ) ->pValue() ) );
								break;
							}

							case type_float:
							{
								se = *( static_cast<const CqParameterTyped<TqFloat>*>( pParam ) ->pValue() );
								break;
							}

							case type_string:
							{
								se = *( static_cast<const CqParameterTyped<CqString>*>( pParam ) ->pValue() );
								break;
							}

							case type_point:
							case type_vector:
							case type_normal:
							{
								se = *( static_cast<const CqParameterTyped<CqVector3D>*>( pParam ) ->pValue() );
								break;
							}

							case type_color:
							{
								se = *( static_cast<const CqParameterTyped<CqColor>*>( pParam ) ->pValue() );
								break;
							}

							case type_matrix:
							{
								se = *( static_cast<const CqParameterTyped<CqMatrix>*>( pParam ) ->pValue() );
								break;
							}
					}
					pV->SetValue( se );
					Ret = 1.0f;
				}
			}
		}
	}

	Result.SetValue( 0, Ret );
}


//----------------------------------------------------------------------
// rendererinfo
//

STD_SOIMPL CqShaderExecEnv::SO_rendererinfo( STRINGVAL name, IqShaderVariable* pV, DEFPARAMIMPL )
{
	TqInt i = 0;
	GETSTRING( name );
	TqFloat Ret = 0.0f;
	CqVMStackEntry se;

	if ( STRING( name ).compare( "renderer" ) == 0 )
	{
		if ( pV->Type() == type_string )
		{
			se = STRNAME;
			pV->SetValue( se );
			Ret = 1.0f;
		}
	}
	else if ( STRING( name ).compare( "version" ) == 0 )
	{
		if ( pV->Type() == type_float &&
		     pV->ArrayLength() > 0 )
		{
			CqShaderVariableArray * paV = static_cast<CqShaderVariableArray*>( pV );
			if ( paV->ArrayLength() >= 4 )
			{
				se = static_cast<TqFloat>( VERMAJOR );
				( *paV ) [ 0 ] ->SetValue( se );
				se = static_cast<TqFloat>( VERMINOR );
				( *paV ) [ 1 ] ->SetValue( se );
				se = static_cast<TqFloat>( BUILD );
				( *paV ) [ 2 ] ->SetValue( se );
				se = static_cast<TqFloat>( 0.0f );
				( *paV ) [ 3 ] ->SetValue( se );
				Ret = 1.0f;
			}
		}
	}
	else if ( STRING( name ).compare( "versionstring" ) == 0 )
	{
		if ( pV->Type() == type_string )
		{
#if defined(AQSIS_SYSTEM_WIN32) || defined(AQSIS_SYSTEM_MACOSX)
			se = VERSION_STR;
#else // AQSIS_SYSTEM_WIN32
			se = VERSION;
#endif // !AQSIS_SYSTEM_WIN32
			pV->SetValue( se );
			Ret = 1.0f;
		}
	}

	Result.SetValue( 0, Ret );
}


//----------------------------------------------------------------------
// incident

STD_SOIMPL CqShaderExecEnv::SO_incident( STRINGVAL name, IqShaderVariable* pV, DEFPARAMIMPL )
{
	TqFloat Ret = 0.0f;
	Result.SetValue( 0, Ret );
}


//----------------------------------------------------------------------
// opposite

STD_SOIMPL CqShaderExecEnv::SO_opposite( STRINGVAL name, IqShaderVariable* pV, DEFPARAMIMPL )
{
	TqFloat Ret = 0.0f;
	Result.SetValue( 0, Ret );
}


//----------------------------------------------------------------------
// ctransform(s,s,c)
STD_SOIMPL CqShaderExecEnv::SO_ctransform( STRINGVAL fromspace, STRINGVAL tospace, COLORVAL c, DEFPARAMIMPL )
{
	TqInt i = 0;
	GETSTRING( fromspace );
	GETSTRING( tospace );
	
	INIT_SOR
	CHECKVARY( c )
	CHECKVARY( Result )
	FOR_EACHR
	GETCOLOR( c );
	CqColor res( COLOR( c ) );
	if ( STRING( fromspace ).compare( "hsv" ) ) res = COLOR( c ).hsvtorgb();
	else if ( STRING( fromspace ).compare( "hsl" ) ) res = COLOR( c ).hsltorgb();
	else if ( STRING( fromspace ).compare( "XYZ" ) ) res = COLOR( c ).XYZtorgb();
	else if ( STRING( fromspace ).compare( "xyY" ) ) res = COLOR( c ).xyYtorgb();
	else if ( STRING( fromspace ).compare( "YIQ" ) ) res = COLOR( c ).YIQtorgb();

	if ( STRING( tospace ).compare( "hsv" ) ) res = COLOR( c ).rgbtohsv();
	else if ( STRING( tospace ).compare( "hsl" ) ) res = COLOR( c ).rgbtohsl();
	else if ( STRING( tospace ).compare( "XYZ" ) ) res = COLOR( c ).rgbtoXYZ();
	else if ( STRING( tospace ).compare( "xyY" ) ) res = COLOR( c ).rgbtoxyY();
	else if ( STRING( tospace ).compare( "YIQ" ) ) res = COLOR( c ).rgbtoYIQ();

	Result.SetValue( i, res );
	END_FORR
}


//----------------------------------------------------------------------
// ctransform(s,c)
STD_SOIMPL CqShaderExecEnv::SO_ctransform( STRINGVAL tospace, COLORVAL c, DEFPARAMIMPL )
{
	static CqVMStackEntry fromspace;
	fromspace = "rgb";

	assert( pShader != 0 );
	SO_ctransform( fromspace, tospace, c, Result, pShader );
}


//----------------------------------------------------------------------
// ctransform(s,c)
STD_SOIMPL CqShaderExecEnv::SO_ptlined( POINTVAL P0, POINTVAL P1, POINTVAL Q, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( P0 )
	CHECKVARY( P1 )
	CHECKVARY( Q )
	CHECKVARY( Result )
	FOR_EACHR
	GETPOINT( P0 );
	GETPOINT( P1 );
	GETPOINT( Q );
	CqVector3D kDiff = POINT( Q ) - POINT( P0 );
	CqVector3D vecDir = POINT( P1 ) - POINT( P0 );
	TqFloat fT = kDiff * vecDir;

	if ( fT <= 0.0f )
		fT = 0.0f;
	else
	{
		TqFloat fSqrLen = vecDir.Magnitude2();
		if ( fT >= fSqrLen )
		{
			fT = 1.0f;
			kDiff -= vecDir;
		}
		else
		{
			fT /= fSqrLen;
			kDiff -= fT * vecDir;
		}
	}
	Result.SetValue( i, kDiff.Magnitude() );
	END_FORR
}


STD_SOIMPL	CqShaderExecEnv::SO_inversesqrt( FLOATVAL x, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( x )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( x );
	Result.SetValue( i, 1.0f / static_cast<TqFloat>( sqrt( FLOAT( x ) ) ) );
	END_FORR
}

STD_SOIMPL	CqShaderExecEnv::SO_match( STRINGVAL a, STRINGVAL b, DEFPARAMIMPL )
{
	// TODO: Do this properly.
	INIT_SOR
	FOR_EACHR
	Result.SetValue( i, 0.0f );
	END_FORR
}


//----------------------------------------------------------------------
// pnoise(u,period)
STD_SOIMPL CqShaderExecEnv::SO_fpnoise1( FLOATVAL v, FLOATVAL period, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( v )
	CHECKVARY( period )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( v );
	GETFLOAT( period );
	Result.SetValue( i, ( m_noise.FGNoise1( fmod( FLOAT( v ), FLOAT( period ) ) ) + 1 ) / 2.0f );
	END_FORR
}

//----------------------------------------------------------------------
// pnoise(u,v,uperiod,vperiod)
STD_SOIMPL CqShaderExecEnv::SO_fpnoise2( FLOATVAL u, FLOATVAL v, FLOATVAL uperiod, FLOATVAL vperiod, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( u )
	CHECKVARY( uperiod )
	CHECKVARY( v )
	CHECKVARY( vperiod )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( u );
	GETFLOAT( v );
	GETFLOAT( uperiod );
	GETFLOAT( vperiod );
	Result.SetValue( i, ( m_noise.FGNoise2( fmod( FLOAT( u ), FLOAT( uperiod ) ),
	                                        fmod( FLOAT( v ), FLOAT( vperiod ) ) ) + 1 ) / 2.0f );
	END_FORR
}

//----------------------------------------------------------------------
// pnoise(p,pperiod)
STD_SOIMPL CqShaderExecEnv::SO_fpnoise3( POINTVAL p, POINTVAL pperiod, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( pperiod )
	CHECKVARY( Result )
	FOR_EACHR
	GETPOINT( p );
	GETPOINT( pperiod );
	Result.SetValue( i, ( m_noise.FGNoise3( CqVector3D( fmod( POINT( p ).x(), POINT( pperiod ).x() ),
	                                        fmod( POINT( p ).y(), POINT( pperiod ).y() ),
	                                        fmod( POINT( p ).z(), POINT( pperiod ).z() ) ) ) + 1 ) / 2.0f );
	END_FORR
}

//----------------------------------------------------------------------
// pnoise(p,t,pperiod,tperiod)
STD_SOIMPL CqShaderExecEnv::SO_fpnoise4( POINTVAL p, FLOATVAL t, POINTVAL pperiod, FLOATVAL tperiod, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( pperiod )
	CHECKVARY( t )
	CHECKVARY( tperiod )
	CHECKVARY( Result )
	FOR_EACHR
	GETPOINT( p );
	GETFLOAT( t );
	GETPOINT( pperiod );
	GETFLOAT( tperiod );
	Result.SetValue( i, ( m_noise.FGNoise3( CqVector3D( fmod( POINT( p ).x(), POINT( pperiod ).x() ),
	                                        fmod( POINT( p ).y(), POINT( pperiod ).y() ),
	                                        fmod( POINT( p ).z(), POINT( pperiod ).z() ) ) ) + 1 ) / 2.0f );
	END_FORR
}

//----------------------------------------------------------------------
// pnoise(u,period)
STD_SOIMPL CqShaderExecEnv::SO_cpnoise1( FLOATVAL v, FLOATVAL period, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( v )
	CHECKVARY( period )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( v );
	GETFLOAT( period );
	Result.SetValue( i, ( m_noise.CGNoise1( fmod( FLOAT( v ), FLOAT( period ) ) ) + 1 ) / 2.0f );
	END_FORR
}

//----------------------------------------------------------------------
// pnoise(u,v,uperiod,vperiod)
STD_SOIMPL CqShaderExecEnv::SO_cpnoise2( FLOATVAL u, FLOATVAL v, FLOATVAL uperiod, FLOATVAL vperiod, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( u )
	CHECKVARY( uperiod )
	CHECKVARY( v )
	CHECKVARY( vperiod )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( u );
	GETFLOAT( v );
	GETFLOAT( uperiod );
	GETFLOAT( vperiod );
	Result.SetValue( i, ( m_noise.CGNoise2( fmod( FLOAT( u ), FLOAT( uperiod ) ),
	                                        fmod( FLOAT( v ), FLOAT( vperiod ) ) ) + 1 ) / 2.0f );
	END_FORR
}

//----------------------------------------------------------------------
// pnoise(p,pperiod)
STD_SOIMPL CqShaderExecEnv::SO_cpnoise3( POINTVAL p, POINTVAL pperiod, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( pperiod )
	CHECKVARY( Result )
	FOR_EACHR
	GETPOINT( p );
	GETPOINT( pperiod );
	Result.SetValue( i, ( m_noise.CGNoise3( CqVector3D( fmod( POINT( p ).x(), POINT( pperiod ).x() ),
	                                        fmod( POINT( p ).y(), POINT( pperiod ).y() ),
	                                        fmod( POINT( p ).z(), POINT( pperiod ).z() ) ) ) + 1 ) / 2.0f );
	END_FORR
}

//----------------------------------------------------------------------
// pnoise(p,t,pperiod,tperiod)
STD_SOIMPL CqShaderExecEnv::SO_cpnoise4( POINTVAL p, FLOATVAL t, POINTVAL pperiod, FLOATVAL tperiod, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( pperiod )
	CHECKVARY( t )
	CHECKVARY( tperiod )
	CHECKVARY( Result )
	FOR_EACHR
	GETPOINT( p );
	GETFLOAT( t );
	GETPOINT( pperiod );
	GETFLOAT( tperiod );
	Result.SetValue( i, ( m_noise.CGNoise3( CqVector3D( fmod( POINT( p ).x(), POINT( pperiod ).x() ),
	                                        fmod( POINT( p ).y(), POINT( pperiod ).y() ),
	                                        fmod( POINT( p ).z(), POINT( pperiod ).z() ) ) ) + 1 ) / 2.0f );
	END_FORR
}

//----------------------------------------------------------------------
// pnoise(u,period)
STD_SOIMPL CqShaderExecEnv::SO_ppnoise1( FLOATVAL v, FLOATVAL period, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( v )
	CHECKVARY( period )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( v );
	GETFLOAT( period );
	Result.SetValue( i, ( m_noise.PGNoise1( fmod( FLOAT( v ), FLOAT( period ) ) ) + 1 ) / 2.0f );
	END_FORR
}

//----------------------------------------------------------------------
// pnoise(u,v,uperiod,vperiod)
STD_SOIMPL CqShaderExecEnv::SO_ppnoise2( FLOATVAL u, FLOATVAL v, FLOATVAL uperiod, FLOATVAL vperiod, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( u )
	CHECKVARY( uperiod )
	CHECKVARY( v )
	CHECKVARY( vperiod )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( u );
	GETFLOAT( v );
	GETFLOAT( uperiod );
	GETFLOAT( vperiod );
	Result.SetValue( i, ( m_noise.PGNoise2( fmod( FLOAT( u ), FLOAT( uperiod ) ),
	                                        fmod( FLOAT( v ), FLOAT( vperiod ) ) ) + 1 ) / 2.0f );
	END_FORR
}

//----------------------------------------------------------------------
// pnoise(p,pperiod)
STD_SOIMPL CqShaderExecEnv::SO_ppnoise3( POINTVAL p, POINTVAL pperiod, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( pperiod )
	CHECKVARY( Result )
	FOR_EACHR
	GETPOINT( p );
	GETPOINT( pperiod );
	Result.SetValue( i, ( m_noise.PGNoise3( CqVector3D( fmod( POINT( p ).x(), POINT( pperiod ).x() ),
	                                        fmod( POINT( p ).y(), POINT( pperiod ).y() ),
	                                        fmod( POINT( p ).z(), POINT( pperiod ).z() ) ) ) + 1 ) / 2.0f );
	END_FORR
}

//----------------------------------------------------------------------
// pnoise(p,t,pperiod,tperiod)
STD_SOIMPL CqShaderExecEnv::SO_ppnoise4( POINTVAL p, FLOATVAL t, POINTVAL pperiod, FLOATVAL tperiod, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( p )
	CHECKVARY( pperiod )
	CHECKVARY( t )
	CHECKVARY( tperiod )
	CHECKVARY( Result )
	FOR_EACHR
	GETPOINT( p );
	GETFLOAT( t );
	GETPOINT( pperiod );
	GETFLOAT( tperiod );
	Result.SetValue( i, ( m_noise.PGNoise3( CqVector3D( fmod( POINT( p ).x(), POINT( pperiod ).x() ),
	                                        fmod( POINT( p ).y(), POINT( pperiod ).y() ),
	                                        fmod( POINT( p ).z(), POINT( pperiod ).z() ) ) ) + 1 ) / 2.0f );
	END_FORR
}


//----------------------------------------------------------------------
// rotate(Q,angle,P0,P1)
STD_SOIMPL CqShaderExecEnv::SO_rotate( VECTORVAL Q, FLOATVAL angle, POINTVAL P0, POINTVAL P1, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( Q )
	CHECKVARY( angle )
	CHECKVARY( P0 )
	CHECKVARY( P1 )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( angle );
	GETVECTOR( Q );
	GETPOINT( P0 );
	GETPOINT( P1 );
	CqMatrix matR( FLOAT( angle ), POINT( P1 ) - POINT( P0 ) );

	CqVector3D	Res( VECTOR( Q ) );
	Res = matR * Res;

	Result.SetValue( i, Res );
	END_FORR
}

//----------------------------------------------------------------------
// filterstep(edge,s1)
STD_SOIMPL CqShaderExecEnv::SO_filterstep( FLOATVAL edge, FLOATVAL s1, DEFPARAMVARIMPL )
{
	GET_FILTER_PARAMS;

	INIT_SOR
	CHECKVARY( edge )
	CHECKVARY( s1 )
	CHECKVARY( Result )

	// We can get these here because the are uniform.
	CqVMStackEntry SEdu, SEdv;
	du().GetValue( 0, SEdu );
	dv().GetValue( 0, SEdv );

	FOR_EACHR
	GETFLOAT( s1 );
	GETFLOAT( edge );
	TqFloat dsdu = SO_DuType<TqFloat>( s1, m_GridI, *this );
	TqFloat dsdv = SO_DvType<TqFloat>( s1, m_GridI, *this );

	GETFLOAT( SEdu );
	GETFLOAT( SEdv );
	TqFloat w = fabs( dsdu * FLOAT( SEdu ) * dsdv * FLOAT( SEdv ) );
	w *= _pswidth;

	Result.SetValue( i, CLAMP( ( FLOAT( s1 ) + w / 2.0f - FLOAT( edge ) ) / w, 0, 1 ) );
	END_FORR
}

//----------------------------------------------------------------------
// filterstep(edge,s1,s2)
STD_SOIMPL CqShaderExecEnv::SO_filterstep2( FLOATVAL edge, FLOATVAL s1, FLOATVAL s2, DEFPARAMVARIMPL )
{
	GET_FILTER_PARAMS;

	INIT_SOR
	CHECKVARY( edge )
	CHECKVARY( s1 )
	CHECKVARY( s2 )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( edge );
	GETFLOAT( s1 );
	GETFLOAT( s2 );
	TqFloat w = FLOAT( s2 ) - FLOAT( s1 );
	w *= _pswidth;
	Result.SetValue( i, CLAMP( ( FLOAT( s1 ) + w / 2.0f - FLOAT( edge ) ) / w, 0, 1 ) );
	END_FORR
}

//----------------------------------------------------------------------
// specularbrdf(L,N,V,rough)
STD_SOIMPL CqShaderExecEnv::SO_specularbrdf( VECTORVAL L, NORMALVAL N, VECTORVAL V, FLOATVAL rough, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( L )
	CHECKVARY( N )
	CHECKVARY( V )
	CHECKVARY( rough )
	CHECKVARY( Result )
	CqColor colTemp;
	FOR_EACHR
	GETVECTOR( L );
	GETVECTOR( V );
	VECTOR( L ).Unit();
	
	CqVector3D	H = VECTOR( L ) + VECTOR( V );	
	H.Unit();
	/// \note The (roughness/8) term emulates the BMRT behaviour for prmanspecular.
	CqVMStackEntry SECl;
	Cl().GetValue( i, SECl );
	GETCOLOR( SECl );
	GETNORMAL( N );
	GETFLOAT( rough );
	Result.SetValue( i, COLOR( SECl ) * pow( MAX( 0.0f, NORMAL( N ) * H ), 1.0f / ( FLOAT( rough ) / 8.0f ) ) );
	END_FORR
}


//----------------------------------------------------------------------
// determinant(m)
STD_SOIMPL CqShaderExecEnv::SO_determinant( MATRIXVAL M, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( M )
	CHECKVARY( Result )
	FOR_EACHR
	GETMATRIX( M );
	Result.SetValue( i, MATRIX( M ).Determinant() );
	END_FORR
}


//----------------------------------------------------------------------
// translate(m,v)
STD_SOIMPL CqShaderExecEnv::SO_mtranslate( MATRIXVAL M, VECTORVAL V, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( M )
	CHECKVARY( V )
	CHECKVARY( Result )
	FOR_EACHR
	GETMATRIX( M );
	GETVECTOR( V );
	MATRIX( M ).Translate( VECTOR( V ) );
	Result.SetValue( i, MATRIX( M ) );
	END_FORR
}

//----------------------------------------------------------------------
// rotate(m,v)
STD_SOIMPL CqShaderExecEnv::SO_mrotate( MATRIXVAL M, FLOATVAL angle, VECTORVAL axis, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( M )
	CHECKVARY( angle )
	CHECKVARY( axis )
	CHECKVARY( Result )
	FOR_EACHR
	GETMATRIX( M );
	GETFLOAT( angle );
	GETVECTOR( axis );
	MATRIX( M ).Rotate( FLOAT( angle ), VECTOR( axis ) );
	Result.SetValue( i, MATRIX( M ) );
	END_FORR
}

//----------------------------------------------------------------------
// scale(m,p)
STD_SOIMPL CqShaderExecEnv::SO_mscale( MATRIXVAL M, POINTVAL S, DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( M )
	CHECKVARY( S )
	CHECKVARY( Result )
	FOR_EACHR
	GETPOINT( S );
	GETMATRIX( M );
	MATRIX( M ).Scale( POINT( S ).x(), POINT( S ).y(), POINT( S ).z() );
	Result.SetValue( i, MATRIX( M ) );
	END_FORR
}


//----------------------------------------------------------------------
// setmcomp(p,v)
STD_SOIMPL	CqShaderExecEnv::SO_setmcomp( MATRIXVAL M, FLOATVAL r, FLOATVAL c, FLOATVAL v, DEFVOIDPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( M )
	CHECKVARY( r )
	CHECKVARY( c )
	CHECKVARY( v )
	FOR_EACHR
	GETMATRIX( M );
	GETFLOAT( r );
	GETFLOAT( c );
	GETFLOAT( v );
	MATRIX( M )[ static_cast<TqInt>( FLOAT( r ) ) ][ static_cast<TqInt>( FLOAT( c ) ) ] = FLOAT( v );
	M.SetValue( i, MATRIX( M ) );
	END_FORR
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_fsplinea( FLOATVAL value, FLOATARRAYVAL a, DEFPARAMIMPL )
{
	assert( a.fVariable() );
	assert( a.pVariable() ->ArrayLength() > 0 );
	assert( a.pVariable() ->Type() == type_float );

	CqShaderVariableArray* pArray = static_cast<CqShaderVariableArray*>( a.pVariable() );
	TqInt	cParams = pArray->aVariables().size();
	CqSplineCubic spline( cParams );

	INIT_SOR
	CHECKVARY( value )
	CHECKVARY( a )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( value );

	if ( FLOAT( value ) >= 1.0f ) pArray->aVariables() [ cParams - 2 ] ->GetValue( i, Result );
	else if ( FLOAT( value ) <= 0.0f ) pArray->aVariables() [ 1 ] ->GetValue( i, Result );
	else
	{
		TqInt j;
		for ( j = 0; j < cParams; j++ )
		{
			CqVMStackEntry next;
			( pArray->aVariables() [ j ] ) ->GetValue( i, next );
			GETFLOAT( next );
			spline[ j ] = CqVector4D( FLOAT( next ), 0.0f, 0.0f, 1.0f );
		}

		CqVector4D	res = spline.Evaluate( FLOAT( value ) );
		Result.SetValue( i, res.x() );
	}
	END_FORR
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_csplinea( FLOATVAL value, COLORARRAYVAL a, DEFPARAMIMPL )
{
	assert( a.fVariable() );
	assert( a.pVariable() ->ArrayLength() > 0 );
	assert( a.pVariable() ->Type() == type_color );

	CqShaderVariableArray* pArray = static_cast<CqShaderVariableArray*>( a.pVariable() );
	TqInt	cParams = pArray->aVariables().size();
	CqSplineCubic spline( cParams );

	INIT_SOR
	CHECKVARY( value )
	CHECKVARY( a )
	CHECKVARY( Result )
	CqColor colTemp;
	FOR_EACHR
	GETFLOAT( value );

	if ( FLOAT( value ) >= 1.0f ) pArray->aVariables() [ cParams - 2 ] ->GetValue( i, Result );
	else if ( FLOAT( value ) <= 0.0f ) pArray->aVariables() [ 1 ] ->GetValue( i, Result );
	else
	{
		TqInt j;
		for ( j = 0; j < cParams; j++ )
		{
			CqVMStackEntry next;
			( pArray->aVariables() [ j ] ) ->GetValue( i, next );
			GETCOLOR( next );
			spline[ j ] = CqVector4D( COLOR( next ).fRed(), COLOR( next ).fGreen(), COLOR( next ).fBlue(), 1.0f );
		}

		CqVector4D	res = spline.Evaluate( FLOAT( value ) );
		Result.SetValue( i, CqColor( res.x(), res.y(), res.z() ) );
	}
	END_FORR
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_psplinea( FLOATVAL value, POINTARRAYVAL a, DEFPARAMIMPL )
{
	assert( a.fVariable() );
	assert( a.pVariable() ->ArrayLength() > 0 );
	assert( a.pVariable() ->Type() == type_point );

	CqShaderVariableArray* pArray = static_cast<CqShaderVariableArray*>( a.pVariable() );
	TqInt	cParams = pArray->aVariables().size();
	CqSplineCubic spline( cParams );

	INIT_SOR
	CHECKVARY( value )
	CHECKVARY( a )
	CHECKVARY( Result )
	CqVector3D vecTemp;
	FOR_EACHR
	GETFLOAT( value );

	if ( FLOAT( value ) >= 1.0f ) pArray->aVariables() [ cParams - 2 ] ->GetValue( i, Result );
	else if ( FLOAT( value ) <= 0.0f ) pArray->aVariables() [ 1 ] ->GetValue( i, Result );
	else
	{
		TqInt j;
		for ( j = 0; j < cParams; j++ )
		{
			CqVMStackEntry next;
			( pArray->aVariables() [ j ] ) ->GetValue( i, next );
			GETPOINT( next );
			spline[ j ] = POINT( next );
		}

		CqVector4D	res = spline.Evaluate( FLOAT( value ) );
		Result.SetValue( i, res );
	}
	END_FORR
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_sfsplinea( STRINGVAL basis, FLOATVAL value, FLOATARRAYVAL a, DEFPARAMIMPL )
{
	assert( a.fVariable() );
	assert( a.pVariable() ->ArrayLength() > 0 );
	assert( a.pVariable() ->Type() == type_float );

	CqShaderVariableArray* pArray = static_cast<CqShaderVariableArray*>( a.pVariable() );
	TqInt	cParams = pArray->aVariables().size();
	CqSplineCubic spline( cParams );
	TqInt i = 0;
	GETSTRING( basis );
	spline.SetmatBasis( STRING( basis ) );

	INIT_SOR
	CHECKVARY( value )
	CHECKVARY( a )
	CHECKVARY( Result )
	FOR_EACHR
	GETFLOAT( value );
	
	if ( FLOAT( value ) >= 1.0f ) pArray->aVariables() [ cParams - 2 ] ->GetValue( i, Result );
	else if ( FLOAT( value ) <= 0.0f ) pArray->aVariables() [ 1 ] ->GetValue( i, Result );
	else
	{
		TqInt j;
		for ( j = 0; j < cParams; j++ )
		{
			CqVMStackEntry next;
			( pArray->aVariables() [ j ] ) ->GetValue( i, next );
			GETFLOAT( next );
			spline[ j ] = CqVector4D( FLOAT( next ), 0.0f, 0.0f, 1.0f );
		}

		CqVector4D	res = spline.Evaluate( FLOAT( value ) );
		Result.SetValue( i, res.x() );
	}
	END_FORR
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_scsplinea( STRINGVAL basis, FLOATVAL value, COLORARRAYVAL a, DEFPARAMIMPL )
{
	assert( a.fVariable() );
	assert( a.pVariable() ->ArrayLength() > 0 );
	assert( a.pVariable() ->Type() == type_color );

	CqShaderVariableArray* pArray = static_cast<CqShaderVariableArray*>( a.pVariable() );
	TqInt	cParams = pArray->aVariables().size();
	CqSplineCubic spline( cParams );
	TqInt i = 0;
	GETSTRING( basis );
	spline.SetmatBasis( STRING( basis ) );

	INIT_SOR
	CHECKVARY( value )
	CHECKVARY( a )
	CHECKVARY( Result )
	CqColor colTemp;
	FOR_EACHR
	GETFLOAT( value );

	if ( FLOAT( value ) >= 1.0f ) pArray->aVariables() [ cParams - 2 ] ->GetValue( i, Result );
	else if ( FLOAT( value ) <= 0.0f ) pArray->aVariables() [ 1 ] ->GetValue( i, Result );
	else
	{
		TqInt j;
		for ( j = 0; j < cParams; j++ )
		{
			CqVMStackEntry next;
			( pArray->aVariables() [ j ] ) ->GetValue( i, next );
			GETCOLOR( next );
			spline[ j ] = CqVector4D( COLOR( next ).fRed(), COLOR( next ).fGreen(), COLOR( next ).fBlue(), 1.0f );
		}

		CqVector4D	res = spline.Evaluate( FLOAT( value ) );
		Result.SetValue( i, CqColor( res.x(), res.y(), res.z() ) );
	}
	END_FORR
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_spsplinea( STRINGVAL basis, FLOATVAL value, POINTARRAYVAL a, DEFPARAMIMPL )
{
	assert( a.fVariable() );
	assert( a.pVariable() ->ArrayLength() > 0 );
	assert( a.pVariable() ->Type() == type_point );

	CqShaderVariableArray* pArray = static_cast<CqShaderVariableArray*>( a.pVariable() );
	TqInt	cParams = pArray->aVariables().size();
	CqSplineCubic spline( cParams );
	TqInt i = 0;
	GETSTRING( basis );
	spline.SetmatBasis( STRING( basis ) );

	INIT_SOR
	CHECKVARY( value )
	CHECKVARY( a )
	CHECKVARY( Result )
	CqVector3D vecTemp;
	FOR_EACHR
	GETFLOAT( value );

	if ( FLOAT( value ) >= 1.0f ) pArray->aVariables() [ cParams - 2 ] ->GetValue( i, Result );
	else if ( FLOAT( value ) <= 0.0f ) pArray->aVariables() [ 1 ] ->GetValue( i, Result );
	else
	{
		TqInt j;
		for ( j = 0; j < cParams; j++ )
		{
			CqVMStackEntry next;
			( pArray->aVariables() [ j ] )->GetValue( i, next );
			GETPOINT( next );
			spline[ j ] = POINT( next );
		}

		CqVector4D	res = spline.Evaluate( FLOAT( value ) );
		Result.SetValue( i, res );
	}
	END_FORR
}


//----------------------------------------------------------------------
// shadername()
STD_SOIMPL	CqShaderExecEnv::SO_shadername( DEFPARAMIMPL )
{
	INIT_SOR
	CHECKVARY( Result )
	FOR_EACHR
	Result.SetValue( i, pShader->strName() );
	END_FORR
}


//----------------------------------------------------------------------
// shadername(s)
STD_SOIMPL	CqShaderExecEnv::SO_shadername2( STRINGVAL shader, DEFPARAMIMPL )
{
	CqString strName( "" );
	CqString strShader;
	CqShader* pSurface = m_pSurface->pAttributes() ->pshadSurface();
	CqShader* pDisplacement = m_pSurface->pAttributes() ->pshadDisplacement();
	CqShader* pAtmosphere = m_pSurface->pAttributes() ->pshadAtmosphere();

	INIT_SOR
	CHECKVARY( Result )
	FOR_EACHR
	strName = "";
	GETSTRING( shader );
	if ( STRING( shader ).compare( "surface" ) == 0 && pSurface != 0 ) strName = pSurface->strName();
	else if ( STRING( shader ).compare( "displacement" ) == 0 && pDisplacement != 0 ) strName = pDisplacement->strName();
	else if ( STRING( shader ).compare( "atmosphere" ) == 0 && pAtmosphere != 0 ) strName = pAtmosphere->strName();
	Result.SetValue( i, strName );
	END_FORR
}


//----------------------------------------------------------------------
// textureinfo
// support resolution, type, channels, projectionmatrix(*) and viewingmatrix(*)
// User has to provide an array of float (2) for resolution
//                     an string for type
//                     an integer for channels
//                     an array of floats (16) for both projectionmatrix and viewingmatrix
//                     (*) the name must be a shadow map
//

STD_SOIMPL CqShaderExecEnv::SO_textureinfo( STRINGVAL name, STRINGVAL dataname, IqShaderVariable* pV, DEFPARAMIMPL )
{

	TqInt i = 0;
	GETSTRING( name );
	GETSTRING( dataname );

	TqFloat Ret = 0.0f;
	CqVMStackEntry se;
	CqShaderVariableArray* paV = static_cast<CqShaderVariableArray*>( pV );
	CqTextureMap* pMap = NULL;
	CqShadowMap *pSMap = NULL;
	CqLatLongMap *pLMap = NULL;
	CqEnvironmentMap *pEMap = NULL;
	CqTextureMap *pTMap = NULL;


	if ( !pMap && strstr( STRING( name ).c_str(), ".tif" ) )
	{
		pTMap = ( CqTextureMap* ) CqTextureMap::GetTextureMap( STRING( name ).c_str() );
		if ( pTMap && ( pTMap->Type() == MapType_Texture ) )
		{
			pMap = pTMap;
		}
		else if ( pTMap ) delete pTMap;
	}
	if ( !pMap )
	{
		pSMap = ( CqShadowMap * ) CqTextureMap::GetShadowMap( STRING( name ).c_str() );
		if ( pSMap && ( pSMap->Type() == MapType_Shadow ) )
		{
			pMap = pSMap;
		}
		else if ( pSMap ) delete pSMap;
	}

	if ( !pMap )
	{
		pEMap = ( CqEnvironmentMap* ) CqTextureMap::GetEnvironmentMap( STRING( name ).c_str() );
		if ( pEMap && ( pEMap->Type() == MapType_Environment ) )
		{
			pMap = pEMap;
		}
		else if ( pEMap ) delete pEMap;

		pLMap = ( CqLatLongMap * ) CqTextureMap::GetLatLongMap( STRING( name ).c_str() );
		if ( pLMap && ( pLMap->Type() == MapType_LatLong ) )
		{
			pMap = pLMap;
		}
		else if ( pLMap ) delete pLMap;
	}

	if ( !pMap )
	{
		pTMap = ( CqTextureMap* ) CqTextureMap::GetTextureMap( STRING( name ).c_str() );
		if ( pTMap && ( pTMap->Type() == MapType_Texture ) )
		{
			pMap = pTMap;
		}
		else if ( pTMap ) delete pTMap;
	}


	if ( pMap == 0 ) return ;

	if ( STRING( dataname ).compare( "resolution" ) == 0 )
	{
		if ( pV->Type() == type_float && 
		     pV->ArrayLength() > 0 )
		{

			if ( paV->ArrayLength() == 2 )
			{
				se = static_cast<TqFloat>( pMap->XRes() );
				( *paV ) [ 0 ] ->SetValue( se );
				se = static_cast<TqFloat>( pMap->YRes() );
				( *paV ) [ 1 ] ->SetValue( se );
				Ret = 1.0f;

			}
		}
	}
	if ( STRING( dataname ).compare( "type" ) == 0 )
	{
		if ( pV->Type() == type_string )
		{
			if ( pMap->Type() == MapType_Texture )
			{
				se = "texture";
				pV->SetValue( se );
				Ret = 1.0f;

			}
			if ( pMap->Type() == MapType_Bump )
			{
				se = "bump";
				pV->SetValue( se );
				Ret = 1.0f;

			}

			if ( pMap->Type() == MapType_Shadow )
			{
				se = "shadow";
				pV->SetValue( se );
				Ret = 1.0f;

			}
			if ( pMap->Type() == MapType_Environment )
			{
				se = "environment";
				pV->SetValue( se );
				Ret = 1.0f;

			}
			if ( pMap->Type() == MapType_LatLong )
			{
				// both latlong/cube respond the same way according to BMRT
				// It makes sense since both use environment() shader fct.
				se = "environment";
				pV->SetValue( se );
				Ret = 1.0f;

			}


		}
	}

	if ( STRING( dataname ).compare( "channels" ) == 0 )
	{
		if ( pV->Type() == type_float )
		{
			se = ( static_cast<TqFloat>( pMap->SamplesPerPixel() ) );
			pV->SetValue( se );
			Ret = 1.0f;
		}

	}

	if ( STRING( dataname ).compare( "viewingmatrix" ) == 0 )
	{
		if ( pV->Type() == type_float && 
			 pV->ArrayLength() > 0 )
		{
			if ( pSMap ) // && pSMap->Type() == MapType_Shadow)
			{

				if ( paV->ArrayLength() == 16 )
				{

					CqMatrix m = pSMap->matWorldToCamera();

					se = static_cast<TqFloat>( m[ 0 ][ 0 ] );
					( *paV ) [ 0 ] ->SetValue( se );
					se = static_cast<TqFloat>( m[ 0 ][ 1 ] );
					( *paV ) [ 1 ] ->SetValue( se );
					se = static_cast<TqFloat>( m[ 0 ][ 2 ] );
					( *paV ) [ 2 ] ->SetValue( se );
					se = static_cast<TqFloat>( m[ 0 ][ 3 ] );
					( *paV ) [ 3 ] ->SetValue( se );
					se = static_cast<TqFloat>( m[ 1 ][ 0 ] );
					( *paV ) [ 4 ] ->SetValue( se );
					se = static_cast<TqFloat>( m[ 1 ][ 1 ] );
					( *paV ) [ 5 ] ->SetValue( se );
					se = static_cast<TqFloat>( m[ 1 ][ 2 ] );
					( *paV ) [ 6 ] ->SetValue( se );
					se = static_cast<TqFloat>( m[ 1 ][ 3 ] );
					( *paV ) [ 7 ] ->SetValue( se );
					se = static_cast<TqFloat>( m[ 2 ][ 0 ] );
					( *paV ) [ 8 ] ->SetValue( se );
					se = static_cast<TqFloat>( m[ 2 ][ 1 ] );
					( *paV ) [ 9 ] ->SetValue( se );
					se = static_cast<TqFloat>( m[ 2 ][ 2 ] );
					( *paV ) [ 10 ] ->SetValue( se );
					se = static_cast<TqFloat>( m[ 2 ][ 3 ] );
					( *paV ) [ 11 ] ->SetValue( se );
					se = static_cast<TqFloat>( m[ 3 ][ 0 ] );
					( *paV ) [ 12 ] ->SetValue( se );
					se = static_cast<TqFloat>( m[ 3 ][ 1 ] );
					( *paV ) [ 13 ] ->SetValue( se );
					se = static_cast<TqFloat>( m[ 3 ][ 2 ] );
					( *paV ) [ 14 ] ->SetValue( se );
					se = static_cast<TqFloat>( m[ 3 ][ 3 ] );
					( *paV ) [ 15 ] ->SetValue( se );

					Ret = 1.0f;
				}
			}

		}
	}

	if ( STRING( dataname ).compare( "projectionmatrix" ) == 0 )
	{
		if ( pV->Type() == type_float && 
			 pV->ArrayLength() > 0 )
		{
			if ( pSMap )  // && pSMap->Type() == MapType_Shadow)
			{

				if ( paV->ArrayLength() == 16 )
				{

					CqMatrix m = pSMap->matWorldToScreen();

					se = static_cast<TqFloat>( m[ 0 ][ 0 ] );
					( *paV ) [ 0 ] ->SetValue( se );
					se = static_cast<TqFloat>( m[ 0 ][ 1 ] );
					( *paV ) [ 1 ] ->SetValue( se );
					se = static_cast<TqFloat>( m[ 0 ][ 2 ] );
					( *paV ) [ 2 ] ->SetValue( se );
					se = static_cast<TqFloat>( m[ 0 ][ 3 ] );
					( *paV ) [ 3 ] ->SetValue( se );
					se = static_cast<TqFloat>( m[ 1 ][ 0 ] );
					( *paV ) [ 4 ] ->SetValue( se );
					se = static_cast<TqFloat>( m[ 1 ][ 1 ] );
					( *paV ) [ 5 ] ->SetValue( se );
					se = static_cast<TqFloat>( m[ 1 ][ 2 ] );
					( *paV ) [ 6 ] ->SetValue( se );
					se = static_cast<TqFloat>( m[ 1 ][ 3 ] );
					( *paV ) [ 7 ] ->SetValue( se );
					se = static_cast<TqFloat>( m[ 2 ][ 0 ] );
					( *paV ) [ 8 ] ->SetValue( se );
					se = static_cast<TqFloat>( m[ 2 ][ 1 ] );
					( *paV ) [ 9 ] ->SetValue( se );
					se = static_cast<TqFloat>( m[ 2 ][ 2 ] );
					( *paV ) [ 10 ] ->SetValue( se );
					se = static_cast<TqFloat>( m[ 2 ][ 3 ] );
					( *paV ) [ 11 ] ->SetValue( se );
					se = static_cast<TqFloat>( m[ 3 ][ 0 ] );
					( *paV ) [ 12 ] ->SetValue( se );
					se = static_cast<TqFloat>( m[ 3 ][ 1 ] );
					( *paV ) [ 13 ] ->SetValue( se );
					se = static_cast<TqFloat>( m[ 3 ][ 2 ] );
					( *paV ) [ 14 ] ->SetValue( se );
					se = static_cast<TqFloat>( m[ 3 ][ 3 ] );
					( *paV ) [ 15 ] ->SetValue( se );


					Ret = 1.0f;
				}
			}

		}
	}


	delete pMap;

	Result.SetValue( 0, Ret );
	return ;
}


END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------
