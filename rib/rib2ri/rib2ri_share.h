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
 * \brief Define RIB2RI_SHARE macro for DLL symbol import/export on windows.
 *
 * \author Chris Foster  [chris42f (at) gmail (dot) com]
 */

#ifndef RIB2RI_SHARE_H_INCLUDED
#define RIB2RI_SHARE_H_INCLUDED

#include "aqsis_compiler.h"


#ifdef AQSIS_SYSTEM_WIN32
#	ifdef AQSIS_STATIC_LINK
#		define RIB2RI_SHARE
#	else
#		ifdef RIB2RI_EXPORTS
#			define RIB2RI_SHARE __declspec(dllexport)
#		else
#			define RIB2RI_SHARE __declspec(dllimport)
#		endif
#	endif
#else
#	define RIB2RI_SHARE
#endif


#endif // RIB2RI_SHARE_H_INCLUDED
