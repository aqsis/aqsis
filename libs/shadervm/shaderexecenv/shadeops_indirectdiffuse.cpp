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
		\brief Implements the basic shader operations. (Lights related)
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	<string>
#include	<stdio.h>

#include	<aqsis/math/math.h>
#include	<aqsis/core/ilightsource.h>
#include	"shaderexecenv.h"

#include <OpenEXR/ImathMath.h>
#include <OpenEXR/ImathVec.h>
#include <OpenEXR/ImathColor.h>

#include	"../../pointrender/MicroBuf.h"
#include	"../../pointrender/diffuse/DiffusePointOctreeCache.h"
#include	"../../pointrender/RadiosityIntegrator.h"
#include	"../../pointrender/OcclusionIntegrator.h"
#include	"../../pointrender/microbuf_proj_func.h"




namespace Aqsis {

using Imath::V3f;
using Imath::C3f;


namespace {
/// Store zeros in shader variable, depending on integrator type:
/// OcclusionIntegrator has result type float, whereas RadiosityIntegrator has
/// result type Color.
template<typename T>
void storeZeroResult(IqShaderData* result, int igrid)
{
	result->SetFloat(0.0f,igrid);
}
template<>
void storeZeroResult<RadiosityIntegrator>(IqShaderData* result, int igrid)
{
	result->SetColor(CqColor(0.0f),igrid);
}
}


// FIXME: It's pretty ugly to have a global cache here!
//
// Missing cache features:
// * Ri search paths
static DiffusePointOctreeCache g_pointOctreeCache;

void clearPointCloudCache()
{
	g_pointOctreeCache.clear();
}


template<typename IntegratorT>
void CqShaderExecEnv::pointCloudIntegrate(IqShaderData* P, IqShaderData* N,
										  IqShaderData* result, int cParams,
										  IqShaderData** apParams,
										  IqShader* pShader)
{
	if(!getRenderContext())
		return;

	// Extract options
	CqString paramName;
	const DiffusePointOctree* pointTree = 0;
	int faceRes = 10;
	float maxSolidAngle = 0.03;
	float coneAngle = M_PI_2;
	float bias = 0;
	CqString coordSystem = "world";
	IqShaderData* occlusionResult = 0;
	for(int i = 0; i < cParams; i+=2)
	{
		apParams[i]->GetString(paramName, 0);
		IqShaderData* paramValue = apParams[i+1];
		if(paramName == "coneangle")
		{
			if(paramValue->Type() == type_float)
				paramValue->GetFloat(coneAngle);
		}
		else if(paramName == "filename")
		{
			if(paramValue->Type() == type_string)
			{
				CqString fileName;
				paramValue->GetString(fileName, 0);
				pointTree = g_pointOctreeCache.find(fileName);
			}
		}
		else if(paramName == "maxsolidangle")
		{
			if(paramValue->Type() == type_float)
				paramValue->GetFloat(maxSolidAngle);
		}
		else if(paramName == "bias")
		{
			if(paramValue->Type() == type_float)
				paramValue->GetFloat(bias);
		}
		else if(paramName == "microbufres")
		{
			if(paramValue->Type() == type_float)
			{
				float res = 10;
				paramValue->GetFloat(res);
				faceRes = std::max(1, static_cast<int>(res));
			}
		}
		else if(paramName == "coordsystem")
		{
			if(paramValue->Type() == type_string)
				paramValue->GetString(coordSystem);
		}
		else if(paramName == "occlusion")
		{
			if(paramValue->Type() == type_float)
				occlusionResult = paramValue;
		}
		// Interesting arguments which could be implemented:
		//   "hitsides"    - sidedness culling: "front", "back", "both"
		//   "falloff", "falloffmode" - falloff of occlusion with distance
		//   ... more!
		//
		// Other arguments we may not bother with:
		//   "pointbased"  - we don't support any other method...
	}

	// Compute transform from current to appropriate space.
	CqMatrix positionTrans;
	getRenderContext()->matSpaceToSpace("current", coordSystem.c_str(),
										pShader->getTransform(),
										pTransform().get(), 0, positionTrans);
	CqMatrix normalTrans = normalTransform(positionTrans);

	// TODO: interpolation.  3delight uses Attribute "irradiance"
	// "shadingrate" to control interpolation; PRMan uses the "maxvariation"
	// parameter.

	// Number of vertices in u-direction of grid
	int uSize = m_uGridRes+1;

	bool varying = result->Class() == class_varying;
	const CqBitVector& RS = RunningState();
	if(pointTree)
	{
		int npoints = varying ? shadingPointCount() : 1;
#pragma omp parallel
		{
		// Compute occlusion for each point
		IntegratorT integrator(faceRes);
#pragma omp for
		for(int igrid = 0; igrid < npoints; ++igrid)
		{
			if(!varying || RS.Value(igrid))
			{
				CqVector3D Pval;
				// TODO: What about RiPoints?  They're not a 2D grid!
				int v = igrid/uSize;
				int u = igrid - v*uSize;
				float uinterp = 0;
				float vinterp = 0;
				// Microgrids sometimes meet each other at an acute angle.
				// Computing occlusion at the vertices where the grids meet is
				// then rather difficult because an occluding disk passes
				// exactly through the point to be occluded.  This usually
				// results in obvious light leakage from the other side of the
				// surface.
				//
				// To avoid this problem, we modify the position of any
				// vertices at the edges of grids by moving them inward
				// slightly.
				//
				// TODO: Make adjustable?
				const float edgeShrink = 0.2f;
				if(u == 0)
					uinterp = edgeShrink;
				else if(u == m_uGridRes)
				{
					uinterp = 1 - edgeShrink;
					--u;
				}
				if(v == 0)
					vinterp = edgeShrink;
				else if(v == m_vGridRes)
				{
					vinterp = 1 - edgeShrink;
					--v;
				}
				if(uinterp != 0 || vinterp != 0)
				{
					CqVector3D _P1; CqVector3D _P2;
					CqVector3D _P3; CqVector3D _P4;
					int uSize = m_uGridRes + 1;
					P->GetPoint(_P1, v*uSize + u);
					P->GetPoint(_P2, v*uSize + u+1);
					P->GetPoint(_P3, (v+1)*uSize + u);
					P->GetPoint(_P4, (v+1)*uSize + u+1);
					Pval = (1-vinterp)*(1-uinterp) * _P1 +
						   (1-vinterp)*uinterp     * _P2 +
						   vinterp*(1-uinterp)     * _P3 +
						   vinterp*uinterp         * _P4;
				}
				else
					P->GetVector(Pval, igrid);
				CqVector3D Nval;   N->GetVector(Nval, igrid);
				Pval = positionTrans * Pval;
				Nval = normalTrans * Nval;
				V3f Pval2(Pval.x(), Pval.y(), Pval.z());
				V3f Nval2(Nval.x(), Nval.y(), Nval.z());
				// TODO: It may make more sense to scale bias by the current
				// micropolygon radius - that way we avoid problems with an
				// absolute length scale.
				if(bias != 0)
					Pval2 += Nval2*bias;
				integrator.clear();
				microRasterize(integrator, Pval2, Nval2, coneAngle,
							   maxSolidAngle, *pointTree);
				storeIntegratedResult(integrator, Nval2, coneAngle, result,
									  occlusionResult, igrid);
			}
		}
		}
	}
	else
	{
		// Couldn't find point cloud, set result to zero.
		TqUint igrid = 0;
		do
		{
			if(!varying || RS.Value(igrid))
			{
				storeZeroResult<IntegratorT>(result, igrid);
			}
		}
		while( ( ++igrid < shadingPointCount() ) && varying);
	}
}


//----------------------------------------------------------------------
static void storeIntegratedResult(const OcclusionIntegrator& integrator,
								  const V3f& N, float coneAngle,
								  IqShaderData* result,
								  IqShaderData* /*occlusionResult*/, int igrid)
{
	result->SetFloat(integrator.occlusion(N, coneAngle), igrid);
}

// occlusion(P,N,samples)
void CqShaderExecEnv::SO_occlusion_rt( IqShaderData* P, IqShaderData* N, IqShaderData* samples, IqShaderData* Result, IqShader* pShader, int cParams, IqShaderData** apParams )
{
	pointCloudIntegrate<OcclusionIntegrator>(P, N, Result, cParams, apParams,
											 pShader);
}


//----------------------------------------------------------------------
// indirectdiffuse(P, N, samples, ...)
static void storeIntegratedResult(const RadiosityIntegrator& integrator,
								  const V3f& N, float coneAngle,
								  IqShaderData* result,
								  IqShaderData* occlusionResult, int igrid)
{
	float occ = 0;
	C3f col = integrator.radiosity(N, coneAngle, &occ);
	result->SetColor(CqColor(col.x, col.y, col.z), igrid);
	if(occlusionResult)
		occlusionResult->SetFloat(occ, igrid);
}

void CqShaderExecEnv::SO_indirectdiffuse(IqShaderData* P,
										IqShaderData* N,
										IqShaderData* samples,
										IqShaderData* Result,
										IqShader* pShader, int cParams,
										IqShaderData** apParams)
{
	pointCloudIntegrate<RadiosityIntegrator>(P, N, Result, cParams, apParams,
											 pShader);
}

}
