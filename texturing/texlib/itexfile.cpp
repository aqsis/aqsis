#include "itexfile.h"

#include "tiffinputfile.h"

//------------------------------------------------------------------------------
// IqTexInputFile
//------------------------------------------------------------------------------


void IqTextureInputFile::readPixels(CqTextureBufferBase& buffer,
		TqInt startLine, TqInt numScanlines) const
{
	if(numScanlines <= 0)
		numScanlines = header().height() - startLine;
	if(startLine < 0 || startLine + numScanlines > header().height())
		throw XqInternal("Attempt to read scanlines outside image boundaries",
				__FILE__, __LINE__);
	readPixelsImpl(buffer, startLine, numScanlines);
}

boost::shared_ptr<IqTexInputFile> IqTextureInputFile::open(const std::string& fileName)
{
	boost::shared_ptr<IqTexInputFile> newFile;

	// Only TIFF for the moment...
	newFile = boost::shared_ptr<IqTexInputFile>(new CqTiffInputFile(fileName));

	return newFile;
}
