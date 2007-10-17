// Aqsis
// Copyright (C) 1997 - 2003, Paul C. Gregory
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
		\brief Implements a IV GLUT based viewer for Aqsis.
			It is based on framebuffer_glut and
				sgigt from libtiff source/contribs.
				This file is the forward definition of sgigt.c
		\author Michel Joron joron@sympatico.ca
*/


#ifndef SGIGT_H
#define SGIGT_H 1

extern  char *	sgigt_get_filename();
extern	int	sgigt_prev_image( char* argv[], int ix, int b, int e, int wrap );
extern	int	sgigt_next_image( char* argv[], int ix, int b, int e, int wrap );
extern  int	sgigt_read_images( int nargc, char * nargv[] );
extern  void	sgigt_new_page();
extern	void	sgigt_new_file(int idx);
extern  int     sgigt_get_ix();
extern  TIFF*   sgigt_get_tif();

#define ALPHA 1
#define RED   2
#define GREEN 4
#define BLUE  8
#define ALL   15


#endif /* SGIGT_H */
