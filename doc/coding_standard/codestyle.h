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

// ^^^ Files include a GPLv2 copyleft notice at the top.


//------------------------------------------------------------
// DOXYGEN
//
// Code should be documented with doxygen comments.  Documenting the contents
// of this file, we have:

/** \file
 *
 * \author Chris Foster (hopefully representing the teams' view)
 *
 * \brief An example file describing the aqsis coding style.
 *
 * Aqsis has a bunch of informal conventions; this file tries to gather
 * together the best parts of such conventions and give a relatively short and
 * easy-to-digest introduction to the way we'd like new code to look.
 *
 * We know a lot of our old code doesn't follow these rules, but we have to
 * start somewhere.
 */

// Doxygen comments should precede every function and type declaration.
//
// Note that I've been very verbose here for the sake of demonstration, but
// don't take this to mean "document for the sake of documenting".  Remember
// that documentation is only useful if it increases the clarity of the code.
//
// Aqsis includes many specialised algorithms which have been taken from
// various books and papers, (or just plain worked out).  If you use a
// particular source to understand an algorithm, reference it!

//------------------------------------------------------------
// INDENTING; astyle
//
// Rules for indenting, bracket placement etc. are encapsulated in a custom
// .astylerc file for the code formatting program "Artistic Style".  See also
// the comments in the astylerc file.
//
// The more important rules encapsulate:
//   * Indent using tab characters equivalent to 4 spaces
//   * Break block brackets to separate lines, i.e. ANSI C/C++ style
//   * Indent classes so that the keywords public, protected and private are indented
//   * Indent switches so that the case statements are indented
//   * Indent cases so that the code block is indented from the case statement
//
// While astyle tries to do a good job, it doesn't really match carefully
// hand-written code.  If you follow the guidelines above (and in particular
// the examples below), the code will be clearer than if it needed to go
// through astyle.

// Include guards are named with the convention "FILENAME_H_INCLUDED"
#ifndef CODESTYLE_H_INCLUDED
#define CODESTYLE_H_INCLUDED

// Includes go here.  Note the order.

// First include aqsis.h - it has all the compiler-dependent #defines etc
#include <aqsis/aqsis.h>

// Next include any C++ standard libraries
#include <string>
#include <sstream>

// Followed by external library headers
#include <boost/shared_ptr.hpp>

// Followed by the remaining aqsis headers
// #include "blah.h"


// All aqsis components (apart from C-library interfaces) live inside some
// namespace.  The core renderer lives in "namespace Aqsis".  Libraries have
// their own namespace, for instance, "namespace libri2rib".

namespace Aqsis
{

//------------------------------------------------------------------------------
/** \brief An example enum.
 *
 * Enum names start with Eq (read: "Enum aQsis"), followed by a CamelCase suffix.
 */
enum EqPlaneSide
{
	// Individual elements of an enum should be named with a common CamelCase
	// prefix, followed by a single underscore, and the enumeration name using
	// CamelCase:
	Side_Outside = -1,
	Side_Both = 0,
	Side_Inside = 1,
};

//------------------------------------------------------------------------------
// Typedef names start with Tq, followed by a name in CamelCase, as usual:

/// A smart pointer type to a dynamically allocated integer.
typedef boost::shared_ptr<int> TqIntPtr;

// (Smart pointers are the preferred way of handling the lifecycle of
// dynamically allocated objects)

//------------------------------------------------------------------------------
/** \brief An (admittedly pointless) example class which wraps about a
 * dynamically allocated integer.
 *
 * All classes are named starting with Cq (read: "Class aQsis"), followed by
 * CamelCase lettering.
 */
class CqIntWrapper
{
	public:
		// First, public typedefs, followed by constants
		static const TqInt minValue = -100;

		// Public methods come next.

		/** \brief A boring constructor
		 */
		CqIntWrapper();

		/** \brief The destructor.
		 *
		 * Note: virtual so we can extend from CqIntWrapper.  (Undesirable
		 * for lightweight classes.)
		 *
		 * Note also that virtual methods which are overridden from base
		 * classes should explicitly be declared virtual for clarity.
		 *
		 * "Implementations" which are blank can be put inline to avoid a lot
		 * of extra boilerplate code.
		 */
		virtual ~CqIntWrapper() {}

		/** \brief Get the integer data held in CqIntWrapper
		 *
		 * Notice that accessor "get" methods are named _without_ the "get" prefix.
		 *
		 * Also, notice that we should try to be const-correct: since this
		 * method doesn't modify the object, we document this fact by declaring
		 * it const.
		 *
		 * \return The integer held by CqIntWrapper
		 */
		inline TqInt data() const;

		/** \brief Set the class data to the given integer
		 *
		 * Modifier, or "set" methods are named _with_ a "set" prefix.  Notice
		 * the use of lowerCamelCase for the member functions.
		 *
		 * \param data - value to give to the held integer.  Valid values are
		 *               any integer greater than or equal to minValue
		 */
		void setData(const TqInt data);

		/** \brief Return a string containing inane comments related to the
		 * value of the held integer.
		 *
		 * (Really to explain further code standards - see the function
		 * body...)
		 */
		std::string inaneComment() const;

	protected:
		// Protected stuff goes here, after the public section.

	private:
		// Private member data and functions go last.

		// Member data is named with a prefix of m_ followed by lowerCamelCase,
		// such as m_myMember, and should never be public.

		TqIntPtr m_data; ///< Doxygen comment describing m_data.
};


//------------------------------------------------------------------------------
/** \brief A structure holding a 2D point.
 *
 * Structures are named (surprise!) starting with the prefix Sq, followed by a
 * CamelCase name.
 *
 * They are intended as composite types with no "object behaviour".  If you
 * find yourself wanting to add lots of methods, or make any data private, you
 * should use a class.
 *
 * Although structs are intended not to have "behaviour", simple member
 * functions can be useful - the type which would usually be utility functions
 * in C.  For example, initialisation via a constructor.
 */
struct SqPoint
{
	// Members of a struct should be named without the m_ prefix which is used
	// for classes, as they are part of the *public* interface.  (The fact that
	// they are members is immediately obvious from the method of access: p.x
	// for example.)

	/// x-coordinate
	TqFloat x;
	/// y-coordinate
	TqFloat y;
	/** \brief Trivial constructor for points.
	 *
	 * \param x - x-coordinate
	 * \param y - y-coordinate
	 */
	SqPoint(const TqFloat x = 0, const TqFloat y = 0);
};


//------------------------------------------------------------------------------
/** \brief Return a string from any object with a stream-insertion operator
 *
 * This is a simple example of a template function, partly to emphasise where
 * the implementation should go.
 */
template<typename T>
std::string toString(const T& obj);

// Never use "using" declarations (especially in header files).  Rather use
// fully-qualified names such as std::string.



//==============================================================================
// Implementation details
//==============================================================================
// The implementation section of a header should be clearly marked using a
// notice like that above.  This helps readers distinguish between the interface
// and implementation.

// Inline function implementations should be placed at the end of the header
// files.
//
// The same goes for template implementations - put them at the end of the
// header, rather than in with the template declarations.

//------------------------------------------------------------------------------
// CqIntWrapper implementation
inline TqInt CqIntWrapper::data() const
{
	return *m_data;
}


//------------------------------------------------------------------------------
// SqPoint implementation

// Use initialisation lists for the member data before the constructor-body
// proper where possible.  They are clearer, less error prone, and more
// efficient.
//
// Note that according to the C++ standard, an initialisation list is executed
// in the order which the members are declared in the class, *not* in the
// order of the initialisation list itself.  Therefore you should keep the
// order the same.  (In this case, x before y.) otherwise (a) it'll be
// confusing, and (b) compilers will spit warnings at you.
inline SqPoint::SqPoint(const TqFloat x, const TqFloat y)
	: x(x),
	y(y)
{ }


//------------------------------------------------------------------------------
// Free function implementations

template<typename T>
std::string toString(const T& obj)
{
	std::stringstream output;
	output << obj;
	return output.str();
}


//------------------------------------------------------------------------------
} // namespace Aqsis

#endif // MYCLASS_H_INCLUDED
