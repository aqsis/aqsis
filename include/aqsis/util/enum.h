// Aqsis
// Copyright (C) 2001, Paul C. Gregory and the other authors and contributors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the software's owners nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// (This is the New BSD license)

/** \file
 *
 * \brief String conversion capabilities for C++ enums
 *
 * \author Chris Foster [ chris42f (at) g mail (dot) com ]
 *
 * A very common problem is to handle the conversion between enumeration values
 * and a string representation of those values.  This file implements functions
 * and macros to make that conversion more convenient and less error prone.
 * It does this by providing a uniform way to do the conversions, and allowing
 * the table of associated strings to be placed in a header file next to the
 * definition of the enum constants themselves.
 *
 * These facilities can ONLY be used for enums which have the default values
 * for enum constants - that is, they start from 0 and go to numItems-1.  This
 * restriction could be relaxed with a small amount of effort.
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
 * For every enum which we desire to have string convertion support, we need to
 * define the string translation table - the macros AQSIS_ENUM_INFO_BEGIN and
 * AQSIS_ENUM_INFO_END are provided for this purpose:
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
 * Following this, it's simple to convert from a string to the enum using
 * enumCast, and vice versa using enumString.  The stream insertion and
 * extraction operators are also defined by the macros.
 *
 * \code
 *
 *  // STRING --> ENUM
 *
 *	const char* someStr = "Monday";
 *  // ...
 *	Day d = enumCast<Day>(someStr);
 *
 *  // stream extraction works too...
 *  std::cin >> d;
 *
 * 
 *  // ENUM --> STRING
 *	
 *	Day d = Friday;
 *  // ...
 *  std::string s = enumString(d);   // s is "Friday"
 *	
 *	// stream-insertion provides the string-representation
 *	std::cout << d << "\n";     // yeilds Monday on stdout
 *
 * \endcode
 */

#ifndef ENUM_H_INCLUDED
#define ENUM_H_INCLUDED

#include <aqsis/aqsis.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include <boost/static_assert.hpp>

#include <aqsis/util/sstring.h> // for CqString::hash()

namespace Aqsis {

//------------------------------------------------------------------------------

/** \brief Convert from a string to an enum
 *
 * This function attempts to convert the provided string into an enum value of
 * the provided type, using information specified in an AQSIS_ENUM_INFO_BEGIN /
 * AQSIS_ENUM_INFO_END block.
 *
 * \param str - string representation of enumeration constant
 * \return Enumeration constant corresponding to str, or the default specified
 *         in the enum info block if the string doesn't represent a valid
 *         constant.
 */
template<typename EnumT>
EnumT enumCast(const std::string& str);

/** \brief Convert an enumeration constant to a string.
 *
 * This function attempts to convert the provided enum value into a string of
 * using information specified in an AQSIS_ENUM_INFO_BEGIN /
 * AQSIS_ENUM_INFO_END block.
 *
 * \param e - enumeration value to convert
 * \return a string representation of the enumeration constant.
 */
template<typename EnumT>
const std::string& enumString(EnumT e);

//------------------------------------------------------------------------------
/** \brief Define enum info block and stream IO operators.
 *
 * This macro begins a comma separated list of strings which define the string
 * names associated with the given enumeration.
 *
 * It also causes the standard operator<< and operator>> stream insertion and
 * extraction operators to be implemented for the enumeration type.  (Note that
 * we can't do this with a template, because it would pollute the namespace
 * with an operator<< which tries to use the enum conversion machinary for any
 * type.)
 *
 * \param EnumT - name of the enum type for which strings are to be provided
 * \param defaultValue - default value for the enum, used when string
 *                conversion in enumCast() fails.
 */
#define AQSIS_ENUM_INFO_BEGIN(EnumType, defValue)                              \
inline std::ostream& operator<<(std::ostream& out, EnumType e)                 \
{                                                                              \
	out << enumString(e);                                                      \
	return out;                                                                \
}                                                                              \
inline std::istream& operator>>(std::istream& in, EnumType& e)                 \
{                                                                              \
	std::string str;                                                           \
	in >> str;                                                                 \
	e = enumCast<EnumType>(str);                                               \
	return in;                                                                 \
}                                                                              \
namespace detail { \
template<>                                                                     \
inline CqEnumInfo<EnumType>::CqEnumInfo()                                      \
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
} \
}


//==============================================================================
// Implementation details
//==============================================================================

namespace detail {

/** \brief Class holding information about enum <---> string conversions.
 *
 * This class encapsulates a lookup scheme for converting strings into enum
 * constants and vice versa.  The current implementation only deals with enum
 * constants with the default ordering, but extending it would be fairly easy
 * if necessary.
 */
template<typename EnumT>
class CqEnumInfo
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
			bool operator()(TqHash p, const std::pair<TqHash, EnumT>& h)
			{
				return p < h.first;
			}
			bool operator()(const std::pair<TqHash, EnumT>& p, const std::pair<TqHash, EnumT>& h)
			{
				return p.first < h.first;
			}
		};

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
		}

		/// vector of name strings
		TqNameVec m_names;
		/// sorted vector of (name_hash, enum_const) pairs for fast lookups
		TqLookupVec m_lookup;
		/// default value for when string conversion fails and default constructor.
		EnumT m_defaultValue;

		/// Singleton instance
		static const CqEnumInfo m_instance;

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
			// Binary search on hashed strings.
			typename TqLookupVec::const_iterator pos =
				std::lower_bound(m_lookup.begin(), m_lookup.end(), h, SqHashCmp());
			// Find the correct string in the presence of hash collisions.
			for(typename TqLookupVec::const_iterator end = m_lookup.end();
					pos != end && pos->first == h; ++pos)
			{
				EnumT value = pos->second;
				if(stringFromValue(value) == str)
					return value;
			}
			return m_defaultValue;
		}
		/// Return the string corresponding to the provided enum value.
		const std::string& stringFromValue(EnumT value) const
		{
			assert(value >= 0);
			assert(value < static_cast<int>(m_names.size()));
			return m_names[value];
		}
		/// Return the default value for the enum
		EnumT defaultValue() const
		{
			return m_defaultValue;
		}

		static const CqEnumInfo& instance()
		{
			return m_instance;
		}
};

template<typename EnumT>
const CqEnumInfo<EnumT> CqEnumInfo<EnumT>::m_instance;

} // namespace detail


//------------------------------------------------------------------------------
// Free function implementations.
template<typename EnumT>
inline EnumT enumCast(const std::string& str)
{
	return detail::CqEnumInfo<EnumT>::instance().valueFromString(str);
}

template<typename EnumT>
inline const std::string& enumString(EnumT e)
{
	return detail::CqEnumInfo<EnumT>::instance().stringFromValue(e);
}


} // namespace Aqsis

#endif // ENUM_H_INCLUDED
