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
		\brief Declares the CqAttributes class for handling RenderMan attributes.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is attributes.h included already?
#ifndef ATTRIBUTES_H_INCLUDED
//{
#define ATTRIBUTES_H_INCLUDED 1

#include	<vector>

#include	"specific.h"

#include	"color.h"
#include	"ri.h"
#include	"matrix.h"
#include	"options.h"
#include	"bound.h"
#include	"spline.h"

#define		_qShareName	CORE
#include	"share.h"

START_NAMESPACE(Aqsis)

/// The options for the orientation of a coordinate system.
enum EqOrientation	
{
	OrientationLH,	///< Left hand coordinate system.
	OrientationRH,	///< Right and coordinate system.
};

class CqLightsource;
class CqShader;

//-----------------------------------------------------------------------
/**
	Class containing the current status of the attributes related shading state
*/

class CqShadingAttributes
{
	public:
						CqShadingAttributes();
		virtual			~CqShadingAttributes()	{}

						/// Possible values for the shading interplation attribute.
			enum		ShadingInterpolation	
						{
							ShadingConstant,	///< use constant shading, i.e. one value per micropoly.
							ShadingSmooth,		///< use smooth shading, i.e. interpolate the values at the corners of a micropoly.
						};

						/** Get a read only pointer to the current texture coordinates.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 * \return a pointer to an array of 2D vectors representing the four corners of texture space.
						 */
	const	CqVector2D*	aTextureCoordinates(TqFloat time=0.0f) const
												{return(m_aTextureCoordinates);}
						/** Get a pointer to the current texture coordinates.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 * \return a pointer to an array of 2D vectors representing the four corners of texture space.
						 */
			CqVector2D*	aTextureCoordinates(TqFloat time=0.0f) 
												{return(m_aTextureCoordinates);}
						/** Get the current shading rate.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 * \return a float shading rate.
						 */
			TqFloat		fEffectiveShadingRate(TqFloat time=0.0f) const
												{return(m_fEffectiveShadingRate);}
						/** Set the current shading rate.
						 * \param fValue the new value for the shading rate.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 */
			void		SetfEffectiveShadingRate(const TqFloat fValue,TqFloat time=0.0f)
												{m_fEffectiveShadingRate=fValue;}
						/** Get the current shading interpolation mode.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 * \return the current mode.
						 */
			ShadingInterpolation eShadingInterpolation(TqFloat time=0.0f) const
												{return(m_eShadingInterpolation);}
						/** Set the current shading interpolation mode.
						 * \param eValue the new value for the shading interpolation mode.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 */
			void		SeteShadingInterpolation(const ShadingInterpolation eValue,TqFloat time=0.0f)
												{m_eShadingInterpolation=eValue;}
						/** Get the current matte flag state.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 * \return a bool representing the flag state.
						 */
			TqBool		bMatteSurfaceFlag(TqFloat time=0.0f) const
												{return(m_bMatteSurfaceFlag);}
						/** Set the current state of the mate flag.
						 * \param bValue the new value for the flag.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 */
			void		SetbMatteSurfaceFlag(const TqBool bValue,TqFloat time=0.0f)
												{m_bMatteSurfaceFlag=bValue;}

						/** Get the current surface shader.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 * \return a pointer to the surface shader.
						 */
			CqShader*	pshadSurface(TqFloat time=0.0f) const	
												{return(m_pshadSurface);}
						/** Set the current surface shader.
						 * \param pshadSurface a pointer to a shader to use as the surface shader.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 */
			void		SetpshadSurface(CqShader* pshadSurface,TqFloat time=0.0f)
												{m_pshadSurface=pshadSurface;}
						/** Get the current atmosphere shader.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 * \return a pointer to the atmosphere shader.
						 */
			CqShader*	pshadAtmosphere(TqFloat time=0.0f) const	
												{return(m_pshadAtmosphere);}
						/** Set the current atmosphere shader.
						 * \param pshadAtmosphere a pointer to a shader to use as the atmosphere shader.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 */
			void		SetpshadAtmosphere(CqShader* pshadAtmosphere,TqFloat time=0.0f)
												{m_pshadAtmosphere=pshadAtmosphere;}
						/** Get the current color.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 * \return a reference to the color.
						 */
	const	CqColor&	colColor(TqFloat time=0.0f) const		{return(m_colColor);}
						
						/** Set the current color.
						 * \param colValue the new color.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 */
			void		SetcolColor(const CqColor& colValue, TqFloat time=0.0f)				
																{m_colColor=colValue;}
						/** Get the current opacity.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 * \return a reference to the opacity.
						 */
	const	CqColor&	colOpacity(TqFloat time=0.0f) const		{return(m_colOpacity);}
						
						/** Set the current opacity.
						 * \param colValue the new opacity.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 */
			void		SetcolOpacity(const CqColor& colValue, TqFloat time=0.0f)			
																{m_colOpacity=colValue;}

						/** Add a lightsource to the current available list.
						 * \param pL a pointer to the new lightsource.
						 */
			void		AddLightsource(CqLightsource* pL)
												{
													// Check if the ligthsource is already active
													TqInt i;
													for(i=0; i<m_apLightsources.size(); i++)
													{
														if(m_apLightsources[i]==pL)
															return;
													}
													m_apLightsources.push_back(pL);
												}
						/** Remove a lightsource from the current available list.
						 * \param pL a pointer to the lightsource to remove.
						 */
			void		RemoveLightsource(CqLightsource* pL)
												{
													// Check if the ligthsource is in the active list.
													for(std::vector<CqLightsource*>::iterator i=m_apLightsources.begin(); i!=m_apLightsources.end(); i++)
													{
														if(*i==pL)
														{
															m_apLightsources.erase(i);
															return;
														}
													}
												}
						/** Get a reference to the lightsource list.
						 * \return a reference to the vector of lightsource pointers.
						 */
	const	std::vector<CqLightsource*>&	apLights()	const {return(m_apLightsources);}

	protected:
			CqColor	m_colColor;						///< the current color attribute.
			CqColor	m_colOpacity;						///< the current opacity attribute.
			CqVector2D	m_aTextureCoordinates[4];			///< an array of 2D vectors representing the coordinate space.
			std::vector<CqLightsource*> m_apLightsources;	///< a vector of currently available lightsources.
			CqShader*	m_pshadAreaLightSource;				///< a pointer to the current area ligthsource shader.
			CqShader*	m_pshadSurface;						///< a pointer to the current surface shader.
			CqShader*	m_pshadAtmosphere;					///< a pointer to the current atmosphere shader.
			CqShader*	m_pshadInteriorVolume;				///< a pointer to the current interior shader.
			CqShader*	m_pshadExteriorVolume;				///< a pointer to the current exterior shader.
			TqFloat		m_fEffectiveShadingRate;			///< the current effective shading rate.
			ShadingInterpolation m_eShadingInterpolation;	///< the current shading interpolation mode.
			TqBool		m_bMatteSurfaceFlag;				///< the current state of the matte flag.
};


//-----------------------------------------------------------------------
/**
	Class containing the current status of the attributes related geometry state
*/

class CqGeometricAttributes
{
	public:
						/// Dfault contructor
						CqGeometricAttributes():	m_Bound(-FLT_MAX,-FLT_MAX,-FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX),
													m_fDetailRangeMinVisible(0.0),
													m_fDetailRangeLowerTransition(0.0),
													m_fDetailRangeUpperTransition(FLT_MAX),
													m_fDetailRangeMaxVisible(FLT_MAX),
													m_matuBasis(RiBezierBasis),
													m_matvBasis(RiBezierBasis),
													m_uSteps(3),
													m_vSteps(3),
													m_eOrientation(OrientationLH),
													m_eCoordsysOrientation(OrientationLH),
													m_iNumberOfSides(2),
													m_pshadDisplacement(0)
													{}
	virtual				~CqGeometricAttributes()	{}

			
						/** Get the current geometric bound.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 * \return a reference to the bound.
						 */
	const	CqBound&	Bound(TqFloat time=0.0f) const			
													{return(m_Bound);}
						/** Set the current geometric bound.
						 * \param bndValue the new geometric bound of any primitives.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 */
			void		SetBound(const CqBound& bndValue, TqFloat time=0.0f)	
													{m_Bound=bndValue;}
						/** Get the current value for the detail range minimum visible distance.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 * \return float minimum detail visble distance.
						 */
			TqFloat		fDetailRangeMinVisible(TqFloat time=0.0f) const
													{return(m_fDetailRangeMinVisible);}
						/** Set the current detail range minimum visible distance.
						 * \param fValue the new minimum distance.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 */
			void		SetfDetailRangeMinVisible(const TqFloat fValue,TqFloat time=0.0f)
													{m_fDetailRangeMinVisible=fValue;}
						/** Get the current value for the detail range lower transition distance.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 * \return float lower transition distance.
						 */
			TqFloat		fDetailRangeLowerTransition(TqFloat time=0.0f) const
													{return(m_fDetailRangeLowerTransition);}
						/** Set the current detail range lower transition distance.
						 * \param fValue the new value for the lower transition distance.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 */
			void		SetfDetailRangeLowerTransition(const TqFloat fValue,TqFloat time=0.0f)
													{m_fDetailRangeLowerTransition=fValue;}
						/** Get the current value for the detail range upper transition distance.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 * \return float upper transition distance.
						 */
			TqFloat		fDetailRangeUpperTransition(TqFloat time=0.0f) const
													{return(m_fDetailRangeUpperTransition);}
						/** Set the current detail range upper transition distance.
						 * \param fValue the new value for the upper transition distance.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 */
			void		SetfDetailRangeUpperTransition(const TqFloat fValue,TqFloat time=0.0f)
													{m_fDetailRangeUpperTransition=fValue;}
						/** Get the current value for the detail range maximum visible distance.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 * \return float maximum detail visble distance.
						 */
			TqFloat		fDetailRangeMaxVisible(TqFloat time=0.0f) const
													{return(m_fDetailRangeMaxVisible);}
						/** Set the current detail range maximum distance.
						 * \param fValue the new detail ramge maximum distance.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 */
			void		SetfDetailRangeMaxVisible(const TqFloat fValue,TqFloat time=0.0f)
													{m_fDetailRangeMaxVisible=fValue;}
						/** Get the current basis matrix for the u direction.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 * \return a reference to the basis matrix.
						 */
	const	CqMatrix&	matuBasis(TqFloat time=0.0f) const  
													{return(m_matuBasis);}
						/** Get the current basis matrix for the v direction.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 * \return a reference to the basis matrix.
						 */
	const	CqMatrix&	matvBasis(TqFloat time=0.0f) const 
													{return(m_matvBasis);}
						/** Set the current basis matrix for the u direction.
						 * \param u the new basis matrix for the u direction.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 */
			void		SetmatuBasis(CqMatrix& u,TqFloat time=0.0f)	
													{m_matuBasis=u;}
						/** Set the current basis matrix for the v direction.
						 * \param u the new basis matrix for the v direction.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 */
			void		SetmatvBasis(CqMatrix& v,TqFloat time=0.0f)	
													{m_matvBasis=v;}
						/** Get the number of steps to advance the evaluation window in the u direction.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 * \return an integer steps count.
						 */
			TqInt		uSteps(TqFloat time=0.0f) const				
													{return(m_uSteps);}
						/** Set the current number of steps to advance the evaluation window in the u direction.
						 * \param u the new steps value for the u direction.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 */
			void		SetuSteps(const TqInt u,TqFloat time=0.0f)	
													{m_uSteps=u;}
						/** Get the number of steps to advance the evaluation window in the v direction.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 * \return an integer steps count.
						 */
			TqInt		vSteps(TqFloat time=0.0f) const				
													{return(m_vSteps);}
						/** Set the current number of steps to advance the evaluation window in the v direction.
						 * \param u the new steps value for the v direction.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 */
			void		SetvSteps(const TqInt v,TqFloat time=0.0f)	
													{m_vSteps=v;}
						/** Get the current value of the orientation, in whichprimitives are described.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 * \return the orientation value as an enum.
						 */
			EqOrientation eOrientation(TqFloat time=0.0f) const		
													{return(m_eOrientation);}
						/** Set the current orientation in which primitives are described.
						 * \param eValue the new orientation.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 */
			void		SeteOrientation(const EqOrientation eValue,TqFloat time=0.0f)
													{m_eOrientation=eValue;}
						/** Flip the orientation in which primitives are described between left and right handed.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 */
			void		FlipeOrientation(TqFloat time=0.0f)			
													{m_eOrientation=(m_eOrientation==OrientationLH)?OrientationRH:OrientationLH;}
						/** Get the current orientation of the coordinate system, affected by transforms.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 * \return the orientation as an enum.
						 */
			EqOrientation eCoordsysOrientation(TqFloat time=0.0f) const
													{return(m_eCoordsysOrientation);}
						/** Set the current coordinate system orientation.
						 * \param eValue the new orientation.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 */
			void		SeteCoordsysOrientation(const EqOrientation eValue,TqFloat time=0.0f)
													{m_eCoordsysOrientation=eValue;}
						/** Flip the orientation of the coordinate system between left and right handed.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 */
			void		FlipeCoordsysOrientation(TqFloat time=0.0f)	
													{m_eCoordsysOrientation=(m_eCoordsysOrientation==OrientationLH)?OrientationRH:OrientationLH;}
						/** Get the number of visible sides any primitives have.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 * \return the sides count, 1 or 2.
						 */
			TqInt		iNumberOfSides(TqFloat time=0.0f) const		
													{return(m_iNumberOfSides);}
						/** Set the number of visible sides primitives have.
						 * \param iValue the new sides count.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 */
			void		SetiNumberOfSides(const TqInt iValue,TqFloat time=0.0f)
													{m_iNumberOfSides=iValue;}
						/** Get the current dislacement shader.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 * \return a pointer to the displacement shader.
						 */
			CqShader*	pshadDisplacement(TqFloat time=0.0f) const	
													{return(m_pshadDisplacement);}
						/** Set the current displacement shader.
						 * \param pshadAtmosphere a pointer to a shader to use as the displacement shader.
						 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
						 */
			void		SetpshadDisplacement(CqShader* pshadDisplacement,TqFloat time=0.0f)
												{m_pshadDisplacement=pshadDisplacement;}

	protected:
			CqBound		m_Bound;							///< the bound used for any associated primitives.
			TqFloat		m_fDetailRangeMinVisible,			///< the detail range minimum visible distance.
						m_fDetailRangeLowerTransition,		///< the detail range lower transition distance.
						m_fDetailRangeUpperTransition,		///< the detail range upper transition distance. 
						m_fDetailRangeMaxVisible;			///< the detail range maximum visible distance.
			CqMatrix	m_matuBasis,						///< the basis matrix for the u direction.
						m_matvBasis;						///< the basis matrix for the v direction.
			TqInt		m_uSteps,							///< the steps to advance the evaluation window in the u direction. 
						m_vSteps;							///< the steps to advance the evaluation window in the v direction.
			EqOrientation m_eOrientation;					///< the orientation associated primitives are described in.
			EqOrientation	m_eCoordsysOrientation;			///< the orientation of the current coordinate system.
			TqInt		m_iNumberOfSides;					///< the number of visible sides associated primitives have.
			CqShader*	m_pshadDisplacement;				///< a pointer to the current displacement shader.
};


//----------------------------------------------------------------------
/**
	Container class for the attributes definitions of the graphics state.
*/

class _qShareC	CqAttributes : public CqShadingAttributes, public CqGeometricAttributes
{
	public:
	_qShareM			CqAttributes();
	_qShareM			CqAttributes(const CqAttributes& From);
	_qShareM	virtual	~CqAttributes();

						/// increment the number of external references to this attribute state.
	_qShareM	void	Reference()			{m_cReferences++;}
						/// decrement the number of external references to theses attributes, destroy them if no more references.
	_qShareM	void	UnReference()		{m_cReferences--; if(m_cReferences==0)	delete(this);}
						/** Get a pointer to this attribute state suitable for writing.
						 * I the external references count is greater than 1, then create a copy on the stack and return that.
						 * \return a pointer to these attribute safe to write into.
						 */
	_qShareM	CqAttributes* Write()		{
												// We are about to write to this attribute,so clone if references exist.
												if(m_cReferences>1)
												{
													CqAttributes* pWrite=Clone();
													pWrite->Reference();
													UnReference();
													return(pWrite);
												}
												else
													return(this);
											}

	_qShareM	CqAttributes& operator=(const CqAttributes& From);

						/** Add a new user defined attribute.
						 * \param pAttribute a pointer to the new user defined attribute.
						 */
	_qShareM	void	AddAttribute(CqSystemOption* pAttribute)
											{m_aAttributes.push_back(pAttribute);}
						/** Get a pointer to a named user defined attribute.
						 * \param straName the name of the attribute to retrieve.
						 * \return a pointer to the attribute or 0 if not found.
						 */
	const	CqSystemOption* pAttribute(const char* strName) const
											{
												TqInt i;
												for(i=0; i<m_aAttributes.size(); i++)
													if(m_aAttributes[i]->strName().compare(strName)==0)	return(m_aAttributes[i]);
												return(0);
											}
						/** Get a pointer to a named user defined attribute suitable for writing.
						 * If the attribute has more than 1 external reference, create a duplicate an return that.
						 * \attention If the attribute does not exist in the list, one will automatically be created and added.
						 * \param straName the name of the attribute to retrieve.
						 * \return a pointer to the attribute.
						 */
			CqSystemOption* pAttributeWrite(const char* strName)
											{
												TqInt i;
												for(i=0; i<m_aAttributes.size(); i++)
												{
													if(m_aAttributes[i]->strName().compare(strName)==0)
													{
														if(m_aAttributes[i]->Refcount()==1)
															return(m_aAttributes[i]);
														else
														{
															m_aAttributes[i]->UnReference();
															m_aAttributes[i]=new CqSystemOption(*(m_aAttributes[i]));
															m_aAttributes[i]->Reference();
															return(m_aAttributes[i]);
														}
													}
												}
												m_aAttributes.push_back(new CqSystemOption(strName));
												m_aAttributes.back()->Reference();
												return(m_aAttributes.back());
											}

						/** Clone the entire attribute state.
						 * \return a pointer to the new attribute state.
						 */
			CqAttributes*	Clone() const	{return(new CqAttributes(*this));}

	const	CqParameter* pParameter(const char* strName, const char* strParam) const;
			CqParameter* pParameterWrite(const char* strName, const char* strParam);
			
	const	TqFloat*	GetFloatAttribute(const char* strName, const char* strParam) const;
	const	TqInt*		GetIntegerAttribute(const char* strName, const char* strParam) const;
	const	CqString* GetStringAttribute(const char* strName, const char* strParam) const;
	const	CqVector3D*	GetPointAttribute(const char* strName, const char* strParam) const;
	const	CqColor*	GetColorAttribute(const char* strName, const char* strParam) const;

			TqFloat*	GetFloatAttributeWrite(const char* strName, const char* strParam);
			TqInt*		GetIntegerAttributeWrite(const char* strName, const char* strParam);
			CqString* GetStringAttributeWrite(const char* strName, const char* strParam);
			CqVector3D*	GetPointAttributeWrite(const char* strName, const char* strParam);
			CqColor*	GetColorAttributeWrite(const char* strName, const char* strParam);

	private:
			std::vector<CqSystemOption*>	m_aAttributes;		///< a vector of user defined attribute pointers.
			TqInt		m_cReferences;							///< the count of external references to this attribute state.
			TqInt		m_StackIndex;							///< the index of this attribute state in the global stack, used for destroying when last reference is removed.
};

extern std::vector<CqAttributes*>	Attribute_stack;


//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

//}  // End of #ifdef ATTRIBUTES_H_INCLUDED
#endif
