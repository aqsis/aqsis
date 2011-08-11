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

/// \file
/// \brief Ri::Renderer frame dropping
/// \author Chris Foster [chris42f (at) g mail (d0t) com]

#include <aqsis/riutil/ricxx_filter.h>

#include <algorithm>
#include <cstdlib>
#include <set>
#include <vector>

#include <iostream>

#include <aqsis/util/exception.h>

namespace Aqsis {

//------------------------------------------------------------------------------
class FrameDropFilter : public OnOffFilter
{
    private:
        std::set<int> m_desiredFrames;

    public:
        FrameDropFilter(const std::vector<int>& desiredFrames)
            : m_desiredFrames(desiredFrames.begin(), desiredFrames.end())
        { }

        RtVoid FrameBegin(RtInt number)
        {
            setActive(m_desiredFrames.find(number) != m_desiredFrames.end());
            if(isActive())
                return nextFilter().FrameBegin(number);
        }

        RtVoid FrameEnd()
        {
            if(isActive())
                nextFilter().FrameEnd();
            setActive(true);
        }
};


static void parseFrames(const Ri::ParamList& pList,
                        std::vector<int>& desiredFrames)
{
    desiredFrames.clear();
    // search for the "frames" parameter; if it's an integer array, just use
    // those frames.
    int idx = pList.find(Ri::TypeSpec(Ri::TypeSpec::Integer), "frames");
    if(idx >= 0)
    {
        desiredFrames.assign(pList[idx].intData().begin(),
                             pList[idx].intData().end());
        return;
    }
    idx = pList.find(Ri::TypeSpec(Ri::TypeSpec::String), "frames");
    if(idx < 0 || pList[idx].size() == 0)
        AQSIS_THROW_XQERROR(XqValidation, EqE_BadToken, "no frames found");
    // If it's a string array, parse it as a comma separated list of ranges, eg,
    // "1,3,5,6-9,10-20"
    const char* frames = pList[idx].stringData()[0];
    const char* nptr = frames;
    while(*nptr)
    {
        char *nend = 0;
        int f1 = std::strtol(nptr, &nend, 10);
        if(nend != nptr)
        {
            // Check for a range.
            if(*nend == '-')
            {
                nptr = nend+1;
                int f2 = std::strtol(nptr, &nend, 10);
                if(nend != nptr)
                {
                    // Store the range between f1 and f2;
                    int fmin = std::min(f1, f2);
                    int fmax = std::max(f1, f2);
                    for(int i = fmin; i <= fmax; i++)
                        desiredFrames.push_back(i);
                    nptr = nend;
                }
                else
                    AQSIS_THROW_XQERROR(XqValidation, EqE_BadToken,
                            "unrecognized frame range \"" << frames << "\"");
            }
            else
            {
                desiredFrames.push_back(f1);
                nptr = nend;
            }
        }
        else if(*nptr == ',')
            ++nptr;
        else
            AQSIS_THROW_XQERROR(XqValidation, EqE_BadToken,
                    "bad character in frame list \"" << frames << "\"" << *nptr);
    }
}

Ri::Filter* createFrameDropFilter(const Ri::ParamList& pList)
{
    std::vector<int> desiredFrames;
    parseFrames(pList, desiredFrames);
    return new FrameDropFilter(desiredFrames);
}

} // namespace Aqsis

// vi: set et:
