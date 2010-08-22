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
///
/// \brief ErrorHandler implementation
/// \author Chris Foster [chris42f (at) g mail (d0t) com]
///
#ifndef AQSIS_ERRORHANDLER_IMPL_INCLUDED
#define AQSIS_ERRORHANDLER_IMPL_INCLUDED

#include <aqsis/riutil/errorhandler.h>

#include <aqsis/util/logging.h>

namespace Aqsis {

/// An error handler which just sends errors to the Aqsis::log() stream.
class AqsisLogErrorHandler : public Ri::ErrorHandler
{
    public:
        explicit AqsisLogErrorHandler(ErrorCategory verbosity = Warning)
            : ErrorHandler(verbosity)
        { }

    protected:
        virtual void sendError(int code, const std::string& message)
        {
            std::ostream& out = Aqsis::log();
            switch(errorCategory(code))
            {
                case Debug:   out << debug    << message << std::endl; break;
                case Info:    out << info     << message << std::endl; break;
                case Warning: out << warning  << message << std::endl; break;
                case Error:   out << error    << message << std::endl; break;
                case Severe:  out << critical << message << std::endl; break;
                case Message: out << info     << message << std::endl; break;
            }
        }
};

} // namespace Aqsis

#endif // AQSIS_ERRORHANDLER_IMPL_INCLUDED
// vi: set et:
