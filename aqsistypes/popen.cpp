// Aqsis
// Copyright (C) 1997 - 2007, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
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
 *
 * \brief Utilities for opening a subprocess with connected pipes.
 *
 * \author Chris Foster [ chris42f (at) g mail (dot) com ]
 */

#include "popen.h"

#ifdef AQSIS_SYSTEM_WIN32
//#	include <io.h>
#else
#	include <unistd.h>
#	include <signal.h>
#endif // AQSIS_SYSTEM_WIN32

#include <boost/scoped_array.hpp>

#include "exception.h"

namespace Aqsis
{

#ifdef AQSIS_SYSTEM_WIN32
//------------------------------------------------------------------------------
// windows implementation for CqPopenDevice::CqImpl
class CqPopenDevice::CqImpl
{
	// TODO: Implementation!
	private:
	public:
		CqImpl(const std::string& progName, const std::vector<std::string>& argv)
		{
			AQSIS_THROW_XQERROR(XqInternal, EqE_Unimplement, "CqPopenDevice::CqImpl not yet implemented on windows!");
		}
};

#else // AQSIS_SYSTEM_WIN32
//------------------------------------------------------------------------------
// posix implementation for CqPopenDevice::CqImpl
class CqPopenDevice::CqImpl
{
	private:
		/// pipe fd from which we can read the child stdout
		int m_pipeReadFd;
		/// pipe fd into which we can write to the child stdin
		int m_pipeWriteFd;

		/** \brief Connect the given file descriptors to stdin and stdout.
		 * 
		 * \param newStdin - file descriptor to connect to stdin
		 * \param newStdout - file descriptor to connect to stdout
		 *
		 * This function is used after a fork() to connect the new child
		 * process stdin and stdout to the provided file descriptors which
		 * correspond to pipes back to the parent process.
		 */
		static bool connectStdInOut(int newStdin, int newStdout)
		{
			// There's some special cases if the STDIN and STDOUT fd's have got
			// mixed up somehow.
			if(newStdin == STDIN_FILENO && newStdout == STDOUT_FILENO)
			{
				return true;
			}
			if(newStdin == STDOUT_FILENO && newStdout == STDIN_FILENO)
			{
				// need to swap stdout & stdin
				int tmpStdout = ::dup(newStdout);
				if(tmpStdout == -1
					|| ::close(newStdout) == -1
					|| ::dup2(newStdin, STDIN_FILENO) == -1
					|| ::close(newStdin) == -1
					|| ::dup2(tmpStdout, STDOUT_FILENO) == -1
					|| ::close(tmpStdout) == -1 )
				{
					return false;
				}
				return true;
			}
			// In other cases we can just connect the new stdout and stdin fd's
			if(newStdin != STDIN_FILENO)
			{
				if(::dup2(newStdin, STDIN_FILENO) == -1)
					return false;
				::close(newStdin);
			}
			if(newStdout != STDOUT_FILENO)
			{
				if(::dup2(newStdout, STDOUT_FILENO) == -1)
					return false;
				::close(newStdout);
			}
			return true;
		}

	public:
		CqImpl(const std::string& progName, const std::vector<std::string>& argv)
			: m_pipeReadFd(-1),
			m_pipeWriteFd(-1)
		{
			// fd array corresponds to [read, write] ends of the pipe.
			// pipe fd's which will have the read connected to the child stdin
			int childStdinFd[2];
			// pipe fd's which will have the write connected to the child stdout
			int childStdoutFd[2];
			if(pipe(childStdinFd) == -1)
			{
				AQSIS_THROW_XQERROR(XqEnvironment, EqE_System,
					"Could not creating pipe");
			}
			if(pipe(childStdoutFd) == -1)
			{
				::close(childStdinFd[0]);
				::close(childStdinFd[1]);
				AQSIS_THROW_XQERROR(XqEnvironment, EqE_System,
					"Could not creating pipe");
			}
			const int childRead = childStdinFd[0];
			const int parentWrite = childStdinFd[1];
			const int childWrite = childStdoutFd[1];
			const int parentRead = childStdoutFd[0];
			::signal(SIGPIPE, SIG_IGN);
			if(pid_t pid = fork())
			{
				// Parent process
				if(pid == -1)
				{
					// Could not create child; close fd's and throw.
					::close(childRead);
					::close(childWrite);
					::close(parentRead);
					::close(parentWrite);
					AQSIS_THROW_XQERROR(XqEnvironment, EqE_System,
						"could not fork child process");
				}
				// Close the child ends of the pipe 
				::close(childRead);
				::close(childWrite);
				m_pipeReadFd = parentRead;
				m_pipeWriteFd = parentWrite;
			}
			else
			{
				// Child process
				// Close the parent ends of the pipe
				::close(parentRead);
				::close(parentWrite);
				// Connect the process stdin to the pipe
				if(!connectStdInOut(childRead, childWrite))
					std::exit(EXIT_FAILURE);
				// Copy argument list into a C-style array of raw pointers.
				TqInt argc = argv.size();
				boost::scoped_array<char*> argvRaw(new char*[argc + 1]);
				for(TqInt i = 0; i < argc; ++i)
					argvRaw[i] = const_cast<char*>(argv[i].c_str());
				// Terminate the argv list with a null.
				argvRaw[argc] = 0;
				// Execute the child program in a new process.
				::execvp(progName.c_str(), &argvRaw[0]);
				// We only ever get here if there's an error.
				std::exit(EXIT_FAILURE);
				// (note errno's of interest - EACCES ENOENT ENOEXEC)
			}
		}

		/// Get the file descriptor associated with pipe to read from.
		int pipeReadFd()
		{
			return m_pipeReadFd;
		}
		/// Get the file descriptor associated with pipe to write to.
		int pipeWriteFd()
		{
			return m_pipeWriteFd;
		}
};
#endif // AQSIS_SYSTEM_WIN32

//------------------------------------------------------------------------------
// CqPopenDevice implementation.
CqPopenDevice::CqPopenDevice(const std::string& progName,
		const std::vector<std::string>& argv)
	: m_impl(new CqImpl(progName, argv))
{ }

std::streamsize CqPopenDevice::read(char_type* s, std::streamsize n)
{
#	ifdef AQSIS_SYSTEM_WIN32
	AQSIS_THROW_XQERROR(XqInternal, EqE_Unimplement, "Not implemented!");
	return 0;
#	else // AQSIS_SYSTEM_WIN32
	std::streamsize nRead = ::read(m_impl->pipeReadFd(), s, n);
	if(nRead == -1)
		throw std::ios_base::failure("Bad read from pipe");
	return nRead == 0 ? -1 : nRead;
#	endif // AQSIS_SYSTEM_WIN32
}

std::streamsize CqPopenDevice::write(const char_type* s, std::streamsize n)
{
#	ifdef AQSIS_SYSTEM_WIN32
	AQSIS_THROW_XQERROR(XqInternal, EqE_Unimplement, "Not implemented!");
	return 0;
#	else // AQSIS_SYSTEM_WIN32
	std::streamsize nWrite = ::write(m_impl->pipeWriteFd(), s, n);
	if(nWrite < n)
		throw std::ios_base::failure("Bad write to pipe");
	return nWrite;
#	endif // AQSIS_SYSTEM_WIN32
}

void CqPopenDevice::close(std::ios_base::openmode mode)
{
#	ifdef AQSIS_SYSTEM_WIN32
	AQSIS_THROW_XQERROR(XqInternal, EqE_Unimplement, "Not implemented!");
#	else // AQSIS_SYSTEM_WIN32
	if(mode == std::ios_base::in)
		::close(m_impl->pipeReadFd());
	else if(mode == std::ios_base::out)
		::close(m_impl->pipeWriteFd());
#	endif // AQSIS_SYSTEM_WIN32
}

} // namespace Aqsis
