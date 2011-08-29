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
		\brief Implements the basic shader operations. (Texture, shadow, env, bake, occlusion related)
		\author Paul C. Gregory (pgregory@aqsis.org)
		\author Chris J. Foster (chris42f (at) gmail (dot) com)
*/


#ifdef AQSIS_SYSTEM_WIN32
#include	<io.h>
#endif

#include	<map>
#include	<string>
#include	<cstdio>
#include	<cstring>

#include	"shaderexecenv.h"
#include	<aqsis/tex/filtering/ienvironmentsampler.h>
#include	<aqsis/tex/filtering/iocclusionsampler.h>
#include	<aqsis/tex/filtering/ishadowsampler.h>
#include	<aqsis/tex/filtering/itexturecache.h>
#include	<aqsis/tex/filtering/itexturesampler.h>
#include	<aqsis/tex/io/texfileheader.h>
#include	<aqsis/tex/buffers/channellist.h>

namespace Aqsis
{

namespace
{

// helper functions and classes.

/** \brief Basic extractor for sample options from RSL texture() varargs
 * parameter list.
 *
 * Extracts options which are valid for all texture function types.
 */
template<typename SampleOptsT>
class CqSampleOptionExtractorBase
{
	private:
		/**
		 * Possible texture sample options; these will be null if no sample
		 * options are specified.
		 *
		 * \todo Inspection of the parameter list would be better done at
		 * shader load-time, assuming the parameter names are constant.
		 */
		IqShaderData* m_sBlur;
		IqShaderData* m_tBlur;
		IqShaderData* m_channel;

	protected:
		/** \brief Cache varying options, and extract uniform ones.
		 *
		 * \param paramList - list of additional parameters to an RSL texture()
		 *                    call as (name,value) pairs.
		 * \param numParams - length of paramList.
		 * \param opts - sample options in which to place uniform options.
		 */
		void extractUniformAndCacheVarying(IqShaderData** paramList, TqInt numParams,
				SampleOptsT& opts)
		{
			CqString paramName;
			for(TqInt i = 0; i < numParams; i+=2)
			{
				// Parameter name and data
				paramList[i]->GetString(paramName, 0);
				IqShaderData* param = paramList[i+1];
				handleParam(paramName, param, opts);
			}
		}

		/** \brief extract or cache a single parameter.
		 *
		 * Cache the parameter in the desired member variable if it's varying.
		 * If it's uniform then set the appropriate field in the sample
		 * options.
		 *
		 * \param name - parameter name
		 * \param value - parameter shader data
		 * \param opts - sample options into which uniform parameters should be placed.
		 */
		virtual void handleParam(const CqString& name, IqShaderData* value,
				SampleOptsT& opts)
		{
			// The following are varying
			if(name == "blur")
			{
				m_sBlur = value;
				m_tBlur = value;
			}
			else if(name == "sblur")
			{
				m_sBlur = value;
			}
			else if(name == "tblur")
			{
				m_tBlur = value;
			}
			else if(name == "channel")
			{
				m_channel = value;
			}
			// The rest are uniform
			else if(name == "width")
			{
				TqFloat tmp = 0;
				value->GetFloat(tmp, 0);
				opts.setSWidth(tmp);
				opts.setTWidth(tmp);
			}
			else if(name == "swidth")
			{
				TqFloat tmp = 0;
				value->GetFloat(tmp, 0);
				opts.setSWidth(tmp);
			}
			else if(name == "twidth")
			{
				TqFloat tmp = 0;
				value->GetFloat(tmp, 0);
				opts.setTWidth(tmp);
			}
			else if(name == "minwidth")
			{
				TqFloat tmp = 0;
				value->GetFloat(tmp, 0);
				opts.setMinWidth(tmp);
			}
			else if(name == "trunc")
			{
				TqFloat tmp = 0;
				value->GetFloat(tmp, 0);
				opts.setTruncAmount(tmp);
			}
			else if(name == "filter")
			{
				CqString tmp;
				value->GetString(tmp, 0);
				opts.setFilterType(enumCast<EqTextureFilter>(tmp.c_str()));
			}
		}

	public:
		/** \brief Initialize option extractor: extract uniform options, and cache varying ones.
		 *
		 * Cache the parameter in the desired member variable if it's varying.
		 * If it's uniform then set the appropriate field in the sample
		 * options.  Whether things are uniform or varying is described by the
		 * RISpec in the section dealing with the texture() shadeops.
		 *
		 * \param paramList - list of additional parameters to an RSL texture()
		 *                    call as (name,value) pairs.
		 * \param numParams - length of paramList.
		 * \param opts - sample options container to extract options into.
		 */
		CqSampleOptionExtractorBase()
			: m_sBlur(0),
			m_tBlur(0),
			m_channel(0)
		{ }

		/// Null destructor
		virtual ~CqSampleOptionExtractorBase() {}

		/** \brief Extract texture sample options from cached parameters
		 *
		 * \param gridIdx - index into varying shader parameter data.
		 */
		void extractVarying(TqInt gridIdx, SampleOptsT& opts)
		{
			if(m_sBlur)
			{
				TqFloat tmp = 0;
				m_sBlur->GetFloat(tmp, gridIdx);
				opts.setSBlur(tmp);
			}
			if(m_tBlur)
			{
				TqFloat tmp = 0;
				m_tBlur->GetFloat(tmp, gridIdx);
				opts.setTBlur(tmp);
			}
			if(m_channel)
			{
				TqFloat tmp = 0;
				m_channel->GetFloat(tmp, gridIdx);
				opts.setStartChannel(tmp);
			}
		}
};


//------------------------------------------------------------------------------
/** \brief Extractor for plain texture options
 */
class CqSampleOptionExtractor
	: private CqSampleOptionExtractorBase<CqTextureSampleOptions>
{
	protected:
		// From CqSampleOptionExtractorBase.
		virtual void handleParam(const CqString& name, IqShaderData* value,
				CqTextureSampleOptions& opts)
		{
			if(name == "fill")
			{
				TqFloat tmp = 0;
				value->GetFloat(tmp, 0);
				opts.setFill(tmp);
			}
			else if(name == "lerp")
			{
				TqFloat tmp = 0;
				value->GetFloat(tmp, 0);
				// Make sure lerp is one of the valid values.
				opts.setLerp(static_cast<EqMipmapLerp>(
							clamp<TqInt>(lround(tmp), 0, 2)));
			}
			else
			{
				// Else call through to the base class for the more basic
				// texture sample options.
				CqSampleOptionExtractorBase<CqTextureSampleOptions>
					::handleParam(name, value, opts);
			}
		}
	public:
		CqSampleOptionExtractor(IqShaderData** paramList, TqInt numParams,
				CqTextureSampleOptions& opts)
			: CqSampleOptionExtractorBase<CqTextureSampleOptions>()
		{
			extractUniformAndCacheVarying(paramList, numParams, opts);
		}

		CqSampleOptionExtractorBase<CqTextureSampleOptions>::extractVarying;
};


//------------------------------------------------------------------------------
class CqShadowOptionExtractor
	: private CqSampleOptionExtractorBase<CqShadowSampleOptions>
{
	private:
		/// Cached values for varying shadow bias.
		IqShaderData* m_biasLow;
		IqShaderData* m_biasHigh;
	protected:
		// From CqSampleOptionExtractor.
		virtual void handleParam(const CqString& name, IqShaderData* value,
				CqShadowSampleOptions& opts)
		{
			if(name == "bias")
			{
				m_biasLow = value;
				m_biasHigh = value;
			}
			else if(name == "bias0")
			{
				m_biasLow = value;
				if(!m_biasHigh)
					m_biasHigh = value;
			}
			else if(name == "bias1")
			{
				m_biasHigh = value;
				if(!m_biasLow)
					m_biasLow = value;
			}
			else if(name == "samples")
			{
				TqFloat tmp = 0;
				value->GetFloat(tmp, 0);
				opts.setNumSamples(static_cast<TqInt>(tmp));
			}
			else if(name == "depthapprox")
			{
				CqString tmp;
				value->GetString(tmp, 0);
				opts.setDepthApprox(enumCast<EqDepthApprox>(tmp.c_str()));
			}
			else
			{
				// Else call through to the base class for the more basic
				// texture sample options.
				CqSampleOptionExtractorBase<CqShadowSampleOptions>
					::handleParam(name, value, opts);
			}
		}
	public:
		CqShadowOptionExtractor(IqShaderData** paramList, TqInt numParams,
				CqShadowSampleOptions& opts)
			: CqSampleOptionExtractorBase<CqShadowSampleOptions>(),
			m_biasLow(0),
			m_biasHigh(0)
		{
			extractUniformAndCacheVarying(paramList, numParams, opts);
		}

		void extractVarying(TqInt gridIdx, CqShadowSampleOptions& opts)
		{
			if(m_biasLow)
			{
				TqFloat tmp = 0;
				m_biasLow->GetFloat(tmp, gridIdx);
				opts.setBiasLow(tmp);
			}
			if(m_biasHigh)
			{
				TqFloat tmp = 0;
				m_biasHigh->GetFloat(tmp, gridIdx);
				opts.setBiasHigh(tmp);
			}
			CqSampleOptionExtractorBase<CqShadowSampleOptions>::extractVarying(gridIdx, opts);
		}
};


//------------------------------------------------------------------------------
/// Fill any shadow sampling options obtainable from the renderer context via RiOptions.
void getRenderContextShadowOpts(const IqRenderer& context, CqShadowSampleOptions& sampleOpts)
{
	// Shadow biases
	if(const TqFloat* biasPtr = context.GetFloatOption("shadow", "bias"))
		sampleOpts.setBias(*biasPtr);
	if(const TqFloat* biasPtr = context.GetFloatOption("shadow", "bias0"))
		sampleOpts.setBiasLow(*biasPtr);
	if(const TqFloat* biasPtr = context.GetFloatOption("shadow", "bias1"))
		sampleOpts.setBiasHigh(*biasPtr);
}

} // unnamed namespace.

//----------------------------------------------------------------------
// texture(S)
void CqShaderExecEnv::SO_ftexture1( IqShaderData* name, IqShaderData* Result, IqShader* pShader, TqInt cParams, IqShaderData** apParams )
{
	SO_ftexture2(name, s(), t(), Result, pShader, cParams, apParams);
}

//----------------------------------------------------------------------
// texture(S,F,F)
void CqShaderExecEnv::SO_ftexture2(IqShaderData* name, IqShaderData* s, IqShaderData* t, IqShaderData* Result, IqShader* pShader, TqInt cParams, IqShaderData** apParams)
{
	TqInt gridIdx = 0;

	if(!getRenderContext())
	{
		/// \todo This check seems unnecessary - how could the render context be null?
		return;
	}

	// Get the texture map.
	CqString mapName;
	name->GetString(mapName, gridIdx);
	const IqTextureSampler& texSampler
		= getRenderContext()->textureCache().findTextureSampler(mapName.c_str());

	// Create new sample options to sample the texture with.
	CqTextureSampleOptions sampleOpts = texSampler.defaultSampleOptions();
	// Set some uniform sample options.
	sampleOpts.setNumChannels(1);

	// Initialize extraction of varargs texture options.
	CqSampleOptionExtractor optExtractor(apParams, cParams, sampleOpts);

	const CqBitVector& RS = RunningState();
	gridIdx = 0;
	do
	{
		if(RS.Value(gridIdx))
		{
			optExtractor.extractVarying(gridIdx, sampleOpts);
			// Edges of region to be filtered.
			CqVector2D diffUst(diffU<TqFloat>(s, gridIdx), diffU<TqFloat>(t, gridIdx));
			CqVector2D diffVst(diffV<TqFloat>(s, gridIdx), diffV<TqFloat>(t, gridIdx));
			// Centre of the texture region to be filtered.
			TqFloat ss = 0;
			TqFloat tt = 0;
			s->GetFloat(ss,gridIdx);
			t->GetFloat(tt,gridIdx);
			// Filter region
			SqSamplePllgram region(CqVector2D(ss,tt), diffUst, diffVst);
			// length-1 "array" where filtered results will be placed.
			TqFloat texSample = 0;
			texSampler.sample(region, sampleOpts, &texSample);
			Result->SetFloat(texSample, gridIdx);
		}
	}
	while( ++gridIdx < static_cast<TqInt>(shadingPointCount()) );
}

//----------------------------------------------------------------------
// texture(S,F,F,F,F,F,F,F,F)
void CqShaderExecEnv::SO_ftexture3( IqShaderData* name, IqShaderData* s1, IqShaderData* t1, IqShaderData* s2, IqShaderData* t2, IqShaderData* s3, IqShaderData* t3, IqShaderData* s4, IqShaderData* t4, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqInt gridIdx = 0;

	if(!getRenderContext())
	{
		/// \todo This check seems unnecessary - how could the render context be null?
		return;
	}

	// Get the texture map.
	CqString mapName;
	name->GetString(mapName, gridIdx);
	const IqTextureSampler& texSampler
		= getRenderContext()->textureCache().findTextureSampler(mapName.c_str());

	// Create new sample options to sample the texture with.
	CqTextureSampleOptions sampleOpts = texSampler.defaultSampleOptions();
	// Set some uniform sample options.
	sampleOpts.setNumChannels(1);

	// Initialize extraction of varargs texture options.
	CqSampleOptionExtractor optExtractor(apParams, cParams, sampleOpts);

	const CqBitVector& RS = RunningState();
	gridIdx = 0;
	do
	{
		if(RS.Value(gridIdx))
		{
			optExtractor.extractVarying(gridIdx, sampleOpts);
			// Compute the sample quadrilateral box.  Unfortunately we need all
			// these temporaries because the shader data interface leaves a bit
			// to be desired ;-)
			TqFloat s1Val = 0;  s1->GetFloat(s1Val, gridIdx);
			TqFloat s2Val = 0;  s2->GetFloat(s2Val, gridIdx);
			TqFloat s3Val = 0;  s3->GetFloat(s3Val, gridIdx);
			TqFloat s4Val = 0;  s4->GetFloat(s4Val, gridIdx);
			TqFloat t1Val = 0;  t1->GetFloat(t1Val, gridIdx);
			TqFloat t2Val = 0;  t2->GetFloat(t2Val, gridIdx);
			TqFloat t3Val = 0;  t3->GetFloat(t3Val, gridIdx);
			TqFloat t4Val = 0;  t4->GetFloat(t4Val, gridIdx);
			SqSampleQuad sampleQuad(CqVector2D(s1Val, t1Val), CqVector2D(s2Val, t2Val),
						CqVector2D(s3Val, t3Val), CqVector2D(s4Val, t4Val));

			// length-1 "array" where filtered results will be placed.
			TqFloat texSample = 0;
			texSampler.sample(sampleQuad, sampleOpts, &texSample);
			Result->SetFloat(texSample, gridIdx);
		}
	}
	while( ++gridIdx < static_cast<TqInt>(shadingPointCount()) );
}

//----------------------------------------------------------------------
// texture(S)
void CqShaderExecEnv::SO_ctexture1( IqShaderData* name, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	SO_ctexture2(name, s(), t(), Result, pShader, cParams, apParams);
}

//----------------------------------------------------------------------
// texture(S,F,F)
void CqShaderExecEnv::SO_ctexture2( IqShaderData* name, IqShaderData* s, IqShaderData* t, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqInt gridIdx = 0;

	if(!getRenderContext())
	{
		/// \todo This check seems unnecessary - how could the render context be null?
		return;
	}

	// Get the texture map.
	CqString mapName;
	name->GetString(mapName, gridIdx);
	const IqTextureSampler& texSampler
		= getRenderContext()->textureCache().findTextureSampler(mapName.c_str());

	// Create new sample options to sample the texture with.
	CqTextureSampleOptions sampleOpts = texSampler.defaultSampleOptions();
	// Set some uniform sample options.
	sampleOpts.setNumChannels(3);

	// Initialize extraction of varargs texture options.
	CqSampleOptionExtractor optExtractor(apParams, cParams, sampleOpts);

	const CqBitVector& RS = RunningState();
	gridIdx = 0;
	do
	{
		if(RS.Value(gridIdx))
		{
			optExtractor.extractVarying(gridIdx, sampleOpts);
			// Edges of region to be filtered.
			CqVector2D diffUst(diffU<TqFloat>(s, gridIdx), diffU<TqFloat>(t, gridIdx));
			CqVector2D diffVst(diffV<TqFloat>(s, gridIdx), diffV<TqFloat>(t, gridIdx));
			// Centre of the texture region to be filtered.
			TqFloat ss = 0;
			TqFloat tt = 0;
			s->GetFloat(ss,gridIdx);
			t->GetFloat(tt,gridIdx);
			// Filter region
			SqSamplePllgram region(CqVector2D(ss,tt), diffUst, diffVst);
			// array where filtered results will be placed.
			TqFloat texSample[3] = {0,0,0};
			texSampler.sample(region, sampleOpts, texSample);
			CqColor resultCol(texSample[0], texSample[1], texSample[2]);
			Result->SetColor(resultCol, gridIdx);
		}
	}
	while( ++gridIdx < static_cast<TqInt>(shadingPointCount()) );
}

//----------------------------------------------------------------------
// texture(S,F,F,F,F,F,F,F,F)
void CqShaderExecEnv::SO_ctexture3( IqShaderData* name, IqShaderData* s1, IqShaderData* t1, IqShaderData* s2, IqShaderData* t2, IqShaderData* s3, IqShaderData* t3, IqShaderData* s4, IqShaderData* t4, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqInt gridIdx = 0;

	if(!getRenderContext())
	{
		/// \todo This check seems unnecessary - how could the render context be null?
		return;
	}

	// Get the texture map.
	CqString mapName;
	name->GetString(mapName, gridIdx);
	const IqTextureSampler& texSampler
		= getRenderContext()->textureCache().findTextureSampler(mapName.c_str());

	// Create new sample options to sample the texture with.
	CqTextureSampleOptions sampleOpts = texSampler.defaultSampleOptions();
	// Set some uniform sample options.
	sampleOpts.setNumChannels(3);

	// Initialize extraction of varargs texture options.
	CqSampleOptionExtractor optExtractor(apParams, cParams, sampleOpts);

	const CqBitVector& RS = RunningState();
	gridIdx = 0;
	do
	{
		if(RS.Value(gridIdx))
		{
			optExtractor.extractVarying(gridIdx, sampleOpts);
			// Compute the sample quadrilateral box.  Unfortunately we need all
			// these temporaries because the shader data interface leaves a bit
			// to be desired ;-)
			TqFloat s1Val = 0;  s1->GetFloat(s1Val, gridIdx);
			TqFloat s2Val = 0;  s2->GetFloat(s2Val, gridIdx);
			TqFloat s3Val = 0;  s3->GetFloat(s3Val, gridIdx);
			TqFloat s4Val = 0;  s4->GetFloat(s4Val, gridIdx);
			TqFloat t1Val = 0;  t1->GetFloat(t1Val, gridIdx);
			TqFloat t2Val = 0;  t2->GetFloat(t2Val, gridIdx);
			TqFloat t3Val = 0;  t3->GetFloat(t3Val, gridIdx);
			TqFloat t4Val = 0;  t4->GetFloat(t4Val, gridIdx);
			SqSampleQuad sampleQuad(CqVector2D(s1Val, t1Val), CqVector2D(s2Val, t2Val),
					CqVector2D(s3Val, t3Val), CqVector2D(s4Val, t4Val));
			// array where filtered results will be placed.
			TqFloat texSample[3] = {0,0,0};
			texSampler.sample(sampleQuad, sampleOpts, texSample);
			CqColor resultCol(texSample[0], texSample[1], texSample[2]);
			Result->SetColor(resultCol, gridIdx);
		}
	}
	while( ++gridIdx < static_cast<TqInt>(shadingPointCount()) );
}



//----------------------------------------------------------------------
// environment(S,P)
void CqShaderExecEnv::SO_fenvironment2( IqShaderData* name, IqShaderData* R, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqInt gridIdx = 0;

	if(!getRenderContext())
	{
		/// \todo This check seems unnecessary - how could the render context be null?
		return;
	}

	// Get the texture map.
	CqString mapName;
	name->GetString(mapName, gridIdx);
	const IqEnvironmentSampler& texSampler
		= getRenderContext()->textureCache().findEnvironmentSampler(mapName.c_str());

	// Create new sample options to sample the texture with.
	CqTextureSampleOptions sampleOpts = texSampler.defaultSampleOptions();
	// Set some uniform sample options.
	sampleOpts.setNumChannels(1);

	// Initialize extraction of varargs texture options.
	CqSampleOptionExtractor optExtractor(apParams, cParams, sampleOpts);

	const CqBitVector& RS = RunningState();
	gridIdx = 0;
	do
	{
		if(RS.Value(gridIdx))
		{
			optExtractor.extractVarying(gridIdx, sampleOpts);
			// Get texture region to be filtered.
			CqVector3D RR;
			R->GetVector(RR, gridIdx);
			Sq3DSamplePllgram region(
				RR,
				diffU<CqVector3D>(R, gridIdx),
				diffV<CqVector3D>(R, gridIdx)
			);
			// buffer where filtered results will be placed.
			TqFloat texSample = 0;
			texSampler.sample(region, sampleOpts, &texSample);
			Result->SetFloat(texSample, gridIdx);
		}
	}
	while( ++gridIdx < static_cast<TqInt>(shadingPointCount()) );
}

//----------------------------------------------------------------------
// environment(S,P,P,P,P)
void CqShaderExecEnv::SO_fenvironment3( IqShaderData* name, IqShaderData* R1, IqShaderData* R2, IqShaderData* R3, IqShaderData* R4, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqInt gridIdx = 0;

	if(!getRenderContext())
	{
		/// \todo This check seems unnecessary - how could the render context be null?
		return;
	}

	// Get the texture map.
	CqString mapName;
	name->GetString(mapName, gridIdx);
	const IqEnvironmentSampler& texSampler
		= getRenderContext()->textureCache().findEnvironmentSampler(mapName.c_str());

	// Create new sample options to sample the texture with.
	CqTextureSampleOptions sampleOpts = texSampler.defaultSampleOptions();
	// Set some uniform sample options.
	sampleOpts.setNumChannels(1);

	// Initialize extraction of varargs texture options.
	CqSampleOptionExtractor optExtractor(apParams, cParams, sampleOpts);

	const CqBitVector& RS = RunningState();
	gridIdx = 0;
	do
	{
		if(RS.Value(gridIdx))
		{
			optExtractor.extractVarying(gridIdx, sampleOpts);
			// Construct the sample quadrilateral
			CqVector3D r1Val;  R1->GetVector(r1Val, gridIdx);
			CqVector3D r2Val;  R2->GetVector(r2Val, gridIdx);
			CqVector3D r3Val;  R3->GetVector(r3Val, gridIdx);
			CqVector3D r4Val;  R4->GetVector(r4Val, gridIdx);
			Sq3DSampleQuad sampleQuad(r1Val, r2Val, r3Val, r4Val);
			// buffer where filtered results will be placed.
			TqFloat texSample = 0;
			texSampler.sample(sampleQuad, sampleOpts, &texSample);
			Result->SetFloat(texSample, gridIdx);
		}
	}
	while( ++gridIdx < static_cast<TqInt>(shadingPointCount()) );
}


//----------------------------------------------------------------------
// environment(S,P)
void CqShaderExecEnv::SO_cenvironment2( IqShaderData* name, IqShaderData* R, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqInt gridIdx = 0;

	if(!getRenderContext())
	{
		/// \todo This check seems unnecessary - how could the render context be null?
		return;
	}

	// Get the texture map.
	CqString mapName;
	name->GetString(mapName, gridIdx);
	const IqEnvironmentSampler& texSampler
		= getRenderContext()->textureCache().findEnvironmentSampler(mapName.c_str());

	// Create new sample options to sample the texture with.
	CqTextureSampleOptions sampleOpts = texSampler.defaultSampleOptions();
	// Set some uniform sample options.
	sampleOpts.setNumChannels(3);

	// Initialize extraction of varargs texture options.
	CqSampleOptionExtractor optExtractor(apParams, cParams, sampleOpts);

	const CqBitVector& RS = RunningState();
	gridIdx = 0;
	do
	{
		if(RS.Value(gridIdx))
		{
			optExtractor.extractVarying(gridIdx, sampleOpts);
			// Get texture region to be filtered.
			CqVector3D RR;
			R->GetVector(RR, gridIdx);
			Sq3DSamplePllgram region(
				RR,
				diffU<CqVector3D>(R, gridIdx),
				diffV<CqVector3D>(R, gridIdx)
			);
			// buffer where filtered results will be placed.
			TqFloat texSample[3] = {0,0,0};
			texSampler.sample(region, sampleOpts, texSample);
			CqColor resultCol(texSample[0], texSample[1], texSample[2]);
			Result->SetColor(resultCol, gridIdx);
		}
	}
	while( ++gridIdx < static_cast<TqInt>(shadingPointCount()) );
}

//----------------------------------------------------------------------
// environment(S,P,P,P,P)
void CqShaderExecEnv::SO_cenvironment3( IqShaderData* name, IqShaderData* R1, IqShaderData* R2, IqShaderData* R3, IqShaderData* R4, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqInt gridIdx = 0;

	if(!getRenderContext())
	{
		/// \todo This check seems unnecessary - how could the render context be null?
		return;
	}

	// Get the texture map.
	CqString mapName;
	name->GetString(mapName, gridIdx);
	const IqEnvironmentSampler& texSampler
		= getRenderContext()->textureCache().findEnvironmentSampler(mapName.c_str());

	// Create new sample options to sample the texture with.
	CqTextureSampleOptions sampleOpts = texSampler.defaultSampleOptions();
	// Set some uniform sample options.
	sampleOpts.setNumChannels(3);

	// Initialize extraction of varargs texture options.
	CqSampleOptionExtractor optExtractor(apParams, cParams, sampleOpts);

	const CqBitVector& RS = RunningState();
	gridIdx = 0;
	do
	{
		if(RS.Value(gridIdx))
		{
			optExtractor.extractVarying(gridIdx, sampleOpts);
			// Construct the sample quadrilateral
			CqVector3D r1Val;  R1->GetVector(r1Val, gridIdx);
			CqVector3D r2Val;  R2->GetVector(r2Val, gridIdx);
			CqVector3D r3Val;  R3->GetVector(r3Val, gridIdx);
			CqVector3D r4Val;  R4->GetVector(r4Val, gridIdx);
			Sq3DSampleQuad sampleQuad(r1Val, r2Val, r3Val, r4Val);
			// buffer where filtered results will be placed.
			TqFloat texSample[3] = {0,0,0};
			texSampler.sample(sampleQuad, sampleOpts, texSample);
			CqColor resultCol(texSample[0], texSample[1], texSample[2]);
			Result->SetColor(resultCol, gridIdx);
		}
	}
	while( ++gridIdx < static_cast<TqInt>(shadingPointCount()) );
}

//----------------------------------------------------------------------
// bump(S)
void CqShaderExecEnv::SO_bump1( IqShaderData* name, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying = true;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
void CqShaderExecEnv::SO_bump2( IqShaderData* name, IqShaderData* s, IqShaderData* t, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying = true;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
void CqShaderExecEnv::SO_bump3( IqShaderData* name, IqShaderData* s1, IqShaderData* t1, IqShaderData* s2, IqShaderData* t2, IqShaderData* s3, IqShaderData* t3, IqShaderData* s4, IqShaderData* t4, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	bool __fVarying;
	TqUint __iGrid;

	__fVarying = true;

	__iGrid = 0;
	const CqBitVector& RS = RunningState();
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
void CqShaderExecEnv::SO_shadow( IqShaderData* name, IqShaderData* P, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqInt gridIdx = 0;

	if(!getRenderContext())
	{
		/// \todo This check seems unnecessary - how could the render context be null?
		return;
	}

	// Get the shadow map.
	CqString mapName;
	name->GetString(mapName, gridIdx);
	const IqShadowSampler& shadSampler
		= getRenderContext()->textureCache().findShadowSampler(mapName.c_str());

	// Create new sample options to sample the texture with.
	CqShadowSampleOptions sampleOpts = shadSampler.defaultSampleOptions();
	// Set some uniform sample options.
	// Number of channels.
	sampleOpts.setNumChannels(1);
	getRenderContextShadowOpts(*getRenderContext(), sampleOpts);

	// Initialize extraction of varargs texture options.
	CqShadowOptionExtractor optExtractor(apParams, cParams, sampleOpts);

	const CqBitVector& RS = RunningState();
	gridIdx = 0;
	do
	{
		if(RS.Value(gridIdx))
		{
			optExtractor.extractVarying(gridIdx, sampleOpts);
			// Get first differences along U & V directions for the sample quad.
			CqVector3D dP_uOn2 = 0.5f*diffU<CqVector3D>(P, gridIdx);
			CqVector3D dP_vOn2 = 0.5f*diffV<CqVector3D>(P, gridIdx);
			// Centre of the texture region to be filtered.
			CqVector3D centerP;
			P->GetPoint(centerP, gridIdx);
			// Compute the sample quadrilateral box.
			Sq3DSampleQuad sampleQuad(
				centerP - dP_uOn2 - dP_vOn2, centerP + dP_uOn2 - dP_vOn2, 
				centerP - dP_uOn2 + dP_vOn2, centerP + dP_uOn2 + dP_vOn2);
			// length-1 "array" where filtered results will be placed.
			TqFloat shadSample = 0;
			shadSampler.sample(sampleQuad, sampleOpts, &shadSample);
			Result->SetFloat(shadSample, gridIdx);
		}
	}
	while( ++gridIdx < static_cast<TqInt>(shadingPointCount()) );
}

//----------------------------------------------------------------------
// shadow(S,P,P,P,P)

void CqShaderExecEnv::SO_shadow1( IqShaderData* name, IqShaderData* P1, IqShaderData* P2, IqShaderData* P3, IqShaderData* P4, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	TqInt gridIdx = 0;

	if(!getRenderContext())
	{
		/// \todo This check seems unnecessary - how could the render context be null?
		return;
	}

	// Get the shadow map.
	CqString mapName;
	name->GetString(mapName, gridIdx);
	const IqShadowSampler& shadSampler
		= getRenderContext()->textureCache().findShadowSampler(mapName.c_str());

	// Create new sample options to sample the texture with.
	CqShadowSampleOptions sampleOpts = shadSampler.defaultSampleOptions();
	// Set some uniform sample options.
	// Number of channels.
	sampleOpts.setNumChannels(1);
	getRenderContextShadowOpts(*getRenderContext(), sampleOpts);

	// Initialize extraction of varargs texture options.
	CqShadowOptionExtractor optExtractor(apParams, cParams, sampleOpts);

	const CqBitVector& RS = RunningState();
	gridIdx = 0;
	do
	{
		if(RS.Value(gridIdx))
		{
			optExtractor.extractVarying(gridIdx, sampleOpts);

			// Get sampling quad, explicitly provided by user.
			Sq3DSampleQuad sampleQuad;
			P1->GetPoint(sampleQuad.v1, gridIdx);
			P2->GetPoint(sampleQuad.v2, gridIdx);
			P3->GetPoint(sampleQuad.v3, gridIdx);
			P4->GetPoint(sampleQuad.v4, gridIdx);

			// length-1 "array" where filtered results will be placed.
			TqFloat shadSample = 0;
			shadSampler.sample(sampleQuad, sampleOpts, &shadSample);
			Result->SetFloat(shadSample, gridIdx);
		}
	}
	while( ++gridIdx < static_cast<TqInt>(shadingPointCount()) );
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
				if (!fseek(file, 0, SEEK_END) && ftell(file) == 0)
				{
					// write once the header part
					fprintf ( file, "Aqsis bake file\n");
					fprintf ( file, "%d\n", elsize - 2);
				}
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
typedef std::map<std::string, bool> BakingAccess;

static BakingAccess *Existing = new BakingAccess;

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
	BakingAccess::iterator exist = Existing->find ( name );

        if (exist == Existing->end())
	{
		// Erase the bake file if they were not managed yet.
		// The bake file must be already processed earlier and 
		// it is time to start from stratch.
		unlink ( name.c_str() );
		(*Existing)[ name ] = true;
	}
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
	const CqBitVector& RS = RunningState();
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
	const CqBitVector& RS = RunningState();
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
			rgb[0] = _aq_f.r();
			rgb[1] = _aq_f.g();
			rgb[2] = _aq_f.b();
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
	const CqBitVector& RS = RunningState();
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
	const CqBitVector& RS = RunningState();
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
	const CqBitVector& RS = RunningState();
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
void CqShaderExecEnv::SO_occlusion(IqShaderData* name, IqShaderData* P, IqShaderData* N, IqShaderData* samples, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams)
{
	TqInt gridIdx = 0;

	// TODO: Formally deprecate and remove the samples parameter?
	// TODO: Investigate the need for a CqOcclusionSampleOpts options class ?

	if(!getRenderContext())
	{
		/// \todo This check seems unnecessary - how could the render context be null?
		return;
	}

	// Get the occlusion map.
	CqString mapName;
	name->GetString(mapName, gridIdx);
	const IqOcclusionSampler& occSampler
		= getRenderContext()->textureCache().findOcclusionSampler(mapName.c_str());

	// Create new sample options to sample the texture with.
	CqShadowSampleOptions sampleOpts = occSampler.defaultSampleOptions();
	// Set some uniform sample options.
	// Number of channels.
	sampleOpts.setNumChannels(1);
	getRenderContextShadowOpts(*getRenderContext(), sampleOpts);

	// Initialize extraction of varargs texture options.
	CqShadowOptionExtractor optExtractor(apParams, cParams, sampleOpts);

	const CqBitVector& RS = RunningState();
	gridIdx = 0;
	do
	{
		if(RS.Value(gridIdx))
		{
			optExtractor.extractVarying(gridIdx, sampleOpts);
			// Get normal to region.
			CqVector3D NN;
			N->GetNormal(NN, gridIdx);
			// Get texture region to be filtered.
			CqVector3D PP;
			P->GetPoint(PP, gridIdx);
			Sq3DSamplePllgram region(
				PP,
				diffU<CqVector3D>(P, gridIdx),
				diffV<CqVector3D>(P, gridIdx)
			);
			// length-1 "array" where filtered results will be placed.
			TqFloat occSample = 0;
			occSampler.sample(region, NN, sampleOpts, &occSample);
			Result->SetFloat(occSample, gridIdx);
		}
	}
	while( ++gridIdx < static_cast<TqInt>(shadingPointCount()) );
}

//----------------------------------------------------------------------
// textureinfo(texturename, dataname, output variable);
void CqShaderExecEnv::SO_textureinfo( IqShaderData* name, IqShaderData* dataName, IqShaderData* pV, IqShaderData* result, IqShader* pShader )
{
	if(!getRenderContext())
	{
		// \todo Is this check necessary?
		return;
	}

	// Name of the texture file.
	CqString textureName;
	name->GetString(textureName, 0);
	const CqTexFileHeader* header
		= getRenderContext()->textureCache().textureInfo(textureName.c_str());
	if(!header)
	{
		// Texture not found - return 0.
		result->SetFloat(0);
		return;
	}

	// Name identifying the texture attribute desired.
	CqString dataNameStr;
	dataName->GetString(dataNameStr, 0);

	TqFloat returnVal = 0;
	if(dataNameStr == "exists" && pV->Type() == type_float)
	{
		pV->SetFloat(1);
		returnVal = 1;
	}
	else if(dataNameStr == "resolution" && pV->Type() == type_float
			&& pV->ArrayLength() == 2)
	{
		pV->ArrayEntry(0)->SetFloat(header->width());
		pV->ArrayEntry(1)->SetFloat(header->height());
		returnVal = 1;
	}
	else if(dataNameStr == "type" && pV->Type() == type_string)
	{
		const EqTextureFormat* texFormat = header->findPtr<Attr::TextureFormat>();
		CqString formatStr = "texture";
		if(texFormat)
		{
			switch(*texFormat)
			{
				case TextureFormat_CubeEnvironment:
				case TextureFormat_LatLongEnvironment:
					formatStr = "environment";
					break;
				case TextureFormat_Shadow:
					formatStr = "shadow";
					break;
				case TextureFormat_Occlusion:
					formatStr = "occlusion";
					break;
				case TextureFormat_Unknown:
				case TextureFormat_Plain:
					formatStr = "texture";
					break;
			}
		}
		pV->SetString(formatStr);
		returnVal = 1;
	}
	else if(dataNameStr == "channels" && pV->Type() == type_float)
	{
		pV->SetFloat(header->channelList().numChannels());
		returnVal = 1;
	}
	else if(dataNameStr == "viewingmatrix" && pV->Type() == type_matrix)
	{
		const CqMatrix* worldToLight = header->findPtr<Attr::WorldToCameraMatrix>();
		if(worldToLight)
		{
			CqMatrix currToWorld;
			getRenderContext()->matSpaceToSpace("current", "world",
					NULL, NULL, 0, currToWorld);
			pV->SetMatrix((*worldToLight)*currToWorld);
			returnVal = 1;
		}
	}
	else if(dataNameStr == "projectionmatrix" && pV->Type() == type_matrix)
	{
		const CqMatrix* worldToLightNdc = header->findPtr<Attr::WorldToScreenMatrix>();
		if(worldToLightNdc)
		{
			CqMatrix currToWorld;
			getRenderContext()->matSpaceToSpace("current", "world",
					NULL, NULL, 0, currToWorld);
			pV->SetMatrix((*worldToLightNdc)*currToWorld);
			returnVal = 1;
		}
	}
	result->SetFloat(returnVal);
}

//---------------------------------------------------------------------
} // namespace Aqsis

