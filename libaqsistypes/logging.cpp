// Copyright © 2003, Timothy M. Shead
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

#include "logging.h"
#include "logging_streambufs.h"

#include <ostream>
#include <sstream>
#include <vector>
#include <time.h>

#ifndef	NO_SYSLOG
#include <syslog.h>
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

typedef enum
{
	EMERGENCY = 1,
	ALERT,
	CRITICAL,
	ERROR,
	WARNING,
	NOTICE,
	INFO,
	DEBUG
} log_level_t;

} // namespace detail

START_NAMESPACE( Aqsis )

//---------------------------------------------------------------------

std::ostream& emergency(std::ostream& Stream)
{
	detail::log_level(Stream) = detail::EMERGENCY;
	return Stream;
}

std::ostream& alert(std::ostream& Stream)
{
	detail::log_level(Stream) = detail::ALERT;
	return Stream;
}

std::ostream& critical(std::ostream& Stream)
{
	detail::log_level(Stream) = detail::CRITICAL;
	return Stream;
}

std::ostream& error(std::ostream& Stream)
{
	detail::log_level(Stream) = detail::ERROR;
	return Stream;
}

std::ostream& warning(std::ostream& Stream)
{
	detail::log_level(Stream) = detail::WARNING;
	return Stream;
}

std::ostream& notice(std::ostream& Stream)
{
	detail::log_level(Stream) = detail::NOTICE;
	return Stream;
}

std::ostream& info(std::ostream& Stream)
{
	detail::log_level(Stream) = detail::INFO;
	return Stream;
}

std::ostream& debug(std::ostream& Stream)
{
	detail::log_level(Stream) = detail::DEBUG;
	return Stream;
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

			if(m_streambuf->sputn(&buffer[0], buffer.size()) != buffer.size())
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
					case detail::EMERGENCY:
						buffer = "EMERGENCY: ";
						break;
					case detail::ALERT:
						buffer = "ALERT: ";
						break;
					case detail::CRITICAL:
						buffer = "CRITICAL: ";
						break;
					case detail::ERROR:
						buffer = "ERROR: ";
						break;
					case detail::WARNING:
						buffer = "WARNING: ";
						break;
					case detail::NOTICE:
						buffer = "NOTICE: ";
						break;
					case detail::INFO:
						buffer = "INFO: ";
						break;
					case detail::DEBUG:
						buffer = "DEBUG: ";
						break;
				}

			if(m_streambuf->sputn(buffer.c_str(), buffer.size()) != buffer.size())
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
	if(m_buffer.size())
		m_streambuf->sputn(m_buffer.c_str(), m_buffer.size());

	m_stream.rdbuf(m_streambuf);
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
					if(m_duplicate_count)
						{
							std::ostringstream buffer;
							buffer << "Last message repeated " << m_duplicate_count << " time";
							if(m_duplicate_count > 1)
								buffer << "s";
							buffer << "\n";

							const std::string message(buffer.str());

							if(m_streambuf->sputn(message.c_str(), message.size()) != message.size())
								return EOF;

							m_duplicate_count = 0;
						}

					if(m_streambuf->sputn(m_buffer.c_str(), m_buffer.size()) != m_buffer.size())
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

#ifndef	NO_SYSLOG
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
			case detail::EMERGENCY:
				priority = LOG_EMERG;
				break;
			case detail::ALERT:
				priority = LOG_ALERT;
				break;
			case detail::CRITICAL:
				priority = LOG_CRIT;
				break;
			case detail::ERROR:
				priority = LOG_ERR;
				break;
			case detail::WARNING:
				priority = LOG_WARNING;
				break;
			case detail::NOTICE:
				priority = LOG_NOTICE;
				break;
			case detail::INFO:
				priority = LOG_INFO;
				break;
			case detail::DEBUG:
				priority = LOG_DEBUG;
				break;
		}
		
	syslog(LOG_USER | priority, "%s", Message.c_str());
}

#endif //NO_SYSLOG

END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------
