// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
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
		\brief Declares the CqJitter class responsible for jittering the sampling points.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is jitter.h included already?
#ifndef JITTER_H_INCLUDED
//{
#define JITTER_H_INCLUDED 1

#include	"aqsis.h"

#include	"bucket.h"

namespace Aqsis {

//-----------------------------------------------------------------------


class CqJitter
{
	public:
		CqJitter();
		~CqJitter();

		void jitterSamples(CqBucket::TqSampleList& samples, TqInt xres, TqInt yres);

	private:
};

} // namespace Aqsis

//}  // End of #ifdef JITTER_H_INCLUDED
#endif
