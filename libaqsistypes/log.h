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

#include "mtable.h"
#include "ilog.h"
#include "aqsis.h"

namespace log4cpp
{

	class AqLayout : public Layout  
	{
	public:
		AqLayout();
		virtual ~AqLayout();

		virtual std::string format(const LoggingEvent& event);


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
		
		void setMessageTable( CqMessageTable* pTable );
		CqMessageTable* getMessageTable();

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

		enum {	BASIC_ERROR_TABLE = 0,
				RI_ERROR_TABLE,
				SHADER_ERROR_TABLE } m_ErrorTables;

		enum {	UNKNOW_ERROR = 0,
				RI_COLOR_SAMPLES_INVALID,
				RI_RELATIVE_DETAIL_INVALID,
				RI_UNKNOWN_SYMBOL,
				RI_AREA_LIGHT_UNSUPPORTED,
				RI_INTERIOR_UNSUPPORTED,
				RI_EXTERIOR_UNSUPPORTED,
				RI_DETAIL_RANGE_INVALID,
				RI_GEOMETRIC_APPROX_UNSUPPORTED,
				RI_PERSPECTIVE_BAD_FOV,
				RI_DEFORMATION_UNSUPPORTED,
				RI_TRANSFORM_POINTS_UNSUPPORTED,
				RI_BLOBBY_V_UNSUPPORTED,
				RI_CURVES_V_UNKNOWN_WRAP_MODE,
				RI_CURVES_V_UNKNOWN_TYPE,
				RI_PROC_DELAYED_READ_ARCHIVE_UNSUPPORTED,
				RI_PROC_RUN_PROGRAM_UNSUPPORTED,
				RI_PROC_DYNAMIC_LOAD_UNSUPPORTED,
				RI_PROCEDURAL_UNKNOWN_SUBDIV,
				RI_GEOMETRY_V_UNKNOWN,
				RI_OBJECT_BEGIN_UNSUPPORTED,
				RI_OBJECT_END_UNSUPPORTED,
				RI_MAKE_BUMP_UNSUPPORTED } m_RiErrorTable;

				
	private:
		
		void createFileLog( std::string filename, std::string name );
		void createCOUTLog( std::string name = "AqsisConsoleLog" );

		void log2( log4cpp::Priority::Value priority, const char* stringFormat, va_list va );
		void log2( log4cpp::Priority::Value priority, const char* string );

		log4cpp::Appender* m_pAppender;
		log4cpp::Appender* m_pFileAppender;
		log4cpp::AqLayout* m_pLayout;
		log4cpp::AqLayout* m_pFileLayout;
		log4cpp::Category* m_pRoot;

		CqMessageTable* m_pTable;

		bool m_FirstRun;
};

END_NAMESPACE( Aqsis )

#endif

