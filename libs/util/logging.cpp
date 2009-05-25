// Copyright (C) 2003, Timothy M. Shead
//
// Contact: tshead@k-3d.com
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include <aqsis/aqsis.h>

#include <aqsis/util/logging.h>
#include <aqsis/util/logging_streambufs.h>

#include <cstdio>              // For EOF
#include <iostream>
#include <sstream>
#include <vector>
#include <time.h>

#ifndef	AQSIS_NO_SYSLOG
#include <syslog.h>
#endif

#if defined __GNUC__ && (__GNUC__ < 3)
	#define pubsync sync
#endif

namespace detail
{

int log_level_index()
{
	static int index = std::ios::xalloc();
	return index;
}

long& log_level(std::ostream& Stream)
{
	return Stream.iword(log_level_index());
}

} // namespace detail

namespace Aqsis {

//---------------------------------------------------------------------

std::ostream& log()
{
	detail::log_level(std::cerr) = 0;
	return std::cerr;
}
	
std::ostream& critical(std::ostream& Stream)
{
	detail::log_level(Stream) = CRITICAL;
	return Stream;
}

std::ostream& error(std::ostream& Stream)
{
	detail::log_level(Stream) = ERROR;
	return Stream;
}

std::ostream& warning(std::ostream& Stream)
{
	detail::log_level(Stream) = WARNING;
	return Stream;
}

std::ostream& info(std::ostream& Stream)
{
	detail::log_level(Stream) = INFO;
	return Stream;
}

std::ostream& debug(std::ostream& Stream)
{
	detail::log_level(Stream) = DEBUG;
	return Stream;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// tag_buf

tag_buf::tag_buf(const std::string& Tag, std::ostream& Stream) :
		m_stream(Stream),
		m_streambuf(Stream.rdbuf()),
		m_start_new_line(true),
		m_tag(Tag + " ")
{
	setp(0, 0);
	m_stream.rdbuf(this);
}

tag_buf::~tag_buf()
{
	m_stream.rdbuf(m_streambuf);
}

int tag_buf::overflow(int c)
{
	if(c == EOF)
		return 0;

	if(m_start_new_line)
	{
		m_start_new_line = false;

		if(static_cast<size_t>(m_streambuf->sputn(&m_tag[0], m_tag.size()))
		        != m_tag.size())
			return EOF;
	}

	if(c == '\n')
		m_start_new_line = true;

	return m_streambuf->sputc(c);
}

int tag_buf::sync()
{
	m_streambuf->pubsync();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// timestamp_buf

timestamp_buf::timestamp_buf(std::ostream& Stream) :
		m_stream(Stream),
		m_streambuf(Stream.rdbuf()),
		m_start_new_line(true)
{
	setp(0, 0);
	m_stream.rdbuf(this);
}

timestamp_buf::~timestamp_buf()
{
	m_stream.rdbuf(m_streambuf);
}

int timestamp_buf::overflow(int c)
{
	if(c == EOF)
		return 0;

	if(m_start_new_line)
	{
		m_start_new_line = false;

		std::vector<char> buffer(256, '\0');
		time_t current_time = time(0);
		buffer.resize(strftime(&buffer[0], buffer.size(), "%m/%d/%Y %H:%M:%S ", localtime(&current_time)));

		if(m_streambuf->sputn(&buffer[0], buffer.size()) != (std::streamsize) buffer.size())
			return EOF;
	}

	if(c == '\n')
		m_start_new_line = true;

	int rc = m_streambuf->sputc(c);

	return rc;
}

int timestamp_buf::sync()
{
	m_streambuf->pubsync();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// show_level_buf

show_level_buf::show_level_buf(std::ostream& Stream) :
		m_stream(Stream),
		m_streambuf(Stream.rdbuf()),
		m_start_new_line(true)
{
	setp(0, 0);
	m_stream.rdbuf(this);
}

show_level_buf::~show_level_buf()
{
	m_stream.rdbuf(m_streambuf);
}

int show_level_buf::overflow(int c)
{
	if(c == EOF)
		return 0;

	if(m_start_new_line)
	{
		m_start_new_line = false;

		std::string buffer;
		switch(detail::log_level(m_stream))
		{
				case CRITICAL:
				buffer = "CRITICAL: ";
				break;
				case ERROR:
				buffer = "ERROR: ";
				break;
				case WARNING:
				buffer = "WARNING: ";
				break;
				case INFO:
				buffer = "INFO: ";
				break;
				case DEBUG:
				buffer = "DEBUG: ";
				break;
		}

		if(m_streambuf->sputn(buffer.c_str(), buffer.size()) != (std::streamsize) buffer.size())
			return EOF;
	}

	if(c == '\n')
		m_start_new_line = true;

	int rc = m_streambuf->sputc(c);

	return rc;
}

int show_level_buf::sync()
{
	m_streambuf->pubsync();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// color_level_buf

color_level_buf::color_level_buf(std::ostream& Stream) :
		m_stream(Stream),
		m_streambuf(Stream.rdbuf()),
		m_start_new_line(true)
{
	setp(0, 0);
	m_stream.rdbuf(this);
}

color_level_buf::~color_level_buf()
{
	m_stream.rdbuf(m_streambuf);
}

int color_level_buf::overflow(int c)
{
	if(c == EOF)
		return 0;

	if(m_start_new_line)
	{
		m_start_new_line = false;

		std::string buffer;
		switch(detail::log_level(m_stream))
		{
				case CRITICAL:
				buffer = "\033[1;31m";
				break;
				case ERROR:
				buffer = "\033[1;31m";
				break;
				case WARNING:
				buffer = "\033[36m";
				break;
				case INFO:
				buffer = "\033[0m";
				break;
				case DEBUG:
				buffer = "\033[32m";
				break;
				default:
				buffer = "\033[0m";
				break;
		}
		if(static_cast<size_t>(m_streambuf->sputn(buffer.c_str(), buffer.size()))
		        != buffer.size())
			return EOF;
	}

	if(c == '\n')
	{
		m_start_new_line = true;

		const std::string buffer = "\033[0m";
		if(static_cast<size_t>(m_streambuf->sputn(buffer.c_str(), buffer.size()))
		        != buffer.size())
			return EOF;
	}

	return m_streambuf->sputc(c);
}

int color_level_buf::sync()
{
	m_streambuf->pubsync();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// reset_level_buf

reset_level_buf::reset_level_buf(std::ostream& Stream) :
		m_stream(Stream),
		m_streambuf(Stream.rdbuf())
{
	setp(0, 0);
	m_stream.rdbuf(this);
}

reset_level_buf::~reset_level_buf()
{
	m_stream.rdbuf(m_streambuf);
}

int reset_level_buf::overflow(int c)
{
	if(c == '\n')
		detail::log_level(m_stream) = 0;

	return m_streambuf->sputc(c);
}

int reset_level_buf::sync()
{
	m_streambuf->pubsync();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// filter_by_level_buf

filter_by_level_buf::filter_by_level_buf(const log_level_t MinimumLevel, std::ostream& Stream) :
		m_stream(Stream),
		m_streambuf(Stream.rdbuf()),
		m_minimum_level(MinimumLevel)
{
	setp(0, 0);
	m_stream.rdbuf(this);
}

filter_by_level_buf::~filter_by_level_buf()
{
	m_stream.rdbuf(m_streambuf);
}

int filter_by_level_buf::overflow(int c)
{
	if(detail::log_level(m_stream) <= m_minimum_level)
		return m_streambuf->sputc(c);

	return c;
}

int filter_by_level_buf::sync()
{
	m_streambuf->pubsync();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// fold_duplicates_buf

fold_duplicates_buf::fold_duplicates_buf(std::ostream& Stream) :
		m_stream(Stream),
		m_streambuf(Stream.rdbuf()),
		m_duplicate_count(0)
{
	setp(0, 0);
	m_stream.rdbuf(this);
}

fold_duplicates_buf::~fold_duplicates_buf()
{
	// Flush any leftover output ...
	print_duplicates();

	if(m_buffer.size())
		m_streambuf->sputn(m_buffer.c_str(), m_buffer.size());

	m_stream.rdbuf(m_streambuf);
}

bool fold_duplicates_buf::print_duplicates()
{
	if(m_duplicate_count)
	{
		std::ostringstream buffer;
		buffer << "Last message repeated " << m_duplicate_count << " time";
		if(m_duplicate_count > 1)
			buffer << "s";
		buffer << "\n";

		const std::string message(buffer.str());

		if(m_streambuf->sputn(message.c_str(), message.size()) != (std::streamsize) message.size())
			return false;

		m_duplicate_count = 0;
	}

	return true;
}

int fold_duplicates_buf::overflow(int c)
{
	if(c == EOF)
		return 0;

	m_buffer += c;

	if(c == '\n')
	{
		if(m_buffer == m_last_buffer)
		{
			m_duplicate_count++;
		}
		else
		{
			if(!print_duplicates())
				return EOF;

			if(m_streambuf->sputn(m_buffer.c_str(), m_buffer.size()) != (std::streamsize) m_buffer.size())
				return EOF;

			m_last_buffer = m_buffer;
		}

		m_buffer.erase();
	}

	return c;
}

int fold_duplicates_buf::sync()
{
	m_streambuf->pubsync();
	return 0;
}

#ifndef	AQSIS_NO_SYSLOG
/////////////////////////////////////////////////////////////////////////////////////////////
// syslog_buf

syslog_buf::syslog_buf(std::ostream& Stream) :
		m_stream(Stream),
		m_streambuf(Stream.rdbuf())
{
	setp(0, 0);
	m_stream.rdbuf(this);
}

syslog_buf::~syslog_buf()
{
	// Flush any leftover data ...
	if(!m_buffer.empty())
		write_to_system_log(m_buffer);

	m_stream.rdbuf(m_streambuf);
}

int syslog_buf::overflow(int c)
{
	if(c == EOF)
		return 0;

	m_buffer += c;
	if(c == '\n')
	{
		write_to_system_log(m_buffer.substr(0, m_buffer.size()-1));
		m_buffer.erase();
	}

	return m_streambuf->sputc(c);
}

int syslog_buf::sync()
{
	m_streambuf->pubsync();
	return 0;
}

void syslog_buf::write_to_system_log(const std::string& Message)
{
	int priority = LOG_INFO;

	switch(detail::log_level(m_stream))
	{
			case CRITICAL:
			priority = LOG_CRIT;
			break;
			case ERROR:
			priority = LOG_ERR;
			break;
			case WARNING:
			priority = LOG_WARNING;
			break;
			case INFO:
			priority = LOG_INFO;
			break;
			case DEBUG:
			priority = LOG_DEBUG;
			break;
	}

	syslog(LOG_USER | priority, "%s", Message.c_str());
}

#endif //AQSIS_NO_SYSLOG

#ifdef	AQSIS_SYSTEM_WIN32

/////////////////////////////////////////////////////////////////////////////////////////////
// ansi_buf

ansi_buf::ansi_buf(std::ostream& Stream) :
		m_stream(Stream),
		m_streambuf(Stream.rdbuf()),
		m_processing_ansi(0)
{
	setp(0, 0);
	m_stream.rdbuf(this);
	m_sbh = GetStdHandle(STD_ERROR_HANDLE);
	GetConsoleScreenBufferInfo(m_sbh, &m_csbInfo);
	m_attributes = m_csbInfo.wAttributes;
}

ansi_buf::~ansi_buf()
{
	m_stream.rdbuf(m_streambuf);
}

int ansi_buf::overflow(int c)
{
	if(c == EOF)
		return 0;

	if(m_processing_ansi == 1)
	{
		if( c == '[' )	// Code terminator
		{
			m_code = "";
			m_attributes = 0;
			m_processing_ansi = 2;
			return(0);
		}
		else
			m_processing_ansi = 0;
	}

	if(m_processing_ansi == 2)
	{
		if( c == ';' )	// Code terminator
		{
			process_code();
			m_code = "";
			return(0);
		}
		else if( c == 'm' )
		{
			m_processing_ansi = false;
			process_code();
			set_attributes();
			return(0);
		}
		else
		{
			m_code.append(reinterpret_cast<char*>(&c), 1);
			return(0);
		}
	}

	if(c == '\033')
	{
		m_processing_ansi = 1;
		m_code = "";
		return(0);
	}

	return m_streambuf->sputc(c);
}


void ansi_buf::process_code()
{
	if(m_code.compare( "0" ) == 0)
		m_attributes = m_csbInfo.wAttributes;
	else if(m_code.compare( "1" ) == 0)
		m_attributes |= FOREGROUND_INTENSITY;
	else if(m_code.compare( "31" ) == 0)
		m_attributes |= FOREGROUND_RED;
	else if(m_code.compare( "32" ) == 0)
		m_attributes |= FOREGROUND_GREEN;
	else if(m_code.compare( "33" ) == 0)
		m_attributes |= FOREGROUND_RED|FOREGROUND_GREEN;
	else if(m_code.compare( "34" ) == 0)
		m_attributes |= FOREGROUND_BLUE;
	else if(m_code.compare( "35" ) == 0)
		m_attributes |= FOREGROUND_BLUE|FOREGROUND_RED;
	else if(m_code.compare( "36" ) == 0)
		m_attributes |= FOREGROUND_GREEN|FOREGROUND_BLUE;
	else if(m_code.compare( "37" ) == 0)
		m_attributes |= FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE;
	else if(m_code.compare( "41" ) == 0)
		m_attributes |= BACKGROUND_RED;
	else if(m_code.compare( "42" ) == 0)
		m_attributes |= BACKGROUND_GREEN;
	else if(m_code.compare( "43" ) == 0)
		m_attributes |= BACKGROUND_RED|BACKGROUND_GREEN;
	else if(m_code.compare( "44" ) == 0)
		m_attributes |= BACKGROUND_BLUE;
	else if(m_code.compare( "45" ) == 0)
		m_attributes |= BACKGROUND_BLUE|BACKGROUND_RED;
	else if(m_code.compare( "46" ) == 0)
		m_attributes |= BACKGROUND_GREEN|BACKGROUND_BLUE;
	else if(m_code.compare( "47" ) == 0)
		m_attributes |= BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE;
}


void ansi_buf::set_attributes()
{
	SetConsoleTextAttribute(m_sbh, m_attributes);
}

int ansi_buf::sync()
{
	m_streambuf->pubsync();
	return 0;
}

#endif


} // namespace Aqsis
//---------------------------------------------------------------------
