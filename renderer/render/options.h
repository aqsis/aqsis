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
		\brief Declares the classes and structures for handling RenderMan options.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is options.h included already?
#ifndef OPTIONS_H_INCLUDED
//{
#define OPTIONS_H_INCLUDED 1

#include	<vector>

#include	"ri.h"
#include	"matrix.h"
#include	"sstring.h"
#include	"refcount.h"
#include	"color.h"
#include	"exception.h"
#include	"parameters.h"
#include	"ishader.h"
#include	"ioptions.h"

START_NAMESPACE( Aqsis )


class CqImagersource;
struct IqBucket;

//----------------------------------------------------------------------
/** \class CqOptions
 * Storage for the graphics state options.
 */

class CqOptions;
typedef boost::shared_ptr<CqOptions> CqOptionsPtr;
class CqOptions : public IqOptions
{
	public:
		CqOptions();
		CqOptions( const CqOptions& From );
		virtual ~CqOptions();

		CqOptions&	operator=( const CqOptions& From );

		/** Add a named user option to the current state.
		 * \param pOption A pointer to a new CqNamedParameterList class containing the initialised option.
		 */
		void	AddOption( const boost::shared_ptr<CqNamedParameterList>& pOption )
		{
			m_aOptions.push_back( pOption );
		}
		/** Clear all user options from the state.
		 */
		void	ClearOptions()
		{
			m_aOptions.clear();
			InitialiseDefaultOptions();
		}
		/** Initialise default system options.
		 */
		void InitialiseDefaultOptions();
		/** Get a read only pointer to a named user option.
		 * \param strName Character pointer to the requested options name.
		 * \return A pointer to the option, or 0 if not found. 
		 */
		boost::shared_ptr<const CqNamedParameterList> pOption( const char* strName ) const
		{
			const TqUlong hash = CqString::hash( strName );
			std::vector<boost::shared_ptr<CqNamedParameterList> >::const_iterator
			i = m_aOptions.begin(), end = m_aOptions.end();
			for ( i = m_aOptions.begin(); i != end; ++i )
			{
				if ( ( *i ) ->hash() == hash )
				{
					return ( *i );
				}
			}
			boost::shared_ptr<const CqNamedParameterList> retval;
			return ( retval );
			// return ( boost::shared_ptr<const CqNamedParameterList>() );
		}
		/** Get a pointer to a named user option.
		 * \param strName Character pointer to the requested options name.
		 * \return A pointer to the option, or 0 if not found. 
		 */
		boost::shared_ptr<CqNamedParameterList> pOptionWrite( const char* strName )
		{
			const TqUlong hash = CqString::hash( strName );
			std::vector<boost::shared_ptr<CqNamedParameterList> >::iterator
			i = m_aOptions.begin(), end = m_aOptions.end();
			for ( ; i != end; ++i )
			{
				if ( ( *i ) ->hash() == hash )
				{
					if ( ( *i ).unique() )
					{
						return ( *i );
					}
					else
					{
						boost::shared_ptr<CqNamedParameterList> pNew( new CqNamedParameterList( *( *i ) ) );
						( *i ) = pNew;
						return ( pNew );
					}
				}
			}
			m_aOptions.push_back( boost::shared_ptr<CqNamedParameterList>( new CqNamedParameterList( strName ) ) );
			return ( m_aOptions.back() );
		}
		const	CqParameter* pParameter( const char* strName, const char* strParam ) const;
		CqParameter* pParameterWrite( const char* strName, const char* strParam );
		virtual const	TqFloat*	GetFloatOption( const char* strName, const char* strParam ) const;
		virtual const	TqInt*	GetIntegerOption( const char* strName, const char* strParam ) const;
		virtual const	CqString* GetStringOption( const char* strName, const char* strParam ) const;
		virtual const	CqVector3D*	GetPointOption( const char* strName, const char* strParam ) const;
		virtual const	CqColor*	GetColorOption( const char* strName, const char* strParam ) const;

		virtual TqFloat*	GetFloatOptionWrite( const char* strName, const char* strParam, TqInt arraySize = 1 );
		virtual TqInt*	GetIntegerOptionWrite( const char* strName, const char* strParam, TqInt arraySize = 1 );
		virtual CqString* GetStringOptionWrite( const char* strName, const char* strParam, TqInt arraySize = 1 );
		virtual CqVector3D*	GetPointOptionWrite( const char* strName, const char* strParam, TqInt arraySize = 1 );
		virtual CqColor*	GetColorOptionWrite( const char* strName, const char* strParam, TqInt arraySize = 1 );

		virtual EqVariableType getParameterType(const char* strName, const char* strParam) const;
		virtual EqVariableClass getParameterClass(const char* strName, const char* strParam) const;
		virtual TqUint getParameterSize(const char* strName, const char* strParam) const;
		virtual TqInt getParameterArraySize(const char* strName, const char* strParam) const;

		virtual void	InitialiseCamera();

		/** Get a pointer to the pixel filter function.
		 */
		virtual RtFilterFunc funcFilter() const
		{
			return ( m_funcFilter );
		}
		/** Set the pixel filter function to use.
		 * \param fValue A pointer to a function which follows the RtFilterFunc convention.
		 */
		virtual void	SetfuncFilter( const RtFilterFunc fValue )
		{
			m_funcFilter = fValue;
		}
		virtual void SetpshadImager( const boost::shared_ptr<IqShader>& pshadImager );
		virtual boost::shared_ptr<IqShader>	pshadImager() const;

		virtual void	InitialiseColorImager( IqBucket* pBucket );
		virtual CqColor GetColorImager( TqFloat x, TqFloat y );
		virtual CqColor GetOpacityImager( TqFloat x, TqFloat y );
		virtual TqFloat GetAlphaImager( TqFloat x, TqFloat y );

	private:
		std::vector<boost::shared_ptr<CqNamedParameterList> >	m_aOptions;	///< Vector of user specified options.

		RtFilterFunc m_funcFilter;						///< Pointer to the pixel filter function.
		CqImagersource* m_pshadImager;		///< Pointer to the imager shader.
}
;


END_NAMESPACE( Aqsis )

//-----------------------------------------------------------------------
//}  // End of #ifdef OPTIONS_H_INCLUDED
#endif
