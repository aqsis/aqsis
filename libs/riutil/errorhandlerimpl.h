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
        virtual void dispatch(int code, const std::string& message)
        {
            std::ostream& out = Aqsis::log();
            switch(errorCategory(code))
            {
                case Debug:   out << Aqsis::debug    << message << std::endl; break;
                case Info:    out << Aqsis::info     << message << std::endl; break;
                case Warning: out << Aqsis::warning  << message << std::endl; break;
                case Error:   out << Aqsis::error    << message << std::endl; break;
                case Severe:  out << Aqsis::critical << message << std::endl; break;
                case Message: out << Aqsis::info     << message << std::endl; break;
            }
        }
};

} // namespace Aqsis

#endif // AQSIS_ERRORHANDLER_IMPL_INCLUDED
// vi: set et:
