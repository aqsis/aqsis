//------------------------------------------------------------------------------
/**
 *	@file	ilog.h
 *	@author	Matthäus G. Chajdas
 *	@brief	Declare the interface structure for the log class.
 *
 *	Last change by:		$Author: mw_matti $
 *	Last change date:	$Date: 2003/03/10 17:15:39 $
 */ 
//------------------------------------------------------------------------------
#ifndef	___ilog_Loaded___
#define	___ilog_Loaded___

#include "aqsis.h"
#include <string>
#include "sstring.h"

#include "mtable.h"


START_NAMESPACE( Aqsis )


struct IqLog  
{
	public:
		virtual ~IqLog()
		{}

		/** Add a file log, i.e. write the output to a file
		 */
		virtual void addFileLog( std::string filename, std::string name) = 0;
		
		/** Remove a file log, use the same name as used in addFileLog
		 */
		virtual void removeFileLog( std::string name) = 0;
		
		/** Set a message table as the current message table
		 */
		virtual void setMessageTable( CqMessageTable* pTable ) = 0;
		
		/** Get the current message table
		 */
		virtual CqMessageTable* getMessageTable() = 0;

		/** Log a message
		 */
		virtual void log( const char* priority, const char* stringFormat, ... ) = 0;
		virtual void log( const char* priority, const CqString &stringFormat ) = 0;
				
		/** Log an error message
		 */
		virtual void error( int table, int error_id ) = 0;
		virtual void error( const char* stringFormat, ... ) = 0;
		virtual void error( const CqString &string ) = 0;

		/** Log an warning
		 */
		virtual void warn( int table, int error_id ) = 0;
		virtual void warn( const char* stringFormat, ... ) = 0;
		virtual void warn( const CqString &string ) = 0;

		/** Log an critical error message
		 */
		virtual void critical( int table, int error_id ) = 0;
		virtual void critical( const char* stringFormat, ... ) = 0;
		virtual void critical( const CqString &string ) = 0;

		/** Log an notice message
		 */
		virtual void notice( int table, int error_id ) = 0;
		virtual void notice( const char* stringFormat, ... ) = 0;
		virtual void notice( const CqString &string ) = 0;
		
		/** Log an info message
		 */
		virtual void info( int table, int error_id ) = 0;
		virtual void info( const char* stringFormat, ... ) = 0;
		virtual void info( const CqString &string ) = 0;
		
		/** Log an fatal error
		 */
		virtual void fatal( int table, int error_id ) = 0;
		virtual void fatal( const char* stringFormat, ... ) = 0;
		virtual void fatal( const CqString &string ) = 0;
		
		/** Log an debug message
		 */
		virtual void debug( const char* stringFormat, ... ) = 0;
		virtual void debug( const CqString &string ) = 0;


		/** Get an error from the message table
		 */
		virtual const char* getError( int table, int error_id ) = 0;

};

//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	//	___ilog_Loaded___

