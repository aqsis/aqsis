// -*- C++ -*-

/* Simple argument-parsing class
 * Copyright (C) 2001 Patrick E. Pelletier <ppelleti@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef ARGPARSE_H_INCLUDED
#define ARGPARSE_H_INCLUDED

#if	_MSC_VER
#pragma warning( disable : 4786 )
#endif	// WIN32

#include <aqsis/aqsis.h>

#include <string>
#include <vector>

class ArgParseInternalData;	// hide implementation details

/*
 * The ArgParse class allows you to specify names of options
 * that you want parsed, along with a usage message for them.
 * Options come in four flavors: flag, int, float, and string.
 * Flags don't take arguments, but the other kinds do.  For
 * an option that takes an argument, it can be specified with
 * an equals sign, with a colon, or by putting it in the next
 * element of argv.  ("--foo=stuff", "--foo:stuff", or
 * "--foo stuff", respectively)
 *
 * The flavors that take arguments also come in array flavors.
 * With an array, you specify a pointer to a vector of the basic
 * type, instead of just a pointer to a basic type.  This allows
 * the option to appear more than once, and the new values are
 * appended to the array.  Optionally, you can also specify a
 * separator character, so that multiple array elements can be
 * parsed up from a single instance of the option.
 *
 * Options can start with either a single dash or a double dash,
 * but see allowOneCharOptionsToBeCombined() for more information.
 *
 * The argument "--" all by itself signals the end of the options,
 * and everything after it will go into the leftovers (see leftovers())
 * even if it starts with a dash.
 *
 * The usage text for an option gets appended immediately after the
 * name of the option and its aliases.  The ASCII BEL character (\a)
 * can be used as a special kind of tab, which moves the cursor to
 * the current indent level (see usageHeader()), to make it easy to
 * make things line up in columns.
 */

/*
 * Rationale for some design decisions:
 *
 * Things which need to be modified are passed as pointers, not as
 * references, because some people feel that non-const references make
 * what's going on less clear, and I kind of agree with them.
 *
 * Arrays are explicitly passed as std::vector (or whatever you change
 * the typedefs below to) instead of passing around iterators and
 * allowing any STL container to be used, just because unnecessary
 * templatization can be less clear and sometimes leads to portability
 * and debugability issues, and doesn't seem justified for this simple
 * application.
 */

class AQSIS_UTIL_SHARE ArgParse
{
	public:
		// Basic types: ArgParse always uses these typedefs, so that there's
		// just one place to change things if you want to use float instead
		// of double, or CqString instead of std::string, etc.
		typedef bool apflag;
		typedef int apint;
		typedef double apfloat;
		typedef std::string apstring;

		// Array types: ditto
		typedef std::vector<apint> apintvec;
		typedef std::vector<apfloat> apfloatvec;
		typedef std::vector<apstring> apstringvec;

		// These two constants are valid values for the "separator"
		// argument to argInts, argFloats, and argStrings.  (Any ASCII
		// character is also a valid argument.)  SEP_NONE means that the
		// only way to get more than one thing in the array is to specify
		// the option more than once.  (e. g. "--foo one --foo two")
		// SEP_ARGV means that each element of argv, up to the next option,
		// will be put into the array.  (e. g. "--foo one two")  Using an
		// ASCII character means that only one argv element is used, but
		// it can become multiple elements in the array by being split
		// on the given character.  (e. g. "--foo one,two")
		enum {
		    SEP_NONE = -1,
		    SEP_ARGV = -2
		};

		ArgParse();
		~ArgParse();

		// Calling this method means that "-bar" will be treated as if
		// it was "-b -a -r", and to specify a multi-character option,
		// you need to use a double dash.  (e. g. "--bar")  If you don't
		// call this method, then single dash and double dash are treated
		// the same.
		void allowOneCharOptionsToBeCombined(); // long but descriptive :)

		// Normally, parse() will return an error if there are any
		// unrecognized options.  But if you call this method before
		// calling parse(), then unrecognized options will go into
		// the leftovers, without causing an error.
		void allowUnrecognizedOptions();

		// The following functions add an option to be parsed. Options:
		// name - option name
		// usage - usage string.  If usage contains the substring "%default",
		//         then this substring will be replaced with the default value
		//         in the corresponding value pointer
		// value(s) - pointer at where the value(s) of the option should be stored.

		// "--foo" will set *value to true.  If allow_negation is true,
		// then "--nofoo" will set *value to false.
		void argFlag(apstring name, apstring usage,
		             apflag* value, bool allow_negation = true);

		void argInt(apstring name, apstring usage, apint* value);
		void argInts(apstring name, apstring usage,
		             apintvec* values, int separator = SEP_NONE, int count = -1);

		void argFloat(apstring name, apstring usage, apfloat* value);
		void argFloats(apstring name, apstring usage,
		               apfloatvec* values, int separator = SEP_NONE, int count = -1);

		void argString(apstring name, apstring usage, apstring* value);
		void argStrings(apstring name, apstring usage,
		                apstringvec* values, int separator = SEP_NONE, int count = -1);

		// Makes "aliasname" work just like "realname".  Note that
		// "realname" can be a negated flag name (if the flag allows
		// negation), so you can make "--fooless" mean "--nofoo", for
		// example.
		void alias(apstring realname, apstring aliasname);

		// This inserts literal text into the usage message.  The order
		// is significant with respect to calls to argFlag, argInt, etc.
		// The most common use of this would be to add the
		// "Usage: blech [options] files" at the top, but it can also be
		// used to make different "sections" of options (give the --help
		// option to GNU tar to see what I mean) or to make a footer at the
		// end of the usage.  Your string can contain newlines, but it
		// shouldn't end in a newline unless you want an extra blank line.
		// The "indent" value specifies how to line up
		// columns of individual usage messages in the follow section.
		// (Again, refer to the GNU tar usage message to see what I mean.)
		void usageHeader(apstring text, int indent = 25);

		// This makes the parsing actually happen.  Returns true on
		// success, or false on failure.
		bool parse(int argc, const char** argv);

		// If parse() returns false, this method will give you an
		// error message that describes what's wrong.  Note that
		// although it's legal to call parse() more than once on the
		// same ArgParse object, only the most recent error message is
		// retained.
		apstring errmsg() const;

		// Returns a usage string made up of the usage messages of
		// the individual arguments, with things from usageHeader()
		// interspersed at the appropriate places.  The string returned
		// contains embedded newlines, and also a trailing newline.
		apstring usagemsg() const;

		// Returns any leftover arguments.  (arguments which didn't start
		// with a dash and didn't get eaten by another option, or any
		// arguments which appeared after "--")
		// Again, this is only valid until the next call to parse() on
		// this ArgParse object.
		const apstringvec& leftovers() const;

	private:
		ArgParseInternalData* d;
};

#endif // !ARGPARSE_H_INCLUDED
