// Copyright (C) 2004 Andrew J. Bromage
//
// Contact: ajb@spamcop.net
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

// $Id: genkeywords.cpp 2846 2009-05-03 10:38:15Z c42f $

/** \file GenKeywords.cc
    \brief Pre-compile the keyword matching table.
    \author ajb@spamcop.net
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <vector>
#include <map>
#include <cstdlib>


struct Keyword
{
	const char* m_text;
	const char* m_tok;

	Keyword(const char* p_text, const char* p_tok)
			: m_text(p_text), m_tok(p_tok)
	{}

	bool operator<(const Keyword& p_rhs) const
	{
		return std::strcmp(m_text, p_rhs.m_text) < 0;
	}
};


Keyword
g_keywordTable[] = {
                       Keyword( "if",       "tIf" ),
                       Keyword( "else",     "tElse" ),
                       Keyword( "for",      "tFor" ),
                       Keyword( "while",    "tWhile" ),
                       Keyword( "do",       "tDo" ),
                       Keyword( "in",       "tIn" ),
                       Keyword( "break",    "tBreak" ),
                       Keyword( "continue", "tContinue" ),
                       Keyword( "default",  "tDefault" ),
                       Keyword( "switch",   "tSwitch" ),
                       Keyword( "case",     "tCase" ),
                       Keyword( "int",      "tInt" ),
                       Keyword( "float" ,   "tFloat" ),
                       Keyword( "string",   "tString" ),
                       Keyword( "vector",   "tVector" ),
                       Keyword( "matrix",   "tMatrix" ),
                       Keyword( "yes",      "tYes" ),
                       Keyword( "no",       "tNo" ),
                       Keyword( "on",       "tOn" ),
                       Keyword( "off",      "tOff" ),
                       Keyword( "true",     "tTrue" ),
                       Keyword( "false",    "tFalse" ),
                       Keyword( "global",   "tGlobal" ),
                       Keyword( "return",   "tReturn" ),
                       Keyword( "source",   "tSource" ),
                       Keyword( "catch",    "tCatch" ),
                       Keyword( "alias",    "tAlias" ),
                       Keyword( "proc",     "tProc" )
                   };


typedef std::set
	<Keyword>
	KeywordSet;


KeywordSet
initialKeywordSet()
{
	int n = sizeof(g_keywordTable) / sizeof(g_keywordTable[0]);
	return KeywordSet(g_keywordTable, g_keywordTable + n);
}


std::string
indent(int p_indent)
{
	std::string s;
	for (int i = 0; i < p_indent; ++i)
	{
		s += "    ";
	}
	return s;
}


void
genBranch(std::ostream& p_os, int p_indent, const KeywordSet& p_set)
{
	KeywordSet::const_iterator ii;

	if (p_set.size() == 1)
	{
		ii = p_set.begin();
		if (!ii->m_text[0])
		{
			p_os << indent(p_indent) << "if (!*c)\n";
		}
		else
		{
			p_os << indent(p_indent)
			<< "if (std::strcmp(c, \"" << ii->m_text << "\") == 0)\n";
		}
		p_os << indent(p_indent) << "{\n";
		p_os << indent(p_indent) << "    return " << ii->m_tok << ";\n";
		p_os << indent(p_indent) << "}\n";
		return;
	}

	std::map<char, KeywordSet> branch;
	std::vector<const char*> leaves;

	for (ii = p_set.begin(); ii != p_set.end(); ++ii)
	{
		if (ii->m_text[0])
		{
			branch[ii->m_text[0]] = KeywordSet();
		}
		else
		{
			leaves.push_back(ii->m_tok);
		}
	}
	for (ii = p_set.begin(); ii != p_set.end(); ++ii)
	{
		if (ii->m_text[0])
		{
			branch[ii->m_text[0]].insert(Keyword(&ii->m_text[1], ii->m_tok));
		}
	}

	p_os << indent(p_indent) << "switch (*c)\n";
	p_os << indent(p_indent) << "{\n";

	if (leaves.size() > 0)
	{
		if (leaves.size() > 1)
		{
			throw "Internal error: Duplicate keywords?";
		}
		p_os << indent(p_indent + 1) << "case '\\0':\n";
		p_os << indent(p_indent + 1) << "{\n";
		p_os << indent(p_indent + 2) << "return " << leaves[0] << ";\n";
		p_os << indent(p_indent + 1) << "}\n";
	}

	std::map<char, KeywordSet>::const_iterator jj;
	for (jj = branch.begin(); jj != branch.end(); ++jj)
	{
		p_os << indent(p_indent + 1) << "case '" << jj->first << "':\n";
		p_os << indent(p_indent + 1) << "{\n";
		p_os << indent(p_indent + 2) << "++c;\n";
		genBranch(p_os, p_indent + 2, jj->second);
		p_os << indent(p_indent + 2) << "break;\n";
		p_os << indent(p_indent + 1) << "}\n";
	}

	p_os << indent(p_indent) << "}\n";
}


void
genKeywordMatcher(std::ostream& p_os)
{
	p_os << "// DO NOT EDIT THIS FILE!\n";
	p_os << "// Automatically generated from genkeywords.cpp\n";
	p_os << "\n";
	p_os << "namespace Kin {\n";
	p_os << "\n";
	p_os << "Scanner::Token\n";
	p_os << "Scanner::matchKeyword()\n";
	p_os << "{\n";
	p_os << "    const char* c = m_tokenText.c_str();\n";
	genBranch(p_os, 1, initialKeywordSet());
	p_os << "    return tIdentifier;\n";
	p_os << "}\n";
	p_os << "\n";
	p_os << "} // namespace Kin\n";
	p_os << "\n";
	p_os << "\n";
}


int
main(int argc, char* argv[])
{
	if (argc < 2)
	{
		Aqsis::log() << "Usage: " << argv[0] << " [filename]\n";
		return 1;
	}


	std::ostringstream str;

	try
	{
		genKeywordMatcher(str);
	}
	catch (const char* errtext)
	{
		Aqsis::log() << errtext << "\n";
		return 1;
	}

	std::ofstream out(argv[1]);
	out << str.str();

	return 0;
}


