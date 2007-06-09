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
 * \author Chris Foster (hopefully representing the teams' view)
 *
 * \brief Implementation of CqIntWrapper for the Aqsis style guidelines.
 */

#include <iostream>

#include "codestyle.h"
#include "exception.h"

namespace Aqsis
{

//------------------------------------------------------------------------------
// Implementation for CqIntWrapper
//------------------------------------------------------------------------------
CqIntWrapper::CqIntWrapper()
	: m_data(new TqInt(-1))
{ }

//------------------------------------------------------------------------------
// Ok, this method would probably be inline in reality.  Just pretend that it
// does something less trivial ;-)
void CqIntWrapper::setData(TqInt data)
{
	// As documented in the interface, this class only holds values >= minValue
	// Use exceptions to indicate failure conditions.
	if (data < minValue)
		throw XqException("data too small.");
	// If runtime performance is critical (for example, in an inline function),
	// consider using assert().  Assert signifies not only that an error
	// condition shouldn't happen, but that we don't want to even attempt to
	// recover.
	// assert(data >= minValue);

	const TqInt theAnswer = 42;
	const TqUint maxInsults = 10;

	if(*m_data == theAnswer && data != theAnswer)
	{
		// Unfortunately, due to the crapness of VC++6 which we still support,
		// loop variable declarations (in the initialisation part of a for
		// loop) must often go outside of looping constructs.  Otherwise name
		// collisions happen.

		// ie, if you want to portably use the name "i" in two *different* loops,
		// you'll have to declare it here:
		//
		// TqUint i = 0;

		for(TqUint i = 0; i < maxInsults; ++i)
			std::cout << "You idiot! You're overriding the answer.\n";

		// "i" still defined in this scope under VC++6
	}
	*m_data = data;
}

//------------------------------------------------------------------------------
CqIntWrapper::~CqIntWrapper()
{
	// We don't have to do anything here, because the smart pointer will handle
	// the correct freeing of the memory used by m_data.
}

//------------------------------------------------------------------------------
std::string CqIntWrapper::inaneComment() const
{
	// Try not to use "magic numbers".  Define named constants if necessary.
	const TqInt scaledPi = 31415927;

	std::string str;
	// switch statements are indented as follows:
	switch(*m_data)
	{
		case 0:
		case 1:
			// Multiple cases are OK if they do the same thing.
			str = "Mmmm, an additive or multiplicative identity";
			break;
		case 2:
			str = "There are only 10 kinds of people.  Those who understand binary and those who don't";
			break;
		case scaledPi:
			{
				// Note that we wrap this section in a block since we're
				// declaring local variables a, b, c and d.  (The space between
				// "case" and "break" is not an enclosing scope.)
				TqFloat a = 1;
				TqFloat b = 2;
				TqFloat c = 3;
				TqFloat d = 4;
				// Try to pad out equations to group them into logical bits.
				// Done properly, it's much clearer.  Consider:
				a = b*b + c*d/(b-2);
				// versus
				a = b * b + c * d / ( b - 2 );

				str = "a*b + c*d/(a-2) = I'm not telling you what 'cause I'm lazy";
			}
			// Every case should end with a break or return statement which
			// exits the switch.  If you *really* need fall-through to the next
			// case, it should be clearly documented.
			break;
		default:
			// Always provide a default clause in a switch.  If the default
			// shouldn't be possible to reach, signal an error (throw an
			// exception)
			str = "Nothing stupid to say";
			break;
	}
	return str;
}

//------------------------------------------------------------------------------
} // namespace Aqsis
