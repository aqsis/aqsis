// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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
		\brief Implements the basic shader operations. (Texture, shadow, env, bake, occlusion related)
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	"aqsis.h"

#include	<math.h>
#include	<map>
#include	<vector>
#include	<string>
#include	<stdio.h>

#include	"shaderexecenv.h"
#include	"shadervm.h"
#include	"irenderer.h"
#include	"itexturemap.h"
#include	"version.h"
#include	"logging.h"

START_NAMESPACE(    Aqsis )


//----------------------------------------------------------------------
// texture(S)
void CqShaderExecEnv::SO_ftexture1( IqShaderData* name, IqShaderData* channel, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqFloat Deffloat = 0.0f;
	bool __fVarying;
	TqUint __iGrid;

	if ( !getRenderContext() )
		return ;

	std::map<std::string, IqShaderData*> paramMap;
	GetTexParams(cParams, apParams, paramMap);

	TqFloat fill = 0.0f;
	if ( paramMap.find( "fill" ) != paramMap.end() )
		paramMap[ "fill" ] ->GetFloat( fill );

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	TqFloat _aq_channel;
	(channel)->GetFloat(_aq_channel,__iGrid);
	IqTextureMap* pTMap = getRenderContext() ->GetTextureMap( _aq_name );
	TqFloat fdu = 0.0f, fdv = 0.0f;
	if ( m_pAttributes )
	{
		du() ->GetFloat( fdu );
		dv() ->GetFloat( fdv );
	}



	__fVarying = true;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		std::valarray<TqFloat> val;
		pTMap->PrepareSampleOptions( paramMap );

		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
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
				TqFloat fchan = _aq_channel;
				if ( fchan >= val.size() )
					(Result)->SetFloat(fill,__iGrid);
				else
					(Result)->SetFloat(val[ static_cast<unsigned int>( fchan ) ],__iGrid);
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				(Result)->SetFloat(0.0f,__iGrid);
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
}

//----------------------------------------------------------------------
// texture(S,F,F)
void CqShaderExecEnv::SO_ftexture2( IqShaderData* name, IqShaderData* channel, IqShaderData* s, IqShaderData* t, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqFloat Deffloat = 0.0f;
	bool __fVarying;
	TqUint __iGrid;

	if ( !getRenderContext() )
		return ;

	std::map<std::string, IqShaderData*> paramMap;
	GetTexParams(cParams, apParams, paramMap);

	TqFloat fill = 0.0f;
	if ( paramMap.find( "fill" ) != paramMap.end() )
		paramMap[ "fill" ] ->GetFloat( fill );

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	TqFloat _aq_channel;
	(channel)->GetFloat(_aq_channel,__iGrid);
	IqTextureMap* pTMap = getRenderContext() ->GetTextureMap( _aq_name );
	TqFloat fdu = 0.0f, fdv = 0.0f;
	if ( m_pAttributes )
	{
		du() ->GetFloat( fdu );
		dv() ->GetFloat( fdv );
	}



	__fVarying = true;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		std::valarray<TqFloat> val;
		pTMap->PrepareSampleOptions( paramMap );

		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
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
				TqFloat _aq_s;
				(s)->GetFloat(_aq_s,__iGrid);
				TqFloat _aq_t;
				(t)->GetFloat(_aq_t,__iGrid);
				pTMap->SampleMap( _aq_s, _aq_t, swidth, twidth, val );

				// Grab the appropriate channel.
				TqFloat fchan = _aq_channel;
				if ( fchan >= val.size() )
					(Result)->SetFloat(fill,__iGrid);
				else
					(Result)->SetFloat(val[ static_cast<unsigned int>( fchan ) ],__iGrid);
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				(Result)->SetFloat(0.0f,__iGrid);	// Default, completely lit
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
}

//----------------------------------------------------------------------
// texture(S,F,F,F,F,F,F,F,F)
void CqShaderExecEnv::SO_ftexture3( IqShaderData* name, IqShaderData* channel, IqShaderData* s1, IqShaderData* t1, IqShaderData* s2, IqShaderData* t2, IqShaderData* s3, IqShaderData* t3, IqShaderData* s4, IqShaderData* t4, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	if ( !getRenderContext() )
		return ;

	std::map<std::string, IqShaderData*> paramMap;
	GetTexParams(cParams, apParams, paramMap);

	TqFloat fill = 0.0f;
	if ( paramMap.find( "fill" ) != paramMap.end() )
		paramMap[ "fill" ] ->GetFloat( fill );

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	TqFloat _aq_channel;
	(channel)->GetFloat(_aq_channel,__iGrid);
	IqTextureMap* pTMap = getRenderContext() ->GetTextureMap( _aq_name );



	__fVarying = true;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		std::valarray<TqFloat> val;
		pTMap->PrepareSampleOptions( paramMap );

		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{

				// Sample the texture.
				TqFloat _aq_s1;
				(s1)->GetFloat(_aq_s1,__iGrid);
				TqFloat _aq_t1;
				(t1)->GetFloat(_aq_t1,__iGrid);
				TqFloat _aq_s2;
				(s2)->GetFloat(_aq_s2,__iGrid);
				TqFloat _aq_t2;
				(t2)->GetFloat(_aq_t2,__iGrid);
				TqFloat _aq_s3;
				(s3)->GetFloat(_aq_s3,__iGrid);
				TqFloat _aq_t3;
				(t3)->GetFloat(_aq_t3,__iGrid);
				TqFloat _aq_s4;
				(s4)->GetFloat(_aq_s4,__iGrid);
				TqFloat _aq_t4;
				(t4)->GetFloat(_aq_t4,__iGrid);
				pTMap->SampleMap( _aq_s1, _aq_t1, _aq_s2, _aq_t2, _aq_s3, _aq_t3, _aq_s4, _aq_t4, val );

				// Grab the appropriate channel.
				TqFloat fchan = _aq_channel;
				if ( fchan >= val.size() )
					(Result)->SetFloat(fill,__iGrid);
				else
					(Result)->SetFloat(val[ static_cast<unsigned int>( fchan ) ],__iGrid);
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				(Result)->SetFloat(0.0f,__iGrid);	// Default, completely lit
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
}

//----------------------------------------------------------------------
// texture(S)
void CqShaderExecEnv::SO_ctexture1( IqShaderData* name, IqShaderData* channel, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqFloat Deffloat = 0.0f;
	bool __fVarying;
	TqUint __iGrid;

	if ( !getRenderContext() )
		return ;

	std::map<std::string, IqShaderData*> paramMap;
	GetTexParams(cParams, apParams, paramMap);

	TqFloat fill = 0.0f;
	if ( paramMap.find( "fill" ) != paramMap.end() )
		paramMap[ "fill" ] ->GetFloat( fill );

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	TqFloat _aq_channel;
	(channel)->GetFloat(_aq_channel,__iGrid);
	IqTextureMap* pTMap = getRenderContext() ->GetTextureMap( _aq_name );
	TqFloat fdu = 0.0f, fdv = 0.0f;
	if ( m_pAttributes )
	{
		du() ->GetFloat( fdu );
		dv() ->GetFloat( fdv );
	}



	__fVarying = true;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		std::valarray<TqFloat> val;
		pTMap->PrepareSampleOptions( paramMap );

		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
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
				TqFloat fchan = _aq_channel;
				CqColor colResult;
				colResult.SetfRed( (fchan >= val.size())? fill : val[ static_cast<unsigned int>( fchan ) ] );
				colResult.SetfGreen( ((fchan + 1) >= val.size())? fill : val[ static_cast<unsigned int>( fchan+1 ) ] );
				colResult.SetfBlue( ((fchan + 2) >= val.size())? fill : val[ static_cast<unsigned int>( fchan+2 ) ] );

				(Result)->SetColor(colResult,__iGrid);
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				(Result)->SetColor(CqColor( 0, 0, 0 ),__iGrid);	// Default, no color
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
}

//----------------------------------------------------------------------
// texture(S,F,F)
void CqShaderExecEnv::SO_ctexture2( IqShaderData* name, IqShaderData* channel, IqShaderData* s, IqShaderData* t, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqFloat Deffloat = 0.0f;
	bool __fVarying;
	TqUint __iGrid;

	if ( !getRenderContext() )
		return ;

	std::map<std::string, IqShaderData*> paramMap;
	GetTexParams(cParams, apParams, paramMap);

	TqFloat fill = 0.0f;
	if ( paramMap.find( "fill" ) != paramMap.end() )
		paramMap[ "fill" ] ->GetFloat( fill );

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	TqFloat _aq_channel;
	(channel)->GetFloat(_aq_channel,__iGrid);
	IqTextureMap* pTMap = getRenderContext() ->GetTextureMap( _aq_name );
	TqFloat fdu = 0.0f, fdv = 0.0f;
	if ( m_pAttributes )
	{
		du() ->GetFloat( fdu );
		dv() ->GetFloat( fdv );
	}



	__fVarying = true;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		std::valarray<TqFloat> val;
		pTMap->PrepareSampleOptions( paramMap );

		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
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
				TqFloat _aq_s;
				(s)->GetFloat(_aq_s,__iGrid);
				TqFloat _aq_t;
				(t)->GetFloat(_aq_t,__iGrid);
				pTMap->SampleMap( _aq_s, _aq_t, swidth, twidth, val );

				// Grab the appropriate channel.
				TqFloat fchan = _aq_channel;
				CqColor colResult;
				colResult.SetfRed( (fchan >= val.size())? fill : val[ static_cast<unsigned int>( fchan ) ] );
				colResult.SetfGreen( ((fchan + 1) >= val.size())? fill : val[ static_cast<unsigned int>( fchan+1 ) ] );
				colResult.SetfBlue( ((fchan + 2) >= val.size())? fill : val[ static_cast<unsigned int>( fchan+2 ) ] );

				(Result)->SetColor(colResult,__iGrid);
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				(Result)->SetColor(CqColor( 0, 0, 0 ),__iGrid);	// Default, completely lit
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
}

//----------------------------------------------------------------------
// texture(S,F,F,F,F,F,F,F,F)
void CqShaderExecEnv::SO_ctexture3( IqShaderData* name, IqShaderData* channel, IqShaderData* s1, IqShaderData* t1, IqShaderData* s2, IqShaderData* t2, IqShaderData* s3, IqShaderData* t3, IqShaderData* s4, IqShaderData* t4, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	if ( !getRenderContext() )
		return ;

	std::map<std::string, IqShaderData*> paramMap;
	GetTexParams(cParams, apParams, paramMap);

	TqFloat fill = 0.0f;
	if ( paramMap.find( "fill" ) != paramMap.end() )
		paramMap[ "fill" ] ->GetFloat( fill );

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	TqFloat _aq_channel;
	(channel)->GetFloat(_aq_channel,__iGrid);
	IqTextureMap* pTMap = getRenderContext() ->GetTextureMap( _aq_name );



	__fVarying = true;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		std::valarray<TqFloat> val;
		pTMap->PrepareSampleOptions( paramMap );

		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				// Sample the texture.
				TqFloat _aq_s1;
				(s1)->GetFloat(_aq_s1,__iGrid);
				TqFloat _aq_t1;
				(t1)->GetFloat(_aq_t1,__iGrid);
				TqFloat _aq_s2;
				(s2)->GetFloat(_aq_s2,__iGrid);
				TqFloat _aq_t2;
				(t2)->GetFloat(_aq_t2,__iGrid);
				TqFloat _aq_s3;
				(s3)->GetFloat(_aq_s3,__iGrid);
				TqFloat _aq_t3;
				(t3)->GetFloat(_aq_t3,__iGrid);
				TqFloat _aq_s4;
				(s4)->GetFloat(_aq_s4,__iGrid);
				TqFloat _aq_t4;
				(t4)->GetFloat(_aq_t4,__iGrid);
				pTMap->SampleMap( _aq_s1, _aq_t1, _aq_s2, _aq_t2, _aq_s3, _aq_t3, _aq_s4, _aq_t4, val );

				// Grab the appropriate channel.
				TqFloat fchan = _aq_channel;
				CqColor colResult;
				colResult.SetfRed( (fchan >= val.size())? fill : val[ static_cast<unsigned int>( fchan ) ] );
				colResult.SetfGreen( ((fchan + 1) >= val.size())? fill : val[ static_cast<unsigned int>( fchan+1 ) ] );
				colResult.SetfBlue( ((fchan + 2) >= val.size())? fill : val[ static_cast<unsigned int>( fchan+2 ) ] );

				(Result)->SetColor(colResult,__iGrid);
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				(Result)->SetColor(CqColor( 0, 0, 0 ),__iGrid);	// Default, completely lit
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
}


//----------------------------------------------------------------------
// environment(S,P)
void CqShaderExecEnv::SO_fenvironment2( IqShaderData* name, IqShaderData* channel, IqShaderData* R, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	CqVector3D Defvec( 0.0f, 0.0f, 0.0f );
	bool __fVarying;
	TqUint __iGrid;

	if ( !getRenderContext() )
		return ;

	std::map<std::string, IqShaderData*> paramMap;
	GetTexParams(cParams, apParams, paramMap);

	TqFloat fill = 0.0f;
	if ( paramMap.find( "fill" ) != paramMap.end() )
		paramMap[ "fill" ] ->GetFloat( fill );

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	TqFloat _aq_channel;
	(channel)->GetFloat(_aq_channel,__iGrid);
	IqTextureMap* pTMap = getRenderContext() ->GetEnvironmentMap( _aq_name );

	// Try with LatLong map file
	if ( pTMap == 0 )
	{
		pTMap = getRenderContext() ->GetLatLongMap( _aq_name );
	}
	TqFloat fdu = 0.0f, fdv = 0.0f;
	if ( m_pAttributes )
	{
		du() ->GetFloat( fdu );
		dv() ->GetFloat( fdv );
	}


	__fVarying = true;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		pTMap->PrepareSampleOptions( paramMap );
		std::valarray<TqFloat> val;

		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
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
				CqVector3D _aq_R;
				(R)->GetVector(_aq_R,__iGrid);
				pTMap->SampleMap( _aq_R, swidth, twidth, val );

				// Grab the appropriate channel.
				TqFloat fchan = _aq_channel;
				if ( fchan >= val.size() )
					(Result)->SetFloat(fill,__iGrid);
				else
					(Result)->SetFloat(val[ static_cast<unsigned int>( fchan ) ],__iGrid);
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				(Result)->SetFloat(0.0f,__iGrid);	// Default, completely lit
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
}

//----------------------------------------------------------------------
// environment(S,P,P,P,P)
void CqShaderExecEnv::SO_fenvironment3( IqShaderData* name, IqShaderData* channel, IqShaderData* R1, IqShaderData* R2, IqShaderData* R3, IqShaderData* R4, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	if ( !getRenderContext() )
		return ;

	std::map<std::string, IqShaderData*> paramMap;
	GetTexParams(cParams, apParams, paramMap);

	TqFloat fill = 0.0f;
	if ( paramMap.find( "fill" ) != paramMap.end() )
		paramMap[ "fill" ] ->GetFloat( fill );

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	TqFloat _aq_channel;
	(channel)->GetFloat(_aq_channel,__iGrid);
	IqTextureMap* pTMap = getRenderContext() ->GetEnvironmentMap( _aq_name );
	// Try with LatLong map file
	if ( pTMap == 0 )
	{
		pTMap = getRenderContext() ->GetLatLongMap( _aq_name );
	}


	__fVarying = true;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		std::valarray<TqFloat> val;
		pTMap->PrepareSampleOptions( paramMap );

		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				// Sample the texture.
				CqVector3D _aq_R1;
				(R1)->GetVector(_aq_R1,__iGrid);
				CqVector3D _aq_R2;
				(R2)->GetVector(_aq_R2,__iGrid);
				CqVector3D _aq_R3;
				(R3)->GetVector(_aq_R3,__iGrid);
				CqVector3D _aq_R4;
				(R4)->GetVector(_aq_R4,__iGrid);
				pTMap->SampleMap( _aq_R1, _aq_R2, _aq_R3, _aq_R4, val );

				// Grab the appropriate channel.
				TqFloat fchan = _aq_channel;
				if ( fchan >= val.size() )
					(Result)->SetFloat(fill,__iGrid);
				else
					(Result)->SetFloat(val[ static_cast<unsigned int>( fchan ) ],__iGrid);
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				(Result)->SetFloat(0.0f,__iGrid);	// Default, completely lit
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
}


//----------------------------------------------------------------------
// environment(S,P)
void CqShaderExecEnv::SO_cenvironment2( IqShaderData* name, IqShaderData* channel, IqShaderData* R, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	CqVector3D Defvec( 0.0f, 0.0f, 0.0f );
	bool __fVarying;
	TqUint __iGrid;

	if ( !getRenderContext() )
		return ;

	std::map<std::string, IqShaderData*> paramMap;
	GetTexParams(cParams, apParams, paramMap);

	TqFloat fill = 0.0f;
	if ( paramMap.find( "fill" ) != paramMap.end() )
		paramMap[ "fill" ] ->GetFloat( fill );

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	TqFloat _aq_channel;
	(channel)->GetFloat(_aq_channel,__iGrid);
	IqTextureMap* pTMap = getRenderContext() ->GetEnvironmentMap( _aq_name );
	// Try with LatLong map file
	if ( pTMap == 0 )
	{
		pTMap = getRenderContext() ->GetLatLongMap( _aq_name );
	}
	TqFloat fdu = 0.0f, fdv = 0.0f;
	if ( m_pAttributes )
	{
		du() ->GetFloat( fdu );
		dv() ->GetFloat( fdv );
	}


	__fVarying = true;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		std::valarray<TqFloat> val;
		pTMap->PrepareSampleOptions( paramMap );

		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
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
				CqVector3D _aq_R;
				(R)->GetVector(_aq_R,__iGrid);
				pTMap->SampleMap( _aq_R, swidth, twidth, val );


				// Grab the appropriate channel.
				TqFloat fchan = _aq_channel;
				if ( fchan + 2 >= val.size() )
					(Result)->SetColor(CqColor( fill, fill, fill ),__iGrid);
				else
					(Result)->SetColor(CqColor( val[ static_cast<unsigned int>( fchan ) ], val[ static_cast<unsigned int>( fchan ) + 1 ], val[ static_cast<unsigned int>( fchan ) + 2 ] ),__iGrid);
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				(Result)->SetColor(CqColor( 0.0f, 0.0f, 0.0f ),__iGrid);	// Default, completely lit
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
}

//----------------------------------------------------------------------
// environment(S,P,P,P,P)
void CqShaderExecEnv::SO_cenvironment3( IqShaderData* name, IqShaderData* channel, IqShaderData* R1, IqShaderData* R2, IqShaderData* R3, IqShaderData* R4, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	if ( !getRenderContext() )
		return ;

	std::map<std::string, IqShaderData*> paramMap;
	GetTexParams(cParams, apParams, paramMap);

	TqFloat fill = 0.0f;
	if ( paramMap.find( "fill" ) != paramMap.end() )
		paramMap[ "fill" ] ->GetFloat( fill );

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	TqFloat _aq_channel;
	(channel)->GetFloat(_aq_channel,__iGrid);
	IqTextureMap* pTMap = getRenderContext() ->GetEnvironmentMap( _aq_name );
	// Try with LatLong map file
	if ( pTMap == 0 )
	{
		pTMap = getRenderContext() ->GetLatLongMap( _aq_name );
	}
	__iGrid = 0;

	__fVarying = true;
	if ( pTMap != 0 && pTMap->IsValid() )
	{
		std::valarray<TqFloat> val;
		pTMap->PrepareSampleOptions( paramMap );

		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				// Sample the texture.
				CqVector3D _aq_R1;
				(R1)->GetVector(_aq_R1,__iGrid);
				CqVector3D _aq_R2;
				(R2)->GetVector(_aq_R2,__iGrid);
				CqVector3D _aq_R3;
				(R3)->GetVector(_aq_R3,__iGrid);
				CqVector3D _aq_R4;
				(R4)->GetVector(_aq_R4,__iGrid);
				pTMap->SampleMap( _aq_R1, _aq_R2, _aq_R3, _aq_R4, val );

				// Grab the appropriate channel.
				TqFloat fchan = _aq_channel;
				if ( fchan + 2 >= val.size() )
					(Result)->SetColor(CqColor( fill, fill, fill ),__iGrid);
				else
					(Result)->SetColor(CqColor( val[ static_cast<unsigned int>( fchan ) ], val[ static_cast<unsigned int>( fchan ) + 1 ], val[ static_cast<unsigned int>( fchan ) + 2 ] ),__iGrid);
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				(Result)->SetColor(CqColor( 0.0f, 0.0f, 0.0f ),__iGrid);	// Default, completely lit
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
}

//----------------------------------------------------------------------
// bump(S)
void CqShaderExecEnv::SO_bump1( IqShaderData* name, IqShaderData* channel, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying = true;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			(Result)->SetPoint(CqVector3D( 0, 0, 0 ),__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// bump(S,F,F)
void CqShaderExecEnv::SO_bump2( IqShaderData* name, IqShaderData* channel, IqShaderData* s, IqShaderData* t, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying = true;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			(Result)->SetPoint(CqVector3D( 0, 0, 0 ),__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// bump(S,F,F,F,F,F,F,F,F)
void CqShaderExecEnv::SO_bump3( IqShaderData* name, IqShaderData* channel, IqShaderData* s1, IqShaderData* t1, IqShaderData* s2, IqShaderData* t2, IqShaderData* s3, IqShaderData* t3, IqShaderData* s4, IqShaderData* t4, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying = true;

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			(Result)->SetPoint(CqVector3D( 0, 0, 0 ),__iGrid);
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
}

//----------------------------------------------------------------------
// shadow(S,P)
void CqShaderExecEnv::SO_shadow( IqShaderData* name, IqShaderData* channel, IqShaderData* P, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	if ( !getRenderContext() )
		return ;

	std::map<std::string, IqShaderData*> paramMap;
	GetTexParams(cParams, apParams, paramMap);

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	TqFloat _aq_channel;
	(channel)->GetFloat(_aq_channel,__iGrid);
	IqTextureMap* pMap = getRenderContext() ->GetShadowMap( _aq_name );


	__fVarying = true;
	if ( pMap != 0 && pMap->IsValid() )
	{
		std::valarray<TqFloat> fv;
		pMap->PrepareSampleOptions( paramMap );

		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{

			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqVector3D swidth = 0.0f, twidth = 0.0f;

				CqVector3D _aq_P;
				(P)->GetPoint(_aq_P,__iGrid);

				pMap->SampleMap( _aq_P, swidth, twidth, fv, 0 );
				(Result)->SetFloat(fv[ 0 ],__iGrid);
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				(Result)->SetFloat(0.0f,__iGrid);	// Default, completely lit
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
}

//----------------------------------------------------------------------
// shadow(S,P,P,P,P)

void CqShaderExecEnv::SO_shadow1( IqShaderData* name, IqShaderData* channel, IqShaderData* P1, IqShaderData* P2, IqShaderData* P3, IqShaderData* P4, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	if ( !getRenderContext() )
		return ;

	std::map<std::string, IqShaderData*> paramMap;
	GetTexParams(cParams, apParams, paramMap);

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	TqFloat _aq_channel;
	(channel)->GetFloat(_aq_channel,__iGrid);
	IqTextureMap* pMap = getRenderContext() ->GetShadowMap( _aq_name );


	__fVarying = true;
	if ( pMap != 0 && pMap->IsValid() )
	{
		std::valarray<TqFloat> fv;
		pMap->PrepareSampleOptions( paramMap );

		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				CqVector3D _aq_P1;
				(P1)->GetPoint(_aq_P1,__iGrid);
				CqVector3D _aq_P2;
				(P2)->GetPoint(_aq_P2,__iGrid);
				CqVector3D _aq_P3;
				(P3)->GetPoint(_aq_P3,__iGrid);
				CqVector3D _aq_P4;
				(P4)->GetPoint(_aq_P4,__iGrid);
				pMap->SampleMap( _aq_P1, _aq_P2, _aq_P3, _aq_P4, fv, 0 );
				(Result)->SetFloat(fv[ 0 ],__iGrid);
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				(Result)->SetFloat(0.0f,__iGrid);	// Default, completely lit
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
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



void CqShaderExecEnv::SO_bake_f( IqShaderData* name, IqShaderData* s, IqShaderData* t, IqShaderData* f, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(f)->Class()==class_varying;
	__fVarying=(s)->Class()==class_varying||__fVarying;
	__fVarying=(t)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	BakingData *bd = bake_init(  /*STRING( name).c_str() */ );


	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_s;
			(s)->GetFloat(_aq_s,__iGrid);
			TqFloat _aq_t;
			(t)->GetFloat(_aq_t,__iGrid);
			TqFloat _aq_f;
			(f)->GetFloat(_aq_f,__iGrid);
			bake_f( bd, ( char * ) _aq_name.c_str(), _aq_s, _aq_t, _aq_f );
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);

	__iGrid = 0;
	bake_done( bd );

}

void CqShaderExecEnv::SO_bake_3c( IqShaderData* name, IqShaderData* s, IqShaderData* t, IqShaderData* f, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(f)->Class()==class_varying;
	__fVarying=(s)->Class()==class_varying||__fVarying;
	__fVarying=(t)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	TqFloat rgb[ 3 ];

	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	BakingData *bd = bake_init(  /*(char *) STRING( name ).c_str()*/ );

	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_s;
			(s)->GetFloat(_aq_s,__iGrid);
			TqFloat _aq_t;
			(t)->GetFloat(_aq_t,__iGrid);
			CqColor _aq_f;
			(f)->GetColor(_aq_f,__iGrid);
			_aq_f.GetColorRGB( &rgb[ 0 ], &rgb[ 1 ], &rgb[ 2 ] );
			bake_3( bd, ( char * ) _aq_name.c_str(), _aq_s, _aq_t, rgb );
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	__iGrid = 0;
	bake_done( bd );

}

void CqShaderExecEnv::SO_bake_3n( IqShaderData* name, IqShaderData* s, IqShaderData* t, IqShaderData* f, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(f)->Class()==class_varying;
	__fVarying=(s)->Class()==class_varying||__fVarying;
	__fVarying=(t)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	BakingData *bd = bake_init(  /*(char *) STRING( name ).c_str() */ );


	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_s;
			(s)->GetFloat(_aq_s,__iGrid);
			TqFloat _aq_t;
			(t)->GetFloat(_aq_t,__iGrid);
			CqVector3D _aq_f;
			(f)->GetNormal(_aq_f,__iGrid);
			TqFloat rgb[ 3 ];
			rgb[ 0 ] = _aq_f [ 0 ];
			rgb[ 1 ] = _aq_f [ 1 ];
			rgb[ 2 ] = _aq_f [ 2 ];
			bake_3( bd, ( char * ) _aq_name.c_str(), _aq_s, _aq_t, rgb );
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);

	__iGrid = 0;
	bake_done( bd );

}

void CqShaderExecEnv::SO_bake_3p( IqShaderData* name, IqShaderData* s, IqShaderData* t, IqShaderData* f, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying=(f)->Class()==class_varying;
	__fVarying=(s)->Class()==class_varying||__fVarying;
	__fVarying=(t)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	BakingData *bd = bake_init(  /*(char *) STRING( name ).c_str()  */ );


	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_s;
			(s)->GetFloat(_aq_s,__iGrid);
			TqFloat _aq_t;
			(t)->GetFloat(_aq_t,__iGrid);
			CqVector3D _aq_f;
			(f)->GetPoint(_aq_f,__iGrid);
			TqFloat rgb[ 3 ];
			rgb[ 0 ] = _aq_f [ 0 ];
			rgb[ 1 ] = _aq_f [ 1 ];
			rgb[ 2 ] = _aq_f [ 2 ];
			bake_3( bd, ( char * ) _aq_name.c_str(), _aq_s, _aq_t, rgb );
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);

	__iGrid = 0;
	bake_done( bd );

}

void CqShaderExecEnv::SO_bake_3v( IqShaderData* name, IqShaderData* s, IqShaderData* t, IqShaderData* f, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;
	__fVarying=(f)->Class()==class_varying;
	__fVarying=(s)->Class()==class_varying||__fVarying;
	__fVarying=(t)->Class()==class_varying||__fVarying;

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	BakingData *bd = bake_init(  /*(char *) STRING( name ).c_str()  */ );


	__iGrid = 0;
	CqBitVector& RS = RunningState();
	do
	{
		if(!__fVarying || RS.Value( __iGrid ) )
		{
			TqFloat _aq_s;
			(s)->GetFloat(_aq_s,__iGrid);
			TqFloat _aq_t;
			(t)->GetFloat(_aq_t,__iGrid);
			CqVector3D _aq_f;
			(f)->GetVector(_aq_f,__iGrid);
			TqFloat rgb[ 3 ];
			rgb[ 0 ] = _aq_f [ 0 ];
			rgb[ 1 ] = _aq_f [ 1 ];
			rgb[ 2 ] = _aq_f [ 2 ];
			bake_3( bd, ( char * ) _aq_name.c_str(), _aq_s, _aq_t, rgb );
		}
	}
	while( ( ++__iGrid < shadingPointCount() ) && __fVarying);

	__iGrid = 0;
	bake_done( bd );

}

//----------------------------------------------------------------------
// occlusion(occlmap,P,N,samples)
void CqShaderExecEnv::SO_occlusion( IqShaderData* occlmap, IqShaderData* channel, IqShaderData* P, IqShaderData* N, IqShaderData* samples, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	if ( !getRenderContext() )
		return ;

	std::map<std::string, IqShaderData*> paramMap;
	GetTexParams(cParams, apParams, paramMap);

	__iGrid = 0;
	CqString _aq_occlmap;
	(occlmap)->GetString(_aq_occlmap,__iGrid);
	CqVector3D _aq_N;
	(N)->GetNormal(_aq_N,__iGrid);
	TqFloat _aq_samples;
	(samples)->GetFloat(_aq_samples,__iGrid);
	IqTextureMap* pMap = getRenderContext() ->GetShadowMap( _aq_occlmap );

	CqVector3D L(0,0,-1);

	__fVarying = true;
	if ( pMap != 0 && pMap->IsValid() )
	{
		std::valarray<TqFloat> fv;
		pMap->PrepareSampleOptions( paramMap );

		__iGrid = 0;
		CqBitVector& RS = RunningState();
		TqInt nPages = pMap->NumPages() - 1;
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				// Storage for the final combined occlusion value.
				TqFloat occlsum = 0.0f;
				TqFloat dotsum = 0.0f;

				CqVector3D swidth = 0.0f, twidth = 0.0f;

				CqVector3D _aq_N;
				CqVector3D _aq_P;

				(N)->GetNormal(_aq_N,__iGrid);
				(P)->GetPoint(_aq_P,__iGrid);
				TqInt i = nPages;
				for( ; i >= 0; i-- )
				{
					// Check if the lightsource is behind the sample.
					CqVector3D Nl = pMap->GetMatrix(2, i) * _aq_N;

					// In case of surface shader transform L
					TqFloat cosangle = Nl * L;

					if( cosangle <= 0.0f )
					{
						continue;
					}
					pMap->SampleMap( _aq_P, swidth, twidth, fv, i );
					occlsum += cosangle * fv[0];
					dotsum += cosangle;
				}
				if (dotsum != 0.0f)
					occlsum /= dotsum;
				(Result)->SetFloat(occlsum,__iGrid);
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
	else
	{
		__iGrid = 0;
		CqBitVector& RS = RunningState();
		do
		{
			if(!__fVarying || RS.Value( __iGrid ) )
			{
				(Result)->SetFloat(0.0f,__iGrid);	// Default, completely lit
			}
		}
		while( ( ++__iGrid < shadingPointCount() ) && __fVarying);
	}
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

void CqShaderExecEnv::SO_textureinfo( IqShaderData* name, IqShaderData* dataname, IqShaderData* pV, IqShaderData* Result, IqShader* pShader )
{
	TqUint __iGrid;

	if ( !getRenderContext() )
		return ;

	TqFloat Ret = 0.0f;
	IqTextureMap* pMap = NULL;
	IqTextureMap *pSMap = NULL;
	IqTextureMap *pEMap = NULL;
	IqTextureMap *pTMap = NULL;

	__iGrid = 0;
	CqString _aq_name;
	(name)->GetString(_aq_name,__iGrid);
	CqString _aq_dataname;
	(dataname)->GetString(_aq_dataname,__iGrid);

	if ( !pMap && strstr( _aq_name.c_str(), ".tif" ) )
	{
		pTMap = getRenderContext() ->GetTextureMap( _aq_name );
		if ( pTMap && ( pTMap->Type() == MapType_Texture ) )
		{
			pMap = pTMap;
		}
		else if ( pTMap )
		{
			//delete pTMap;
			pTMap = NULL;
		}
	}
	if ( !pMap )
	{
		pSMap = getRenderContext() ->GetShadowMap( _aq_name );
		if ( pSMap && ( pSMap->Type() == MapType_Shadow ) )
		{
			pMap = pSMap;
		}
		else if ( pSMap )
		{
			//delete pSMap;
			pSMap = NULL;
		}
	}

	if ( !pMap )
	{
		pEMap = getRenderContext() ->GetEnvironmentMap( _aq_name );
		if ( pEMap && ( pEMap->Type() == MapType_Environment ) )
		{
			pMap = pEMap;
		}
		else if ( pEMap )
		{
			//delete pEMap;
			pEMap = NULL;
		}
	}

	if ( !pMap )
	{
		pTMap = getRenderContext() ->GetTextureMap( _aq_name );
		if ( pTMap && ( pTMap->Type() == MapType_Texture ) )
		{
			pMap = pTMap;
		}
		else if ( pTMap )
		{
			//delete pTMap;
			pTMap = NULL;
		}
	}


	if ( pMap == 0 )
	{
		(Result)->SetFloat(Ret,__iGrid);
		return ;
	}

	if ( _aq_dataname.compare( "exists" ) == 0 )
	{
		if ( pV->Type() == type_float )
		{
			pV->SetFloat( 1.0f );
			Ret = 1.0f;
		}
	}

	if ( _aq_dataname.compare( "resolution" ) == 0 )
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
	if ( _aq_dataname.compare( "type" ) == 0 )
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

	if ( _aq_dataname.compare( "channels" ) == 0 )
	{
		if ( pV->Type() == type_float )
		{
			pV->SetFloat( static_cast<TqFloat>( pMap->SamplesPerPixel() ) );
			Ret = 1.0f;
		}

	}

	if ( _aq_dataname.compare( "viewingmatrix" ) == 0 )
	{
		if ( ( ( pV->Type() == type_float ) && ( pV->ArrayLength() == 16 ) ) ||
		        ( pV->Type() == type_matrix ) )
		{
			IqTextureMap* pTmp = NULL;
			if (pTMap) pTmp = pTMap;
			if (pSMap) pTmp = pSMap;
			if ( pTmp )   // && pTmp->Type() == MapType_Shadow)
			{


				CqMatrix m = pTmp->GetMatrix( 0 );  /* WorldToCamera */
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

	if ( _aq_dataname.compare( "projectionmatrix" ) == 0 )
	{
		if ( ( ( pV->Type() == type_float ) && ( pV->ArrayLength() == 16 ) ) ||
		        ( pV->Type() == type_matrix ) )
		{
			// init the matrix in case of wrong sl logic
			IqTextureMap* pTmp = NULL;
			if (pTMap) pTmp = pTMap;
			if (pSMap) pTmp = pSMap;
			if ( pTmp )    // && pTmp->Type() == MapType_Shadow)
			{

				CqMatrix m = pTmp->GetMatrix( 1 ); /* WorldToScreen */
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

	//delete pMap;

	(Result)->SetFloat(Ret,__iGrid);

}



END_NAMESPACE(    Aqsis )
//---------------------------------------------------------------------
