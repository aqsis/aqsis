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
#	include <windows.h>
#else
#	include <unistd.h>
#	include <signal.h>
#endif // AQSIS_SYSTEM_WIN32

#include <boost/scoped_array.hpp>

#include "exception.h"
#include "logging.h"

namespace Aqsis
{

#ifdef AQSIS_SYSTEM_WIN32
//------------------------------------------------------------------------------
// windows implementation for CqPopenDevice::CqImpl
class CqPopenDevice::CqImpl
{
	private:
		/// Handle to the file from which we can read child stdout.
		HANDLE m_pipeReadHandle;
		/// Handle to the file into which we can write to the child stdin.
		HANDLE m_pipeWriteHandle;
	public:
		CqImpl(const std::string& progName, const std::vector<std::string>& argv)
		{
			HANDLE hOutputReadTmp,hOutputWrite;
			HANDLE hInputWriteTmp,hInputRead;
			HANDLE hErrorWrite;
			SECURITY_ATTRIBUTES sa;
			BOOL bFuncRetn = FALSE;

			// Set up the security attributes struct.
			sa.nLength= sizeof(SECURITY_ATTRIBUTES);
			sa.lpSecurityDescriptor = NULL;
			sa.bInheritHandle = TRUE;

			// Create the child output pipe.
			if (!CreatePipe(&hOutputReadTmp,&hOutputWrite,&sa,0))
			{
				Aqsis::log() << error << "CreatePipe" << std::endl;
				return;
			}

			// Create a duplicate of the output write handle for the std error
			// write handle. This is necessary in case the child application
			// closes one of its std output handles.
			if (!DuplicateHandle(GetCurrentProcess(),hOutputWrite,
									GetCurrentProcess(),&hErrorWrite,0,
									TRUE,DUPLICATE_SAME_ACCESS))
			{
				Aqsis::log() << error << "DuplicateHandle" << std::endl;
				return;
			}

			// Create the child input pipe.
			if (!CreatePipe(&hInputRead,&hInputWriteTmp,&sa,0))
			{
				Aqsis::log() << error << "CreatePipe" << std::endl;
			}

			// Create new output read handle and the input write handles. Set
			// the Properties to FALSE. Otherwise, the child inherits the
			// properties and, as a result, non-closeable handles to the pipes
			// are created.
			if (!DuplicateHandle(GetCurrentProcess(),hOutputReadTmp,
								GetCurrentProcess(),
								&m_pipeReadHandle, // Address of new handle.
								0,FALSE, // Make it uninheritable.
								DUPLICATE_SAME_ACCESS))
			{
				Aqsis::log() << error << "DuplicateHandle" << std::endl;
				return;
			}

			if (!DuplicateHandle(GetCurrentProcess(),hInputWriteTmp,
								GetCurrentProcess(),
								&m_pipeWriteHandle, // Address of new handle.
								0,FALSE, // Make it uninheritable.
								DUPLICATE_SAME_ACCESS))
			{
				Aqsis::log() << error << "DuplicateHandle" << std::endl;
				return;
			}


			// Close inheritable copies of the handles you do not want to be
			// inherited.
			if (!CloseHandle(hOutputReadTmp)) 
			{
				Aqsis::log() << error << "CloseHandle" << std::endl;
				return;
			}
			if (!CloseHandle(hInputWriteTmp))
			{
				Aqsis::log() << error << "CloseHandle" << std::endl;
				return;
			}

			PROCESS_INFORMATION pi;
			STARTUPINFO si;

			// Set up the start up info struct.
			ZeroMemory(&si,sizeof(STARTUPINFO));
			si.cb = sizeof(STARTUPINFO);
			si.dwFlags = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
			si.hStdOutput = hOutputWrite;
			si.hStdInput  = hInputRead;
			si.hStdError  = hErrorWrite;
			si.wShowWindow = SW_HIDE;
			// Use this if you want to hide the child:
			//     si.wShowWindow = SW_HIDE;
			// Note that dwFlags must include STARTF_USESHOWWINDOW if you want to
			// use the wShowWindow flags.

			// Build the command string.
			std::stringstream strCommand;
			strCommand << "\"" << progName << "\"";
			for(std::vector<std::string>::const_iterator arg = argv.begin() + 1; arg != argv.end(); ++arg)
				strCommand << " \"" << *arg << "\"";
			TqInt totlen = strCommand.str().size();
			boost::scoped_array<char> command(new char[totlen+1]);
			strncpy(&command[0], strCommand.str().c_str(), totlen);
			command[totlen] = '\0';
			
			// Create the child process.
			bFuncRetn = CreateProcess(NULL,
									  command.get(), // command line
									  NULL,          // process security attributes
									  NULL,          // primary thread security attributes
									  TRUE,          // handles are inherited
									  0,             // creation flags
									  NULL,          // use parent's environment
									  NULL,		     // use parent's current directory
									  &si,			 // STARTUPINFO pointer
									  &pi);			 // receives PROCESS_INFORMATION

			if (bFuncRetn == 0)
			{
				Aqsis::log() << error << "CreateProcess " << strCommand.str() << " failed." << std::endl;
				return;
			}

			// Close any unnecessary handles.
			if (!CloseHandle(pi.hThread))
			{
				Aqsis::log() << error << "CloseHandle" << std::endl;
				return;
			}


			// Close pipe handles (do not continue to modify the parent).
			// You need to make sure that no handles to the write end of the
			// output pipe are maintained in this process or else the pipe will
			// not close when the child process exits and the ReadFile will hang.
			if (!CloseHandle(hOutputWrite))
			{
				Aqsis::log() << error << "CloseHandle" << std::endl;
				return;
			}

			if (!CloseHandle(hInputRead ))
			{
				Aqsis::log() << error << "CloseHandle" << std::endl;
				return;
			}

			if (!CloseHandle(hErrorWrite))
			{
				Aqsis::log() << error << "CloseHandle" << std::endl;
				return;
			}
		}

		/// Get the handle associated with pipe to read from.
		HANDLE pipeReadHandle()
		{
			return m_pipeReadHandle;
		}
		/// Get the handle associated with pipe to write to.
		HANDLE pipeWriteHandle()
		{
			return m_pipeWriteHandle;
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
	DWORD nBytesRead;
	if (!ReadFile(m_impl->pipeReadHandle(),s,n,&nBytesRead,NULL) || !nBytesRead)
	{
		if (GetLastError() != ERROR_BROKEN_PIPE)
			Aqsis::log() << error << "Reading child output" << std::endl; // Something bad happened.
	}
	return static_cast<std::streamsize>(nBytesRead);
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
	DWORD nBytesWritten;
	if (!WriteFile(m_impl->pipeWriteHandle(),s,n,&nBytesWritten,NULL) || !nBytesWritten)
	{
		if (GetLastError() != ERROR_BROKEN_PIPE)
			Aqsis::log() << error << "Writing child input" << std::endl; // Something bad happened.
	}
	return static_cast<std::streamsize>(nBytesWritten);
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
	if(mode == std::ios_base::in)
		if (!CloseHandle(m_impl->pipeReadHandle()))
		{
			Aqsis::log() << error << "CloseHandle" << std::endl;
			return;
		}
	else if(mode == std::ios_base::out)
		if (!CloseHandle(m_impl->pipeWriteHandle()))
		{
			Aqsis::log() << error << "CloseHandle" << std::endl;
			return;
		}
#	else // AQSIS_SYSTEM_WIN32
	if(mode == std::ios_base::in)
		::close(m_impl->pipeReadFd());
	else if(mode == std::ios_base::out)
		::close(m_impl->pipeWriteFd());
#	endif // AQSIS_SYSTEM_WIN32
}

} // namespace Aqsis
