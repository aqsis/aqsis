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

#include "ricxx2ri.h"

#include <set>

#include "ricxx_filter.h"

namespace Aqsis {

//------------------------------------------------------------------------------
class FrameDropFilter : public OnOffFilter
{
    private:
        std::set<int> m_desiredFrames;

    public:
        FrameDropFilter(Ri::RendererServices& services, Ri::Renderer& out,
                       const IntArray& desiredFrames)
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


Ri::Renderer* createFrameDropFilter(Ri::RendererServices& serv,
                    Ri::Renderer& out, const Ri::ParamList& pList)
{
    int idx = pList.find(Ri::TypeSpec(Ri::TypeSpec::Integer), "frames");
    if(idx < 0)
        return 0;
    return new FrameDropFilter(serv, out, pList[idx].intData());
}

} // namespace Aqsis

// vi: set et:
