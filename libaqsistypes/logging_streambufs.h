#ifndef LOG_BUFFERS_H
#define LOG_BUFFERS_H

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

#include <iostream>

///////////////////////////////////////////////////////////
// timestamp_buf

/// When attached to an output stream, prefixes every line of output with a timestamp
class timestamp_buf :
	public std::streambuf
{
public:
	timestamp_buf(std::ostream& Stream);
	~timestamp_buf();

protected:
	int overflow(int);
	int sync();

private:
	std::ostream& m_stream;
	std::streambuf* const m_streambuf;
	bool m_start_new_line;
};

///////////////////////////////////////////////////////////
// show_level_buf

/// When attached to an output stream, prefixes every line of output with its priority (if any)
class show_level_buf :
	public std::streambuf
{
public:
	show_level_buf(std::ostream& Stream);
	~show_level_buf();

protected:
	int overflow(int);
	int sync();

private:
	std::ostream& m_stream;
	std::streambuf* const m_streambuf;
	bool m_start_new_line;
};

///////////////////////////////////////////////////////////
// fold_duplicates_buf

/// When attached to an output stream, replaces duplicate lines of output with a message indicating the number of duplicates
class fold_duplicates_buf :
	public std::streambuf
{
public:
	fold_duplicates_buf(std::ostream& Stream);
	~fold_duplicates_buf();

protected:
	int overflow(int);
	int sync();

private:
	std::ostream& m_stream;
	std::streambuf* const m_streambuf;
	std::string m_buffer;
	std::string m_last_buffer;
	unsigned long m_duplicate_count;
};

///////////////////////////////////////////////////////////
// syslog_buf

/// When attached to an output stream, copies output to the system log
class syslog_buf :
	public std::streambuf
{
public:
	syslog_buf(std::ostream& Stream);
	~syslog_buf();

protected:
	int overflow(int);
	int sync();

private:
	void write_to_system_log(const std::string& Message);

	std::ostream& m_stream;
	std::streambuf* const m_streambuf;
	std::string m_buffer;
};

#endif // !LOG_BUFFERS_H

