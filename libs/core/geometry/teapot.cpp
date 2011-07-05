// Aqsis
// Copyright c 1997 - 2001, Paul C. Gregory
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
		\brief Implements RiGeometry "teapot" option.
		\author T. Burge (tburge@affine.org)
*/
/*    References:
 *          [CROW87]  Crow, F. C. The Origins of the Teapot. 
 *                    IEEE Computer Graphics and Applications, 
 *                    pp. 8-19, Vol 7 No 1, 1987.
 *          [PIXA89]  Pixar, The RenderMan Interface, Version 3.1, 
 *                    Richmond, CA, September 1989.  
 *
 */

#include	<math.h>

#include	"teapot.h"
#include	"patch.h"

namespace Aqsis {

static CqVector3D Patch01[ 13*10 ] = { /*u=13 v=10 */
                                         CqVector3D( 1.5, 0.0, 0.0 ),
                                         CqVector3D( 1.5, 0.828427, 0.0 ),
                                         CqVector3D( 0.828427, 1.5, 0.0 ),
                                         CqVector3D( 0, 1.5, 0.0 ),
                                         CqVector3D( -0.828427, 1.5, 0.0 ),
                                         CqVector3D( -1.5, 0.828427, 0.0 ),
                                         CqVector3D( -1.5, 0, 0.0 ),
                                         CqVector3D( -1.5, -0.828427, 0.0 ),
                                         CqVector3D( -0.828427, -1.5, 0.0 ),
                                         CqVector3D( 0, -1.5, 0.0 ),
                                         CqVector3D( 0.828427, -1.5, 0.0 ),
                                         CqVector3D( 1.5, -0.828427, 0.0 ),
                                         CqVector3D( 1.5, 0.0, 0.0 ),
                                         CqVector3D( 1.5, 0.0, 0.075 ),
                                         CqVector3D( 1.5, 0.828427, 0.075 ),
                                         CqVector3D( 0.828427, 1.5, 0.075 ),
                                         CqVector3D( 0, 1.5, 0.075 ),
                                         CqVector3D( -0.828427, 1.5, 0.075 ),
                                         CqVector3D( -1.5, 0.828427, 0.075 ),
                                         CqVector3D( -1.5, 0, 0.075 ),
                                         CqVector3D( -1.5, -0.828427, 0.075 ),
                                         CqVector3D( -0.828427, -1.5, 0.075 ),
                                         CqVector3D( 0.0, -1.5, 0.075 ),
                                         CqVector3D( 0.828427, -1.5, 0.075 ),
                                         CqVector3D( 1.5, -0.828427, 0.075 ),
                                         CqVector3D( 1.5, 0, 0.075 ),
                                         CqVector3D( 2, 0, 0.3 ),
                                         CqVector3D( 2, 1.10457, 0.3 ),
                                         CqVector3D( 1.10457, 2, 0.3 ),
                                         CqVector3D( 0.0, 2, 0.3 ),
                                         CqVector3D( -1.10457, 2, 0.3 ),
                                         CqVector3D( -2, 1.10457, 0.3 ),
                                         CqVector3D( -2, 0, 0.3 ),
                                         CqVector3D( -2, -1.10457, 0.3 ),
                                         CqVector3D( -1.10457, -2, 0.3 ),
                                         CqVector3D( 0, -2, 0.3 ),
                                         CqVector3D( 1.10457, -2, 0.3 ),
                                         CqVector3D( 2, -1.10457, 0.3 ),
                                         CqVector3D( 2, 0, 0.3 ),
                                         CqVector3D( 2, 0, 0.75 ),
                                         CqVector3D( 2, 1.10457, 0.75 ),
                                         CqVector3D( 1.10457, 2, 0.75 ),
                                         CqVector3D( 0, 2, 0.75 ),
                                         CqVector3D( -1.10457, 2, 0.75 ),
                                         CqVector3D( -2, 1.10457, 0.75 ),
                                         CqVector3D( -2, 0, 0.75 ),
                                         CqVector3D( -2, -1.10457, 0.75 ),
                                         CqVector3D( -1.10457, -2, 0.75 ),
                                         CqVector3D( 0, -2, 0.75 ),
                                         CqVector3D( 1.10457, -2, 0.75 ),
                                         CqVector3D( 2, -1.10457, 0.75 ),
                                         CqVector3D( 2, 0, 0.75 ),
                                         CqVector3D( 2, 0, 1.2 ),
                                         CqVector3D( 2, 1.10457, 1.2 ),
                                         CqVector3D( 1.10457, 2, 1.2 ),
                                         CqVector3D( 0, 2, 1.2 ),
                                         CqVector3D( -1.10457, 2, 1.2 ),
                                         CqVector3D( -2, 1.10457, 1.2 ),
                                         CqVector3D( -2, 0, 1.2 ),
                                         CqVector3D( -2, -1.10457, 1.2 ),
                                         CqVector3D( -1.10457, -2, 1.2 ),
                                         CqVector3D( 0, -2, 1.2 ),
                                         CqVector3D( 1.10457, -2, 1.2 ),
                                         CqVector3D( 2, -1.10457, 1.2 ),
                                         CqVector3D( 2, 0, 1.2 ),
                                         CqVector3D( 1.75, 0, 1.725 ),
                                         CqVector3D( 1.75, 0.966498, 1.725 ),
                                         CqVector3D( 0.966498, 1.75, 1.725 ),
                                         CqVector3D( 0, 1.75, 1.725 ),
                                         CqVector3D( -0.966498, 1.75, 1.725 ),
                                         CqVector3D( -1.75, 0.966498, 1.725 ),
                                         CqVector3D( -1.75, 0, 1.725 ),
                                         CqVector3D( -1.75, -0.966498, 1.725 ),
                                         CqVector3D( -0.966498, -1.75, 1.725 ),
                                         CqVector3D( 0, -1.75, 1.725 ),
                                         CqVector3D( 0.966498, -1.75, 1.725 ),
                                         CqVector3D( 1.75, -0.966498, 1.725 ),
                                         CqVector3D( 1.75, 0, 1.725 ),
                                         CqVector3D( 1.5, 0, 2.25 ),
                                         CqVector3D( 1.5, 0.828427, 2.25 ),
                                         CqVector3D( 0.828427, 1.5, 2.25 ),
                                         CqVector3D( 0, 1.5, 2.25 ),
                                         CqVector3D( -0.828427, 1.5, 2.25 ),
                                         CqVector3D( -1.5, 0.828427, 2.25 ),
                                         CqVector3D( -1.5, 0, 2.25 ),
                                         CqVector3D( -1.5, -0.828427, 2.25 ),
                                         CqVector3D( -0.828427, -1.5, 2.25 ),
                                         CqVector3D( 0, -1.5, 2.25 ),
                                         CqVector3D( 0.828427, -1.5, 2.25 ),
                                         CqVector3D( 1.5, -0.828427, 2.25 ),
                                         CqVector3D( 1.5, 0, 2.25 ),
                                         CqVector3D( 1.4375, 0, 2.38125 ),
                                         CqVector3D( 1.4375, 0.793909, 2.38125 ),
                                         CqVector3D( 0.793909, 1.4375, 2.38125 ),
                                         CqVector3D( 0, 1.4375, 2.38125 ),
                                         CqVector3D( -0.793909, 1.4375, 2.38125 ),
                                         CqVector3D( -1.4375, 0.793909, 2.38125 ),
                                         CqVector3D( -1.4375, 0, 2.38125 ),
                                         CqVector3D( -1.4375, -0.793909, 2.38125 ),
                                         CqVector3D( -0.793909, -1.4375, 2.38125 ),
                                         CqVector3D( 0, -1.4375, 2.38125 ),
                                         CqVector3D( 0.793909, -1.4375, 2.38125 ),
                                         CqVector3D( 1.4375, -0.793909, 2.38125 ),
                                         CqVector3D( 1.4375, 0, 2.38125 ),
                                         CqVector3D( 1.3375, 0, 2.38125 ),
                                         CqVector3D( 1.3375, 0.738681, 2.38125 ),
                                         CqVector3D( 0.738681, 1.3375, 2.38125 ),
                                         CqVector3D( 0, 1.3375, 2.38125 ),
                                         CqVector3D( -0.738681, 1.3375, 2.38125 ),
                                         CqVector3D( -1.3375, 0.738681, 2.38125 ),
                                         CqVector3D( -1.3375, 0, 2.38125 ),
                                         CqVector3D( -1.3375, -0.738681, 2.38125 ),
                                         CqVector3D( -0.738681, -1.3375, 2.38125 ),
                                         CqVector3D( 0, -1.3375, 2.38125 ),
                                         CqVector3D( 0.738681, -1.3375, 2.38125 ),
                                         CqVector3D( 1.3375, -0.738681, 2.38125 ),
                                         CqVector3D( 1.3375, 0, 2.38125 ),
                                         CqVector3D( 1.4, 0, 2.25 ),
                                         CqVector3D( 1.4, 0.773198, 2.25 ),
                                         CqVector3D( 0.773198, 1.4, 2.25 ),
                                         CqVector3D( 0, 1.4, 2.25 ),
                                         CqVector3D( -0.773198, 1.4, 2.25 ),
                                         CqVector3D( -1.4, 0.773198, 2.25 ),
                                         CqVector3D( -1.4, 0, 2.25 ),
                                         CqVector3D( -1.4, -0.773198, 2.25 ),
                                         CqVector3D( -0.773198, -1.4, 2.25 ),
                                         CqVector3D( 0, -1.4, 2.25 ),
                                         CqVector3D( 0.773198, -1.4, 2.25 ),
                                         CqVector3D( 1.4, -0.773198, 2.25 ),
                                         CqVector3D( 1.4, 0, 2.25 ),
                                     };

static CqVector3D Patch02[ 13*7 ] = { /*u=13 v=7 */
                                        CqVector3D( 1.3, 0, 2.25 ),
                                        CqVector3D( 1.3, 0.71797, 2.25 ),
                                        CqVector3D( 0.71797, 1.3, 2.25 ),
                                        CqVector3D( 0, 1.3, 2.25 ),
                                        CqVector3D( -0.71797, 1.3, 2.25 ),
                                        CqVector3D( -1.3, 0.71797, 2.25 ),
                                        CqVector3D( -1.3, 0, 2.25 ),
                                        CqVector3D( -1.3, -0.71797, 2.25 ),
                                        CqVector3D( -0.71797, -1.3, 2.25 ),
                                        CqVector3D( 0, -1.3, 2.25 ),
                                        CqVector3D( 0.71797, -1.3, 2.25 ),
                                        CqVector3D( 1.3, -0.71797, 2.25 ),
                                        CqVector3D( 1.3, 0, 2.25 ),
                                        CqVector3D( 1.3, 0, 2.4 ),
                                        CqVector3D( 1.3, 0.71797, 2.4 ),
                                        CqVector3D( 0.71797, 1.3, 2.4 ),
                                        CqVector3D( 0, 1.3, 2.4 ),
                                        CqVector3D( -0.71797, 1.3, 2.4 ),
                                        CqVector3D( -1.3, 0.71797, 2.4 ),
                                        CqVector3D( -1.3, 0, 2.4 ),
                                        CqVector3D( -1.3, -0.71797, 2.4 ),
                                        CqVector3D( -0.71797, -1.3, 2.4 ),
                                        CqVector3D( 0, -1.3, 2.4 ),
                                        CqVector3D( 0.71797, -1.3, 2.4 ),
                                        CqVector3D( 1.3, -0.71797, 2.4 ),
                                        CqVector3D( 1.3, 0, 2.4 ),
                                        CqVector3D( 0.4, 0, 2.4 ),
                                        CqVector3D( 0.4, 0.220914, 2.4 ),
                                        CqVector3D( 0.220914, 0.4, 2.4 ),
                                        CqVector3D( 0, 0.4, 2.4 ),
                                        CqVector3D( -0.220914, 0.4, 2.4 ),
                                        CqVector3D( -0.4, 0.220914, 2.4 ),
                                        CqVector3D( -0.4, 0, 2.4 ),
                                        CqVector3D( -0.4, -0.220914, 2.4 ),
                                        CqVector3D( -0.220914, -0.4, 2.4 ),
                                        CqVector3D( 0, -0.4, 2.4 ),
                                        CqVector3D( 0.220914, -0.4, 2.4 ),
                                        CqVector3D( 0.4, -0.220914, 2.4 ),
                                        CqVector3D( 0.4, 0, 2.4 ),
                                        CqVector3D( 0.2, 0, 2.55 ),
                                        CqVector3D( 0.2, 0.110457, 2.55 ),
                                        CqVector3D( 0.110457, 0.2, 2.55 ),
                                        CqVector3D( 0, 0.2, 2.55 ),
                                        CqVector3D( -0.110457, 0.2, 2.55 ),
                                        CqVector3D( -0.2, 0.110457, 2.55 ),
                                        CqVector3D( -0.2, 0, 2.55 ),
                                        CqVector3D( -0.2, -0.110457, 2.55 ),
                                        CqVector3D( -0.110457, -0.2, 2.55 ),
                                        CqVector3D( 0, -0.2, 2.55 ),
                                        CqVector3D( 0.110457, -0.2, 2.55 ),
                                        CqVector3D( 0.2, -0.110457, 2.55 ),
                                        CqVector3D( 0.2, 0, 2.55 ),
                                        CqVector3D( 0, 0, 2.7 ),
                                        CqVector3D( 0, 0, 2.7 ),
                                        CqVector3D( 0, 0, 2.7 ),
                                        CqVector3D( 0, 0, 2.7 ),
                                        CqVector3D( 0, 0, 2.7 ),
                                        CqVector3D( 0, 0, 2.7 ),
                                        CqVector3D( 0, 0, 2.7 ),
                                        CqVector3D( 0, 0, 2.7 ),
                                        CqVector3D( 0, 0, 2.7 ),
                                        CqVector3D( 0, 0, 2.7 ),
                                        CqVector3D( 0, 0, 2.7 ),
                                        CqVector3D( 0, 0, 2.7 ),
                                        CqVector3D( 0, 0, 2.7 ),
                                        CqVector3D( 0.8, 0, 3 ),
                                        CqVector3D( 0.8, 0.441828, 3 ),
                                        CqVector3D( 0.441828, 0.8, 3 ),
                                        CqVector3D( 0, 0.8, 3 ),
                                        CqVector3D( -0.441828, 0.8, 3 ),
                                        CqVector3D( -0.8, 0.441828, 3 ),
                                        CqVector3D( -0.8, 0, 3 ),
                                        CqVector3D( -0.8, -0.441828, 3 ),
                                        CqVector3D( -0.441828, -0.8, 3 ),
                                        CqVector3D( 0, -0.8, 3 ),
                                        CqVector3D( 0.441828, -0.8, 3 ),
                                        CqVector3D( 0.8, -0.441828, 3 ),
                                        CqVector3D( 0.8, 0, 3 ),
                                        CqVector3D( 0, 0, 3 ),
                                        CqVector3D( 0, 0, 3 ),
                                        CqVector3D( 0, 0, 3 ),
                                        CqVector3D( 0, 0, 3 ),
                                        CqVector3D( 0, 0, 3 ),
                                        CqVector3D( 0, 0, 3 ),
                                        CqVector3D( 0, 0, 3 ),
                                        CqVector3D( 0, 0, 3 ),
                                        CqVector3D( 0, 0, 3 ),
                                        CqVector3D( 0, 0, 3 ),
                                        CqVector3D( 0, 0, 3 ),
                                        CqVector3D( 0, 0, 3 ),
                                        CqVector3D( 0, 0, 3 ),
                                    };

static CqVector3D Patch03[ 4*7 ] = { /*u=4 v=7 */
                                       CqVector3D( -2, 0, 0.75 ),
                                       CqVector3D( -2, 0.3, 0.75 ),
                                       CqVector3D( -1.9, 0.3, 0.45 ),
                                       CqVector3D( -1.9, 0, 0.45 ),
                                       CqVector3D( -2.5, 0, 0.975 ),
                                       CqVector3D( -2.5, 0.3, 0.975 ),
                                       CqVector3D( -2.65, 0.3, 0.7875 ),
                                       CqVector3D( -2.65, 0, 0.7875 ),
                                       CqVector3D( -2.7, 0, 1.425 ),
                                       CqVector3D( -2.7, 0.3, 1.425 ),
                                       CqVector3D( -3, 0.3, 1.2 ),
                                       CqVector3D( -3, 0, 1.2 ),
                                       CqVector3D( -2.7, 0, 1.65 ),
                                       CqVector3D( -2.7, 0.3, 1.65 ),
                                       CqVector3D( -3, 0.3, 1.65 ),
                                       CqVector3D( -3, 0, 1.65 ),
                                       CqVector3D( -2.7, 0, 1.875 ),
                                       CqVector3D( -2.7, 0.3, 1.875 ),
                                       CqVector3D( -3, 0.3, 2.1 ),
                                       CqVector3D( -3, 0, 2.1 ),
                                       CqVector3D( -2.3, 0, 1.875 ),
                                       CqVector3D( -2.3, 0.3, 1.875 ),
                                       CqVector3D( -2.5, 0.3, 2.1 ),
                                       CqVector3D( -2.5, 0, 2.1 ),
                                       CqVector3D( -1.6, 0, 1.875 ),
                                       CqVector3D( -1.6, 0.3, 1.875 ),
                                       CqVector3D( -1.5, 0.3, 2.1 ),
                                       CqVector3D( -1.5, 0, 2.1 ),
                                   };
static CqVector3D Patch04[ 4*7 ] = { /*u=4 v=7 */
                                       CqVector3D( 2.8, 0, 2.25 ),
                                       CqVector3D( 2.8, 0.15, 2.25 ),
                                       CqVector3D( 3.2, 0.15, 2.25 ),
                                       CqVector3D( 3.2, 0, 2.25 ),
                                       CqVector3D( 2.9, 0, 2.325 ),
                                       CqVector3D( 2.9, 0.25, 2.325 ),
                                       CqVector3D( 3.45, 0.15, 2.3625 ),
                                       CqVector3D( 3.45, 0, 2.3625 ),
                                       CqVector3D( 2.8, 0, 2.325 ),
                                       CqVector3D( 2.8, 0.25, 2.325 ),
                                       CqVector3D( 3.525, 0.25, 2.34375 ),
                                       CqVector3D( 3.525, 0, 2.34375 ),
                                       CqVector3D( 2.7, 0, 2.25 ),
                                       CqVector3D( 2.7, 0.25, 2.25 ),
                                       CqVector3D( 3.3, 0.25, 2.25 ),
                                       CqVector3D( 3.3, 0, 2.25 ),
                                       CqVector3D( 2.3, 0, 1.95 ),
                                       CqVector3D( 2.3, 0.25, 1.95 ),
                                       CqVector3D( 2.4, 0.25, 1.875 ),
                                       CqVector3D( 2.4, 0, 1.875 ),
                                       CqVector3D( 2.6, 0, 1.275 ),
                                       CqVector3D( 2.6, 0.66, 1.275 ),
                                       CqVector3D( 3.1, 0.66, 0.675 ),
                                       CqVector3D( 3.1, 0, 0.675 ),
                                       CqVector3D( 1.7, 0, 1.275 ),
                                       CqVector3D( 1.7, 0.66, 1.275 ),
                                       CqVector3D( 1.7, 0.66, 0.45 ),
                                       CqVector3D( 1.7, 0, 0.45 ),
                                   };
static CqVector3D Patch05[ 4*7 ] = { /*u=4 v=7 */
                                       CqVector3D( -1.9, 0, 0.45 ),
                                       CqVector3D( -1.9, -0.3, 0.45 ),
                                       CqVector3D( -2, -0.3, 0.75 ),
                                       CqVector3D( -2, 0, 0.75 ),
                                       CqVector3D( -2.65, 0, 0.7875 ),
                                       CqVector3D( -2.65, -0.3, 0.7875 ),
                                       CqVector3D( -2.5, -0.3, 0.975 ),
                                       CqVector3D( -2.5, 0, 0.975 ),
                                       CqVector3D( -3, 0, 1.2 ),
                                       CqVector3D( -3, -0.3, 1.2 ),
                                       CqVector3D( -2.7, -0.3, 1.425 ),
                                       CqVector3D( -2.7, 0, 1.425 ),
                                       CqVector3D( -3, 0, 1.65 ),
                                       CqVector3D( -3, -0.3, 1.65 ),
                                       CqVector3D( -2.7, -0.3, 1.65 ),
                                       CqVector3D( -2.7, 0, 1.65 ),
                                       CqVector3D( -3, 0, 2.1 ),
                                       CqVector3D( -3, -0.3, 2.1 ),
                                       CqVector3D( -2.7, -0.3, 1.875 ),
                                       CqVector3D( -2.7, 0, 1.875 ),
                                       CqVector3D( -2.5, 0, 2.1 ),
                                       CqVector3D( -2.5, -0.3, 2.1 ),
                                       CqVector3D( -2.3, -0.3, 1.875 ),
                                       CqVector3D( -2.3, 0, 1.875 ),
                                       CqVector3D( -1.5, 0, 2.1 ),
                                       CqVector3D( -1.5, -0.3, 2.1 ),
                                       CqVector3D( -1.6, -0.3, 1.875 ),
                                       CqVector3D( -1.6, 0, 1.875 ),
                                   };

static CqVector3D Patch06[ 4*7 ] = { /*u=4 v=7 */
                                       CqVector3D( 3.2, 0, 2.25 ),
                                       CqVector3D( 3.2, -0.15, 2.25 ),
                                       CqVector3D( 2.8, -0.15, 2.25 ),
                                       CqVector3D( 2.8, 0, 2.25 ),
                                       CqVector3D( 3.45, 0, 2.3625 ),
                                       CqVector3D( 3.45, -0.15, 2.3625 ),
                                       CqVector3D( 2.9, -0.25, 2.325 ),
                                       CqVector3D( 2.9, 0, 2.325 ),
                                       CqVector3D( 3.525, 0, 2.34375 ),
                                       CqVector3D( 3.525, -0.25, 2.34375 ),
                                       CqVector3D( 2.8, -0.25, 2.325 ),
                                       CqVector3D( 2.8, 0, 2.325 ),
                                       CqVector3D( 3.3, 0, 2.25 ),
                                       CqVector3D( 3.3, -0.25, 2.25 ),
                                       CqVector3D( 2.7, -0.25, 2.25 ),
                                       CqVector3D( 2.7, 0, 2.25 ),
                                       CqVector3D( 2.4, 0, 1.875 ),
                                       CqVector3D( 2.4, -0.25, 1.875 ),
                                       CqVector3D( 2.3, -0.25, 1.95 ),
                                       CqVector3D( 2.3, 0, 1.95 ),
                                       CqVector3D( 3.1, 0, 0.675 ),
                                       CqVector3D( 3.1, -0.66, 0.675 ),
                                       CqVector3D( 2.6, -0.66, 1.275 ),
                                       CqVector3D( 2.6, 0, 1.275 ),
                                       CqVector3D( 1.7, 0, 0.45 ),
                                       CqVector3D( 1.7, -0.66, 0.45 ),
                                       CqVector3D( 1.7, -0.66, 1.275 ),
                                       CqVector3D( 1.7, 0, 1.275 ),
                                   };

static CqVector3D Patch07[ 13*2 ] = { /*u=13 v=2 */
                                        CqVector3D( 1.5, 0.0, 0.0 ),
                                        CqVector3D( 1.5, 0.828427, 0.0 ),
                                        CqVector3D( 0.828427, 1.5, 0.0 ),
                                        CqVector3D( 0, 1.5, 0.0 ),
                                        CqVector3D( -0.828427, 1.5, 0.0 ),
                                        CqVector3D( -1.5, 0.828427, 0.0 ),
                                        CqVector3D( -1.5, 0, 0.0 ),
                                        CqVector3D( -1.5, -0.828427, 0.0 ),
                                        CqVector3D( -0.828427, -1.5, 0.0 ),
                                        CqVector3D( 0, -1.5, 0.0 ),
                                        CqVector3D( 0.828427, -1.5, 0.0 ),
                                        CqVector3D( 1.5, -0.828427, 0.0 ),
                                        CqVector3D( 1.5, 0.0, 0.0 ),
                                        CqVector3D( 1.5, 0.0, 0.075 ),
                                        CqVector3D( 1.5, 0.828427, 0.075 ),
                                        CqVector3D( 0.828427, 1.5, 0.075 ),
                                        CqVector3D( 0, 1.5, 0.075 ),
                                        CqVector3D( -0.828427, 1.5, 0.075 ),
                                        CqVector3D( -1.5, 0.828427, 0.075 ),
                                        CqVector3D( -1.5, 0, 0.075 ),
                                        CqVector3D( -1.5, -0.828427, 0.075 ),
                                        CqVector3D( -0.828427, -1.5, 0.075 ),
                                        CqVector3D( 0.0, -1.5, 0.075 ),
                                        CqVector3D( 0.828427, -1.5, 0.075 ),
                                        CqVector3D( 1.5, -0.828427, 0.075 ),
                                        CqVector3D( 1.5, 0, 0.075 )
                                    };

//---------------------------------------------------------------------
/** Constructor.
 */

CqTeapot::CqTeapot( bool addCrowBase ) : m_CrowBase( addCrowBase )
{
	int i;
	TqInt lUses;
	boost::shared_ptr<CqSurfacePatchMeshBicubic> pSurface( new CqSurfacePatchMeshBicubic( 13, 10, RI_TRUE, RI_TRUE ) );
	lUses = pSurface->Uses();


	// Fill in default values for all primitive variables not explicitly specified.
	// Fill in primitive variables specified.
	pSurface->AddPrimitiveVariable( new CqParameterTypedVertex<CqVector4D, type_hpoint, CqVector3D>( "P", 1 ) );
	pSurface->P() ->SetSize( 13 * 10 );
	for ( i = 0; i < 13*10; i++ )
		pSurface->P()->pValue( i )[0] = vectorCast<CqVector4D>(Patch01[ i ]);
	pSurface->SetDefaultPrimitiveVariables();
	pSurface->SetSurfaceParameters( *this );

	pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<CqColor, type_color, CqColor>( "Cs" ) );
	pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<CqColor, type_color, CqColor>( "Os" ) );
	pSurface->Cs() ->SetSize( 130 );
	pSurface->Os() ->SetSize( 130 );
	for ( i = 0; i < 13*10; i++ )
	{
		pSurface->Cs()->pValue( i )[0] = m_pAttributes->GetColorAttribute( "System", "Color" ) [ 0 ];
		pSurface->Os()->pValue( i )[0] = m_pAttributes->GetColorAttribute( "System", "Opacity" ) [ 0 ];
	}
	pSurface->ConvertToBezierBasis();
	this->pPatchMeshBicubic[ 0 ] = pSurface;

	pSurface = boost::shared_ptr<CqSurfacePatchMeshBicubic>( new CqSurfacePatchMeshBicubic( 13, 7, RI_TRUE, RI_TRUE ) );
	lUses = pSurface->Uses();

	// Fill in default values for all primitive variables not explicitly specified.
	// Fill in primitive variables specified.
	pSurface->AddPrimitiveVariable( new CqParameterTypedVertex<CqVector4D, type_hpoint, CqVector3D>( "P", 1 ) );
	pSurface->P() ->SetSize( 13 * 7 );
	for ( i = 0; i < 13*7; i++ )
		pSurface->P()->pValue( i )[0] = vectorCast<CqVector4D>(Patch02[ i ]);
	pSurface->SetDefaultPrimitiveVariables();
	pSurface->SetSurfaceParameters( *this );

	pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<CqColor, type_color, CqColor>( "Cs" ) );
	pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<CqColor, type_color, CqColor>( "Os" ) );
	pSurface->Cs() ->SetSize( 13 * 7 );
	pSurface->Os() ->SetSize( 13 * 7 );
	for ( i = 0; i < 13*7; i++ )
	{
		pSurface->Cs()->pValue( i )[0] = m_pAttributes->GetColorAttribute( "System", "Color" ) [ 0 ];
		pSurface->Os()->pValue( i )[0] = m_pAttributes->GetColorAttribute( "System", "Opacity" ) [ 0 ];
	}
	pSurface->ConvertToBezierBasis();
	this->pPatchMeshBicubic[ 1 ] = pSurface;

	pSurface = boost::shared_ptr<CqSurfacePatchMeshBicubic>( new CqSurfacePatchMeshBicubic( 4, 7, RI_TRUE, RI_TRUE ) );
	lUses = pSurface->Uses();

	// Fill in default values for all primitive variables not explicitly specified.
	// Fill in primitive variables specified.
	pSurface->AddPrimitiveVariable( new CqParameterTypedVertex<CqVector4D, type_hpoint, CqVector3D>( "P", 1 ) );
	pSurface->P() ->SetSize( 4 * 7 );
	for ( i = 0; i < 4*7; i++ )
		pSurface->P()->pValue( i )[0] = vectorCast<CqVector4D>(Patch03[ i ]);
	pSurface->SetDefaultPrimitiveVariables();
	pSurface->SetSurfaceParameters( *this );

	pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<CqColor, type_color, CqColor>( "Cs" ) );
	pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<CqColor, type_color, CqColor>( "Os" ) );
	pSurface->Cs() ->SetSize( 4 * 7 );
	pSurface->Os() ->SetSize( 4 * 7 );
	for ( i = 0; i < 4*7; i++ )
	{
		pSurface->Cs()->pValue( i )[0] = m_pAttributes->GetColorAttribute( "System", "Color" ) [ 0 ];
		pSurface->Os()->pValue( i )[0] = m_pAttributes->GetColorAttribute( "System", "Opacity" ) [ 0 ];
	}
	pSurface->ConvertToBezierBasis();
	this->pPatchMeshBicubic[ 2 ] = pSurface;

	pSurface = boost::shared_ptr<CqSurfacePatchMeshBicubic>( new CqSurfacePatchMeshBicubic( 4, 7, RI_TRUE, RI_TRUE ) );
	lUses = pSurface->Uses();

	// Fill in default values for all primitive variables not explicitly specified.
	// Fill in primitive variables specified.
	pSurface->AddPrimitiveVariable( new CqParameterTypedVertex<CqVector4D, type_hpoint, CqVector3D>( "P", 1 ) );
	pSurface->P() ->SetSize( 4 * 7 );
	for ( i = 0; i < 4*7; i++ )
		pSurface->P()->pValue( i )[0] = vectorCast<CqVector4D>(Patch04[ i ]);
	pSurface->SetDefaultPrimitiveVariables();
	pSurface->SetSurfaceParameters( *this );

	pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<CqColor, type_color, CqColor>( "Cs" ) );
	pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<CqColor, type_color, CqColor>( "Os" ) );
	pSurface->Cs() ->SetSize( 4 * 7 );
	pSurface->Os() ->SetSize( 4 * 7 );
	for ( i = 0; i < 4*7; i++ )
	{
		pSurface->Cs()->pValue( i )[0] = m_pAttributes->GetColorAttribute( "System", "Color" ) [ 0 ];
		pSurface->Os()->pValue( i )[0] = m_pAttributes->GetColorAttribute( "System", "Opacity" ) [ 0 ];
	}
	pSurface->ConvertToBezierBasis();
	this->pPatchMeshBicubic[ 3 ] = pSurface;

	pSurface = boost::shared_ptr<CqSurfacePatchMeshBicubic>( new CqSurfacePatchMeshBicubic( 4, 7, RI_TRUE, RI_TRUE ) );
	lUses = pSurface->Uses();

	// Fill in default values for all primitive variables not explicitly specified.
	// Fill in primitive variables specified.
	pSurface->AddPrimitiveVariable( new CqParameterTypedVertex<CqVector4D, type_hpoint, CqVector3D>( "P", 1 ) );
	pSurface->P() ->SetSize( 4 * 7 );
	for ( i = 0; i < 4*7; i++ )
		pSurface->P()->pValue( i )[0] = vectorCast<CqVector4D>(Patch05[ i ]);
	pSurface->SetDefaultPrimitiveVariables();
	pSurface->SetSurfaceParameters( *this );

	pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<CqColor, type_color, CqColor>( "Cs" ) );
	pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<CqColor, type_color, CqColor>( "Os" ) );
	pSurface->Cs() ->SetSize( 4 * 7 );
	pSurface->Os() ->SetSize( 4 * 7 );
	for ( i = 0; i < 4*7; i++ )
	{
		pSurface->Cs()->pValue( i )[0] = m_pAttributes->GetColorAttribute( "System", "Color" ) [ 0 ];
		pSurface->Os()->pValue( i )[0] = m_pAttributes->GetColorAttribute( "System", "Opacity" ) [ 0 ];
	}
	pSurface->ConvertToBezierBasis();
	this->pPatchMeshBicubic[ 4 ] = pSurface;

	pSurface = boost::shared_ptr<CqSurfacePatchMeshBicubic>( new CqSurfacePatchMeshBicubic( 4, 7, RI_TRUE, RI_TRUE ) );
	lUses = pSurface->Uses();

	// Fill in default values for all primitive variables not explicitly specified.
	// Fill in primitive variables specified.
	pSurface->AddPrimitiveVariable( new CqParameterTypedVertex<CqVector4D, type_hpoint, CqVector3D>( "P", 1 ) );
	pSurface->P() ->SetSize( 4 * 7 );
	for ( i = 0; i < 4*7; i++ )
		pSurface->P()->pValue( i )[0] = vectorCast<CqVector4D>(Patch06[ i ]);
	pSurface->SetDefaultPrimitiveVariables();
	pSurface->SetSurfaceParameters( *this );

	pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<CqColor, type_color, CqColor>( "Cs" ) );
	pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<CqColor, type_color, CqColor>( "Os" ) );
	pSurface->Cs() ->SetSize( 4 * 7 );
	pSurface->Os() ->SetSize( 4 * 7 );
	for ( i = 0; i < 4*7; i++ )
	{
		pSurface->Cs()->pValue( i )[0] = m_pAttributes->GetColorAttribute( "System", "Color" ) [ 0 ];
		pSurface->Os()->pValue( i )[0] = m_pAttributes->GetColorAttribute( "System", "Opacity" ) [ 0 ];
	}
	pSurface->ConvertToBezierBasis();
	this->pPatchMeshBicubic[ 5 ] = pSurface;
	this->cNbrPatchMeshBicubic = 6;
	// bottom
	if ( m_CrowBase )
	{
		pSurface = boost::shared_ptr<CqSurfacePatchMeshBicubic>( new CqSurfacePatchMeshBicubic( 13, 4, RI_TRUE, RI_TRUE ) );
		lUses = pSurface->Uses();

		// Fill in default values for all primitive variables not explicitly specified.
		// Fill in primitive variables specified.
		pSurface->AddPrimitiveVariable( new CqParameterTypedVertex<CqVector4D, type_hpoint, CqVector3D>( "P", 1 ) );
		pSurface->P() ->SetSize( 13 * 4 );
		for ( i = 0; i < 13*4; i++ )
		{

			if ( i < 13 )
				pSurface->P()->pValue( i )[0] = CqVector4D(0.0f);
			else if ( i < 39 )
				pSurface->P()->pValue( i )[0] = vectorCast<CqVector4D>(Patch01[ i - 13 ]);
			else
				pSurface->P()->pValue( i )[0] = vectorCast<CqVector4D>(Patch01[ i - 13 ] * 0.85);


		}
		pSurface->SetDefaultPrimitiveVariables();
		pSurface->SetSurfaceParameters( *this );

		pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<CqColor, type_color, CqColor>( "Cs" ) );
		pSurface->AddPrimitiveVariable( new CqParameterTypedVarying<CqColor, type_color, CqColor>( "Os" ) );
		pSurface->Cs() ->SetSize( 13 * 4 );
		pSurface->Os() ->SetSize( 13 * 4 );
		for ( i = 0; i < 13*4; i++ )
		{
			pSurface->Cs()->pValue( i )[0] = m_pAttributes->GetColorAttribute( "System", "Color" ) [ 0 ];
			pSurface->Os()->pValue( i )[0] = m_pAttributes->GetColorAttribute( "System", "Opacity" ) [ 0 ];
		}
		pSurface->ConvertToBezierBasis();
		this->pPatchMeshBicubic[ 6 ] = pSurface;
		this->cNbrPatchMeshBicubic = 7;
	}


}


//---------------------------------------------------------------------
/** Create a clone of this teapot surface.
 */

CqSurface*	CqTeapot::Clone() const
{
	CqTeapot* clone = new CqTeapot();
	CqSurface::CloneData( clone );
	clone->m_CrowBase = m_CrowBase;
	clone->m_matTx = m_matTx;
	clone->m_matITTx = m_matITTx;

	return ( clone );
}


//---------------------------------------------------------------------
/** Transform the quadric primitive by the specified matrix.
 */

void	CqTeapot::Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx, TqInt iTime )
{
	m_matTx *= matTx;
	m_matITTx *= matITTx;
}


//---------------------------------------------------------------------
/** Get the geometric bound of this GPrim.
 */

void	CqTeapot::Bound(CqBound* bound) const
{
	CqVector3D vecMin( -3.000, -2.0, 0.0 );
	CqVector3D vecMax( 3.525, 2.0, ( m_CrowBase ? 3.15 : 3.15 - 0.15 /* remove bottom */ ) );

	bound->vecMin() = vecMin;
	bound->vecMax() = vecMax;
	bound->Transform( m_matTx );
	AdjustBoundForTransformationMotion( bound );
}

//---------------------------------------------------------------------
/** Split this GPrim into bicubic patches.
 */

TqInt CqTeapot::Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits )
{


	return 0;
}


} // namespace Aqsis
//---------------------------------------------------------------------


