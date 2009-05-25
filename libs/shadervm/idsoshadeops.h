// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.  // // This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


/** \file
		\brief  Interface and structures for supporting DSOs
		\author Tristan Colgate <tristan@inuxtech.co.uk>
*/

#ifndef IDSOSHADEOPS_H_INCLUDED
#define IDSOSHADEOPS_H_INCLUDED

#include <aqsis/aqsis.h>

#include <list>

#include <aqsis/shadervm/ishader.h>
#include <aqsis/shadervm/ishaderexecenv.h>
#include <aqsis/util/plugins.h>
#include <aqsis/riutil/primvartype.h>
#include <aqsis/ri/shadeop.h>
#include <aqsis/util/sstring.h>

namespace Aqsis {

// This is used as the datablock for SO_external within UsProgramElement
// Additionally it cotains the init and shutdown funnction pointers, it
// is basically a parsed SHADEOP_TABLE entry
struct SqDSOExternalCall
{
	DSOMethod method;
	DSOInit init;
	DSOShutdown shutdown;
	enum EqVariableType return_type;
	std::list<EqVariableType> arg_types;
	void *initData;
	bool initialised;
};

/** Interface to the repostiory of dynamic shader operations*/

struct AQSIS_SHADERVM_SHARE IqDSORepository
{
	virtual void SetDSOPath(const char*) = 0;
	virtual ~IqDSORepository() {}

private:
	virtual std::list<SqDSOExternalCall*>* getShadeOpMethods(CqString*) = 0;

	// Parse a single DSO SHADEOP_TABLE entry
	virtual SqDSOExternalCall* parseShadeOpTableEntry(void*, SqShadeOp*) = 0;
};

} // namespace Aqsis

#endif	// IDSOSHADEOPS_H_INCLUDED
