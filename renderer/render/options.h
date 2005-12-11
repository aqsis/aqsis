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
		\brief Declares the classes and structures for handling RenderMan options.
		\author Paul C. Gregory (pgregory@aqsis.com)
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


START_NAMESPACE( Aqsis )


class CqImagersource;
struct IqBucket;

//----------------------------------------------------------------------
/** \enum EqDisplayMode
 */
enum EqDisplayMode
{
    ModeNone = 0x0000,   	///< Invalid.
    ModeRGB = 0x0001,   		///< Red Green and Blue channels.
    ModeA = 0x0002,   		///< Alpha channel.
    ModeZ = 0x0004		///< Depth channel.
};


//----------------------------------------------------------------------
/** \enum EqProjection
 * Possible projection modes for the camera.
 */
enum EqProjection
{
    ProjectionOrthographic,   		///< Orthographic projection.
    ProjectionPerspective		///< Perspective projection.
};


//----------------------------------------------------------------------
/** \class CqOptions
 * Storage for the graphics state options.
 */

class CqOptions
{
	public:
		CqOptions();
		CqOptions( const CqOptions& From );
		~CqOptions();

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
			TqUlong hash = CqString::hash( strName );
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
			TqUlong hash = CqString::hash( strName );
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
		const	TqFloat*	GetFloatOption( const char* strName, const char* strParam ) const;
		const	TqInt*	GetIntegerOption( const char* strName, const char* strParam ) const;
		const	CqString* GetStringOption( const char* strName, const char* strParam ) const;
		const	CqVector3D*	GetPointOption( const char* strName, const char* strParam ) const;
		const	CqColor*	GetColorOption( const char* strName, const char* strParam ) const;

		TqFloat*	GetFloatOptionWrite( const char* strName, const char* strParam );
		TqInt*	GetIntegerOptionWrite( const char* strName, const char* strParam );
		CqString* GetStringOptionWrite( const char* strName, const char* strParam );
		CqVector3D*	GetPointOptionWrite( const char* strName, const char* strParam );
		CqColor*	GetColorOptionWrite( const char* strName, const char* strParam );

		void	InitialiseCamera();

		/** Indicate that the RiFormat function has been called.
		 * This and other similar are used to overcome the problem with multiple ways to specify frame dimensions.
		 */
		void	CallFormat()
		{
			m_bFormatCalled = TqTrue;
		}
		/** Indicate that the RiScreenWindow function has been called.
		 * This and other similar are used to overcome the problem with multiple ways to specify frame dimensions.
		 */
		void	CallScreenWindow()
		{
			m_bScreenWindowCalled = TqTrue;
		}
		/** Indicate that the RiFrameAspectRatio function has been called.
		 * This and other similar are used to overcome the problem with multiple ways to specify frame dimensions.
		 */
		void	CallFrameAspectRatio()
		{
			m_bFrameAspectRatioCalled = TqTrue;
		}
		/** Determine if the RiFormat function has been called.
		 * This and other similar are used to overcome the problem with multiple ways to specify frame dimensions.
		 */
		TqBool	FormatCalled() const
		{
			return ( m_bFormatCalled );
		}
		/** Determine if the RiScreenWindow function has been called.
		 * This and other similar are used to overcome the problem with multiple ways to specify frame dimensions.
		 */
		TqBool	ScreenWindowCalled() const
		{
			return ( m_bScreenWindowCalled );
		}
		/** Determine if the RiFrameAspectRatio function has been called.
		 * This and other similar are used to overcome the problem with multiple ways to specify frame dimensions.
		 */
		TqBool	FrameAspectRatioCalled() const
		{
			return ( m_bFrameAspectRatioCalled );
		}

		/** Get a pointer to the pixel filter function.
		 */
		RtFilterFunc funcFilter() const
		{
			return ( m_funcFilter );
		}
		/** Set the pixel filter function to use.
		 * \param fValue A pointer to a function which follows the RtFilterFunc convention.
		 */
		void	SetfuncFilter( const RtFilterFunc fValue )
		{
			m_funcFilter = fValue;
		}
		/** Get the name of the Imager shader.
		 */
		//		CqString	Get_imager() const
		//		{
		//			if( NULL == m_pParameters[ option_imager ] )	return( "null" );
		//
		//			return(*static_cast<CqParameterTyped<CqString>*>(m_pParameters[option_imager])->pValue());
		//		}
		/** Set the name of the Imager shader.
		 * \param strValue Character pointer to name of Imager shader.
		 */
		//		void	Set_imager( const CqString& value )
		//		{
		//			if( NULL == m_pParameters[ option_imager ] )
		//				m_pParameters[ option_imager ] = new CqParameterTypedUniform<CqString, type_string>("imager");
		//			*static_cast<CqParameterTyped<CqString>*>(m_pParameters[ option_imager ])->pValue() = value;
		//
		//			LoadImager( value );
		//		}
		void	LoadImager( const CqString& strValue );
		void	DeleteImager();
		void	SetValueImager( char *token, char *value );
		/** Get a pointer to the imager shader.
		 */
		const CqImagersource*	pshadImager() const
		{
			return ( m_pshadImager );
		}

		void	InitialiseColorImager( IqBucket* pBucket );
		CqColor GetColorImager( TqFloat x, TqFloat y );
		CqColor GetOpacityImager( TqFloat x, TqFloat y );
		TqFloat GetAlphaImager( TqFloat x, TqFloat y );

	private:
		std::vector<boost::shared_ptr<CqNamedParameterList> >	m_aOptions;	///< Vector of user specified options.

		RtFilterFunc m_funcFilter;						///< Pointer to the pixel filter function.
		CqImagersource* m_pshadImager;		///< Pointer to the imager shader.

		TqBool	m_bFrameAspectRatioCalled,   		///< Indicate RiFrameAspectRatio has been called. Calculation of the screen geometry is reliant on which of these have been called.
		m_bScreenWindowCalled,   			///< Indicate RiScreenWindow has been called. Calculation of the screen geometry is reliant on which of these have been called.
		m_bFormatCalled;				///< Indicate RiFormat has been called. Calculation of the screen geometry is reliant on which of these have been called.
}
;


END_NAMESPACE( Aqsis )

//-----------------------------------------------------------------------
//}  // End of #ifdef OPTIONS_H_INCLUDED
#endif
