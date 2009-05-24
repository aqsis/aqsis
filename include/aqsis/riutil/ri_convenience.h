// Aqsis
// Copyright 1997 - 2001, Paul C. Gregory
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
 * \brief Convenience functions and macros for working with the RenderMan
 * C-Style API.
 **/

#ifndef RI_CONVENIENCE_H
#define RI_CONVENIENCE_H

#include <aqsis/aqsis.h>

#include <vector>
#include <cstdarg>

#include <aqsis/ri/ritypes.h>

namespace Aqsis
{

//------------------------------------------------------------------------------

/** AQSIS_COLLECT_RI_PARAMETERS
 *
 * \brief helper macro to collect Ri* variadic function parameters.
 *
 * \code
 * AQSIS_COLLECT_RI_PARAMETERS( name )
 * RiProjectionV( name, AQSIS_PASS_RI_PARAMETERS );
 * \endcode
 *
 * \param _last named parameter before the ellipsis
 */

#define AQSIS_COLLECT_RI_PARAMETERS(_last)                                     \
	va_list pArgs;                                                             \
	va_start( pArgs, _last );                                                  \
	std::vector<RtToken> aTokens;                                              \
	std::vector<RtPointer> aValues;                                            \
	RtInt count = Aqsis::buildRiParameterList( pArgs, aTokens, aValues );      \
	va_end( pArgs );

/** AQSIS_PASS_RI_PARAMETERS
 *
 * \brief helper macro to pass variadic parameters collected by
 * AQSIS_EXTRACT_RI_PARAMETERS to another Ri* style function.
 *
 * \see AQSIS_COLLECT_RI_PARAMETERS
 */

#define AQSIS_PASS_RI_PARAMETERS                                               \
	count, aTokens.size()>0?&aTokens[0]:0, aValues.size()>0?&aValues[0]:0

/** buildRiParameterList
 *
 * \brief Helper function used by the AQSIS_EXTRACT_RI_PARAMETERS macro to build
 * a parameter list to pass on to the variadic Ri* functions.
 *
 * \code
 * va_list pArgs;
 * va_start( pArgs, nverts );
 * std::vector<RtToken> aTokens;
 * std::vector<RtPointer> aValues;
 * RtInt count = BuildRiParameterList( pArgs, aTokens, aValues );
 * va_end( pArgs );
 * \endcode
 *
 * \param pArgs
 * \param aTokens
 * \param aValues
 * \param returns a parameter count
 */
RtInt buildRiParameterList( va_list pArgs, std::vector<RtToken>& aTokens,
                                 std::vector<RtPointer>& aValues );

/** \brief Adaptor allowing RtArchiveCallback to be bound to
 * IqRibParser::TqCommentCallback
 */
class CqArchiveCallbackAdaptor
{
	public:
		/// Construct adaptor with an underlying callback (may not be null).
		CqArchiveCallbackAdaptor(RtArchiveCallback callback);

		/** Call through to the underlying RtArchiveCallback function.
		 *
		 * The input comment string is examined for a leading #.  If present,
		 * the # is stripped and the comment sent through to the underlying
		 * archive callback as a RIB structure comment.  Otherwise the comment
		 * is sent through unchanged as the plain comment type.
		 */
		void operator()(const std::string& comment);
	private:
		RtArchiveCallback m_callback;
};


//==============================================================================
// Implementation details
//==============================================================================

inline RtInt buildRiParameterList( va_list pArgs, std::vector<RtToken>& aTokens,
                                 std::vector<RtPointer>& aValues )
{
	RtInt count = 0;
	RtToken pToken = va_arg( pArgs, RtToken );
	RtPointer pValue;
	aTokens.clear();
	aValues.clear();
	while ( pToken != RI_NULL )
	{
		aTokens.push_back( pToken );
		pValue = va_arg( pArgs, RtPointer );
		aValues.push_back( pValue );
		pToken = va_arg( pArgs, RtToken );
		count++;
	}
	return ( count );
}

//------------------------------------------------------------------------------
inline CqArchiveCallbackAdaptor::CqArchiveCallbackAdaptor(RtArchiveCallback callback)
	: m_callback(callback)
{
	assert(m_callback);
}

inline void CqArchiveCallbackAdaptor::operator()(const std::string& comment)
{
	// If the first character is # then the comment is a special "RIB
	// structure comment"
	if(!comment.empty() && comment[0] == '#')
		m_callback(tokenCast("structure"), tokenCast("%s"), comment.c_str()+1);
	else
		m_callback(tokenCast("comment"), tokenCast("%s"), comment.c_str());
}


} // namespace Aqsis

#endif // RI_CONVENIENCE_H
