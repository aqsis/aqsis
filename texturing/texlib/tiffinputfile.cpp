#include "tiffinputfile.h"

#include "tiffdirhandle.h"

CqTiffInputFile::CqTiffInputFile(const std::string& fileName)
	: m_fileHandle(new CqTiffFileHandle(fileName, "r")),
	m_header()
{
	CqTiffDirHandle dirHandle(m_fileHandle);
	if(!dirHandle.isTiled())
		fillHeader(m_header, dirHandle);
	else
		throw XqInternal("Can't read tiled tiff files", __FILE__, __LINE__);

	// Check that the tiff file is of a type which we know how to handle
	// easily.  If it's not, then we should use the generic RGBA handling of
	// libtiff to read the data (probably slower; doesn't preserve precision (?))
	if( !(
		// require either RGB, or grayscale photometric interpretation.
		( newDirectory.photometricInterp == PHOTOMETRIC_RGB
			|| newDirectory.photometricInterp == PHOTOMETRIC_MINISBLACK )
		// require directly addressable data samples
		&& (newDirectory.bitsPerSample % 8) == 0
		// require (0,0) coord to be in the top left
		&& newDirectory.orientation == ORIENTATION_TOPLEFT
		// require that colour values are stored packed together, not seperately.
		&& newDirectory.planarConfig == PLANARCONFIG_CONTIG
		// require that the sample type is either uint, int, or floating point.
		&& (newDirectory.sampleFormat == SAMPLEFORMAT_UINT
			|| newDirectory.sampleFormat == SAMPLEFORMAT_INT
			|| newDirectory.sampleFormat == SAMPLEFORMAT_IEEEFP)
		) )
	{
		// for the moment, just throw an error...
		throw XqTiffError("Unrecognised tiff format", __FILE__, __LINE__);
	}
}

const std::string& CqTiffInputFile::fileName() const
{
	return m_fileHandle->fileName();
}

virtual void CqTiffInputFile::readPixelsImpl(CqTextureBufferBase& buffer,
		TqInt startLine, TqInt numScanlines) const
{
	
}


