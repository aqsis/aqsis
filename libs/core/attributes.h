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
		\brief Declares the CqAttributes class for handling RenderMan attributes.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is attributes.h included already?
#ifndef ATTRIBUTES_H_INCLUDED
//{
#define ATTRIBUTES_H_INCLUDED 1

#include	<vector>
#include	<list>
#include	<map>

#include	<boost/weak_ptr.hpp>
#include	<boost/enable_shared_from_this.hpp>

#include	<aqsis/aqsis.h>

#include	<aqsis/math/color.h>
#include	<aqsis/ri/ri.h>
#include	<aqsis/math/matrix.h>
#include	"options.h"
#include	<aqsis/math/spline.h>
#include	"trimcurve.h"
#include	<aqsis/core/iattributes.h>

namespace Aqsis {
struct IqShader;
class	CqLightsource;

class CqAttributes;
typedef boost::shared_ptr<CqAttributes> CqAttributesPtr;
typedef boost::shared_ptr<const CqAttributes> CqConstAttributesPtr;

//----------------------------------------------------------------------
/**
	Container class for the attributes definitions of the graphics state.
*/


class CqAttributes : public IqAttributes, public boost::enable_shared_from_this<CqAttributes>
{
	public:
		CqAttributes();
		CqAttributes( const CqAttributes& From );
		virtual	~CqAttributes();

		/** Get a pointer to this attribute state suitable for writing.
		 * I the external references count is greater than 1, then create a copy on the stack and return that.
		 * \return a pointer to these attribute safe to write into.
		 */
		CqAttributesPtr Write()
		{
			// We are about to write to this attribute,so clone if references exist.
			CqAttributesPtr pWrite(shared_from_this());
			if ( pWrite.use_count() > 2 )
				pWrite = Clone();
			return ( pWrite );
		}

		CqAttributes& operator=( const CqAttributes& From );

		/** Add a new user defined attribute.
		 * \param pAttribute a pointer to the new user defined attribute.
		 */
		void	AddAttribute( const boost::shared_ptr<CqNamedParameterList>& pAttribute )
		{
			m_aAttributes.Add( pAttribute );
		}
		/** Get a pointer to a named user defined attribute.
		 * \param strName the name of the attribute to retrieve.
		 * \return a pointer to the attribute or 0 if not found.
		 */
		const	boost::shared_ptr<CqNamedParameterList> pAttribute( const char* strName ) const
		{
			return ( m_aAttributes.Find( strName ) );
		}
		/** Get a pointer to a named user defined attribute suitable for writing.
		 * If the attribute has more than 1 external reference, create a duplicate an return that.
		 * \attention If the attribute does not exist in the list, one will automatically be created and added.
		 * \param strName the name of the attribute to retrieve.
		 * \return a pointer to the attribute.
		 */
		boost::shared_ptr<CqNamedParameterList> pAttributeWrite( const char* strName )
		{
			boost::shared_ptr<CqNamedParameterList> pAttr = m_aAttributes.Find( strName );
			if ( pAttr )
			{
				if ( pAttr.use_count() <= 2 )
					return ( pAttr );
				else
				{
					boost::shared_ptr<CqNamedParameterList> pNew( new CqNamedParameterList( *pAttr ) );
					m_aAttributes.Remove( pAttr );
					m_aAttributes.Add( pNew );
					return ( pNew );
				}
			}
			boost::shared_ptr<CqNamedParameterList> pNew( new CqNamedParameterList( strName ) );
			m_aAttributes.Add( pNew );
			return ( pNew );
		}

		/** Add a lightsource to the current available list.
		 * \param pL a pointer to the new lightsource.
		 */
		void AddLightsource( const boost::shared_ptr<CqLightsource>& pL )
		{
			// Check if the ligthsource is already active
			std::vector<boost::weak_ptr<CqLightsource> >::iterator end = m_apLightsources.end();
			for ( std::vector<boost::weak_ptr<CqLightsource> >::iterator i = m_apLightsources.begin(); i != end; i++ )
			{
				if ( boost::shared_ptr<CqLightsource>(*i) == pL )
					return ;
			}
			m_apLightsources.push_back( boost::weak_ptr<CqLightsource>(pL) );
		}
		/** Remove a lightsource from the current available list.
		 * \param pL a pointer to the lightsource to remove.
		 */
		void RemoveLightsource( const boost::shared_ptr<CqLightsource>& pL )
		{
			// Check if the ligthsource is in the active list.
			std::vector<boost::weak_ptr<CqLightsource> >::iterator end = m_apLightsources.end();
			for ( std::vector<boost::weak_ptr<CqLightsource> >::iterator i = m_apLightsources.begin(); i != end; i++ )
			{
				if ( boost::shared_ptr<CqLightsource>(*i) == pL )
				{
					m_apLightsources.erase( i );
					return ;
				}
			}
		}
		/** Get a reference to the lightsource list.
		 * \return a reference to the vector of lightsource pointers.
		 */
		virtual const std::vector<boost::weak_ptr<CqLightsource> >& apLights() const
		{
			return ( m_apLightsources );
		}

		/** Flip the orientation in which primitives are described between left and right handed.
		 * \param time the frame time to get the values in the case of a motion blurred attribute. (not used).
		 */
		void	FlipeOrientation( TqFloat time )
		{
			bool co = GetIntegerAttribute( "System", "Orientation" ) [ 0 ] == 0;
			GetIntegerAttributeWrite( "System", "Orientation" ) [ 0 ] = ( co ) ? 1 : 0;
		}

		virtual boost::shared_ptr<IqShader> pshadDisplacement( TqFloat /* time */) const
		{
			return ( m_pshadDisplacement );
		}
		virtual void SetpshadDisplacement( const boost::shared_ptr<IqShader>& pshadDisplacement, TqFloat /* time */ )
		{
			m_pshadDisplacement = pshadDisplacement;
		}
		virtual boost::shared_ptr<IqShader> pshadAreaLightSource( TqFloat /* time */ ) const
		{
			return ( m_pshadAreaLightSource );
		}
		virtual void SetpshadAreaLightSource( const boost::shared_ptr<IqShader>& pshadAreaLightSource, TqFloat /* time */ )
		{
			m_pshadAreaLightSource = pshadAreaLightSource;
		}
		virtual boost::shared_ptr<IqShader> pshadSurface( TqFloat /* time */ ) const
		{
			return ( m_pshadSurface );
		}
		virtual void SetpshadSurface( const boost::shared_ptr<IqShader>& pshadSurface, TqFloat /* time */ )
		{
			m_pshadSurface = pshadSurface;
		}
		virtual boost::shared_ptr<IqShader> pshadAtmosphere( TqFloat /* time */ ) const
		{
			return ( m_pshadAtmosphere );
		}
		virtual void SetpshadAtmosphere( const boost::shared_ptr<IqShader>& pshadAtmosphere, TqFloat /* time */ )
		{
			m_pshadAtmosphere = pshadAtmosphere;
		}
		virtual boost::shared_ptr<IqShader> pshadExteriorVolume( TqFloat /* time */ ) const
		{
			return ( m_pshadExteriorVolume );
		}
		virtual void SetpshadExteriorVolume( const boost::shared_ptr<IqShader>& pshadExteriorVolume, TqFloat /* time */ )
		{
			m_pshadExteriorVolume = pshadExteriorVolume;
		}
		virtual boost::shared_ptr<IqShader> pshadAreaInteriorVolume( TqFloat /* time */ ) const
		{
			return ( m_pshadInteriorVolume );
		}
		virtual void SetpshadInteriorVolume( const boost::shared_ptr<IqShader>& pshadInteriorVolume, TqFloat /* time */ )
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
		CqAttributesPtr	Clone() const
		{
			return ( CqAttributesPtr(new CqAttributes( *this )) );
		}

		const	CqParameter* pParameter( const char* strName, const char* strParam ) const;
		CqParameter* pParameterWrite( const char* strName, const char* strParam );

		virtual const	IqParameter* GetAttribute( const char* strName, const char* strParam ) const;
		virtual IqParameter* GetAttributeWrite( const char* strName, const char* strParam );

		virtual const	TqFloat*	GetFloatAttribute( const char* strName, const char* strParam ) const;
		virtual const	TqInt*	GetIntegerAttribute( const char* strName, const char* strParam ) const;
		virtual const	CqString* GetStringAttribute( const char* strName, const char* strParam ) const;
		virtual const	CqVector3D*	GetPointAttribute( const char* strName, const char* strParam ) const;
		virtual const	CqVector3D*	GetVectorAttribute( const char* strName, const char* strParam ) const;
		virtual const	CqVector3D*	GetNormalAttribute( const char* strName, const char* strParam ) const;
		virtual const	CqColor*	GetColorAttribute( const char* strName, const char* strParam ) const;
		virtual const	CqMatrix*	GetMatrixAttribute( const char* strName, const char* strParam ) const;

		virtual const	TqInt	GetIntegerAttributeDef( const char* strName, const char* strParam, TqInt defaultVal) const;

		virtual TqFloat*	GetFloatAttributeWrite( const char* strName, const char* strParam );
		virtual TqInt*	GetIntegerAttributeWrite( const char* strName, const char* strParam );
		virtual CqString* GetStringAttributeWrite( const char* strName, const char* strParam );
		virtual CqVector3D*	GetPointAttributeWrite( const char* strName, const char* strParam );
		virtual CqVector3D*	GetVectorAttributeWrite( const char* strName, const char* strParam );
		virtual CqVector3D*	GetNormalAttributeWrite( const char* strName, const char* strParam );
		virtual CqColor*	GetColorAttributeWrite( const char* strName, const char* strParam );
		virtual CqMatrix*	GetMatrixAttributeWrite( const char* strName, const char* strParam );

		virtual	TqUint	cLights() const
		{
			return ( apLights().size() );
		}
		virtual	IqLightsource*	pLight( TqInt index ) const;

#ifdef _DEBUG
		CqString className() const
		{
			return CqString("CqAttributes");
		}
#endif

	private:
#ifdef REQUIRED

		class CqHashTable
		{
			private:
				static const TqUlong tableSize;

			public:
				CqHashTable()
				{
					m_aLists.resize( tableSize );
				}
				virtual	~CqHashTable()
				{}

				const boost::shared_ptr<CqNamedParameterList>	Find( const TqChar* pname ) const
				{
					const TqUlong hash = CqString::hash(pname);
					TqInt i = _hash( pname);

					if ( m_aLists[ i ].empty() )
					{
						boost::shared_ptr<CqNamedParameterList> retval;
						return ( retval );
					}


					std::list<boost::shared_ptr<CqNamedParameterList> >::const_iterator iEntry = m_aLists[ i ].begin();
					if ( iEntry == m_aLists[ i ].end() )
						return ( *iEntry );
					else
					{
						while ( iEntry != m_aLists[ i ].end() )
						{
							if ( ( *iEntry ) ->hash() == hash )
								return ( *iEntry );
							++iEntry;
						}
					}

					boost::shared_ptr<CqNamedParameterList> retval;
					return ( retval );
				}

				boost::shared_ptr<CqNamedParameterList>	Find( const TqChar* pname )
				{
					const TqUlong hash = CqString::hash(pname);
					TqUlong i = _hash( pname);

					if ( m_aLists[ i ].empty() )
					{
						boost::shared_ptr<CqNamedParameterList> retval;
						return ( retval );
					}


					std::list<boost::shared_ptr<CqNamedParameterList> >::const_iterator iEntry = m_aLists[ i ].begin();
					if ( iEntry == m_aLists[ i ].end() )
						return ( *iEntry );
					else
					{
						while ( iEntry != m_aLists[ i ].end() )
						{
							if ( ( *iEntry ) ->hash() == hash )
								return ( *iEntry );
							++iEntry;
						}
					}

					boost::shared_ptr<CqNamedParameterList> retval;
					return ( retval );
				}

				void Add( const boost::shared_ptr<CqNamedParameterList>& pOption )
				{
					TqUlong i = _hash( pOption->strName().c_str());
					m_aLists[ i ].push_back( pOption );
				}

				void Remove( const boost::shared_ptr<CqNamedParameterList>& pOption )
				{
					TqUlong i = _hash( pOption->strName().c_str());

					std::list<boost::shared_ptr<CqNamedParameterList> >::iterator iEntry = m_aLists[ i ].begin();
					while ( iEntry != m_aLists[ i ].end() )
					{
						if ( ( *iEntry ) == pOption )
						{
							m_aLists[ i ].remove( *iEntry );
							return ;
						}
						iEntry++;
					}
				}

				CqHashTable& operator=( const CqHashTable& From )
				{
					std::vector<std::list<boost::shared_ptr<CqNamedParameterList> > >::const_iterator i;
					for ( i = From.m_aLists.begin(); i != From.m_aLists.end(); i++ )
					{
						std::list<boost::shared_ptr<CqNamedParameterList> >::const_iterator i2;
						for ( i2 = ( *i ).begin(); i2 != ( *i ).end(); i2++ )
							Add( *i2 );
					}
					return ( *this );
				}

			private:
				TqUlong _hash( const TqChar* string ) const
				{
					TqUlong h = string[0];
					return (  h % tableSize ); // remainder
				}

				std::vector<std::list<boost::shared_ptr<CqNamedParameterList> > >	m_aLists;
		};
#else

		class CqHashTable
		{
			private:
				static const TqUlong tableSize;

				typedef	std::map<std::string, boost::shared_ptr<CqNamedParameterList>, std::less<std::string> > plist_type;
				typedef	plist_type::value_type	value_type;
				typedef	plist_type::iterator plist_iterator;
				typedef	plist_type::const_iterator plist_const_iterator;

			public:
				CqHashTable()
				{}
				virtual	~CqHashTable()
				{}

				const boost::shared_ptr<CqNamedParameterList>	Find( const TqChar* pname ) const
				{
					std::string strName( pname );
					plist_const_iterator it = m_ParameterLists.find( strName );
					if( it != m_ParameterLists.end() )
						return ( it->second );
					else
						return boost::shared_ptr<CqNamedParameterList>(static_cast<CqNamedParameterList*>(0));
				}

				boost::shared_ptr<CqNamedParameterList>	Find( const TqChar* pname )
				{
					std::string strName( pname );
					plist_iterator it = m_ParameterLists.find( strName );
					if( it != m_ParameterLists.end() )
						return ( it->second );
					else
						return boost::shared_ptr<CqNamedParameterList>(static_cast<CqNamedParameterList*>(0));
				}

				void Add( const boost::shared_ptr<CqNamedParameterList>& pOption )
				{
					m_ParameterLists.insert(value_type(pOption->strName(), pOption) );
				}

				void Remove( const boost::shared_ptr<CqNamedParameterList>& pOption )
				{
					plist_iterator it = m_ParameterLists.find( pOption->strName() );
					if( it != m_ParameterLists.end() )
					{
						m_ParameterLists.erase(it);
					}
				}

				CqHashTable& operator=( const CqHashTable& From )
				{
					plist_const_iterator it = From.m_ParameterLists.begin();
					while( it != From.m_ParameterLists.end() )
					{
						Add( (*it).second );
						++it;
					}
					return ( *this );
				}

			private:
				plist_type	m_ParameterLists;
		};
#endif

		CqHashTable	m_aAttributes;						///< a vector of user defined attribute pointers.

		boost::shared_ptr<IqShader> m_pshadDisplacement;            ///< a pointer to the current displacement shader.
		boost::shared_ptr<IqShader> m_pshadAreaLightSource;         ///< a pointer to the current area ligthsource shader.
		boost::shared_ptr<IqShader> m_pshadSurface;                 ///< a pointer to the current surface shader.
		boost::shared_ptr<IqShader> m_pshadAtmosphere;              ///< a pointer to the current atmosphere shader.
		boost::shared_ptr<IqShader> m_pshadInteriorVolume;          ///< a pointer to the current interior shader.
		boost::shared_ptr<IqShader> m_pshadExteriorVolume;          ///< a pointer to the current exterior shader.

		CqTrimLoopArray m_TrimLoops;					///< the array of closed trimcurve loops.
		std::vector<boost::weak_ptr<CqLightsource> > m_apLightsources;	///< a set of currently available lightsources.

		std::list<CqAttributes*>::iterator	m_StackIterator;	///< the index of this attribute state in the global stack, used for destroying when last reference is removed.
}
;

//----------------------------------------------------------------------
// Implementation details.

inline const	IqParameter* CqAttributes::GetAttribute( const char* strName, const char* strParam ) const
{
	return pParameter(strName, strParam);
}

inline IqParameter* CqAttributes::GetAttributeWrite( const char* strName, const char* strParam )
{
	return pParameterWrite(strName, strParam);
}


/// Global attribute stack.
extern std::list<CqAttributes*>	Attribute_stack;


//-----------------------------------------------------------------------

} // namespace Aqsis

//}  // End of #ifdef ATTRIBUTES_H_INCLUDED
#endif
