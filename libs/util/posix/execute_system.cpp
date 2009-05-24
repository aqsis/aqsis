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

#include <cstring> // strncpy, strlen
#include <sstream>

#include <boost/scoped_array.hpp>

#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

#include <aqsis/util/logging.h>
#include <aqsis/util/exception.h>

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

void CqExecute::operator()() 
{
	int fd_out[2];
	int fd_in[2];
	char input[101];

	if(pipe(fd_in) || pipe(fd_out))
	{
		/// \todo: Should except here.
		Aqsis::log() << error << "Creating redirection pipes" << std::endl;
		return;	// Failure to open pipes for communication with child process.
	}
	pid_t pid = fork();
	if (pid >= 0)
	{
		if (pid == 0)
		{
			// Child process
			// Fix up the filehandles
			close(fd_out[1]);
			close(fd_in[0]);
			close(STDIN_FILENO);
			if(dup( fd_out[0] ) < 0)
			{
				/// \todo: Should except here.
				Aqsis::log() << error << "Redirecting standard file handles" << std::endl;
				return;
			}
			close(STDOUT_FILENO);
			if(dup( fd_in[1] ) < 0)
			{
				/// \todo: Should except here.
				Aqsis::log() << error << "Redirecting standard file handles" << std::endl;
				return;
			}
			// Redirect stderr to stdout
			dup2(1, 2);
			boost::scoped_array<char*> args(new char*[m_args.size()+2]);
			args[m_args.size()+1] = NULL;
			boost::scoped_array<boost::scoped_array<char> > argStore(new boost::scoped_array<char>[m_args.size()+1]);
			boost::scoped_array<char> command(new char[m_command.size()+1]);
			strncpy(&command[0], m_command.c_str(), m_command.size());
			command[m_command.size()] = '\0';
			argStore[0].reset(new char[m_command.size()+1]);
			strncpy(&(argStore[0])[0], m_command.c_str(), m_command.size());
			argStore[0][m_command.size()] = '\0';
			args[0] = &(argStore[0])[0];
			TqInt iarg = 1;
			for(std::vector<std::string>::iterator arg = m_args.begin(); arg != m_args.end(); ++arg, ++iarg)
			{
				argStore[iarg].reset(new char[arg->size()+1]);
				strncpy(&(argStore[iarg])[0], arg->c_str(), arg->size());
				argStore[iarg][arg->size()] = '\0';
				args[iarg] = &(argStore[iarg])[0];
			}
			if(chdir(m_currDir.c_str()) == -1)
			{
				AQSIS_THROW_XQERROR(XqInternal, EqE_System,
					"Could not change to directory \"" << m_currDir << "\"");
			}
			signal(SIGHUP, SIG_IGN);
			execvp(&command[0],&args[0]);
		}
		else
		{
			// Main process
			// Fix up the filehandles
			close(fd_out[0]);
			close(fd_in[1]);
			while(!waitpid(pid, 0, WNOHANG))
			{
				input[read(fd_in[0], input, 100)] = 0;
				if(strlen(input) > 0 && !m_stdCallback.empty())
					m_stdCallback(input);
			}
		}
	}
	else
	{
		/// \todo: Should except here.
		Aqsis::log() << error << "Forking child process" << std::endl;
		return;	// Failure to fork child process
	} 
}

} // namespace Aqsis
