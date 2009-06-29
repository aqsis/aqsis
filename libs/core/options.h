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
		\brief Declares the classes and structures for handling RenderMan options.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is options.h included already?
#ifndef OPTIONS_H_INCLUDED
//{
#define OPTIONS_H_INCLUDED 1

#include <aqsis/aqsis.h>

#include <vector>

#include <aqsis/math/color.h>
#include <aqsis/util/file.h>
#include <aqsis/core/ioptions.h>
#include <aqsis/shadervm/ishader.h>
#include <aqsis/math/matrix.h>
#include "parameters.h"
#include <aqsis/ri/ri.h>
#include <aqsis/util/sstring.h>

namespace Aqsis {


class CqImagersource;

class CqOptions;
typedef boost::shared_ptr<CqOptions> CqOptionsPtr;

//----------------------------------------------------------------------
/** \class CqOptions
 * Storage for the graphics state options.
 */
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
		boost::shared_ptr<const CqNamedParameterList> pOption( const char* strName ) const;
		/** Get a pointer to a named user option.
		 * \param strName Character pointer to the requested options name.
		 * \return A pointer to the option, or 0 if not found. 
		 */
		boost::shared_ptr<CqNamedParameterList> pOptionWrite( const char* strName );
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

		virtual void	InitialiseColorImager( const CqRegion& DRegion, IqChannelBuffer* buffer );
		virtual CqColor GetColorImager( TqFloat x, TqFloat y );
		virtual CqColor GetOpacityImager( TqFloat x, TqFloat y );
		virtual TqFloat GetAlphaImager( TqFloat x, TqFloat y );

		//@{
		/** \brief Find a file in the given RI search path.
		 *
		 * This function finds a file in the named RI search path as set using
		 * RiOption("searchpath", "path_name", "path", RI_NULL).  The findFile()
		 * function is used to perform the search.  If the file is not found in the
		 * given search path, the "resource" search path is used as a fallback.
		 *
		 * If the file is still not found, an XqInvalidFile exception is thrown, or an
		 * empty path returned for the "Nothrow" version.  Both versions may throw
		 * boost::filesystem::filesystem_error for underlying OS-level errors.
		 *
		 * \see findFile
		 *
		 * \param fileName - name of the file
		 * \param riSearchPathName - name of the search path as specified to RiOption
		 */
		virtual boost::filesystem::path findRiFile(const std::string& fileName,
				const char* riSearchPathName) const;
		virtual boost::filesystem::path findRiFileNothrow(const std::string& fileName,
				const char* riSearchPathName) const;
		//@}

	private:
		std::vector<boost::shared_ptr<CqNamedParameterList> >	m_aOptions;	///< Vector of user specified options.

		RtFilterFunc m_funcFilter;						///< Pointer to the pixel filter function.
		CqImagersource* m_pshadImager;		///< Pointer to the imager shader.
}
;


} // namespace Aqsis

//-----------------------------------------------------------------------
//}  // End of #ifdef OPTIONS_H_INCLUDED
#endif
