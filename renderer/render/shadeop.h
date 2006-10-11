// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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
                  \brief Utility macros for creating Dynamic Shader Operations for Aqsis
		                  \author Tristan Colgate <tristan@inuxtech.co.uk>
				  */

/** Renderman Interface is Copyright (c) 1988 Pixar. All Rights reserved.*/

#ifndef SHADEOP_H
#define SHADEOP_H


#ifdef WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

struct SqShadeOp
{
	char *m_opspec;
	char *m_init;
	char *m_shutdown;
} ;

typedef struct _STRING_DESC
{
	char *s;
	int bufflen;
}
STRING_DESC;

// Some of the DSO's out there seem to use this
#define SHADEOP_SPEC struct SqShadeOp

// Utility macro for declaring a table of shader operations
#define SHADEOP_TABLE(opname) struct SqShadeOp EXPORT  opname ## _shadeops []

// Utility macro for declaring a shadeop method
#define SHADEOP(method) EXTERN_C EXPORT int method (void *initdata, int argc, void **argv)

// Utility macro for declaring a shadeop initilaisation function
#define SHADEOP_INIT(initfunc) EXTERN_C EXPORT void* initfunc (int ctx, void *texturectx)

// Utility macro for declaring a shadeop shutdown function
#define SHADEOP_SHUTDOWN(shutdownfunc) EXTERN_C EXPORT void shutdownfunc (void *initdata)
// alternative name for the above seen in bbox.c in the RMR
#define SHADEOP_CLEANUP(shutdownfunc) EXTERN_C EXPORT void shutdownfunc (void *initdata)

// We declare these here for access from shaderexecenv
typedef void (*DSOMethod)(void*,int,void**);
typedef void* (*DSOInit)(int,void*);
typedef void (*DSOShutdown)(void*);

#endif // SHADEOP_H
