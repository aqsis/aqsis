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


/** \file	log.h
		\brief The Log class
		\author Matthäus G. Chajdas (Matthaeus@darkside-conflict.net)
*/


//? Is .h included already?
#ifndef LOG_H_INCLUDED
#define LOG_H_INCLUDED 1

#include <stdio.h>
#include <log4cpp/Portability.hh>
#include <iostream>
#include <log4cpp/Category.hh>
#include <log4cpp/Appender.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/Layout.hh>
#include <log4cpp/BasicLayout.hh>
#include <log4cpp/Priority.hh>

#include "imtable.h"
#include "mtable.h"		//!TODO: Remove this as soon as IqMessageTable is used everywhere

#include "ilog.h"
#include "aqsis.h"

namespace log4cpp
{

class AqLayout : public Layout
{
	public:
		AqLayout();
		virtual ~AqLayout();

		virtual std::string format( const LoggingEvent& event );


};

class AqFileLayout : public Layout
{
	public:
		AqFileLayout();
		virtual ~AqFileLayout();

		virtual std::string format( const LoggingEvent& event );


};
}

START_NAMESPACE( Aqsis )

class CqLog	: public IqLog
{
	public:
		CqLog( char* name = "AqsisLog", bool noConsoleOutput = false );
		virtual ~CqLog();

		void addFileLog( std::string filename = "render.log", std::string name = "AqsisFileLog" );
		void removeFileLog( std::string name );

		void setMessageTable( IqMessageTable* pTable );
		IqMessageTable* getMessageTable();

		void log( const char* priority, const char* stringFormat, ... );
		void log( const char* priority, const CqString &stringFormat );

		/** Log an error message
		 */
		void error( int table, int error_id );
		void error( const char* stringFormat, ... );
		void error( const CqString &string );

		/** Log an warning
		 */
		void warn( int table, int error_id );
		void warn( const char* stringFormat, ... );
		void warn( const CqString &string );

		/** Log an critical error message
		 */
		void critical( int table, int error_id );
		void critical( const char* stringFormat, ... );
		void critical( const CqString &string );

		/** Log an notice message
		 */
		void notice( int table, int error_id );
		void notice( const char* stringFormat, ... );
		void notice( const CqString &string );

		/** Log an info message
		 */
		void info( int table, int error_id );
		void info( const char* stringFormat, ... );
		void info( const CqString &string );

		/** Log an fatal error
		 */
		void fatal( int table, int error_id );
		void fatal( const char* stringFormat, ... );
		void fatal( const CqString &string );

		/** Log an debug message
		 */
		void debug( const char* stringFormat, ... );
		void debug( const CqString &string );

		const char* getError( int table, int error_id );

	private:

		void createFileLog( std::string filename, std::string name );
		void createCOUTLog( std::string name = "AqsisConsoleLog" );

		void log2( log4cpp::Priority::Value priority, const char* stringFormat, va_list va );
		void log2( log4cpp::Priority::Value priority, const char* string );

		log4cpp::Appender* m_pAppender;
		log4cpp::Appender* m_pFileAppender;
		log4cpp::AqLayout* m_pLayout;
		log4cpp::AqFileLayout* m_pFileLayout;
		log4cpp::Category* m_pRoot;

		IqMessageTable* m_pTable;
};

END_NAMESPACE( Aqsis )

#endif

