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

#include	"aqsis.h"

#include	"color.h"
#include	"ri.h"
#include	"matrix.h"
#include	"options.h"
#include	"bound.h"
#include	"spline.h"
#include	"trimcurve.h"
#include	"iattributes.h"

#define		_qShareName	CORE
#include	"share.h"

START_NAMESPACE( Aqsis )
struct IqShader;

/// The options for the orientation of a coordinate system.
enum EqOrientation
{
    OrientationLH,   	///< Left hand coordinate system.
    OrientationRH,   	///< Right and coordinate system.
};

enum	ShadingInterpolation
{
	ShadingConstant,   	///< use constant shading, i.e. one value per micropoly.
	ShadingSmooth,   		///< use smooth shading, i.e. interpolate the values at the corners of a micropoly.
};

class CqLightsource;

//----------------------------------------------------------------------
/**
	Container class for the attributes definitions of the graphics state.
*/


class _qShareC	CqAttributes : public CqRefCount, public IqAttributes
{
	public:
		_qShareM	CqAttributes();
		_qShareM	CqAttributes( const CqAttributes& From );
		_qShareM	virtual	~CqAttributes();

		/** Get a pointer to this attribute state suitable for writing.
		 * I the external references count is greater than 1, then create a copy on the stack and return that.
		 * \return a pointer to these attribute safe to write into.
		 */
		_qShareM	CqAttributes* Write()
		{
			// We are about to write to this attribute,so clone if references exist.
			if ( RefCount() > 1 )
			{
				CqAttributes * pWrite = Clone();
				pWrite->AddRef();
				Release();
				return ( pWrite );
			}
			else
				return ( this );
		}

		_qShareM	CqAttributes& operator=( const CqAttributes& From );

		/** Add a new user defined attribute.
		 * \param pAttribute a pointer to the new user defined attribute.
		 */
		_qShareM	void	AddAttribute( CqSystemOption* pAttribute )
		{
			m_aAttributes.push_back( pAttribute );
		}
		/** Get a pointer to a named user defined attribute.
		 * \param straName the name of the attribute to retrieve.
		 * \return a pointer to the attribute or 0 if not found.
		 */
		const	CqSystemOption* pAttribute( const char* strName ) const
		{
			for ( std::vector<CqSystemOption*>::const_iterator i = m_aAttributes.begin(); i != m_aAttributes.end(); i++ )
				if ( ( *i ) ->strName().compare( strName ) == 0 ) return ( *i );
			return ( 0 );
		}
		/** Get a pointer to a named user defined attribute suitable for writing.
		 * If the attribute has more than 1 external reference, create a duplicate an return that.
		 * \attention If the attribute does not exist in the list, one will automatically be created and added.
		 * \param straName the name of the attribute to retrieve.
		 * \return a pointer to the attribute.
		 */
		CqSystemOption* pAttributeWrite( const char* strName )
		{
			for ( std::vector<CqSystemOption*>::iterator i = m_aAttributes.begin(); i != m_aAttributes.end(); i++ )
			{
				if ( ( *i ) ->strName().compare( strName ) == 0 )
				{
					if ( ( *i ) ->RefCount() == 1 )
						return ( *i );
					else
					{
						( *i ) ->Release();
						( *i ) = new CqSystemOption( *( *i ) );
						( *i ) ->AddRef();
						return ( *i );
					}
				}
			}
			m_aAttributes.push_back( new CqSystemOption( strName ) );
			m_aAttributes.back() ->AddRef();
			return ( m_aAttributes.back() );
		}

		/** Add a lightsource to the current available list.
		 * \param pL a pointer to the new lightsource.
		 */
		void	AddLightsource( CqLightsource* pL )
		{
			// Check if the ligthsource is already active
			for ( std::vector<CqLightsource*>::iterator i = m_apLightsources.begin(); i != m_apLightsources.end(); i++ )
			{
				if ( ( *i ) == pL )
					return ;
			}
			m_apLightsources.push_back( pL );
		}
		/** Remove a lightsource from the current available list.
		 * \param pL a pointer to the lightsource to remove.
		 */
		void	RemoveLightsource( CqLightsource* pL )
		{
			// Check if the ligthsource is in the active list.
			for ( std::vector<CqLightsource*>::iterator i = m_apLightsources.begin(); i != m_apLightsources.end(); i++ )
			{
				if ( *i == pL )
				{
					m_apLightsources.erase( i );
					return ;
				}
			}
		}
		/** Get a reference to the lightsource list.
		 * \return a reference to the vector of lightsource pointers.
		 */
		const	std::vector<CqLightsource*>&	apLights() const
		{
			return ( m_apLightsources );
		}

		/** Get the current geometric bound.
		 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
		 * \return a reference to the bound.
		 */
		const	CqBound&	Bound( TqFloat time = 0.0f ) const
		{
			return ( m_Bound );
		}
		/** Set the current geometric bound.
		 * \param bndValue the new geometric bound of any primitives.
		 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
		 */
		void	SetBound( const CqBound& bndValue, TqFloat time = 0.0f )
		{
			m_Bound = bndValue;
		}

		/** Flip the orientation in which primitives are described between left and right handed.
		 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
		 */
		void	FlipeOrientation( TqFloat time = 0.0f )
		{
			TqInt co = GetIntegerAttribute("System", "Orientation")[0];
			GetIntegerAttributeWrite("System", "Orientation")[0] = ( co == OrientationLH ) ? OrientationRH : OrientationLH;
		}

		/** Flip the orientation of the coordinate system between left and right handed.
		 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
		 */
		void	FlipeCoordsysOrientation( TqFloat time = 0.0f )
		{
			TqInt co = GetIntegerAttribute("System", "Orientation")[1];
			GetIntegerAttributeWrite("System", "Orientation")[1] = ( co == OrientationLH ) ? OrientationRH : OrientationLH;
		}

		/** Get the current dislacement shader.
		 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
		 * \return a pointer to the displacement shader.
		 */
		IqShader*	pshadDisplacement( TqFloat time = 0.0f ) const
		{
			return ( m_pshadDisplacement );
		}
		/** Set the current displacement shader.
		 * \param pshadAtmosphere a pointer to a shader to use as the displacement shader.
		 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
		 */
		void	SetpshadDisplacement( IqShader* pshadDisplacement, TqFloat time = 0.0f )
		{
			m_pshadDisplacement = pshadDisplacement;
		}

		/** Get the current area light source shader.
		 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
		 * \return a pointer to the displacement shader.
		 */
		IqShader*	pshadAreaLightSource( TqFloat time = 0.0f ) const
		{
			return ( m_pshadAreaLightSource );
		}
		/** Set the current area light source shader.
		 * \param pshadAtmosphere a pointer to a shader to use as the displacement shader.
		 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
		 */
		void	SetpshadAreaLightSource( IqShader* pshadAreaLightSource, TqFloat time = 0.0f )
		{
			m_pshadAreaLightSource = pshadAreaLightSource;
		}

		/** Get the current surface shader.
		 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
		 * \return a pointer to the displacement shader.
		 */
		IqShader*	pshadSurface( TqFloat time = 0.0f ) const
		{
			return ( m_pshadSurface );
		}
		/** Set the current surface shader.
		 * \param pshadAtmosphere a pointer to a shader to use as the displacement shader.
		 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
		 */
		void	SetpshadSurface( IqShader* pshadSurface, TqFloat time = 0.0f )
		{
			m_pshadSurface = pshadSurface;
		}

		/** Get the current atmosphere shader.
		 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
		 * \return a pointer to the displacement shader.
		 */
		IqShader*	pshadAtmosphere( TqFloat time = 0.0f ) const
		{
			return ( m_pshadAtmosphere );
		}
		/** Set the current atmosphere shader.
		 * \param pshadAtmosphere a pointer to a shader to use as the displacement shader.
		 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
		 */
		void	SetpshadAtmosphere( IqShader* pshadAtmosphere, TqFloat time = 0.0f )
		{
			m_pshadAtmosphere = pshadAtmosphere;
		}

		/** Get the current external volume shader.
		 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
		 * \return a pointer to the displacement shader.
		 */
		IqShader*	pshadExteriorVolume( TqFloat time = 0.0f ) const
		{
			return ( m_pshadExteriorVolume );
		}
		/** Set the current external volume shader.
		 * \param pshadAtmosphere a pointer to a shader to use as the displacement shader.
		 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
		 */
		void	SetpshadExteriorVolume( IqShader* pshadExteriorVolume, TqFloat time = 0.0f )
		{
			m_pshadExteriorVolume = pshadExteriorVolume;
		}

		/** Get the current internal volume shader.
		 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
		 * \return a pointer to the displacement shader.
		 */
		IqShader*	pshadAreaInteriorVolume( TqFloat time = 0.0f ) const
		{
			return ( m_pshadInteriorVolume );
		}
		/** Set the current internal volume shader.
		 * \param pshadAtmosphere a pointer to a shader to use as the displacement shader.
		 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
		 */
		void	SetpshadInteriorVolume( IqShader* pshadInteriorVolume, TqFloat time = 0.0f )
		{
			m_pshadInteriorVolume = pshadInteriorVolume;
		}

		/** Get the array of trim curve loops.
		 *	\return A pointer to the trim loops array object.
		 */
		const CqTrimLoopArray& TrimLoops() const
		{
			return ( m_TrimLoops );
		}
		/** Get the array of trim curve loops.
		 *	\return A pointer to the trim loops array object.
		 */
		CqTrimLoopArray& TrimLoops()
		{
			return ( m_TrimLoops );
		}

		/** Clone the entire attribute state.
		 * \return a pointer to the new attribute state.
		 */
		CqAttributes*	Clone() const
		{
			return ( new CqAttributes( *this ) );
		}

		const	CqParameter* pParameter( const char* strName, const char* strParam ) const;
		CqParameter* pParameterWrite( const char* strName, const char* strParam );

		const	TqFloat*	GetFloatAttribute( const char* strName, const char* strParam ) const;
		const	TqInt*	GetIntegerAttribute( const char* strName, const char* strParam ) const;
		const	CqString* GetStringAttribute( const char* strName, const char* strParam ) const;
		const	CqVector3D*	GetPointAttribute( const char* strName, const char* strParam ) const;
		const	CqColor*	GetColorAttribute( const char* strName, const char* strParam ) const;
		const	CqMatrix*	GetMatrixAttribute( const char* strName, const char* strParam ) const;

		TqFloat*	GetFloatAttributeWrite( const char* strName, const char* strParam );
		TqInt*	GetIntegerAttributeWrite( const char* strName, const char* strParam );
		CqString* GetStringAttributeWrite( const char* strName, const char* strParam );
		CqVector3D*	GetPointAttributeWrite( const char* strName, const char* strParam );
		CqColor*	GetColorAttributeWrite( const char* strName, const char* strParam );
		CqMatrix*	GetMatrixAttributeWrite( const char* strName, const char* strParam );

	private:
		std::vector<CqSystemOption*>	m_aAttributes;		///< a vector of user defined attribute pointers.

		CqBound	m_Bound;							///< the bound used for any associated primitives.
		IqShader*	m_pshadDisplacement;				///< a pointer to the current displacement shader.
		CqTrimLoopArray m_TrimLoops;					///< the array of closed trimcurve loops.
		TqInt	m_StackIndex;							///< the index of this attribute state in the global stack, used for destroying when last reference is removed.
		std::vector<CqLightsource*> m_apLightsources;	///< a vector of currently available lightsources.
		IqShader*	m_pshadAreaLightSource;				///< a pointer to the current area ligthsource shader.
		IqShader*	m_pshadSurface;						///< a pointer to the current surface shader.
		IqShader*	m_pshadAtmosphere;					///< a pointer to the current atmosphere shader.
		IqShader*	m_pshadInteriorVolume;				///< a pointer to the current interior shader.
		IqShader*	m_pshadExteriorVolume;				///< a pointer to the current exterior shader.
}
;

extern std::vector<CqAttributes*>	Attribute_stack;


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

//}  // End of #ifdef ATTRIBUTES_H_INCLUDED
#endif
