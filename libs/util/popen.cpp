// Aqsis
// Copyright (C) 2001, Paul C. Gregory and the other authors and contributors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the software's owners nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// (This is the New BSD license)

/** \file
 *
 * \brief Utilities for opening a subprocess with connected pipes.
 *
 * \author Chris Foster [ chris42f (at) g mail (dot) com ]
 */

#include <aqsis/util/popen.h>

#include <errno.h>

#ifdef AQSIS_SYSTEM_WIN32
#	include <windows.h>
#else
#	include <cstring> // for strerror()
#	include <fcntl.h>
#	include <unistd.h>
#	include <signal.h>
#endif // AQSIS_SYSTEM_WIN32

#include <boost/scoped_array.hpp>

#include <aqsis/util/exception.h>
#include <aqsis/util/logging.h>

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

		void close(std::ios_base::openmode mode)
		{
			if(mode == std::ios_base::in)
			{
				if (!CloseHandle(m_pipeReadHandle))
				{
					Aqsis::log() << error << "CloseHandle" << std::endl;
					return;
				}
			}
			else if(mode == std::ios_base::out)
			{
				if (!CloseHandle(m_pipeWriteHandle))
				{
					Aqsis::log() << error << "CloseHandle" << std::endl;
					return;
				}
			}
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
		 * This function is called _after_ fork() by the child process.
		 * It connects the new process' stdin and stdout to the provided file
		 * descriptors corresponding to pipes back to the parent process.
		 *
		 * \param newStdin - file descriptor to connect to stdin
		 * \param newStdout - file descriptor to connect to stdout
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

		/** Call std::exit() from the child process, writing the error message
		 * to the error pipe.
		 */
		static void errorExit(int errorPipeFd, const char* error)
		{
			std::string errorStr = error;
			errorStr += ": ";
			errorStr += std::strerror(errno);
			::write(errorPipeFd, errorStr.c_str(), errorStr.length());
			std::exit(EXIT_FAILURE);
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
					"Could not create pipe");
			}
			if(pipe(childStdoutFd) == -1)
			{
				::close(childStdinFd[0]);
				::close(childStdinFd[1]);
				AQSIS_THROW_XQERROR(XqEnvironment, EqE_System,
					"Could not create pipe");
			}
			// special pipe for error reporting from the child
			int errorPipe[2];
			if(::pipe(errorPipe) == -1)
			{
				::close(childStdinFd[0]);
				::close(childStdinFd[1]);
				::close(childStdoutFd[0]);
				::close(childStdoutFd[1]);
				AQSIS_THROW_XQERROR(XqEnvironment, EqE_System,
					"Could not create pipe");
			}
			const int childRead = childStdinFd[0];
			const int parentWrite = childStdinFd[1];
			const int childWrite = childStdoutFd[1];
			const int parentRead = childStdoutFd[0];
			const int childErrorWrite = errorPipe[1];
			const int parentErrorRead = errorPipe[0];

			// Ignore all broken-pipe signals; catching and using these to
			// report would be difficult at best...
			::signal(SIGPIPE, SIG_IGN);
			if(pid_t pid = ::fork())
			{
				//----------------------------------------
				// Parent process
				if(pid == -1)
				{
					// Could not create child; close fd's and throw.
					::close(childRead);       ::close(childWrite);
					::close(parentRead);      ::close(parentWrite);
					::close(childErrorWrite); ::close(parentErrorRead);
					AQSIS_THROW_XQERROR(XqEnvironment, EqE_System,
						"could not fork child process");
				}
				// Close the child ends of the pipe 
				::close(childRead);
				::close(childWrite);
				::close(childErrorWrite);
				// Read any error from the child process.  If the child process
				// execvp() is successful the child end of the pipe will be
				// closed and ::read() will return here with zero bytes read.
				const int bufSize = 256;
				char errBuf[bufSize+1];
				int nRead = 0;
				while((nRead = ::read(parentErrorRead, errBuf, bufSize))
						== -1 && errno == EINTR);
				errBuf[nRead] = 0;
				if(nRead > 0)
					AQSIS_THROW_XQERROR(XqEnvironment, EqE_System, errBuf);
				::close(parentErrorRead);
				// Save the file descriptors connected to the child process
				// stdin and stdout.
				m_pipeReadFd = parentRead;
				m_pipeWriteFd = parentWrite;
			}
			else
			{
				//----------------------------------------
				// Child process
				// Close the parent ends of the pipes
				::close(parentRead);
				::close(parentWrite);
				::close(parentErrorRead);
				// Set the error pipe to auto-close if execvp() succeeds
				if(int flags = ::fcntl(childErrorWrite, F_GETFD) >= 0)
				{
					flags |= FD_CLOEXEC;
					if(::fcntl(childErrorWrite, F_SETFD, flags) == -1)
						errorExit(childErrorWrite, "Could not set error pipe mode");
				}
				else
					errorExit(childErrorWrite, "Could not set error pipe mode");
				// Connect the process stdin and stdout to the pipes
				if(!connectStdInOut(childRead, childWrite))
					errorExit(childErrorWrite, "Could not connect to child process");

				// Copy argument list into a C-style array of raw pointers.
				TqInt argc = argv.size();
				boost::scoped_array<char*> argvRaw(new char*[argc + 1]);
				for(TqInt i = 0; i < argc; ++i)
					argvRaw[i] = const_cast<char*>(argv[i].c_str());
				// Terminate the argv list with a null.
				argvRaw[argc] = 0;
				// Execute the child program in a new process.
				::execvp(progName.c_str(), &argvRaw[0]);

				// We only ever get here if there's an execvp() error.
				errorExit(childErrorWrite, "Could not execute child process");
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

		/// Close the incoming or outgoing pipe depending on the given mode.
		void close(std::ios_base::openmode mode)
		{
			if(mode == std::ios_base::in && m_pipeReadFd != -1)
			{
				::close(m_pipeReadFd);
				// Set the pipe file descriptor to something invalid after
				// closing.  This means that 
				m_pipeReadFd = -1;
			}
			else if(mode == std::ios_base::out && m_pipeWriteFd != -1)
			{
				::close(m_pipeWriteFd);
				m_pipeWriteFd = -1;
			}
		}

		~CqImpl()
		{
			// Make sure that the pipe ends are correctly closed on
			// destruction.
			this->close(std::ios_base::in);
			this->close(std::ios_base::out);
		}
};
#endif // !AQSIS_SYSTEM_WIN32

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
	assert(m_impl->pipeReadFd() != -1);
	std::streamsize nRead = 0;
	// Read from pipe, igorning interrupts due to signals.
	while( (nRead = ::read(m_impl->pipeReadFd(), s, n)) == -1 && errno == EINTR );
	if(nRead == -1)
		// TODO: Reconsider which exception this should be.
		throw std::ios_base::failure("Bad read from pipe");
	return nRead == 0 ? -1 : nRead;
#	endif // !AQSIS_SYSTEM_WIN32
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
	assert(m_impl->pipeWriteFd() != -1);
	std::streamsize nWrite = 0;
	// Write to pipe, ignoring any interrupts due to signals.
	while( (nWrite = ::write(m_impl->pipeWriteFd(), s, n)) == -1 && errno == EINTR );
	if(nWrite < n)
		// TODO: Reconsider which exception this should be.
		throw std::ios_base::failure("Bad write to pipe");
	return nWrite;
#	endif // !AQSIS_SYSTEM_WIN32
}

void CqPopenDevice::close(std::ios_base::openmode mode)
{
	m_impl->close(mode);
}

} // namespace Aqsis
