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
 * \brief File path utilities
 *
 * \author Paul C. Gregory (pgregory@aqsis.org)
 * \author Chris Foster [chris42f (at) gmail (dot) com]
 */

#ifndef FILE_H_INCLUDED
#define FILE_H_INCLUDED

#include <aqsis/aqsis.h>

#include <string>

#include <boost/filesystem/path.hpp>
#include <boost/tokenizer.hpp>

namespace Aqsis {

// Namespace alias.  boost::filesystem is far too verbose to keep repeating.
namespace boostfs = boost::filesystem;

//-----------------------------------------------------------------------
//@{
/** \brief Find a full path for the file name in the given search path.
 *
 * There are two versions of this function; one which throws an XqInvalidFile
 * exception, and one which reports nonexistant files by returning an empty
 * path.
 *
 * The search path is a list of path strings seperated by ":" or ";" (see
 * CqSearchPathsTokenFunc for details).  A file is found by first checking
 * whether it's relative to the current directory (ie, starts with ".") or is
 * absolute; in either of these cases, the file is looked up directly.  In all
 * other cases, we attempt to locate the file by prefixing fileName with each
 * path in the search path list, starting from the left.  The current directory
 * is not included in the search path by default.
 *
 * Only regular files will be found by the search; directories are excluded.
 *
 * \param fileName - path-relative name of the file.
 * \param searchPath - colon or semicolon-separated string of directories in
 *                     which to search for the given file name.
 * \return A full path to the file name.
 */
AQSIS_UTIL_SHARE boostfs::path findFile(const std::string& fileName,
		const std::string& searchPath);
AQSIS_UTIL_SHARE boostfs::path findFileNothrow(const std::string& fileName,
		const std::string& searchPath);
//@}


/** \brief expand a file pattern to a list of paths.
 *
 * Expands the given pattern string to a set of file paths which match.  The
 * meaning of the glob pattern follows the system conventions - on posix, the
 * functions from glob.h are used, while on windows the _find* functions from
 * io.h are used.
 *
 * \todo: Make this into a globbing iterator instead.
 *
 * \param pattern - file matching pattern 
 */
AQSIS_UTIL_SHARE std::vector<std::string> Glob(const std::string& pattern);

/** \brief Exapand a command-line file pattern to a list of paths
 *
 * On windows this function uses the Glob() function to expand a file pattern
 * specified on the command line.  On posix the shell does the expansion so
 * this function does nothing but return the pattern in a single element
 * vector.
 *
 * \param pattern - file matching pattern 
 */
AQSIS_UTIL_SHARE std::vector<std::string> cliGlob(const std::string& pattern);


/** \brief Splits a list of paths delimited by ';' or ':' into tokens.
 *
 * Model of the boost TokenizerFunction concept for use with boost::tokenizer.
 *
 * A search path is a set of paths with individual paths delimited by the
 * characters ':' or ';'.  On windows, using the colon as a path seperator can
 * be ambiguous with respect to drive letters like C:\.  On windows a heuristic
 * is used to attempt to identify such uses and avoid splitting at the drive
 * letter colon.
 *
 * \see TqPathsTokenizer
 */
template<typename TokenType>
class CqSearchPathsTokenFunc
{
	public:
		bool operator()(std::string::const_iterator& next,
			const std::string::const_iterator end, TokenType& tok) const;
		void reset() {};
	private:
		static bool isDelim(char c);
};

/** \brief A tokenizer type to split search paths into individual paths.
 *
 * \code
 *
 * // Example
 *
 * std::string searchPaths = ".:/home/foo/bar:../../asdf";
 * TqPathsTokenizer paths(searchPaths);
 * TqPathsTokenizer::iterator i = paths.begin();
 * boostfs::path p1 = *i;  // yeilds .
 * boostfs::path p2 = *i;  // yeilds /home/foo/bar
 * boostfs::path p3 = *i;  // yeilds ../../asdf
 *
 * \endcode
 */
typedef boost::tokenizer<CqSearchPathsTokenFunc<boostfs::path>,
		std::string::const_iterator, boostfs::path> TqPathsTokenizer;



//==============================================================================
// Implementation details
//==============================================================================
// CqSearchPathsTokenFunc implementation 

/// Determine whether c is a delimiter between paths
template<typename TokenType>
inline bool CqSearchPathsTokenFunc<TokenType>::isDelim(char c)
{
	return c == ':' || c == ';';
}

/// Split a path string into tokens at ':' or ';' characters.
template<typename TokenType>
bool CqSearchPathsTokenFunc<TokenType>::operator()(
		std::string::const_iterator& next, const std::string::const_iterator end,
		TokenType& tok) const
{
	while(next != end && isDelim(*next))
		++next;
	if(next == end)
		return false;
	// next now points to the first non-delimiter character.
	std::string::const_iterator start = next;
	while(next != end && !isDelim(*next))
		++next;
#	ifdef AQSIS_SYSTEM_WIN32
	// Heuristic to allow windows paths with drive letters to be separated by
	// colons, ie,  C:/foo/bar:../asdf  is split into C:/foo/bar and ../asdf
	// rather than the C being treated as the first path.
	if(next - start == 1 && next != end && *next == ':' && std::isalpha(*start))
	{
		++next;
		while(next != end && !isDelim(*next))
			++next;
	}
#	endif // AQSIS_SYSTEM_WIN32
	// next now points to the delimiter directly after the path.
	tok.assign(start, next);
	return true;
}


} // namespace Aqsis

#endif // !FILE_H_INCLUDED
