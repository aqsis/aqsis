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
 *
 * \brief List of predeclared standard RIB primitive variables
 * \author Chris Foster [chris42f (at) g mail (d0t) com]
 */

#ifndef STANDARDVARS_H_INCLUDED
#define STANDARDVARS_H_INCLUDED

#include "aqsis.h"

#include <vector>

#include "primvartoken.h"

namespace Aqsis {

/** \brief Get the standard RIB variables predefined by aqsis
 *
 * The returned vector includes predefined token declarations for tokens
 * involving:
 *   * Standard shader instance variables (eg, "Ka")
 *   * Standar primvars (eg, "P")
 *   * Arguments for standard attributes and options (eg, "gridsize")
 *   * Some aqsis-specific attributes and options.
 *
 * \return A vector of CqPrimvarTokens representing standard predefined tokens.
 */
const std::vector<CqPrimvarToken>& standardRibVariables();


} // namespace Aqsis

#endif // STANDARDVARS_H_INCLUDED
