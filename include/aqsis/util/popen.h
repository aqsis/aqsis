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


#include <aqsis/aqsis.h>

#include <string>
#include <vector>

#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/shared_ptr.hpp>

#ifndef AQSIS_POPEN_H_INCLUDED
#define AQSIS_POPEN_H_INCLUDED

namespace Aqsis
{

/** \brief Creation of a child process and bidirectional communication via pipes.
 *
 * This class provides popen-like functionality as an implementation of the
 * boost::iostreams bidirectional device concept.  This allows input and output
 * to a child process' stdout and stin respectively.  In conjunction with
 * boost::iostreams::stream and boost::iostreams::streambuf it can be used to
 * create a std::iostream compatible stream which reads and writes to pipes
 * (see TqPopenStream)
 *
 * The class is copied by the boost iostreams machinary, so should have
 * handle-semantics to the underlying pipe resources (this is handily achieved
 * as a side effect of keeping a pimpl using a boost::shared_ptr).
 */
class AQSIS_UTIL_SHARE CqPopenDevice
{
	public:
		typedef char char_type;
		struct category : boost::iostreams::bidirectional,
						  boost::iostreams::device_tag,
						  boost::iostreams::closable_tag
        { };

		/** \brief Create a child process and connect it with pipes for stdin
		 * and stdout.
		 *
		 * Throws XqEnvironment if a system call fails when creating the child
		 * process or associated pipes.
		 *
		 * \param progName - Absolute or relative path to the program, or name
		 *                   of the program to be found in the system path.
		 * \param argv - A list of arguments to the program in the argv array.
		 *               As usual, this should start with the program name
		 *               (which may be different from progName, for instance
		 *               not include the full path).
		 */
		CqPopenDevice(const std::string& progName,
				const std::vector<std::string>& argv);

		/// Read n characters from the child process stdout pipe into the buffer s.
		std::streamsize read(char_type* s, std::streamsize n);

		/// Write n characters into the child process stdin pipe from the buffer s.
		std::streamsize write(const char_type* s, std::streamsize n);

		/// Close the stdin or stdout pipes to the child process.
		void close(std::ios_base::openmode mode);
	private:
		class CqImpl;
		/// Pimpl to hide system-dependent implementation details.
		boost::shared_ptr<CqImpl> m_impl;
};


/** \brief A stream connected to a child process stdin/stdout
 *
 * Instantiating a TqPopenStream causes a child process to be created.  Data
 * can then be written to the stdin of the child process using operator<<(), and
 * read from the stdout of the child process using operator>>().  For example,
 *
 * \code
 *   TqPopenStream pipe(progName, commandLine);
 *   pipe << "write this string to the child process stdin\n";
 *
 *   // read a string from the child process stdout.  Beware that this may
 *   // block if not enough characters are available.
 *   std::string s;
 *   pipe >> s;
 * \endcode
 */
typedef boost::iostreams::stream<CqPopenDevice> TqPopenStream;


} // namespace Aqsis

#endif // AQSIS_POPEN_H_INCLUDED
