// -*- C++ -*-
// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
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
 *  \brief RiContext and Options parsing class
 *  \author Lionel J. Lacour (intuition01@online.fr)
 */

#ifndef RI2RIB_CONTEXT_H
#define RI2RIB_CONTEXT_H 1

#include <list>
#include <aqsis/ri/ri.h>
#include <aqsis/aqsis.h>
#include "options.h"
#include "output.h"

namespace libri2rib {


class CqContext
{
		std::list<CqOutput *> m_lContextHandle;
		CqOutput *m_Active;

		bool m_PipeHandleSet;
		int m_PipeHandle;

		SqOptions::EqOutputType m_OutputType;
		SqOptions::EqCompression m_Compression;
		SqOptions::EqIndentation m_Indentation;
		TqInt m_IndentSize;

		void parseOutputType( RtInt n, RtToken[], RtPointer params[] );
		void parseIndentation( RtInt n, RtToken[], RtPointer params[] );

	public:
		CqContext();
		~CqContext()
		{}

		void addContext( RtToken name );
		RtContextHandle getContext();
		CqOutput & current();
		void switchTo ( RtContextHandle );
		void removeCurrent();

		void parseOption( const char *name, RtInt n, RtToken tokens[], RtPointer params[] );
};

} // namespace libri2rib
#endif
