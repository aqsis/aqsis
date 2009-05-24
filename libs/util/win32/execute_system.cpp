// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
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
		\brief Implements the system specific parts of the CqSocket class for wrapping socket communications.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include <aqsis/util/execute.h>

#include <aqsis/util/logging.h>

#include <boost/scoped_array.hpp>
#include <sstream>

#include <windows.h>
#include <fcntl.h>
#include <io.h>

namespace Aqsis {

//---------------------------------------------------------------------
/** Default constructor.
 */
CqExecute::CqExecute(const std::string& command, const std::vector<std::string>& args, const std::string& currDir) : 
		m_command(command), m_args(args), m_currDir(currDir)
{}

void CqExecute::setStdOutCallback(CqExecute::TqCallback& callback)
{
	m_stdCallback = callback;
}


/////////////////////////////////////////////////////////////////////// 
// ReadAndHandleOutput
// Monitors handle for input. Exits when child exits or pipe breaks.
/////////////////////////////////////////////////////////////////////// 
void ReadAndHandleOutput(HANDLE hPipeRead, CqExecute::TqCallback& callBack)
{
	CHAR lpBuffer[256];
	DWORD nBytesRead;

	lpBuffer[0] = '\0';
	while(TRUE)
	{
		if (!ReadFile(hPipeRead,lpBuffer,sizeof(lpBuffer)-1,
									  &nBytesRead,NULL) || !nBytesRead)
		{
			if (GetLastError() == ERROR_BROKEN_PIPE)
				break; // pipe done - normal exit path.
			else
				Aqsis::log() << error << "Reading child output" << std::endl; // Something bad happened.
		}

		lpBuffer[nBytesRead] = '\0';
		if(!callBack.empty())
			callBack(lpBuffer);
		lpBuffer[0] = '\0';
	}
}



void CqExecute::operator()() 
{
	HANDLE hOutputReadTmp,hOutputRead,hOutputWrite;
	HANDLE hInputWriteTmp,hInputRead,hInputWrite;
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
						&hOutputRead, // Address of new handle.
						0,FALSE, // Make it uninheritable.
						DUPLICATE_SAME_ACCESS))
	{
		Aqsis::log() << error << "DuplicateHandle" << std::endl;
		return;
	}

	if (!DuplicateHandle(GetCurrentProcess(),hInputWriteTmp,
						GetCurrentProcess(),
						&hInputWrite, // Address of new handle.
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
	strCommand << "\"" << m_command << "\"";
	for(std::vector<std::string>::iterator arg = m_args.begin(); arg != m_args.end(); ++arg)
		strCommand << " \"" << *arg << "\"";
	TqInt totlen = strCommand.str().size();
	boost::scoped_array<char> command(new char[totlen+1]);
	strncpy(&command[0], strCommand.str().c_str(), totlen);
	command[totlen] = '\0';
	
	boost::scoped_array<char> dir(0);
	if(!m_currDir.empty())
	{
		dir.reset(new char[m_currDir.size()+1]);
		strncpy(&dir[0], m_currDir.c_str(), m_currDir.size());
		dir[m_currDir.size()] = '\0';
	}
	// Create the child process.
	bFuncRetn = CreateProcess(NULL,
	                          command.get(),       // command line
	                          NULL,          // process security attributes
	                          NULL,          // primary thread security attributes
	                          TRUE,          // handles are inherited
	                          0,             // creation flags
	                          NULL,          // use parent's environment
	                          dir.get(),     // use parent's current directory
	                          &si,			 // STARTUPINFO pointer
	                          &pi);			 // receives PROCESS_INFORMATION

	if (bFuncRetn == 0)
	{
		Aqsis::log() << error << "CreateProcess" << std::endl;
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

	// Read the child's output.
	ReadAndHandleOutput(hOutputRead, m_stdCallback);
	// Redirection is complete

	if (!CloseHandle(hOutputRead))
	{
		Aqsis::log() << error << "CloseHandle" << std::endl;
		return;
	}

	if (!CloseHandle(hInputWrite))
	{
		Aqsis::log() << error << "CloseHandle" << std::endl;
		return;
	}
}

} // namespace Aqsis
