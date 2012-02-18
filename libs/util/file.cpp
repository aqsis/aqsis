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

#include <aqsis/util/file.h>

#include <cctype>
#include <cstring>
#include <fstream>

#ifdef AQSIS_SYSTEM_WIN32
#	include <direct.h>
#	include <io.h>
#else
#	include <glob.h>
#endif

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <aqsis/util/exception.h>

namespace Aqsis {


//------------------------------------------------------------------------------
// findFile implementations

boostfs::path findFile(const std::string& fileName,
		const std::string& searchPath)
{
	boostfs::path loc = findFileNothrow(fileName, searchPath);
	if(loc.empty())
	{
		AQSIS_THROW_XQERROR(XqInvalidFile, EqE_NoFile,
			"Could not find file \"" << fileName << "\" in path: \""
			<< searchPath << "\"");
	}
	return loc;
}

namespace {
	// Wrapper around boostfs::is_regular which ignores exceptions occurring in
	// the underlying file layer.  I'm not sure if this is the right thing to
	// do (it masks permissions problems for example), but it allows
	// findFile*() to continue searching other paths in the search path set in
	// the case that one of them is inaccessible.
	bool isRegularFile(boostfs::path filePath)
	{
		try
		{
			return is_regular(filePath);
		}
		catch(boostfs::filesystem_error& e)
		{ }
		return false;
	}
}

boostfs::path findFileNothrow(const std::string& fileName,
		const std::string& searchPath)
{
	boostfs::path filePath = fileName;
	if(filePath.empty())
		return boostfs::path();
	// First check whether the path is complete (often called "absolute") or is
	// relative to the current directory.
	if(filePath.is_complete() || *filePath.begin() == "." || *filePath.begin() == "..")
	{
		if(isRegularFile(filePath))
			return filePath;
		else
			return boostfs::path();
	}
	// In other cases, look for fileName in each path of the searchpath.
	TqPathsTokenizer paths(searchPath);
	for(TqPathsTokenizer::iterator i = paths.begin(), end = paths.end();
			i != end; ++i)
	{
		boostfs::path candidate = (*i)/filePath;
		if(isRegularFile(candidate))
			return candidate;
	}
	return boostfs::path();
}


/** \brief Fudge MTOR paths like //C/foo/bar into C:/foo/bar for windows
 *
 * This is a fudge for search paths produced by pixar's RAT on windows; MTOR
 * seems to produce paths like //C/foo/bar rather than the standard C:/foo/bar.
 * This function translates such paths.
 *
 * \param originalPath - previous value of the path.
 * \return A path with all instacnes of //<drive_letter>/ changed to
 *         <drive_letter>:/ on windows.  On posix, just return originalPath.
 */
inline static std::string mtorPathFudge(const std::string& originalPath)
{
#	ifdef AQSIS_SYSTEM_WIN32
	if(originalPath.find("//") != std::string::npos)
	{
		std::string fixedPath;
		typedef boost::tokenizer<CqSearchPathsTokenFunc<std::string>,
				std::string::const_iterator, std::string> TqPathsStrTokenizer;
		TqPathsStrTokenizer paths(originalPath);
		for(TqPathsStrTokenizer::iterator path = paths.begin(), end = paths.end();
				path != end; ++path)
		{
			if(!fixedPath.empty())
				fixedPath += ';'; // add a path separator.
			if(path->find("//", 0, 2) == 0 && path->length() >= 4 && (*path)[3] == '/')
			{
				fixedPath += (*path)[2];
				fixedPath += ":/";
				fixedPath += path->substr(4);
			}
			else
				fixedPath += *path;
		}
		return fixedPath;
	}
#	endif // AQSIS_SYSTEM_WIN32
	return originalPath;
}

std::string expandSearchPath(const std::string& newPath,
							 const std::string& oldPath,
							 const std::string& defaultPath)
{
	// Build the string, checking for & and @ characters  and replace with old
	// and default string, respectively.
	std::string::size_type strt = 0;
	std::string::size_type match = 0;
	std::string expandedPath;
	while ( 1 )
	{
		if ( ( match = newPath.find_first_of("&@%", strt ) ) != std::string::npos )
		{
			expandedPath += newPath.substr( strt, (match-strt) );
			if( newPath[match] == '&' )
			{
				// replace & with the old path
				expandedPath += oldPath;
				strt = match + 1;
			}
			else if( newPath[match] == '@' )
			{
				// replace @ with the default path
				expandedPath += defaultPath;
				strt = match + 1;
			}
			else if( newPath[match] == '%' )
			{
				// replace %var% with environment variable %var%
				std::string::size_type envvarend = newPath.find('%', match + 1);
				if( envvarend != std::string::npos )
				{
					std::string strEnv = newPath.substr(match + 1, (envvarend - (match + 1)));
					const char* strVal = getenv(strEnv.c_str());
					if(strVal)
						expandedPath += strVal;
					strt = envvarend + 1;
				}
			}
		}
		else
		{
			expandedPath += newPath.substr(strt);
			break;
		}
	}
	expandedPath = mtorPathFudge(expandedPath);
	return expandedPath;
}


// Glob function implementations

#ifdef AQSIS_SYSTEM_WIN32
// windows globbing implementation

std::vector<std::string> Glob( const std::string& pattern )
{
	_finddata_t c_file;
	long hFile;
	const char *pt = pattern.c_str();

	char drive[_MAX_PATH];
	char dir[_MAX_PATH];
	char fname[_MAX_PATH];
	char ext[_MAX_PATH];

	_splitpath( pt, drive, dir, fname, ext);

	std::string strPath(drive);
	strPath += dir;

	std::vector<std::string> result;
	if ( ( hFile = _findfirst( pt, &c_file ) ) != -1L )
	{
		// we found something here; then we list
		// all of them with the directory first
		result.push_back(strPath + c_file.name);
		while ( _findnext( hFile, &c_file ) == 0 )
			result.push_back(strPath + c_file.name);
		_findclose( hFile );
	}

	return result;
}

std::vector<std::string> cliGlob( const std::string& pattern )
{
	return Glob(pattern);
}

#else // AQSIS_SYSTEM_WIN32

// posix globbing implementation
std::vector<std::string> Glob( const std::string& pattern )
{
	glob_t globbuf;
	globbuf.gl_offs = 0;
	glob( pattern.c_str(), GLOB_DOOFFS, NULL, &globbuf );
	std::vector<std::string> result;
	result.reserve(globbuf.gl_pathc);
	for(size_t i = 0; i < globbuf.gl_pathc; ++i)
		result.push_back(globbuf.gl_pathv[i]);

	globfree( &globbuf );
	return result;
}

std::vector<std::string> cliGlob(const std::string& pattern)
{
	// Under unix, the shell takes care of command-line globbing.
	return std::vector<std::string>(1, pattern);
}

#endif // AQSIS_SYSTEM_WIN32

// Define BOOST_FILESYSTEM_VERSION for convenience; older boost versions don't
// define this for us.
#ifndef BOOST_FILESYSTEM_VERSION
#	define BOOST_FILESYSTEM_VERSION 2
#endif

std::string native(const boostfs::path& path)
{
#if BOOST_FILESYSTEM_VERSION == 3
	return path.string();
#elif BOOST_FILESYSTEM_VERSION == 2
	return path.file_string();
#endif
}


std::string filename(const boostfs::path& path)
{
#if BOOST_FILESYSTEM_VERSION == 3
	return path.filename().string();
#elif BOOST_FILESYSTEM_VERSION == 2
	return path.leaf();
#endif
}

} // namespace Aqsis
