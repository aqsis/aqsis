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
#include	<list>

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
class	CqLightsource;


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
		_qShareM	void	AddAttribute( CqNamedParameterList* pAttribute )
		{
			m_aAttributes.Add( pAttribute );
		}
		/** Get a pointer to a named user defined attribute.
		 * \param strName the name of the attribute to retrieve.
		 * \return a pointer to the attribute or 0 if not found.
		 */
		const	CqNamedParameterList* pAttribute( const char* strName ) const
		{
			return ( m_aAttributes.Find( strName ) );
		}
		/** Get a pointer to a named user defined attribute suitable for writing.
		 * If the attribute has more than 1 external reference, create a duplicate an return that.
		 * \attention If the attribute does not exist in the list, one will automatically be created and added.
		 * \param strName the name of the attribute to retrieve.
		 * \return a pointer to the attribute.
		 */
		CqNamedParameterList* pAttributeWrite( const char* strName )
		{
			CqNamedParameterList* pAttr = m_aAttributes.Find( strName );
			if ( NULL != pAttr )
			{
				if ( pAttr->RefCount() == 1 )
					return ( pAttr );
				else
				{
					CqNamedParameterList* pNew = new CqNamedParameterList( *pAttr );
					m_aAttributes.Remove( pAttr );
					m_aAttributes.Add( pNew );
					pNew->AddRef();
					return ( pNew );
				}
			}
			CqNamedParameterList* pNew = new CqNamedParameterList( strName );
			m_aAttributes.Add( pNew );
			return ( pNew );
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
		virtual const	std::vector<CqLightsource*>&	apLights() const
		{
			return ( m_apLightsources );
		}

		/** Flip the orientation in which primitives are described between left and right handed.
		 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
		 */
		void	FlipeOrientation( TqFloat time = 0.0f )
		{
			TqInt co = GetIntegerAttribute( "System", "Orientation" ) [ 0 ];
			GetIntegerAttributeWrite( "System", "Orientation" ) [ 0 ] = ( co == OrientationLH ) ? OrientationRH : OrientationLH;
		}

		/** Flip the orientation of the coordinate system between left and right handed.
		 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
		 */
		void	FlipeCoordsysOrientation( TqFloat time = 0.0f )
		{
			TqInt co = GetIntegerAttribute( "System", "Orientation" ) [ 1 ];
			GetIntegerAttributeWrite( "System", "Orientation" ) [ 1 ] = ( co == OrientationLH ) ? OrientationRH : OrientationLH;
		}

		virtual IqShader*	pshadDisplacement( TqFloat time = 0.0f ) const
		{
			return ( m_pshadDisplacement );
		}
		virtual void	SetpshadDisplacement( IqShader* pshadDisplacement, TqFloat time = 0.0f )
		{
			m_pshadDisplacement = pshadDisplacement;
		}
		virtual IqShader*	pshadAreaLightSource( TqFloat time = 0.0f ) const
		{
			return ( m_pshadAreaLightSource );
		}
		virtual void	SetpshadAreaLightSource( IqShader* pshadAreaLightSource, TqFloat time = 0.0f )
		{
			m_pshadAreaLightSource = pshadAreaLightSource;
		}
		virtual IqShader*	pshadSurface( TqFloat time = 0.0f ) const
		{
			return ( m_pshadSurface );
		}
		virtual void	SetpshadSurface( IqShader* pshadSurface, TqFloat time = 0.0f )
		{
			m_pshadSurface = pshadSurface;
		}
		virtual IqShader*	pshadAtmosphere( TqFloat time = 0.0f ) const
		{
			return ( m_pshadAtmosphere );
		}
		virtual void	SetpshadAtmosphere( IqShader* pshadAtmosphere, TqFloat time = 0.0f )
		{
			m_pshadAtmosphere = pshadAtmosphere;
		}
		virtual IqShader*	pshadExteriorVolume( TqFloat time = 0.0f ) const
		{
			return ( m_pshadExteriorVolume );
		}
		virtual void	SetpshadExteriorVolume( IqShader* pshadExteriorVolume, TqFloat time = 0.0f )
		{
			m_pshadExteriorVolume = pshadExteriorVolume;
		}
		virtual IqShader*	pshadAreaInteriorVolume( TqFloat time = 0.0f ) const
		{
			return ( m_pshadInteriorVolume );
		}
		virtual void	SetpshadInteriorVolume( IqShader* pshadInteriorVolume, TqFloat time = 0.0f )
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

		virtual const	TqFloat*	GetFloatAttribute( const char* strName, const char* strParam ) const;
		virtual const	TqInt*	GetIntegerAttribute( const char* strName, const char* strParam ) const;
		virtual const	CqString* GetStringAttribute( const char* strName, const char* strParam ) const;
		virtual const	CqVector3D*	GetPointAttribute( const char* strName, const char* strParam ) const;
		virtual const	CqVector3D*	GetVectorAttribute( const char* strName, const char* strParam ) const;
		virtual const	CqVector3D*	GetNormalAttribute( const char* strName, const char* strParam ) const;
		virtual const	CqColor*	GetColorAttribute( const char* strName, const char* strParam ) const;
		virtual const	CqMatrix*	GetMatrixAttribute( const char* strName, const char* strParam ) const;

		virtual TqFloat*	GetFloatAttributeWrite( const char* strName, const char* strParam );
		virtual TqInt*	GetIntegerAttributeWrite( const char* strName, const char* strParam );
		virtual CqString* GetStringAttributeWrite( const char* strName, const char* strParam );
		virtual CqVector3D*	GetPointAttributeWrite( const char* strName, const char* strParam );
		virtual CqVector3D*	GetVectorAttributeWrite( const char* strName, const char* strParam );
		virtual CqVector3D*	GetNormalAttributeWrite( const char* strName, const char* strParam );
		virtual CqColor*	GetColorAttributeWrite( const char* strName, const char* strParam );
		virtual CqMatrix*	GetMatrixAttributeWrite( const char* strName, const char* strParam );

		virtual	TqInt	cLights() const
		{
			return ( apLights().size() );
		}
		virtual	IqLightsource*	pLight( TqInt index );

		virtual	void	Release()
		{
			CqRefCount::Release();
		}
		virtual	void	AddRef()
		{
			CqRefCount::AddRef();
		}

	private:
		class CqHashTable
		{
			private:
				static const TqInt tableSize;

			public:
				CqHashTable()
				{
					m_aLists.resize( tableSize );
				}
				virtual	~CqHashTable()
				{
					// Release all the system options we have a handle on.
					std::vector<std::list<CqNamedParameterList*> >::iterator i;
					for ( i = m_aLists.begin(); i != m_aLists.end(); i++ )
					{
						std::list<CqNamedParameterList*>::iterator i2;
						for ( i2 = ( *i ).begin(); i2 != ( *i ).end(); i2++ )
							( *i2 ) ->Release();
					}
				}

				const CqNamedParameterList*	Find( const TqChar* pname ) const
				{
					TqInt i = _hash( pname );

					if ( m_aLists[ i ].empty() )
						return ( 0 );

					std::list<CqNamedParameterList*>::const_iterator iEntry = m_aLists[ i ].begin();
					if ( iEntry == m_aLists[ i ].end() )
						return ( *iEntry );
					else
					{
						while ( iEntry != m_aLists[ i ].end() )
						{
							if ( ( *iEntry ) ->strName().compare( pname ) == 0 )
								return ( *iEntry );
							iEntry++;
						}
					}

					return ( 0 );
				}

				CqNamedParameterList*	Find( const TqChar* pname )
				{
					TqInt i = _hash( pname );

					if ( m_aLists[ i ].empty() )
						return ( 0 );

					std::list<CqNamedParameterList*>::const_iterator iEntry = m_aLists[ i ].begin();
					if ( iEntry == m_aLists[ i ].end() )
						return ( *iEntry );
					else
					{
						while ( iEntry != m_aLists[ i ].end() )
						{
							if ( ( *iEntry ) ->strName().compare( pname ) == 0 )
								return ( *iEntry );
							iEntry++;
						}
					}

					return ( 0 );
				}

				void Add( CqNamedParameterList* pOption )
				{
					TqInt i = _hash( pOption->strName().c_str() );
					m_aLists[ i ].push_back( pOption );
					pOption->AddRef();
				}

				void Remove( CqNamedParameterList* pOption )
				{
					TqInt i = _hash( pOption->strName().c_str() );

					std::list<CqNamedParameterList*>::iterator iEntry = m_aLists[ i ].begin();
					while ( iEntry != m_aLists[ i ].end() )
					{
						if ( ( *iEntry ) == pOption )
						{
							pOption->Release();
							m_aLists[ i ].remove( *iEntry );
							return ;
						}
						iEntry++;
					}
				}

				CqHashTable& operator=( const CqHashTable& From )
				{
					std::vector<std::list<CqNamedParameterList*> >::const_iterator i;
					for ( i = From.m_aLists.begin(); i != From.m_aLists.end(); i++ )
					{
						std::list<CqNamedParameterList*>::const_iterator i2;
						for ( i2 = ( *i ).begin(); i2 != ( *i ).end(); i2++ )
							Add( *i2 );
					}
					return ( *this );
				}

			private:
				TqInt _hash( const TqChar* string ) const
				{
					assert ( string != 0 && string[ 0 ] != 0 );

					TqUchar h = string[ 0 ];
					for ( TqInt i = 1; string[ i ] != 0; ++i )
						h = ( h << 4 ) + string[ i ];

					return ( h % tableSize ); // remainder
				}

				std::vector<std::list<CqNamedParameterList*> >	m_aLists;
		};

		CqHashTable	m_aAttributes;						///< a vector of user defined attribute pointers.

		IqShader*	m_pshadDisplacement;				///< a pointer to the current displacement shader.
		IqShader*	m_pshadAreaLightSource;				///< a pointer to the current area ligthsource shader.
		IqShader*	m_pshadSurface;						///< a pointer to the current surface shader.
		IqShader*	m_pshadAtmosphere;					///< a pointer to the current atmosphere shader.
		IqShader*	m_pshadInteriorVolume;				///< a pointer to the current interior shader.
		IqShader*	m_pshadExteriorVolume;				///< a pointer to the current exterior shader.

		CqTrimLoopArray m_TrimLoops;					///< the array of closed trimcurve loops.
		std::vector<CqLightsource*> m_apLightsources;	///< a vector of currently available lightsources.

		TqInt	m_StackIndex;							///< the index of this attribute state in the global stack, used for destroying when last reference is removed.
}
;

/// Global attribute stack.
extern std::vector<CqAttributes*>	Attribute_stack;


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

//}  // End of #ifdef ATTRIBUTES_H_INCLUDED
#endif
