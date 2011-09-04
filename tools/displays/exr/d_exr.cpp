///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2003, Industrial Light & Magic, a division of Lucas
// Digital Ltd. LLC
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// *       Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// *       Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
// *       Neither the name of Industrial Light & Magic nor the names of
// its contributors may be used to endorse or promote products derived
// from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//
//      PhotoRealistic RenderMan display driver that outputs
//      floating-point image files, using ILM's IlmImf library.
//
//      When you use this display driver for RGBA or Z output, you should
//      turn RGBA and Z quantization off by adding the following lines to
//      your RIB file:
//
//          Quantize "rgba" 0 0 0 0
//          Quantize "z"    0 0 0 0
//
//      Like Pixar's Tiff driver, this display driver can output image
//      channels other than R, G, B and A; for details on RIB file and
//      shader syntax, see the Renderman Release Notes (New Display
//      System, RGBAZ Output Images, Arbitrary Output Variables).
//
//      This driver maps Renderman's output variables to image channels
//      as follows:
//
//      Renderman output        image channel           image channel
//      variable name           name                    type
//      --------------------------------------------------------------
//
//      "r"                     "R"                     HALF
//
//      "g"                     "G"                     HALF
//
//      "b"                     "B"                     HALF
//
//      "a"                     "A"                     HALF
//
//      "z"                     "Z"                     FLOAT
//
//      other                   same as output          preferred type
//                              variable name           (see below)
//
//      By default, the "preferred" channel type is HALF; the
//      preferred type can be changed by adding an "exrpixeltype"
//      argument to the Display command in the RIB file.
//      For example:
//
//          Declare "exrpixeltype" "string"
//
//          # Store point positions in FLOAT format
//          Display "gnome.points.exr" "exr" "P" "exrpixeltype" "float"
//
//      The default compression method for the image's pixel data
//      is defined in ImfHeader.h.  You can select a different
//      compression method by adding an "exrcompression" argument
//      to the Display command.  For example:
//
//          Declare "exrcompression" "string"
//
//          # Store RGBA using run-length encoding
//          Display "gnome.rgba.exr" "exr" "rgba" "exrcompression" "rle"
//
//      See function DspyImageOpen(), below, for a list of valid
//      "exrpixeltype" and "exrcompression" values.
//
//-----------------------------------------------------------------------------

#include <aqsis/aqsis.h>

#include <assert.h>

#include <boost/shared_ptr.hpp>

// Lower the warning level to eliminate unavoidable warnings from the OpenEXR headers.
#if AQSIS_SYSTEM_WIN32 && (defined(AQSIS_COMPILER_MSVC6) || defined(AQSIS_COMPILER_MSVC7))
#	pragma warning(push,1)
#endif
#include <OpenEXR/ImfOutputFile.h>
#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfIntAttribute.h>
#include <OpenEXR/ImfFloatAttribute.h>
#include <OpenEXR/ImfMatrixAttribute.h>
#include <OpenEXR/ImfStringAttribute.h>
#include <OpenEXR/ImfChromaticitiesAttribute.h>
#include <OpenEXR/ImfStandardAttributes.h>
#include <OpenEXR/ImfCompressionAttribute.h>
#include <OpenEXR/ImfLut.h>
#include <OpenEXR/ImfArray.h>
#include <OpenEXR/ImathFun.h>
#include <OpenEXR/Iex.h>
#include <OpenEXR/half.h>
#include <OpenEXR/halfFunction.h>
#if AQSIS_SYSTEM_WIN32 && (defined(AQSIS_COMPILER_MSVC6) || defined(AQSIS_COMPILER_MSVC7))
#	pragma warning(pop)
#endif

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <algorithm>
#include <aqsis/ri/ndspy.h>

#include <cstring>
#include <cctype>

#define DspyError(a,b,c) printf(b,c)

using namespace Imath;
using namespace Imf;
using namespace std;

namespace
{

//
// Define halfFunctions for the identity and piz12
//
half                halfID( half x )
{
	return x;
}

halfFunction <half> id( halfID );
halfFunction <half> piz12( round12log );

struct SqImageLayerChannel
{
	std::string		channelName;
	Imf::Channel	channel;
	int				dataOffset;
	int				bufferOffset;
};

typedef vector <SqImageLayerChannel >   LayerChannelList;
typedef vector <halfFunction <half> *>  LayerChannelLuts;

struct SqImageLayer
{
	std::string	layerName;
	LayerChannelList	channelList;
	LayerChannelLuts	channelLuts;
};
typedef std::map<std::string, SqImageLayer> LayerList;


#include "dspyhlpr.h"

class Image
{
	public:

		Image (const char filename[],
		       const Header &header);

			  Header &      header ();
		const Header &      header () const;

			  LayerList&	layers ();
	    const LayerList&	layers () const;

		void                writePixels (int xMin, int xMaxPlusone,
		                                 int yMin, int yMaxPlusone,
		                                 int entrySize,
		                                 const unsigned char *data,
										 std::string layerName);
		void				addLayer(SqImageLayer& layer);
		void				open();
	private:

		boost::shared_ptr<OutputFile>          _file;
		std::string			_fileName;
		Header				_header;
		Array <char>        _buffer;
		std::map<int, std::vector<char> >	_scanlines;
		std::map<int, int >	_scanlinesReceived;
		int                 _bufferPixelSize;
		int                 _bufferXMin;
		int                 _bufferNumPixels;
		LayerList			_layers;
};


typedef map<string, boost::shared_ptr<Image> > ImageMap;
ImageMap gImages;

typedef pair<std::string, std::string>	ImageLayerEntry;
typedef vector<ImageLayerEntry> ImageLayerList;
ImageLayerList	gImageLayers;



Image::Image (const char filename[],
              const Header &header)
		:
		_fileName (filename),
		_header (header),
		_bufferPixelSize (0),
		_bufferXMin (header.dataWindow().min.x),
		_bufferNumPixels (header.dataWindow().max.x - _bufferXMin + 1)
{}


void Image::addLayer(SqImageLayer& layer)
{
	// Insert the channels into the header
	for (LayerChannelList::iterator chan = layer.channelList.begin(), chan_end = layer.channelList.end(); chan != chan_end; ++chan)
	{
		header().channels().insert((layer.layerName+"."+chan->channelName).c_str(), chan->channel);

		switch (chan->channel.type)
		{
				case HALF:

				chan->bufferOffset = _bufferPixelSize;
				_bufferPixelSize += sizeof (float); // Note: to avoid alignment
				break;                              // problems when float and half
				// channels are mixed, halfs
				case FLOAT:                           // are not packed densely.

				chan->bufferOffset = _bufferPixelSize;
				_bufferPixelSize += sizeof (float);
				break;

				default:

				assert (false);                     // unsupported channel type
				break;
		}
	}
	_layers[layer.layerName] = layer;
}

const Header &Image::header () const
{
	return _header;
}

Header &Image::header ()
{
	return _header;
}

LayerList& Image::layers()
{
	return _layers;
}

const LayerList& Image::layers() const
{
	return _layers;
}


void Image::open()
{
	V2i dwSize = _header.dataWindow().size();

	_buffer.resizeErase (_bufferNumPixels * _bufferPixelSize);

	FrameBuffer  fb;
	int          yStride = 0;
	char        *base = &_buffer[0] -
	                    _bufferXMin * _bufferPixelSize;


	for(LayerList::iterator layer = _layers.begin(), layerEnd = _layers.end(); layer != layerEnd; ++layer)
	{
		for(LayerChannelList::iterator chan = layer->second.channelList.begin(), chanEnd = layer->second.channelList.end(); chan != chanEnd; ++chan)
		{
			fb.insert((layer->second.layerName+"."+chan->channelName).c_str(),
				Slice(chan->channel.type,
				base + chan->bufferOffset,
				_bufferPixelSize,
				yStride,
				1,
				1));
		}
	}

	_file = boost::shared_ptr<OutputFile>(new OutputFile(_fileName.c_str(), _header));
	_file->setFrameBuffer (fb);
}

void
Image::writePixels (int xMin, int xMaxPlusone,
                    int yMin, int yMaxPlusone,
                    int entrySize,
                    const unsigned char *data,
					std::string layerName)
{
	// If the image isn't open yet, open it now, the
	// channel setup must be complete by the time the
	// first data is written.
	if(!_file)
		open();

	//
	// We can only deal with one scan line at a time.
	//

	assert (yMin == yMaxPlusone - 1);


	int      numPixels = xMaxPlusone - xMin;
	int      j = 0;

	char    *toBase;
	int      toInc;

	//
	// Copy the pixels into our internal scanline array, collating multiple layers
	// before copying to the fb and then to the file.
	//

	// If there is no scanline for this y position, allocate one now.
	if(_scanlines.find(yMin) == _scanlines.end())
	{
		_scanlines[yMin].resize (_bufferNumPixels * _bufferPixelSize);
		_scanlinesReceived[yMin] = 0;
	}

	toBase = &(_scanlines[yMin][0]) + _bufferPixelSize * xMin;
	toInc = _bufferPixelSize;

	for(LayerChannelList::iterator i = layers()[layerName].channelList.begin(), e = layers()[layerName].channelList.end(); i != e; ++i)
	{
		const unsigned char *from = data + i->dataOffset;
		const unsigned char *end  = from + numPixels * entrySize;

		char *to = toBase + i->bufferOffset;

		switch (i->channel.type)
		{
				case HALF:
				{
					halfFunction <half> &lut = *layers()[layerName].channelLuts[j];

					while (from < end)
					{
						*(half *) to = lut( ( half )( *(float *) from ) );
						from += entrySize;
						to += toInc;
					}

					break;
				}

				case FLOAT:

				while (from < end)
				{
					*(float *) to = *(float *) from;
					from += entrySize;
					to += toInc;
				}

				break;

				default:

				assert (false);  // channel type is not currently supported
				break;
		}

		++j;
	}


	_scanlinesReceived[yMin] += numPixels;

	if(_scanlinesReceived[yMin] == _bufferNumPixels*static_cast<int>(layers().size()))
	{
		//
		// If our one-line frame buffer is full, then write it to
		// the output file.
		//
		for(int i = 0; i < (_bufferNumPixels * _bufferPixelSize); ++i)
			_buffer[i] = _scanlines[yMin][i];

		_file->writePixels();

		_scanlines.erase(yMin);
		_scanlinesReceived.erase(yMin);
	}
}


} // namespace

extern "C"
{


	PtDspyError
	DspyImageOpen (PtDspyImageHandle *pvImage,
	               const char *drivername,
	               const char *filename,
	               int width,
	               int height,
	               int paramCount,
	               const UserParameter *parameters,
	               int formatCount,
	               PtDspyDevFormat *format,
	               PtFlagStuff *flagstuff)
	{
		try
		{
			//
			// Build an output file header
			//

			Header               header;

			// Add a new layer specification to the image reference.
			SqImageLayer layer;
			int                  pixelSize = 0;

			halfFunction <half> *rgbLUT = &id;
			halfFunction <half> *otherLUT = &id;

			//
			// Open the output file
			//

			ImageMap::iterator image = gImages.end();
			if(gImages.find(filename) != gImages.end())
			{
				image = gImages.find(filename);
				flagstuff->flags |= PkDspyFlagsWantsScanLineOrder;
			}
			else
			{
				//
				// Data window
				//

				{
					Box2i &dw = header.dataWindow();
					int n = 2;

					DspyFindIntsInParamList ("origin", &n, &dw.min.x,
											 paramCount, parameters);
					assert (n == 2);

					dw.max.x = dw.min.x + width  - 1;
					dw.max.y = dw.min.y + height - 1;
				}

				//
				// Display window
				//

				{
					Box2i &dw = header.displayWindow();
					int n = 2;

					DspyFindIntsInParamList ("OriginalSize", &n, &dw.max.x,
											 paramCount, parameters);
					assert (n == 2);

					dw.min.x  = 0;
					dw.min.y  = 0;
					dw.max.x -= 1;
					dw.max.y -= 1;
				}

				//
				//Chromaticities conversion from YB/YA->RGB
				//

				addChromaticities(header,Chromaticities());

				//
				// Camera parameters
				//

				{
					//
					// World-to-NDC matrix, world-to-camera matrix,
					// near and far clipping plane distances
					//

					M44f NP, Nl;
					float near = 0, far = 0;

					DspyFindMatrixInParamList ("NP", &NP[0][0], paramCount, parameters);
					DspyFindMatrixInParamList ("Nl", &Nl[0][0], paramCount, parameters);
					DspyFindFloatInParamList ("near", &near, paramCount, parameters);
					DspyFindFloatInParamList ("far", &far, paramCount, parameters);

					//
					// The matrices reflect the orientation of the camera at
					// render time.
					//

					header.insert ("worldToNDC", M44fAttribute (NP));
					header.insert ("worldToCamera", M44fAttribute (Nl));
					header.insert ("clipNear", FloatAttribute (near));
					header.insert ("clipFar", FloatAttribute (far));

					//
					// Projection matrix
					//

					M44f P = Nl.inverse() * NP;

					//
					// Derive pixel aspect ratio, screen window width, screen
					// window center from projection matrix.
					//

					Box2f sw (V2f ((-1 - P[3][0] - P[2][0]) / P[0][0],
								   (-1 - P[3][1] - P[2][1]) / P[1][1]),
							  V2f (( 1 - P[3][0] - P[2][0]) / P[0][0],
								   ( 1 - P[3][1] - P[2][1]) / P[1][1]));

					header.screenWindowWidth() = sw.max.x - sw.min.x;
					header.screenWindowCenter() = (sw.max + sw.min) / 2;

					const Box2i &dw = header.displayWindow();

					header.pixelAspectRatio()   = (sw.max.x - sw.min.x) /
												  (sw.max.y - sw.min.y) *
												  (dw.max.y - dw.min.y + 1) /
												  (dw.max.x - dw.min.x + 1);
				}

				//
				// Line order
				//

				header.lineOrder() = INCREASING_Y;
				flagstuff->flags |= PkDspyFlagsWantsScanLineOrder;

				//
				// Compression
				//

				{
					char *comp = 0;

					DspyFindStringInParamList ("exrcompression", &comp,
											   paramCount, parameters);

					if (comp)
					{
						if (!strcmp (comp, "none"))
							header.compression() = NO_COMPRESSION;
						else if (!strcmp (comp, "rle"))
							header.compression() = RLE_COMPRESSION;
						else if (!strcmp (comp, "zips"))
							header.compression() = ZIPS_COMPRESSION;
						else if (!strcmp (comp, "zip"))
							header.compression() = ZIP_COMPRESSION;
						else if (!strcmp (comp, "piz"))
							header.compression() = PIZ_COMPRESSION;

						else if (!strcmp (comp, "piz12"))
						{
							header.compression() = PIZ_COMPRESSION;
							rgbLUT = &piz12;
						}

						else
							THROW (Iex::ArgExc,
								   "Invalid exrcompression \"" << comp << "\" "
								   "for image file " << filename << ".");
					}

					else
						header.compression() = ZIP_COMPRESSION;
				}

				/** If any custom attribute parameters have been specified, add them
				 *  to the header now.
				 */
				const UserParameter* p = parameters;
				for(int i = 0; i < paramCount; i++, p++)
				{
					std::string paramName(p->name);
					static std::string image_metadata_id("image_metadata:");
					std::string::size_type loc = paramName.find(image_metadata_id);
					if(loc == 0)
					{
						std::string attrName = paramName.substr(image_metadata_id.size());
						if(p->vtype == 's')
						{
							header.insert(attrName.c_str(), StringAttribute(*(char **)p->value));
						}
					}
				}

				Image *newImage = new Image (filename, header);
				gImages[filename] = boost::shared_ptr<Image>(newImage);
			}

			char* layerNameParameter = 0;
			DspyFindStringInParamList ("layername", &layerNameParameter, paramCount, parameters);
			if(layerNameParameter)
			{
				layer.layerName = layerNameParameter;
			}
			else
			{
				std::stringstream layerName;
				layerName << "layer_" << gImages[filename]->layers().size();
				layer.layerName = layerName.str();
			}

			//
			// Channel list
			//

			{
				PixelType pixelType = HALF;
				char *ptype = 0;

				DspyFindStringInParamList ("exrpixeltype", &ptype,
										   paramCount, parameters);

				if (ptype)
				{
					if (!strcmp (ptype, "float"))
						pixelType = FLOAT;
					else if (!strcmp (ptype, "half"))
						pixelType = HALF;
					else
						THROW (Iex::ArgExc,
							   "Invalid exrpixeltype \"" << ptype << "\" "
							   "for image file " << filename << ".");
				}

				// Check for a parameter called "channelnames"
				char** channelNamesParameter = 0;
				DspyFindStringsInParamList("channelnames", &channelNamesParameter, paramCount, parameters);

				for (int i = 0; i < formatCount; ++i)
				{
					std::string chanName("");
					if(channelNamesParameter)
						chanName = channelNamesParameter[i];

					std::string formatName(format[i].name);
					if(formatName.size() == 1 &&
						formatName.find_first_of("rgbaz") != std::string::npos)
					{
						SqImageLayerChannel chan;
						chan.dataOffset = pixelSize;
						chan.channel = Channel(pixelType);
						chan.bufferOffset = 0; // This is filled in by the image when the layer is added.
						if(!chanName.empty())
							chan.channelName = chanName;
						else
						{
							/// \note: The horrid cast on toupper is to keep gcc3.0 happy
							///  http://lists.debian.org/debian-gcc/2002/04/msg00092.html
							std::transform(formatName.begin(), formatName.end(), formatName.begin(), (int(*)(int))std::toupper);
							chan.channelName = formatName;
						}
						layer.channelList.push_back(chan);
						layer.channelLuts.push_back( rgbLUT );
					}
					else
					{
						//
						// Unknown channel name; keep its name and store
						// the channel, unless the name conflicts with
						// another channel's name.
						//

						//if (layer.channelList.find(format[i].name) != layer.channelOffsets.end())
						{
							SqImageLayerChannel chan;
							chan.dataOffset = pixelSize;
							chan.channel = Channel(pixelType);
							chan.bufferOffset = 0; // This is filled in by the image when the layer is added.
							if(!chanName.empty())
								chan.channelName = chanName;
							else
								chan.channelName = format[i].name;
							layer.channelList.push_back(chan);
							layer.channelLuts.push_back( otherLUT );
						}
					}

					format[i].type = PkDspyFloat32 | PkDspyByteOrderNative;
					pixelSize += sizeof (float);
				}
			}

			// Add our layer to the image.
			gImages[filename]->addLayer(layer);
			// Setup a new image layer entry for this layer, and pass the index back.
			gImageLayers.push_back(std::make_pair(filename, layer.layerName));
			*pvImage = (PtDspyImageHandle) (gImageLayers.size()-1);

		}
		catch (const exception &e)
		{
			DspyError ("OpenEXR display driver", "%s\n", e.what());
			return PkDspyErrorUndefined;
		}

		return PkDspyErrorNone;
	}


	PtDspyError
	DspyImageData (PtDspyImageHandle pvImage,
	               int xmin,
	               int xmax_plusone,
	               int ymin,
	               int ymax_plusone,
	               int entrysize,
	               const unsigned char *data)
	{
		try
		{
			size_t imageLayerIndex = reinterpret_cast<size_t>(pvImage);
			std::string imageName = gImageLayers[imageLayerIndex].first;
			if(gImages.find(imageName) != gImages.end())
			{
				boost::shared_ptr<Image> image = gImages[imageName];
				image->writePixels (xmin, xmax_plusone,
							ymin, ymax_plusone,
							entrysize, data, gImageLayers[imageLayerIndex].second);
			}
		}
		catch (const exception &e)
		{
			DspyError ("OpenEXR display driver", "%s\n", e.what());
			return PkDspyErrorUndefined;
		}

		return PkDspyErrorNone;
	}


	PtDspyError
	DspyImageClose (PtDspyImageHandle pvImage)
	{
		try
		{
			size_t imageLayerIndex = reinterpret_cast<size_t>(pvImage);
			std::string imageName = gImageLayers[imageLayerIndex].first;
			if(gImages.find(imageName) != gImages.end())
			{
				boost::shared_ptr<Image> image = gImages[imageName];
				image->layers().erase(gImageLayers[imageLayerIndex].second);
				if(image->layers().size() == 0)
					gImages.erase(imageName);
			}
		}
		catch (const exception &e)
		{
			DspyError ("OpenEXR display driver", "%s\n", e.what());
			return PkDspyErrorUndefined;
		}

		return PkDspyErrorNone;
	}


	PtDspyError
	DspyImageQuery (PtDspyImageHandle pvImage,
	                PtDspyQueryType querytype,
	                size_t datalen,
	                void *data)
	{
		unsigned int _datalen = static_cast<unsigned int>(datalen);
		if (_datalen > 0 && data)
		{
			switch (querytype)
			{
					case PkOverwriteQuery:
					{
						PtDspyOverwriteInfo overwriteInfo;

						if (_datalen > sizeof(overwriteInfo))
							_datalen = sizeof(overwriteInfo);

						overwriteInfo.overwrite = 1;
						overwriteInfo.interactive = 0;

						memcpy(data, &overwriteInfo, _datalen);
					}
					break;

					case PkSizeQuery:
					{
						PtDspySizeInfo sizeInfo;

						if (_datalen > sizeof(sizeInfo))
							_datalen = sizeof(sizeInfo);

						size_t imageLayerIndex = reinterpret_cast<size_t>(pvImage);
						std::string imageName = gImageLayers[imageLayerIndex].first;
						if(gImages.find(imageName) != gImages.end())
						{
							boost::shared_ptr<Image> image = gImages[imageName];

							const Box2i &dw = image->header().dataWindow();

							sizeInfo.width  = dw.max.x - dw.min.x + 1;
							sizeInfo.height = dw.max.y - dw.min.y + 1;

							//
							// Renderman documentation does not specify if
							// sizeInfo.aspectRatio refers to the pixel or
							// the image aspect ratio, but sample code in
							// the documentation suggests pixel aspect ratio.
							//

							sizeInfo.aspectRatio = image->header().pixelAspectRatio();
						}
						else
						{
							sizeInfo.width  = 640;
							sizeInfo.height = 480;
							sizeInfo.aspectRatio = 1.0f;
						}

						memcpy(data, &sizeInfo, _datalen);
					}
					break;

					default:

					return PkDspyErrorUnsupported;
			}
		}
		else
		{
			return PkDspyErrorBadParams;
		}

		return PkDspyErrorNone;
	}


} // extern C
