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
		\brief Declares the classes for outputting error and informational messages.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is messages.h included already?
#ifndef MESSAGES_H_INCLUDED
#define MESSAGES_H_INCLUDED 1

#include	<vector>

#include	"specific.h"	// Needed for namespace macros.

#include	"sstring.h"
#include	"attributes.h"

#define		_qShareName	CORE
#include	"share.h"

START_NAMESPACE(Aqsis)

//----------------------------------------------------------------------
/** \enum EqErrorType
 * Specifies the type of error being reported.
 */

enum EqErrorType
{
	ErrorType_Basic,			///< Basic error message.
	ErrorType_Attribute,		///< Error linked to an attribute state,
};


//----------------------------------------------------------------------
/** \enum EqErrorID
 * Possible error identifiers.
 */

enum EqErrorID
{
	ErrorID_MaxEyeSplits=1,				///< Max eyeplits exceeded
	ErrorID_InvalidPixel,				///< Invalid pixel requested in image buffer.
	ErrorID_InvalidShadowMap,			///< Shadow map not of supported format.
	ErrorID_FileNotFound,				///< Generic file not found.
	ErrorID_NoShadowName,				///< No shadow or lightource name supplied to use for shadowmap name.
	ErrorID_DisplayDriver,				///< Error loading display driver.
	ErrorID_System,						///< Generic system error.
	ErrorID_UnknownSymbol,				///< Unknown synbol fuond parsing RIB file.
	ErrorID_InvalidType,				///< Generic invalid type.
	ErrorID_InvalidData,				///< Generic invalid data.
	ErrorID_InvalidShader,				///< Shader not of correct type.
	ErrorID_NonmanifoldSubdivision,		///< Geometry used in subdivision surface is non-manifold.
}; 


//----------------------------------------------------------------------
/** \enum EqWarningID
 * Specifies the type of warning being reported.
 */

enum EqWarningID
{
	WarningID_NoDisplacementBound=1,	///< No displacement bound specified on displaced surface.
}; 

//----------------------------------------------------------------------
/** \enum EqMessageType
 * Specifies the type of message being displayed.
 */
enum EqMessageType
{
	MessageType_Message=0,	///< Just an informational message.

	MessageType_Warning,	///< Non fatal warning.
	MessageType_Error,		///< Fatal error.

	MessageType_Last
};

//----------------------------------------------------------------------
/** \enum EqSeverity
 * Specifies the severity of the error/warning being reported.
 */
enum EqSeverity
{
	Severity_Normal=0,			///< Normal error.
	Importance_Normal=0,		///< Normal warning.

	Severity_Fatal=255,			///< Fatal error.
	Importance_Imperative=255,	///< Important warning.
};


class CqBasicError;

//----------------------------------------------------------------------
/** \class CqReportedErrors
 * Storage class for the errors which have been reported, used to avoid repeated error messages.
 */

class _qShareC CqReportedErrors
{
	public:
	_qShareM	CqReportedErrors()	{}
	_qShareM	~CqReportedErrors();

	_qShareM	void	SetReported(CqBasicError* pError);
	_qShareM	TqBool	CheckReport(CqBasicError* pError);
	_qShareM	void	ClearReported();
	private:
		std::vector<CqBasicError*>	m_aReportedErrors;	///< Array of reported errors.
};

extern CqReportedErrors	gReportedErrors;	///< Global reported errors instance.


//----------------------------------------------------------------------
/** \struct SqMessage
 * Storege structure for message information.
 */
struct SqMessage
{
				SqMessage(TqInt code, TqInt severity, const char* strMessage)	:
							m_strMessage(strMessage),
							m_Code(code),
							m_Severity(severity)
					{}

					/** Get a pointer to the message text
					 * \return Contant CqStrign reference.
					 */
	const CqString& strMessage() const	{return(m_strMessage);}
					/** Get the error/warning code.
					 * \return Integer code.
					 */
	TqInt			Code() const		{return(m_Code);}
					/** Get the error/warning severity.
					 * \return Integer severity code.
					 */
	TqInt			Severity() const	{return(m_Severity);}

	CqString		m_strMessage;		///< Message text.
	TqInt			m_Code;				///< Message code, from EqErrorID or EqWarningID.
	TqInt			m_Severity;			///< Message severity, from EqSeverity.
};


//----------------------------------------------------------------------
/** \class CqBasicError
 * Base error class from which all errors are derived. Also serves as the simplest error, not linked to anything.
 */

class _qShareC CqBasicError
{
	public:
					/** Constructor taking just a code.
					 * \param code The error code from EqErrorID or EqWarningID.
					 */
	_qShareM		CqBasicError(TqInt code) : m_Code(code)
							{}
	_qShareM		CqBasicError(TqInt code, TqInt severity, const char* Message, TqBool OncePer=TqFalse);
					/** Copy constructor.
					 * \param From The error to copy.
					 */
	_qShareM		CqBasicError(CqBasicError* From)
							{
								m_Code=From->m_Code;
							}
					/** Get the error code.
					 * \return Integer error code.
					 */
	_qShareM virtual	~CqBasicError()	{}

	_qShareM virtual	TqInt Code()	{return(m_Code);}
					/** Check if the spcified error from the reported list matches sufficiently to not report it again.
					 * \return Boolean match.
					 */
	_qShareM virtual	TqBool	CheckReport(CqBasicError* from)
							{
								if(from->m_Code==m_Code)
									return(true);
								else
									return(false);
							}

	protected:
			TqInt		m_Code;		///< The error code from EqErrorID or EqWarningID.
};


//----------------------------------------------------------------------
/** \class CqAttributeError
 * Error directly related to an attribute state.
 */

class _qShareC CqAttributeError : public CqBasicError
{
	public:
	_qShareM		CqAttributeError(TqInt code, TqInt severity, const char* Message, const CqAttributes* pAttributes, TqBool OncePer=TqFalse);
					/** Copy constructor
					 * \param From Error to copy.
					 */
	_qShareM		CqAttributeError(CqAttributeError& From) :
							CqBasicError(From)
							{
								m_pAttributes=From.m_pAttributes;
							}
	_qShareM virtual	~CqAttributeError()	
							{}

					/** Check if the spcified error from the reported list matches sufficiently to not report it again.
					 * \return Boolean match.
					 */
	_qShareM virtual	TqBool	CheckReport(CqBasicError* From)
							{
								if(From->Code()==Code())
								{
									if(CqBasicError::CheckReport(From) && 
									   static_cast<CqAttributeError*>(From)->m_pAttributes==m_pAttributes)
										return(true);
									else
										return(false);
								}
								return(false);
							}

	protected:
			const CqAttributes* m_pAttributes;		///< Pointer to the attributes state this error is linked to.
};

#define	MSG_COUNT	50

//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif	// !MESSAGES_H_INCLUDED
