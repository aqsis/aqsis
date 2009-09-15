// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
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
		\brief Declares the abstract interface for accessing parameters.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#ifndef IPARAMETER_H_INCLUDED //{
#define IPARAMETER_H_INCLUDED

#include <aqsis/aqsis.h>

#include <aqsis/core/interfacefwd.h>
#include <aqsis/riutil/primvartype.h>
#include <aqsis/util/sstring.h>

namespace Aqsis {

struct IqShaderData;

//----------------------------------------------------------------------
/** \struct IqParameter
 * Interface to a parameter class.
 */

struct IqParameter
{
	/** Pure virtual, get value class.
	 * \return Class as an EqVariableClass.
	 */
	virtual	EqVariableClass	Class() const = 0;
	/** Pure virtual, get value type.
	 * \return Type as an EqVariableType.
	 */
	virtual	EqVariableType	Type() const = 0;
	/** Pure virtual, set value size, not array, but varying/vertex size.
	 */
	virtual	void	SetSize( TqInt size ) = 0;
	/** Pure virtual, get value size, not array, but varying/vertex size.
	 */
	virtual	TqUint	Size() const = 0;
	/** Pure virtual, get the value array length.
	 */
	virtual TqInt ArrayLength() const = 0;
	/** Pure virtual, clear value contents.
	 */
	virtual	void	Clear() = 0;

	/** Pure virtual, dice the value into a grid using appropriate interpolation for the class.
	 * \param u Integer dice count for the u direction.
	 * \param v Integer dice count for the v direction.
	 * \param pResult Pointer to storage for the result.
	 * \param pSurface Pointer to the surface we are processing used for vertex class variables to perform natural interpolation.
	 */
	virtual	void	Dice( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface = 0 ) = 0;
	virtual	void	CopyToShaderVariable( IqShaderData* pResult ) const = 0;

	/** Pure virtual, dice a single array element of the value into a grid using appropriate interpolation for the class.
	 * \param u Integer dice count for the u direction.
	 * \param v Integer dice count for the v direction.
	 * \param pResult Pointer to storage for the result.
	 * \param pSurface Pointer to the surface we are processing used for vertex class variables to perform natural interpolation.
	 */
	virtual	void	DiceOne( TqInt u, TqInt v, IqShaderData* pResult, IqSurface* pSurface = 0, TqInt ArrayIndex = 0 ) = 0;

	/** Get a reference to the parameter name.
	 */
	virtual const	CqString& strName() const = 0;

	virtual ~IqParameter() {};
};

} // namespace Aqsis

//-----------------------------------------------------------------------
#endif //} IPARAMETER_H_INCLUDED
