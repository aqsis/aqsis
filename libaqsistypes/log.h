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

		void log( char* priority, const char* stringFormat, ... );
		void error( int table, int error_id );

		const char* getError( int table, int error_id );

				
	private:
		
		void createFileLog( std::string filename, std::string name );
		void createCOUTLog( std::string name = "AqsisConsoleLog" );

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