// Aqsis
// Copyright © 2006, Paul C. Gregory
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
    \brief Interface to AIR' DBO

*/

#ifndef IMPLICIT_H_INCLUDED
#define IMPLICIT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#define	_qShareName	CORE
#ifdef	WIN32
#  ifdef	AQSIS_STATIC_LINK

#    define  _qShare
#    define	_qShareC
#    define	_qShareM

#    undef _qShareName

#  else // AQSIS_DYNAMIC_LINK

#    define _qExportM __declspec(dllexport)
#    define _qImportM __declspec(dllimport)

#    define _qExportC
#    define _qImportC

#    define _qExport __declspec(dllexport)
#    define _qImport __declspec(dllimport)

#    undef  _qShare
#    undef	_qShareC
#    undef	_qShareM

#    ifdef _qBUILDING
#      if _qBUILDING == _qShareName
#        define _qShare _qExport
#        define _qShareM _qExportM
#        define _qShareC _qExportC
#      else
#        define _qShare _qImport
#        define _qShareM _qImportM
#        define _qShareC _qImportC
#      endif
#    else
#      define	_qShare _qImport
#      define _qShareM _qImportM
#      define _qShareC _qImportC
#    endif // _qBUILDING

#    undef _qShareName

#  endif	// AQSIS_DYNAMIC_LINK

#else	// !WIN32

#  define  _qShare
#  define	_qShareC
#  define	_qShareM

#  undef _qShareName

#endif // WIN32

typedef struct {
	int BlobbyId;
	int OpId;
} State;

/* Aqsis uses the same interface as AIR but the first parameter won't be 
   meaningfull 
*/
_qShare void ImplicitBound(State *s, float *bd, int niarg, int *iarg,
                                          int nfarg, float *farg, int nsarg, 
					  char **sarg);

_qShare void ImplicitValue(State *s, float *result, float *p, 
					int niarg, int *iarg, int nfarg, 
					float *farg, int nsarg, char **sarg);

_qShare void ImplicitRange(State *s, float *rng, float *bd, int niarg, 
					int *iarg, int nfarg, float *farg, 
					int nsarg, char **sarg);
_qShare void ImplicitFree(State *s);

/* Not supported for now */
_qShare void ImplicitMotion(State *s, float *movec, float *pt, float *times,
                           int niarg, int *iarg, int nfarg, float *farg,
                           int nsarg, char **sarg);

typedef void *TqImplicitBound(State *, float *, int, int *, int , float *, int, char **);
typedef void *TqImplicitValue(State *, float *, float *, int, int *, int, float *, int, char **);
typedef void *TqImplicitRange(State *, float *, float *, int, int *, int, float *, int, char **);
typedef void *TqImplicitFree (State *);

#ifdef __cplusplus
}
#endif

#endif  // !IMPLICIT_H_INCLUDED
