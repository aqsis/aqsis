//------------------------------------------------------------------------------
/**
 *	@file	ilog.h
 *	@author	Matthäus G. Chajdas
 *	@brief	Declare the interface structure for the log class.
 *
 *	Last change by:		$Author: mw_matti $
 *	Last change date:	$Date: 2003/02/03 13:53:42 $
 */ 
//------------------------------------------------------------------------------
#ifndef	___ilog_Loaded___
#define	___ilog_Loaded___

#include "aqsis.h"
#include <string>

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
		virtual void log( char* priority, const char* stringFormat, ... ) = 0;
		
		/** Log an error message
		 */
		virtual void error( int table, int error_id ) = 0;

		/** Get an error from the message table
		 */
		virtual const char* getError( int table, int error_id ) = 0;

				
	private:
		
		virtual void createFileLog( std::string filename, std::string name)  = 0;
		virtual void createCOUTLog( std::string name ) = 0;
};

//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	//	___ilog_Loaded___