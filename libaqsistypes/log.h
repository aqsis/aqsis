// CqLog.h: Schnittstelle für die Klasse CqLog.
//
//////////////////////////////////////////////////////////////////////

#ifndef __log_inc_
#define __log_inc_

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

class CqLog  
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


#endif