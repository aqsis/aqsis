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

#include	"aqsis.h"

#include	<math.h>
#include	<map>
#include	<vector>
#include	<string>
#include	<stdio.h>

#include	"shaderexecenv.h"
#include	"spline.h"
#include	"shadervm.h"
#include	"irenderer.h"
#include	"itexturemap.h"
#include	"ilightsource.h"
#include	"version.h"

START_NAMESPACE( Aqsis )

IqRenderer* QGetRenderContextI();


//----------------------------------------------------------------------
// SO_sprintf
// Helper function to process a string inserting variable, used in printf and format.

static	CqString	SO_sprintf( const char* str, int cParams, IqShaderData** apParams, int varyingindex )
{
    CqString strRes( "" );
    CqString strTrans = str;
    strTrans = strTrans.TranslateEscapes();

    TqUint i = 0;
    TqUint ivar = 0;
    while ( i < strTrans.size() )
    {
        switch ( strTrans[ i ] )
        {
        case '%':   	// Insert variable.
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
    while ( m_li < m_pAttributes ->cLights() &&
            m_pAttributes ->pLight( m_li ) ->pShader() ->fAmbient() )
    {
        m_li++;
    }
    if ( m_li < m_pAttributes ->cLights() )
        return ( TqTrue );
    else
        return ( TqFalse );
}


void CqShaderExecEnv::ValidateIlluminanceCache( IqShaderData* pP, IqShaderData* pN, IqShader* pShader )
{
    // If this is the first call to illuminance this time round, call all lights and setup the Cl and L caches.
    if ( !m_IlluminanceCacheValid )
    {
        IqShaderData* Ns = (pN != NULL )? pN : N();
        IqShaderData* Ps = (pP != NULL )? pP : P();
        TqUint li = 0;
        while ( li < m_pAttributes ->cLights() )
        {
            IqLightsource * lp = m_pAttributes ->pLight( li );
            // Initialise the lightsource
            lp->Initialise( uGridRes(), vGridRes() );
            m_Illuminate = 0;
            // Evaluate the lightsource
            lp->Evaluate( Ps, Ns, m_pCurrentSurface );
            li++;
        }
        m_IlluminanceCacheValid = TqTrue;;
    }
}


STD_SOIMPL	CqShaderExecEnv::SO_radians( FLOATVAL degrees, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_radians );

    INIT_SO

    CHECKVARY( degrees )
    CHECKVARY( Result )

    BEGIN_VARYING_SECTION
    GETFLOAT( degrees );
    SETFLOAT( Result, RAD( FLOAT( degrees ) ) );
    END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_degrees( FLOATVAL radians, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_degrees );

    INIT_SO

    CHECKVARY( radians )
    CHECKVARY( Result )

    BEGIN_VARYING_SECTION
    GETFLOAT( radians );
    SETFLOAT( Result, DEG( FLOAT( radians ) ) );
    END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_sin( FLOATVAL a, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_sin );

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
    STATS_INC( SHD_so_asin );

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
    STATS_INC( SHD_so_cos );

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
    STATS_INC( SHD_so_acos );

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
    STATS_INC( SHD_so_tan );

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
    STATS_INC( SHD_so_atan );

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
    STATS_INC( SHD_so_atan );

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
    STATS_INC( SHD_so_pow );

    INIT_SO

    CHECKVARY( x )
    CHECKVARY( y )
    CHECKVARY( Result )

    BEGIN_VARYING_SECTION
    GETFLOAT( x );
    GETFLOAT( y );
    TqFloat yy = FLOAT( y );
    TqFloat xx = FLOAT( x );
    if ( xx < 0.0f ) yy = FLOOR( yy );
    SETFLOAT( Result, static_cast<TqFloat>( pow( xx, yy ) ) );
    END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_exp( FLOATVAL x, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_exp );

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
    STATS_INC( SHD_so_sqrt );

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
    STATS_INC( SHD_so_log );

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
    STATS_INC( SHD_so_mod );

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
    STATS_INC( SHD_so_log );

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
    STATS_INC( SHD_so_abs );

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
    STATS_INC( SHD_so_sign );

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
    STATS_INC( SHD_so_min );

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
        apParams[ cParams ] ->GetFloat( fn, __iGrid );
        fRes = MIN( fRes, fn );
    }
    SETFLOAT( Result, fRes );
    END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_max( FLOATVAL a, FLOATVAL b, DEFPARAMVARIMPL )
{
    STATS_INC( SHD_so_max );

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
        apParams[ cParams ] ->GetFloat( fn, __iGrid );
        fRes = MAX( fRes, fn );
    }
    SETFLOAT( Result, fRes );
    END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_pmin( POINTVAL a, POINTVAL b, DEFPARAMVARIMPL )
{
    STATS_INC( SHD_so_pmin );

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
        apParams[ cParams ] ->GetPoint( pn, __iGrid );
        res = VMIN( res, pn );
    }
    SETPOINT( Result, res );
    END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_pmax( POINTVAL a, POINTVAL b, DEFPARAMVARIMPL )
{
    STATS_INC( SHD_so_pmax );

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
        apParams[ cParams ] ->GetPoint( pn, __iGrid );
        res = VMAX( res, pn );
    }
    SETPOINT( Result, res );
    END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_cmin( COLORVAL a, COLORVAL b, DEFPARAMVARIMPL )
{
    STATS_INC( SHD_so_cmin );

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
        apParams[ cParams ] ->GetColor( cn, __iGrid );
        res = CMIN( res, cn );
    }
    SETCOLOR( Result, res );
    END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_cmax( COLORVAL a, COLORVAL b, DEFPARAMVARIMPL )
{
    STATS_INC( SHD_so_cmax );

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
        apParams[ cParams ] ->GetColor( cn, __iGrid );
        res = CMAX( res, cn );
    }
    SETCOLOR( Result, res );
    END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_clamp( FLOATVAL a, FLOATVAL _min, FLOATVAL _max, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_clamp );

    INIT_SO

    CHECKVARY( a )
    CHECKVARY( _min )
    CHECKVARY( _max )
    CHECKVARY( Result )

    BEGIN_VARYING_SECTION
    GETFLOAT( a );
    GETFLOAT( _min );
    GETFLOAT( _max );
    SETFLOAT( Result, CLAMP( FLOAT( a ), FLOAT( _min ), FLOAT( _max ) ) );
    END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_pclamp( POINTVAL a, POINTVAL _min, POINTVAL _max, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_pclamp );

    INIT_SO

    CHECKVARY( a )
    CHECKVARY( _min )
    CHECKVARY( _max )
    CHECKVARY( Result )

    BEGIN_VARYING_SECTION
    GETPOINT( a );
    GETPOINT( _min );
    GETPOINT( _max );
    SETPOINT( Result, VCLAMP( POINT( a ), POINT( _min ), POINT( _max ) ) );
    END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_cclamp( COLORVAL a, COLORVAL _min, COLORVAL _max, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_cclamp );

    INIT_SO

    CHECKVARY( a )
    CHECKVARY( _min )
    CHECKVARY( _max )
    CHECKVARY( Result )

    BEGIN_VARYING_SECTION
    GETCOLOR( a );
    GETCOLOR( _min );
    GETCOLOR( _max );
    SETCOLOR( Result, CCLAMP( COLOR( a ), COLOR( _min ), COLOR( _max ) ) );
    END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_floor( FLOATVAL x, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_floor );

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
    STATS_INC( SHD_so_ceil );

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
	TqFloat res;

    CHECKVARY( x )
    CHECKVARY( Result )

    BEGIN_VARYING_SECTION
    GETFLOAT( x );
	if ( FLOAT( x ) >= 0.0f )	res = static_cast<TqFloat>( static_cast<TqInt>( FLOAT( x ) + 0.5f ) );
	else res = static_cast<TqFloat>( static_cast<TqInt>( ( CEIL( FLOAT( x ) - 0.5f ) ) ) );
	SETFLOAT( Result, res );
    END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_step( FLOATVAL _min, FLOATVAL value, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_step );

    INIT_SO

    CHECKVARY( _min )
    CHECKVARY( value )
    CHECKVARY( Result )

    BEGIN_VARYING_SECTION
    GETFLOAT( _min );
    GETFLOAT( value );
    SETFLOAT( Result, ( FLOAT( value ) < FLOAT( _min ) ) ? 0.0f : 1.0f );
    END_VARYING_SECTION
}


//----------------------------------------------------------------------
// smoothstep(_min,_max,value)
STD_SOIMPL	CqShaderExecEnv::SO_smoothstep( FLOATVAL _min, FLOATVAL _max, FLOATVAL value, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_smoothstep );

    INIT_SO

    CHECKVARY( value )
    CHECKVARY( _min )
    CHECKVARY( _max )
    CHECKVARY( Result )

    BEGIN_VARYING_SECTION
    GETFLOAT( _min );
    GETFLOAT( _max );
    GETFLOAT( value );
    if ( FLOAT( value ) < FLOAT( _min ) )
        SETFLOAT( Result, 0.0f );
    else if ( FLOAT( value ) >= FLOAT( _max ) )
        SETFLOAT( Result, 1.0f );
    else
    {
        TqFloat v = ( FLOAT( value ) - FLOAT( _min ) ) / ( FLOAT( _max ) - FLOAT( _min ) );
        SETFLOAT( Result, v * v * ( 3.0f - 2.0f * v ) );
    }
    END_VARYING_SECTION
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_fspline( FLOATVAL value, DEFPARAMVARIMPL )
{
    STATS_INC( SHD_so_fspline );

    INIT_SO

    CqSplineCubic spline( cParams );
	spline.SetBasis("catmull-rom");

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
        apParams[ cParams - 2 ] ->GetFloat( fl, __iGrid );
        SETFLOAT( Result, fl );
    }
    else if ( FLOAT( value ) <= 0.0f )
    {
        TqFloat ff;
        apParams[ 1 ] ->GetFloat( ff, __iGrid );
        SETFLOAT( Result, ff );
    }
    else
    {
        TqInt j;
        for ( j = 0; j < cParams; j++ )
        {
            TqFloat fn;
            apParams[ j ] ->GetFloat( fn, __iGrid );
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
    STATS_INC( SHD_so_cspline );

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
        apParams[ cParams - 2 ] ->GetColor( cl, __iGrid );
        SETCOLOR( Result, cl );
    }
    else if ( FLOAT( value ) <= 0.0f )
    {
        CqColor cf;
        apParams[ 1 ] ->GetColor( cf, __iGrid );
        SETCOLOR( Result, cf );
    }
    else
    {
        TqInt j;
        for ( j = 0; j < cParams; j++ )
        {
            CqColor cn;
            apParams[ j ] ->GetColor( cn, __iGrid );
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
    STATS_INC( SHD_so_pspline );

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
        apParams[ cParams - 2 ] ->GetPoint( pl, __iGrid );
        SETPOINT( Result, pl );
    }
    else if ( FLOAT( value ) <= 0.0f )
    {
        CqVector3D pf;
        apParams[ 1 ] ->GetPoint( pf, __iGrid );
        SETPOINT( Result, pf );
    }
    else
    {
        TqInt j;
        for ( j = 0; j < cParams; j++ )
        {
            CqVector3D pn;
            apParams[ j ] ->GetPoint( pn, __iGrid );
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
    STATS_INC( SHD_so_sfspline );

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
    spline.SetBasis( STRING( basis ) );
    END_UNIFORM_SECTION

    BEGIN_VARYING_SECTION
    GETFLOAT( value );
    if ( FLOAT( value ) >= 1.0f )
    {
        TqFloat fl;
        apParams[ cParams - 2 ] ->GetFloat( fl, __iGrid );
        SETFLOAT( Result, fl );
    }
    else if ( FLOAT( value ) <= 0.0f )
    {
        TqFloat ff;
        apParams[ 1 ] ->GetFloat( ff, __iGrid );
        SETFLOAT( Result, ff );
    }
    else
    {
        TqInt j;
        for ( j = 0; j < cParams; j++ )
        {
            TqFloat fn;
            apParams[ j ] ->GetFloat( fn, __iGrid );
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
    STATS_INC( SHD_so_scspline );

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
    spline.SetBasis( STRING( basis ) );
    END_UNIFORM_SECTION

    BEGIN_VARYING_SECTION
    GETFLOAT( value );
    if ( FLOAT( value ) >= 1.0f )
    {
        CqColor cl;
        apParams[ cParams - 2 ] ->GetColor( cl, __iGrid );
        SETCOLOR( Result, cl );
    }
    else if ( FLOAT( value ) <= 0.0f )
    {
        CqColor cf;
        apParams[ 1 ] ->GetColor( cf, __iGrid );
        SETCOLOR( Result, cf );
    }
    else
    {
        TqInt j;
        for ( j = 0; j < cParams; j++ )
        {
            CqColor cn;
            apParams[ j ] ->GetColor( cn, __iGrid );
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
    STATS_INC( SHD_so_spspline );

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
    spline.SetBasis( STRING( basis ) );
    END_UNIFORM_SECTION

    BEGIN_VARYING_SECTION
    GETFLOAT( value );
    if ( FLOAT( value ) >= 1.0f )
    {
        CqVector3D pl;
        apParams[ cParams - 2 ] ->GetPoint( pl, __iGrid );
        SETPOINT( Result, pl );
    }
    else if ( FLOAT( value ) <= 0.0f )
    {
        CqVector3D pf;
        apParams[ 1 ] ->GetPoint( pf, __iGrid );
        SETPOINT( Result, pf );
    }
    else
    {
        TqInt j;
        for ( j = 0; j < cParams; j++ )
        {
            CqVector3D pn;
            apParams[ j ] ->GetPoint( pn, __iGrid );
            spline[ j ] = pn;
        }

        CqVector3D	res = spline.Evaluate( FLOAT( value ) );
        SETPOINT( Result, res );
    }
    END_VARYING_SECTION
}


STD_SOIMPL	CqShaderExecEnv::SO_fDu( FLOATVAL p, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_fDu );

    TqFloat Deffloat = 0.0f;
    INIT_SO

    CHECKVARY( p )
    CHECKVARY( Result )

    BEGIN_VARYING_SECTION
    SETFLOAT( Result, SO_DuType<TqFloat>( p, __iGrid, this, Deffloat ) );
    END_VARYING_SECTION
}


STD_SOIMPL	CqShaderExecEnv::SO_fDv( FLOATVAL p, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_fDv );

    TqFloat Deffloat = 0.0f;
    INIT_SO

    CHECKVARY( p )
    CHECKVARY( Result )

    BEGIN_VARYING_SECTION
    SETFLOAT( Result, SO_DvType<TqFloat>( p, __iGrid, this, Deffloat ) );
    END_VARYING_SECTION
}


STD_SOIMPL	CqShaderExecEnv::SO_fDeriv( FLOATVAL p, FLOATVAL den, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_fDeriv );

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
    STATS_INC( SHD_so_cDu );

    CqColor Defcol( 0.0f, 0.0f, 0.0f );
    INIT_SO

    CHECKVARY( p )
    CHECKVARY( Result )

    BEGIN_VARYING_SECTION
    SETCOLOR( Result, SO_DuType<CqColor>( p, __iGrid, this, Defcol ) );
    END_VARYING_SECTION
}


STD_SOIMPL	CqShaderExecEnv::SO_cDv( COLORVAL p, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_cDv );

    CqColor Defcol( 0.0f, 0.0f, 0.0f );
    INIT_SO

    CHECKVARY( p )
    CHECKVARY( Result )

    BEGIN_VARYING_SECTION
    SETCOLOR( Result, SO_DvType<CqColor>( p, __iGrid, this, Defcol ) );
    END_VARYING_SECTION
}


STD_SOIMPL	CqShaderExecEnv::SO_cDeriv( COLORVAL p, FLOATVAL den, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_cDeriv );

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
    STATS_INC( SHD_so_pDu );

    CqVector3D Defvec( 0.0f, 0.0f, 0.0f );
    INIT_SO

    CHECKVARY( p )
    CHECKVARY( Result )

    BEGIN_VARYING_SECTION
    SETPOINT( Result, SO_DuType<CqVector3D>( p, __iGrid, this, Defvec ) );
    END_VARYING_SECTION
}


STD_SOIMPL	CqShaderExecEnv::SO_pDv( POINTVAL p, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_pDv );

    CqVector3D Defvec( 0.0f, 0.0f, 0.0f );
    INIT_SO

    CHECKVARY( p )
    CHECKVARY( Result )

    BEGIN_VARYING_SECTION
    SETPOINT( Result, SO_DvType<CqVector3D>( p, __iGrid, this, Defvec ) );
    END_VARYING_SECTION
}


STD_SOIMPL	CqShaderExecEnv::SO_pDeriv( POINTVAL p, FLOATVAL den, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_pDeriv );

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
    STATS_INC( SHD_so_frandom );

    INIT_SO

    CHECKVARY( Result )

    BEGIN_VARYING_SECTION
    SETFLOAT( Result, m_random.RandomFloat() );
    END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_crandom( DEFPARAMIMPL )
{
    STATS_INC( SHD_so_crandom );

    INIT_SO

    CHECKVARY( Result )

    BEGIN_VARYING_SECTION
    SETCOLOR( Result, CqColor( m_random.RandomFloat(), m_random.RandomFloat(), m_random.RandomFloat() ) );
    END_VARYING_SECTION
}

STD_SOIMPL	CqShaderExecEnv::SO_prandom( DEFPARAMIMPL )
{
    STATS_INC( SHD_so_prandom );

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
    STATS_INC( SHD_so_fnoise1 );

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
    STATS_INC( SHD_so_fnoise2 );

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
    STATS_INC( SHD_so_fnoise3 );

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
    STATS_INC( SHD_so_fnoise4 );

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
    STATS_INC( SHD_so_cnoise1 );

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
    STATS_INC( SHD_so_cnoise2 );

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
    STATS_INC( SHD_so_cnoise3 );

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
    STATS_INC( SHD_so_cnoise4 );

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
    STATS_INC( SHD_so_pnoise1 );

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
    STATS_INC( SHD_so_pnoise2 );

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
    STATS_INC( SHD_so_pnoise3 );

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
    STATS_INC( SHD_so_pnoise4 );

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
    STATS_INC( SHD_so_setcomp );

    INIT_SO

    CHECKVARY( p )
    CHECKVARY( v )
    CHECKVARY( index )

    BEGIN_VARYING_SECTION
    GETCOLOR( p );
    GETFLOAT( index );
    GETFLOAT( v );
    COLOR( p ) [ static_cast<int>( FLOAT( index ) ) ] = FLOAT( v );
    SETCOLOR( p, COLOR( p ) );
    END_VARYING_SECTION
}

//----------------------------------------------------------------------
// setxcomp(p,v)
STD_SOIMPL	CqShaderExecEnv::SO_setxcomp( POINTVAL p, FLOATVAL v, DEFVOIDPARAMIMPL )
{
    STATS_INC( SHD_so_setxcomp );

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
    STATS_INC( SHD_so_setycomp );

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
    STATS_INC( SHD_so_setzcomp );

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
    STATS_INC( SHD_so_length );

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
    STATS_INC( SHD_so_distance );

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
    STATS_INC( SHD_so_area );

    CqVector3D Defvec( 0.0f, 0.0f, 0.0f );
    INIT_SO

    CqVector3D	vecR;

    CHECKVARY( p )
    CHECKVARY( Result )

    BEGIN_VARYING_SECTION
    if ( m_pAttributes )
    {
        TqFloat fdu, fdv;
        du() ->GetFloat( fdu, __iGrid );
        dv() ->GetFloat( fdv, __iGrid );
        vecR = ( SO_DuType<CqVector3D>( p, __iGrid, this, Defvec ) * fdu ) %
               ( SO_DvType<CqVector3D>( p, __iGrid, this, Defvec ) * fdv );
        SETFLOAT( Result, vecR.Magnitude() );
    }

    END_VARYING_SECTION
}


STD_SOIMPL	CqShaderExecEnv::SO_normalize( VECTORVAL V, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_normalize );

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
// faceforward(N,I)
STD_SOIMPL CqShaderExecEnv::SO_faceforward( NORMALVAL N, VECTORVAL I, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_faceforward );

    INIT_SO

    CHECKVARY( N )
    CHECKVARY( I )
    CHECKVARY( Result )

    BEGIN_VARYING_SECTION
    GETNORMAL( N );
    GETVECTOR( I );
    CqVector3D Nref;
    Ng() ->GetNormal( Nref, __iGrid );
    TqFloat s = ( ( ( -VECTOR( I ) ) * Nref ) < 0.0f ) ? -1.0f : 1.0f;
    SETNORMAL( Result, NORMAL( N ) * s );
    END_VARYING_SECTION
}


//----------------------------------------------------------------------
// faceforward(N,I,Nref)
STD_SOIMPL CqShaderExecEnv::SO_faceforward2( NORMALVAL N, VECTORVAL I, NORMALVAL Nref, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_faceforward2 );

    INIT_SO

    CHECKVARY( N )
    CHECKVARY( I )
    CHECKVARY( Nref )
    CHECKVARY( Result )

    BEGIN_VARYING_SECTION
    GETNORMAL( N );
    GETVECTOR( I );
    GETNORMAL( Nref );
    TqFloat s = ( ( ( -VECTOR( I ) ) * NORMAL( Nref ) ) < 0.0f ) ? -1.0f : 1.0f;
    SETNORMAL( Result, NORMAL( N ) * s );
    END_VARYING_SECTION
}


//----------------------------------------------------------------------
// reflect(I,N)
STD_SOIMPL CqShaderExecEnv::SO_reflect( VECTORVAL I, NORMALVAL N, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_reflect );

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
    STATS_INC( SHD_so_refract );

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
    STATS_INC( SHD_so_fresnel );

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
    TqFloat fuvB = fabs( fuvA );
    TqFloat fu2 = ( fuvA + fuvB ) / 2;
    TqFloat fv2 = ( -fuvA + fuvB ) / 2;
    TqFloat fv2sqrt = ( fv2 == 0.0f ) ? 0.0f : sqrt( fabs( fv2 ) );
    TqFloat fu2sqrt = ( fu2 == 0.0f ) ? 0.0f : sqrt( fabs( fu2 ) );
    TqFloat fperp2 = ( SQR( cos_theta - fu2sqrt ) + fv2 ) / ( SQR( cos_theta + fu2sqrt ) + fv2 );
    TqFloat feta = FLOAT( eta );
    TqFloat fpara2 = ( SQR( SQR( 1.0f / feta ) * cos_theta - fu2sqrt ) + SQR( -fv2sqrt ) ) /
                     ( SQR( SQR( 1.0f / feta ) * cos_theta + fu2sqrt ) + SQR( fv2sqrt ) );

    TqFloat __Kr = 0.5f * ( fperp2 + fpara2 );
    SETFLOAT( Kr, __Kr );
    SETFLOAT( Kt, 1.0f - __Kr );
    END_VARYING_SECTION
}

//----------------------------------------------------------------------
// fresnel(I,N,eta,Kr,Kt,R,T)
STD_SOIMPL CqShaderExecEnv::SO_fresnel( VECTORVAL I, NORMALVAL N, FLOATVAL eta, FLOATVAL Kr, FLOATVAL Kt, VECTORVAL R, VECTORVAL T, DEFVOIDPARAMIMPL )
{
    STATS_INC( SHD_so_fresnel );

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
    TqFloat fuvB = fabs( fuvA );
    TqFloat fu2 = ( fuvA + fuvB ) / 2;
    TqFloat fv2 = ( -fuvA + fuvB ) / 2;
    TqFloat feta = FLOAT( eta );
    TqFloat fv2sqrt = ( fv2 == 0.0f ) ? 0.0f : sqrt( fabs( fv2 ) );
    TqFloat fu2sqrt = ( fu2 == 0.0f ) ? 0.0f : sqrt( fabs( fu2 ) );
    TqFloat fperp2 = ( SQR( cos_theta - fu2sqrt ) + fv2 ) / ( SQR( cos_theta + fu2sqrt ) + fv2 );
    TqFloat fpara2 = ( SQR( SQR( 1.0f / feta ) * cos_theta - fu2sqrt ) + SQR( -fv2sqrt ) ) /
                     ( SQR( SQR( 1.0f / feta ) * cos_theta + fu2sqrt ) + SQR( fv2sqrt ) );
    TqFloat __Kr = 0.5f * ( fperp2 + fpara2 );
    SETFLOAT( Kr, __Kr );
    SETFLOAT( Kt, 1.0f - __Kr );
    END_VARYING_SECTION

    SO_reflect( I, N, R );
    SO_refract( I, N, eta, T );
}


//----------------------------------------------------------------------
// transform(s,s,P)
STD_SOIMPL CqShaderExecEnv::SO_transform( STRINGVAL fromspace, STRINGVAL tospace, POINTVAL p, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_transform );

    INIT_SO

    assert( pShader != 0 );

    CHECKVARY( p )
    CHECKVARY( Result )

    if ( NULL != QGetRenderContextI() )
    {
        BEGIN_UNIFORM_SECTION
        GETSTRING( fromspace );
        GETSTRING( tospace );
        const CqMatrix& mat = QGetRenderContextI() ->matSpaceToSpace( STRING( fromspace ).c_str(), STRING( tospace ).c_str(), pShader->matCurrent(), matObjectToWorld(), QGetRenderContextI()->Time() );
        END_UNIFORM_SECTION

        BEGIN_VARYING_SECTION
        GETPOINT( p );
        SETPOINT( Result, mat * POINT( p ) );
        END_VARYING_SECTION
    }
    else
    {
        BEGIN_VARYING_SECTION
        GETPOINT( p );
        SETPOINT( Result, POINT( p ) );
        END_VARYING_SECTION
    }
}


//----------------------------------------------------------------------
// transform(s,P)
STD_SOIMPL CqShaderExecEnv::SO_transform( STRINGVAL tospace, POINTVAL p, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_transform );

    INIT_SO

    assert( pShader != 0 );

    CHECKVARY( p )
    CHECKVARY( Result )

    if ( NULL != QGetRenderContextI() )
    {
        BEGIN_UNIFORM_SECTION
        GETSTRING( tospace );
        const CqMatrix& mat = QGetRenderContextI() ->matSpaceToSpace( "current", STRING( tospace ).c_str(), pShader->matCurrent(), matObjectToWorld(), QGetRenderContextI()->Time() );
        END_UNIFORM_SECTION

        BEGIN_VARYING_SECTION
        GETPOINT( p );
        SETPOINT( Result, mat * POINT( p ) );
        END_VARYING_SECTION
    }
    else
    {
        BEGIN_VARYING_SECTION
        GETPOINT( p );
        SETPOINT( Result, POINT( p ) );
        END_VARYING_SECTION
    }
}


//----------------------------------------------------------------------
// transform(m,P)
STD_SOIMPL CqShaderExecEnv::SO_transformm( MATRIXVAL tospace, POINTVAL p, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_transform );

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
    STATS_INC( SHD_so_vtransform );

    INIT_SO

    assert( pShader != 0 );

    CHECKVARY( p )
    CHECKVARY( Result )

    if ( NULL != QGetRenderContextI() )
    {
        BEGIN_UNIFORM_SECTION
        GETSTRING( fromspace );
        GETSTRING( tospace );
        const CqMatrix& mat = QGetRenderContextI() ->matVSpaceToSpace( STRING( fromspace ).c_str(), STRING( tospace ).c_str(), pShader->matCurrent(), matObjectToWorld(), QGetRenderContextI()->Time() );
        END_UNIFORM_SECTION

        BEGIN_VARYING_SECTION
        GETVECTOR( p );
        SETVECTOR( Result, mat * VECTOR( p ) );
        END_VARYING_SECTION
    }
    else
    {
        BEGIN_VARYING_SECTION
        GETVECTOR( p );
        SETVECTOR( Result, VECTOR( p ) );
        END_VARYING_SECTION
    }
}


//----------------------------------------------------------------------
// vtransform(s,P)
STD_SOIMPL CqShaderExecEnv::SO_vtransform( STRINGVAL tospace, VECTORVAL p, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_vtransform );

    INIT_SO

    assert( pShader != 0 );

    CHECKVARY( p )
    CHECKVARY( Result )

    if ( NULL != QGetRenderContextI() )
    {
        BEGIN_UNIFORM_SECTION
        GETSTRING( tospace );
        const CqMatrix& mat = QGetRenderContextI() ->matVSpaceToSpace( "current", STRING( tospace ).c_str(), pShader->matCurrent(), matObjectToWorld(), QGetRenderContextI()->Time() );
        END_UNIFORM_SECTION

        BEGIN_VARYING_SECTION
        GETVECTOR( p );
        SETVECTOR( Result, mat * VECTOR( p ) );
        END_VARYING_SECTION
    }
    else
    {
        BEGIN_VARYING_SECTION
        GETVECTOR( p );
        SETVECTOR( Result, VECTOR( p ) );
        END_VARYING_SECTION
    }
}


//----------------------------------------------------------------------
// vtransform(m,P)
STD_SOIMPL CqShaderExecEnv::SO_vtransformm( MATRIXVAL tospace, VECTORVAL p, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_vtransform );

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
    STATS_INC( SHD_so_ntransform );

    INIT_SO

    assert( pShader != 0 );

    CHECKVARY( p )
    CHECKVARY( Result )

    if ( NULL != QGetRenderContextI() )
    {
        BEGIN_UNIFORM_SECTION
        GETSTRING( fromspace );
        GETSTRING( tospace );
        const CqMatrix& mat = QGetRenderContextI() ->matNSpaceToSpace( STRING( fromspace ).c_str(), STRING( tospace ).c_str(), pShader->matCurrent(), matObjectToWorld(), QGetRenderContextI()->Time() );
        BEGIN_UNIFORM_SECTION

        BEGIN_VARYING_SECTION
        GETNORMAL( p );
        SETNORMAL( Result, mat * NORMAL( p ) );
        END_VARYING_SECTION
    }
    else
    {
        BEGIN_VARYING_SECTION
        GETNORMAL( p );
        SETNORMAL( Result, NORMAL( p ) );
        END_VARYING_SECTION
    }
}


//----------------------------------------------------------------------
// ntransform(s,P)
STD_SOIMPL CqShaderExecEnv::SO_ntransform( STRINGVAL tospace, NORMALVAL p, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_ntransform );

    INIT_SO

    assert( pShader != 0 );

    CHECKVARY( p )
    CHECKVARY( Result )

    if ( NULL != QGetRenderContextI() )
    {
        BEGIN_UNIFORM_SECTION
        GETSTRING( tospace );
        const CqMatrix& mat = QGetRenderContextI() ->matNSpaceToSpace( "current", STRING( tospace ).c_str(), pShader->matCurrent(), matObjectToWorld(), QGetRenderContextI()->Time() );
        BEGIN_UNIFORM_SECTION

        BEGIN_VARYING_SECTION
        GETNORMAL( p );
        SETNORMAL( Result, mat * NORMAL( p ) );
        END_VARYING_SECTION
    }
    else
    {
        BEGIN_VARYING_SECTION
        GETNORMAL( p );
        SETNORMAL( Result, NORMAL( p ) );
        END_VARYING_SECTION
    }
}


//----------------------------------------------------------------------
// ntransform(m,P)
STD_SOIMPL CqShaderExecEnv::SO_ntransformm( MATRIXVAL tospace, NORMALVAL p, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_ntransform );

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
    STATS_INC( SHD_so_depth );

    INIT_SO

    if ( NULL == QGetRenderContextI() )
        return ;

    CHECKVARY( p )
    CHECKVARY( Result )

    BEGIN_VARYING_SECTION
    GETPOINT( p );
    TqFloat d = POINT( p ).z();
    d = ( d - QGetRenderContextI() ->GetFloatOption( "System", "Clipping" ) [ 0 ] ) /
        ( QGetRenderContextI() ->GetFloatOption( "System", "Clipping" ) [ 1 ] - QGetRenderContextI() ->GetFloatOption( "System", "Clipping" ) [ 0 ] );
    SETFLOAT( Result, d );
    END_VARYING_SECTION
}


//----------------------------------------------------------------------
// calculatenormal(P)
STD_SOIMPL CqShaderExecEnv::SO_calculatenormal( POINTVAL p, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_calculatenormal );

    CqVector3D Defvec( 0.0f, 0.0f, 0.0f );
    INIT_SO

    // Find out if the orientation is inverted.
    TqBool CSO = pTransform()->GetHandedness(QGetRenderContextI()->Time());
    TqBool O = TqFalse;
	if( pAttributes() )
	    O = pAttributes() ->GetIntegerAttribute( "System", "Orientation" ) [ 0 ] != 0;
    TqFloat neg = 1;
	if ( O != CSO ) neg = -1;

    CHECKVARY( p )
    CHECKVARY( Result )

    BEGIN_VARYING_SECTION
    //CqVector3D	dPdu = SO_DuType<CqVector3D>( p, __iGrid, this, Defvec );
    //CqVector3D	dPdv = SO_DvType<CqVector3D>( p, __iGrid, this, Defvec );
    //CqVector3D	N = dPdu % dPdv;

    CqVector3D Ret, Ret2;
    TqInt uRes = uGridRes();
    TqInt GridX = __iGrid % ( uRes + 1 );

    CqVector3D v1, v2;
    if ( GridX < uRes )
    {
        p->GetValue( v1, __iGrid + 1 );
        p->GetValue( v2, __iGrid );
        Ret = ( v1 - v2 );
    }
    else
    {
        p->GetValue( v1, __iGrid );
        p->GetValue( v2, __iGrid - 1 );
        Ret = ( v1 - v2 );
    }
    TqInt vRes = vGridRes();
    TqInt GridY = ( __iGrid / ( uRes + 1 ) );

    if ( GridY < vRes )
    {
        p->GetValue( v1, __iGrid + uRes + 1 );
        p->GetValue( v2, __iGrid );
        Ret2 = ( v1 - v2 );
    }
    else
    {
        p->GetValue( v1, __iGrid );
        p->GetValue( v2, __iGrid - ( uRes + 1 ) );
        Ret2 = ( v1 - v2 );
    }

    CqVector3D N = Ret % Ret2;
    N.Unit();
    N *= neg;
    SETNORMAL( Result, N );
    END_VARYING_SECTION
}

STD_SOIMPL CqShaderExecEnv::SO_cmix( COLORVAL color0, COLORVAL color1, FLOATVAL value, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_cmix );

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
    STATS_INC( SHD_so_fmix );

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
    STATS_INC( SHD_so_pmix );

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
    STATS_INC( SHD_so_vmix );

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
    STATS_INC( SHD_so_ftexture1 );

    TqFloat Deffloat = 0.0f;
    INIT_SO

    if ( NULL == QGetRenderContextI() )
        return ;

    GET_TEXTURE_PARAMS;

    TqFloat fill = 0.0f;
    if ( paramMap.find( "fill" ) != paramMap.end() )
        paramMap[ "fill" ] ->GetFloat( fill );

    BEGIN_UNIFORM_SECTION
    GETSTRING( name );
    GETFLOAT( channel );
    IqTextureMap* pTMap = QGetRenderContextI() ->GetTextureMap( STRING( name ) );
    TqFloat fdu = 0.0f, fdv = 0.0f;
    if ( m_pAttributes )
    {
        du() ->GetFloat( fdu );
        dv() ->GetFloat( fdv );
    }
    END_UNIFORM_SECTION


    __fVarying = TqTrue;
    if ( pTMap != 0 && pTMap->IsValid() )
    {
        std::valarray<TqFloat> val;
        pTMap->PrepareSampleOptions( paramMap );

        BEGIN_VARYING_SECTION
        TqFloat swidth = 0.0f, twidth = 0.0f;
        if ( fdu != 0.0f && fdv != 0.0f )
        {
            TqFloat dsdu = SO_DuType<TqFloat>( s(), __iGrid, this, Deffloat );
            swidth = fabs( dsdu * fdu );
            TqFloat dtdu = SO_DuType<TqFloat>( t(), __iGrid, this, Deffloat );
            twidth = fabs( dtdu * fdu );

            TqFloat dsdv = SO_DvType<TqFloat>( s(), __iGrid, this, Deffloat );
            swidth += fabs( dsdv * fdv );
            TqFloat dtdv = SO_DvType<TqFloat>( t(), __iGrid, this, Deffloat );
            twidth += fabs( dtdv * fdv );
        }
        else
        {
            swidth = 1.0 / pTMap->XRes();
            twidth = 1.0 / pTMap->YRes();
        }

        // Sample the texture.
        TqFloat fs, ft;
        s() ->GetFloat( fs, __iGrid );
        t() ->GetFloat( ft, __iGrid );
        pTMap->SampleMap( fs, ft, swidth, twidth, val );

        // Grab the appropriate channel.
        TqFloat fchan = FLOAT( channel );
        if ( fchan >= val.size() )
            SETFLOAT( Result, fill );
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
    STATS_INC( SHD_so_ftexture2 );

    TqFloat Deffloat = 0.0f;
    INIT_SO

    if ( NULL == QGetRenderContextI() )
        return ;

    GET_TEXTURE_PARAMS;

    TqFloat fill = 0.0f;
    if ( paramMap.find( "fill" ) != paramMap.end() )
        paramMap[ "fill" ] ->GetFloat( fill );

    BEGIN_UNIFORM_SECTION
    GETSTRING( name );
    GETFLOAT( channel );
    IqTextureMap* pTMap = QGetRenderContextI() ->GetTextureMap( STRING( name ) );
    TqFloat fdu = 0.0f, fdv = 0.0f;
    if ( m_pAttributes )
    {
        du() ->GetFloat( fdu );
        dv() ->GetFloat( fdv );
    }
    END_UNIFORM_SECTION


    __fVarying = TqTrue;
    if ( pTMap != 0 && pTMap->IsValid() )
    {
        std::valarray<TqFloat> val;
        pTMap->PrepareSampleOptions( paramMap );

        BEGIN_VARYING_SECTION
        TqFloat swidth = 0.0f, twidth = 0.0f;
        if ( fdu != 0.0f && fdv != 0.0f )
        {
            TqFloat dsdu = SO_DuType<TqFloat>( s, __iGrid, this, Deffloat );
            swidth = fabs( dsdu * fdu );
            TqFloat dtdu = SO_DuType<TqFloat>( t, __iGrid, this, Deffloat );
            twidth = fabs( dtdu * fdu );

            TqFloat dsdv = SO_DvType<TqFloat>( s, __iGrid, this, Deffloat );
            swidth += fabs( dsdv * fdv );
            TqFloat dtdv = SO_DvType<TqFloat>( t, __iGrid, this, Deffloat );
            twidth += fabs( dtdv * fdv );
        }
        else
        {
            swidth = 1.0 / pTMap->XRes();
            twidth = 1.0 / pTMap->YRes();
        }

        // Sample the texture.
        GETFLOAT( s );
        GETFLOAT( t );
        pTMap->SampleMap( FLOAT( s ), FLOAT( t ), swidth, twidth, val );

        // Grab the appropriate channel.
        TqFloat fchan = FLOAT( channel );
        if ( fchan >= val.size() )
            SETFLOAT( Result, fill );
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
    STATS_INC( SHD_so_ftexture3 );

    INIT_SO

    if ( NULL == QGetRenderContextI() )
        return ;

    GET_TEXTURE_PARAMS;

    TqFloat fill = 0.0f;
    if ( paramMap.find( "fill" ) != paramMap.end() )
        paramMap[ "fill" ] ->GetFloat( fill );

    BEGIN_UNIFORM_SECTION
    GETSTRING( name );
    GETFLOAT( channel );
    IqTextureMap* pTMap = QGetRenderContextI() ->GetTextureMap( STRING( name ) );
    END_UNIFORM_SECTION


    __fVarying = TqTrue;
    if ( pTMap != 0 && pTMap->IsValid() )
    {
        std::valarray<TqFloat> val;
        pTMap->PrepareSampleOptions( paramMap );

        BEGIN_VARYING_SECTION

        // Sample the texture.
        GETFLOAT( s1 );
        GETFLOAT( t1 );
        GETFLOAT( s2 );
        GETFLOAT( t2 );
        GETFLOAT( s3 );
        GETFLOAT( t3 );
        GETFLOAT( s4 );
        GETFLOAT( t4 );
        pTMap->SampleMap( FLOAT( s1 ), FLOAT( t1 ), FLOAT( s2 ), FLOAT( t2 ), FLOAT( s3 ), FLOAT( t3 ), FLOAT( s4 ), FLOAT( t4 ), val );

        // Grab the appropriate channel.
        TqFloat fchan = FLOAT( channel );
        if ( fchan >= val.size() )
            SETFLOAT( Result, fill );
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
    STATS_INC( SHD_so_ctexture1 );

    TqFloat Deffloat = 0.0f;
    INIT_SO

    if ( NULL == QGetRenderContextI() )
        return ;

    GET_TEXTURE_PARAMS;

    TqFloat fill = 0.0f;
    if ( paramMap.find( "fill" ) != paramMap.end() )
        paramMap[ "fill" ] ->GetFloat( fill );

    BEGIN_UNIFORM_SECTION
    GETSTRING( name );
    GETFLOAT( channel );
    IqTextureMap* pTMap = QGetRenderContextI() ->GetTextureMap( STRING( name ) );
    TqFloat fdu = 0.0f, fdv = 0.0f;
    if ( m_pAttributes )
    {
        du() ->GetFloat( fdu );
        dv() ->GetFloat( fdv );
    }
    END_UNIFORM_SECTION


    __fVarying = TqTrue;
    if ( pTMap != 0 && pTMap->IsValid() )
    {
        std::valarray<TqFloat> val;
        pTMap->PrepareSampleOptions( paramMap );

        BEGIN_VARYING_SECTION
        TqFloat swidth = 0.0f, twidth = 0.0f;
        if ( fdu != 0.0f && fdv != 0.0f )
        {
            TqFloat dsdu = SO_DuType<TqFloat>( s(), __iGrid, this, Deffloat );
            swidth = fabs( dsdu * fdu );
            TqFloat dsdv = SO_DvType<TqFloat>( s(), __iGrid, this, Deffloat );
            swidth += fabs( dsdv * fdv );

            TqFloat dtdu = SO_DuType<TqFloat>( t(), __iGrid, this, Deffloat );
            twidth = fabs( dtdu * fdu );
            TqFloat dtdv = SO_DvType<TqFloat>( t(), __iGrid, this, Deffloat );
            twidth += fabs( dtdv * fdv );
        }
        else
        {
            swidth = 1.0 / pTMap->XRes();
            twidth = 1.0 / pTMap->YRes();
        }

        // Sample the texture.
        TqFloat fs, ft;
        s() ->GetFloat( fs, __iGrid );
        t() ->GetFloat( ft, __iGrid );
        pTMap->SampleMap( fs, ft, swidth, twidth, val );

        // Grab the appropriate channel.
        TqFloat fchan = FLOAT( channel );
		CqColor colResult;
        colResult.SetfRed( (fchan >= val.size())? fill : val[ static_cast<unsigned int>( fchan ) ] );
        colResult.SetfGreen( ((fchan + 1) >= val.size())? fill : val[ static_cast<unsigned int>( fchan+1 ) ] );
        colResult.SetfBlue( ((fchan + 2) >= val.size())? fill : val[ static_cast<unsigned int>( fchan+2 ) ] );

		SETCOLOR( Result, colResult );
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
    STATS_INC( SHD_so_ctexture2 );

    TqFloat Deffloat = 0.0f;
    INIT_SO

    if ( NULL == QGetRenderContextI() )
        return ;

    GET_TEXTURE_PARAMS;

    TqFloat fill = 0.0f;
    if ( paramMap.find( "fill" ) != paramMap.end() )
        paramMap[ "fill" ] ->GetFloat( fill );

    BEGIN_UNIFORM_SECTION
    GETSTRING( name );
    GETFLOAT( channel );
    IqTextureMap* pTMap = QGetRenderContextI() ->GetTextureMap( STRING( name ) );
    TqFloat fdu = 0.0f, fdv = 0.0f;
    if ( m_pAttributes )
    {
        du() ->GetFloat( fdu );
        dv() ->GetFloat( fdv );
    }
    END_UNIFORM_SECTION


    __fVarying = TqTrue;
    if ( pTMap != 0 && pTMap->IsValid() )
    {
        std::valarray<TqFloat> val;
        pTMap->PrepareSampleOptions( paramMap );

        BEGIN_VARYING_SECTION
        TqFloat swidth = 0.0f, twidth = 0.0f;
        if ( fdu != 0.0f && fdv != 0.0f )
        {
            TqFloat dsdu = SO_DuType<TqFloat>( s, __iGrid, this, Deffloat );
            swidth = fabs( dsdu * fdu );
            TqFloat dsdv = SO_DvType<TqFloat>( s, __iGrid, this, Deffloat );
            swidth += fabs( dsdv * fdv );

            TqFloat dtdu = SO_DuType<TqFloat>( t, __iGrid, this, Deffloat );
            twidth = fabs( dtdu * fdu );
            TqFloat dtdv = SO_DvType<TqFloat>( t, __iGrid, this, Deffloat );
            twidth += fabs( dtdv * fdv );
        }
        else
        {
            swidth = 1.0 / pTMap->XRes();
            twidth = 1.0 / pTMap->YRes();
        }

        // Sample the texture.
        GETFLOAT( s );
        GETFLOAT( t );
        pTMap->SampleMap( FLOAT( s ), FLOAT( t ), swidth, twidth, val );

        // Grab the appropriate channel.
        TqFloat fchan = FLOAT( channel );
		CqColor colResult;
        colResult.SetfRed( (fchan >= val.size())? fill : val[ static_cast<unsigned int>( fchan ) ] );
        colResult.SetfGreen( ((fchan + 1) >= val.size())? fill : val[ static_cast<unsigned int>( fchan+1 ) ] );
        colResult.SetfBlue( ((fchan + 2) >= val.size())? fill : val[ static_cast<unsigned int>( fchan+2 ) ] );

		SETCOLOR( Result, colResult );
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
    STATS_INC( SHD_so_ctexture3 );

    INIT_SO

    if ( NULL == QGetRenderContextI() )
        return ;

    GET_TEXTURE_PARAMS;

    TqFloat fill = 0.0f;
    if ( paramMap.find( "fill" ) != paramMap.end() )
        paramMap[ "fill" ] ->GetFloat( fill );

    BEGIN_UNIFORM_SECTION
    GETSTRING( name );
    GETFLOAT( channel );
    IqTextureMap* pTMap = QGetRenderContextI() ->GetTextureMap( STRING( name ) );
    END_UNIFORM_SECTION


    __fVarying = TqTrue;
    if ( pTMap != 0 && pTMap->IsValid() )
    {
        std::valarray<TqFloat> val;
        pTMap->PrepareSampleOptions( paramMap );

        BEGIN_VARYING_SECTION
        // Sample the texture.
        GETFLOAT( s1 );
        GETFLOAT( t1 );
        GETFLOAT( s2 );
        GETFLOAT( t2 );
        GETFLOAT( s3 );
        GETFLOAT( t3 );
        GETFLOAT( s4 );
        GETFLOAT( t4 );
        pTMap->SampleMap( FLOAT( s1 ), FLOAT( t1 ), FLOAT( s2 ), FLOAT( t2 ), FLOAT( s3 ), FLOAT( t3 ), FLOAT( s4 ), FLOAT( t4 ), val );

        // Grab the appropriate channel.
        TqFloat fchan = FLOAT( channel );
		CqColor colResult;
        colResult.SetfRed( (fchan >= val.size())? fill : val[ static_cast<unsigned int>( fchan ) ] );
        colResult.SetfGreen( ((fchan + 1) >= val.size())? fill : val[ static_cast<unsigned int>( fchan+1 ) ] );
        colResult.SetfBlue( ((fchan + 2) >= val.size())? fill : val[ static_cast<unsigned int>( fchan+2 ) ] );

		SETCOLOR( Result, colResult );
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
    STATS_INC( SHD_so_fenvironment2 );

    CqVector3D Defvec( 0.0f, 0.0f, 0.0f );
    INIT_SO

    if ( NULL == QGetRenderContextI() )
        return ;

    GET_TEXTURE_PARAMS;

    TqFloat fill = 0.0f;
    if ( paramMap.find( "fill" ) != paramMap.end() )
        paramMap[ "fill" ] ->GetFloat( fill );

    BEGIN_UNIFORM_SECTION
    GETSTRING( name );
    GETFLOAT( channel );
    IqTextureMap* pTMap = QGetRenderContextI() ->GetEnvironmentMap( STRING( name ) );

    // Try with LatLong map file
    if ( pTMap == 0 )
    {
        pTMap = QGetRenderContextI() ->GetLatLongMap( STRING( name ) );
    }
    TqFloat fdu = 0.0f, fdv = 0.0f;
    if ( m_pAttributes )
    {
        du() ->GetFloat( fdu );
        dv() ->GetFloat( fdv );
    }
    END_UNIFORM_SECTION

    __fVarying = TqTrue;
    if ( pTMap != 0 && pTMap->IsValid() )
    {
        pTMap->PrepareSampleOptions( paramMap );
        std::valarray<TqFloat> val;

        BEGIN_VARYING_SECTION
        CqVector3D swidth = 0.0f, twidth = 0.0f;
        if ( fdu != 0.0f )
        {
            CqVector3D dRdu = SO_DuType<CqVector3D>( R, __iGrid, this, Defvec );
            swidth = dRdu * fdu;
        }
        if ( fdv != 0.0f )
        {
            CqVector3D dRdv = SO_DvType<CqVector3D>( R, __iGrid, this, Defvec );
            twidth = dRdv * fdv;
        }
        else
        {
            swidth = CqVector3D( 1.0 / pTMap->XRes() );
            twidth = CqVector3D( 1.0 / pTMap->YRes() );
        }

        // Sample the texture.
        GETVECTOR( R );
        pTMap->SampleMap( VECTOR( R ), swidth, twidth, val );

        // Grab the appropriate channel.
        TqFloat fchan = FLOAT( channel );
        if ( fchan >= val.size() )
            SETFLOAT( Result, fill );
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
    STATS_INC( SHD_so_fenvironment3 );

    INIT_SO

    if ( NULL == QGetRenderContextI() )
        return ;

    GET_TEXTURE_PARAMS;

    TqFloat fill = 0.0f;
    if ( paramMap.find( "fill" ) != paramMap.end() )
        paramMap[ "fill" ] ->GetFloat( fill );

    BEGIN_UNIFORM_SECTION
    GETSTRING( name );
    GETFLOAT( channel );
    IqTextureMap* pTMap = QGetRenderContextI() ->GetEnvironmentMap( STRING( name ) );
    // Try with LatLong map file
    if ( pTMap == 0 )
    {
        pTMap = QGetRenderContextI() ->GetLatLongMap( STRING( name ) );
    }
    END_UNIFORM_SECTION

    __fVarying = TqTrue;
    if ( pTMap != 0 && pTMap->IsValid() )
    {
        std::valarray<TqFloat> val;
        pTMap->PrepareSampleOptions( paramMap );

        BEGIN_VARYING_SECTION
        // Sample the texture.
        GETVECTOR( R1 );
        GETVECTOR( R2 );
        GETVECTOR( R3 );
        GETVECTOR( R4 );
        pTMap->SampleMap( VECTOR( R1 ), VECTOR( R2 ), VECTOR( R3 ), VECTOR( R4 ), val );

        // Grab the appropriate channel.
        TqFloat fchan = FLOAT( channel );
        if ( fchan >= val.size() )
            SETFLOAT( Result, fill );
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
    STATS_INC( SHD_so_cenvironment2 );

    CqVector3D Defvec( 0.0f, 0.0f, 0.0f );
    INIT_SO

    if ( NULL == QGetRenderContextI() )
        return ;

    GET_TEXTURE_PARAMS;

    TqFloat fill = 0.0f;
    if ( paramMap.find( "fill" ) != paramMap.end() )
        paramMap[ "fill" ] ->GetFloat( fill );

    BEGIN_UNIFORM_SECTION
    GETSTRING( name );
    GETFLOAT( channel );
    IqTextureMap* pTMap = QGetRenderContextI() ->GetEnvironmentMap( STRING( name ) );
    // Try with LatLong map file
    if ( pTMap == 0 )
    {
        pTMap = QGetRenderContextI() ->GetLatLongMap( STRING( name ) );
    }
    TqFloat fdu = 0.0f, fdv = 0.0f;
    if ( m_pAttributes )
    {
        du() ->GetFloat( fdu );
        dv() ->GetFloat( fdv );
    }
    END_UNIFORM_SECTION

    __fVarying = TqTrue;
    if ( pTMap != 0 && pTMap->IsValid() )
    {
        std::valarray<TqFloat> val;
        pTMap->PrepareSampleOptions( paramMap );

        BEGIN_VARYING_SECTION
        CqVector3D swidth = 0.0f, twidth = 0.0f;
        if ( fdu != 0.0f )
        {
            CqVector3D dRdu = SO_DuType<CqVector3D>( R, __iGrid, this, Defvec );
            swidth = dRdu * fdu;
        }
        if ( fdv != 0.0f )
        {
            CqVector3D dRdv = SO_DvType<CqVector3D>( R, __iGrid, this, Defvec );
            twidth = dRdv * fdv;
        }
        else
        {
            swidth = CqVector3D( 1.0 / pTMap->XRes() );
            twidth = CqVector3D( 1.0 / pTMap->YRes() );
        }

        // Sample the texture.
        GETVECTOR( R );
        pTMap->SampleMap( VECTOR( R ), swidth, twidth, val );


        // Grab the appropriate channel.
        TqFloat fchan = FLOAT( channel );
        if ( fchan + 2 >= val.size() )
            SETCOLOR( Result, CqColor( fill, fill, fill ) );
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
    STATS_INC( SHD_so_cenvironment3 );

    INIT_SO

    if ( NULL == QGetRenderContextI() )
        return ;

    GET_TEXTURE_PARAMS;

    TqFloat fill = 0.0f;
    if ( paramMap.find( "fill" ) != paramMap.end() )
        paramMap[ "fill" ] ->GetFloat( fill );

    BEGIN_UNIFORM_SECTION
    GETSTRING( name );
    GETFLOAT( channel );
    IqTextureMap* pTMap = QGetRenderContextI() ->GetEnvironmentMap( STRING( name ) );
    // Try with LatLong map file
    if ( pTMap == 0 )
    {
        pTMap = QGetRenderContextI() ->GetLatLongMap( STRING( name ) );
    }
    BEGIN_UNIFORM_SECTION

    __fVarying = TqTrue;
    if ( pTMap != 0 && pTMap->IsValid() )
    {
        std::valarray<TqFloat> val;
        pTMap->PrepareSampleOptions( paramMap );

        BEGIN_VARYING_SECTION
        // Sample the texture.
        GETVECTOR( R1 );
        GETVECTOR( R2 );
        GETVECTOR( R3 );
        GETVECTOR( R4 );
        pTMap->SampleMap( VECTOR( R1 ), VECTOR( R2 ), VECTOR( R3 ), VECTOR( R4 ), val );

        // Grab the appropriate channel.
        TqFloat fchan = FLOAT( channel );
        if ( fchan + 2 >= val.size() )
            SETCOLOR( Result, CqColor( fill, fill, fill ) );
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
    STATS_INC( SHD_so_shadow );

    INIT_SO

    if ( NULL == QGetRenderContextI() )
        return ;

    GET_TEXTURE_PARAMS;

    BEGIN_UNIFORM_SECTION
    GETSTRING( name );
    GETFLOAT( channel );
    IqTextureMap* pMap = QGetRenderContextI() ->GetShadowMap( STRING( name ) );
    END_UNIFORM_SECTION

    __fVarying = TqTrue;
    if ( pMap != 0 && pMap->IsValid() )
    {
        std::valarray<TqFloat> fv;
        pMap->PrepareSampleOptions( paramMap );

        BEGIN_VARYING_SECTION
        CqVector3D swidth = 0.0f, twidth = 0.0f, nullv = 0.0f;

        swidth = SO_DerivType<CqVector3D>( P, NULL, __iGrid, this );
        twidth = SO_DerivType<CqVector3D>( P, NULL, __iGrid, this );

        GETPOINT( P );

        pMap->SampleMap( POINT( P ), swidth, twidth, fv, 0 );
        pMap->PrepareSampleOptions( paramMap );
        SETFLOAT( Result, fv[ 0 ] );
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
    STATS_INC( SHD_so_shadow1 );

    INIT_SO

    if ( NULL == QGetRenderContextI() )
        return ;

    GET_TEXTURE_PARAMS;

    BEGIN_UNIFORM_SECTION
    GETSTRING( name );
    GETFLOAT( channel );
    IqTextureMap* pMap = QGetRenderContextI() ->GetShadowMap( STRING( name ) );
    END_UNIFORM_SECTION

    __fVarying = TqTrue;
    if ( pMap != 0 && pMap->IsValid() )
    {
        std::valarray<TqFloat> fv;
        pMap->PrepareSampleOptions( paramMap );

        BEGIN_VARYING_SECTION
        GETPOINT( P1 );
        GETPOINT( P2 );
        GETPOINT( P3 );
        GETPOINT( P4 );
        pMap->SampleMap( POINT( P1 ), POINT( P2 ), POINT( P3 ), POINT( P4 ), fv, 0 );
        SETFLOAT( Result, fv[ 0 ] );
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
    STATS_INC( SHD_so_ambient );

    INIT_SO

    // Use the lightsource stack on the current surface
    if ( m_pAttributes != 0 )
    {
        // If this is the first call to illuminance this time round, call all lights and setup the Cl and L caches.
        if ( !m_IlluminanceCacheValid )
        {
            ValidateIlluminanceCache( NULL, NULL, pShader );
        }

        Result->SetColor( gColBlack );

        for ( TqUint light_index = 0; light_index < m_pAttributes ->cLights(); light_index++ )
        {
            __fVarying = TqTrue;

            IqLightsource* lp = m_pAttributes ->pLight( light_index );
            if ( lp->pShader() ->fAmbient() )
            {
                BEGIN_VARYING_SECTION
                // Now Combine the color of all ambient lightsources.
                GETCOLOR( Result );
                CqColor colCl;
                if ( NULL != lp->Cl() )
                    lp->Cl() ->GetColor( colCl, __iGrid );
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
    STATS_INC( SHD_so_diffuse );

    INIT_SO

    // If the illuminance cache is already OK, then we don't need to bother filling in the illuminance parameters.
    if ( !m_IlluminanceCacheValid )
    {
        ValidateIlluminanceCache( NULL, N, pShader );
    }

    IqShaderData* pDefAngle = pShader->CreateTemporaryStorage( type_float, class_uniform );
    if ( NULL == pDefAngle ) return ;

    pDefAngle->SetFloat( PIO2 );

    Result->SetColor( gColBlack );

    __fVarying = TqTrue;
	IqShaderData* __nondiffuse = NULL;
	__nondiffuse = pShader->CreateTemporaryStorage( type_float, class_varying );

    // SO_init_illuminance returns TRUE if there are any non ambient ligthsources available.
    if ( SO_init_illuminance() )
    {
        boost::shared_ptr<IqShader> pLightsource;
        do
        {
            // Get the "__nondiffuse" setting from the current lightsource, if specified.
            TqFloat	__nondiffuse_val;
            if ( m_li < m_pAttributes ->cLights() )
                pLightsource = m_pAttributes ->pLight( m_li ) ->pShader();
            if ( pLightsource )
            {
                pLightsource->GetValue( "__nondiffuse", __nondiffuse );
                /// \note: This is OK here, outside the BEGIN_VARYING_SECTION as, varying in terms of lightsources
                /// is not valid.
                if( NULL != __nondiffuse )
                {
                    __nondiffuse->GetFloat( __nondiffuse_val, 0 );
                    if( __nondiffuse_val != 0.0f )
                        continue;
                }
            }

            // SO_illuminance sets the current state to whether the lightsource illuminates the points or not.
            SO_illuminance( NULL, NULL, N, pDefAngle, NULL );

            PushState();
            GetCurrentState();

            BEGIN_VARYING_SECTION

            // Get the light vector and color from the lightsource.
            CqVector3D Ln;
            L() ->GetVector( Ln, __iGrid );
            Ln.Unit();

            // Combine the light color into the result
            GETCOLOR( Result );
            GETNORMAL( N );
            CqColor colCl;
            Cl() ->GetColor( colCl, __iGrid );
            SETCOLOR( Result, COLOR( Result ) + colCl * ( Ln * NORMAL( N ) ) );

            END_VARYING_SECTION
            PopState();
            // SO_advance_illuminance returns TRUE if there are any more non ambient lightsources.
        }
        while ( SO_advance_illuminance() );
    }
	pShader->DeleteTemporaryStorage( __nondiffuse );
    pShader->DeleteTemporaryStorage( pDefAngle );
}


//----------------------------------------------------------------------
// specular(N,V,roughness)
STD_SOIMPL CqShaderExecEnv::SO_specular( NORMALVAL N, VECTORVAL V, FLOATVAL roughness, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_specular );

    INIT_SO

    // If the illuminance cache is already OK, then we don't need to bother filling in the illuminance parameters.
    if ( !m_IlluminanceCacheValid )
    {
        ValidateIlluminanceCache( NULL, N, pShader );
    }

    IqShaderData* pDefAngle = pShader->CreateTemporaryStorage( type_float, class_uniform );
    if ( NULL == pDefAngle ) return ;

    pDefAngle->SetFloat( PIO2 );

    Result->SetColor( gColBlack );
    __fVarying = TqTrue;

	IqShaderData* __nonspecular = NULL;
	__nonspecular = pShader->CreateTemporaryStorage( type_float, class_varying );

    // SO_init_illuminance returns TRUE if there are any non ambient ligthsources available.
    if ( SO_init_illuminance() )
    {
        boost::shared_ptr<IqShader> pLightsource;
        do
        {
            // Get the "__nonspecular" setting from the current lightsource, if specified.
            TqFloat	__nonspecular_val;
            if ( m_li < m_pAttributes ->cLights() )
                pLightsource = m_pAttributes ->pLight( m_li ) ->pShader();
            if ( pLightsource )
            {
                pLightsource->GetValue( "__nonspecular", __nonspecular );
                /// \note: This is OK here, outside the BEGIN_VARYING_SECTION as, varying in terms of lightsources
                /// is not valid.
                if( NULL != __nonspecular )
                {
                    __nonspecular->GetFloat( __nonspecular_val, 0 );
                    if( __nonspecular_val != 0.0f )
                        continue;
                }
            }

            // SO_illuminance sets the current state to whether the lightsource illuminates the points or not.
            SO_illuminance( NULL, NULL, N, pDefAngle, NULL );

            PushState();
            GetCurrentState();
            BEGIN_VARYING_SECTION

            GETVECTOR( V );
            // Get the ligth vector and color from the lightsource
            CqVector3D Ln;
            L() ->GetVector( Ln, __iGrid );
            Ln.Unit();
            CqVector3D	H = Ln + VECTOR( V );
            H.Unit();

            // Combine the color into the result.
            /// \note The (roughness/8) term emulates the BMRT behaviour for prmanspecular.
            GETCOLOR( Result );
            GETNORMAL( N );
            GETFLOAT( roughness );
            CqColor colCl;
            Cl() ->GetColor( colCl, __iGrid );
            SETCOLOR( Result, COLOR( Result ) + colCl * pow( MAX( 0.0f, NORMAL( N ) * H ), 1.0f / ( FLOAT( roughness ) / 8.0f ) ) );

            END_VARYING_SECTION
            PopState();
            // SO_advance_illuminance returns TRUE if there are any more non ambient lightsources.
        }
        while ( SO_advance_illuminance() );
    }
	pShader->DeleteTemporaryStorage( __nonspecular );
    pShader->DeleteTemporaryStorage( pDefAngle );
}


//----------------------------------------------------------------------
// phong(N,V,size)
STD_SOIMPL CqShaderExecEnv::SO_phong( NORMALVAL N, VECTORVAL V, FLOATVAL size, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_phong );

    INIT_SO

    IqShaderData * pnV = pShader ->CreateTemporaryStorage( type_vector, class_varying );
    IqShaderData* pnN = pShader ->CreateTemporaryStorage( type_normal, class_varying );
    IqShaderData* pR = pShader ->CreateTemporaryStorage( type_vector, class_varying );

    /// note: Not happy about this, the shader should take care of this at construction time,
    /// but at the moment, it can't guarantee the validity of the m_u/vGridRes data members.
    pnV->Initialise( uGridRes(), vGridRes() );
    pnN->Initialise( uGridRes(), vGridRes() );
    pR->Initialise( uGridRes(), vGridRes() );

    SO_normalize( V, pnV );
    SO_normalize( N, pnN );

    __fVarying = TqTrue;
    BEGIN_VARYING_SECTION
    CqVector3D vecnV;
    pnV->GetVector( vecnV, __iGrid );
    pnV->SetVector( -vecnV, __iGrid );
    END_VARYING_SECTION

    SO_reflect( pnV, pnN, pR );

    pShader->DeleteTemporaryStorage( pnV );
    pShader->DeleteTemporaryStorage( pnN );

    // If the illuminance cache is already OK, then we don't need to bother filling in the illuminance parameters.
    if ( !m_IlluminanceCacheValid )
    {
        ValidateIlluminanceCache( NULL, N, pShader );
    }

    IqShaderData* pDefAngle = pShader->CreateTemporaryStorage( type_float, class_uniform );
    if ( NULL == pDefAngle ) return ;

    pDefAngle->SetFloat( PIO2 );

    // Initialise the return value
    Result->SetColor( gColBlack );

    // SO_init_illuminance returns TRUE if there are any non ambient ligthsources available.
    if ( SO_init_illuminance() )
    {
        do
        {
            // SO_illuminance sets the current state to whether the lightsource illuminates the points or not.
            SO_illuminance( NULL, NULL, N, pDefAngle, NULL );

            PushState();
            GetCurrentState();

            BEGIN_VARYING_SECTION

            // Get the light vector and color from the loght source.
            CqVector3D Ln;
            L() ->GetVector( Ln, __iGrid );
            Ln.Unit();

            // Now combine the color into the result.
            GETCOLOR( Result );
            CqVector3D vecR;
            pR->GetVector( vecR, __iGrid );
            GETFLOAT( size );
            CqColor colCl;
            Cl() ->GetColor( colCl, __iGrid );
            SETCOLOR( Result, COLOR( Result ) + colCl * pow( MAX( 0.0f, vecR * Ln ), FLOAT( size ) ) );

            END_VARYING_SECTION

            PopState();
            // SO_advance_illuminance returns TRUE if there are any more non ambient lightsources.
        }
        while ( SO_advance_illuminance() );
    }
    pShader->DeleteTemporaryStorage( pDefAngle );
    pShader->DeleteTemporaryStorage( pR );
}


//----------------------------------------------------------------------
// trace(P,R)
STD_SOIMPL CqShaderExecEnv::SO_trace( POINTVAL P, VECTORVAL R, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_trace );

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
STD_SOIMPL CqShaderExecEnv::SO_illuminance( STRINGVAL Category, POINTVAL P, VECTORVAL Axis, FLOATVAL Angle, DEFVOIDPARAMIMPL )
{
    STATS_INC( SHD_so_illuminance );

    INIT_SO

    BEGIN_UNIFORM_SECTION
    CqString cat( "" );
    if ( NULL != Category ) Category->GetString( cat );
    END_UNIFORM_SECTION

    __fVarying = TqTrue;

    // Fill in the lightsource information, and transfer the results to the shader variables,
    if ( m_pAttributes != 0 )
    {
        IqLightsource * lp = m_pAttributes ->pLight( m_li );

        if ( NULL != Axis ) CHECKVARY( Axis )
            if ( NULL != Angle ) CHECKVARY( Angle )

                TqBool exec = TqTrue;

        if( cat.size() )
        {

            TqBool exclude = TqFalse;
            CqString lightcategories;
            CqString catname;


            if( cat.find( "-" ) == 0 )
            {
                exclude = true;
                catname = cat.substr( 1, cat.size() );
            }
            else
            {
                catname = cat;
            }

            IqShaderData* pcats = lp->pShader()->FindArgument("__category");
            if( pcats )
            {
                pcats->GetString( lightcategories );

                exec = TqFalse;
                // While no matching category has been found...
                TqInt tokenpos = 0, tokenend;
                while( 1 )
                {
                    tokenend = lightcategories.find(',', tokenpos);
                    CqString token = lightcategories.substr( tokenpos, tokenend );
                    if( catname.compare( token ) == 0 )
                    {
                        if( !exclude )
                        {
                            exec = TqTrue;
                            break;
                        }
                    }
                    if( tokenend == std::string::npos )
                        break;
                    else
                        tokenpos = tokenend+1;
                }
            }
        }

        if( exec )
        {
            BEGIN_VARYING_SECTION

            CqVector3D Ln;
            lp->L() ->GetVector( Ln, __iGrid );
            Ln = -Ln;

            // Store them locally on the surface.
            L() ->SetVector( Ln, __iGrid );
            CqColor colCl;
            lp->Cl() ->GetColor( colCl, __iGrid );
            Cl() ->SetColor( colCl, __iGrid );

            // Check if its within the cone.
            Ln.Unit();
            CqVector3D vecAxis( 0, 1, 0 );
            if ( NULL != Axis ) Axis->GetVector( vecAxis, __iGrid );
            TqFloat fAngle = PI;
            if ( NULL != Angle ) Angle->GetFloat( fAngle, __iGrid );

            TqFloat cosangle = Ln * vecAxis;
            cosangle = CLAMP( cosangle, -1, 1 );
            if ( acos( cosangle ) > fAngle )
                m_CurrentState.SetValue( __iGrid, TqFalse );
            else
                m_CurrentState.SetValue( __iGrid, TqTrue );
            END_VARYING_SECTION
        }
    }
}


STD_SOIMPL	CqShaderExecEnv::SO_illuminance( STRINGVAL Category, POINTVAL P, DEFVOIDPARAMIMPL )
{
    STATS_INC( SHD_so_illuminance );

    SO_illuminance( Category, P, NULL, NULL );
}


//----------------------------------------------------------------------
// illuminate(P)
STD_SOIMPL CqShaderExecEnv::SO_illuminate( POINTVAL P, VECTORVAL Axis, FLOATVAL Angle, DEFVOIDPARAMIMPL )
{
    STATS_INC( SHD_so_illuminate );

    INIT_SO

    TqBool res = TqTrue;
    if ( m_Illuminate > 0 ) res = TqFalse;

    __fVarying = TqTrue;
    if ( res )
    {
        BEGIN_VARYING_SECTION
        // Get the point being lit and set the ligth vector.
        GETPOINT( P );
        CqVector3D vecPs;
        Ps() ->GetPoint( vecPs, __iGrid );
        L() ->SetVector( vecPs - POINT( P ), __iGrid );

        // Check if its within the cone.
        CqVector3D Ln;
        L() ->GetVector( Ln, __iGrid );
        Ln.Unit();

        CqVector3D vecAxis( 0.0f, 1.0f, 0.0f );
        if ( NULL != Axis ) Axis->GetVector( vecAxis, __iGrid );
        TqFloat fAngle = PI;
        if ( NULL != Angle ) Angle->GetFloat( fAngle, __iGrid );
        TqFloat cosangle = Ln * vecAxis;
        cosangle = CLAMP( cosangle, -1, 1 );
		if ( acos( cosangle ) > fAngle )
        {
            // Make sure we set the light color to zero in the areas that won't be lit.
            Cl() ->SetColor( CqColor( 0, 0, 0 ), __iGrid );
            m_CurrentState.SetValue( __iGrid, TqFalse );
        }
        else
            m_CurrentState.SetValue( __iGrid, TqTrue );
        END_VARYING_SECTION
    }

    m_Illuminate++;
}


STD_SOIMPL	CqShaderExecEnv::SO_illuminate( POINTVAL P, DEFVOIDPARAMIMPL )
{
    STATS_INC( SHD_so_illuminate );

    SO_illuminate( P, NULL, NULL, pShader );
}


//----------------------------------------------------------------------
// solar()
STD_SOIMPL CqShaderExecEnv::SO_solar( VECTORVAL Axis, FLOATVAL Angle, DEFVOIDPARAMIMPL )
{
    // TODO: Check light cone, and exclude points outside.
    STATS_INC( SHD_so_solar );

    INIT_SO

    TqBool res = TqTrue;
    if ( m_Illuminate > 0 ) res = TqFalse;

    __fVarying = TqTrue;
    BEGIN_VARYING_SECTION
    if ( res )
    {
        CqVector3D vecAxis( 0.0f, 1.0f, 0.0f );
        if ( NULL != Axis ) Axis->GetVector( vecAxis, __iGrid );
        L() ->SetVector( vecAxis, __iGrid );
        m_CurrentState.SetValue( __iGrid, TqTrue );
    }
    END_VARYING_SECTION

    m_Illuminate++;
}


STD_SOIMPL	CqShaderExecEnv::SO_solar( DEFVOIDPARAMIMPL )
{
    STATS_INC( SHD_so_solar );

    SO_solar( NULL, NULL, pShader );
}


//----------------------------------------------------------------------
// printf(s,...)

STD_SOIMPL	CqShaderExecEnv::SO_printf( STRINGVAL str, DEFVOIDPARAMVARIMPL )
{
    STATS_INC( SHD_so_printf );

    INIT_SO

    if ( NULL == QGetRenderContextI() )
        return ;

    CHECKVARY( str )
    TqInt ii;
    for ( ii = 0; ii < cParams; ii++ )
    {
        CHECKVARY( apParams[ ii ] );
    }

    BEGIN_VARYING_SECTION
    GETSTRING( str );
    CqString strA = SO_sprintf( STRING( str ).c_str(), cParams, apParams, __iGrid );
    QGetRenderContextI() ->PrintString( strA.c_str() );
    END_VARYING_SECTION
}


//----------------------------------------------------------------------
// format(s,...)

STD_SOIMPL	CqShaderExecEnv::SO_format( STRINGVAL str, DEFPARAMVARIMPL )
{
    STATS_INC( SHD_so_format );

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
    STATS_INC( SHD_so_concat );

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
        apParams[ ii ] ->GetString( sn, __iGrid );
        strRes += sn;
    }
    SETSTRING( Result, strRes );
    END_VARYING_SECTION
}


//----------------------------------------------------------------------
// noise(v)
STD_SOIMPL CqShaderExecEnv::SO_fcellnoise1( FLOATVAL v, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_fcellnoise1 );

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
    STATS_INC( SHD_so_ccellnoise1 );

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
    STATS_INC( SHD_so_pcellnoise1 );

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
    STATS_INC( SHD_so_fcellnoise2 );

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
    STATS_INC( SHD_so_ccellnoise2 );

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
    STATS_INC( SHD_so_pcellnoise3 );

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
    STATS_INC( SHD_so_fcellnoise3 );

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
    STATS_INC( SHD_so_ccellnoise3 );

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
    STATS_INC( SHD_so_pcellnoise2 );

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
    STATS_INC( SHD_so_fcellnoise4 );

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
    STATS_INC( SHD_so_ccellnoise4 );

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
    STATS_INC( SHD_so_pcellnoise2 );

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
    STATS_INC( SHD_so_atmosphere );

    INIT_SO

    boost::shared_ptr<IqShader> pAtmosphere;

    if ( NULL != m_pAttributes && (m_pAttributes ->pshadAtmosphere(QGetRenderContextI()->Time())) )
        pAtmosphere = m_pAttributes ->pshadAtmosphere(QGetRenderContextI()->Time());

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
    STATS_INC( SHD_so_displacement );

    INIT_SO

    boost::shared_ptr<IqShader> pDisplacement;

    if ( NULL != m_pAttributes && (m_pAttributes ->pshadDisplacement(QGetRenderContextI()->Time())) )
        pDisplacement = m_pAttributes ->pshadDisplacement(QGetRenderContextI()->Time());

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
    STATS_INC( SHD_so_lightsource );

    INIT_SO

    // This should only be called within an Illuminance construct, so m_li should be valid.
    boost::shared_ptr<IqShader> pLightsource;

    BEGIN_UNIFORM_SECTION
    GETSTRING( name );
    if ( m_li < m_pAttributes ->cLights() )
        pLightsource = m_pAttributes ->pLight( m_li ) ->pShader();
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
    STATS_INC( SHD_so_surface );

    INIT_SO

    boost::shared_ptr<IqShader> pSurface;

    if ( GetCurrentSurface() &&
		 NULL != GetCurrentSurface()->pAttributes() && 
		 GetCurrentSurface()->pAttributes() ->pshadSurface(QGetRenderContextI()->Time()) )
        pSurface = GetCurrentSurface()->pAttributes() ->pshadSurface(QGetRenderContextI()->Time());

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
    STATS_INC( SHD_so_attribute );

    INIT_SO

    //Find out if it is a specific attribute request
    BEGIN_UNIFORM_SECTION
    GETSTRING( name );
    TqFloat Ret = 0.0f;

    if ( STRING( name ).compare( "ShadingRate" ) == 0 )
    {
        if ( pV->Type() == type_float )
        {
            pV->SetFloat( m_pAttributes ->GetFloatAttribute( "System", "ShadingRate" ) [ 0 ] );
            Ret = 1.0f;
        }
    }
    else if ( STRING( name ).compare( "Sides" ) == 0 )
    {
        if ( pV->Type() == type_float )
        {
            pV->SetFloat( m_pAttributes ->GetIntegerAttribute( "System", "Sides" ) [ 0 ] );
            Ret = 1.0f;
        }
    }
    else if ( STRING( name ).compare( "Matte" ) == 0 )
    {
        if ( pV->Type() == type_float )
        {
            pV->SetFloat( m_pAttributes ->GetIntegerAttribute( "System", "Matte" ) [ 0 ] );
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
            //const CqParameter* pParam = m_pAttributes ->pParameter( STRING( name ).c_str(), strParam.c_str() );

            Ret = 1.0f;
            if ( NULL != pAttributes() ->GetFloatAttribute( STRING( name ).c_str(), strParam.c_str() ) )
                pV->SetFloat( pAttributes() ->GetFloatAttribute( STRING( name ).c_str(), strParam.c_str() ) [ 0 ] );
            else if ( NULL != pAttributes() ->GetIntegerAttribute( STRING( name ).c_str(), strParam.c_str() ) )
                pV->SetFloat( pAttributes() ->GetIntegerAttribute( STRING( name ).c_str(), strParam.c_str() ) [ 0 ] );
            else if ( NULL != pAttributes() ->GetStringAttribute( STRING( name ).c_str(), strParam.c_str() ) )
                pV->SetString( pAttributes() ->GetStringAttribute( STRING( name ).c_str(), strParam.c_str() ) [ 0 ] );
            else if ( NULL != pAttributes() ->GetPointAttribute( STRING( name ).c_str(), strParam.c_str() ) )
                pV->SetPoint( pAttributes() ->GetPointAttribute( STRING( name ).c_str(), strParam.c_str() ) [ 0 ] );
            else if ( NULL != pAttributes() ->GetVectorAttribute( STRING( name ).c_str(), strParam.c_str() ) )
                pV->SetVector( pAttributes() ->GetVectorAttribute( STRING( name ).c_str(), strParam.c_str() ) [ 0 ] );
            else if ( NULL != pAttributes() ->GetNormalAttribute( STRING( name ).c_str(), strParam.c_str() ) )
                pV->SetNormal( pAttributes() ->GetNormalAttribute( STRING( name ).c_str(), strParam.c_str() ) [ 0 ] );
            else if ( NULL != pAttributes() ->GetColorAttribute( STRING( name ).c_str(), strParam.c_str() ) )
                pV->SetColor( pAttributes() ->GetColorAttribute( STRING( name ).c_str(), strParam.c_str() ) [ 0 ] );
            else if ( NULL != pAttributes() ->GetMatrixAttribute( STRING( name ).c_str(), strParam.c_str() ) )
                pV->SetMatrix( pAttributes() ->GetMatrixAttribute( STRING( name ).c_str(), strParam.c_str() ) [ 0 ] );
            else
                Ret = 0.0f;
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
    STATS_INC( SHD_so_option );

    INIT_SO

    if ( NULL == QGetRenderContextI() )
        return ;

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
                pV->ArrayEntry( 0 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetIntegerOption( "System", "Resolution" ) [ 0 ] ) );
                pV->ArrayEntry( 1 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetIntegerOption( "System", "Resolution" ) [ 1 ] ) );
                pV->ArrayEntry( 2 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "PixelAspectRatio" ) [ 2 ] ) );
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
                pV->ArrayEntry( 0 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "CropWindow" ) [ 0 ] ) );
                pV->ArrayEntry( 1 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "CropWindow" ) [ 1 ] ) );
                pV->ArrayEntry( 2 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "CropWindow" ) [ 2 ] ) );
                pV->ArrayEntry( 3 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "CropWindow" ) [ 3 ] ) );
                Ret = 1.0f;
            }
        }
    }
    else if ( STRING( name ).compare( "FrameAspectRatio" ) == 0 )
    {
        if ( pV->Type() == type_float )
        {
            pV->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "FrameAspectRatio" ) [ 0 ] ) );
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
                pV->ArrayEntry( 0 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "DepthOfField" ) [ 0 ] ) );
                pV->ArrayEntry( 1 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "DepthOfField" ) [ 1 ] ) );
                pV->ArrayEntry( 2 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "DepthOfField" ) [ 2 ] ) );
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
                pV->ArrayEntry( 0 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "Shutter" ) [ 0 ] ) );
                pV->ArrayEntry( 1 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "Shutter" ) [ 1 ] ) );
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
                pV->ArrayEntry( 0 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "Clipping" ) [ 0 ] ) );
                pV->ArrayEntry( 1 ) ->SetFloat( static_cast<TqFloat>( QGetRenderContextI() ->GetFloatOption( "System", "Clipping" ) [ 1 ] ) );
                Ret = 1.0f;
            }
        }
    }
    else
    {
        CqString strName = STRING( name ).c_str();
        int iColon = strName.find_first_of( ':' );
        if ( iColon >= 0 )
        {
            CqString strParam = strName.substr( iColon + 1, strName.size() - iColon - 1 );
            strName = strName.substr( 0, iColon );
            //const CqParameter* pParam = m_pAttributes ->pParameter( strName.c_str(), strParam.c_str() );

            Ret = 1.0f;

            if ( NULL != QGetRenderContextI() ->GetStringOption( strName.c_str(), strParam.c_str() ) )
                pV->SetString( QGetRenderContextI() ->GetStringOption( strName.c_str(), strParam.c_str() ) [ 0 ] );
            else if ( NULL != QGetRenderContextI() ->GetIntegerOption( strName.c_str(), strParam.c_str() ) )
                pV->SetFloat( QGetRenderContextI() ->GetIntegerOption( strName.c_str(), strParam.c_str() ) [ 0 ] );

            else if ( NULL != QGetRenderContextI() ->GetPointOption( strName.c_str(), strParam.c_str() ) )
                pV->SetPoint( QGetRenderContextI() ->GetPointOption( strName.c_str(), strParam.c_str() ) [ 0 ] );

            else if ( NULL != QGetRenderContextI() ->GetColorOption( strName.c_str(), strParam.c_str() ) )
                pV->SetColor( QGetRenderContextI() ->GetColorOption( strName.c_str(), strParam.c_str() ) [ 0 ] );
            else if ( NULL != QGetRenderContextI() ->GetFloatOption( strName.c_str(), strParam.c_str() ) )
                pV->SetFloat( QGetRenderContextI() ->GetFloatOption( strName.c_str(), strParam.c_str() ) [ 0 ] );
            /* did not deal with Vector, Normal and Matrix yet */
            else
                Ret = 0.0f;
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
    STATS_INC( SHD_so_rendererinfo );

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
    STATS_INC( SHD_so_incident );

    INIT_SO

    BEGIN_UNIFORM_SECTION
    Result->SetValue( 0.0f, 0 );
    END_UNIFORM_SECTION
}


//----------------------------------------------------------------------
// opposite

STD_SOIMPL CqShaderExecEnv::SO_opposite( STRINGVAL name, IqShaderData* pV, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_opposite );

    INIT_SO

    BEGIN_UNIFORM_SECTION
    Result->SetValue( 0.0f, 0 );
    END_UNIFORM_SECTION
}


//----------------------------------------------------------------------
// ctransform(s,s,c)
STD_SOIMPL CqShaderExecEnv::SO_ctransform( STRINGVAL fromspace, STRINGVAL tospace, COLORVAL c, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_ctransform );

    INIT_SO

    CHECKVARY( c )
    CHECKVARY( Result )

    BEGIN_UNIFORM_SECTION
    CqString strfromspace( "rgb" );
    if ( NULL != fromspace ) fromspace->GetString( strfromspace );
    GETSTRING( tospace );
    END_UNIFORM_SECTION

    BEGIN_VARYING_SECTION
    GETCOLOR( c );
    CqColor res( COLOR( c ) );
    if ( strfromspace.compare( "hsv" ) == 0 ) res = COLOR( c ).hsvtorgb();
    else if ( strfromspace.compare( "hsl" ) == 0 ) res = COLOR( c ).hsltorgb();
    else if ( strfromspace.compare( "XYZ" ) == 0 ) res = COLOR( c ).XYZtorgb();
    else if ( strfromspace.compare( "xyY" ) == 0 ) res = COLOR( c ).xyYtorgb();
    else if ( strfromspace.compare( "YIQ" ) == 0 ) res = COLOR( c ).YIQtorgb();

    if ( STRING( tospace ).compare( "hsv" ) == 0 ) res = COLOR( c ).rgbtohsv();
    else if ( STRING( tospace ).compare( "hsl" ) == 0 ) res = COLOR( c ).rgbtohsl();
    else if ( STRING( tospace ).compare( "XYZ" ) == 0 ) res = COLOR( c ).rgbtoXYZ();
    else if ( STRING( tospace ).compare( "xyY" ) == 0 ) res = COLOR( c ).rgbtoxyY();
    else if ( STRING( tospace ).compare( "YIQ" ) == 0 ) res = COLOR( c ).rgbtoYIQ();

    SETCOLOR( Result, res );
    END_VARYING_SECTION
}


//----------------------------------------------------------------------
// ctransform(s,c)
STD_SOIMPL CqShaderExecEnv::SO_ctransform( STRINGVAL tospace, COLORVAL c, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_ctransform );

    assert( pShader != 0 );
    SO_ctransform( NULL, tospace, c, Result, pShader );
}


//----------------------------------------------------------------------
// ctransform(s,c)
STD_SOIMPL CqShaderExecEnv::SO_ptlined( POINTVAL P0, POINTVAL P1, POINTVAL Q, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_ptlined );

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
    STATS_INC( SHD_so_inversesqrt );

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
    STATS_INC( SHD_so_match );

    INIT_SO

    BEGIN_UNIFORM_SECTION
    float r = 0.0f;
    GETSTRING( a );
    GETSTRING( b );
    if ( STRING( a ).size() == 0 ) r = 0.0f;
    else if ( STRING( b ).size() == 0 ) r = 0.0f;
    else
    {
        // MJO> Only check the occurrence of string b in string a
        // It doesn't support the regular expression yet
        r = ( float ) ( strstr( STRING( b ).c_str(), STRING( a ).c_str() ) != 0 );
    }

    SETFLOAT( Result, r );
    END_UNIFORM_SECTION
}


//----------------------------------------------------------------------
// pnoise(u,period)
STD_SOIMPL CqShaderExecEnv::SO_fpnoise1( FLOATVAL v, FLOATVAL period, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_fpnoise1 );

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
    STATS_INC( SHD_so_fpnoise2 );

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
    STATS_INC( SHD_so_fnoise3 );

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
    STATS_INC( SHD_so_fnoise4 );

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
    STATS_INC( SHD_so_cpnoise1 );

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
    STATS_INC( SHD_so_cpnoise2 );

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
    STATS_INC( SHD_so_cpnoise3 );

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
    STATS_INC( SHD_so_cpnoise4 );

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
    STATS_INC( SHD_so_ppnoise1 );

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
    STATS_INC( SHD_so_ppnoise2 );

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
    STATS_INC( SHD_so_ppnoise3 );

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
    STATS_INC( SHD_so_ppnoise4 );

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
    STATS_INC( SHD_so_rotate );

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
    STATS_INC( SHD_so_filterstep );

    TqFloat Deffloat = 0.0f;
    INIT_SO

    GET_FILTER_PARAMS;

    CHECKVARY( edge )
    CHECKVARY( s1 )
    CHECKVARY( Result )

    BEGIN_UNIFORM_SECTION
    TqFloat fdu, fdv;
    du() ->GetFloat( fdu );
    dv() ->GetFloat( fdv );
    END_UNIFORM_SECTION

    BEGIN_VARYING_SECTION
    GETFLOAT( s1 );
    GETFLOAT( edge );
    TqFloat dsdu = SO_DuType<TqFloat>( s1, __iGrid, this, Deffloat );
    TqFloat dsdv = SO_DvType<TqFloat>( s1, __iGrid, this, Deffloat );

    TqFloat uwidth = fabs( dsdu * fdu );
    TqFloat vwidth = fabs( dsdv * fdv );

    TqFloat w = uwidth + vwidth;
    w *= _pswidth;

    SETFLOAT( Result, CLAMP( ( FLOAT( s1 ) + w / 2.0f - FLOAT( edge ) ) / w, 0, 1 ) );

    //	TqFloat res  = RiCatmullRomFilter( FLOAT( s1 ) - FLOAT( edge ), 0, w, 0);
    //	SETFLOAT( Result, res );

    //	std::cout << res << std::endl;
    //	TqFloat res = 1.0f - CLAMP( ( FLOAT( s1 ) + w / 2.0f - FLOAT( edge ) ) / w, 0, 1 );
    //	if( res > 0.0f )
    //		std::cout << "Aqsis angle/dangle: " << FLOAT(s1) << ", edge: " << FLOAT(edge) << ", dsdu: " << dsdu << ", dsdv: " << dsdv << ", w: " << w << ", res: " << res << std::endl;
    END_VARYING_SECTION
}

//----------------------------------------------------------------------
// filterstep(edge,s1,s2)
STD_SOIMPL CqShaderExecEnv::SO_filterstep2( FLOATVAL edge, FLOATVAL s1, FLOATVAL s2, DEFPARAMVARIMPL )
{
    STATS_INC( SHD_so_filterstep2 );

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
    STATS_INC( SHD_so_specularbrdf );

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
    Cl() ->GetColor( colCl, __iGrid );
    SETCOLOR( Result, colCl * pow( MAX( 0.0f, NORMAL( N ) * H ), 1.0f / ( FLOAT( rough ) / 8.0f ) ) );
    END_VARYING_SECTION
}


//----------------------------------------------------------------------
// determinant(m)
STD_SOIMPL CqShaderExecEnv::SO_determinant( MATRIXVAL M, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_determinant );

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
    STATS_INC( SHD_so_mtranslate );

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
    STATS_INC( SHD_so_mrotate );

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
    STATS_INC( SHD_so_mscale );

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
    STATS_INC( SHD_so_setmcomp );

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
    MATRIX( M ) [ static_cast<TqInt>( FLOAT( r ) ) ][ static_cast<TqInt>( FLOAT( c ) ) ] = FLOAT( v );
    MATRIX( M ).SetfIdentity( TqFalse );
    M->SetValue( MATRIX( M ), __iGrid );
    END_VARYING_SECTION
}


//----------------------------------------------------------------------
// spline(value, f1,f2,...,fn)
STD_SOIMPL	CqShaderExecEnv::SO_fsplinea( FLOATVAL value, FLOATARRAYVAL a, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_fsplinea );

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
    STATS_INC( SHD_so_csplinea );

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
    STATS_INC( SHD_so_psplinea );

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
    STATS_INC( SHD_so_sfsplinea );

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
    spline.SetBasis( STRING( basis ) );
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
    STATS_INC( SHD_so_scsplinea );

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
    spline.SetBasis( STRING( basis ) );
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
    STATS_INC( SHD_so_spsplinea );

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
    spline.SetBasis( STRING( basis ) );
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
            a->ArrayEntry( j ) ->GetPoint( vecTemp, __iGrid );
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
    STATS_INC( SHD_so_shadername );

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
    STATS_INC( SHD_so_shadername2 );

    INIT_SO

    CqString strName( "" );
    CqString strShader;
	boost::shared_ptr<IqShader> pSurface;
	boost::shared_ptr<IqShader> pDisplacement;
	boost::shared_ptr<IqShader> pAtmosphere;
    if( m_pAttributes )
	{
		pSurface = m_pAttributes ->pshadSurface(QGetRenderContextI()->Time());
		pDisplacement = m_pAttributes ->pshadDisplacement(QGetRenderContextI()->Time());
		pAtmosphere = m_pAttributes ->pshadAtmosphere(QGetRenderContextI()->Time());
	}

    CHECKVARY( Result )

    BEGIN_VARYING_SECTION
    strName = "";
    GETSTRING( shader );
    if ( STRING( shader ).compare( "surface" ) == 0 && pSurface ) strName = pSurface->strName();
    else if ( STRING( shader ).compare( "displacement" ) == 0 && pDisplacement ) strName = pDisplacement->strName();
    else if ( STRING( shader ).compare( "atmosphere" ) == 0 && pAtmosphere ) strName = pAtmosphere->strName();
    SETSTRING( Result, strName );
    END_VARYING_SECTION
}


//----------------------------------------------------------------------
// textureinfo
// support resolution, type, channels, projectionmatrix(*) and viewingmatrix(*)
// User has to provide an array of TqFloat (2) for resolution
//                     an string for type
//                     an integer for channels
//                     an array of floats (16) for both projectionmatrix and viewingmatrix
//                     (*) the name must be a shadow map
//

STD_SOIMPL CqShaderExecEnv::SO_textureinfo( STRINGVAL name, STRINGVAL dataname, IqShaderData* pV, DEFPARAMIMPL )
{
    STATS_INC( SHD_so_textureinfo );

    INIT_SO

    if ( NULL == QGetRenderContextI() )
        return ;

    TqFloat Ret = 0.0f;
    IqTextureMap* pMap = NULL;
    IqTextureMap *pSMap = NULL;
    IqTextureMap *pLMap = NULL;
    IqTextureMap *pEMap = NULL;
    IqTextureMap *pTMap = NULL;

    BEGIN_UNIFORM_SECTION
    GETSTRING( name );
    GETSTRING( dataname );

    if ( !pMap && strstr( STRING( name ).c_str(), ".tif" ) )
    {
        pTMap = QGetRenderContextI() ->GetTextureMap( STRING( name ) );
        if ( pTMap && ( pTMap->Type() == MapType_Texture ) )
        {
            pMap = pTMap;
        }
        else if ( pTMap ) delete pTMap;
    }
    if ( !pMap )
    {
        pSMap = QGetRenderContextI() ->GetShadowMap( STRING( name ) );
        if ( pSMap && ( pSMap->Type() == MapType_Shadow ) )
        {
            pMap = pSMap;
        }
        else if ( pSMap ) delete pSMap;
    }

    if ( !pMap )
    {
        pEMap = QGetRenderContextI() ->GetEnvironmentMap( STRING( name ) );
        if ( pEMap && ( pEMap->Type() == MapType_Environment ) )
        {
            pMap = pEMap;
        }
        else if ( pEMap ) delete pEMap;
    }

    if ( !pMap )
    {
        pTMap = QGetRenderContextI() ->GetTextureMap( STRING( name ) );
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
        if ( ( ( pV->Type() == type_float ) && ( pV->ArrayLength() == 16 ) ) ||
                ( pV->Type() == type_matrix ) )
        {
            if ( pSMap )   // && pSMap->Type() == MapType_Shadow)
            {

                CqMatrix m = pSMap->GetMatrix( 0 );  /* WorldToCamera */
                if ( pV->ArrayLength() == 16 )
                {

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

                }
                else
                {
                    pV->SetMatrix( m, 0 );
                }
                Ret = 1.0f;

            }

        }
    }

    if ( STRING( dataname ).compare( "projectionmatrix" ) == 0 )
    {
        if ( ( ( pV->Type() == type_float ) && ( pV->ArrayLength() == 16 ) ) ||
                ( pV->Type() == type_matrix ) )
        {
            if ( pSMap )    // && pSMap->Type() == MapType_Shadow)
            {

                CqMatrix m = pSMap->GetMatrix( 1 ); /* WorldToScreen */

                if ( pV->ArrayLength() == 16 )
                {
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


                }
                else
                {
                    pV->SetMatrix( m, 0 );

                }
                Ret = 1.0f;
            }

        }
    }

    delete pMap;

    SETFLOAT( Result, Ret );
    END_UNIFORM_SECTION
}

// SIGGRAPH 2002; Larry G. Bake functions

const int batchsize = 10240; // elements to buffer before writing
// Make sure we're thread-safe on those file writes

class BakingChannel
{
    // A "BakingChannel" is the buffer for a single baking output file.
    // We buffer up samples until "batchsize" has been accepted, then
    // write them all at once. This keeps us from constantly accessing
    // the disk. Note that we are careful to use a mutex to keep
    // simultaneous multithreaded writes from clobbering each other.
public:
    // Constructors
    BakingChannel ( void ) : buffered( 0 ), data( NULL ), filename( NULL )
    { }
    BakingChannel ( const char *_filename, int _elsize )
    {
        init ( _filename, _elsize );
    }
    // Initialize - allocate memory, etc.
    void init ( const char *_filename, int _elsize )
    {
        elsize = _elsize + 2;
        buffered = 0;
        data = new float [ elsize * batchsize ];
        filename = strdup ( _filename );
    }
    // Destructor: write buffered output, close file, deallocate
    ~BakingChannel ()
    {
        writedata();
        free ( filename );
        delete [] data;
    }
    // Add one more data item
    void moredata ( float s, float t, float *newdata )
    {
        if ( buffered >= batchsize )
            writedata();
        float *f = data + elsize * buffered;
        f[ 0 ] = s;
        f[ 1 ] = t;
        for ( int j = 2; j < elsize; ++j )
            f[ j ] = newdata[ j - 2 ];
        ++buffered;
    }
private:
    int elsize; // element size (e.g., 3 for colors)
    int buffered; // how many elements are currently buffered
    float *data; // pointer to the allocated buffer (new'ed)
    char *filename; // pointer to filename (strdup'ed)
    // Write any buffered data to the file
    void writedata ( void )
    {

        if ( buffered > 0 && filename != NULL )
        {
            FILE * file = fopen ( filename, "a" );
            float *f = data;
            for ( int i = 0; i < buffered; ++i, f += elsize )
            {
                for ( int j = 0; j < elsize; ++j )
                    fprintf ( file, "%g ", f[ j ] );
                fprintf ( file, "\n" );
            }
            fclose ( file );
        }

        buffered = 0;
    }
};

typedef std::map<std::string, BakingChannel> BakingData;


extern "C" BakingData *bake_init()
{
    BakingData * bd = new BakingData;

    return bd;
}
extern "C" void bake_done( BakingData *bd )
{
    delete bd; // Will destroy bd, and in turn all its BakingChannel's
}
// Workhorse routine -- look up the channel name, add a new BakingChannel
// if it doesn't exist, add one point's data to the channel.
extern "C" void bake ( BakingData *bd, const std::string &name,
                           float s, float t, int elsize, float *data )
{
    BakingData::iterator found = bd->find ( name );

    if ( found == bd->end() )
    {
        // This named map doesn't yet exist
        ( *bd ) [ name ] = BakingChannel();
        found = bd->find ( name );
        BakingChannel &bc = ( found->second );
        bc.init ( name.c_str(), elsize );
        bc.moredata ( s, t, data );
    }
    else
    {
        BakingChannel &bc = ( found->second );
        bc.moredata ( s, t, data );
    }
}

extern "C" int bake_f( BakingData *bd, char *name, float s, float t, float f )
{
    float * bakedata = ( float * ) & f;

    bake ( bd, name, s, t, 1, bakedata );
    return 0;
}
// for baking a triple -- just call bake with appropriate args
extern "C" int bake_3( BakingData *bd, char *name, float s, float t, float *bakedata )
{
    bake ( bd, name, s, t, 3, bakedata );
    return 0;
}



STD_SOIMPL CqShaderExecEnv::SO_bake_f( STRINGVAL name, FLOATVAL s, FLOATVAL t, FLOATVAL f, DEFVOIDPARAMVARIMPL )
{
    STATS_INC( SHD_so_bake );

    INIT_SO

    CHECKVARY( f );
    CHECKVARY( s );
    CHECKVARY( t );

    BEGIN_UNIFORM_SECTION
    GETSTRING( name );
    BakingData *bd = bake_init(  /*STRING( name).c_str() */ );
    END_UNIFORM_SECTION

    BEGIN_VARYING_SECTION
    GETFLOAT( s );
    GETFLOAT( t );
    GETFLOAT( f );
    bake_f( bd, ( char * ) STRING( name ).c_str(), FLOAT( s ), FLOAT( t ), FLOAT( f ) );
    END_VARYING_SECTION

    BEGIN_UNIFORM_SECTION
    bake_done( bd );
    END_UNIFORM_SECTION
}

STD_SOIMPL CqShaderExecEnv::SO_bake_3c( STRINGVAL name, FLOATVAL s, FLOATVAL t, COLORVAL f, DEFVOIDPARAMVARIMPL )
{
    STATS_INC( SHD_so_bake );

    INIT_SO

    CHECKVARY( f );
    CHECKVARY( s );
    CHECKVARY( t );

    BEGIN_UNIFORM_SECTION
    TqFloat rgb[ 3 ];

    GETSTRING( name );
    BakingData *bd = bake_init(  /*(char *) STRING( name ).c_str()*/ );
    END_UNIFORM_SECTION
    BEGIN_VARYING_SECTION
    GETFLOAT( s );
    GETFLOAT( t );
    GETCOLOR( f );
    COLOR( f ).GetColorRGB( &rgb[ 0 ], &rgb[ 1 ], &rgb[ 2 ] );
    bake_3( bd, ( char * ) STRING( name ).c_str(), FLOAT( s ), FLOAT( t ), rgb );
    END_VARYING_SECTION
    BEGIN_UNIFORM_SECTION
    bake_done( bd );
    END_UNIFORM_SECTION
}

STD_SOIMPL CqShaderExecEnv::SO_bake_3n( STRINGVAL name, FLOATVAL s, FLOATVAL t, NORMALVAL f, DEFVOIDPARAMVARIMPL )
{
    STATS_INC( SHD_so_bake );

    INIT_SO

    CHECKVARY( f );
    CHECKVARY( s );
    CHECKVARY( t );

    BEGIN_UNIFORM_SECTION
    GETSTRING( name );
    BakingData *bd = bake_init(  /*(char *) STRING( name ).c_str() */ );
    END_UNIFORM_SECTION

    BEGIN_VARYING_SECTION
    GETFLOAT( s );
    GETFLOAT( t );
    GETNORMAL( f );
    TqFloat rgb[ 3 ];
    rgb[ 0 ] = VECTOR( f ) [ 0 ];
    rgb[ 1 ] = VECTOR( f ) [ 1 ];
    rgb[ 2 ] = VECTOR( f ) [ 2 ];
    bake_3( bd, ( char * ) STRING( name ).c_str(), FLOAT( s ), FLOAT( t ), rgb );
    END_VARYING_SECTION

    BEGIN_UNIFORM_SECTION
    bake_done( bd );
    END_UNIFORM_SECTION
}

STD_SOIMPL CqShaderExecEnv::SO_bake_3p( STRINGVAL name, FLOATVAL s, FLOATVAL t, POINTVAL f, DEFVOIDPARAMVARIMPL )
{
    STATS_INC( SHD_so_bake );

    INIT_SO

    CHECKVARY( f );
    CHECKVARY( s );
    CHECKVARY( t );

    BEGIN_UNIFORM_SECTION
    GETSTRING( name );
    BakingData *bd = bake_init(  /*(char *) STRING( name ).c_str()  */ );
    END_UNIFORM_SECTION

    BEGIN_VARYING_SECTION
    GETFLOAT( s );
    GETFLOAT( t );
    GETPOINT( f );
    TqFloat rgb[ 3 ];
    rgb[ 0 ] = VECTOR( f ) [ 0 ];
    rgb[ 1 ] = VECTOR( f ) [ 1 ];
    rgb[ 2 ] = VECTOR( f ) [ 2 ];
    bake_3( bd, ( char * ) STRING( name ).c_str(), FLOAT( s ), FLOAT( t ), rgb );
    END_VARYING_SECTION

    BEGIN_UNIFORM_SECTION
    bake_done( bd );
    END_UNIFORM_SECTION
}

STD_SOIMPL CqShaderExecEnv::SO_bake_3v( STRINGVAL name, FLOATVAL s, FLOATVAL t, VECTORVAL f, DEFVOIDPARAMVARIMPL )
{
    STATS_INC( SHD_so_bake );

    INIT_SO
    CHECKVARY( f );
    CHECKVARY( s );
    CHECKVARY( t );

    BEGIN_UNIFORM_SECTION
    GETSTRING( name );
    BakingData *bd = bake_init(  /*(char *) STRING( name ).c_str()  */ );
    END_UNIFORM_SECTION

    BEGIN_VARYING_SECTION
    GETFLOAT( s );
    GETFLOAT( t );
    GETVECTOR( f );
    TqFloat rgb[ 3 ];
    rgb[ 0 ] = VECTOR( f ) [ 0 ];
    rgb[ 1 ] = VECTOR( f ) [ 1 ];
    rgb[ 2 ] = VECTOR( f ) [ 2 ];
    bake_3( bd, ( char * ) STRING( name ).c_str(), FLOAT( s ), FLOAT( t ), rgb );
    END_VARYING_SECTION

    BEGIN_UNIFORM_SECTION
    bake_done( bd );
    END_UNIFORM_SECTION
}


// We manually decalr th
STD_SOIMPL CqShaderExecEnv::SO_external( DSOMethod method, void *initData, DEFPARAMVARIMPL )
{
    STATS_INC( SHD_so_external );

    INIT_SO

    CHECKVARY( Result );
    int p;
    for ( p = 0;p < cParams;p++ )
    {
        CHECKVARY( apParams[ p ] );
    };

    int dso_argc = cParams + 1; // dso_argv[0] is used for the return value
    void **dso_argv = new void * [ dso_argc ] ;

    // create storage for the returned value
    switch ( Result->Type() )
    {

    case type_float:
        dso_argv[ 0 ] = ( void* ) new TqFloat; break;
    case type_point:
    case type_color:
    case type_triple:
    case type_vector:
    case type_normal:
    case type_hpoint:
        dso_argv[ 0 ] = ( void* ) new TqFloat[ 3 ]; break;
    case type_string:
        dso_argv[ 0 ] = ( void* ) new STRING_DESC;
        ( ( STRING_DESC* ) dso_argv[ 0 ] ) ->s = NULL;
        ( ( STRING_DESC* ) dso_argv[ 0 ] ) ->bufflen = 0;
        break;
    case type_matrix:
    case type_sixteentuple:
        dso_argv[ 0 ] = ( void* ) new TqFloat[ 16 ];
        break;
    default:
        // Unhandled TYpe
        break;
    };

    // Allocate space for the arguments
    for ( p = 1;p <= cParams;p++ )
    {

        switch ( apParams[ p - 1 ] ->Type() )
        {
        case type_float:
            dso_argv[ p ] = ( void* ) new TqFloat; break;
        case type_hpoint:
        case type_point:
        case type_triple:  // This seems reasonable
        case type_vector:
        case type_normal:
        case type_color:
            dso_argv[ p ] = ( void* ) new TqFloat[ 3 ]; break;
        case type_string:
            dso_argv[ p ] = ( void* ) new STRING_DESC;
            ( ( STRING_DESC* ) dso_argv[ p ] ) ->s = NULL;
            ( ( STRING_DESC* ) dso_argv[ p ] ) ->bufflen = 0;
            break;
        case type_matrix:
        case type_sixteentuple:
            dso_argv[ 0 ] = ( void* ) new TqFloat[ 16 ];
            break;
        default:
            // Unhandled TYpe
            break;
        };
    };


    BEGIN_VARYING_SECTION

    // Convert the arguments to the required format for the DSO
    for ( p = 1;p <= cParams;p++ )
    {

        switch ( apParams[ p - 1 ] ->Type() )
        {
        case type_float:
            apParams[ p - 1 ] ->GetFloat( *( ( float* ) dso_argv[ p ] ), __iGrid );
            break;
        case type_hpoint:
        case type_point:
            {
                CqVector3D v;
                apParams[ p - 1 ] ->GetPoint( v, __iGrid );
                ( ( float* ) dso_argv[ p ] ) [ 0 ] = v[ 0 ];
                ( ( float* ) dso_argv[ p ] ) [ 1 ] = v[ 1 ];
                ( ( float* ) dso_argv[ p ] ) [ 2 ] = v[ 2 ];
            }
            ; break;
        case type_triple:  // This seems reasonable
        case type_vector:
            {
                CqVector3D v;
                apParams[ p - 1 ] ->GetVector( v, __iGrid );
                ( ( float* ) dso_argv[ p ] ) [ 0 ] = v[ 0 ];
                ( ( float* ) dso_argv[ p ] ) [ 1 ] = v[ 1 ];
                ( ( float* ) dso_argv[ p ] ) [ 2 ] = v[ 2 ];
            }
            ; break;
        case type_normal:
            {
                CqVector3D v;
                apParams[ p - 1 ] ->GetNormal( v, __iGrid );
                ( ( float* ) dso_argv[ p ] ) [ 0 ] = v[ 0 ];
                ( ( float* ) dso_argv[ p ] ) [ 1 ] = v[ 1 ];
                ( ( float* ) dso_argv[ p ] ) [ 2 ] = v[ 2 ];
            }
            ; break;
        case type_color:
            {
                CqColor c;
                apParams[ p - 1 ] ->GetColor( c, __iGrid );
                ( ( float* ) dso_argv[ p ] ) [ 0 ] = c[ 0 ];
                ( ( float* ) dso_argv[ p ] ) [ 1 ] = c[ 1 ];
                ( ( float* ) dso_argv[ p ] ) [ 2 ] = c[ 2 ];
            }
            ; break;
        case type_string:
            {
                CqString s;
                apParams[ p - 1 ] ->GetString( s, __iGrid );
                char *ps = new char[ s.size() + 1 ];
                strncpy ( ps, s.c_str(), s.size() + 1 );
                ( ( STRING_DESC* ) dso_argv[ p ] ) ->s = ps;
                ( ( STRING_DESC* ) dso_argv[ p ] ) ->bufflen = s.size() + 1;
            }
            ; break;
        case type_matrix:
        case type_sixteentuple:
            {
                CqMatrix m;
                int r, c;
                apParams[ p - 1 ] ->GetMatrix( m, __iGrid );
                for ( r = 0; r < 4; r++ )
                    for ( c = 0; c < 4; c++ )
                        ( ( TqFloat* ) dso_argv[ p ] ) [ ( r * 4 ) + c ] = m[ r ][ c ];
            }
            ; break;
        default:
            // Unhandled TYpe
            break;
        };
    };

    // Atlast, we call the shadeop method, looks rather dull after all this effort.
    method( initData, dso_argc, dso_argv );

    // Pass the returned value back to aqsis
    switch ( Result->Type() )
    {

    case type_float:
        {
            TqFloat val = *( ( float* ) ( dso_argv[ 0 ] ) );
            Result->SetFloat( val, __iGrid );
        }
        ; break;
    case type_hpoint:
    case type_point:
        {
            CqVector3D v;
            v[ 0 ] = ( ( float* ) dso_argv[ 0 ] ) [ 0 ];
            v[ 1 ] = ( ( float* ) dso_argv[ 0 ] ) [ 1 ];
            v[ 2 ] = ( ( float* ) dso_argv[ 0 ] ) [ 2 ];
            Result->SetPoint( v, __iGrid );
        }
        ; break;
    case type_triple:  // This seems reasonable
    case type_vector:
        {
            CqVector3D v;
            v[ 0 ] = ( ( float* ) dso_argv[ 0 ] ) [ 0 ];
            v[ 1 ] = ( ( float* ) dso_argv[ 0 ] ) [ 1 ];
            v[ 2 ] = ( ( float* ) dso_argv[ 0 ] ) [ 2 ];
            Result->SetVector( v, __iGrid );
        }
        ; break;
    case type_normal:
        {
            CqVector3D v;
            v[ 0 ] = ( ( float* ) dso_argv[ 0 ] ) [ 0 ];
            v[ 1 ] = ( ( float* ) dso_argv[ 0 ] ) [ 1 ];
            v[ 2 ] = ( ( float* ) dso_argv[ 0 ] ) [ 2 ];
            Result->SetNormal( v, __iGrid );
        }
        ; break;
    case type_color:
        {
            CqColor c;
            c[ 0 ] = ( ( float* ) dso_argv[ 0 ] ) [ 0 ];
            c[ 1 ] = ( ( float* ) dso_argv[ 0 ] ) [ 1 ];
            c[ 2 ] = ( ( float* ) dso_argv[ 0 ] ) [ 2 ];
            Result->SetColor( c, __iGrid );
        }
        ; break;
    case type_string:
        {
            CqString s( ( ( STRING_DESC* ) dso_argv[ 0 ] ) ->s );
            Result->SetString( s, __iGrid );
        }
        ; break;
    case type_matrix:
    case type_sixteentuple:
        {
            CqMatrix m( ( float* ) dso_argv[ 0 ] );
            Result->SetMatrix( m, __iGrid );
        }
        ; break;
    default:
        // Unhandled TYpe
        std::cout << "Unsupported type" << std::endl;
        break;
    };


    // Set the values that were altered by the Shadeop
    for ( p = 1;p <= cParams;p++ )
    {
        switch ( apParams[ p - 1 ] ->Type() )
        {
        case type_float:
            {
                TqFloat val = *( ( float* ) dso_argv[ p ] ) ;
                apParams[ p - 1 ] ->SetFloat( val, __iGrid );
            }
            ;break;
        case type_hpoint:
        case type_point:
            {
                CqVector3D v;
                v[ 0 ] = ( ( float* ) dso_argv[ p ] ) [ 0 ];
                v[ 1 ] = ( ( float* ) dso_argv[ p ] ) [ 1 ];
                v[ 2 ] = ( ( float* ) dso_argv[ p ] ) [ 2 ];
                apParams[ p - 1 ] ->SetPoint( v, __iGrid );
            }
            ;break;
        case type_triple:  // This seems reasonable
        case type_vector:
            {
                CqVector3D v;
                v[ 0 ] = ( ( float* ) dso_argv[ p ] ) [ 0 ];
                v[ 1 ] = ( ( float* ) dso_argv[ p ] ) [ 1 ];
                v[ 2 ] = ( ( float* ) dso_argv[ p ] ) [ 2 ];
                apParams[ p - 1 ] ->SetVector( v, __iGrid );
            }
            ;break;
        case type_normal:
            {
                CqVector3D v;
                v[ 0 ] = ( ( float* ) dso_argv[ p ] ) [ 0 ];
                v[ 1 ] = ( ( float* ) dso_argv[ p ] ) [ 1 ];
                v[ 2 ] = ( ( float* ) dso_argv[ p ] ) [ 2 ];
                apParams[ p - 1 ] ->SetNormal( v, __iGrid );
            }
            ;break;
        case type_color:
            {
                CqColor c;
                c[ 0 ] = ( ( float* ) dso_argv[ p ] ) [ 0 ];
                c[ 1 ] = ( ( float* ) dso_argv[ p ] ) [ 1 ];
                c[ 2 ] = ( ( float* ) dso_argv[ p ] ) [ 2 ];
                apParams[ p - 1 ] ->SetColor( c, __iGrid );
            }
            ;break;
        case type_string:
            {
                CqString s( ( ( STRING_DESC* ) dso_argv[ p ] ) ->s );
                apParams[ p - 1 ] ->SetString( s, __iGrid );
            }
            ; break;
        case type_matrix:
        case type_sixteentuple:
            {
                CqMatrix m( ( float* ) dso_argv[ p ] );
                apParams[ p - 1 ] ->SetMatrix( m, __iGrid );
            }
            ; break;
        default:
            // Unhandled TYpe
            break;
        };
    };

    END_VARYING_SECTION

    // Free up the storage allocated for the return type
    switch ( Result->Type() )
    {

    case type_float:
        delete ( float* ) dso_argv[ 0 ]; break;
    case type_point:
    case type_triple:  // This seems reasonable
    case type_vector:
    case type_normal:
    case type_color:
    case type_hpoint:
        delete[] ( float* ) dso_argv[ 0 ]; break;
    case type_string:  // Need to look into these
        delete ( STRING_DESC* ) dso_argv[ 0 ]; break;
    case type_matrix:
    case type_sixteentuple:
        delete[] ( float* ) dso_argv[ 0 ]; break;
    default:
        // Unhandled TYpe
        break;
    };

    // Free up the storage allocated for the args
    for ( p = 1;p <= cParams;p++ )
    {
        switch ( apParams[ p - 1 ] ->Type() )
        {
        case type_float:
            delete ( float* ) dso_argv[ p ]; break;
        case type_point:
        case type_triple:
        case type_vector:
        case type_normal:
        case type_color:
        case type_hpoint:
            delete[] ( float* ) dso_argv[ p ]; break;
        case type_string:
            delete ( STRING_DESC* ) dso_argv[ p ]; break;
        case type_matrix:
        case type_sixteentuple:
            delete[] ( float* ) dso_argv[ p ]; break;
        default:
            // Unhandled TYpe
            break;
        };
    };

    delete dso_argv;
}

//----------------------------------------------------------------------
// occlusion(occlmap,P,N,samples)
STD_SOIMPL CqShaderExecEnv::SO_occlusion( STRINGVAL occlmap, FLOATVAL channel, POINTVAL P, NORMALVAL N, FLOATVAL samples, DEFPARAMVARIMPL )
{
    STATS_INC( SHD_so_occlusion );

    INIT_SO

    if ( NULL == QGetRenderContextI() )
        return ;

    GET_TEXTURE_PARAMS;

    BEGIN_UNIFORM_SECTION
    GETSTRING( occlmap );
    GETNORMAL( N );
    GETFLOAT( samples );
    IqTextureMap* pMap = QGetRenderContextI() ->GetShadowMap( STRING( occlmap ) );
    END_UNIFORM_SECTION

    CqVector3D L(0,0,-1);

    __fVarying = TqTrue;
    if ( pMap != 0 && pMap->IsValid() )
    {
        std::valarray<TqFloat> fv;
        pMap->PrepareSampleOptions( paramMap );

        BEGIN_VARYING_SECTION
        // Storage for the final combined occlusion value.
        TqFloat occlsum = 0.0f;
        TqFloat dotsum = 0.0f;

        CqVector3D swidth = 0.0f, twidth = 0.0f;

        swidth = SO_DerivType<CqVector3D>( P, NULL, __iGrid, this );
        twidth = SO_DerivType<CqVector3D>( P, NULL, __iGrid, this );

        GETPOINT( P );
        GETNORMAL( N );
        TqInt i = pMap->NumPages() - 1;
        for( ; i >= 0; i-- )
        {
            // Check if the lightsource is behind the sample.
            CqVector3D Nl = pMap->GetMatrix(2, i) * NORMAL( N );
            TqFloat cosangle = Nl * L;
            if( cosangle < 0.0f )
                continue;

            fv = 0.0f;
            pMap->SampleMap( POINT( P ), swidth, twidth, fv, i );
            occlsum += cosangle * fv[0];
            dotsum += cosangle;
        }
        occlsum /= dotsum;
        SETFLOAT( Result, occlsum);
        END_VARYING_SECTION
    }
    else
    {
        BEGIN_VARYING_SECTION
        SETFLOAT( Result, 0.0f );	// Default, completely lit
        END_VARYING_SECTION
    }
}


END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------
