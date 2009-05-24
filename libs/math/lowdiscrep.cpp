// Aqsis
// Copyright (C) 1997 - 2005, Paul C. Gregory
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
        \brief Declares the CqLowDiscrepancy class responsible for producing low-discrepancy numbers.
        \author Andrew Bromage (ajb@spamcop.net)
*/

#include    <stdlib.h>
#include    <stdio.h>

#include    <aqsis/aqsis.h>
#include    <aqsis/math/lowdiscrep.h>
#include    <boost/array.hpp>
#include    <algorithm>

namespace
{

// All prime numbers less than 2*3*5*7*11.
const boost::array<TqUint,343>
s_primes =
    {
        {
            2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53,
            59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113,
            127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181,
            191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251,
            257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317,
            331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397,
            401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463,
            467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 547, 557,
            563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619,
            631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691, 701,
            709, 719, 727, 733, 739, 743, 751, 757, 761, 769, 773, 787,
            797, 809, 811, 821, 823, 827, 829, 839, 853, 857, 859, 863,
            877, 881, 883, 887, 907, 911, 919, 929, 937, 941, 947, 953,
            967, 971, 977, 983, 991, 997, 1009, 1013, 1019, 1021, 1031,
            1033, 1039, 1049, 1051, 1061, 1063, 1069, 1087, 1091, 1093,
            1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163, 1171,
            1181, 1187, 1193, 1201, 1213, 1217, 1223, 1229, 1231, 1237,
            1249, 1259, 1277, 1279, 1283, 1289, 1291, 1297, 1301, 1303,
            1307, 1319, 1321, 1327, 1361, 1367, 1373, 1381, 1399, 1409,
            1423, 1427, 1429, 1433, 1439, 1447, 1451, 1453, 1459, 1471,
            1481, 1483, 1487, 1489, 1493, 1499, 1511, 1523, 1531, 1543,
            1549, 1553, 1559, 1567, 1571, 1579, 1583, 1597, 1601, 1607,
            1609, 1613, 1619, 1621, 1627, 1637, 1657, 1663, 1667, 1669,
            1693, 1697, 1699, 1709, 1721, 1723, 1733, 1741, 1747, 1753,
            1759, 1777, 1783, 1787, 1789, 1801, 1811, 1823, 1831, 1847,
            1861, 1867, 1871, 1873, 1877, 1879, 1889, 1901, 1907, 1913,
            1931, 1933, 1949, 1951, 1973, 1979, 1987, 1993, 1997, 1999,
            2003, 2011, 2017, 2027, 2029, 2039, 2053, 2063, 2069, 2081,
            2083, 2087, 2089, 2099, 2111, 2113, 2129, 2131, 2137, 2141,
            2143, 2153, 2161, 2179, 2203, 2207, 2213, 2221, 2237, 2239,
            2243, 2251, 2267, 2269, 2273, 2281, 2287, 2293, 2297, 2309
        }
    };


// Compute the reverse radical of p_n in base p_b.
// Because the primes are all fairly small, we can safely perform
// the sum in forward order and not get too much round-off error.
//
TqFloat
reverseRadical(TqUint p_n, TqUint p_base)
{
	TqFloat invbase = 1.0 / p_base;
	TqFloat scale = invbase;
	TqFloat value = 0;
	for (; p_n != 0; p_n /= p_base)
	{
		value += (p_n % p_base) * scale;
		scale *= invbase;
	}

	return value;
}


} // Anonymous namespace


namespace Aqsis {


// Constructor
//
CqLowDiscrepancy::CqLowDiscrepancy(TqUint p_dim)
		: m_NextBase(2), m_Dimensions(p_dim)
{
	m_Bases.resize(p_dim);
	Reset();
}


// Reset to the next set of bases.
//
void
CqLowDiscrepancy::Reset()
{
	// Pick m_Dimensions bases randomly.
	unsigned low = 0;
	unsigned high = m_Bases.size() - m_Dimensions;
	unsigned i;
	for (i = 0; i < m_Dimensions; ++i)
	{
		unsigned j = m_Random.RandomInt(high - low) + low;
		m_Bases[i] = s_primes[j];
		low = j + 1;
		++high;
	}

	// Now randomly permute.
	for (i = m_Dimensions - 1; i > 0; --i)
	{
		unsigned j = m_Random.RandomInt(i - 1);
		std::swap(m_Bases[i], m_Bases[j]);
	}
}


// Generate Hammersley points
//
TqFloat
CqLowDiscrepancy::Generate(TqUint p_axis, TqUint p_i)
{
	return reverseRadical(p_i, m_Bases[p_axis]);
}


//-----------------------------------------------------------------------

} // namespace Aqsis

// vim: ts=4:sts=4:expandtab
