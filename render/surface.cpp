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
		\brief Implements the base GPrim handling classes.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	"aqsis.h"
#include	"renderer.h"
#include	"micropolygon.h"
#include	"surface.h"
#include	"vector2d.h"

START_NAMESPACE(Aqsis)


//---------------------------------------------------------------------
/** Default constructor
 */

CqBasicSurface::CqBasicSurface()	:	CqListEntry<CqBasicSurface>(), m_fDiceable(TqTrue), m_fDiscard(TqFalse), m_EyeSplitCount(0), 
										m_pAttributes(0), m_pTransform(0), m_SplitDir(SplitDir_U)
{
	// Set a refernce with the current attributes.
	m_pAttributes=const_cast<CqAttributes*>(QGetRenderContext()->pattrCurrent());
	m_pAttributes->AddRef();

	m_pTransform=const_cast<CqTransform*>(QGetRenderContext()->ptransCurrent());
	m_pTransform->AddRef();

	m_CachedBound=TqFalse;
}


//---------------------------------------------------------------------
/** Copy constructor
 */

CqBasicSurface::CqBasicSurface(const CqBasicSurface& From) : m_fDiceable(TqTrue),m_SplitDir(SplitDir_U)
{
	*this=From;

	// Set a reference with the donors attributes.
	m_pAttributes=From.m_pAttributes;
	m_pAttributes->AddRef();

	m_pTransform=From.m_pTransform;
	m_pTransform->AddRef();

	m_CachedBound=From.m_CachedBound;
	m_Bound=From.m_Bound;
}


//---------------------------------------------------------------------
/** Assignement operator
 */

CqBasicSurface& CqBasicSurface::operator=(const CqBasicSurface& From)
{
	m_fDiceable=From.m_fDiceable;
	m_EyeSplitCount=From.m_EyeSplitCount;
	m_fDiscard=From.m_fDiscard;
	
	return(*this);
}


//---------------------------------------------------------------------
/** Copy the local surface parameters from the donor surface.
 */

void CqBasicSurface::SetSurfaceParameters(const CqBasicSurface& From)
{
	// If we already have attributes, unreference them now as we don't need them anymore.
	if(m_pAttributes)	m_pAttributes->Release();
	if(m_pTransform)	m_pTransform->Release();

	// Now store and reference our new attributes.
	m_pAttributes=From.m_pAttributes;
	m_pAttributes->AddRef();

	m_pTransform=From.m_pTransform;
	m_pTransform->AddRef();
}


//---------------------------------------------------------------------
/** Return the name of this primitive surface if specified as a "identifier" "name" attribute,
 * otherwise return "not named"
 */

CqString CqBasicSurface::strName() const
{
	const CqString* pattrLightName=pAttributes()->GetStringAttribute("identifier","name");
	CqString strName("not named");
	if(pattrLightName!=0)	strName=pattrLightName[0];

	return(strName);
}


//---------------------------------------------------------------------
/** Work out which standard shader variables this surface requires by looking at the shaders.
 */

TqInt CqBasicSurface::Uses() const
{
	TqInt Uses=gDefUses;
	CqShader* pshadSurface=pAttributes()->pshadSurface();
	CqShader* pshadDisplacement=pAttributes()->pshadDisplacement();
	CqShader* pshadAtmosphere=pAttributes()->pshadAtmosphere();

	if(pshadSurface)		Uses|=pshadSurface->Uses();
	if(pshadDisplacement)	Uses|=pshadDisplacement->Uses();
	if(pshadAtmosphere)		Uses|=pshadAtmosphere->Uses();

	// Just a quick check, if it uses dPdu/dPdv must also use du/dv
	if(USES(Uses,EnvVars_dPdu))	Uses|=(1<<EnvVars_du);
	if(USES(Uses,EnvVars_dPdv))	Uses|=(1<<EnvVars_dv);
	// Just a quick check, if it uses du/dv must also use u/v
	if(USES(Uses,EnvVars_du))	Uses|=(1<<EnvVars_u);
	if(USES(Uses,EnvVars_dv))	Uses|=(1<<EnvVars_v);

	return(Uses);
}


//---------------------------------------------------------------------
/** Default constructor
 */

CqSurface::CqSurface()	:	CqBasicSurface(),
							m_P("P"),
							m_N("N"),
							m_Cs("Cs"),
							m_Os("Os"),
							m_s("s"),
							m_t("t"),
							m_u("u"),
							m_v("v")
{
}

//---------------------------------------------------------------------
/** Copy constructor
 */

CqSurface::CqSurface(const CqSurface& From) : CqBasicSurface(From),
											m_P("P"),
											m_N("N"),
											m_Cs("Cs"),
											m_Os("Os"),
											m_s("s"),
											m_t("t"),
											m_u("u"),
											m_v("v")
{
	*this=From;
}


//---------------------------------------------------------------------
/** Assignement operator
 */

CqSurface& CqSurface::operator=(const CqSurface& From)
{
	CqBasicSurface::operator=(From);

	// Copy primitive variables.
	m_P=From.m_P;
	m_N=From.m_N;
	m_Cs=From.m_Cs;
	m_Os=From.m_Os;
	m_s=From.m_s;
	m_t=From.m_t;
	m_u=From.m_u;
	m_v=From.m_v;

	return(*this);
}


//---------------------------------------------------------------------
/** Set the default values (where available) from the attribute state for all standard
 * primitive variables.
 */

void CqSurface::SetDefaultPrimitiveVariables(TqBool bUseDef_st)
{
	TqInt bUses=Uses();

	// Set default values for all of our parameters
	if(USES(bUses,EnvVars_Cs))
	{
		m_Cs.SetSize(1);
		m_Cs.pValue()[0]=m_pAttributes->colColor();
	}
	else
		m_Cs.SetSize(0);

	if(USES(bUses,EnvVars_Os))
	{
		m_Os.SetSize(1);
		m_Os.pValue()[0]=m_pAttributes->colOpacity();
	}
	else
		m_Os.SetSize(0);

	// s and t default to four values, if the particular surface type requires different it is up
	// to the surface to override or change this after the fact.
	if(USES(bUses,EnvVars_s) && bUseDef_st)
	{
		m_s.SetSize(4);	
		TqInt i;
		for(i=0; i<4; i++)
			m_s.pValue()[i]=m_pAttributes->aTextureCoordinates()[i].x();
	}
	else
		m_s.SetSize(0);

	if(USES(bUses,EnvVars_t) && bUseDef_st)
	{
		m_t.SetSize(4);	
		TqInt i;
		for(i=0; i<4; i++)
			m_t.pValue()[i]=m_pAttributes->aTextureCoordinates()[i].y();
	}
	else
		m_t.SetSize(0);

	if(USES(bUses,EnvVars_u))
	{
		m_u.SetSize(4);
		m_u.pValue()[0]=m_u.pValue()[2]=0.0;
		m_u.pValue()[1]=m_u.pValue()[3]=1.0;
	}
	else
		m_u.SetSize(0);
	
	if(USES(bUses,EnvVars_v))
	{
		m_v.SetSize(4);	
		m_v.pValue()[0]=m_v.pValue()[1]=0.0;
		m_v.pValue()[2]=m_v.pValue()[3]=1.0;	
	}
	else
		m_v.SetSize(0);
}


//---------------------------------------------------------------------

END_NAMESPACE(Aqsis)


