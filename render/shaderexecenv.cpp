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
		\brief Implements classes and support structures for the shader execution environment.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	"aqsis.h"
#include	"shaderexecenv.h"
#include	"shadervm.h"
#include	"surface.h"

START_NAMESPACE(Aqsis)

CqNoise		CqShaderExecEnv::m_noise;
CqCellNoise	CqShaderExecEnv::m_cellnoise;
CqRandom	CqShaderExecEnv::m_random;
CqMatrix	CqShaderExecEnv::m_matIdentity;

char*		CqShaderExecEnv::m_apVariableNames[EnvVars_Last]=
{
	"Cs",
	"Os",
	"Ng",
	"du",
	"dv",
	"L",
	"Cl",
	"Ol",
 	"P",
	"dPdu",
	"dPdv",
	"N",
	"u",
	"v",
	"s",
	"t",
	"I",
	"Ci",
	"Oi",
	"Ps",
 	"E",
	"ncomps",
	"time",
	"alpha"
};


// TODO: See if we can reduce these default requires further!
TqInt	gDefUses=(1<<EnvVars_P)|(1<<EnvVars_I)|(1<<EnvVars_N)|(1<<EnvVars_Ng)|(1<<EnvVars_L)|(1<<EnvVars_Cl)|(1<<EnvVars_Ci)|(1<<EnvVars_Oi);
TqInt	gDefLightUses=(1<<EnvVars_P)|(1<<EnvVars_L)|(1<<EnvVars_Ps);

//----------------------------------------------------------------------
/** Constructor.
 */

CqShaderExecEnv::CqShaderExecEnv() : m_li(0), m_Illuminate(0), m_pSurface(0), m_vfCulled("__culled")
{
	m_apVariables.resize(EnvVars_Last);
	TqInt i;
	for(i=0; i<EnvVars_Last; i++)
		m_apVariables[i]=0;
}				                


//----------------------------------------------------------------------
/** Destructor.
 */

CqShaderExecEnv::~CqShaderExecEnv()
{
	TqInt i;
	for(i=0; i<EnvVars_Last; i++)
		delete(m_apVariables[i]);
}				                

//---------------------------------------------------------------------
/** Initialise variables to correct size for current grid.
 */

void CqShaderExecEnv::Initialise(const TqInt uGridRes, const TqInt vGridRes, CqSurface* pSurface, TqInt Uses)
{
	m_uGridRes=uGridRes;
	m_vGridRes=vGridRes;
	m_GridSize=(uGridRes+1)*(vGridRes+1);
	m_GridI=0;

	// Store a pointer to the surface definition.
	m_pSurface=pSurface;

	m_li=0;
	m_Illuminate=0;
	m_IlluminanceCacheValid=TqFalse;

	// Initialise the state bitvectors
	m_CurrentState.SetSize((uGridRes+1)*(vGridRes+1));
	m_RunningState.SetSize((uGridRes+1)*(vGridRes+1));
	m_RunningState.SetAll(TqTrue);

	if(USES(Uses,EnvVars_Cs    )&&m_apVariables[EnvVars_Cs    ]==0)	m_apVariables[EnvVars_Cs    ]=new CqShaderVariableVarying<Type_Color,CqColor>(m_apVariableNames[EnvVars_Cs]);
	if(USES(Uses,EnvVars_Os    )&&m_apVariables[EnvVars_Os    ]==0)	m_apVariables[EnvVars_Os    ]=new CqShaderVariableVarying<Type_Color,CqColor>(m_apVariableNames[EnvVars_Os]);
	if(USES(Uses,EnvVars_Ng    )&&m_apVariables[EnvVars_Ng    ]==0)	m_apVariables[EnvVars_Ng    ]=new CqShaderVariableVarying<Type_Normal,CqVector3D>(m_apVariableNames[EnvVars_Ng]);
	if(USES(Uses,EnvVars_du    )&&m_apVariables[EnvVars_du    ]==0)	m_apVariables[EnvVars_du    ]=new CqShaderVariableUniform<Type_Float,TqFloat>(m_apVariableNames[EnvVars_du]);
	if(USES(Uses,EnvVars_dv    )&&m_apVariables[EnvVars_dv    ]==0)	m_apVariables[EnvVars_dv    ]=new CqShaderVariableUniform<Type_Float,TqFloat>(m_apVariableNames[EnvVars_dv]);
	if(USES(Uses,EnvVars_L     )&&m_apVariables[EnvVars_L     ]==0)	m_apVariables[EnvVars_L     ]=new CqShaderVariableVarying<Type_Point,CqVector3D>(m_apVariableNames[EnvVars_L]);
	if(USES(Uses,EnvVars_Cl    )&&m_apVariables[EnvVars_Cl    ]==0)	m_apVariables[EnvVars_Cl    ]=new CqShaderVariableVarying<Type_Color,CqColor>(m_apVariableNames[EnvVars_Cl]);
	if(USES(Uses,EnvVars_Ol    )&&m_apVariables[EnvVars_Ol    ]==0)	m_apVariables[EnvVars_Ol    ]=new CqShaderVariableVarying<Type_Color,CqColor>(m_apVariableNames[EnvVars_Ol]);
	if(USES(Uses,EnvVars_P     )&&m_apVariables[EnvVars_P     ]==0)	m_apVariables[EnvVars_P     ]=new CqShaderVariableVarying<Type_Point,CqVector3D>(m_apVariableNames[EnvVars_P]);
	if(USES(Uses,EnvVars_dPdu  )&&m_apVariables[EnvVars_dPdu  ]==0)	m_apVariables[EnvVars_dPdu  ]=new CqShaderVariableVarying<Type_Point,CqVector3D>(m_apVariableNames[EnvVars_dPdu]);
	if(USES(Uses,EnvVars_dPdv  )&&m_apVariables[EnvVars_dPdv  ]==0)	m_apVariables[EnvVars_dPdv  ]=new CqShaderVariableVarying<Type_Point,CqVector3D>(m_apVariableNames[EnvVars_dPdv]);
	if(USES(Uses,EnvVars_N     )&&m_apVariables[EnvVars_N     ]==0)	m_apVariables[EnvVars_N     ]=new CqShaderVariableVarying<Type_Normal,CqVector3D>(m_apVariableNames[EnvVars_N]);
	if(USES(Uses,EnvVars_u     )&&m_apVariables[EnvVars_u     ]==0)	m_apVariables[EnvVars_u     ]=new CqShaderVariableVarying<Type_Float,TqFloat>(m_apVariableNames[EnvVars_u]);
	if(USES(Uses,EnvVars_v     )&&m_apVariables[EnvVars_v     ]==0)	m_apVariables[EnvVars_v     ]=new CqShaderVariableVarying<Type_Float,TqFloat>(m_apVariableNames[EnvVars_v]);
	if(USES(Uses,EnvVars_s     )&&m_apVariables[EnvVars_s     ]==0)	m_apVariables[EnvVars_s     ]=new CqShaderVariableVarying<Type_Float,TqFloat>(m_apVariableNames[EnvVars_s]);
	if(USES(Uses,EnvVars_t     )&&m_apVariables[EnvVars_t     ]==0)	m_apVariables[EnvVars_t     ]=new CqShaderVariableVarying<Type_Float,TqFloat>(m_apVariableNames[EnvVars_t]);
	if(USES(Uses,EnvVars_I     )&&m_apVariables[EnvVars_I     ]==0)	m_apVariables[EnvVars_I     ]=new CqShaderVariableVarying<Type_Point,CqVector3D>(m_apVariableNames[EnvVars_I]);
	if(USES(Uses,EnvVars_Ci    )&&m_apVariables[EnvVars_Ci    ]==0)	m_apVariables[EnvVars_Ci    ]=new CqShaderVariableVarying<Type_Color,CqColor>(m_apVariableNames[EnvVars_Ci]);
	if(USES(Uses,EnvVars_Oi    )&&m_apVariables[EnvVars_Oi    ]==0)	m_apVariables[EnvVars_Oi    ]=new CqShaderVariableVarying<Type_Color,CqColor>(m_apVariableNames[EnvVars_Oi]);
	if(USES(Uses,EnvVars_Ps    )&&m_apVariables[EnvVars_Ps    ]==0)	m_apVariables[EnvVars_Ps    ]=new CqShaderVariableVarying<Type_Point,CqVector3D>(m_apVariableNames[EnvVars_Ps]);
	if(USES(Uses,EnvVars_E     )&&m_apVariables[EnvVars_E     ]==0)	m_apVariables[EnvVars_E     ]=new CqShaderVariableUniform<Type_Point,CqVector3D>(m_apVariableNames[EnvVars_E]);
	if(USES(Uses,EnvVars_ncomps)&&m_apVariables[EnvVars_ncomps]==0)	m_apVariables[EnvVars_ncomps]=new CqShaderVariableUniform<Type_Float,TqFloat>(m_apVariableNames[EnvVars_ncomps]);
	if(USES(Uses,EnvVars_time  )&&m_apVariables[EnvVars_time  ]==0)	m_apVariables[EnvVars_time  ]=new CqShaderVariableUniform<Type_Float,TqFloat>(m_apVariableNames[EnvVars_time]);
	if(USES(Uses,EnvVars_alpha )&&m_apVariables[EnvVars_alpha ]==0)	m_apVariables[EnvVars_alpha ]=new CqShaderVariableUniform<Type_Float,TqFloat>(m_apVariableNames[EnvVars_alpha]);

	TqInt i;
	for(i=0; i<EnvVars_Last; i++)
	{
		if(m_apVariables[i] && USES(Uses,i))
			m_apVariables[i]->Initialise(uGridRes, vGridRes, m_GridI);
	}
}


//---------------------------------------------------------------------
/** Return the object to world matrix for the surface this shader is attached to, if
 * no surface (i.e. a lightsource) return identity.
 */

const CqMatrix& CqShaderExecEnv::matObjectToWorld() const
{
	return(m_pSurface==0?m_matIdentity:m_pSurface->pTransform()->matObjectToWorld());
}


END_NAMESPACE(Aqsis)
//---------------------------------------------------------------------
