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

/** \file
 *
 * \brief Interface wrapper functions and factory functions for texture file
 * interface.
 *
 * \author Chris Foster
 */

#include "itexinputfile.h"

#include "tiffinputfile.h"
#include "exrinputfile.h"
#include "magicnumber.h"
#include "logging.h"

namespace Aqsis {

//------------------------------------------------------------------------------
// IqTexInputFile
//------------------------------------------------------------------------------

boost::shared_ptr<IqTexInputFile> IqTexInputFile::open(const std::string& fileName)
{
	boost::shared_ptr<IqTexInputFile> newFile;

	TqMagicNumberPtr magicNum = getMagicNumber(fileName);
	if(isTiffMagicNumber(*magicNum))
		newFile.reset(new CqTiffInputFile(fileName));
	else if(isOpenExrMagicNumber(*magicNum))
	{
#		ifdef USE_OPENEXR
		newFile.reset(new CqExrInputFile(fileName));
#		else
		throw XqInternal("Aqsis was compiled without OpenEXR support", __FILE__, __LINE__);
#		endif
	}
	// else if(Add new formats here!)
	else
		throw XqInternal("Unknown file type", __FILE__, __LINE__);

	return newFile;
}

} // namespace Aqsis
