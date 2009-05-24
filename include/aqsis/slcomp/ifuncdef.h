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
		\brief Interface classes for function definitions
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef IFUNCDEF_H_INCLUDED
#define IFUNCDEF_H_INCLUDED 1

#include	<aqsis/aqsis.h>

namespace Aqsis {

struct IqParseNode;

///----------------------------------------------------------------------
/// EqFuncType
/// Type of function

enum EqFuncType
{
    FuncTypeStandard = 0,
    FuncTypeLocal,
};


///----------------------------------------------------------------------
/// SqFuncRef
/// Structure storing a function reference.

struct SqFuncRef
{
	EqFuncType	m_Type;
	TqUint	m_Index;
};



struct IqParseNode;
struct IqFuncDef
{
	virtual	TqInt	Type() const = 0;
	virtual	bool	fLocal() const = 0;
	virtual	const char*	strName() const = 0;
	virtual	const char*	strVMName() const = 0;
	virtual	const char*	strParams() const = 0;
	virtual	const IqParseNode* pArgs() const = 0;
	virtual	IqParseNode* pDef() = 0;
	virtual	const IqParseNode* pDef() const = 0;
	virtual	bool	fVarying() const = 0;
	virtual	TqInt	VariableLength() const = 0;
	virtual	TqInt	InternalUsage() const = 0;

	static	IqFuncDef*	GetFunctionPtr( const SqFuncRef& Ref );

	virtual ~IqFuncDef()
	{
	};
};

//-----------------------------------------------------------------------

} // namespace Aqsis

#endif	// !IFUNCDEF_H_INCLUDED
