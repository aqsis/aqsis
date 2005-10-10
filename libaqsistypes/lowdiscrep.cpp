// Aqsis
// Copyright © 1997 - 2005, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
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

#include    "aqsis.h"
#include    "lowdiscrep.h"
#include    <boost/array.hpp>
#include    <boost/static_assert.hpp>
#include    <algorithm>
#include    <limits>

namespace {

// XXX This prime stuff should almost certainly be factored out.  It
// could come in handy if we ever need to size a hash table or something.

// Low-discrepancy points rely on prime bases.  So we need to be able
// to generate prime numbers.  Thankfully, the prime number search isn't
// in an inner loop, but even so, we shouldn't waste time.
//
// The wheel factorisation method relies on taking a set of very small
// primes (we use 2, 3, 5, 7 and 11), and multiplying them together to
// form a modulus.  We precompute all numbers which are relatively prime
// to this modulus, and only search those numbers for factors.
//
// See http://primes.utm.edu/glossary/page.php?sort=WheelFactorization

// The modulus of the wheel.
const TqUint
s_wheelModulus = 2*3*5*7*11;

// All prime numbers less than s_wheelModulus.
const boost::array<TqUint,343>
s_smallPrimes = {{
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
}};


// All numbers less than s_wheelModulus such that
// gcd(n, s_wheelModulus) == 1.
const boost::array<TqUint,480>
s_wheelSettings = {{
    1, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67,
    71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131,
    137, 139, 149, 151, 157, 163, 167, 169, 173, 179, 181, 191,
    193, 197, 199, 211, 221, 223, 227, 229, 233, 239, 241, 247,
    251, 257, 263, 269, 271, 277, 281, 283, 289, 293, 299, 307,
    311, 313, 317, 323, 331, 337, 347, 349, 353, 359, 361, 367,
    373, 377, 379, 383, 389, 391, 397, 401, 403, 409, 419, 421,
    431, 433, 437, 439, 443, 449, 457, 461, 463, 467, 479, 481,
    487, 491, 493, 499, 503, 509, 521, 523, 527, 529, 533, 541,
    547, 551, 557, 559, 563, 569, 571, 577, 587, 589, 593, 599,
    601, 607, 611, 613, 617, 619, 629, 631, 641, 643, 647, 653,
    659, 661, 667, 673, 677, 683, 689, 691, 697, 701, 703, 709,
    713, 719, 727, 731, 733, 739, 743, 751, 757, 761, 767, 769,
    773, 779, 787, 793, 797, 799, 809, 811, 817, 821, 823, 827,
    829, 839, 841, 851, 853, 857, 859, 863, 871, 877, 881, 883,
    887, 893, 899, 901, 907, 911, 919, 923, 929, 937, 941, 943,
    947, 949, 953, 961, 967, 971, 977, 983, 989, 991, 997, 1003,
    1007, 1009, 1013, 1019, 1021, 1027, 1031, 1033, 1037, 1039,
    1049, 1051, 1061, 1063, 1069, 1073, 1079, 1081, 1087, 1091,
    1093, 1097, 1103, 1109, 1117, 1121, 1123, 1129, 1139, 1147,
    1151, 1153, 1157, 1159, 1163, 1171, 1181, 1187, 1189, 1193,
    1201, 1207, 1213, 1217, 1219, 1223, 1229, 1231, 1237, 1241,
    1247, 1249, 1259, 1261, 1271, 1273, 1277, 1279, 1283, 1289,
    1291, 1297, 1301, 1303, 1307, 1313, 1319, 1321, 1327, 1333,
    1339, 1343, 1349, 1357, 1361, 1363, 1367, 1369, 1373, 1381,
    1387, 1391, 1399, 1403, 1409, 1411, 1417, 1423, 1427, 1429,
    1433, 1439, 1447, 1451, 1453, 1457, 1459, 1469, 1471, 1481,
    1483, 1487, 1489, 1493, 1499, 1501, 1511, 1513, 1517, 1523,
    1531, 1537, 1541, 1543, 1549, 1553, 1559, 1567, 1571, 1577,
    1579, 1583, 1591, 1597, 1601, 1607, 1609, 1613, 1619, 1621,
    1627, 1633, 1637, 1643, 1649, 1651, 1657, 1663, 1667, 1669,
    1679, 1681, 1691, 1693, 1697, 1699, 1703, 1709, 1711, 1717,
    1721, 1723, 1733, 1739, 1741, 1747, 1751, 1753, 1759, 1763,
    1769, 1777, 1781, 1783, 1787, 1789, 1801, 1807, 1811, 1817,
    1819, 1823, 1829, 1831, 1843, 1847, 1849, 1853, 1861, 1867,
    1871, 1873, 1877, 1879, 1889, 1891, 1901, 1907, 1909, 1913,
    1919, 1921, 1927, 1931, 1933, 1937, 1943, 1949, 1951, 1957,
    1961, 1963, 1973, 1979, 1987, 1993, 1997, 1999, 2003, 2011,
    2017, 2021, 2027, 2029, 2033, 2039, 2041, 2047, 2053, 2059,
    2063, 2069, 2071, 2077, 2081, 2083, 2087, 2089, 2099, 2111,
    2113, 2117, 2119, 2129, 2131, 2137, 2141, 2143, 2147, 2153,
    2159, 2161, 2171, 2173, 2179, 2183, 2197, 2201, 2203, 2207,
    2209, 2213, 2221, 2227, 2231, 2237, 2239, 2243, 2249, 2251,
    2257, 2263, 2267, 2269, 2273, 2279, 2281, 2287, 2291, 2293,
    2297, 2309
}};


// Test a number to see if it's prime.
//
bool
isPrime(TqUint p_n)
{
    TqUint nn = 0; 
    const TqUint* ii = s_smallPrimes.begin();
    const TqUint* iend = s_smallPrimes.end();

    for (;;)
    {
        for (; ii != iend; ++ii)
        {
            unsigned f = nn + *ii;
            if (f * f > p_n)
            {
                return true;
            }

            if (p_n % f == 0)
            {
                return false;
            }
        }

        nn += s_wheelModulus;
        ii = s_wheelSettings.begin();
        iend = s_wheelSettings.end();
    }
}


// Get the nth prime number.
//
TqUint
prime(TqUint p_seed)
{
    if (p_seed < s_smallPrimes.size())
    {
        return s_smallPrimes[p_seed];
    }

    TqUint nn = s_wheelModulus;
    for (;;)
    {
        const TqUint* ii = s_wheelSettings.begin();
        const TqUint* iend = s_wheelSettings.end();

        for (; ii != iend; ++ii)
        {
            TqUint p = nn + *ii;
            if (isPrime(p))
            {
                if (p_seed-- == 0)
                {
                    return p;
                }
            }
        }
        nn += s_wheelModulus;
    }
}


// Get the next prime after p_p.
//
TqUint
nextPrime(TqUint p_p)
{
    if (p_p < s_smallPrimes.back())
    {
        const TqUint* i = std::lower_bound(s_smallPrimes.begin(),
                                           s_smallPrimes.end(), p_p);
        return *(++i);
    }

    TqUint nn = (p_p / s_wheelModulus) * s_wheelModulus;
    TqUint pp = p_p - nn;

    const TqUint* ii = std::lower_bound(s_wheelSettings.begin(),
                                        s_wheelSettings.end(), pp);
    const TqUint* ie = s_wheelSettings.end();
    for (;;)
    {
        for (; ii != ie; ++ii)
        {
            TqUint p = nn + *ii;
            if (isPrime(p))
            {
                return p;
            }
        }

        nn += s_wheelModulus;
        ii = s_wheelSettings.begin();
    }
}


// Compute the reverse radical of p_n in base p_b.
// To avoid round-off error, we perform the sum backwards.  This
// may make the computation a little more expensive than it otherwise
// would be.
//
TqFloat
reverseRadical(TqUint p_n, TqUint p_base)
{
    BOOST_STATIC_ASSERT(std::numeric_limits<TqUint>::radix == 2);
    TqFloat stack[std::numeric_limits<TqUint>::digits];

    int i;
    TqFloat invbase = 1.0 / p_base;
    TqFloat scale = invbase;
    for (i = 0; p_n != 0; ++i, p_n /= p_base)
    {
        stack[i] = (p_n % p_base) * scale;
        scale *= invbase;
    }

    TqFloat value = 0;
    for (--i; i >= 0; --i)
    {
        value += stack[i];
    }

    return value;
}


} // Anonymous namespace


START_NAMESPACE( Aqsis )


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
    for (TqUint i = 0; i < m_Dimensions; ++i)
    {
        m_Bases[i] = m_NextBase;
        m_NextBase = nextPrime(m_NextBase);
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

END_NAMESPACE( Aqsis )

// vim: ts=4:sts=4:expandtab
