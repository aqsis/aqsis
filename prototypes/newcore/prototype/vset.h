// Aqsis
// Copyright (C) 1997 - 2010, Paul C. Gregory
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

#ifndef VSET_H_INCLUDED
#define VSET_H_INCLUDED

#include <cassert>
#include <algorithm>
#include <iterator>
#include <vector>

// forward declarations.
template<typename T> class VSet;

template<typename T1, typename T2, typename T3>
void setUnion(const VSet<T1>& set1, const VSet<T2>& set2, VSet<T3>& result);
template<typename T1, typename T2, typename T3>
void setIntersection(const VSet<T1>& set1, const VSet<T2>& set2, VSet<T3>& result);


/// Set class using a std::vector as the object container.
///
/// The idea of this set is that it should be more efficient in memory
/// allocations and cache coherence than std::set for element containment
/// testing and whole-set operations like union and intersection.  On the
/// other hand, it's not designed to be modified in a per-element manner since
/// doing so using the sorted std::vector implementation won't be efficient.
///
template<class T>
class VSet
{
    private:
        typedef std::vector<T> ContainerType;
        ContainerType m_elems;

    public:
        typedef T value_type;
        /// Iterator types
        typedef typename ContainerType::iterator iterator;
        typedef typename ContainerType::const_iterator const_iterator;

        /// Create an empty set
        VSet() : m_elems() { }

        /// Initialize the set with the unordered sequence [b, e)
        template<typename ForwardIterT>
        VSet(ForwardIterT b, ForwardIterT e)
            : m_elems()
        {
            assign(b, e);
        }

        /// Initialize the set with the unordered sequence [b, e)
        template<typename ForwardIterT>
        void assign(ForwardIterT b, ForwardIterT e)
        {
            m_elems.assign(b, e);
            std::sort(m_elems.begin(), m_elems.end());
        }

        /// Iterator access to the contained, sorted, sequence
        iterator begin() { return m_elems.begin(); }
        iterator end() { return m_elems.end(); }
        const_iterator begin() const { return m_elems.begin(); }
        const_iterator end() const { return m_elems.end(); }

        /// Random element access.
        const value_type& operator[](int i) const
        {
            assert(i >= 0 && i < m_elems.size());
            return m_elems[i];
        }

        /// Get the number of elements
        int size() const { return m_elems.size(); }

        /// Test whether elem is contained within the set
        bool contains(const T& elem) const
        {
            return std::binary_search(m_elems.begin(), m_elems.end(), elem);
        }

        /// Test whether s is a subset of this set
        bool includes(const VSet& s) const
        {
            return std::includes(m_elems.begin(), m_elems.end(),
                                 s.begin(), s.end());
        }

        /// Set union between s1 & s2.
        template<typename T1, typename T2, typename T3>
        friend void setUnion(const VSet<T1>& set1, const VSet<T2>& set2,
                             VSet<T3>& result);

        /// Set intersection between s1 & s2.
        template<typename T1, typename T2, typename T3>
        friend void setIntersection(const VSet<T1>& set1, const VSet<T2>& set2,
                                    VSet<T3>& result);
};


//==============================================================================

template<typename T1, typename T2, typename T3>
void setUnion(const VSet<T1>& set1, const VSet<T2>& set2,
                        VSet<T3>& result)
{
    typename VSet<T3>::ContainerType& r = result.m_elems;
    const typename VSet<T1>::ContainerType& s1 = set1.m_elems;
    const typename VSet<T2>::ContainerType& s2 = set2.m_elems;
    r.reserve(s1.size() + s2.size());
    std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(),
                    std::back_inserter(r));
}

template<typename T1, typename T2, typename T3>
void setIntersection(const VSet<T1>& set1, const VSet<T2>& set2,
                            VSet<T3>& result)
{
    typename VSet<T3>::ContainerType& r = result.m_elems;
    const typename VSet<T1>::ContainerType& s1 = set1.m_elems;
    const typename VSet<T2>::ContainerType& s2 = set2.m_elems;
    r.reserve(std::min(s1.size(), s2.size()));
    std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(),
                            std::back_inserter(r));
}

#endif // VSET_H_INCLUDED
