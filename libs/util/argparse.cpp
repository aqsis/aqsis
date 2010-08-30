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

#include <aqsis/util/argparse.h>
#include <map>
#include <list>
#include <functional>
#include <sstream>
#include <cstdlib>

typedef std::list<ArgParse::apstring> StringList;

class OptionHandler
{
	public:
		ArgParse::apstring usage;
		int separator;
		int count;
		StringList aliases;

		OptionHandler(ArgParse::apstring usage_in,
		              int separator_in = ArgParse::SEP_NONE, int count_in = -1);
		virtual ~OptionHandler();
		// Returns true if the option takes an argument.  This is the
		// same thing as saying it returns false if the option is a flag.
		// (The implementation in the base class returns true, so we
		// only have to override it for FlagHandler.)
		virtual bool takesarg() const;
		// Returns true if we can stick "no" in front of the option.
		// This will be false for everything except flag, and might be
		// true or false for flag.
		// (The implementation in the base class returns false, so we
		// only have to override it for FlagHandler.)
		virtual bool allownegation() const;
		// Resets the internal state of the OptionHandler, so that
		// parse() can be called more than once.
		// (This has a no-op implementation in the base class,
		// because not all subclasses need to override it.)
		virtual void reset();
		// This is a wrapper for handlearg() which first splits up the
		// argument according to "separator".  As an additional, unrelated
		// function, it also will append "prefix" to any non-empty error message.
		ArgParse::apstring handleargsplit(ArgParse::apstring arg,
		                                  ArgParse::apstring prefix);
	protected:
		// Takes an argument and sticks it somewhere.  Returns the empty
		// string on success, or an error message on failure.  If
		// takesarg() is false, then arg is either "no" or the empty string.
		virtual ArgParse::apstring handlearg(ArgParse::apstring arg) = 0;
		// Replace first substring named defaultReplacementStr with the value
		// pointed to by defaultVal.  Used for automatic interpolation of
		// default values into the usage strings.
		template<typename T>
		static ArgParse::apstring replaceDefault(ArgParse::apstring usage,
												 T* defaultVal);
		template<typename T>
		static ArgParse::apstring replaceDefault(ArgParse::apstring usage,
												 std::vector<T>* defaultVal);
	private:
		static const ArgParse::apstring defaultReplacementStr;
};
const ArgParse::apstring OptionHandler::defaultReplacementStr = "%default";

// If first is nonnegative, then this is a usage header, and second is
// the text of the header, and first is the indent.  If first is
// negative, then second is the name of an option.
typedef std::pair<int, ArgParse::apstring> UsageInfo;

typedef std::map<ArgParse::apstring, OptionHandler*> OptionHandlerMap;
typedef std::pair<ArgParse::apstring, OptionHandler*> OptionHandlerPair;

typedef std::list<UsageInfo> UsageInfoList;

typedef std::map<ArgParse::apstring, ArgParse::apstring> StringMap;
typedef std::pair<ArgParse::apstring, ArgParse::apstring> StringPair;

class ArgParseInternalData
{
	public:
		bool allowOneCharOptionsToBeCombined;
		bool allowUnrecognizedOptions;
		ArgParse::apstring errmsg;
		ArgParse::apstringvec leftovers;
		OptionHandlerMap options;
		StringMap aliases;
		UsageInfoList usage;

		void addOption(ArgParse::apstring name, OptionHandler* oh);
		OptionHandler* findOption(ArgParse::apstring name, bool* no);
};

void ArgParseInternalData::addOption(ArgParse::apstring name,
                                     OptionHandler* oh)
{
	if (options.find(name) != options.end())
	{
		/* Okay, they registered the same option name twice.
		 * We don't have a good way to handle this error.
		 * We could throw an exception, or we could just
		 * ignore it.  I'm going to pick the latter. */
		delete oh;		// since we can't store it anywhere
	}
	else
	{
		options.insert(OptionHandlerPair(name, oh));
		usage.push_back(UsageInfo(-1, name));
	}
}

OptionHandler* ArgParseInternalData::findOption(ArgParse::apstring name,
        bool* no)
{
	// First check if it's an alias.
	StringMap::iterator foundalias = aliases.find(name);
	ArgParse::apstring newname = name;
	if (foundalias != aliases.end())
		newname = foundalias->second;
	*no = false;
	// Now try to find it by its real name
	OptionHandlerMap::iterator foundoption = options.find(newname);
	if (foundoption != options.end())
		return foundoption->second;
	// If not found, see if we can find it by stripping "no"
	if (newname.substr(0, 2) == "no")
	{
		newname = newname.substr(2);
		foundalias = aliases.find(newname);
		if (foundalias != aliases.end())
			newname = foundalias->second;
		foundoption = options.find(newname);
		if (foundoption != options.end())
		{
			// We found it, but is "no" legal for this option?
			OptionHandler* oh = foundoption->second;
			if (oh->allownegation())
			{
				*no = true;
				return oh;
			}
		}
	}
	return NULL;
}


ArgParse::ArgParse()
{
	d = new ArgParseInternalData;
	d->allowOneCharOptionsToBeCombined = false;
	d->allowUnrecognizedOptions = false;
}

ArgParse::~ArgParse()
{
	/* Most of the stuff will clean itself up automatically,
	 * but since d->options contains pointers we allocated,
	 * we need to free them explicitly. */
	for (OptionHandlerMap::iterator e = d->options.begin();
	        e != d->options.end(); e++)
	{
		delete e->second;
		e->second = NULL;
	}
	delete d;
	d = NULL;
}

void ArgParse::allowOneCharOptionsToBeCombined()
{
	d->allowOneCharOptionsToBeCombined = true;
}

void ArgParse::allowUnrecognizedOptions()
{
	d->allowUnrecognizedOptions = true;
}

void ArgParse::alias(apstring realname, apstring aliasname)
{
	if (d->aliases.find(aliasname) == d->aliases.end())
	{
		d->aliases.insert(StringPair(aliasname, realname));
		OptionHandlerMap::iterator e = d->options.find(realname);
		if (e != d->options.end())
		{
			e->second->aliases.push_back(aliasname);
		}
	}
}

void ArgParse::usageHeader(apstring text, int indent)
{
	d->usage.push_back(UsageInfo(indent, text));
}

ArgParse::apstring ArgParse::errmsg() const
{
	return d->errmsg;
}

const ArgParse::apstringvec& ArgParse::leftovers() const
{
	return d->leftovers;
}

bool ArgParse::parse(int argc, const char** argv)
{
	OptionHandler* argeater = NULL;
	OptionHandler* oneshotargeater = NULL;
	bool endofoptions = false;
	const char* eatername = "";

	// These statments are to reset things so we can call
	// parse() more than once.  This probably isn't a very
	// useful feature, though, so maybe it should be taken
	// out to simplify things.
	d->leftovers.clear();
	for (OptionHandlerMap::iterator e = d->options.begin();
	        e != d->options.end(); e++)
	{
		e->second->reset();
	}
	d->errmsg = "";

	for (int i = 0; i < argc; i++)
	{
		if (d->errmsg.size() > 0)
			break;
		if (endofoptions)
		{
			d->leftovers.push_back(argv[i]);
			continue;
		}
		if (oneshotargeater != NULL)
		{
			d->errmsg = oneshotargeater->handleargsplit(argv[i], eatername);
			oneshotargeater = NULL;
			continue;
		}
		// The argv[i][1] == '\0' part below is so we treat "-"
		// by itself as a regular argument, not as an option,
		// since it's a common abbreviation for stdin, and isn't
		// really meaningful as an option.
		if (( argv[i][0] != '-' || argv[i][1] == '\0' ) &&
		        ( !argeater || ( argeater->count > 0 || argeater->count == -1 ) ) )
		{
			if (argeater == NULL)
				d->leftovers.push_back(argv[i]);
			else
				d->errmsg = argeater->handleargsplit(argv[i], eatername);
			continue;
		}
		// At this point, we know the first character is a dash
		argeater = NULL;
		const char* rest = argv[i] + 1;
		if (*rest == '-')
		{
			rest++;
			if (*rest == '\0')
			{ // just "--" by itself
				endofoptions = true;
				continue;
			}
		}
		int ndashes = rest - argv[i];
		int l;
		for (l = 0; rest[l] != '\0' && rest[l] != ':' && rest[l] != '='; l++)
			;			// empty; all we do is compute l
		if (d->allowOneCharOptionsToBeCombined && ndashes == 1 && l > 1)
		{
			/* Do all the bundled options except the last one.
			 * We will let the last one fall through to the regular
			 * logic.  This is because the last option could take
			 * an argument, but the others can't. */
			for (int j = 0; j < l - 1; j++)
			{
				apstring onechar(1, rest[j]);
				bool no;
				OptionHandler* oh = d->findOption(onechar, &no);
				if (oh == NULL)
				{
					d->errmsg = (apstring(argv[i]) + ": '" + onechar +
					             "' is an unrecognized option");
					/* if only this was Shading Language and I could
					 * say "break 2"... */
					goto doublebreak;
				}
				if (oh->takesarg())
				{
					d->errmsg = (apstring(argv[i]) + ": '" + onechar +
					             "' requires an argument");
					goto doublebreak;
				}
				d->errmsg = oh->handleargsplit(no ? "no" : "", argv[i]);
				if (d->errmsg.size() > 0)
					goto doublebreak;
			}
			rest += l - 1;
			l = 1;
		}
		bool no;
		apstring option_name(rest, l);
		OptionHandler* oh = d->findOption(option_name, &no);
		if (oh == NULL)
		{
			// See if there is an option which matches the first character and takes and
			// argument, if so, match it as an option/argument pair.
			apstring onechar(rest, 1);
			bool no;
			oh = d->findOption(onechar, &no);
			if( oh != NULL && oh->takesarg())
			{
				// Setup the lenght to point to the remainder.
				l = 0;
			}
			else
			{
				if (d->allowUnrecognizedOptions)
				{
					d->leftovers.push_back(argv[i]);
					continue;
				}
				else
				{
					d->errmsg = apstring(argv[i]) + ": unrecognized option";
					break;
				}
			}
		}
		if (oh->takesarg())
		{
			if (rest[l] != '\0')
			{
				d->errmsg = oh->handleargsplit(rest + l + 1, argv[i]);
			}
			else if (oh->separator != SEP_ARGV)
			{
				oneshotargeater = oh;
			}
			if (oh->separator == SEP_ARGV)
			{
				argeater = oh;
			}
			eatername = argv[i];
		}
		else
		{
			if (rest[l] != '\0')
			{
				d->errmsg = apstring(argv[i]) + ": doesn't take an argument";
			}
			else
			{
				d->errmsg = oh->handleargsplit (no ? "no" : "", argv[i]);
			}
		}
	}
doublebreak:
	if (d->errmsg.empty() && oneshotargeater != NULL)
		d->errmsg = "missing an argument at end of command line";
	return d->errmsg.empty();
}

// This STL function object can be used to sort strings by length.
class CompareByLength :
			//public std::binary_function<ArgParse::apstring, ArgParse::apstring, bool>
			public std::greater<ArgParse::apstring>
{
	public:
		result_type operator()(const ArgParse::apstring& a, const ArgParse::apstring b)
		{
			if (a.size() < b.size())
				return true;
			if (a.size() > b.size())
				return false;
			// Same length, so fall back to lexicographic ordering
			return (a < b);
		}
};

ArgParse::apstring ArgParse::usagemsg() const
{
	apstring ret;
	int indent = 25;

	for (UsageInfoList::iterator e = d->usage.begin();
	        e != d->usage.end(); e++)
	{
		if (e->first >= 0)
		{
			// Add the usage text verbatim
			indent = e->first;
			ret += e->second;
			ret += '\n';
		}
		else
		{
			// Add usage info for the option named in e->second.
			OptionHandlerMap::iterator foundoption = d->options.find(e->second);
			if (foundoption == d->options.end())
			{
				// If this happens, something is wrong.
				continue;
			}
			OptionHandler* oh = foundoption->second;
			// First step is to make a list of all of its names
			// (all the aliases, plus the real name)
			StringList names = oh->aliases;
			names.push_back(e->second);
			// Now sort the list by length, so the shortest is first
			names.sort(CompareByLength());
			bool first = true;
			apstring line = "  ";
			for (StringList::iterator it = names.begin();
			        it != names.end(); it++)
			{
				if (first)
				{
					if (d->allowOneCharOptionsToBeCombined && it->size() != 1)
						line += "    ";
				}
				else
				{
					line += ", ";
				}
				first = false;
				if (d->allowOneCharOptionsToBeCombined && it->size() != 1)
					line += '-';
				line += '-';
				line += *it;
			}
			for (apstring::iterator it2 = oh->usage.begin();
			        it2 != oh->usage.end(); it2++)
			{
				if (*it2 == '\n')
				{
					ret += line;
					ret += '\n';
					line = "";
				}
				else if (*it2 == '\a')
				{
					int spaces = indent - line.size();
					if (spaces < 1)
					{
						/* break onto a new line if we're already past
						 * the indent */
						ret += line;
						ret += '\n';
						line = "";
						spaces = indent;
					}
					line.append(spaces, ' ');
				}
				else
				{
					line += *it2;
				}
			}
			ret += line;
			ret += '\n';
		}
	}
	return ret;
}

/*-********************************************************************-*/

static ArgParse::apstring parseInt(ArgParse::apstring arg,
                                   ArgParse::apint* value)
{
	char* endptr;
	ArgParse::apint newvalue = (ArgParse::apint) strtol(arg.c_str(),
	                           &endptr, 0);
	if (*endptr != '\0' || arg.length() == 0)
		return ArgParse::apstring("\"") + arg + "\" is not a valid integer";
	*value = newvalue;
	return "";
}

static ArgParse::apstring parseFloat(ArgParse::apstring arg,
                                     ArgParse::apfloat* value)
{
	char* endptr;
	ArgParse::apfloat newvalue = (ArgParse::apfloat) strtod(arg.c_str(),
	                             &endptr);
	if (*endptr != '\0' || arg.length() == 0)
		return (ArgParse::apstring("\"") + arg +
		        "\" is not a valid floating-point number");
	*value = newvalue;
	return "";
}

/*-********************************************************************-*/

OptionHandler::OptionHandler(ArgParse::apstring usage_in, int separator_in, int count_in) :
		usage(usage_in), separator(separator_in), count(count_in)
{}

OptionHandler::~OptionHandler()
{}

bool OptionHandler::takesarg() const
{
	return true;
}

bool OptionHandler::allownegation() const
{
	return false;
}

void OptionHandler::reset()
{}

ArgParse::apstring OptionHandler::handleargsplit(ArgParse::apstring arg,
        ArgParse::apstring prefix)
{
	if (separator < 0)
	{
		// For SEP_NONE or SEP_ARGV, we have no extra work to do here.
		// (Except to add the prefix)
		ArgParse::apstring ret = handlearg(arg);
		if (ret.empty())
			return "";
		return prefix + ": " + ret;
	}

	for (ArgParse::apstring::size_type i = 0; i < arg.size(); i++)
	{
		int newi = arg.find(separator, i);
		if (newi < 0)
			newi = arg.size();
		ArgParse::apstring ret = handlearg(arg.substr(i, newi - i));
		if (!ret.empty())
			return prefix + ": " + ret;
		i = newi;
	}
	return "";
}

template<typename T>
ArgParse::apstring OptionHandler::replaceDefault(ArgParse::apstring usage,
												 T* defaultVal)
{
	ArgParse::apstring::size_type pos = usage.find(defaultReplacementStr);
	if(pos != ArgParse::apstring::npos)
	{
		std::ostringstream formatStream;
		formatStream << *defaultVal;
		usage.replace(pos, defaultReplacementStr.size(), formatStream.str());
	}
	return usage;
}

// specialised version to deal with pointers to vectors.
template<typename T>
ArgParse::apstring OptionHandler::replaceDefault(
		ArgParse::apstring usage, std::vector<T>* defaultVal)
{
	ArgParse::apstring::size_type pos = usage.find(defaultReplacementStr);
	if(pos != ArgParse::apstring::npos)
	{
		std::ostringstream formatStream;
		for(typename std::vector<T>::iterator i = defaultVal->begin(),
				e = defaultVal->end(); i != e; i++)
		{
			formatStream << *i;
			if(i != e-1)
				formatStream << ",";
		}
		usage.replace(pos, defaultReplacementStr.size(), formatStream.str());
	}
	return usage;
}


// Okay, this is the point in the file where it starts getting
// pretty repetitive.  I'm sure there's a better way I could've
// done this...

/*-********************************************************************-*/

class FlagHandler : public OptionHandler
{
	public:
		bool allow_negation;
		bool already_used;
		ArgParse::apflag* value;

		FlagHandler(ArgParse::apstring usage_in, ArgParse::apflag* value_in,
		            bool allow_negation_in);
		bool takesarg() const;
		bool allownegation() const;
		void reset();
		ArgParse::apstring handlearg(ArgParse::apstring arg);
};

FlagHandler::FlagHandler(ArgParse::apstring usage_in,
                         ArgParse::apflag* value_in,
                         bool allow_negation_in) :
		OptionHandler(replaceDefault(usage_in, value_in)),
		allow_negation(allow_negation_in),
		already_used(false), value(value_in)
{}

bool FlagHandler::takesarg() const
{
	return false;
}

bool FlagHandler::allownegation() const
{
	return allow_negation;
}

void FlagHandler::reset()
{
	already_used = false;
}

ArgParse::apstring FlagHandler::handlearg(ArgParse::apstring arg)
{
	ArgParse::apflag newvalue = (arg.size() == 0);
	if (already_used)
	{
		// If they specified the same flag before, let them
		// get away with it.  Only complain if one is negated
		// and one is not.
		if (newvalue != *value)
			return "negated flag used with non-negated flag";
	}
	else
	{
		*value = newvalue;
		already_used = true;
	}
	return "";
}

void ArgParse::argFlag(apstring name, apstring usage,
                       apflag* value, bool allow_negation)
{
	d->addOption(name, new FlagHandler(usage, value, allow_negation));
}

/*-********************************************************************-*/

class IntHandler : public OptionHandler
{
	public:
		bool already_used;
		ArgParse::apint* value;

		IntHandler(ArgParse::apstring usage_in, ArgParse::apint* value_in);
		void reset();
		ArgParse::apstring handlearg(ArgParse::apstring arg);
};

IntHandler::IntHandler(ArgParse::apstring usage_in,
                       ArgParse::apint* value_in) :
		OptionHandler(replaceDefault(usage_in, value_in)),
		already_used(false), value(value_in)
{}

void IntHandler::reset()
{
	already_used = false;
}

ArgParse::apstring IntHandler::handlearg(ArgParse::apstring arg)
{
	if (already_used)
	{
		return "option specified more than once";
	}
	already_used = true;
	return parseInt(arg, value);
}

void ArgParse::argInt(apstring name, apstring usage, apint* value)
{
	d->addOption(name, new IntHandler(usage, value));
}

/*-********************************************************************-*/

class FloatHandler : public OptionHandler
{
	public:
		bool already_used;
		ArgParse::apfloat* value;

		FloatHandler(ArgParse::apstring usage_in, ArgParse::apfloat* value_in);
		void reset();
		ArgParse::apstring handlearg(ArgParse::apstring arg);
};

FloatHandler::FloatHandler(ArgParse::apstring usage_in,
                           ArgParse::apfloat* value_in) :
		OptionHandler(replaceDefault(usage_in,value_in)),
		already_used(false), value(value_in)
{}

void FloatHandler::reset()
{
	already_used = false;
}

ArgParse::apstring FloatHandler::handlearg(ArgParse::apstring arg)
{
	if (already_used)
	{
		return "option specified more than once";
	}
	already_used = true;
	return parseFloat(arg, value);
}

void ArgParse::argFloat(apstring name, apstring usage, apfloat* value)
{
	d->addOption(name, new FloatHandler(usage, value));
}

/*-********************************************************************-*/

class StringHandler : public OptionHandler
{
	public:
		bool already_used;
		ArgParse::apstring* value;

		StringHandler(ArgParse::apstring usage_in, ArgParse::apstring* value_in);
		void reset();
		ArgParse::apstring handlearg(ArgParse::apstring arg);
};

StringHandler::StringHandler(ArgParse::apstring usage_in,
                             ArgParse::apstring* value_in) :
		OptionHandler(replaceDefault(usage_in, value_in)),
		already_used(false), value(value_in)
{}

void StringHandler::reset()
{
	already_used = false;
}

ArgParse::apstring StringHandler::handlearg(ArgParse::apstring arg)
{
	if (already_used)
	{
		return "option specified more than once";
	}
	already_used = true;
	*value = arg;
	return "";
}

void ArgParse::argString(apstring name, apstring usage, apstring* value)
{
	d->addOption(name, new StringHandler(usage, value));
}

/*-********************************************************************-*/

class IntsHandler : public OptionHandler
{
	public:
		ArgParse::apintvec* value;

		IntsHandler(ArgParse::apstring usage_in, ArgParse::apintvec* value_in,
		            int separator_in, int count_in);
		ArgParse::apstring handlearg(ArgParse::apstring arg);
};

IntsHandler::IntsHandler(ArgParse::apstring usage_in,
                         ArgParse::apintvec* value_in, int separator_in, int count_in) :
		OptionHandler(replaceDefault(usage_in, value_in),
		separator_in, count_in), value(value_in)
{}

ArgParse::apstring IntsHandler::handlearg(ArgParse::apstring arg)
{
	ArgParse::apint newvalue;
	ArgParse::apstring ret = parseInt(arg, &newvalue);
	if (ret.size() == 0)
	{
		value->push_back(newvalue);
		count -= (count == -1)? 0 : 1;
	}
	return ret;
}

void ArgParse::argInts(apstring name, apstring usage,
                       apintvec* values, int separator, int count)
{
	d->addOption(name, new IntsHandler(usage, values, separator, count));
}

/*-********************************************************************-*/

class FloatsHandler : public OptionHandler
{
	public:
		ArgParse::apfloatvec* value;

		FloatsHandler(ArgParse::apstring usage_in, ArgParse::apfloatvec* value_in,
		              int separator_in, int count);
		ArgParse::apstring handlearg(ArgParse::apstring arg);
};

FloatsHandler::FloatsHandler(ArgParse::apstring usage_in,
                             ArgParse::apfloatvec* value_in,
                             int separator_in, int count_in) :
		OptionHandler(replaceDefault(usage_in, value_in),
		separator_in, count_in), value(value_in)
{}

ArgParse::apstring FloatsHandler::handlearg(ArgParse::apstring arg)
{
	ArgParse::apfloat newvalue;
	ArgParse::apstring ret = parseFloat(arg, &newvalue);
	if (ret.size() == 0)
	{
		value->push_back(newvalue);
		count -= (count == -1)? 0 : 1;
	}
	return ret;
}

void ArgParse::argFloats(apstring name, apstring usage,
                         apfloatvec* values, int separator, int count)
{
	d->addOption(name, new FloatsHandler(usage, values, separator, count));
}

/*-********************************************************************-*/

class StringsHandler : public OptionHandler
{
	public:
		ArgParse::apstringvec* value;

		StringsHandler(ArgParse::apstring usage_in,
		               ArgParse::apstringvec* value_in,
		               int separator_in, int count_in);
		ArgParse::apstring handlearg(ArgParse::apstring arg);
};

StringsHandler::StringsHandler(ArgParse::apstring usage_in,
                               ArgParse::apstringvec* value_in,
                               int separator_in, int count_in) :
		OptionHandler(replaceDefault(usage_in, value_in),
		separator_in, count_in), value(value_in)
{}

ArgParse::apstring StringsHandler::handlearg(ArgParse::apstring arg)
{
	value->push_back(arg);
	count -= (count == -1)? 0 : 1;
	return "";
}

void ArgParse::argStrings(apstring name, apstring usage,
                          apstringvec* values, int separator, int count)
{
	d->addOption(name, new StringsHandler(usage, values, separator, count));
}
