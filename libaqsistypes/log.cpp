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
		\brief Implements the CqLog class and a AqLayout helper class
		\author Matthäus G. Chajdas (Matthaeus@darkside-conflict.net)
*/

#include "log.h"

namespace log4cpp {

    AqLayout::AqLayout() {
    }
    
    AqLayout::~AqLayout() {
    }

    std::string AqLayout::format(const LoggingEvent& event) {
        std::ostringstream message;

        const std::string& priorityName = Priority::getPriorityName(event.priority);
        message << priorityName << " " 
                << event.categoryName << " " << event.ndc << ": " 
                << event.message << std::endl;

        return message.str();
    }
}

START_NAMESPACE( Aqsis )

CqLog::CqLog( char* name, bool noConsoleOutput )
{
	log4cpp::Category& root = log4cpp::Category::getRoot();
	
	m_pRoot = &root;

	m_pRoot->removeAllAppenders();

	
	if ( !noConsoleOutput )
	{
		createCOUTLog();
	}

	m_FirstRun = true;
}

CqLog::~CqLog()
{
	log4cpp::Category::shutdown();
}

void CqLog::log(char* priority, const char* stringFormat, ...)
{

	if ( m_FirstRun )
	{
		std::cout << "using log4cpp 0.3.4b ( licensed under the LGPL )" << std::endl;

		m_FirstRun = false;
	}

	va_list va;
	va_start(va, stringFormat);

	log4cpp::Priority::Value pVal;
	
	// Report an error if unknown priority used
	try
	{
		pVal = log4cpp::Priority::getPriorityValue( priority );
	}
	catch ( std::invalid_argument e )
	{
		log( "ERROR", e.what() );
	}

	m_pRoot->logva(pVal, stringFormat, va);


	va_end(va);
}

void CqLog::log2( log4cpp::Priority::Value priority, const char* stringFormat, ... )
{
	if ( m_FirstRun )
	{
		std::cout << "using log4cpp 0.3.4b ( licensed under the LGPL )" << std::endl;

		m_FirstRun = false;
	}

	va_list va;
	va_start(va, stringFormat);

	m_pRoot->logva(priority, stringFormat, va);

	va_end(va);
}

void CqLog::createFileLog( std::string filename , std::string name )
{
	m_pFileAppender = 
        new log4cpp::FileAppender( name, filename );

	m_pFileLayout = new log4cpp::AqLayout();
	
	
	m_pFileAppender->setLayout( m_pFileLayout );

	m_pRoot->addAppender( m_pFileAppender );
	
}

void CqLog::createCOUTLog( std::string name )
{
	m_pAppender = 
        new log4cpp::OstreamAppender(name, &std::cout);

	m_pLayout = new log4cpp::AqLayout();

	m_pAppender->setLayout( m_pLayout );
	
	m_pRoot->addAppender( m_pAppender );
}

void CqLog::setMessageTable( CqMessageTable* pTable )
{
	m_pTable = pTable;
}

void CqLog::addFileLog( std::string filename, std::string name )
{
	// Pass through to createFileLog
	
	createFileLog( filename, name );
}

const char* CqLog::getError( int table, int error_id )
{
	// Call the MessageTable
	return m_pTable->getError( table, error_id );
}

CqMessageTable* CqLog::getMessageTable()
{
	return m_pTable;
}

void CqLog::removeFileLog( std::string name )
{
	m_pRoot->removeAppender( m_pRoot->getAppender( name ) );
}

// ------------------------- Log functions

void CqLog::error( int table, int error_id )
{
	m_pRoot->error( CqLog::getError( table, error_id ) );
}

void CqLog::error( const char* stringFormat, ... )
{
	va_list va;
	va_start(va,stringFormat);
	log2(log4cpp::Priority::ERROR, stringFormat, va);
	va_end(va);
}

// -------------------------- Warning
void CqLog::warn( int table, int error_id )
{
	m_pRoot->warn( CqLog::getError( table, error_id ) );
}

void CqLog::warn( const char* stringFormat, ... )
{
	va_list va;
	va_start(va,stringFormat);
	log2(log4cpp::Priority::WARN, stringFormat, va);
	va_end(va);
}

// --------------------------- Critical
void CqLog::critical( int table, int error_id )
{
	m_pRoot->crit( CqLog::getError( table, error_id ) );
}

void CqLog::critical( const char* stringFormat, ... )
{
	va_list va;
	va_start(va,stringFormat);
	log2(log4cpp::Priority::CRIT, stringFormat, va);
	va_end(va);
}

// ---------------------------- Notice
void CqLog::notice( int table, int error_id )
{
	m_pRoot->notice( CqLog::getError( table, error_id ) );
}

void CqLog::notice( const char* stringFormat, ... )
{
	va_list va;
	va_start(va,stringFormat);
	log2(log4cpp::Priority::NOTICE, stringFormat, va);
	va_end(va);
}

// ---------------------------- Info
void CqLog::info( int table, int error_id )
{
	m_pRoot->info( CqLog::getError( table, error_id ) );
}

void CqLog::info( const char* stringFormat, ... )
{
	va_list va;
	va_start(va,stringFormat);
	log2(log4cpp::Priority::INFO, stringFormat, va);
	va_end(va);
}

// ----------------------------- Fatal
void CqLog::fatal( int table, int error_id )
{
	m_pRoot->fatal( CqLog::getError( table, error_id ) );
}

void CqLog::fatal( const char* stringFormat, ... )
{
	va_list va;
	va_start(va,stringFormat);
	log2(log4cpp::Priority::FATAL, stringFormat, va);
	va_end(va);
}

// ------------------------------ Debug

void CqLog::debug( const char* stringFormat, ... )
{
	va_list va;
	va_start(va,stringFormat);
	log2(log4cpp::Priority::DEBUG, stringFormat, va);
	va_end(va);
}

END_NAMESPACE( Aqsis )