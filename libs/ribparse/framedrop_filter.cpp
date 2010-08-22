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
        FrameDropFilter(Ri::RendererServices& services, Ri::Renderer& out,
                        const std::vector<int>& desiredFrames)
            : OnOffFilter(services, out),
            m_desiredFrames(desiredFrames.begin(), desiredFrames.end())
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

Ri::Renderer* createFrameDropFilter(Ri::RendererServices& serv,
                    Ri::Renderer& out, const Ri::ParamList& pList)
{
    std::vector<int> desiredFrames;
    parseFrames(pList, desiredFrames);
    return new FrameDropFilter(serv, out, desiredFrames);
}

} // namespace Aqsis

// vi: set et:
