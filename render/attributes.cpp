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
		\brief Implements the CqAttributes class for handling RenderMan attributes.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	"aqsis.h"

#include	"attributes.h"
#include	"renderer.h"
#include	"shaders.h"
#include	"trimcurve.h"

START_NAMESPACE(Aqsis)


std::vector<CqAttributes*>	Attribute_stack;
static CqShaderSurfaceConstant	StandardSurface;

//---------------------------------------------------------------------
/** Constructor.
 */

CqShadingAttributes::CqShadingAttributes() :	
					m_colColor(1.0,1.0,1.0),
					m_colOpacity(1.0,1.0,1.0),
					m_pshadAreaLightSource(0),
					m_pshadSurface(0),
					m_pshadAtmosphere(0),
					m_pshadInteriorVolume(0),
					m_pshadExteriorVolume(0),
					m_fEffectiveShadingRate(1.0),
					m_eShadingInterpolation(ShadingConstant),
					m_bMatteSurfaceFlag(TqFalse)
{
	// Setup default surface attributes.
	m_aTextureCoordinates[0]=CqVector2D(0,0);
	m_aTextureCoordinates[1]=CqVector2D(1,0);
	m_aTextureCoordinates[2]=CqVector2D(0,1);
	m_aTextureCoordinates[3]=CqVector2D(1,1);

	// Use a default surface shader in case one isn't specified.
	SetpshadSurface(&StandardSurface);
}


//---------------------------------------------------------------------
/** Copy constructor.
 */
CqGeometricAttributes::CqGeometricAttributes(CqGeometricAttributes& From) :	
							m_Bound(From.m_Bound),
							m_fDetailRangeMinVisible(From.m_fDetailRangeMinVisible),
							m_fDetailRangeLowerTransition(From.m_fDetailRangeLowerTransition),
							m_fDetailRangeUpperTransition(From.m_fDetailRangeUpperTransition),
							m_fDetailRangeMaxVisible(From.m_fDetailRangeMaxVisible),
							m_matuBasis(From.m_matuBasis),
							m_matvBasis(From.m_matvBasis),
							m_uSteps(From.m_uSteps),
							m_vSteps(From.m_vSteps),
							m_eOrientation(From.m_eOrientation),
							m_eCoordsysOrientation(From.m_eCoordsysOrientation),
							m_iNumberOfSides(From.m_iNumberOfSides),
							m_pshadDisplacement(From.m_pshadDisplacement)
{
}


//---------------------------------------------------------------------
/** Destructor.
 */
CqGeometricAttributes::~CqGeometricAttributes()	
{
}



//---------------------------------------------------------------------
/** Constructor.
 */

CqAttributes::CqAttributes()	:	CqShadingAttributes(), CqGeometricAttributes()	
{
	Attribute_stack.push_back(this);
	m_StackIndex=Attribute_stack.size()-1;
}


//---------------------------------------------------------------------
/** Copy constructor.
 */

CqAttributes::CqAttributes(const CqAttributes& From)
{
	*this=From;
	
	// Register ourself with the global attribute stack.
	Attribute_stack.push_back(this);
	m_StackIndex=Attribute_stack.size()-1;
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqAttributes::~CqAttributes()
{
	assert(RefCount()==0);
	
	// Unreference the system attributes.
	TqInt i=m_aAttributes.size();
	while(i-->0)
	{
		m_aAttributes[i]->Release();
		m_aAttributes[i]=0;
	}

	// Remove ourself from the stack
	std::vector<CqAttributes*>::iterator p=Attribute_stack.begin();
	p+=m_StackIndex;
	std::vector<CqAttributes*>::iterator p2=p;
	while(p2!=Attribute_stack.end())
	{
		(*p2)->m_StackIndex--;
		p2++;
	}
	Attribute_stack.erase(p);
}

//---------------------------------------------------------------------
/** Copy function.
 */

CqAttributes& CqAttributes::operator=(const CqAttributes& From)
{
	CqGeometricAttributes::operator=(From);
	CqShadingAttributes::operator=(From);

	// Copy the system attributes.
	m_aAttributes.resize(From.m_aAttributes.size());
	TqInt i=From.m_aAttributes.size();
	while(i-->0)
	{
		m_aAttributes[i]=From.m_aAttributes[i];
		m_aAttributes[i]->AddRef();
	}

	// Copy the lightsource list.
	m_apLightsources.resize(0);
	std::vector<CqLightsource*>::const_iterator il;
	for(il=From.m_apLightsources.begin(); il!=From.m_apLightsources.end(); il++)
		m_apLightsources.push_back(*il);

	return(*this);
}
	

//---------------------------------------------------------------------
/** Get a system attribute parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return CqParameter pointer or 0 if not found.
 */

const CqParameter* CqAttributes::pParameter(const char* strName, const char* strParam) const
{
	const CqSystemOption* pOpt;
	if((pOpt=pAttribute(strName))!=0)
	{
		const CqParameter* pParam;
		if((pParam=pOpt->pParameter(strParam))!=0)
			return(pParam);
	}
	return(0);
}


//---------------------------------------------------------------------
/** Get a system attribute parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return CqParameter pointer or 0 if not found.
 */

CqParameter* CqAttributes::pParameterWrite(const char* strName, const char* strParam)
{
	CqSystemOption* pOpt;
	if((pOpt=pAttributeWrite(strName))!=0)
	{
		CqParameter* pParam;
		if((pParam=pOpt->pParameter(strParam))!=0)
			return(pParam);
	}
	return(0);
}


//---------------------------------------------------------------------
/** Get a float system attribute parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return Float pointer 0 if not found.
 */

TqFloat* CqAttributes::GetFloatAttributeWrite(const char* strName, const char* strParam)
{
	CqParameter* pParam=pParameterWrite(strName, strParam);
	if(pParam!=0)
		return(static_cast<CqParameterTypedUniform<TqFloat,Type_Float>*>(pParam)->pValue());
	else
		return(0);
}


//---------------------------------------------------------------------
/** Get an integer system attribute parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return Integer pointer 0 if not found.
 */

TqInt* CqAttributes::GetIntegerAttributeWrite(const char* strName, const char* strParam)
{
	CqParameter* pParam=pParameterWrite(strName, strParam);
	if(pParam!=0)
		return(static_cast<CqParameterTyped<TqInt>*>(pParam)->pValue());
	else
		return(0);
}


//---------------------------------------------------------------------
/** Get a string system attribute parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return CqString pointer 0 if not found.
 */

CqString* CqAttributes::GetStringAttributeWrite(const char* strName, const char* strParam)
{
	CqParameter* pParam=pParameterWrite(strName, strParam);
	if(pParam!=0)
		return(static_cast<CqParameterTyped<CqString>*>(pParam)->pValue());
	else
		return(0);
}


//---------------------------------------------------------------------
/** Get a point system attribute parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return CqVetor3D pointer 0 if not found.
 */

CqVector3D* CqAttributes::GetPointAttributeWrite(const char* strName, const char* strParam)
{
	CqParameter* pParam=pParameterWrite(strName, strParam);
	if(pParam!=0)
		return(static_cast<CqParameterTyped<CqVector3D>*>(pParam)->pValue());
	else
		return(0);
}


//---------------------------------------------------------------------
/** Get a color system attribute parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return CqColor pointer 0 if not found.
 */

CqColor* CqAttributes::GetColorAttributeWrite(const char* strName, const char* strParam)
{
	CqParameter* pParam=pParameterWrite(strName, strParam);
	if(pParam!=0)
		return(static_cast<CqParameterTyped<CqColor>*>(pParam)->pValue());
	else
		return(0);
}


//---------------------------------------------------------------------
/** Get a float system attribute parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return Float pointer 0 if not found.
 */

const TqFloat* CqAttributes::GetFloatAttribute(const char* strName, const char* strParam) const
{
	const CqParameter* pParam=pParameter(strName, strParam);
	if(pParam!=0)
		return(static_cast<const CqParameterTyped<TqFloat>*>(pParam)->pValue());
	else
		return(0);
}


//---------------------------------------------------------------------
/** Get an integer system attribute parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return Integer pointer 0 if not found.
 */

const TqInt* CqAttributes::GetIntegerAttribute(const char* strName, const char* strParam) const
{
	const CqParameter* pParam=pParameter(strName, strParam);
	if(pParam!=0)
		return(static_cast<const CqParameterTyped<TqInt>*>(pParam)->pValue());
	else
		return(0);
}


//---------------------------------------------------------------------
/** Get a string system attribute parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return CqString pointer 0 if not found.
 */

const CqString* CqAttributes::GetStringAttribute(const char* strName, const char* strParam) const
{
	const CqParameter* pParam=pParameter(strName, strParam);
	if(pParam!=0)
		return(static_cast<const CqParameterTyped<CqString>*>(pParam)->pValue());
	else
		return(0);
}


//---------------------------------------------------------------------
/** Get a point system attribute parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return CqVector3D pointer 0 if not found.
 */

const CqVector3D* CqAttributes::GetPointAttribute(const char* strName, const char* strParam) const
{
	const CqParameter* pParam=pParameter(strName, strParam);
	if(pParam!=0)
		return(static_cast<const CqParameterTyped<CqVector3D>*>(pParam)->pValue());
	else
		return(0);
}


//---------------------------------------------------------------------
/** Get a color system attribute parameter.
 * \param strName The name of the attribute.
 * \param strParam The name of the paramter on the attribute.
 * \return CqColor pointer 0 if not found.
 */

const CqColor* CqAttributes::GetColorAttribute(const char* strName, const char* strParam) const
{
	const CqParameter* pParam=pParameter(strName, strParam);
	if(pParam!=0)
		return(static_cast<const CqParameterTyped<CqColor>*>(pParam)->pValue());
	else
		return(0);
}


//---------------------------------------------------------------------

END_NAMESPACE(Aqsis)


