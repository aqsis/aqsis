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

#include <boost/regex.hpp>

namespace Aqsis {

//----------------------------------------------------------------------
// SO_sprintf
// Helper function to process a string inserting variable, used in printf and format.

static	CqString	SO_sprintf( const char* str, int cParams, IqShaderData** apParams, int varyingindex )
{
	CqString strRes( "" );
	CqString strTrans = str;

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
// match(pattern, str)
void CqShaderExecEnv::SO_match( IqShaderData* patternData, IqShaderData* strData,
                                IqShaderData* resultData, IqShader* pShader )
{
	// Grab pointers to the underlying data.
	const CqString* patPtr = 0;
	const CqString* strPtr = 0;
	patternData->GetStringPtr(patPtr);
	strData->GetStringPtr(strPtr);
	TqFloat* resPtr = 0;
	resultData->GetFloatPtr(resPtr);

	// Running state information
	TqInt simdCount = shadingPointCount();
	const CqBitVector& rs = RunningState();

	bool patVarying = patternData->Size() > 1;
	bool strVarying = strData->Size() > 1;

	// The RISpec says that
	//
	//   /pattern/ can be any regular expression as described in the POSIX
	//   manual page on regex()(3X).
	//
	// This appears to indicate that 'basic' POSIX.2 spec regexs should be
	// used corresponding to
	//
	//   synType = boost::regex::basic
	//
	// Unfortunately basic regexes are a bit crippled: they don't include \|
	// for alternation.  Also \+ for one or more matches and \? for zero or one
	// matches are handy and commonly used, so they're also enabled here.
	boost::regex_constants::syntax_option_type synType
		= boost::regex::basic | boost::regex::bk_vbar | boost::regex::bk_plus_qm; 

	if(patVarying)
	{
		// Why would anyone have a varying pattern string?  I suppose we'd
		// better support it for robustness...
		if(strVarying)
		{
			for(TqInt i = 0; i < simdCount; ++i)
			{
				if(rs.Value(i))
				{
					boost::regex pattern(*patPtr, synType);
					*resPtr = boost::regex_search(*strPtr, pattern);
				}
				++strPtr;
				++patPtr;
				++resPtr;
			}
		}
		else
		{
			for(TqInt i = 0; i < simdCount; ++i)
			{
				if(rs.Value(i))
				{
					boost::regex pattern(*patPtr, synType);
					*resPtr = boost::regex_search(*strPtr, pattern);
				}
				++patPtr;
				++resPtr;
			}
		}
	}
	else
	{
		// Non-varying pattern; probably the only sane option...
		boost::regex pattern(*patPtr, synType);
		if(strVarying)
		{
			for(TqInt i = 0; i < simdCount; ++i)
			{
				if(rs.Value(i))
					*resPtr = boost::regex_search(*strPtr, pattern);
				++strPtr;
				++resPtr;
			}
		}
		else
		{
			TqFloat res = boost::regex_search(*strPtr, pattern);
			// Use SetFloat here in case result is varying.
			resultData->SetFloat(res);
		}
	}
}


} // namespace Aqsis
//---------------------------------------------------------------------
