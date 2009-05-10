/*
 * Aqsis
 * Copyright (C) 1997 - 2001, Paul C. Gregory
 *
 * Contact: pgregory@aqsis.org
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/** \file
 * \brief Version information
 * \author Paul C. Gregory (pgregory@aqsis.org)
 *
 * C-compatible header, no C++ constructs please!
 */

#ifndef AQSIS_VERSION_H_INCLUDED
#define AQSIS_VERSION_H_INCLUDED


#define	AQSIS_VERSION_MAJOR ${VERSION_MAJOR}
#define	AQSIS_VERSION_MINOR ${VERSION_MINOR}
#define	AQSIS_VERSION_BUILD ${VERSION_BUILD}

#define	AQSIS_VERSION_STR   "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUILD}${BUILDTYPE}"
#define AQSIS_VERSION_STR_FULL "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUILD}${BUILDTYPE} (revision ${SCM_REVISION})"


#endif	/* AQSIS_VERSION_H_INCLUDED */
