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
#include	"shaderexecenv.h"
#include	"texturemap.h"
#include	"spline.h"
#include	"surface.h"
#include	"shadervm.h"


START_NAMESPACE( Aqsis )

static	TqFloat	temp_float;

//----------------------------------------------------------------------
// SO_sprintf
// Helper function to process a string inserting variable, used in printf and format.

static	CqString	SO_sprintf( const char* str, int cParams, IqShaderData** apParams, int varyingindex )
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
								apParams[ ivar++ ] ->GetFloat( f, varyingindex );
								CqString strVal;
								strVal.Format( "%f", f );
								strRes += strVal;
							}
							break;

							case 'p':
							{
								CqVector3D vec;
								apParams[ ivar++ ] ->GetPoint( vec, varyingindex );
								CqString strVal;
								strVal.Format( "%f,%f,%f", vec.x(), vec.y(), vec.z() );
								strRes += strVal;
							}
							break;

							case 'c':
							{
								CqColor col;
								apParams[ ivar++ ] ->GetColor( col, varyingindex );
								CqString strVal;
								strVal.Format( "%f,%f,%f", col.fRed(), col.fGreen(), col.fBlue() );
								strRes += strVal;
							}
							break;

							case 'm':
							{
								CqMatrix mat;
								apParams[ ivar++ ] ->GetMatrix( mat, varyingindex );
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
								apParams[ ivar++ ] ->GetString( stra, varyingindex );
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


void CqShaderExecEnv::ValidateIlluminanceCache( IqShaderData* pP, IqShader* pShader )
{
	// If this is the first call to illuminance this time round, call all lights and setup the Cl and L caches.
	if ( !m_IlluminanceCacheValid )
	{
		TqUint li = 0;
		while ( li < m_pSurface->pAttributes() ->apLights().size() )
		{
			CqLightsource * lp = m_pSurface->pAttributes() ->apLights() [ li ];
			// Initialise the lightsource
			lp->Initialise( uGridRes(), vGridRes() );
			m_Illuminate = 0;
			// Evaluate the lightsource
			if( pP != NULL )
				lp->Evaluate( pP );
			else
				lp->Evaluate( P() );
			li++;
		}
		m_IlluminanceCacheValid = TqTrue;;
	}
}


STD_SOIMPL	CqShaderExecEnv::SO_radians( FLOATVAL degrees, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( degrees )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( degrees );
	SETFLOAT( Result, FLOAT( degrees ) / ( 180.0f / RI_PI ) );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_degrees( FLOATVAL radians, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( radians )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( radians );
	SETFLOAT( Result, ( FLOAT( radians ) / 180.0f ) * RI_PI );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_sin( FLOATVAL a, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( a )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( a );
	SETFLOAT( Result, static_cast<TqFloat>( sin( FLOAT( a ) ) ) );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_asin( FLOATVAL a, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( a )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( a );
	SETFLOAT( Result, static_cast<TqFloat>( asin( FLOAT( a ) ) ) );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_cos( FLOATVAL a, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( a )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( a );
	SETFLOAT( Result, static_cast<TqFloat>( cos( FLOAT( a ) ) ) );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_acos( FLOATVAL a, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( a )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( a );
	SETFLOAT( Result, static_cast<TqFloat>( acos( FLOAT( a ) ) ) );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_tan( FLOATVAL a, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( a )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( a );
	SETFLOAT( Result, static_cast<TqFloat>( tan( FLOAT( a ) ) ) );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_atan( FLOATVAL yoverx, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( yoverx )
	CHECKVARY( Result )
	
	BEGIN_VARYING_SECTION
	GETFLOAT( yoverx );
	SETFLOAT( Result, static_cast<TqFloat>( atan( FLOAT( yoverx ) ) ) );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_atan( FLOATVAL y, FLOATVAL x, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( x )
	CHECKVARY( y )
	CHECKVARY( Result )
	
	BEGIN_VARYING_SECTION
	GETFLOAT( x );
	GETFLOAT( y );
	SETFLOAT( Result, static_cast<TqFloat>( atan2( FLOAT( y ), FLOAT( x ) ) ) );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_pow( FLOATVAL x, FLOATVAL y, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( x )
	CHECKVARY( y )
	CHECKVARY( Result )
	
	BEGIN_VARYING_SECTION
	GETFLOAT( x );
	GETFLOAT( y );
	SETFLOAT( Result, static_cast<TqFloat>( pow( FLOAT( x ), FLOAT( y ) ) ) );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_exp( FLOATVAL x, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( x )
	CHECKVARY( Result )
	
	BEGIN_VARYING_SECTION
	GETFLOAT( x );
	SETFLOAT( Result, static_cast<TqFloat>( exp( FLOAT( x ) ) ) );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_sqrt( FLOATVAL x, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( x )
	CHECKVARY( Result )
	
	BEGIN_VARYING_SECTION
	GETFLOAT( x );
	SETFLOAT( Result, static_cast<TqFloat>( sqrt( FLOAT( x ) ) ) );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_log( FLOATVAL x, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( x )
	CHECKVARY( Result )
	
	BEGIN_VARYING_SECTION
	GETFLOAT( x );
	SETFLOAT( Result, static_cast<TqFloat>( log( FLOAT( x ) ) ) );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_mod( FLOATVAL a, FLOATVAL b, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( a )
	CHECKVARY( b )
	CHECKVARY( Result )
	
	BEGIN_VARYING_SECTION
	GETFLOAT( a );
	GETFLOAT( b );
	TqInt n = static_cast<TqInt>( FLOAT( a ) / FLOAT( b ) );
	TqFloat a2 = FLOAT( a ) - n * FLOAT( b );
	if ( a2 < 0.0f )
		a2 += FLOAT( b );
	SETFLOAT( Result, a2 );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// log(x,base)
STD_SOIMPL	CqShaderExecEnv::SO_log( FLOATVAL x, FLOATVAL base, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( x )
	CHECKVARY( base )
	CHECKVARY( Result )
	
	BEGIN_VARYING_SECTION
	GETFLOAT( x );
	GETFLOAT( base );
	SETFLOAT( Result, static_cast<TqFloat>( log( FLOAT( x ) ) / log( FLOAT( base ) ) ) );
	END_VARYING_SECTION
}


STD_SOIMPL	CqShaderExecEnv::SO_abs( FLOATVAL x, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( x )
	CHECKVARY( Result )
	
	BEGIN_VARYING_SECTION
	GETFLOAT( x );
	SETFLOAT( Result, static_cast<TqFloat>( fabs( FLOAT( x ) ) ) );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_sign( FLOATVAL x, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( x )
	CHECKVARY( Result )
	
	BEGIN_VARYING_SECTION
	GETFLOAT( x );
	SETFLOAT( Result, ( FLOAT( x ) < 0.0f ) ? -1.0f : 1.0f );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_min( FLOATVAL a, FLOATVAL b, DEFPARAMVARIMPL )
{
	INIT_SO

	CHECKVARY( a )
	CHECKVARY( b )
	CHECKVARY( Result )
	
	BEGIN_VARYING_SECTION
	GETFLOAT( a );
	GETFLOAT( b );
	TqFloat fRes = MIN( FLOAT( a ), FLOAT( b ) );
	while ( cParams-- > 0 )
	{
		TqFloat fn;
		apParams[ cParams ]->GetFloat( fn, __iGrid );
		fRes = MIN( fRes, fn );
	}
	SETFLOAT( Result, fRes );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_max( FLOATVAL a, FLOATVAL b, DEFPARAMVARIMPL )
{
	INIT_SO

	CHECKVARY( a )
	CHECKVARY( b )
	CHECKVARY( Result )
	
	BEGIN_VARYING_SECTION
	GETFLOAT( a );
	GETFLOAT( b );
	TqFloat fRes = MAX( FLOAT( a ), FLOAT( b ) );
	while ( cParams-- > 0 )
	{
		TqFloat fn;
		apParams[ cParams ]->GetFloat( fn, __iGrid );
		fRes = MAX( fRes, fn );
	}
	SETFLOAT( Result, fRes );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_pmin( POINTVAL a, POINTVAL b, DEFPARAMVARIMPL )
{
	INIT_SO

	CHECKVARY( a )
	CHECKVARY( b )
	CHECKVARY( Result )
	
	BEGIN_VARYING_SECTION
	GETPOINT( a );
	GETPOINT( b );
	CqVector3D res = VMIN( POINT( a ), POINT( b ) );
	while ( cParams-- > 0 )
	{
		CqVector3D pn;
		apParams[ cParams ]->GetPoint( pn, __iGrid );
		res = VMIN( res, pn );
	}
	SETPOINT( Result, res );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_pmax( POINTVAL a, POINTVAL b, DEFPARAMVARIMPL )
{
	INIT_SO

	CHECKVARY( a )
	CHECKVARY( b )
	CHECKVARY( Result )
	
	BEGIN_VARYING_SECTION
	GETPOINT( a );
	GETPOINT( b );
	CqVector3D res = VMAX( POINT( a ), POINT( b ) );
	while ( cParams-- > 0 )
	{
		CqVector3D pn;
		apParams[ cParams ]->GetPoint( pn, __iGrid );
		res = VMAX( res, pn );
	}
	SETPOINT( Result, res );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_cmin( COLORVAL a, COLORVAL b, DEFPARAMVARIMPL )
{
	INIT_SO

	CHECKVARY( a )
	CHECKVARY( b )
	CHECKVARY( Result )
	
	BEGIN_VARYING_SECTION
	GETCOLOR( a );
	GETCOLOR( b );
	CqColor res = CMIN( COLOR( a ), COLOR( b ) );
	while ( cParams-- > 0 )
	{
		CqColor cn;
		apParams[ cParams ]->GetColor( cn, __iGrid );
		res = CMIN( res, cn );
	}
	SETCOLOR( Result, res );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_cmax( COLORVAL a, COLORVAL b, DEFPARAMVARIMPL )
{
	INIT_SO

	CHECKVARY( a )
	CHECKVARY( b )
	CHECKVARY( Result )
	
	BEGIN_VARYING_SECTION
	GETCOLOR( a );
	GETCOLOR( b );
	CqColor res = CMAX( COLOR( a ), COLOR( b ) );
	while ( cParams-- > 0 )
	{
		CqColor cn;
		apParams[ cParams ]->GetColor( cn, __iGrid );
		res = CMAX( res, cn );
	}
	SETCOLOR( Result, res );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_clamp( FLOATVAL a, FLOATVAL min, FLOATVAL max, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( a )
	CHECKVARY( min )
	CHECKVARY( max )
	CHECKVARY( Result )
	
	BEGIN_VARYING_SECTION
	GETFLOAT( a );
	GETFLOAT( min );
	GETFLOAT( max );
	SETFLOAT( Result, CLAMP( FLOAT( a ), FLOAT( min ), FLOAT( max ) ) );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_pclamp( POINTVAL a, POINTVAL min, POINTVAL max, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( a )
	CHECKVARY( min )
	CHECKVARY( max )
	CHECKVARY( Result )
	
	BEGIN_VARYING_SECTION
	GETPOINT( a );
	GETPOINT( min );
	GETPOINT( max );
	SETPOINT( Result, VCLAMP( POINT( a ), POINT( min ), POINT( max ) ) );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_cclamp( COLORVAL a, COLORVAL min, COLORVAL max, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( a )
	CHECKVARY( min )
	CHECKVARY( max )
	CHECKVARY( Result )
	
	BEGIN_VARYING_SECTION
	GETCOLOR( a );
	GETCOLOR( min );
	GETCOLOR( max );
	SETCOLOR( Result, CCLAMP( COLOR( a ), COLOR( min ), COLOR( max ) ) );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_floor( FLOATVAL x, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( x )
	CHECKVARY( Result )
	
	BEGIN_VARYING_SECTION
	GETFLOAT( x );
	SETFLOAT( Result, static_cast<TqFloat>( FLOOR( FLOAT( x ) ) ) );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_ceil( FLOATVAL x, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( x )
	CHECKVARY( Result )
	
	BEGIN_VARYING_SECTION
	GETFLOAT( x );
	SETFLOAT( Result, static_cast<TqFloat>( CEIL( FLOAT( x ) ) ) );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_round( FLOATVAL x, DEFPARAMIMPL )
{
	INIT_SO
	double v;

	CHECKVARY( x )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( x );
	SETFLOAT( Result, ( modf( FLOAT( x ), &v ) > 0.5f ) ? static_cast<TqFloat>( v ) + 1.0f : static_cast<TqFloat>( v ) );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_step( FLOATVAL min, FLOATVAL value, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( min )
	CHECKVARY( value )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( min );
	GETFLOAT( value );
	SETFLOAT( Result, ( FLOAT( value ) < FLOAT( min ) ) ? 0.0f : 1.0f );
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// smoothstep(min,max,value)
STD_SOIMPL	CqShaderExecEnv::SO_smoothstep( FLOATVAL min, FLOATVAL max, FLOATVAL value, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( value )
	CHECKVARY( min )
	CHECKVARY( max )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( min );
	GETFLOAT( max );
	GETFLOAT( value );
	if ( FLOAT( value ) < FLOAT( min ) )
		SETFLOAT( Result, 0.0f );
	else if ( FLOAT( value ) >= FLOAT( max ) )
		SETFLOAT( Result, 1.0f );
	else
	{
		TqFloat v = ( FLOAT( value ) - FLOAT( min ) ) / ( FLOAT( max ) - FLOAT( min ) );
		SETFLOAT( Result, v * v * ( 3.0f - 2.0f * v ) );
	}
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_fspline( FLOATVAL value, DEFPARAMVARIMPL )
{
	INIT_SO

	CqSplineCubic spline( cParams );

	CHECKVARY( value )
	TqInt v;
	for ( v = 0; v < cParams; v++ )
	{
		CHECKVARY( ( apParams[ v ] ) )
	}
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( value );
	if ( FLOAT( value ) >= 1.0f )
	{
		TqFloat fl;
		apParams[ cParams - 2 ]->GetFloat( fl, __iGrid );
		SETFLOAT( Result, fl );
	}
	else if ( FLOAT( value ) <= 0.0f ) 
	{
		TqFloat ff;
		apParams[ 1 ]->GetFloat( ff, __iGrid );
		SETFLOAT( Result, ff );
	}
	else
	{
		TqInt j;
		for ( j = 0; j < cParams; j++ )
		{
			TqFloat fn;
			apParams[ j ]->GetFloat( fn, __iGrid );
			spline[ j ] = CqVector4D( fn, 0.0f, 0.0f, 1.0f );
		}

		CqVector4D	res = spline.Evaluate( FLOAT( value ) );
		SETFLOAT( Result, res.x() );
	}
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_cspline( FLOATVAL value, DEFPARAMVARIMPL )
{
	INIT_SO

	CqSplineCubic spline( cParams );

	CHECKVARY( value )
	TqInt v;
	for ( v = 0; v < cParams; v++ )
	{
		CHECKVARY( ( apParams[ v ] ) )
	}
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( value );
	if ( FLOAT( value ) >= 1.0f )
	{
		CqColor cl;
		apParams[ cParams - 2 ]->GetColor( cl, __iGrid );
		SETCOLOR( Result, cl );
	}
	else if ( FLOAT( value ) <= 0.0f )
	{
		CqColor cf;
		apParams[ 1 ]->GetColor( cf, __iGrid );
		SETCOLOR( Result, cf );
	}
	else
	{
		TqInt j;
		for ( j = 0; j < cParams; j++ )
		{
			CqColor cn;
			apParams[ j ]->GetColor( cn, __iGrid );
			spline[ j ] = CqVector4D( cn.fRed(), cn.fGreen(), cn.fBlue(), 1.0f );
		}

		CqVector4D	res = spline.Evaluate( FLOAT( value ) );
		SETCOLOR( Result, CqColor( res.x(), res.y(), res.z() ) );
	}
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_pspline( FLOATVAL value, DEFPARAMVARIMPL )
{
	INIT_SO

	CqSplineCubic spline( cParams );

	CHECKVARY( value )
	TqInt v;
	for ( v = 0; v < cParams; v++ )
	{
		CHECKVARY( ( apParams[ v ] ) )
	}
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( value );
	if ( FLOAT( value ) >= 1.0f )
	{
		CqVector3D pl;
		apParams[ cParams - 2 ]->GetPoint( pl, __iGrid );
		SETPOINT( Result, pl );
	}
	else if ( FLOAT( value ) <= 0.0f )
	{
		CqVector3D pf;
		apParams[ 1 ]->GetPoint( pf, __iGrid );
		SETPOINT( Result, pf );
	}
	else
	{
		TqInt j;
		for ( j = 0; j < cParams; j++ )
		{
			CqVector3D pn;
			apParams[ j ]->GetPoint( pn, __iGrid );
			spline[ j ] = pn;
		}

		CqVector3D	res = spline.Evaluate( FLOAT( value ) );
		SETPOINT( Result, res );
	}
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_sfspline( STRINGVAL basis, FLOATVAL value, DEFPARAMVARIMPL )
{
	INIT_SO

	CqSplineCubic spline( cParams );

	CHECKVARY( value )
	TqInt v;
	for ( v = 0; v < cParams; v++ )
	{
		CHECKVARY( ( apParams[ v ] ) )
	}
	CHECKVARY( Result )

	BEGIN_UNIFORM_SECTION
	GETSTRING( basis );
	spline.SetmatBasis( STRING( basis ) );
	END_UNIFORM_SECTION

	BEGIN_VARYING_SECTION
	GETFLOAT( value );
	if ( FLOAT( value ) >= 1.0f )
	{
		TqFloat fl;
		apParams[ cParams - 2 ]->GetFloat( fl, __iGrid );
		SETFLOAT( Result, fl );
	}
	else if ( FLOAT( value ) <= 0.0f )
	{
		TqFloat ff;
		apParams[ 1 ]->GetFloat( ff, __iGrid );
		SETFLOAT( Result, ff );
	}
	else
	{
		TqInt j;
		for ( j = 0; j < cParams; j++ )
		{
			TqFloat fn;
			apParams[ j ]->GetFloat( fn, __iGrid );
			spline[ j ] = CqVector4D( fn, 0.0f, 0.0f, 1.0f );
		}

		CqVector4D	res = spline.Evaluate( FLOAT( value ) );
		SETFLOAT( Result, res.x() );
	}
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_scspline( STRINGVAL basis, FLOATVAL value, DEFPARAMVARIMPL )
{
	INIT_SO

	CqSplineCubic spline( cParams );

	CHECKVARY( value )
	TqInt v;
	for ( v = 0; v < cParams; v++ )
	{
		CHECKVARY( ( apParams[ v ] ) )
	}
	CHECKVARY( Result )

	BEGIN_UNIFORM_SECTION
	GETFLOAT( basis );
	spline.SetmatBasis( STRING( basis) );
	END_UNIFORM_SECTION

	BEGIN_VARYING_SECTION
	GETFLOAT( value );
	if ( FLOAT( value ) >= 1.0f )
	{
		CqColor cl;
		apParams[ cParams - 2 ]->GetColor( cl, __iGrid );
		SETCOLOR( Result, cl );
	}
	else if ( FLOAT( value ) <= 0.0f )
	{
		CqColor cf;
		apParams[ 1 ]->GetColor( cf, __iGrid );
		SETCOLOR( Result, cf );
	}
	else
	{
		TqInt j;
		for ( j = 0; j < cParams; j++ )
		{
			CqColor cn;
			apParams[ j ]->GetColor( cn, __iGrid );
			spline[ j ] = CqVector4D( cn.fRed(), cn.fGreen(), cn.fBlue(), 1.0f );
		}

		CqVector4D	res = spline.Evaluate( FLOAT( value ) );
		SETCOLOR( Result, CqColor( res.x(), res.y(), res.z() ) );
	}
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_spspline( STRINGVAL basis, FLOATVAL value, DEFPARAMVARIMPL )
{
	INIT_SO

	CqSplineCubic spline( cParams );

	CHECKVARY( value )
	TqInt v;
	for ( v = 0; v < cParams; v++ )
	{
		CHECKVARY( ( apParams[ v ] ) )
	}
	CHECKVARY( Result )

	BEGIN_UNIFORM_SECTION
	GETSTRING( basis );
	spline.SetmatBasis( STRING( basis ) );
	END_UNIFORM_SECTION

	BEGIN_VARYING_SECTION
	GETFLOAT( value );
	if ( FLOAT( value ) >= 1.0f )
	{
		CqVector3D pl;
		apParams[ cParams - 2 ]->GetPoint( pl, __iGrid );
		SETPOINT( Result, pl );
	}
	else if ( FLOAT( value ) <= 0.0f )
	{
		CqVector3D pf;
		apParams[ 1 ]->GetPoint( pf, __iGrid );
		SETPOINT( Result, pf );
	}
	else
	{
		TqInt j;
		for ( j = 0; j < cParams; j++ )
		{
			CqVector3D pn;
			apParams[ j ]->GetPoint( pn, __iGrid );
			spline[ j ] = pn;
		}

		CqVector3D	res = spline.Evaluate( FLOAT( value ) );
		SETPOINT( Result, res );
	}
	END_VARYING_SECTION
}


STD_SOIMPL	CqShaderExecEnv::SO_fDu( FLOATVAL p, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( p )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	SETFLOAT( Result, SO_DuType<TqFloat>( p, __iGrid, this ) );
	END_VARYING_SECTION
}


STD_SOIMPL	CqShaderExecEnv::SO_fDv( FLOATVAL p, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( p )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	SETFLOAT( Result, SO_DvType<TqFloat>( p, __iGrid, this ) );
	END_VARYING_SECTION
}


STD_SOIMPL	CqShaderExecEnv::SO_fDeriv( FLOATVAL p, FLOATVAL den, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( p )
	CHECKVARY( den )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	SETFLOAT( Result, SO_DerivType<TqFloat>( p, den, __iGrid, this ) );
	END_VARYING_SECTION
}


STD_SOIMPL	CqShaderExecEnv::SO_cDu( COLORVAL p, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( p )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	SETCOLOR( Result, SO_DuType<CqColor>( p, __iGrid, this ) );
	END_VARYING_SECTION
}


STD_SOIMPL	CqShaderExecEnv::SO_cDv( COLORVAL p, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( p )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	SETCOLOR( Result, SO_DvType<CqColor>( p, __iGrid, this ) );
	END_VARYING_SECTION
}


STD_SOIMPL	CqShaderExecEnv::SO_cDeriv( COLORVAL p, FLOATVAL den, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( p )
	CHECKVARY( den )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	SETCOLOR( Result, SO_DerivType<CqColor>( p, den, __iGrid, this ) );
	END_VARYING_SECTION
}


STD_SOIMPL	CqShaderExecEnv::SO_pDu( POINTVAL p, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( p )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	SETPOINT( Result, SO_DuType<CqVector3D>( p, __iGrid, this ) );
	END_VARYING_SECTION
}


STD_SOIMPL	CqShaderExecEnv::SO_pDv( POINTVAL p, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( p )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	SETPOINT( Result, SO_DvType<CqVector3D>( p, __iGrid, this ) );
	END_VARYING_SECTION
}


STD_SOIMPL	CqShaderExecEnv::SO_pDeriv( POINTVAL p, FLOATVAL den, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( p )
	CHECKVARY( den )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	SETPOINT( Result, SO_DerivType<CqVector3D>( p, den, __iGrid, this ) );
	END_VARYING_SECTION
}


STD_SOIMPL	CqShaderExecEnv::SO_frandom( DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	SETFLOAT( Result, m_random.RandomFloat() );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_crandom( DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	SETCOLOR( Result, CqColor( m_random.RandomFloat(), m_random.RandomFloat(), m_random.RandomFloat() ) );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_prandom( DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	SETCOLOR( Result, CqVector3D( m_random.RandomFloat(), m_random.RandomFloat(), m_random.RandomFloat() ) );
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// noise(v)
STD_SOIMPL	CqShaderExecEnv::SO_fnoise1( FLOATVAL v, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( v )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( v );
	SETFLOAT( Result, ( m_noise.FGNoise1( FLOAT( v ) ) + 1 ) / 2.0f );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// noise(u,v)
STD_SOIMPL CqShaderExecEnv::SO_fnoise2( FLOATVAL u, FLOATVAL v, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( u )
	CHECKVARY( v )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( u );
	GETFLOAT( v );
	SETFLOAT( Result, ( m_noise.FGNoise2( FLOAT( u ), FLOAT( v ) ) + 1 ) / 2.0f );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// noise(p)
STD_SOIMPL CqShaderExecEnv::SO_fnoise3( POINTVAL p, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( p )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETPOINT( p );
	SETFLOAT( Result, ( m_noise.FGNoise3( POINT( p ) ) + 1 ) / 2.0f );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// noise(p,t)
STD_SOIMPL CqShaderExecEnv::SO_fnoise4( POINTVAL p, FLOATVAL t, DEFPARAMIMPL )
{
	// TODO: Do proper 4D noise.
	INIT_SO

	CHECKVARY( p )
	CHECKVARY( t )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETPOINT( p );
	GETFLOAT( t );
	SETFLOAT( Result, ( m_noise.FGNoise3( POINT( p ) ) + 1 ) / 2.0f );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// noise(v)
STD_SOIMPL	CqShaderExecEnv::SO_cnoise1( FLOATVAL v, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( v )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( v );
	SETCOLOR( Result, ( m_noise.CGNoise1( FLOAT( v ) ) + 1 ) / 2.0f );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// noise(u,v)
STD_SOIMPL CqShaderExecEnv::SO_cnoise2( FLOATVAL u, FLOATVAL v, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( u )
	CHECKVARY( v )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( u );
	GETFLOAT( v );
	SETCOLOR( Result, ( m_noise.CGNoise2( FLOAT( u ), FLOAT( v ) ) + 1 ) / 2.0f );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// noise(p)
STD_SOIMPL CqShaderExecEnv::SO_cnoise3( POINTVAL p, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( p )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETPOINT( p );
	SETCOLOR( Result, ( m_noise.CGNoise3( POINT( p ) ) + 1 ) / 2.0f );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// noise(p,t)
STD_SOIMPL CqShaderExecEnv::SO_cnoise4( POINTVAL p, FLOATVAL t, DEFPARAMIMPL )
{
	// TODO: Do proper 4D noise.
	INIT_SO

	CHECKVARY( p )
	CHECKVARY( t )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETPOINT( p );
	GETFLOAT( t );
	SETCOLOR( Result, ( m_noise.CGNoise3( POINT( p ) ) + 1 ) / 2.0f );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// noise(v)
STD_SOIMPL CqShaderExecEnv::SO_pnoise1( FLOATVAL v, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( v )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( v );
	SETPOINT( Result, ( m_noise.PGNoise1( FLOAT( v ) ) + 1 ) / 2.0f );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// noise(u,v)
STD_SOIMPL CqShaderExecEnv::SO_pnoise2( FLOATVAL u, FLOATVAL v, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( u )
	CHECKVARY( v )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( u );
	GETFLOAT( v );
	SETPOINT( Result, ( m_noise.PGNoise2( FLOAT( u ), FLOAT( v ) ) + 1 ) / 2.0f );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// noise(p)
STD_SOIMPL CqShaderExecEnv::SO_pnoise3( POINTVAL p, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( p )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETPOINT( p );
	SETPOINT( Result, ( m_noise.PGNoise3( POINT( p ) ) + 1 ) / 2.0f );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// noise(p,t)
STD_SOIMPL CqShaderExecEnv::SO_pnoise4( POINTVAL p, FLOATVAL t, DEFPARAMIMPL )
{
	// TODO: Do proper 4D noise.
	INIT_SO

	CHECKVARY( p )
	CHECKVARY( t )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETPOINT( p );
	GETFLOAT( t );
	SETPOINT( Result, ( m_noise.PGNoise3( POINT( p ) ) + 1 ) / 2.0f );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// setcomp(c,__iGrid,v)
STD_SOIMPL	CqShaderExecEnv::SO_setcomp( COLORVAL p, FLOATVAL index, FLOATVAL v, DEFVOIDPARAMIMPL )
{
	INIT_SO

	CHECKVARY( p )
	CHECKVARY( v )
	CHECKVARY( index )

	BEGIN_VARYING_SECTION
	GETCOLOR( p );
	GETFLOAT( index );
	GETFLOAT( v );
	COLOR( p )[ FLOAT( index ) ] = FLOAT( v );
	SETCOLOR( p, COLOR( p ) );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// setxcomp(p,v)
STD_SOIMPL	CqShaderExecEnv::SO_setxcomp( POINTVAL p, FLOATVAL v, DEFVOIDPARAMIMPL )
{
	INIT_SO

	CHECKVARY( p )
	CHECKVARY( v )

	BEGIN_VARYING_SECTION
	GETPOINT( p );
	GETFLOAT( v );
	POINT( p ).x( FLOAT( v ) );
	SETPOINT( p, POINT( p ) );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// setycomp(p,v)
STD_SOIMPL	CqShaderExecEnv::SO_setycomp( POINTVAL p, FLOATVAL v, DEFVOIDPARAMIMPL )
{
	INIT_SO

	CHECKVARY( p )
	CHECKVARY( v )

	BEGIN_VARYING_SECTION
	GETPOINT( p );
	GETFLOAT( v );
	POINT( p ).y( FLOAT( v ) );
	SETPOINT( p, POINT( p ) );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// setzcomp(p,v)
STD_SOIMPL	CqShaderExecEnv::SO_setzcomp( POINTVAL p, FLOATVAL v, DEFVOIDPARAMIMPL )
{
	INIT_SO

	CHECKVARY( p )
	CHECKVARY( v )

	BEGIN_VARYING_SECTION
	GETPOINT( p );
	GETFLOAT( v );
	POINT( p ).z( FLOAT( v ) );
	SETPOINT( p, POINT( p ) );
	END_VARYING_SECTION
}



STD_SOIMPL	CqShaderExecEnv::SO_length( VECTORVAL V, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( V )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETVECTOR( V );
	SETFLOAT( Result, VECTOR( V ).Magnitude() );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_distance( POINTVAL P1, POINTVAL P2, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( P1 )
	CHECKVARY( P2 )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETPOINT( P1 );
	GETPOINT( P2 );
	SETFLOAT( Result, ( POINT( P1 ) - POINT( P2 ) ).Magnitude() );
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// area(P)
STD_SOIMPL CqShaderExecEnv::SO_area( POINTVAL p, DEFPARAMIMPL )
{
	INIT_SO

	CqVector3D	vecR;

	CHECKVARY( p )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	if ( m_pSurface )
	{
		TqFloat fdu, fdv;
		du()->GetFloat( fdu, __iGrid );
		dv()->GetFloat( fdv, __iGrid );
		vecR = ( SO_DuType<CqVector3D>( p, __iGrid, this ) * fdu ) % 
			   ( SO_DvType<CqVector3D>( p, __iGrid, this ) * fdv );
		SETFLOAT( Result, vecR.Magnitude() );
	}

	END_VARYING_SECTION
}


STD_SOIMPL	CqShaderExecEnv::SO_normalize( VECTORVAL V, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( V )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETVECTOR( V );
	VECTOR( V ).Unit();
	SETVECTOR( Result, VECTOR( V ) );
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// faceforward(N,I,[Nref])
STD_SOIMPL CqShaderExecEnv::SO_faceforward( NORMALVAL N, VECTORVAL I, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( N )
	CHECKVARY( I )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETNORMAL( N );
	GETVECTOR( I );
	TqFloat s = ( ( ( -VECTOR( I ) ) * NORMAL( N ) ) < 0.0f ) ? -1.0f : 1.0f;
	SETNORMAL( Result, NORMAL( N ) * s );
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// reflect(I,N)
STD_SOIMPL CqShaderExecEnv::SO_reflect( VECTORVAL I, NORMALVAL N, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( I )
	CHECKVARY( N )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETVECTOR( I );
	GETNORMAL( N );
	TqFloat idn = 2.0f * ( VECTOR( I ) * NORMAL( N ) );
	CqVector3D res = VECTOR( I ) - ( idn * NORMAL( N ) );
	SETVECTOR( Result, res );
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// reftact(I,N,eta)
STD_SOIMPL CqShaderExecEnv::SO_refract( VECTORVAL I, NORMALVAL N, FLOATVAL eta, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( I )
	CHECKVARY( N )
	CHECKVARY( eta )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETVECTOR( I );
	GETNORMAL( N );
	GETFLOAT( eta );
	TqFloat IdotN = VECTOR( I ) * NORMAL( N );
	TqFloat feta = FLOAT( eta );
	TqFloat k = 1 - feta * feta * ( 1 - IdotN * IdotN );
	SETVECTOR( Result, ( k < 0.0f ) ? CqVector3D( 0, 0, 0 ) : CqVector3D( feta * VECTOR( I ) - ( feta * IdotN + sqrt( k ) ) * NORMAL( N ) ) );
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// fresnel(I,N,eta,Kr,Kt)
#define SQR(A)	((A)*(A))
STD_SOIMPL CqShaderExecEnv::SO_fresnel( VECTORVAL I, NORMALVAL N, FLOATVAL eta, FLOATVAL Kr, FLOATVAL Kt, DEFVOIDPARAMIMPL )
{
	INIT_SO

	CHECKVARY( I )
	CHECKVARY( N )
	CHECKVARY( eta )
	CHECKVARY( Kr )
	CHECKVARY( Kt )

	BEGIN_VARYING_SECTION
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

	SETFLOAT( Kr, 0.5f * ( fperp2 + fpara2 ) );
	SETFLOAT( Kt, 1.0f - FLOAT( Kr ) );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// fresnel(I,N,eta,Kr,Kt,R,T)
STD_SOIMPL CqShaderExecEnv::SO_fresnel( VECTORVAL I, NORMALVAL N, FLOATVAL eta, FLOATVAL Kr, FLOATVAL Kt, VECTORVAL R, VECTORVAL T, DEFVOIDPARAMIMPL )
{
	INIT_SO

	CHECKVARY( I )
	CHECKVARY( N )
	CHECKVARY( eta )
	CHECKVARY( Kr )
	CHECKVARY( Kt )
	CHECKVARY( R )
	CHECKVARY( T )

	BEGIN_VARYING_SECTION
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
	SETFLOAT( Kr, 0.5f * ( fperp2 + fpara2 ) );
	SETFLOAT( Kt, 1.0f - FLOAT( Kr ) );
	END_VARYING_SECTION

	SO_reflect( I, N, R );
	SO_refract( I, N, eta, T );
}


//----------------------------------------------------------------------
// transform(s,s,P)
STD_SOIMPL CqShaderExecEnv::SO_transform( STRINGVAL fromspace, STRINGVAL tospace, POINTVAL p, DEFPARAMIMPL )
{
	INIT_SO

	assert( pShader != 0 );

	CHECKVARY( p )
	CHECKVARY( Result )

	BEGIN_UNIFORM_SECTION	
	GETSTRING( fromspace );
	GETSTRING( tospace );
	const CqMatrix& mat = QGetRenderContext() ->matSpaceToSpace( STRING( fromspace ).c_str(), STRING( tospace ).c_str(), pShader->matCurrent(), matObjectToWorld() );
	END_UNIFORM_SECTION	

	BEGIN_VARYING_SECTION
	GETPOINT( p );
	SETPOINT( Result, mat * POINT( p ) );
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// transform(s,P)
STD_SOIMPL CqShaderExecEnv::SO_transform( STRINGVAL tospace, POINTVAL p, DEFPARAMIMPL )
{
	INIT_SO

	assert( pShader != 0 );

	CHECKVARY( p )
	CHECKVARY( Result )

	BEGIN_UNIFORM_SECTION
	GETSTRING( tospace );
	const CqMatrix& mat = QGetRenderContext() ->matSpaceToSpace( "current", STRING( tospace ).c_str(), pShader->matCurrent(), matObjectToWorld() );
	END_UNIFORM_SECTION

	BEGIN_VARYING_SECTION
	GETPOINT( p );
	SETPOINT( Result, mat * POINT( p ) );
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// transform(m,P)
STD_SOIMPL CqShaderExecEnv::SO_transformm( MATRIXVAL tospace, POINTVAL p, DEFPARAMIMPL )
{
	INIT_SO

	assert( pShader != 0 );

	CHECKVARY( p )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETMATRIX( tospace );
	GETPOINT( p );
	SETPOINT( Result, MATRIX( tospace ) * POINT( p ) );
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// vtransform(s,s,P)
STD_SOIMPL CqShaderExecEnv::SO_vtransform( STRINGVAL fromspace, STRINGVAL tospace, VECTORVAL p, DEFPARAMIMPL )
{
	INIT_SO

	assert( pShader != 0 );

	CHECKVARY( p )
	CHECKVARY( Result )

	BEGIN_UNIFORM_SECTION
	GETSTRING( fromspace );
	GETSTRING( tospace );
	const CqMatrix& mat = QGetRenderContext() ->matVSpaceToSpace( STRING( fromspace ).c_str(), STRING( tospace ).c_str(), pShader->matCurrent(), matObjectToWorld() );
	END_UNIFORM_SECTION

	BEGIN_VARYING_SECTION
	GETVECTOR( p );
	SETVECTOR( Result, mat * VECTOR( p ) );
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// vtransform(s,P)
STD_SOIMPL CqShaderExecEnv::SO_vtransform( STRINGVAL tospace, VECTORVAL p, DEFPARAMIMPL )
{
	INIT_SO

	assert( pShader != 0 );

	CHECKVARY( p )
	CHECKVARY( Result )

	BEGIN_UNIFORM_SECTION
	GETSTRING( tospace );
	const CqMatrix& mat = QGetRenderContext() ->matVSpaceToSpace( "current", STRING( tospace ).c_str(), pShader->matCurrent(), matObjectToWorld() );
	END_UNIFORM_SECTION

	BEGIN_VARYING_SECTION
	GETVECTOR( p );
	SETVECTOR( Result, mat * VECTOR( p ) );
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// vtransform(m,P)
STD_SOIMPL CqShaderExecEnv::SO_vtransformm( MATRIXVAL tospace, VECTORVAL p, DEFPARAMIMPL )
{
	INIT_SO

	assert( pShader != 0 );

	CHECKVARY( p )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETMATRIX( tospace );
	GETVECTOR( p );
	SETVECTOR( Result, MATRIX( tospace ) * VECTOR( p ) );
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// ntransform(s,s,P)
STD_SOIMPL CqShaderExecEnv::SO_ntransform( STRINGVAL fromspace, STRINGVAL tospace, NORMALVAL p, DEFPARAMIMPL )
{
	INIT_SO

	assert( pShader != 0 );

	CHECKVARY( p )
	CHECKVARY( Result )

	BEGIN_UNIFORM_SECTION
	GETSTRING( fromspace );
	GETSTRING( tospace );
	const CqMatrix& mat = QGetRenderContext() ->matNSpaceToSpace( STRING( fromspace ).c_str(), STRING( tospace ).c_str(), pShader->matCurrent(), matObjectToWorld() );
	BEGIN_UNIFORM_SECTION

	BEGIN_VARYING_SECTION
	GETNORMAL( p );
	SETNORMAL( Result, mat * NORMAL( p ) );
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// ntransform(s,P)
STD_SOIMPL CqShaderExecEnv::SO_ntransform( STRINGVAL tospace, NORMALVAL p, DEFPARAMIMPL )
{
	INIT_SO

	assert( pShader != 0 );

	CHECKVARY( p )
	CHECKVARY( Result )

	BEGIN_UNIFORM_SECTION
	GETSTRING( tospace );
	const CqMatrix& mat = QGetRenderContext() ->matNSpaceToSpace( "current", STRING( tospace ).c_str(), pShader->matCurrent(), matObjectToWorld() );
	BEGIN_UNIFORM_SECTION

	BEGIN_VARYING_SECTION
	GETNORMAL( p );
	SETNORMAL( Result, mat * NORMAL( p ) );
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// ntransform(m,P)
STD_SOIMPL CqShaderExecEnv::SO_ntransformm( MATRIXVAL tospace, NORMALVAL p, DEFPARAMIMPL )
{
	INIT_SO

	assert( pShader != 0 );

	CHECKVARY( p )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETMATRIX( tospace );
	GETNORMAL( p );
	SETNORMAL( Result, MATRIX( tospace ) * NORMAL( p ) );
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// depth(P)
STD_SOIMPL CqShaderExecEnv::SO_depth( POINTVAL p, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( p )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETPOINT( p );
	TqFloat d = POINT( p ).z();
	d = ( d - QGetRenderContext() ->optCurrent().fClippingPlaneNear() ) /
	    ( QGetRenderContext() ->optCurrent().fClippingPlaneFar() - QGetRenderContext() ->optCurrent().fClippingPlaneNear() );
	SETNORMAL( Result, d );
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// calculatenormal(P)
STD_SOIMPL CqShaderExecEnv::SO_calculatenormal( POINTVAL p, DEFPARAMIMPL )
{
	INIT_SO

	// Find out if the orientation is inverted.
	EqOrientation O = pSurface() ->pAttributes() ->eOrientation();
	float neg = 1;
	if ( O != OrientationLH ) neg = -1;

	CHECKVARY( p )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	CqVector3D	dPdu = SO_DuType<CqVector3D>( p, __iGrid, this );
	CqVector3D	dPdv = SO_DvType<CqVector3D>( p, __iGrid, this );
	CqVector3D	N = dPdu % dPdv;
	N.Unit();
	N *= neg;
	SETNORMAL( Result, N );
	END_VARYING_SECTION
}

STD_SOIMPL CqShaderExecEnv::SO_cmix( COLORVAL color0, COLORVAL color1, FLOATVAL value, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( color0 )
	CHECKVARY( color1 )
	CHECKVARY( value )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETCOLOR( color0 );
	GETCOLOR( color1 );
	GETFLOAT( value );
	CqColor c( ( 1.0f - FLOAT( value ) ) * COLOR( color0 ) + FLOAT( value ) * COLOR( color1 ) );
	SETCOLOR( Result, c );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_fmix( FLOATVAL f0, FLOATVAL f1, FLOATVAL value, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( f0 )
	CHECKVARY( f1 )
	CHECKVARY( value )

	BEGIN_VARYING_SECTION
	GETFLOAT( f0 );
	GETFLOAT( f1 );
	GETFLOAT( value );
	TqFloat f( ( 1.0f - FLOAT( value ) ) * FLOAT( f0 ) + FLOAT( value ) * FLOAT( f1 ) );
	SETFLOAT( Result, f );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_pmix( POINTVAL p0, POINTVAL p1, FLOATVAL value, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( p0 )
	CHECKVARY( p1 )
	CHECKVARY( value )

	BEGIN_VARYING_SECTION
	GETPOINT( p0 );
	GETPOINT( p1 );
	GETFLOAT( value );
	CqVector3D p( ( 1.0f - FLOAT( value ) ) * POINT( p0 ) + FLOAT( value ) * POINT( p1 ) );
	SETPOINT( Result, p );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_vmix( VECTORVAL v0, VECTORVAL v1, FLOATVAL value, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( v0 )
	CHECKVARY( v1 )
	CHECKVARY( value )

	BEGIN_VARYING_SECTION
	GETVECTOR( v0 );
	GETVECTOR( v1 );
	GETFLOAT( value );
	CqVector3D v( ( 1.0f - FLOAT( value ) ) * VECTOR( v0 ) + FLOAT( value ) * VECTOR( v1 ) );
	SETVECTOR( Result, v );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_nmix( NORMALVAL n0, NORMALVAL n1, FLOATVAL value, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( n0 )
	CHECKVARY( n1 )
	CHECKVARY( value )

	BEGIN_VARYING_SECTION
	GETNORMAL( n0 );
	GETNORMAL( n1 );
	GETFLOAT( value );
	CqVector3D n( ( 1.0f - FLOAT( value ) ) * NORMAL( n0 ) + FLOAT( value ) * NORMAL( n1 ) );
	SETNORMAL( Result, n );
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// texture(S)
STD_SOIMPL CqShaderExecEnv::SO_ftexture1( STRINGVAL name, FLOATVAL channel, DEFPARAMVARIMPL )
{
	INIT_SO

	GET_TEXTURE_PARAMS;

	BEGIN_UNIFORM_SECTION
	GETSTRING( name );
	GETFLOAT( channel );
	CqTextureMap* pTMap = CqTextureMap::GetTextureMap( STRING( name ).c_str() );
	TqFloat fdu = 0.0f, fdv = 0.0f;
	if( m_pSurface )	
	{
		du()->GetFloat( fdu );
		dv()->GetFloat( fdv );
	}
	END_UNIFORM_SECTION

	__fVarying = TqTrue;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		BEGIN_VARYING_SECTION
		TqFloat swidth = 0.0f, twidth = 0.0f;
		if ( fdu != 0.0f && fdv != 0.0f )
		{
			TqFloat dsdu = SO_DuType<TqFloat>( s(), __iGrid, this );
			swidth = fabs( dsdu * fdu );
			TqFloat dtdu = SO_DuType<TqFloat>( t(), __iGrid, this );
			twidth = fabs( dtdu * fdu );

			TqFloat dsdv = SO_DvType<TqFloat>( s(), __iGrid, this );
			swidth += fabs( dsdv * fdv );
			TqFloat dtdv = SO_DvType<TqFloat>( t(), __iGrid, this );
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

		TqFloat fs, ft;
		s()->GetFloat( fs, __iGrid );
		t()->GetFloat( ft, __iGrid );
		pTMap->SampleMIPMAP( fs, ft, swidth, twidth, _psblur, _ptblur, val);

		// Grab the appropriate channel.
		float fchan = FLOAT( channel );
		if ( fchan >= val.size() )
			SETFLOAT( Result, _pfill );
		else
			SETFLOAT( Result, val[ static_cast<unsigned int>( fchan ) ] );
		END_VARYING_SECTION
	}
	else
	{
		BEGIN_VARYING_SECTION
		SETFLOAT( Result, 0.0f );
		END_VARYING_SECTION
	}
}

//----------------------------------------------------------------------
// texture(S,F,F)
STD_SOIMPL CqShaderExecEnv::SO_ftexture2( STRINGVAL name, FLOATVAL channel, FLOATVAL s, FLOATVAL t, DEFPARAMVARIMPL )
{
	INIT_SO

	GET_TEXTURE_PARAMS;

	BEGIN_UNIFORM_SECTION
	GETSTRING( name );
	GETFLOAT( channel );
	CqTextureMap* pTMap = CqTextureMap::GetTextureMap( STRING( name ).c_str() );
	TqFloat fdu = 0.0f, fdv = 0.0f;
	if( m_pSurface )	
	{
		du()->GetFloat( fdu );
		dv()->GetFloat( fdv );
	}
	END_UNIFORM_SECTION

	__fVarying = TqTrue;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		BEGIN_VARYING_SECTION
		TqFloat swidth = 0.0f, twidth = 0.0f;
		if ( fdu != 0.0f && fdv != 0.0f )
		{
			TqFloat dsdu = SO_DuType<TqFloat>( s, __iGrid, this );
			swidth = fabs( dsdu * fdu );
			TqFloat dtdu = SO_DuType<TqFloat>( t, __iGrid, this );
			twidth = fabs( dtdu * fdu );

			TqFloat dsdv = SO_DvType<TqFloat>( s, __iGrid, this );
			swidth += fabs( dsdv * fdv );
			TqFloat dtdv = SO_DvType<TqFloat>( t, __iGrid, this );
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
			SETFLOAT( Result, _pfill );
		else
			SETFLOAT( Result, val[ static_cast<unsigned int>( fchan ) ] );
		END_VARYING_SECTION
	}
	else
	{
		BEGIN_VARYING_SECTION
		SETFLOAT( Result, 0.0f );	// Default, completely lit
		END_VARYING_SECTION
	}
}

//----------------------------------------------------------------------
// texture(S,F,F,F,F,F,F,F,F)
STD_SOIMPL CqShaderExecEnv::SO_ftexture3( STRINGVAL name, FLOATVAL channel, FLOATVAL s1, FLOATVAL t1, FLOATVAL s2, FLOATVAL t2, FLOATVAL s3, FLOATVAL t3, FLOATVAL s4, FLOATVAL t4, DEFPARAMVARIMPL )
{
	INIT_SO

	GET_TEXTURE_PARAMS;

	BEGIN_UNIFORM_SECTION
	GETSTRING( name );
	GETFLOAT( channel );
	CqTextureMap* pTMap = CqTextureMap::GetTextureMap( STRING( name ).c_str() );
	END_UNIFORM_SECTION

	__fVarying = TqTrue;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		BEGIN_VARYING_SECTION
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
			SETFLOAT( Result, _pfill );
		else
			SETFLOAT( Result, val[ static_cast<unsigned int>( fchan ) ] );
		END_VARYING_SECTION
	}
	else
	{
		BEGIN_VARYING_SECTION
		SETFLOAT( Result, 0.0f );	// Default, completely lit
		END_VARYING_SECTION
	}
}

//----------------------------------------------------------------------
// texture(S)
STD_SOIMPL CqShaderExecEnv::SO_ctexture1( STRINGVAL name, FLOATVAL channel, DEFPARAMVARIMPL )
{
	INIT_SO

	GET_TEXTURE_PARAMS;

	BEGIN_UNIFORM_SECTION
	GETSTRING( name );
	GETFLOAT( channel );
	CqTextureMap* pTMap = CqTextureMap::GetTextureMap( STRING( name ).c_str() );
	TqFloat fdu = 0.0f, fdv = 0.0f;
	if( m_pSurface )	
	{
		du()->GetFloat( fdu );
		dv()->GetFloat( fdv );
	}
	END_UNIFORM_SECTION

	__fVarying = TqTrue;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		BEGIN_VARYING_SECTION
		TqFloat swidth = 0.0f, twidth = 0.0f;
		if ( fdu != 0.0f && fdv != 0.0f )
		{
			TqFloat dsdu = SO_DuType<TqFloat>( s(), __iGrid, this );
			swidth = fabs( dsdu * fdu );
			TqFloat dsdv = SO_DvType<TqFloat>( s(), __iGrid, this );
			swidth += fabs( dsdv * fdv );

			TqFloat dtdu = SO_DuType<TqFloat>( t(), __iGrid, this );
			twidth = fabs( dtdu * fdu );
			TqFloat dtdv = SO_DvType<TqFloat>( t(), __iGrid, this );
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

		TqFloat fs, ft;
		s()->GetFloat( fs, __iGrid );
		t()->GetFloat( ft, __iGrid );
		pTMap->SampleMIPMAP( fs, ft, swidth, twidth, _psblur, _ptblur, val);

		// Grab the appropriate channel.
		float fchan = FLOAT( channel );
		if ( fchan + 2 >= val.size() )
			SETCOLOR( Result, CqColor( _pfill, _pfill, _pfill ) );
		else
			SETCOLOR( Result, CqColor( val[ static_cast<unsigned int>( fchan ) ], val[ static_cast<unsigned int>( fchan ) + 1 ], val[ static_cast<unsigned int>( fchan ) + 2 ] ) );
		END_VARYING_SECTION
	}
	else
	{
		BEGIN_VARYING_SECTION
		SETCOLOR( Result, CqColor( 0, 0, 0 ) );	// Default, no color
		END_VARYING_SECTION
	}
}

//----------------------------------------------------------------------
// texture(S,F,F)
STD_SOIMPL CqShaderExecEnv::SO_ctexture2( STRINGVAL name, FLOATVAL channel, FLOATVAL s, FLOATVAL t, DEFPARAMVARIMPL )
{
	INIT_SO

	GET_TEXTURE_PARAMS;

	BEGIN_UNIFORM_SECTION
	GETSTRING( name );
	GETFLOAT( channel );
	CqTextureMap* pTMap = CqTextureMap::GetTextureMap( STRING( name ).c_str() );
	TqFloat fdu = 0.0f, fdv = 0.0f;
	if( m_pSurface )	
	{
		du()->GetFloat( fdu );
		dv()->GetFloat( fdv );
	}
	END_UNIFORM_SECTION

	__fVarying = TqTrue;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		BEGIN_VARYING_SECTION
		TqFloat swidth = 0.0f, twidth = 0.0f;
		if ( fdu != 0.0f && fdv != 0.0f )
		{
			TqFloat dsdu = SO_DuType<TqFloat>( s, __iGrid, this );
			swidth = fabs( dsdu * fdu );
			TqFloat dsdv = SO_DvType<TqFloat>( s, __iGrid, this );
			swidth += fabs( dsdv * fdv );

			TqFloat dtdu = SO_DuType<TqFloat>( t, __iGrid, this );
			twidth = fabs( dtdu * fdu );
			TqFloat dtdv = SO_DvType<TqFloat>( t, __iGrid, this );
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
			SETCOLOR( Result, CqColor( _pfill, _pfill, _pfill ) );
		else
			SETCOLOR( Result, CqColor( val[ static_cast<unsigned int>( fchan ) ], val[ static_cast<unsigned int>( fchan ) + 1 ], val[ static_cast<unsigned int>( fchan ) + 2 ] ) );
		END_VARYING_SECTION
	}
	else
	{
		BEGIN_VARYING_SECTION
		SETCOLOR( Result, CqColor( 0, 0, 0 ) );	// Default, completely lit
		END_VARYING_SECTION
	}
}

//----------------------------------------------------------------------
// texture(S,F,F,F,F,F,F,F,F)
STD_SOIMPL CqShaderExecEnv::SO_ctexture3( STRINGVAL name, FLOATVAL channel, FLOATVAL s1, FLOATVAL t1, FLOATVAL s2, FLOATVAL t2, FLOATVAL s3, FLOATVAL t3, FLOATVAL s4, FLOATVAL t4, DEFPARAMVARIMPL )
{
	INIT_SO

	GET_TEXTURE_PARAMS;

	BEGIN_UNIFORM_SECTION
	GETSTRING( name );
	GETFLOAT( channel );
	CqTextureMap* pTMap = CqTextureMap::GetTextureMap( STRING( name ).c_str() );
	END_UNIFORM_SECTION

	__fVarying = TqTrue;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		BEGIN_VARYING_SECTION
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
			SETCOLOR( Result, CqColor( _pfill, _pfill, _pfill ) );
		else
			SETCOLOR( Result, CqColor( val[ static_cast<unsigned int>( fchan ) ], val[ static_cast<unsigned int>( fchan ) + 1 ], val[ static_cast<unsigned int>( fchan ) + 2 ] ) );
		END_VARYING_SECTION
	}
	else
	{
		BEGIN_VARYING_SECTION
		SETCOLOR( Result, CqColor( 0, 0, 0 ) );	// Default, completely lit
		END_VARYING_SECTION
	}
}


//----------------------------------------------------------------------
// environment(S,P)
STD_SOIMPL CqShaderExecEnv::SO_fenvironment2( STRINGVAL name, FLOATVAL channel, VECTORVAL R, DEFPARAMVARIMPL )
{
	INIT_SO

	GET_TEXTURE_PARAMS;

	BEGIN_UNIFORM_SECTION
	CqTextureMap* pTMap;
	GETSTRING( name );
	GETFLOAT( channel );
	pTMap = CqTextureMap::GetEnvironmentMap( STRING( name ).c_str() );
	// Try with LatLong map file
	if ( pTMap == 0 )
	{
		pTMap = CqTextureMap::GetLatLongMap( STRING( name ).c_str() );
	}
	TqFloat fdu = 0.0f, fdv = 0.0f;
	if( m_pSurface )
	{
		du()->GetFloat( fdu );
		dv()->GetFloat( fdv );
	}
	END_UNIFORM_SECTION

	__fVarying = TqTrue;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		BEGIN_VARYING_SECTION
		CqVector3D swidth = 0.0f, twidth = 0.0f;
		if ( fdu != 0.0f )
		{
			CqVector3D dRdu = SO_DuType<CqVector3D>( R, __iGrid, this );
			swidth = dRdu * fdu;
		}
		if ( fdv != 0.0f )
		{
			CqVector3D dRdv = SO_DvType<CqVector3D>( R, __iGrid, this );
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
			SETFLOAT( Result, _pfill );
		else
			SETFLOAT( Result, val[ static_cast<unsigned int>( fchan ) ] );
		END_VARYING_SECTION
	}
	else
	{
		BEGIN_VARYING_SECTION
		SETFLOAT( Result, 0.0f );	// Default, completely lit
		END_VARYING_SECTION
	}
}

//----------------------------------------------------------------------
// environment(S,P,P,P,P)
STD_SOIMPL CqShaderExecEnv::SO_fenvironment3( STRINGVAL name, FLOATVAL channel, VECTORVAL R1, VECTORVAL R2, VECTORVAL R3, VECTORVAL R4, DEFPARAMVARIMPL )
{
	INIT_SO

	GET_TEXTURE_PARAMS;

	BEGIN_UNIFORM_SECTION
	CqTextureMap* pTMap;
	GETSTRING( name );
	GETFLOAT( channel );
	pTMap = CqTextureMap::GetEnvironmentMap( STRING( name ).c_str() );
	// Try with LatLong map file
	if ( pTMap == 0 )
	{
		pTMap = CqTextureMap::GetLatLongMap( STRING( name ).c_str() );
	}
	END_UNIFORM_SECTION

	__fVarying = TqTrue;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		BEGIN_VARYING_SECTION
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
			SETFLOAT( Result, _pfill );
		else
			SETFLOAT( Result, val[ static_cast<unsigned int>( fchan ) ] );
		END_VARYING_SECTION
	}
	else
	{
		BEGIN_VARYING_SECTION
		SETFLOAT( Result, 0.0f );	// Default, completely lit
		END_VARYING_SECTION
	}
}


//----------------------------------------------------------------------
// environment(S,P)
STD_SOIMPL CqShaderExecEnv::SO_cenvironment2( STRINGVAL name, FLOATVAL channel, VECTORVAL R, DEFPARAMVARIMPL )
{
	INIT_SO

	GET_TEXTURE_PARAMS;

	BEGIN_UNIFORM_SECTION
	CqTextureMap* pTMap = NULL;
	GETSTRING( name );
	GETFLOAT( channel );
	pTMap = CqTextureMap::GetEnvironmentMap( STRING( name ).c_str() );
	// Try with LatLong map file
	if ( pTMap == 0 )
	{
		pTMap = CqTextureMap::GetLatLongMap( STRING( name ).c_str() );
	}
	TqFloat fdu = 0.0f, fdv = 0.0f;
	if( m_pSurface )
	{
		du()->GetFloat( fdu );
		dv()->GetFloat( fdv );
	}
	END_UNIFORM_SECTION

	__fVarying = TqTrue;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		BEGIN_VARYING_SECTION
		CqVector3D swidth = 0.0f, twidth = 0.0f;
		if ( fdu != 0.0f )
		{
			CqVector3D dRdu = SO_DuType<CqVector3D>( R, __iGrid, this );
			swidth = dRdu * fdu;
		}
		if ( fdv != 0.0f )
		{
			CqVector3D dRdv = SO_DvType<CqVector3D>( R, __iGrid, this );
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
			SETCOLOR( Result, CqColor( _pfill, _pfill, _pfill ) );
		else
			SETCOLOR( Result, CqColor( val[ static_cast<unsigned int>( fchan ) ], val[ static_cast<unsigned int>( fchan ) + 1 ], val[ static_cast<unsigned int>( fchan ) + 2 ] ) );
		END_VARYING_SECTION
	}
	else
	{
		BEGIN_VARYING_SECTION
		SETCOLOR( Result, CqColor( 0.0f, 0.0f, 0.0f ) );	// Default, completely lit
		END_VARYING_SECTION
	}
}

//----------------------------------------------------------------------
// environment(S,P,P,P,P)
STD_SOIMPL CqShaderExecEnv::SO_cenvironment3( STRINGVAL name, FLOATVAL channel, VECTORVAL R1, VECTORVAL R2, VECTORVAL R3, VECTORVAL R4, DEFPARAMVARIMPL )
{
	INIT_SO

	GET_TEXTURE_PARAMS;

	BEGIN_UNIFORM_SECTION
	CqTextureMap* pTMap;
	GETSTRING( name );
	GETFLOAT( channel );
	pTMap = CqTextureMap::GetEnvironmentMap( STRING( name ).c_str() );
	// Try with LatLong map file
	if ( pTMap == 0 )
	{
		pTMap = CqTextureMap::GetLatLongMap( STRING( name ).c_str() );
	}
	BEGIN_UNIFORM_SECTION

	__fVarying = TqTrue;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		BEGIN_VARYING_SECTION
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
			SETCOLOR( Result, CqColor( _pfill, _pfill, _pfill ) );
		else
			SETCOLOR( Result, CqColor( val[ static_cast<unsigned int>( fchan ) ], val[ static_cast<unsigned int>( fchan ) + 1 ], val[ static_cast<unsigned int>( fchan ) + 2 ] ) );
		END_VARYING_SECTION
	}
	else
	{
		BEGIN_VARYING_SECTION
		SETCOLOR( Result, CqColor( 0.0f, 0.0f, 0.0f ) );	// Default, completely lit
		END_VARYING_SECTION
	}
}

//----------------------------------------------------------------------
// bump(S)
STD_SOIMPL CqShaderExecEnv::SO_bump1( STRINGVAL name, FLOATVAL channel, DEFPARAMVARIMPL )
{
	INIT_SO

	__fVarying = TqTrue;

	BEGIN_VARYING_SECTION
	SETPOINT( Result, CqVector3D( 0, 0, 0 ) );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// bump(S,F,F)
STD_SOIMPL CqShaderExecEnv::SO_bump2( STRINGVAL name, FLOATVAL channel, FLOATVAL s, FLOATVAL t, DEFPARAMVARIMPL )
{
	INIT_SO

	__fVarying = TqTrue;

	BEGIN_VARYING_SECTION
	SETPOINT( Result, CqVector3D( 0, 0, 0 ) );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// bump(S,F,F,F,F,F,F,F,F)
STD_SOIMPL CqShaderExecEnv::SO_bump3( STRINGVAL name, FLOATVAL channel, FLOATVAL s1, FLOATVAL t1, FLOATVAL s2, FLOATVAL t2, FLOATVAL s3, FLOATVAL t3, FLOATVAL s4, FLOATVAL t4, DEFPARAMVARIMPL )
{
	INIT_SO

	__fVarying = TqTrue;

	BEGIN_VARYING_SECTION
	SETPOINT( Result, CqVector3D( 0, 0, 0 ) );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// shadow(S,P)
STD_SOIMPL CqShaderExecEnv::SO_shadow( STRINGVAL name, FLOATVAL channel, POINTVAL P, DEFPARAMVARIMPL )
{
	INIT_SO

	GET_TEXTURE_PARAMS;

	BEGIN_UNIFORM_SECTION
	GETSTRING( name );
	GETFLOAT( channel );
	CqShadowMap* pMap = static_cast<CqShadowMap*>( CqShadowMap::GetShadowMap( STRING( name ).c_str() ) );
	END_UNIFORM_SECTION

	__fVarying = TqTrue;
	if ( pMap != 0 && pMap->IsValid() )
	{
		BEGIN_VARYING_SECTION
		CqVector3D swidth = 0.0f, twidth = 0.0f;

		swidth = SO_DerivType<CqVector3D>( P, NULL, __iGrid, this );
		twidth = SO_DerivType<CqVector3D>( P, NULL, __iGrid, this );

		swidth *= _pswidth;
		twidth *= _ptwidth;

		TqFloat fv;
		GETPOINT( P );
		pMap->SampleMap( POINT( P ), swidth, twidth, _psblur, _ptblur, fv );
		SETFLOAT( Result, fv );
		END_VARYING_SECTION
	}
	else
	{
		BEGIN_VARYING_SECTION
		SETFLOAT( Result, 0.0f );	// Default, completely lit
		END_VARYING_SECTION
	}
}

//----------------------------------------------------------------------
// shadow(S,P,P,P,P)

STD_SOIMPL CqShaderExecEnv::SO_shadow1( STRINGVAL name, FLOATVAL channel, POINTVAL P1, POINTVAL P2, POINTVAL P3, POINTVAL P4, DEFPARAMVARIMPL )
{
	INIT_SO

	GET_TEXTURE_PARAMS;

	BEGIN_UNIFORM_SECTION
	GETSTRING( name );
	GETFLOAT( channel );
	CqShadowMap* pMap = static_cast<CqShadowMap*>( CqShadowMap::GetShadowMap( STRING( name ).c_str() ) );
	END_UNIFORM_SECTION

	__fVarying = TqTrue;
	if ( pMap != 0 && pMap->IsValid() )
	{
		BEGIN_VARYING_SECTION
		TqFloat fv;
		GETPOINT( P1 );
		GETPOINT( P2 );
		GETPOINT( P3 );
		GETPOINT( P4 );
		pMap->SampleMap( POINT( P1 ), POINT( P2 ), POINT( P3 ), POINT( P4 ), _psblur, _ptblur, fv );
		SETFLOAT( Result, fv );
		END_VARYING_SECTION
	}
	else
	{
		BEGIN_VARYING_SECTION
		SETFLOAT( Result, 0.0f );	// Default, completely lit
		END_VARYING_SECTION
	}
}


//----------------------------------------------------------------------
// ambient()

STD_SOIMPL CqShaderExecEnv::SO_ambient( DEFPARAMIMPL )
{
	INIT_SO

	// Use the lightsource stack on the current surface
	if ( m_pSurface != 0 )
	{
		// If this is the first call to illuminance this time round, call all lights and setup the Cl and L caches.
		if ( !m_IlluminanceCacheValid )
		{
			ValidateIlluminanceCache( NULL, pShader );
		}

		Result->SetColor( gColBlack );

		for ( TqUint light_index = 0; light_index < m_pSurface->pAttributes() ->apLights().size(); light_index++ )
		{
			__fVarying = TqTrue;

			CqLightsource* lp = m_pSurface->pAttributes() ->apLights() [ light_index ];
			if ( lp->pShader() ->fAmbient() )
			{
				BEGIN_VARYING_SECTION
				// Now Combine the color of all ambient lightsources.
				GETCOLOR( Result );
				CqColor colCl;
				lp->Cl()->GetColor( colCl, __iGrid );
				SETCOLOR( Result, COLOR( Result ) + colCl );

				END_VARYING_SECTION
			}
		}
	}
}


//----------------------------------------------------------------------
// diffuse(N)
STD_SOIMPL CqShaderExecEnv::SO_diffuse( NORMALVAL N, DEFPARAMIMPL )
{
	INIT_SO

	// If the illuminance cache is already OK, then we don't need to bother filling in the illuminance parameters.
	if ( !m_IlluminanceCacheValid )
	{
		ValidateIlluminanceCache( NULL, pShader );
	}

	Result->SetColor( gColBlack );

	__fVarying = TqTrue;
	// SO_init_illuminance returns TRUE if there are any non ambient ligthsources available.
	if ( SO_init_illuminance() )
	{
		do
		{
			// SO_illuminance sets the current state to whether the lightsource illuminates the points or not.
			SO_illuminance( NULL, N, NULL, NULL );
			
			PushState();
			GetCurrentState();
			
			BEGIN_VARYING_SECTION
			
			// Get the light vector and color from the lightsource.
			CqVector3D Ln;
			L()->GetVector( Ln, __iGrid );
			Ln.Unit();

			// Combine the light color into the result
			GETCOLOR( Result );
			GETNORMAL( N );
			CqColor colCl;
			Cl()->GetColor( colCl, __iGrid );
			SETCOLOR( Result, COLOR( Result ) + colCl * ( Ln * NORMAL( N ) ) );
			
			END_VARYING_SECTION
			PopState();
			// SO_advance_illuminance returns TRUE if there are any more non ambient lightsources.
		}
		while ( SO_advance_illuminance() );
	}
}


//----------------------------------------------------------------------
// specular(N,V,roughness)
STD_SOIMPL CqShaderExecEnv::SO_specular( NORMALVAL N, VECTORVAL V, FLOATVAL roughness, DEFPARAMIMPL )
{
	INIT_SO

	// If the illuminance cache is already OK, then we don't need to bother filling in the illuminance parameters.
	if ( !m_IlluminanceCacheValid )
	{
		ValidateIlluminanceCache( NULL, pShader );
	}

	Result->SetColor( gColBlack );
	__fVarying = TqTrue;

	// SO_init_illuminance returns TRUE if there are any non ambient ligthsources available.
	if ( SO_init_illuminance() )
	{
		do
		{
			// SO_illuminance sets the current state to whether the lightsource illuminates the points or not.
			SO_illuminance( NULL, N, NULL, NULL );

			PushState();
			GetCurrentState();
			BEGIN_VARYING_SECTION

			GETVECTOR( V );
			// Get the ligth vector and color from the lightsource
			CqVector3D Ln;
			L()->GetVector( Ln, __iGrid );
			Ln.Unit();
			CqVector3D	H = Ln + VECTOR( V );
			H.Unit();
			
			// Combine the color into the result.
			/// \note The (roughness/8) term emulates the BMRT behaviour for prmanspecular.
			GETCOLOR( Result );
			GETNORMAL( N );
			GETFLOAT( roughness );
			CqColor colCl;
			Cl()->GetColor( colCl, __iGrid );
			SETCOLOR( Result, COLOR( Result ) + colCl * pow( MAX( 0.0f, NORMAL( N ) * H ), 1.0f / ( FLOAT( roughness ) / 8.0f ) ) );

			END_VARYING_SECTION
			PopState();
			// SO_advance_illuminance returns TRUE if there are any more non ambient lightsources.
		}
		while ( SO_advance_illuminance() );
	}
}


//----------------------------------------------------------------------
// phong(N,V,size)
STD_SOIMPL CqShaderExecEnv::SO_phong( NORMALVAL N, VECTORVAL V, FLOATVAL size, DEFPARAMIMPL )
{
	INIT_SO

	IqShaderData* pnV = pSurface()->pAttributes()->pshadSurface()->CreateTemporaryStorage();
	IqShaderData* pnN = pSurface()->pAttributes()->pshadSurface()->CreateTemporaryStorage();
	IqShaderData* pR = pSurface()->pAttributes()->pshadSurface()->CreateTemporaryStorage();

	SO_normalize( V, pnV );
	SO_normalize( N, pnN );

	__fVarying = TqTrue;
	BEGIN_VARYING_SECTION
	CqVector3D vecnV;
	pnV->GetVector( vecnV, __iGrid );
	pnV->SetVector( -vecnV, __iGrid );
	END_VARYING_SECTION

	SO_reflect( pnV, pnN, pR );

	// If the illuminance cache is already OK, then we don't need to bother filling in the illuminance parameters.
	if ( !m_IlluminanceCacheValid )
	{
		ValidateIlluminanceCache( NULL, pShader );
	}

	// Initialise the return value
	Result->SetColor( gColBlack );

	// SO_init_illuminance returns TRUE if there are any non ambient ligthsources available.
	if ( SO_init_illuminance() )
	{
		do
		{
			// SO_illuminance sets the current state to whether the lightsource illuminates the points or not.
			SO_illuminance( NULL, N, NULL, NULL );
			
			PushState();
			GetCurrentState();
			
			BEGIN_VARYING_SECTION
			
			// Get the light vector and color from the loght source.
			CqVector3D Ln;
			L()->GetVector( Ln, __iGrid );
			Ln.Unit();	

			// Now combine the color into the result.
			GETCOLOR( Result );
			CqVector3D vecR;
			pR->GetVector( vecR, __iGrid );
			GETFLOAT( size );
			CqColor colCl;
			Cl()->GetColor( colCl, __iGrid );
			SETCOLOR( Result, COLOR( Result ) + colCl * pow( MAX( 0.0f, vecR * Ln ), FLOAT( size ) ) );

			END_VARYING_SECTION

			PopState();
			// SO_advance_illuminance returns TRUE if there are any more non ambient lightsources.
		}
		while ( SO_advance_illuminance() );
	}
}


//----------------------------------------------------------------------
// trace(P,R)
STD_SOIMPL CqShaderExecEnv::SO_trace( POINTVAL P, VECTORVAL R, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( P )
	CHECKVARY( R )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	SETCOLOR( Result, CqColor( 0, 0, 0 ) );
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// illuminance(P,nsamples)
STD_SOIMPL CqShaderExecEnv::SO_illuminance( POINTVAL P, VECTORVAL Axis, FLOATVAL Angle, FLOATVAL nsamples, DEFVOIDPARAMIMPL )
{
	INIT_SO

	// Fill in the lightsource information, and transfer the results to the shader variables,
	if ( m_pSurface != 0 )
	{
		CqLightsource * lp = m_pSurface->pAttributes() ->apLights() [ m_li ];

		if( NULL != Axis )		CHECKVARY( Axis )
		if( NULL != Angle )		CHECKVARY( Angle )
		if( NULL != nsamples )	CHECKVARY( nsamples )

		BEGIN_VARYING_SECTION
		
		CqVector3D Ln;
		lp->L()->GetVector( Ln, __iGrid );
		Ln = -Ln;

		// Store them locally on the surface.
		L()->SetVector( Ln, __iGrid );
		CqColor colCl;
		lp->Cl()->GetColor( colCl, __iGrid );
		Cl()->SetColor( colCl, __iGrid );
		
		// Check if its within the cone.
		Ln.Unit();
		CqVector3D vecAxis(0,1,0);
		if( NULL != Axis )	Axis->GetVector( vecAxis, __iGrid );
		TqFloat fAngle = RI_PIO2;
		if( NULL != Angle )	Angle->GetFloat( fAngle, __iGrid );
		
		TqFloat cosangle = Ln * vecAxis;
		if ( acos( cosangle ) > fAngle )
			m_CurrentState.SetValue( __iGrid, TqFalse );
		else
			m_CurrentState.SetValue( __iGrid, TqTrue );
		
		END_VARYING_SECTION
	}
}


STD_SOIMPL	CqShaderExecEnv::SO_illuminance( POINTVAL P, FLOATVAL nsamples, DEFVOIDPARAMIMPL )
{
	SO_illuminance( P, NULL, NULL, nsamples );
}


//----------------------------------------------------------------------
// illuminate(P)
STD_SOIMPL CqShaderExecEnv::SO_illuminate( POINTVAL P, VECTORVAL Axis, FLOATVAL Angle, DEFVOIDPARAMIMPL )
{
	INIT_SO

	TqBool res = TqTrue;
	if ( m_Illuminate > 0 ) res = TqFalse;

	__fVarying = TqTrue;
	BEGIN_VARYING_SECTION
	if ( res )
	{
		// Get the point being lit and set the ligth vector.
		GETPOINT( P );
		CqVector3D vecPs;
		Ps()->GetPoint( vecPs, __iGrid );
		L()->SetVector( vecPs - POINT( P ), __iGrid );
		
		// Check if its within the cone.
		CqVector3D Ln;
		L()->GetVector( Ln, __iGrid );
		Ln.Unit();

		CqVector3D vecAxis( 0.0f, 1.0f, 0.0f );
		if( NULL != Axis ) Axis->GetVector( vecAxis, __iGrid );
		TqFloat fAngle = RI_PI;
		if( NULL != Angle ) Angle->GetFloat( fAngle, __iGrid );
		TqFloat cosangle = Ln * vecAxis;
		if ( acos( cosangle ) > fAngle )
		{
			// Make sure we set the light color to zero in the areas that won't be lit.
			Cl()->SetColor( CqColor( 0, 0, 0 ), __iGrid );
			m_CurrentState.SetValue( __iGrid, TqFalse );
		}
		else
			m_CurrentState.SetValue( __iGrid, TqTrue );
	}
	END_VARYING_SECTION

	m_Illuminate++;
}


STD_SOIMPL	CqShaderExecEnv::SO_illuminate( POINTVAL P, DEFVOIDPARAMIMPL )
{
	SO_illuminate( P, NULL, NULL, pShader );
}


//----------------------------------------------------------------------
// solar()
STD_SOIMPL CqShaderExecEnv::SO_solar( VECTORVAL Axis, FLOATVAL Angle, DEFVOIDPARAMIMPL )
{
	// TODO: Check light cone, and exclude points outside.
	INIT_SO

	TqBool res = TqTrue;
	if ( m_Illuminate > 0 ) res = TqFalse;

	__fVarying = TqTrue;
	BEGIN_VARYING_SECTION
	if ( res )
	{
		CqVector3D vecAxis(0.0f, 1.0f, 0.0f);
		if( NULL != Axis )	Axis->GetVector( vecAxis, __iGrid );
		L()->SetVector( vecAxis, __iGrid );
		m_CurrentState.SetValue( __iGrid, TqTrue );
	}
	END_VARYING_SECTION

	m_Illuminate++;
}


STD_SOIMPL	CqShaderExecEnv::SO_solar( DEFVOIDPARAMIMPL )
{
	SO_solar( NULL, NULL, pShader );
}


//----------------------------------------------------------------------
// printf(s,...)

STD_SOIMPL	CqShaderExecEnv::SO_printf( STRINGVAL str, DEFVOIDPARAMVARIMPL )
{
	INIT_SO

	CHECKVARY( str )
	TqInt ii;
	for ( ii = 0; ii < cParams; ii++ )
	{
		CHECKVARY( apParams[ ii ] );
	}

	BEGIN_VARYING_SECTION
	GETSTRING( str );
	CqString strA = SO_sprintf( STRING( str ).c_str(), cParams, apParams, __iGrid );
	QGetRenderContext() ->PrintMessage( SqMessage( 0, 0, strA.c_str() ) );
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// format(s,...)

STD_SOIMPL	CqShaderExecEnv::SO_format( STRINGVAL str, DEFPARAMVARIMPL )
{
	INIT_SO

	CHECKVARY( str )
	int ii;
	for ( ii = 0; ii < cParams; ii++ )
	{
		CHECKVARY( apParams[ ii ] );
	}
	CHECKVARY( Result );

	BEGIN_VARYING_SECTION
	GETSTRING( str );
	CqString strA = SO_sprintf( STRING( str ).c_str(), cParams, apParams, __iGrid );
	SETSTRING( Result, strA );
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// concat(s,s,...)

STD_SOIMPL	CqShaderExecEnv::SO_concat( STRINGVAL stra, STRINGVAL strb, DEFPARAMVARIMPL )
{
	INIT_SO

	CHECKVARY( stra )
	CHECKVARY( strb )
	int ii;
	for ( ii = 0; ii < cParams; ii++ )
	{
		CHECKVARY( apParams[ ii ] );
	}
	CHECKVARY( Result );

	BEGIN_VARYING_SECTION
	GETSTRING( stra );
	CqString strRes = STRING( stra );
	GETSTRING( strb );
	strRes += STRING( strb );
	for ( ii = 0; ii < cParams; ii++ )
	{
		CqString sn;
		apParams[ ii ]->GetString( sn, __iGrid );
		strRes += sn;
	}
	SETSTRING( Result, strRes );
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// noise(v)
STD_SOIMPL CqShaderExecEnv::SO_fcellnoise1( FLOATVAL v, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( v )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( v );
	SETFLOAT( Result, m_cellnoise.FCellNoise1( FLOAT( v ) ) );
	END_VARYING_SECTION
}

STD_SOIMPL CqShaderExecEnv::SO_ccellnoise1( FLOATVAL v, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( v )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( v );
	SETCOLOR( Result, CqColor( m_cellnoise.PCellNoise1( FLOAT( v ) ) ) );
	END_VARYING_SECTION
}

STD_SOIMPL CqShaderExecEnv::SO_pcellnoise1( FLOATVAL v, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( v )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( v );
	SETPOINT( Result, m_cellnoise.PCellNoise1( FLOAT( v ) ) );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// noise(u,v)
STD_SOIMPL CqShaderExecEnv::SO_fcellnoise2( FLOATVAL u, FLOATVAL v, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( u )
	CHECKVARY( v )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( u );
	GETFLOAT( v );
	SETFLOAT( Result, m_cellnoise.FCellNoise2( FLOAT( u ), FLOAT( v ) ) );
	END_VARYING_SECTION
}
STD_SOIMPL CqShaderExecEnv::SO_ccellnoise2( FLOATVAL u, FLOATVAL v, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( u )
	CHECKVARY( v )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( u );
	GETFLOAT( v );
	SETCOLOR( Result, CqColor( m_cellnoise.PCellNoise2( FLOAT( u ), FLOAT( v ) ) ) );
	END_VARYING_SECTION
}
STD_SOIMPL CqShaderExecEnv::SO_pcellnoise2( FLOATVAL u, FLOATVAL v, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( u )
	CHECKVARY( v )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( u );
	GETFLOAT( v );
	SETPOINT( Result, m_cellnoise.PCellNoise2( FLOAT( u ), FLOAT( v ) ) );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// noise(p)
STD_SOIMPL CqShaderExecEnv::SO_fcellnoise3( POINTVAL p, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( p )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETPOINT( p );
	SETFLOAT( Result, m_cellnoise.FCellNoise3( POINT( p ) ) );
	END_VARYING_SECTION
}
STD_SOIMPL CqShaderExecEnv::SO_ccellnoise3( POINTVAL p, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( p )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETPOINT( p );
	SETCOLOR( Result, CqColor( m_cellnoise.PCellNoise3( POINT( p ) ) ) );
	END_VARYING_SECTION
}
STD_SOIMPL CqShaderExecEnv::SO_pcellnoise3( POINTVAL p, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( p )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETPOINT( p );
	SETPOINT( Result, m_cellnoise.PCellNoise3( POINT( p ) ) );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// noise(p,f)
STD_SOIMPL CqShaderExecEnv::SO_fcellnoise4( POINTVAL p, FLOATVAL v, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( p )
	CHECKVARY( v )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETPOINT( p );
	GETFLOAT( v );
	SETFLOAT( Result, m_cellnoise.FCellNoise4( POINT( p ), FLOAT( v ) ) );
	END_VARYING_SECTION
}
STD_SOIMPL CqShaderExecEnv::SO_ccellnoise4( POINTVAL p, FLOATVAL v, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( p )
	CHECKVARY( v )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETPOINT( p );
	GETFLOAT( v );
	SETCOLOR( Result, CqColor( m_cellnoise.PCellNoise4( POINT( p ), FLOAT( v ) ) ) );
	END_VARYING_SECTION
}
STD_SOIMPL CqShaderExecEnv::SO_pcellnoise4( POINTVAL p, FLOATVAL v, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( p )
	CHECKVARY( v )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETPOINT( p );
	GETFLOAT( v );
	SETPOINT( Result, m_cellnoise.PCellNoise4( POINT( p ), FLOAT( v ) ) );
	END_VARYING_SECTION
}



//----------------------------------------------------------------------
// atmosphere
//

STD_SOIMPL CqShaderExecEnv::SO_atmosphere( STRINGVAL name, IqShaderData* pV, DEFPARAMIMPL )
{
	INIT_SO

	IqShader * pAtmosphere = m_pSurface->pAttributes() ->pshadAtmosphere();

	BEGIN_UNIFORM_SECTION
	GETSTRING( name );
	if ( pAtmosphere ) 
		Result->SetValue( pAtmosphere->GetValue( STRING( name ).c_str(), pV ) ? 1.0f : 0.0f, 0 );
	else
		Result->SetValue( 0.0f, 0 );
	END_UNIFORM_SECTION
}


//----------------------------------------------------------------------
// displacement
//

STD_SOIMPL CqShaderExecEnv::SO_displacement( STRINGVAL name, IqShaderData* pV, DEFPARAMIMPL )
{
	INIT_SO

	IqShader * pDisplacement = m_pSurface->pAttributes() ->pshadDisplacement();

	BEGIN_UNIFORM_SECTION
	GETSTRING( name );
	if ( pDisplacement ) 
		Result->SetValue( pDisplacement->GetValue( STRING( name ).c_str(), pV ) ? 1.0f : 0.0f, 0 );
	else
		Result->SetValue( 0.0f, 0 );
	END_UNIFORM_SECTION
}


//----------------------------------------------------------------------
// lightsource
//

STD_SOIMPL CqShaderExecEnv::SO_lightsource( STRINGVAL name, IqShaderData* pV, DEFPARAMIMPL )
{
	INIT_SO

	// This should only be called within an Illuminance construct, so m_li should be valid.
	IqShader* pLightsource = 0;
	
	BEGIN_UNIFORM_SECTION
	GETSTRING( name );
	if ( m_li < static_cast<TqInt>( m_pSurface->pAttributes() ->apLights().size() ) )
		pLightsource = m_pSurface->pAttributes() ->apLights() [ m_li ] ->pShader();
	if ( pLightsource )
		Result->SetValue( pLightsource->GetValue( STRING( name ).c_str(), pV ) ? 1.0f : 0.0f, 0 );
	else
		Result->SetValue( 0.0f, 0 );
	END_UNIFORM_SECTION
}


//----------------------------------------------------------------------
// surface
//

STD_SOIMPL CqShaderExecEnv::SO_surface( STRINGVAL name, IqShaderData* pV, DEFPARAMIMPL )
{
	INIT_SO

	IqShader * pSurface = m_pSurface->pAttributes() ->pshadSurface();

	BEGIN_UNIFORM_SECTION
	GETSTRING( name );
	if ( pSurface )
		Result->SetValue( pSurface->GetValue( STRING( name ).c_str(), pV ) ? 1.0f : 0.0f, 0 );
	else
		Result->SetValue( 0.0f, 0 );
	END_UNIFORM_SECTION
}


//----------------------------------------------------------------------
// attribute
//

STD_SOIMPL CqShaderExecEnv::SO_attribute( STRINGVAL name, IqShaderData* pV, DEFPARAMIMPL )
{
	INIT_SO

	//Find out if it is a specific attribute request
	BEGIN_UNIFORM_SECTION
	GETSTRING( name );
	TqFloat Ret = 0.0f;

	if ( STRING( name ).compare( "ShadingRate" ) == 0 )
	{
		if ( pV->Type() == type_float )
		{
			pV->SetFloat( m_pSurface->pAttributes() ->fEffectiveShadingRate() );
			Ret = 1.0f;
		}
	}
	else if ( STRING( name ).compare( "Sides" ) == 0 )
	{
		if ( pV->Type() == type_float )
		{
			pV->SetFloat( m_pSurface->pAttributes() ->iNumberOfSides() );
			Ret = 1.0f;
		}
	}
	else if ( STRING( name ).compare( "Matte" ) == 0 )
	{
		if ( pV->Type() == type_float )
		{
			pV->SetFloat( m_pSurface->pAttributes() ->bMatteSurfaceFlag() );
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
					switch ( pParam->Type() )
					{
							case type_integer:
							{
								pV->SetFloat( static_cast<TqFloat>( *( static_cast<const CqParameterTyped<TqInt>*>( pParam ) ->pValue() ) ) );
								break;
							}

							case type_float:
							{
								pV->SetFloat( *static_cast<const CqParameterTyped<TqFloat>*>( pParam ) ->pValue() );
								break;
							}

							case type_string:
							{
								pV->SetString( *static_cast<const CqParameterTyped<CqString>*>( pParam ) ->pValue() );
								break;
							}

							case type_point:
							{
								pV->SetPoint( *static_cast<const CqParameterTyped<CqVector3D>*>( pParam ) ->pValue() );
								break;
							}

							case type_vector:
							{
								pV->SetVector( *static_cast<const CqParameterTyped<CqVector3D>*>( pParam ) ->pValue() );
								break;
							}

							case type_normal:
							{
								pV->SetNormal( *static_cast<const CqParameterTyped<CqVector3D>*>( pParam ) ->pValue() );
								break;
							}

							case type_color:
							{
								pV->SetColor( *static_cast<const CqParameterTyped<CqColor>*>( pParam ) ->pValue() );
								break;
							}

							case type_matrix:
							{
								pV->SetMatrix( *static_cast<const CqParameterTyped<CqMatrix>*>( pParam ) ->pValue() );
								break;
							}
					}
					Ret = 1.0f;
				}
			}
		}
	}
	Result->SetValue( Ret, 0 );
	END_UNIFORM_SECTION
}


//----------------------------------------------------------------------
// option
//

STD_SOIMPL CqShaderExecEnv::SO_option( STRINGVAL name, IqShaderData* pV, DEFPARAMIMPL )
{
	INIT_SO

	BEGIN_UNIFORM_SECTION
	//Find out if it is a specific option request
	GETSTRING( name );
	TqFloat Ret = 0.0f;

	if ( STRING( name ).compare( "Format" ) == 0 )
	{
		if ( pV->Type() == type_float &&
		     pV->ArrayLength() > 0 )
		{
			if ( pV->ArrayLength() >= 3 )
			{
				pV->ArrayEntry( 0 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContext() ->optCurrent().iXResolution() ) );
				pV->ArrayEntry( 1 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContext() ->optCurrent().iYResolution() ) );
				pV->ArrayEntry( 2 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContext() ->optCurrent().fPixelAspectRatio() ) );
				Ret = 1.0f;
			}
		}
	}
	else if ( STRING( name ).compare( "CropWindow" ) == 0 )
	{
		if ( pV->Type() == type_float &&
		     pV->ArrayLength() > 0 )
		{
			if ( pV->ArrayLength() >= 4 )
			{
				pV->ArrayEntry( 0 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContext() ->optCurrent().fCropWindowXMin() ) );
				pV->ArrayEntry( 1 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContext() ->optCurrent().fCropWindowXMax() ) );
				pV->ArrayEntry( 2 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContext() ->optCurrent().fCropWindowYMin() ) );
				pV->ArrayEntry( 3 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContext() ->optCurrent().fCropWindowYMax() ) );
				Ret = 1.0f;
			}
		}
	}
	else if ( STRING( name ).compare( "FrameAspectRatio" ) == 0 )
	{
		if ( pV->Type() == type_float )
		{
			pV->SetFloat( static_cast<TqFloat>( QGetRenderContext() ->optCurrent().fFrameAspectRatio() ) );
			Ret = 1.0f;
		}
	}
	else if ( STRING( name ).compare( "DepthOfField" ) == 0 )
	{
		if ( pV->Type() == type_float &&
		     pV->ArrayLength() > 0 )
		{
			if ( pV->ArrayLength() >= 3 )
			{
				pV->ArrayEntry( 0 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContext() ->optCurrent().ffStop() ) );
				pV->ArrayEntry( 1 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContext() ->optCurrent().fFocalLength() ) );
				pV->ArrayEntry( 2 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContext() ->optCurrent().fFocalDistance() ) );
				Ret = 1.0f;
			}
		}
	}
	else if ( STRING( name ).compare( "Shutter" ) == 0 )
	{
		if ( pV->Type() == type_float &&
		     pV->ArrayLength() > 0 )
		{
			if ( pV->ArrayLength() >= 2 )
			{
				pV->ArrayEntry( 0 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContext() ->optCurrent().fShutterOpen() ) );
				pV->ArrayEntry( 1 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContext() ->optCurrent().fShutterClose() ) );
				Ret = 1.0f;
			}
		}
	}
	else if ( STRING( name ).compare( "Clipping" ) == 0 )
	{
		if ( pV->Type() == type_float &&
		     pV->ArrayLength() > 0 )
		{
			if ( pV->ArrayLength() >= 2 )
			{
				pV->ArrayEntry( 0 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContext() ->optCurrent().fClippingPlaneNear() ) );
				pV->ArrayEntry( 1 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContext() ->optCurrent().fClippingPlaneFar() ) );
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
								pV->SetFloat( static_cast<TqFloat>( *( static_cast<const CqParameterTyped<TqInt>*>( pParam ) ->pValue() ) ) );
								break;
							}

							case type_float:
							{
								pV->SetFloat( *static_cast<const CqParameterTyped<TqFloat>*>( pParam ) ->pValue() );
								break;
							}

							case type_string:
							{
								pV->SetString( *static_cast<const CqParameterTyped<CqString>*>( pParam ) ->pValue() );
								break;
							}

							case type_point:
							{
								pV->SetPoint( *static_cast<const CqParameterTyped<CqVector3D>*>( pParam ) ->pValue() );
								break;
							}

							case type_vector:
							{
								pV->SetVector( *static_cast<const CqParameterTyped<CqVector3D>*>( pParam ) ->pValue() );
								break;
							}

							case type_normal:
							{
								pV->SetNormal( *static_cast<const CqParameterTyped<CqVector3D>*>( pParam ) ->pValue() );
								break;
							}

							case type_color:
							{
								pV->SetColor( *static_cast<const CqParameterTyped<CqColor>*>( pParam ) ->pValue() );
								break;
							}

							case type_matrix:
							{
								pV->SetMatrix( *static_cast<const CqParameterTyped<CqMatrix>*>( pParam ) ->pValue() );
								break;
							}
					}
					Ret = 1.0f;
				}
			}
		}
	}
	Result->SetValue( Ret, 0 );
	END_UNIFORM_SECTION
}


//----------------------------------------------------------------------
// rendererinfo
//

STD_SOIMPL CqShaderExecEnv::SO_rendererinfo( STRINGVAL name, IqShaderData* pV, DEFPARAMIMPL )
{
	INIT_SO

	BEGIN_UNIFORM_SECTION
	GETSTRING( name );
	TqFloat Ret = 0.0f;

	if ( STRING( name ).compare( "renderer" ) == 0 )
	{
		if ( pV->Type() == type_string )
		{
			pV->SetString( STRNAME );
			Ret = 1.0f;
		}
	}
	else if ( STRING( name ).compare( "version" ) == 0 )
	{
		if ( pV->Type() == type_float &&
		     pV->ArrayLength() > 0 )
		{
			if ( pV->ArrayLength() >= 4 )
			{
				pV->ArrayEntry( 0 ) ->SetFloat( static_cast<TqFloat>( VERMAJOR ) );
				pV->ArrayEntry( 1 ) ->SetFloat( static_cast<TqFloat>( VERMINOR ) );
				pV->ArrayEntry( 2 ) ->SetFloat( static_cast<TqFloat>( BUILD ) );
				pV->ArrayEntry( 3 ) ->SetFloat( 0.0f );
				Ret = 1.0f;
			}
		}
	}
	else if ( STRING( name ).compare( "versionstring" ) == 0 )
	{
		if ( pV->Type() == type_string )
		{
#if defined(AQSIS_SYSTEM_WIN32) || defined(AQSIS_SYSTEM_MACOSX)
			pV->SetString( VERSION_STR );
#else // AQSIS_SYSTEM_WIN32
			pV->SetString( VERSION );
#endif // !AQSIS_SYSTEM_WIN32
			Ret = 1.0f;
		}
	}
	Result->SetValue( Ret, 0 );
	END_UNIFORM_SECTION
}


//----------------------------------------------------------------------
// incident

STD_SOIMPL CqShaderExecEnv::SO_incident( STRINGVAL name, IqShaderData* pV, DEFPARAMIMPL )
{
	INIT_SO

	BEGIN_UNIFORM_SECTION
	Result->SetValue( 0.0f, 0 );
	END_UNIFORM_SECTION
}


//----------------------------------------------------------------------
// opposite

STD_SOIMPL CqShaderExecEnv::SO_opposite( STRINGVAL name, IqShaderData* pV, DEFPARAMIMPL )
{
	INIT_SO

	BEGIN_UNIFORM_SECTION
	Result->SetValue( 0.0f, 0 );
	END_UNIFORM_SECTION
}


//----------------------------------------------------------------------
// ctransform(s,s,c)
STD_SOIMPL CqShaderExecEnv::SO_ctransform( STRINGVAL fromspace, STRINGVAL tospace, COLORVAL c, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( c )
	CHECKVARY( Result )

	BEGIN_UNIFORM_SECTION
	CqString strfromspace("rgb");
	if( NULL != fromspace) fromspace->GetString( strfromspace );
	GETSTRING( tospace );
	END_UNIFORM_SECTION
	
	BEGIN_VARYING_SECTION
	GETCOLOR( c );
	CqColor res( COLOR( c ) );
	if ( strfromspace.compare( "hsv" ) ) res = COLOR( c ).hsvtorgb();
	else if ( strfromspace.compare( "hsl" ) ) res = COLOR( c ).hsltorgb();
	else if ( strfromspace.compare( "XYZ" ) ) res = COLOR( c ).XYZtorgb();
	else if ( strfromspace.compare( "xyY" ) ) res = COLOR( c ).xyYtorgb();
	else if ( strfromspace.compare( "YIQ" ) ) res = COLOR( c ).YIQtorgb();

	if ( STRING( tospace ).compare( "hsv" ) ) res = COLOR( c ).rgbtohsv();
	else if ( STRING( tospace ).compare( "hsl" ) ) res = COLOR( c ).rgbtohsl();
	else if ( STRING( tospace ).compare( "XYZ" ) ) res = COLOR( c ).rgbtoXYZ();
	else if ( STRING( tospace ).compare( "xyY" ) ) res = COLOR( c ).rgbtoxyY();
	else if ( STRING( tospace ).compare( "YIQ" ) ) res = COLOR( c ).rgbtoYIQ();

	SETCOLOR( Result, res );
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// ctransform(s,c)
STD_SOIMPL CqShaderExecEnv::SO_ctransform( STRINGVAL tospace, COLORVAL c, DEFPARAMIMPL )
{
	assert( pShader != 0 );
	SO_ctransform( NULL, tospace, c, Result, pShader );
}


//----------------------------------------------------------------------
// ctransform(s,c)
STD_SOIMPL CqShaderExecEnv::SO_ptlined( POINTVAL P0, POINTVAL P1, POINTVAL Q, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( P0 )
	CHECKVARY( P1 )
	CHECKVARY( Q )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
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
	SETFLOAT( Result, kDiff.Magnitude() );
	END_VARYING_SECTION
}


STD_SOIMPL	CqShaderExecEnv::SO_inversesqrt( FLOATVAL x, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( x )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( x );
	SETFLOAT( Result, 1.0f / static_cast<TqFloat>( sqrt( FLOAT( x ) ) ) );
	END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_match( STRINGVAL a, STRINGVAL b, DEFPARAMIMPL )
{
	// TODO: Do this properly.
	INIT_SO

	BEGIN_VARYING_SECTION
	SETFLOAT( Result, 0.0f );
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// pnoise(u,period)
STD_SOIMPL CqShaderExecEnv::SO_fpnoise1( FLOATVAL v, FLOATVAL period, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( v )
	CHECKVARY( period )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( v );
	GETFLOAT( period );
	SETFLOAT( Result, ( m_noise.FGNoise1( fmod( FLOAT( v ), FLOAT( period ) ) ) + 1 ) / 2.0f );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// pnoise(u,v,uperiod,vperiod)
STD_SOIMPL CqShaderExecEnv::SO_fpnoise2( FLOATVAL u, FLOATVAL v, FLOATVAL uperiod, FLOATVAL vperiod, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( u )
	CHECKVARY( uperiod )
	CHECKVARY( v )
	CHECKVARY( vperiod )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( u );
	GETFLOAT( v );
	GETFLOAT( uperiod );
	GETFLOAT( vperiod );
	SETFLOAT( Result, ( m_noise.FGNoise2( fmod( FLOAT( u ), FLOAT( uperiod ) ),
	                                        fmod( FLOAT( v ), FLOAT( vperiod ) ) ) + 1 ) / 2.0f );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// pnoise(p,pperiod)
STD_SOIMPL CqShaderExecEnv::SO_fpnoise3( POINTVAL p, POINTVAL pperiod, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( p )
	CHECKVARY( pperiod )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETPOINT( p );
	GETPOINT( pperiod );
	SETFLOAT( Result, ( m_noise.FGNoise3( CqVector3D( fmod( POINT( p ).x(), POINT( pperiod ).x() ),
	                                        fmod( POINT( p ).y(), POINT( pperiod ).y() ),
	                                        fmod( POINT( p ).z(), POINT( pperiod ).z() ) ) ) + 1 ) / 2.0f );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// pnoise(p,t,pperiod,tperiod)
STD_SOIMPL CqShaderExecEnv::SO_fpnoise4( POINTVAL p, FLOATVAL t, POINTVAL pperiod, FLOATVAL tperiod, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( p )
	CHECKVARY( pperiod )
	CHECKVARY( t )
	CHECKVARY( tperiod )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETPOINT( p );
	GETFLOAT( t );
	GETPOINT( pperiod );
	GETFLOAT( tperiod );
	SETFLOAT( Result, ( m_noise.FGNoise3( CqVector3D( fmod( POINT( p ).x(), POINT( pperiod ).x() ),
	                                        fmod( POINT( p ).y(), POINT( pperiod ).y() ),
	                                        fmod( POINT( p ).z(), POINT( pperiod ).z() ) ) ) + 1 ) / 2.0f );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// pnoise(u,period)
STD_SOIMPL CqShaderExecEnv::SO_cpnoise1( FLOATVAL v, FLOATVAL period, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( v )
	CHECKVARY( period )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( v );
	GETFLOAT( period );
	SETCOLOR( Result, ( m_noise.CGNoise1( fmod( FLOAT( v ), FLOAT( period ) ) ) + 1 ) / 2.0f );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// pnoise(u,v,uperiod,vperiod)
STD_SOIMPL CqShaderExecEnv::SO_cpnoise2( FLOATVAL u, FLOATVAL v, FLOATVAL uperiod, FLOATVAL vperiod, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( u )
	CHECKVARY( uperiod )
	CHECKVARY( v )
	CHECKVARY( vperiod )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( u );
	GETFLOAT( v );
	GETFLOAT( uperiod );
	GETFLOAT( vperiod );
	SETCOLOR( Result, ( m_noise.CGNoise2( fmod( FLOAT( u ), FLOAT( uperiod ) ),
	                                        fmod( FLOAT( v ), FLOAT( vperiod ) ) ) + 1 ) / 2.0f );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// pnoise(p,pperiod)
STD_SOIMPL CqShaderExecEnv::SO_cpnoise3( POINTVAL p, POINTVAL pperiod, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( p )
	CHECKVARY( pperiod )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETPOINT( p );
	GETPOINT( pperiod );
	SETCOLOR( Result, ( m_noise.CGNoise3( CqVector3D( fmod( POINT( p ).x(), POINT( pperiod ).x() ),
	                                        fmod( POINT( p ).y(), POINT( pperiod ).y() ),
	                                        fmod( POINT( p ).z(), POINT( pperiod ).z() ) ) ) + 1 ) / 2.0f );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// pnoise(p,t,pperiod,tperiod)
STD_SOIMPL CqShaderExecEnv::SO_cpnoise4( POINTVAL p, FLOATVAL t, POINTVAL pperiod, FLOATVAL tperiod, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( p )
	CHECKVARY( pperiod )
	CHECKVARY( t )
	CHECKVARY( tperiod )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETPOINT( p );
	GETFLOAT( t );
	GETPOINT( pperiod );
	GETFLOAT( tperiod );
	SETCOLOR( Result, ( m_noise.CGNoise3( CqVector3D( fmod( POINT( p ).x(), POINT( pperiod ).x() ),
	                                        fmod( POINT( p ).y(), POINT( pperiod ).y() ),
	                                        fmod( POINT( p ).z(), POINT( pperiod ).z() ) ) ) + 1 ) / 2.0f );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// pnoise(u,period)
STD_SOIMPL CqShaderExecEnv::SO_ppnoise1( FLOATVAL v, FLOATVAL period, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( v )
	CHECKVARY( period )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( v );
	GETFLOAT( period );
	SETPOINT( Result, ( m_noise.PGNoise1( fmod( FLOAT( v ), FLOAT( period ) ) ) + 1 ) / 2.0f );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// pnoise(u,v,uperiod,vperiod)
STD_SOIMPL CqShaderExecEnv::SO_ppnoise2( FLOATVAL u, FLOATVAL v, FLOATVAL uperiod, FLOATVAL vperiod, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( u )
	CHECKVARY( uperiod )
	CHECKVARY( v )
	CHECKVARY( vperiod )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( u );
	GETFLOAT( v );
	GETFLOAT( uperiod );
	GETFLOAT( vperiod );
	SETPOINT( Result, ( m_noise.PGNoise2( fmod( FLOAT( u ), FLOAT( uperiod ) ),
	                                        fmod( FLOAT( v ), FLOAT( vperiod ) ) ) + 1 ) / 2.0f );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// pnoise(p,pperiod)
STD_SOIMPL CqShaderExecEnv::SO_ppnoise3( POINTVAL p, POINTVAL pperiod, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( p )
	CHECKVARY( pperiod )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETPOINT( p );
	GETPOINT( pperiod );
	SETPOINT( Result, ( m_noise.PGNoise3( CqVector3D( fmod( POINT( p ).x(), POINT( pperiod ).x() ),
	                                        fmod( POINT( p ).y(), POINT( pperiod ).y() ),
	                                        fmod( POINT( p ).z(), POINT( pperiod ).z() ) ) ) + 1 ) / 2.0f );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// pnoise(p,t,pperiod,tperiod)
STD_SOIMPL CqShaderExecEnv::SO_ppnoise4( POINTVAL p, FLOATVAL t, POINTVAL pperiod, FLOATVAL tperiod, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( p )
	CHECKVARY( pperiod )
	CHECKVARY( t )
	CHECKVARY( tperiod )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETPOINT( p );
	GETFLOAT( t );
	GETPOINT( pperiod );
	GETFLOAT( tperiod );
	SETPOINT( Result, ( m_noise.PGNoise3( CqVector3D( fmod( POINT( p ).x(), POINT( pperiod ).x() ),
	                                        fmod( POINT( p ).y(), POINT( pperiod ).y() ),
	                                        fmod( POINT( p ).z(), POINT( pperiod ).z() ) ) ) + 1 ) / 2.0f );
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// rotate(Q,angle,P0,P1)
STD_SOIMPL CqShaderExecEnv::SO_rotate( VECTORVAL Q, FLOATVAL angle, POINTVAL P0, POINTVAL P1, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( Q )
	CHECKVARY( angle )
	CHECKVARY( P0 )
	CHECKVARY( P1 )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( angle );
	GETVECTOR( Q );
	GETPOINT( P0 );
	GETPOINT( P1 );
	CqMatrix matR( FLOAT( angle ), POINT( P1 ) - POINT( P0 ) );

	CqVector3D	Res( VECTOR( Q ) );
	Res = matR * Res;

	SETPOINT( Result, Res );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// filterstep(edge,s1)
STD_SOIMPL CqShaderExecEnv::SO_filterstep( FLOATVAL edge, FLOATVAL s1, DEFPARAMVARIMPL )
{
	INIT_SO

	GET_FILTER_PARAMS;

	CHECKVARY( edge )
	CHECKVARY( s1 )
	CHECKVARY( Result )

	BEGIN_UNIFORM_SECTION
	TqFloat fdu, fdv;
	du()->GetFloat( fdu );
	dv()->GetFloat( fdv );
	END_UNIFORM_SECTION

	BEGIN_VARYING_SECTION
	GETFLOAT( s1 );
	GETFLOAT( edge );
	TqFloat dsdu = SO_DuType<TqFloat>( s1, __iGrid, this );
	TqFloat dsdv = SO_DvType<TqFloat>( s1, __iGrid, this );

	TqFloat w = fabs( dsdu * fdu * dsdv * fdv );
	w *= _pswidth;

	SETFLOAT( Result, CLAMP( ( FLOAT( s1 ) + w / 2.0f - FLOAT( edge ) ) / w, 0, 1 ) );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// filterstep(edge,s1,s2)
STD_SOIMPL CqShaderExecEnv::SO_filterstep2( FLOATVAL edge, FLOATVAL s1, FLOATVAL s2, DEFPARAMVARIMPL )
{
	GET_FILTER_PARAMS;

	INIT_SO

	CHECKVARY( edge )
	CHECKVARY( s1 )
	CHECKVARY( s2 )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( edge );
	GETFLOAT( s1 );
	GETFLOAT( s2 );
	TqFloat w = FLOAT( s2 ) - FLOAT( s1 );
	w *= _pswidth;
	SETFLOAT( Result, CLAMP( ( FLOAT( s1 ) + w / 2.0f - FLOAT( edge ) ) / w, 0, 1 ) );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// specularbrdf(L,N,V,rough)
STD_SOIMPL CqShaderExecEnv::SO_specularbrdf( VECTORVAL L, NORMALVAL N, VECTORVAL V, FLOATVAL rough, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( L )
	CHECKVARY( N )
	CHECKVARY( V )
	CHECKVARY( rough )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETVECTOR( L );
	GETVECTOR( V );
	VECTOR( L ).Unit();
	
	CqVector3D	H = VECTOR( L ) + VECTOR( V );	
	H.Unit();
	/// \note The (roughness/8) term emulates the BMRT behaviour for prmanspecular.
	GETNORMAL( N );
	GETFLOAT( rough );
	CqColor colCl;
	Cl()->GetColor( colCl, __iGrid );
	SETCOLOR( Result, colCl * pow( MAX( 0.0f, NORMAL( N ) * H ), 1.0f / ( FLOAT( rough ) / 8.0f ) ) );
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// determinant(m)
STD_SOIMPL CqShaderExecEnv::SO_determinant( MATRIXVAL M, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( M )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETMATRIX( M );
	SETFLOAT( Result, MATRIX( M ).Determinant() );
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// translate(m,v)
STD_SOIMPL CqShaderExecEnv::SO_mtranslate( MATRIXVAL M, VECTORVAL V, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( M )
	CHECKVARY( V )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETMATRIX( M );
	GETVECTOR( V );
	MATRIX( M ).Translate( VECTOR( V ) );
	SETMATRIX( Result, MATRIX( M ) );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// rotate(m,v)
STD_SOIMPL CqShaderExecEnv::SO_mrotate( MATRIXVAL M, FLOATVAL angle, VECTORVAL axis, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( M )
	CHECKVARY( angle )
	CHECKVARY( axis )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETMATRIX( M );
	GETFLOAT( angle );
	GETVECTOR( axis );
	MATRIX( M ).Rotate( FLOAT( angle ), VECTOR( axis ) );
	SETMATRIX( Result, MATRIX( M ) );
	END_VARYING_SECTION
}

//----------------------------------------------------------------------
// scale(m,p)
STD_SOIMPL CqShaderExecEnv::SO_mscale( MATRIXVAL M, POINTVAL S, DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( M )
	CHECKVARY( S )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETPOINT( S );
	GETMATRIX( M );
	MATRIX( M ).Scale( POINT( S ).x(), POINT( S ).y(), POINT( S ).z() );
	SETMATRIX( Result, MATRIX( M ) );
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// setmcomp(p,v)
STD_SOIMPL	CqShaderExecEnv::SO_setmcomp( MATRIXVAL M, FLOATVAL r, FLOATVAL c, FLOATVAL v, DEFVOIDPARAMIMPL )
{
	INIT_SO

	CHECKVARY( M )
	CHECKVARY( r )
	CHECKVARY( c )
	CHECKVARY( v )

	BEGIN_VARYING_SECTION
	GETMATRIX( M );
	GETFLOAT( r );
	GETFLOAT( c );
	GETFLOAT( v );
	MATRIX( M )[ static_cast<TqInt>( FLOAT( r ) ) ][ static_cast<TqInt>( FLOAT( c ) ) ] = FLOAT( v );
	M->SetValue( MATRIX( M ), __iGrid );
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_fsplinea( FLOATVAL value, FLOATARRAYVAL a, DEFPARAMIMPL )
{
	INIT_SO

	assert( a->ArrayLength() > 0 );
	assert( a->Type() == type_float );

	TqInt	cParams = a->ArrayLength();
	CqSplineCubic spline( cParams );

	CHECKVARY( value )
	CHECKVARY( a )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( value );

	TqFloat fTemp;
	if ( FLOAT( value ) >= 1.0f )
	{
		a->ArrayEntry( cParams - 2 ) ->GetFloat( fTemp, __iGrid );
		Result->SetFloat( fTemp, __iGrid );
	}
	else if ( FLOAT( value ) <= 0.0f ) 
	{
		a->ArrayEntry( 1 ) ->GetFloat( fTemp, __iGrid );
		Result->SetFloat( fTemp, __iGrid );
	}
	else
	{
		TqInt j;
		for ( j = 0; j < cParams; j++ )
		{
			a->ArrayEntry( j ) ->GetFloat( fTemp, __iGrid );
			spline[ j ] = CqVector4D( fTemp, 0.0f, 0.0f, 1.0f );
		}

		CqVector4D	res = spline.Evaluate( FLOAT( value ) );
		SETFLOAT( Result, res.x() );
	}
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_csplinea( FLOATVAL value, COLORARRAYVAL a, DEFPARAMIMPL )
{
	INIT_SO

	assert( a->ArrayLength() > 0 );
	assert( a->Type() == type_color );

	TqInt	cParams = a->ArrayLength();
	CqSplineCubic spline( cParams );
	CqColor colTemp;

	CHECKVARY( value )
	CHECKVARY( a )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( value );

	CqColor cTemp;
	if ( FLOAT( value ) >= 1.0f ) 
	{
		a->ArrayEntry( cParams - 2 ) ->GetColor( colTemp, __iGrid );
		Result->SetColor( colTemp, __iGrid );
	}
	else if ( FLOAT( value ) <= 0.0f ) 
	{
		a->ArrayEntry( 1 ) ->GetColor( colTemp, __iGrid );
		Result->SetColor( colTemp, __iGrid );
	}
	else
	{
		TqInt j;
		for ( j = 0; j < cParams; j++ )
		{
			a->ArrayEntry( j ) ->GetColor( colTemp, __iGrid );
			spline[ j ] = CqVector4D( colTemp.fRed(), colTemp.fGreen(), colTemp.fBlue(), 1.0f );
		}

		CqVector4D	res = spline.Evaluate( FLOAT( value ) );
		SETCOLOR( Result, CqColor( res.x(), res.y(), res.z() ) );
	}
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_psplinea( FLOATVAL value, POINTARRAYVAL a, DEFPARAMIMPL )
{
	INIT_SO

	assert( a->ArrayLength() > 0 );
	assert( a->Type() == type_point );

	TqInt	cParams = a->ArrayLength();
	CqSplineCubic spline( cParams );
	CqVector3D vecTemp;

	CHECKVARY( value )
	CHECKVARY( a )
	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	GETFLOAT( value );

	CqVector3D vecTemp;
	if ( FLOAT( value ) >= 1.0f ) 
	{
		a->ArrayEntry( cParams - 2 ) ->GetPoint( vecTemp, __iGrid );
		Result->SetPoint( vecTemp, __iGrid );
	}
	else if ( FLOAT( value ) <= 0.0f ) 
	{
		a->ArrayEntry( 1 ) ->GetPoint( vecTemp, __iGrid );
		Result->SetPoint( vecTemp, __iGrid );
	}
	else
	{
		TqInt j;
		for ( j = 0; j < cParams; j++ )
		{
			a->ArrayEntry( j ) ->GetPoint( vecTemp, __iGrid );
			spline[ j ] = vecTemp;
		}

		CqVector3D	res = spline.Evaluate( FLOAT( value ) );
		SETPOINT( Result, res );
	}
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_sfsplinea( STRINGVAL basis, FLOATVAL value, FLOATARRAYVAL a, DEFPARAMIMPL )
{
	INIT_SO

	assert( a->ArrayLength() > 0 );
	assert( a->Type() == type_float );

	TqInt	cParams = a->ArrayLength();
	CqSplineCubic spline( cParams );

	CHECKVARY( value )
	CHECKVARY( a )
	CHECKVARY( Result )

	BEGIN_UNIFORM_SECTION
	GETSTRING( basis );
	spline.SetmatBasis( STRING( basis ) );
	END_UNIFORM_SECTION

	BEGIN_VARYING_SECTION
	GETFLOAT( value );
	
	TqFloat fTemp;
	if ( FLOAT( value ) >= 1.0f ) 
	{
		a->ArrayEntry( cParams - 2 ) ->GetFloat( fTemp, __iGrid );
		Result->SetFloat( fTemp, __iGrid );
	}
	else if ( FLOAT( value ) <= 0.0f ) 
	{
		a->ArrayEntry( 1 ) ->GetFloat( fTemp, __iGrid );
		Result->SetFloat( fTemp, __iGrid );
	}
	else
	{
		TqInt j;
		for ( j = 0; j < cParams; j++ )
		{
			a->ArrayEntry( j ) ->GetFloat( fTemp, __iGrid );
			spline[ j ] = CqVector4D( fTemp, 0.0f, 0.0f, 1.0f );
		}

		CqVector4D	res = spline.Evaluate( FLOAT( value ) );
		SETFLOAT( Result, res.x() );
	}
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_scsplinea( STRINGVAL basis, FLOATVAL value, COLORARRAYVAL a, DEFPARAMIMPL )
{
	INIT_SO

	assert( a->ArrayLength() > 0 );
	assert( a->Type() == type_color );

	TqInt	cParams = a->ArrayLength();
	CqSplineCubic spline( cParams );
	CqColor colTemp;

	CHECKVARY( value )
	CHECKVARY( a )
	CHECKVARY( Result )

	BEGIN_UNIFORM_SECTION
	GETSTRING( basis );
	spline.SetmatBasis( STRING( basis ) );
	END_UNIFORM_SECTION

	BEGIN_VARYING_SECTION
	GETFLOAT( value );

	CqColor colTemp;
	if ( FLOAT( value ) >= 1.0f ) 
	{
		a->ArrayEntry( cParams - 2 ) ->GetColor( colTemp, __iGrid );
		Result->SetColor( colTemp, __iGrid );
	}
	else if ( FLOAT( value ) <= 0.0f )
	{
		a->ArrayEntry( 1 ) ->GetColor( colTemp, __iGrid );
		Result->SetColor( colTemp, __iGrid );
	}
	else
	{
		TqInt j;
		for ( j = 0; j < cParams; j++ )
		{
			a->ArrayEntry( j ) ->GetColor( colTemp, __iGrid );
			spline[ j ] = CqVector4D( colTemp.fRed(), colTemp.fGreen(), colTemp.fBlue(), 1.0f );
		}

		CqVector4D	res = spline.Evaluate( FLOAT( value ) );
		SETCOLOR( Result, CqColor( res.x(), res.y(), res.z() ) );
	}
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_spsplinea( STRINGVAL basis, FLOATVAL value, POINTARRAYVAL a, DEFPARAMIMPL )
{
	INIT_SO

	assert( a->ArrayLength() > 0 );
	assert( a->Type() == type_point );

	TqInt	cParams = a->ArrayLength();
	CqSplineCubic spline( cParams );
	CqVector3D vecTemp;

	CHECKVARY( value )
	CHECKVARY( a )
	CHECKVARY( Result )

	BEGIN_UNIFORM_SECTION
	GETSTRING( basis );
	spline.SetmatBasis( STRING( basis ) );
	END_UNIFORM_SECTION

	BEGIN_VARYING_SECTION
	GETFLOAT( value );

	CqVector3D vecTemp;
	if ( FLOAT( value ) >= 1.0f )
	{
		a->ArrayEntry( cParams - 2 ) ->GetPoint( vecTemp, __iGrid );
		Result->SetPoint( vecTemp, __iGrid );
	}
	else if ( FLOAT( value ) <= 0.0f ) 
	{
		a->ArrayEntry( 1 ) ->GetPoint( vecTemp, __iGrid );
		Result->SetPoint( vecTemp, __iGrid );
	}
	else
	{
		TqInt j;
		for ( j = 0; j < cParams; j++ )
		{
			a->ArrayEntry( j )->GetPoint( vecTemp, __iGrid );
			spline[ j ] = vecTemp;
		}

		CqVector3D	res = spline.Evaluate( FLOAT( value ) );
		SETPOINT( Result, res );
	}
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// shadername()
STD_SOIMPL	CqShaderExecEnv::SO_shadername( DEFPARAMIMPL )
{
	INIT_SO

	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	SETSTRING( Result, pShader->strName() );
	END_VARYING_SECTION
}


//----------------------------------------------------------------------
// shadername(s)
STD_SOIMPL	CqShaderExecEnv::SO_shadername2( STRINGVAL shader, DEFPARAMIMPL )
{
	INIT_SO

	CqString strName( "" );
	CqString strShader;
	IqShader* pSurface = m_pSurface->pAttributes() ->pshadSurface();
	IqShader* pDisplacement = m_pSurface->pAttributes() ->pshadDisplacement();
	IqShader* pAtmosphere = m_pSurface->pAttributes() ->pshadAtmosphere();

	CHECKVARY( Result )

	BEGIN_VARYING_SECTION
	strName = "";
	GETSTRING( shader );
	if ( STRING( shader ).compare( "surface" ) == 0 && pSurface != 0 ) strName = pSurface->strName();
	else if ( STRING( shader ).compare( "displacement" ) == 0 && pDisplacement != 0 ) strName = pDisplacement->strName();
	else if ( STRING( shader ).compare( "atmosphere" ) == 0 && pAtmosphere != 0 ) strName = pAtmosphere->strName();
	SETSTRING( Result, strName );
	END_VARYING_SECTION
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

STD_SOIMPL CqShaderExecEnv::SO_textureinfo( STRINGVAL name, STRINGVAL dataname, IqShaderData* pV, DEFPARAMIMPL )
{
	INIT_SO

	TqFloat Ret = 0.0f;
	CqTextureMap* pMap = NULL;
	CqShadowMap *pSMap = NULL;
	CqLatLongMap *pLMap = NULL;
	CqEnvironmentMap *pEMap = NULL;
	CqTextureMap *pTMap = NULL;

	BEGIN_UNIFORM_SECTION
	GETSTRING( name );
	GETSTRING( dataname );

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

			if ( pV->ArrayLength() == 2 )
			{
				pV->ArrayEntry( 0 ) ->SetFloat( static_cast<TqFloat>( pMap->XRes() ) );
				pV->ArrayEntry( 1 ) ->SetFloat( static_cast<TqFloat>( pMap->YRes() ) );
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
				pV->SetString( "texture" );
				Ret = 1.0f;

			}
			if ( pMap->Type() == MapType_Bump )
			{
				pV->SetString( "bump" );
				Ret = 1.0f;

			}

			if ( pMap->Type() == MapType_Shadow )
			{
				pV->SetString( "shadow" );
				Ret = 1.0f;

			}
			if ( pMap->Type() == MapType_Environment )
			{
				pV->SetString( "environment" );
				Ret = 1.0f;

			}
			if ( pMap->Type() == MapType_LatLong )
			{
				// both latlong/cube respond the same way according to BMRT
				// It makes sense since both use environment() shader fct.
				pV->SetString( "environment" );
				Ret = 1.0f;

			}


		}
	}

	if ( STRING( dataname ).compare( "channels" ) == 0 )
	{
		if ( pV->Type() == type_float )
		{
			pV->SetFloat( static_cast<TqFloat>( pMap->SamplesPerPixel() ) );
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

				if ( pV->ArrayLength() == 16 )
				{

					CqMatrix m = pSMap->matWorldToCamera();

					pV->ArrayEntry( 0 ) ->SetFloat( static_cast<TqFloat>( m[ 0 ][ 0 ] ) );
					pV->ArrayEntry( 1 ) ->SetFloat( static_cast<TqFloat>( m[ 0 ][ 1 ] ) );
					pV->ArrayEntry( 2 ) ->SetFloat( static_cast<TqFloat>( m[ 0 ][ 2 ] ) );
					pV->ArrayEntry( 3 ) ->SetFloat( static_cast<TqFloat>( m[ 0 ][ 3 ] ) );
					pV->ArrayEntry( 4 ) ->SetFloat( static_cast<TqFloat>( m[ 1 ][ 0 ] ) );
					pV->ArrayEntry( 5 ) ->SetFloat( static_cast<TqFloat>( m[ 1 ][ 1 ] ) );
					pV->ArrayEntry( 6 ) ->SetFloat( static_cast<TqFloat>( m[ 1 ][ 2 ] ) );
					pV->ArrayEntry( 7 ) ->SetFloat( static_cast<TqFloat>( m[ 1 ][ 3 ] ) );
					pV->ArrayEntry( 8 ) ->SetFloat( static_cast<TqFloat>( m[ 2 ][ 0 ] ) );
					pV->ArrayEntry( 9 ) ->SetFloat( static_cast<TqFloat>( m[ 2 ][ 1 ] ) );
					pV->ArrayEntry( 10 ) ->SetFloat( static_cast<TqFloat>( m[ 2 ][ 2 ] ) );
					pV->ArrayEntry( 11 ) ->SetFloat( static_cast<TqFloat>( m[ 2 ][ 3 ] ) );
					pV->ArrayEntry( 12 ) ->SetFloat( static_cast<TqFloat>( m[ 3 ][ 0 ] ) );
					pV->ArrayEntry( 13 ) ->SetFloat( static_cast<TqFloat>( m[ 3 ][ 1 ] ) );
					pV->ArrayEntry( 14 ) ->SetFloat( static_cast<TqFloat>( m[ 3 ][ 2 ] ) );
					pV->ArrayEntry( 15 ) ->SetFloat( static_cast<TqFloat>( m[ 3 ][ 3 ] ) );

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

				if ( pV->ArrayLength() == 16 )
				{

					CqMatrix m = pSMap->matWorldToScreen();

					pV->ArrayEntry( 0 ) ->SetFloat( static_cast<TqFloat>( m[ 0 ][ 0 ] ) );
					pV->ArrayEntry( 1 ) ->SetFloat( static_cast<TqFloat>( m[ 0 ][ 1 ] ) );
					pV->ArrayEntry( 2 ) ->SetFloat( static_cast<TqFloat>( m[ 0 ][ 2 ] ) );
					pV->ArrayEntry( 3 ) ->SetFloat( static_cast<TqFloat>( m[ 0 ][ 3 ] ) );
					pV->ArrayEntry( 4 ) ->SetFloat( static_cast<TqFloat>( m[ 1 ][ 0 ] ) );
					pV->ArrayEntry( 5 ) ->SetFloat( static_cast<TqFloat>( m[ 1 ][ 1 ] ) );
					pV->ArrayEntry( 6 ) ->SetFloat( static_cast<TqFloat>( m[ 1 ][ 2 ] ) );
					pV->ArrayEntry( 7 ) ->SetFloat( static_cast<TqFloat>( m[ 1 ][ 3 ] ) );
					pV->ArrayEntry( 8 ) ->SetFloat( static_cast<TqFloat>( m[ 2 ][ 0 ] ) );
					pV->ArrayEntry( 9 ) ->SetFloat( static_cast<TqFloat>( m[ 2 ][ 1 ] ) );
					pV->ArrayEntry( 10 ) ->SetFloat( static_cast<TqFloat>( m[ 2 ][ 2 ] ) );
					pV->ArrayEntry( 11 ) ->SetFloat( static_cast<TqFloat>( m[ 2 ][ 3 ] ) );
					pV->ArrayEntry( 12 ) ->SetFloat( static_cast<TqFloat>( m[ 3 ][ 0 ] ) );
					pV->ArrayEntry( 13 ) ->SetFloat( static_cast<TqFloat>( m[ 3 ][ 1 ] ) );
					pV->ArrayEntry( 14 ) ->SetFloat( static_cast<TqFloat>( m[ 3 ][ 2 ] ) );
					pV->ArrayEntry( 15 ) ->SetFloat( static_cast<TqFloat>( m[ 3 ][ 3 ] ) );


					Ret = 1.0f;
				}
			}

		}
	}

	delete pMap;

	SETFLOAT( Result, Ret );
	END_UNIFORM_SECTION
}


END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------
