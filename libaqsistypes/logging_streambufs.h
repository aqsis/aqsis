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


#ifndef	___logging_streambufs_Loaded___
#define	___logging_streambufs_Loaded___

#include <iostream>
#include <string>

#include "aqsis.h"

#ifdef	AQSIS_SYSTEM_WIN32
#include <windows.h>
#ifdef	ERROR
#undef	ERROR
#endif
#endif

START_NAMESPACE( Aqsis )

///////////////////////////////////////////////////////////
// tag_buf

/// When attached to an output stream, prepends a string "tag" to the beginning of every line of output
class tag_buf :
	public std::streambuf
{
public:
	tag_buf(const std::string& Tag, std::ostream& Stream);
	~tag_buf();

protected:
	int overflow(int);
	int sync();

private:
	
	std::ostream& m_stream;
	std::streambuf* const m_streambuf;
	bool m_start_new_line;
	const std::string m_tag;
};

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

/// When attached to an output stream, prefixes every line of output with its log-level (if any)
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
// color_level_buf

/// When attached to an output stream, colors output based on its log level (if any)
class color_level_buf :
	public std::streambuf
{
public:
	color_level_buf(std::ostream& Stream);
	~color_level_buf();

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
    bool print_duplicates();

    std::ostream& m_stream;
    std::streambuf* const m_streambuf;
    std::string m_buffer;
    std::string m_last_buffer;
    unsigned long m_duplicate_count;
};

///////////////////////////////////////////////////////////
// reset_level_buf

/// When attached to an output stream, resets the log level to "unknown" after every line of output
class reset_level_buf :
            public std::streambuf
{
public:
    reset_level_buf(std::ostream& Stream);
    ~reset_level_buf();

protected:
    int overflow(int);
    int sync();

private:
    std::ostream& m_stream;
    std::streambuf* const m_streambuf;
};

///////////////////////////////////////////////////////////
// filter_by_level_buf

/// Enumerates available log levels
typedef enum
{
    CRITICAL = 1,
    ERROR = 2,
    WARNING = 3,
    INFO = 4,
    DEBUG = 5
} log_level_t;

/// When attached to an output stream, filters-out messages below the given level
class filter_by_level_buf :
            public std::streambuf
{
public:
    filter_by_level_buf(const log_level_t MinimumLevel, std::ostream& Stream);
    ~filter_by_level_buf();

protected:
    int overflow(int);
    int sync();

private:
    std::ostream& m_stream;
    std::streambuf* const m_streambuf;
    const log_level_t m_minimum_level;
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

#ifdef	AQSIS_SYSTEM_WIN32

///////////////////////////////////////////////////////////
// ansi_buf

/// Scans the stream for ANSI control sequences and emulates the behaviour under Windows
class ansi_buf :
	public std::streambuf
{
public:
	ansi_buf(std::ostream& Stream);
	~ansi_buf();

protected:
	int overflow(int);
	int sync();

private:
	void process_code();
	void set_attributes();
	
	std::ostream& m_stream;
	std::streambuf* const m_streambuf;
	int m_processing_ansi;
	WORD m_attributes;
	std::string m_code;
	HANDLE	m_sbh;
	CONSOLE_SCREEN_BUFFER_INFO	m_csbInfo;
};

#endif

//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// ___logging_streambufs_Loaded___

