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

#include "vset.h"

#include <vector>
#include <set>
#include <cstdlib>
#include <iostream>

#define ARRAYEND(array) (array + sizeof(array)/sizeof(array[0]))

__attribute__((noinline))
int contains(const VSet<int>& s, int e)
{
    return s.contains(e);
}

__attribute__((noinline))
int contains(const std::set<int>& s, int e)
{
    return s.find(e) != s.end();
}

void speedTest()
{
    const int timingIters = 10;
    const int nsets = 1000000;
    const int nelem = 10;
    // Create some set data
    std::vector<int> s1Init;
    for(int i = 0; i < nelem; ++i)
        s1Init.push_back(rand());
    int* b = &s1Init[0];
    int* e = &s1Init.back()+1;
    // Create some sets; all the same actually.
    typedef VSet<int> Set;
//    typedef std::set<int> Set;
    std::vector<Set> sets;
    for(int j = 0; j < nsets; ++j)
        sets.push_back(Set(b, e));
    // Timing loop
    long sum = 0;
    for(long iter = 0; iter < timingIters; ++iter)
    {
        for(int j = 0; j < nsets; ++j)
        {
            for(int i = 0; i < nelem; ++i)
                sum += contains(sets[j], s1Init[i]);
        }
    }
    std::cout << sum << "\n";
}

template<typename T>
std::ostream& operator<<(std::ostream& out, const VSet<T>& set)
{
    out << "{ ";
    for(typename VSet<T>::const_iterator i = set.begin(), e = set.end(); i != e; ++i)
        out << *i << " ";
    out << "}";
    return out;
}

void test()
{
    int s1Init[] = {1,6,7,3,99};
    VSet<int> s1(s1Init, ARRAYEND(s1Init));
    int s2Init[] = {1,-4,42,7,100};
    VSet<int> s2(s2Init, ARRAYEND(s2Init));
    std::cout << s1 << "\n";

    std::cout << "s1.contains(1) = " << s1.contains(1) << "\n";
    std::cout << "s1.contains(-1) = " << s1.contains(-1) << "\n";

    int smallInit[] = {1,6};
    VSet<int> small(smallInit, ARRAYEND(smallInit));
    std::cout << s1 << ".includes(" << small << ") = " << s1.includes(small) << "\n";


    VSet<int> u;
    setUnion(s1, s2, u);
    std::cout << "union(" << s1 << ", " << s2 << ") = " << u << "\n";
    VSet<int> i;
    setIntersection(s1, s2, i);
    std::cout << "intersection(" << s1 << ", " << s2 << ") = " << i << "\n";
}

int main()
{
//    speedTest();
    test();
    return 0;
};
