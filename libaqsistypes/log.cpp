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

/* This is a basic layout that is used to display the whole messages.
	The format is:
		PRIORITY: message
*/
namespace log4cpp {

    AqLayout::AqLayout() {
    }
    
    AqLayout::~AqLayout() {
    }

    std::string AqLayout::format(const LoggingEvent& event) {
        std::ostringstream message;

        message << event.message << std::endl;

        return message.str();
    }

	AqFileLayout::AqFileLayout() {
    }
    
    AqFileLayout::~AqFileLayout() {
    }

    std::string AqFileLayout::format(const LoggingEvent& event) {
        std::ostringstream message;

        const std::string& priorityName = Priority::getPriorityName(event.priority);
        message << priorityName << " " 
                << event.categoryName << " " << event.ndc << ": " 
                << event.message << std::endl;

        return message.str();
    }
}

START_NAMESPACE( Aqsis )

// Constructor
// Creates a log
CqLog::CqLog( char* name, bool noConsoleOutput )
{
	log4cpp::Category& root = log4cpp::Category::getRoot();
	
	m_pRoot = &root;

	m_pRoot->removeAllAppenders();

	
	if ( !noConsoleOutput )
	{
		createCOUTLog();
	}
}

// Close down the log and shutdown all categories
CqLog::~CqLog()
{
	log4cpp::Category::shutdown();
}

// Log a message, if possible, don't use this
void CqLog::log( const char* priority, const char* stringFormat, ...)
{

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

// Log a message with CqString, if possible, don't use this
void CqLog::log(const char *priority, const CqString &stringFormat)
{

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

	m_pRoot->log(pVal, stringFormat.c_str());
}

// Internal log function for error(), debug() etc. calls, just like log4cpp does it
void CqLog::log2( log4cpp::Priority::Value priority, const char* stringFormat, va_list va )
{
	m_pRoot->logva(priority, stringFormat, va);
}

// Internal log without ...
void CqLog::log2( log4cpp::Priority::Value priority, const char* string )
{
	m_pRoot->log(priority, string);
}

// Create a file log, i.e. pipe the output to a file
void CqLog::createFileLog( std::string filename , std::string name )
{
	m_pFileAppender = 
        new log4cpp::FileAppender( name, filename );

	m_pFileLayout = new log4cpp::AqFileLayout();
	
	
	m_pFileAppender->setLayout( m_pFileLayout );

	m_pRoot->addAppender( m_pFileAppender );
	
}

// Create a std::cout log, i.e. write to console
void CqLog::createCOUTLog( std::string name )
{
	m_pAppender = 
        new log4cpp::OstreamAppender(name, &std::cout);

	m_pLayout = new log4cpp::AqLayout();

	m_pAppender->setLayout( m_pLayout );
	
	m_pRoot->addAppender( m_pAppender );
}

// Set a message table as the current message table
void CqLog::setMessageTable( IqMessageTable* pTable )
{
	m_pTable = pTable;
}

// Creeate another file log
void CqLog::addFileLog( std::string filename, std::string name )
{
	// Pass through to createFileLog
	
	createFileLog( filename, name );
}

// Get an error message, usually called getError( CqLog::*_ERROR_TABLE, CqLog::ERROR_MESSAGE_ID )
const char* CqLog::getError( int table, int error_id )
{
	// Call the MessageTable
	return m_pTable->getError( table, error_id );
}

// Get a pointer to the current message table
IqMessageTable* CqLog::getMessageTable()
{
	return m_pTable;
}

// Remove a file log, i.e. close the file
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

void CqLog::error( const CqString &string )
{
	log2(log4cpp::Priority::ERROR, string.c_str());
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

void CqLog::warn( const CqString &string )
{
	log2(log4cpp::Priority::WARN, string.c_str());
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

void CqLog::critical( const CqString &string )
{
	log2(log4cpp::Priority::CRIT, string.c_str());
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

void CqLog::notice( const CqString &string )
{
	log2(log4cpp::Priority::NOTICE, string.c_str());
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

void CqLog::info( const CqString &string )
{
	log2(log4cpp::Priority::INFO, string.c_str());
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

void CqLog::fatal( const CqString &string )
{
	log2(log4cpp::Priority::FATAL, string.c_str());
}

// ------------------------------ Debug
void CqLog::debug( const char* stringFormat, ... )
{
	va_list va;
	va_start(va,stringFormat);
	log2(log4cpp::Priority::DEBUG, stringFormat, va);
	va_end(va);
}

void CqLog::debug( const CqString &string )
{
	log2(log4cpp::Priority::DEBUG, string.c_str());
}

//-------------------------------- Utility functions
IqLog* CreateLogger()
{
	return( new CqLog );
}

void DeleteLogger(IqLog *log)
{
	delete( log );
}

END_NAMESPACE( Aqsis )
