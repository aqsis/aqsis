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
		\brief Implements the basic shader operations. (printf, SO_external related)
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	"shaderexecenv.h"

namespace Aqsis {

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
								strVal.Format( "%f,%f,%f", col.r(), col.g(), col.b() );
								strRes += strVal;
							}
							break;

							case 'm':
							{
								CqMatrix mat;
								apParams[ ivar++ ] ->GetMatrix( mat, varyingindex );
								CqString strVal;
								strVal.Format( "[%f,%f,%f,%f,  %f,%f,%f,%f,  %f,%f,%f,%f,  %f,%f,%f,%f]",
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
// printf(s,...)

void	CqShaderExecEnv::SO_printf( IqShaderData* str, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	if ( !getRenderContext() )
		return ;

	__fVarying=(str)->Class()==class_varying;
	TqInt ii;
	for ( ii = 0; ii < cParams; ii++ )
	{
		__fVarying=(apParams[ ii ])->Class()==class_varying||__fVarying;
	}

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqString _aq_str;
			(str)->GetString(_aq_str,__iGrid);
			CqString strA = SO_sprintf( _aq_str.c_str(), cParams, apParams, __iGrid );
			getRenderContext() ->PrintString( strA.c_str() );
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


//----------------------------------------------------------------------
// format(s,...)

void	CqShaderExecEnv::SO_format( IqShaderData* str, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(str)->Class()==class_varying;
	int ii;
	for ( ii = 0; ii < cParams; ii++ )
	{
		__fVarying=(apParams[ ii ])->Class()==class_varying||__fVarying;
	}
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqString _aq_str;
			(str)->GetString(_aq_str,__iGrid);
			CqString strA = SO_sprintf( _aq_str.c_str(), cParams, apParams, __iGrid );
			(Result)->SetString(strA,__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}


//----------------------------------------------------------------------
// concat(s,s,...)

void	CqShaderExecEnv::SO_concat( IqShaderData* stra, IqShaderData* strb, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(stra)->Class()==class_varying;
	__fVarying=(strb)->Class()==class_varying||__fVarying;
	int ii;
	for ( ii = 0; ii < cParams; ii++ )
	{
		__fVarying=(apParams[ ii ])->Class()==class_varying||__fVarying;
	}
	__fVarying=(Result)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			CqString _aq_stra;
			(stra)->GetString(_aq_stra,__iGrid);
			CqString strRes = _aq_stra;
			CqString _aq_strb;
			(strb)->GetString(_aq_strb,__iGrid);
			strRes += _aq_strb;
			for ( ii = 0; ii < cParams; ii++ )
			{
				CqString sn;
				apParams[ ii ] ->GetString( sn, __iGrid );
				strRes += sn;
			}
			(Result)->SetString(strRes,__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
static TqFloat match(const char *string, const char *pattern)
{
#if defined(REGEXP)
	int status;
	regex_t re;
	if (regcomp(&re, pattern, REG_EXTENDED|REG_NOSUB) != 0)
	{
		return(0.0f);      /* report error */
	}
	status = regexec(&re, string, (size_t) 0, NULL, 0);
	regfree(&re);

	if (status != 0)
	{
		return(0.0f);      /* report error */
	}
	return(1.0f);
#else

	return (TqFloat) (strstr(string, pattern) != 0);
#endif
}

// match(pattern, str)
void	CqShaderExecEnv::SO_match( IqShaderData* a, IqShaderData* b, IqShaderData* Result, IqShader* pShader )
{
	// TODO: Do this properly.
	TqUint __iGrid;

	__iGrid = 0;
	float r = 0.0f;
	CqString _aq_a;
	(a)->GetString(_aq_a,__iGrid);
	CqString _aq_b;
	(b)->GetString(_aq_b,__iGrid);
	if ( _aq_a.size() == 0 )
		r = 0.0f;
	else if ( _aq_b.size() == 0 )
		r = 0.0f;
	else
	{
		// Check the simple case first where both strings are identical
		TqUlong hasha = CqString::hash(_aq_a.c_str());
		TqUlong hashb = CqString::hash(_aq_b.c_str());

		if (hasha == hashb)
		{
			r = 1.0f;
		}
		else
		{
			/*
			* Match string b into a
			*/
			r = match(_aq_a.c_str(), _aq_b.c_str());
		}


	}

	(Result)->SetFloat(r,__iGrid);

}


} // namespace Aqsis
//---------------------------------------------------------------------
