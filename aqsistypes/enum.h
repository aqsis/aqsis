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
 * \brief String conversion convenience class for enums
 *
 * \author Chris Foster [ chris42f (at) g mail (dot) com ]
 */

#ifndef ENUM_H_INCLUDED
#define ENUM_H_INCLUDED

#include "aqsis.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include <boost/static_assert.hpp>

#include "sstring.h" // for CqString::hash()

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief Class wrapping enums with string conversion capabilities
 *
 * A very common problem is to handle the conversion between enumeration values
 * and a string representation of those values.  This class and accompanying
 * macros attempts to make that conversion more convenient and (slightly) less
 * error prone by providing a uniform way to do the conversions, and allowing
 * the table of associated strings to be placed in a header file next to the
 * definition of the enum constants themselves.
 *
 * \note This class can ONLY be used for enums which have the default
 * values for enum constants - that is, they start from 0 and go to numItems-1.
 * This restriction could be relaxed with a small amount of effort.
 *
 * Example:
 *
 * First define the enum
 *
 * \code
 *
 *	 enum Day
 *	 {
 *	 	Sunday,
 *	 	Monday,
 *	 	Tuesday,
 *	 	Wednesday,
 *	 	Thursday,
 *	 	Friday,
 *	 	Saturday
 *	 };
 *
 * \endcode
 *
 * For every enum which we desire to use with CqEnum, we need to define the
 * string translation table - the macro AQSIS_ENUM_NAMES is provided for this
 * purpose.
 *
 * A default value for the enum should also be specified.  Due to the
 * way the macros are defined, this needs to come AFTER the names array is
 * defined, but within the same translation unit.
 *
 * \code
 *
 *	 // String translation table for the enum, along with the default value for
 *	 // when string conversion fails.
 *	 AQSIS_ENUM_INFO_BEGIN(Day, Friday)
 *	 	"Sunday",
 *	 	"Monday",
 *	 	"Tuesday",
 *	 	"Wednesday",
 *	 	"Thursday",
 *	 	"Friday",
 *	 	"Saturday"
 *	 AQSIS_ENUM_INFO_END
 *
 * \endcode
 *
 *
 * Following this, the usage is simple:
 *
 * \code
 *
 *	const char* someStr = "Monday";
 *	Day d = CqEnum<Day>(someStr).value();
 *	
 *	// stream-insertion is particularly easy
 *	
 *	std::cout << CqEnum<Day>(d) << "\n";
 *
 *  // alternatively, we can convert from the enum const to a string:
 *
 *  Day d2 = Monday;
 *
 *  // ...
 *
 *  std::string dayName = CqEnum<Day>(d).name();
 *
 * \endcode
 */
template<typename EnumT>
class CqEnum
{
	public:
		/// Initialize the CqEnum to the default value.
		CqEnum();
		/// Initialize the CqEnum to the given value
		CqEnum(EnumT value);
		/** \brief Convert a string representation of the enum into an enum value.
		 *
		 * If the conversion fails, use the default value as specified in
		 * the associated AQSIS_ENUM_INFO_BEGIN macro.
		 */
		CqEnum(const std::string& str);

		/// Return a string associated with the current enum value.
		const std::string& name() const;
		/// Return the current value of the enumeration constant.
		EnumT value() const;

	private:
		class CqEnumInfo;
		// Info about enum type conversions including the default and
		// associated strings constants.
		const static CqEnumInfo m_enumInfo;
		// Underlying value
		EnumT m_value;
};

//------------------------------------------------------------------------------
// Stream insertion for CqEnum<...>
template<typename EnumT>
std::ostream& operator<<(std::ostream& out, const CqEnum<EnumT>& e);
// Stream extraction for CqEnum<...>
template<typename EnumT>
std::istream& operator>>(std::istream& in, const CqEnum<EnumT>& e);

//------------------------------------------------------------------------------
/** The following macros ease the definition of the function returning the
 * string and default values for the enum.
 */

/** \brief Begin an enum string definition block 
 *
 * \param EnumT - name of the enum type for which strings are to be provided
 * \param defaultValue - default value for the enum, used when string
 * conversion fails.
 */
#define AQSIS_ENUM_INFO_BEGIN(EnumType, defValue)                              \
template<>                                                                     \
inline CqEnum<EnumType>::CqEnumInfo::CqEnumInfo()                              \
	: m_names(),                                                               \
	m_lookup(),                                                                \
	m_defaultValue(defValue)                                                   \
{                                                                              \
	const char* enumNames[] = {

/** \brief End an enum string definition block 
 */
#define AQSIS_ENUM_INFO_END                                                    \
	};                                                                         \
	m_names.assign(enumNames, enumNames + sizeof(enumNames)/sizeof(const char*));\
	initLookup(m_names, m_lookup);                                             \
}


//==============================================================================
// Implementation details
//==============================================================================

/** \brief Class holding information about enum <---> string conversions.
 *
 * This class encapsulates a lookup scheme for converting strings into enum
 * constants and vice versa.  The current implementation only deals with enum
 * constants with the default ordering, but extending it would be fairly easy
 * if necessary.
 */
template<typename EnumT>
class CqEnum<EnumT>::CqEnumInfo
{
	private:
		typedef TqUlong TqHash;
		typedef std::vector<std::string> TqNameVec;
		typedef std::vector<std::pair<TqHash, EnumT> > TqLookupVec;
		// Comparison functor for looking up a hash in the lookup vector.
		struct SqHashCmp
		{
			bool operator()(const std::pair<TqHash, EnumT>& p, TqHash h)
			{
				return p.first < h;
			}
		};

		/** Check that hashed values stored in the lookup vector are unique.
		 *
		 * It's pretty unlikely, but not inconceivable to have hash collisions
		 * - this function checks that there are none.
		 */
		static bool hashesAreUnique(const TqLookupVec& lookup)
		{
			for(int i = 1, end = lookup.size(); i < end; ++i)
				if(lookup[i-1].first == lookup[i].first)
					return false;
			return true;
		}
		/** \brief Initialize the lookup vector.
		 *
		 * \param names - string names for 
		 * \param lookup - string lookup vector; consists of a set of
		 *                 (hash, enumConst) pairs, sorted on the hash value
		 *                 for fast lookups.
		 */
		static void initLookup(const TqNameVec& names, TqLookupVec& lookup)
		{
			for(int i = 0, end = names.size(); i < end; ++i)
			{
				TqHash h = CqString::hash(names[i].c_str());
				lookup.push_back(std::make_pair(h, static_cast<EnumT>(i)));
			}
			std::sort(lookup.begin(), lookup.end());
			assert(hashesAreUnique(lookup));
		}

		// vector of name strings
		TqNameVec m_names;
		// sorted vector of (name_hash, enum_const) pairs for fast lookups
		TqLookupVec m_lookup;
		// default value for when string conversion fails and default constructor.
		EnumT m_defaultValue;

	public:
		/** \brief Define enum const --> enum name mapping.
		 *
		 * This constructor needs to be specialized for each mapping via the
		 * AQSIS_ENUM_INFO_BEGIN / AQSIS_ENUM_INFO_END macros.
		 */
		CqEnumInfo()
			: m_names(),
			m_defaultValue(static_cast<EnumT>(0))
		{
			// If a failed compile occurs on the following line, the string
			// translation table is missing - make sure there is a
			// corresponding AQSIS_ENUM_INFO_BEGIN for the enum type you want
			// to convert.
			BOOST_STATIC_ASSERT(static_cast<EnumT>(0));
		}
		/** Return the enum value for the given string.
		 *
		 * The algorithm used here is binary search on a sorted array of hashed
		 * names.
		 *
		 * Several alternative search algorithms were tested using an enum
		 * consisting of 14 distinct values with names of length roughly six
		 * characters long.  Generally speaking, algorithms which first hashed
		 * the string were considerably faster (by up to a factor of 5 or so
		 * depending on the details) than algorithms doing direct string
		 * comparison.  
		 *
		 * Out of the hashing algorithms, binary search clearly has superior
		 * scaling performance compared to a straightforward linear search, and
		 * had comparable speed for small numbers of strings.
		 */
		EnumT valueFromString(const std::string& str) const
		{
			TqHash h = CqString::hash(str.c_str());
			// Binary search on hashed strings.  Assumes hashes are unique.
			typename TqLookupVec::const_iterator i = std::lower_bound(
					m_lookup.begin(), m_lookup.end(), h, SqHashCmp());
			if(i != m_lookup.end() && i->first == h)
				return i->second;
			return m_defaultValue;
		}
		/// Return the string corresponding to the provided enum value.
		const std::string& stringFromValue(EnumT value) const
		{
			assert(value < static_cast<int>(m_names.size()) && value >= 0);
			return m_names[value];
		}
		/// Return the default value for the enum
		EnumT defaultValue() const
		{
			return m_defaultValue;
		}
};


//------------------------------------------------------------------------------
// CqEnum implementation
template<typename EnumT>
const typename CqEnum<EnumT>::CqEnumInfo CqEnum<EnumT>::m_enumInfo;

template<typename EnumT>
CqEnum<EnumT>::CqEnum()
	: m_value(m_enumInfo.defaultValue())
{ }

template<typename EnumT>
CqEnum<EnumT>::CqEnum(EnumT value)
	: m_value(value)
{ }

template<typename EnumT>
CqEnum<EnumT>::CqEnum(const std::string& str)
	: m_value(m_enumInfo.valueFromString(str))
{ }

template<typename EnumT>
const std::string& CqEnum<EnumT>::name() const
{
	return m_enumInfo.stringFromValue(m_value);
}

template<typename EnumT>
EnumT CqEnum<EnumT>::value() const
{
	return m_value;
}


//------------------------------------------------------------------------------
// Free function implementations
template<typename EnumT>
std::ostream& operator<<(std::ostream& out, const CqEnum<EnumT>& e)
{
	out << e.name();
	return out;
}

template<typename EnumT>
std::istream& operator>>(std::istream& in, CqEnum<EnumT>& e)
{
	std::string str;
	in >> str;
	e = CqEnum<EnumT>(str);
	return in;
}


} // namespace Aqsis

#endif // ENUM_H_INCLUDED
