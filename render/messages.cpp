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
		\brief Implements the classes for outputting error and informational messages.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	"aqsis.h"

#include	"messages.h"
#include	"irenderer.h"

START_NAMESPACE(Aqsis)



CqReportedErrors	gReportedErrors;

char* gstrMessageType[MessageType_Last]=
{
	"Message",
	"Warning",
	"Error"
};

//---------------------------------------------------------------------
/** Destructor
 */

CqReportedErrors::~CqReportedErrors()	
{
	// We know that items are only ever added to our list via SetReported, 
	// so need to be deleted here.
	TqInt i;
	for(i=0; i<m_aReportedErrors.size(); i++)
		if(m_aReportedErrors[i]!=0)
			delete(m_aReportedErrors[i]);
}


//---------------------------------------------------------------------
/** Add a nww error report to the list of reported errors.
 * \param pError Pointer to error message to add to list.
 */

void CqReportedErrors::SetReported(CqBasicError* pError)
{
	m_aReportedErrors.push_back(pError);
}


//---------------------------------------------------------------------
/** Check if an error should be reported.
 * \param pError Pointer to error to check.
 */

TqBool CqReportedErrors::CheckReport(CqBasicError* pError)
{
	TqInt i;
	for(i=0; i<m_aReportedErrors.size(); i++)
		if(m_aReportedErrors[i]->CheckReport(pError))
			return(false);
	return(true);
}


//---------------------------------------------------------------------
/** Clear the cache of reported error messages.
 */

void CqReportedErrors::ClearReported()
{
	TqInt i;
	for(i=0; i<m_aReportedErrors.size(); i++)
		delete(m_aReportedErrors[i]);
	m_aReportedErrors.clear();
}


//---------------------------------------------------------------------
/** Default constructor
 * \param code Integer error code.
 * \param severity Integer severity.
 * \param message Character pointer to message text.
 * \param onceper Flag indicating the message should only be show once.
 */

CqBasicError::CqBasicError(TqInt code, TqInt severity, const char* message, TqBool onceper) :	
m_Code(code)
{
	if(gReportedErrors.CheckReport(this))
	{
		(*pCurrentRenderer()->optCurrent().pErrorHandler())(code, severity, message);
		if(onceper)
			gReportedErrors.SetReported(new CqBasicError(*this));
	}
}

//---------------------------------------------------------------------
/** Default constructor
 * \param code Integer error code.
 * \param severity Integer severity.
 * \param message Character pointer to message text.
 * \param pAttributes Pointer to CqAttributes class to associate message with.
 * \param onceper Flag indicating the message should only be show once.
 */
CqAttributeError::CqAttributeError(TqInt code, TqInt severity, const char* message, const CqAttributes* pAttributes, TqBool onceper) : 
CqBasicError(code),
m_pAttributes(pAttributes)
{
	if(gReportedErrors.CheckReport(this))
	{
		// TODO: This needs tidying up.
		CqString strMessage;
		const CqString* pattrName=pAttributes->GetStringAttribute("identifier","name");
		CqString strName("<unnamed>");
		if(pattrName!=0)	strName=pattrName[0];
		strMessage=message;
		strMessage+=" : ";
		strMessage+=strName;
		(*pCurrentRenderer()->optCurrent().pErrorHandler())(code, severity, (char*)strMessage.c_str());
		if(onceper)
			gReportedErrors.SetReported(new CqAttributeError(*this));
	}
}


//---------------------------------------------------------------------

END_NAMESPACE(Aqsis)
