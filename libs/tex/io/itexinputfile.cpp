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

/** \file
 *
 * \brief Interface wrapper functions and factory functions for texture file
 * interface.
 *
 * \author Chris Foster
 */

#include <aqsis/tex/io/itexinputfile.h>

#include "exrinputfile.h"
#include <aqsis/util/logging.h>
#include "magicnumber.h"
#include "tiffinputfile.h"
#ifdef AQSIS_USE_PNG
#	include "pnginputfile.h"
#endif
#include "zinputfile.h"

namespace Aqsis {

namespace {

// Helper functions

// Open multi-image input file
boost::shared_ptr<IqMultiTexInputFile> openMultiInputFile(
		EqImageFileType type, const boostfs::path& fileName)
{
	boost::shared_ptr<IqMultiTexInputFile> file;
	switch(type)
	{
		case ImageFile_Tiff:
			file.reset(new CqTiffInputFile(fileName));
			break;
		default:
			break;
	}
	return file;
}

// Open a simple input file
boost::shared_ptr<IqTexInputFile> openInputFile(
		EqImageFileType type, const boostfs::path& fileName)
{
	boost::shared_ptr<IqTexInputFile> file = openMultiInputFile(type, fileName);
	if(!file)
	{
		switch(type)
		{
			case ImageFile_Exr:
#				ifdef USE_OPENEXR
				file.reset(new CqExrInputFile(fileName));
#				else
				AQSIS_THROW_XQERROR(XqInvalidFile, EqE_Unimplement,
						"Cannot open file \"" << fileName << "\""
						": Aqsis was compiled without OpenEXR support");
#				endif
				break;
			case ImageFile_AqsisZfile:
				file.reset(new CqZInputFile(fileName));
				break;
			case ImageFile_Png:
#				ifdef AQSIS_USE_PNG
				file.reset(new CqPngInputFile(fileName));
#				else
				AQSIS_THROW_XQERROR(XqInvalidFile, EqE_Unimplement,
						"Cannot open file \"" << fileName << "\""
						": Aqsis was compiled without PNG support");
#				endif
				break;
			default:
				break;
		}
	}
	return file;
}

} // unnamed namespace 

//------------------------------------------------------------------------------
// IqTexInputFile

boost::shared_ptr<IqTexInputFile> IqTexInputFile::open(const boostfs::path& fileName)
{
	boost::shared_ptr<IqTexInputFile> file = openInputFile(guessFileType(fileName), fileName);
	if(file)
		return file;
	else
		AQSIS_THROW_XQERROR(XqInvalidFile, EqE_BadFile, "Unknown file type for \""
				<< fileName << "\"");

	assert(0);
	return boost::shared_ptr<IqTexInputFile>();
}

//------------------------------------------------------------------------------
// IqMultiTexInputFile

boost::shared_ptr<IqMultiTexInputFile> IqMultiTexInputFile::open(
		const boostfs::path& fileName)
{
	EqImageFileType type = guessFileType(fileName);
	boost::shared_ptr<IqMultiTexInputFile> file
		= openMultiInputFile(type, fileName);
	if(file)
		return file;
	else
	{
		AQSIS_THROW_XQERROR(XqInvalidFile, EqE_BadFile,
			"File \"" << fileName << "\" of type " << type << " doesn't support "
			"multiple subimages.");
	}

	assert(0);
	return boost::shared_ptr<IqMultiTexInputFile>();
}

} // namespace Aqsis
