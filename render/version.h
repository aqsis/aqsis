// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
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
		\brief Version information and functions
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef VERSION_H_INCLUDED
#define VERSION_H_INCLUDED 1

#include "sstring.h"

#define	STRNAME		"Aqsis"

#define	VERMAJOR	0
#define	VERMINOR	4
#define	BUILD		4004

#define	STRX(x)	#x
#define	STR(x)	STRX(x)

#define	VERMAJOR_STR	STR(VERMAJOR)
#define	VERMINOR_STR	STR(VERMINOR)
#define	BUILD_STR		STR(BUILD)

#define	VERMAJORDOTVERMINOR_STR	STR(VERMAJOR##.##VERMINOR)
#define	VERSION_STR	STR(VERMAJOR##.##VERMINOR##.##BUILD)

#define GET_VERSION_FROM_STRING(a,b,c) TqInt __s=0,__e; \
			__s+=strVersion.find_first_of("0123456789",__s);	__e=strVersion.find_first_of(".",__s); \
			a=CqString(strVersion.substr(__s,__e)).ToDecInt();	__s+=__e;				  \
			__s+=strVersion.find_first_of("0123456789",__s);	__e=strVersion.find_first_of(".",__s); \
			b=CqString(strVersion.substr(__s,__e)).ToDecInt();	__s+=__e;				  \
			__s+=strVersion.find_first_of("0123456789",__s);	__e=strVersion.find_first_of(".",__s); \
			c=CqString(strVersion.substr(__s,__e)).ToDecInt();

#define CHECK_NEWER_VERSION(a,b,c) (((a)>VERMAJOR) || \
								((a)==VERMAJOR && (b)>VERMINOR) || \
								((a)==VERMAJOR && (b)==VERMINOR && (c)>BUILD))

#endif	// !VERSION_H_INCLUDED
