// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
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

#ifndef IDSOSHADEOPS_H
#define IDSOSHADEOPS_H

#include	<stack>
#include	<vector>
#include	<list>

#include	"aqsis.h"

#include	"sstring.h"
#include	"plugins.h"

#include	"shadeop.h"

START_NAMESPACE( Aqsis )

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

struct IqDSORepository
{
	virtual void SetDSOPath(const char*) = 0;
	virtual ~IqDSORepository()
	{}
	;

private:
	virtual std::list<SqDSOExternalCall*>* getShadeOpMethods(CqString*) = 0;

	// Parse a single DSO SHADEOP_TABLE entry
	virtual SqDSOExternalCall* parseShadeOpTableEntry(void*, SqShadeOp*) = 0;
};

END_NAMESPACE( Aqsis )

#endif	// IDSOSHADEOPS_H
