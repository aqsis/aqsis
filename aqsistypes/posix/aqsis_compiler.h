// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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
		\brief Compiler specific options and settings.
		\author Paul C. Gregory (pgregory@aqsis.org)
		\author Timothy M. Shead (tshead@k-3d.com)
*/

//? Is .h included already?
#ifndef AQSIS_COMPILER_H_INCLUDED
#define AQSIS_COMPILER_H_INCLUDED 1

/** Define the system being compiled on.
 */
#define	AQSIS_SYSTEM_POSIX	1

/** If on a BeOS platform add this, as it is mainly Posix, but needs some changes.
 */
#ifdef __BEOS__
#define	AQSIS_SYSTEM_BEOS	1
#define	SOMAXCONN		128
#endif


/** If compiling on Apple platform, set the system identifier AQSIS_SYSTEM_MACOSX, MacOSX is basically Posix, but with some small differences.
 */
#ifdef __APPLE__
#define AQSIS_SYSTEM_MACOSX 1
#endif

/** Define the compiler.
 */
#define	AQSIS_COMPILER_GCC	1


#ifdef  NO_NAMESPACES
#define START_NAMESPACE(x)	/* start disabled namespace x */
#define END_NAMESPACE(x)	/* end disabled namespace x */
#define USING_NAMESPACE(x)  /* using disabled namespace x */
#else
#define START_NAMESPACE(x)	namespace x {
#define END_NAMESPACE(x)	}
#define USING_NAMESPACE(x)  using namespace x;
#endif

#define SHARED_LIBRARY_SUFFIX ".so"

#endif // AQSIS_COMPILER_H_INCLUDED
