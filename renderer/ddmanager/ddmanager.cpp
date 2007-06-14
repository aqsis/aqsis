// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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
		\brief Compliant display device manager.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/


#include	"aqsis.h"

#ifdef	AQSIS_SYSTEM_WIN32
#include	"winsock2.h"
#endif

#include	"sstring.h"
#include	"ddmanager.h"
#include	"rifile.h"
#include	"imagebuffer.h"
#include	"shaderexecenv.h"
#include	"logging.h"
#include	"ndspy.h"
#include	"version.h"
#include	"debugdd.h"

START_NAMESPACE( Aqsis )


/// Required function that implements Class Factory design pattern for DDManager libraries
IqDDManager* CreateDisplayDriverManager()
{
	return new CqDDManager;
}

CqString CqDDManager::m_strOpenMethod("DspyImageOpen");
CqString CqDDManager::m_strQueryMethod("DspyImageQuery");
CqString CqDDManager::m_strDataMethod("DspyImageData");
CqString CqDDManager::m_strCloseMethod("DspyImageClose");
CqString CqDDManager::m_strDelayCloseMethod("DspyImageDelayClose");

char* CqDDManager::m_RedName = "r";
char* CqDDManager::m_GreenName = "g";
char* CqDDManager::m_BlueName = "b";
char* CqDDManager::m_AlphaName = "a";
char* CqDDManager::m_ZName = "z";

TqInt CqDDManager::AddDisplay( const TqChar* name, const TqChar* type, const TqChar* mode, TqInt modeID, TqInt dataOffset, TqInt dataSize, std::map<std::string, void*> mapOfArguments )
{
	SqDisplayRequest req;

	req.m_valid = false;
	req.m_name = name;
	req.m_type = type;
	req.m_mode = mode;
	req.m_AOVOffset = dataOffset;
	req.m_AOVSize = dataSize;
	req.m_modeHash = CqString::hash( mode );
	req.m_modeID = modeID;
	req.m_QuantizeZeroVal = 0.0f;
	req.m_QuantizeOneVal = 255.0f;
	req.m_QuantizeMinVal = 0.0f;
	req.m_QuantizeMaxVal = 0.0f;
	req.m_QuantizeDitherVal = 0.0f;
	req.m_QuantizeSpecified = false;
	req.m_QuantizeDitherSpecified = false;

	// Create the array of UserParameter structures for all the unrecognised extra parameters,
	// while extracting information for the recognised ones.
	PrepareCustomParameters(mapOfArguments, req);

	m_displayRequests.push_back(req);

	return ( 0 );
}

TqInt CqDDManager::ClearDisplays()
{
	// Free any user parameter data specified on the display requests.
	std::vector<SqDisplayRequest>::iterator i;
	for(i = m_displayRequests.begin(); i!=m_displayRequests.end(); i++)
	{
		std::vector<UserParameter>::iterator iup;
		for(iup = i->m_customParams.begin(); iup != i->m_customParams.end(); iup++ )
		{
			if( iup->nbytes )
			{
				free(iup->name);
				free(iup->value);
			}
		}
	}

	m_displayRequests.clear();
	return ( 0 );
}

TqInt CqDDManager::OpenDisplays()
{
	// Now go over any requested displays launching the clients.
	std::vector<SqDisplayRequest>::iterator i;
	for(i = m_displayRequests.begin(); i!=m_displayRequests.end(); i++)
		LoadDisplayLibrary(*i);
	return ( 0 );
}

TqInt CqDDManager::CloseDisplays()
{
	// Now go over any requested displays launching the clients.
	std::vector<SqDisplayRequest>::iterator i;
	for(i = m_displayRequests.begin(); i!=m_displayRequests.end(); i++)
		CloseDisplayLibrary(*i);
	return ( 0 );
}



TqInt CqDDManager::DisplayBucket( IqBucket* pBucket )
{
	static CqRandom random( 61 );

	if( (pBucket->Width() == 0) || (pBucket->Height() == 0) )
		return(0);
	TqUint	xmin = pBucket->XOrigin();
	TqUint	ymin = pBucket->YOrigin();
	TqUint	xmaxplus1 = xmin + pBucket->Width();
	TqUint	ymaxplus1 = ymin + pBucket->Height();

	// If completely outside the crop rectangle, don't bother sending.
	if( xmaxplus1 <= (TqUint) QGetRenderContext()->pImage()->CropWindowXMin() ||
	        ymaxplus1 <= (TqUint) QGetRenderContext()->pImage()->CropWindowYMin() ||
	        xmin > (TqUint) QGetRenderContext()->pImage()->CropWindowXMax() ||
	        ymin > (TqUint) QGetRenderContext()->pImage()->CropWindowYMax() )
		return(0);

	std::vector<SqDisplayRequest>::iterator i;
	for ( i = m_displayRequests.begin(); i != m_displayRequests.end(); i++ )
	{
		// If the display is not validated, don't send it data.
		if( !i->m_valid )
			continue;

		// Allocate enough space to put the whole bucket data into.
		if (i->m_DataBucket == 0)
			i->m_DataBucket = reinterpret_cast<unsigned char*>(malloc(i->m_elementSize * pBucket->Width() * pBucket->Height()));
		if ((i->m_flags.flags & PkDspyFlagsWantsScanLineOrder) && i->m_DataRow == 0)
		{
			TqUint width = QGetRenderContext()->pImage()->CropWindowXMax() - QGetRenderContext()->pImage()->CropWindowXMin();
			TqUint height = pBucket->Height();
			i->m_DataRow = reinterpret_cast<unsigned char*>(malloc(i->m_elementSize * width * height));
		}

		SqImageSample val;
		// Fill in the bucket data for each channel in each element, honoring the requested order and formats.
		unsigned char* pdata = i->m_DataBucket;
		TqUint y;

		for ( y = ymin; y < ymaxplus1; y++ )
		{
			TqUint x;
			for ( x = xmin; x < xmaxplus1; x++ )
			{
				TqInt index = 0;
				const TqFloat* pSamples = pBucket->Data( x, y );
				std::vector<PtDspyDevFormat>::iterator iformat;
				double s = random.RandomFloat();
				for(iformat = i->m_formats.begin(); iformat != i->m_formats.end(); iformat++)
				{
					TqFloat value = pSamples[i->m_dataOffsets[index]];
					// If special quantization instructions have been given for this display, do it now.
					if( !( i->m_QuantizeZeroVal == 0.0f &&
					        i->m_QuantizeOneVal  == 0.0f &&
					        i->m_QuantizeMinVal  == 0.0f &&
					        i->m_QuantizeMaxVal  == 0.0f ) )
					{
						value = ROUND(i->m_QuantizeZeroVal + value * (i->m_QuantizeOneVal - i->m_QuantizeZeroVal) + ( i->m_QuantizeDitherVal * s ) );
						value = CLAMP(value, i->m_QuantizeMinVal, i->m_QuantizeMaxVal) ;
					}
					TqInt type = iformat->type & PkDspyMaskType;
					switch(type)
					{
							case PkDspyFloat32:
							reinterpret_cast<float*>(pdata)[0] = value;
							pdata += sizeof(float);
							break;
							case PkDspyUnsigned32:
							reinterpret_cast<unsigned long*>(pdata)[0] = static_cast<unsigned long>( value );
							pdata += sizeof(unsigned long);
							break;
							case PkDspySigned32:
							reinterpret_cast<long*>(pdata)[0] = static_cast<long>( value );
							pdata += sizeof(long);
							break;
							case PkDspyUnsigned16:
							reinterpret_cast<unsigned short*>(pdata)[0] = static_cast<unsigned short>( value );
							pdata += sizeof(unsigned short);
							break;
							case PkDspySigned16:
							reinterpret_cast<short*>(pdata)[0] = static_cast<short>( value );
							pdata += sizeof(short);
							break;
							case PkDspyUnsigned8:
							reinterpret_cast<unsigned char*>(pdata)[0] = static_cast<unsigned char>( value );
							pdata += sizeof(unsigned char);
							break;
							case PkDspySigned8:
							reinterpret_cast<char*>(pdata)[0] = static_cast<char>( value );
							pdata += sizeof(char);
							break;
					}
					index++;
				}
			}
		}

		// Now that the bucket data has been constructed, send it to the display
		// either lines by lines or bucket by bucket
		if(i->m_DataMethod )
		{
			TqUint elementsize = i->m_elementSize;
			PtDspyError err;

			if (i->m_flags.flags & PkDspyFlagsWantsScanLineOrder)
			{
				TqUint x, y;

				// Accumulate the bucket information to the full row of buckets
				unsigned char* pdata = i->m_DataBucket;
				TqUint width = QGetRenderContext()->pImage()->CropWindowXMax() - QGetRenderContext()->pImage()->CropWindowXMin();
				for (y = ymin; y < ymaxplus1; y++)
				{
					for (x = xmin; x < xmaxplus1; x++)
					{
						memcpy(&(i->m_DataRow[width * elementsize * (y - ymin) + elementsize * x]), pdata, elementsize);
						pdata += elementsize;
					}
				}
				pdata = i->m_DataRow;

				// Is it the time sent complete rows
				if (xmaxplus1 >= width)
				{
					// send to the display one line at the time
					for (y = ymin; y < ymaxplus1; y++)
					{
						err = (i->m_DataMethod)(i->m_imageHandle, 0, width, y, y+1, elementsize, pdata);
						pdata += elementsize * width;
					}
				}

			}
			else
			{
				// Send the bucket information as they come in
				err = (i->m_DataMethod)(i->m_imageHandle, xmin, xmaxplus1, ymin, ymaxplus1, elementsize, i->m_DataBucket);

			}

		}


	}

	return ( 0 );
}

bool CqDDManager::fDisplayNeeds( const TqChar* var )
{
	static TqUlong rgb = CqString::hash( "rgb" );
	static TqUlong rgba = CqString::hash( "rgba" );
	static TqUlong Ci = CqString::hash( "Ci" );
	static TqUlong Oi = CqString::hash( "Oi" );
	static TqUlong Cs = CqString::hash( "Cs" );
	static TqUlong Os = CqString::hash( "Os" );

	TqUlong htoken = CqString::hash( var );



	// Scan all registered displays to see if any of them need the variable specified.
	std::vector<SqDisplayRequest>::iterator i;
	for(i = m_displayRequests.begin(); i!=m_displayRequests.end(); i++)
	{
		bool usage = ( ( i->m_modeHash == rgba ) || ( i->m_modeHash == rgb ) );

		if ( (( htoken == Ci )|| ( htoken == Cs))&& usage )
			return ( true );
		else if ( (( htoken == Oi )|| ( htoken == Os))&& usage )
			return ( true );
		else if ( i->m_modeHash == htoken  )
			return ( true );
	}
	//printf("fDisplayNeeds(%s) returns false ???\n", var);
	return ( false);
}

TqInt CqDDManager::Uses()
{
	TqInt Uses = 0;
	// Scan all registered displays to combine the required variables.
	std::vector<SqDisplayRequest>::iterator i;
	for(i = m_displayRequests.begin(); i!=m_displayRequests.end(); i++)
	{
		TqInt ivar;
		for( ivar = 0; ivar < EnvVars_Last; ivar++ )
		{
			if( i->m_modeHash == gVariableTokens[ ivar ] )
				Uses |= 1 << ivar;
		}
	}
	return ( Uses );
}


void CqDDManager::LoadDisplayLibrary( SqDisplayRequest& req )
{
	// Get the display mapping from the "display" options, if one exists.
	CqString strDriverFile = "";
	CqString displayType = req.m_type;
	const CqString* poptDisplay = QGetRenderContext()->poptCurrent()->GetStringOption("display", displayType.c_str());
	if(0 != poptDisplay)
		strDriverFile = poptDisplay[0];
	else
	{
		const CqString* poptDisplayMapping = QGetRenderContext()->poptCurrent()->GetStringOption("display", "mapping");
		if(0 != poptDisplayMapping)
		{
			CqString strMapping = poptDisplayMapping[0];
			strDriverFile.Format(strMapping.c_str(), displayType.c_str());
		}
	}
	Aqsis::log() << debug << "Attempting to load \"" << strDriverFile.c_str() << "\" for display type \""<< displayType.c_str() << "\"" << std::endl;
	// Display type not found.
	if ( strDriverFile.empty() )
		throw( CqString( "Invalid display type \"" ) + CqString( req.m_type ) + CqString( "\"" ) + CqString(" (") + strDriverFile + CqString(")") );
	if( strDriverFile != "debugdd")
	{
		// Try to open the file to see if it's really there
		CqRiFile fileDriver( strDriverFile.c_str(), "display" );
		if ( !fileDriver.IsValid() )
			throw( CqString( "Error loading display driver [ " ) + strDriverFile + CqString( " ]" ) );
		CqString strDriverPathAndFile = fileDriver.strRealName();
		// Load the dynamic obejct and locate the relevant symbols.
		req.m_DriverHandle = m_DspyPlugin.SimpleDLOpen( &strDriverPathAndFile );
		if( req.m_DriverHandle != NULL )
		{
			req.m_OpenMethod = (DspyImageOpenMethod)m_DspyPlugin.SimpleDLSym( req.m_DriverHandle, &m_strOpenMethod );
			if (!req.m_OpenMethod)
			{
				m_strOpenMethod = "_" + m_strOpenMethod;
				req.m_OpenMethod = (DspyImageOpenMethod)m_DspyPlugin.SimpleDLSym( req.m_DriverHandle, &m_strOpenMethod );
			}
			req.m_QueryMethod = (DspyImageQueryMethod)m_DspyPlugin.SimpleDLSym( req.m_DriverHandle, &m_strQueryMethod );
			if (!req.m_QueryMethod)
			{
				m_strQueryMethod = "_" + m_strQueryMethod;
				req.m_QueryMethod = (DspyImageQueryMethod)m_DspyPlugin.SimpleDLSym( req.m_DriverHandle, &m_strQueryMethod );
			}
			req.m_DataMethod = (DspyImageDataMethod)m_DspyPlugin.SimpleDLSym( req.m_DriverHandle, &m_strDataMethod );
			if (!req.m_DataMethod)
			{
				m_strDataMethod = "_" + m_strDataMethod;
				req.m_DataMethod = (DspyImageDataMethod)m_DspyPlugin.SimpleDLSym( req.m_DriverHandle, &m_strDataMethod );
			}
			req.m_CloseMethod = (DspyImageCloseMethod)m_DspyPlugin.SimpleDLSym( req.m_DriverHandle, &m_strCloseMethod );
			if (!req.m_OpenMethod)
			{
				m_strCloseMethod = "_" + m_strCloseMethod;
				req.m_CloseMethod = (DspyImageCloseMethod)m_DspyPlugin.SimpleDLSym( req.m_DriverHandle, &m_strCloseMethod );
			}
			req.m_DelayCloseMethod = (DspyImageDelayCloseMethod)m_DspyPlugin.SimpleDLSym( req.m_DriverHandle, &m_strDelayCloseMethod );
			if (!req.m_DelayCloseMethod)
			{
				m_strDelayCloseMethod = "_" + m_strDelayCloseMethod;
				req.m_DelayCloseMethod = (DspyImageDelayCloseMethod)m_DspyPlugin.SimpleDLSym( req.m_DriverHandle, &m_strDelayCloseMethod );
			}
		}
	}
	else
	{
		// We are using the in-library internal debugging DD.
		req.m_OpenMethod =  ::DebugDspyImageOpen ;
		req.m_QueryMethod =  ::DebugDspyImageQuery ;
		req.m_DataMethod = ::DebugDspyImageData ;
		req.m_CloseMethod = ::DebugDspyImageClose ;
		req.m_DelayCloseMethod = ::DebugDspyDelayImageClose ;
	}

	// Nullified the data part
	req.m_DataRow = 0;
	req.m_DataBucket = 0;

	if( NULL != req.m_OpenMethod )
	{
		// If the quantization options haven't been set in the RiDisplay call, get the appropriate values out
		// of the RiQuantize option.
		const TqFloat* pQuant = 0;
		if(!req.m_QuantizeSpecified || !req.m_QuantizeDitherSpecified)
		{
			if(req.m_modeID & ModeZ)
				pQuant = QGetRenderContext() ->poptCurrent()->GetFloatOption( "Quantize", "Depth" );
			else
				pQuant = QGetRenderContext() ->poptCurrent()->GetFloatOption( "Quantize", "Color" );
			if( pQuant && !req.m_QuantizeSpecified)
			{
				req.m_QuantizeOneVal = pQuant[0];
				req.m_QuantizeMinVal = pQuant[1];
				req.m_QuantizeMaxVal = pQuant[2];
				req.m_QuantizeSpecified = true;
			}

			if( pQuant && !req.m_QuantizeDitherSpecified)
			{
				req.m_QuantizeDitherVal = pQuant[3];
				req.m_QuantizeDitherSpecified = true;
			}
		}
		// Prepare the information and call the DspyImageOpen function in the display device.
		if(req.m_modeID & ( ModeRGB | ModeA | ModeZ) )
		{
			PtDspyDevFormat fmt;
			if( req.m_QuantizeOneVal == 255 )
				fmt.type = PkDspyUnsigned8;
			else if( req.m_QuantizeOneVal == 65535 )
				fmt.type = PkDspyUnsigned16;
			else if( req.m_QuantizeOneVal == 4294967295u )
				fmt.type = PkDspyUnsigned32;
			else
				fmt.type = PkDspyFloat32;
			if(req.m_modeID & ModeA)
			{
				fmt.name = m_AlphaName;
				req.m_formats.push_back(fmt);
			}
			if(req.m_modeID & ModeRGB)
			{
				fmt.name = m_RedName;
				req.m_formats.push_back(fmt);
				fmt.name = m_GreenName;
				req.m_formats.push_back(fmt);
				fmt.name = m_BlueName;
				req.m_formats.push_back(fmt);
			}
			if(req.m_modeID & ModeZ)
			{
				fmt.name = m_ZName;
				fmt.type = PkDspyFloat32;
				req.m_formats.push_back(fmt);
			}
		}
		// Otherwise we are dealing with AOV and should therefore fill in the formats according to it's type.
		else
		{
			// Determine the type of the AOV data being displayed.
			TqInt type;
			type = QGetRenderContext()->OutputDataType(req.m_mode.c_str());
			std::string componentNames = "";
			switch(type)
			{
					case type_point:
					case type_normal:
					case type_vector:
					case type_hpoint:
					componentNames = "XYZ";
					break;
					case type_color:
					componentNames = "rgb";
					break;
			}
			// Now create the channels formats.
			PtDspyDevFormat fmt;
			TqUint i;
			for( i = 0; i < (TqUint) req.m_AOVSize; i++ )
			{
				if(componentNames.size()>i)
				{

					if (componentNames.substr(i, 1) == "r")
						fmt.name = m_RedName;
					else if (componentNames.substr(i, 1) == "g")
						fmt.name = m_GreenName;
					else if (componentNames.substr(i, 1) == "b")
						fmt.name = m_BlueName;
					else if (componentNames.substr(i, 1) == "a")
						fmt.name = m_AlphaName;
					else if (componentNames.substr(i, 1) == "z")
						fmt.name = m_ZName;
					else if (componentNames.substr(i, 1) == "X")
						fmt.name = m_RedName;
					else if (componentNames.substr(i, 1) == "Y")
						fmt.name = m_GreenName;
					else if (componentNames.substr(i, 1) == "Z")
						fmt.name = m_BlueName;
					else
						fmt.name = m_RedName;
				}
				else
				{
					// by default we will stored into red channel eg. "s" will be saved into 'r' channel
					fmt.name = m_RedName;
				}
				if( req.m_QuantizeOneVal == 255 )
					fmt.type = PkDspyUnsigned8;
				else if( req.m_QuantizeOneVal == 65535 )
					fmt.type = PkDspyUnsigned16;
				else if( req.m_QuantizeOneVal == 4294967295u )
					fmt.type = PkDspyUnsigned32;
				else
					fmt.type = PkDspyFloat32;
				req.m_AOVnames.push_back(fmt.name);
				req.m_formats.push_back(fmt);
			}
		}

		// If we got here, we are dealing with a valid display device, so now is the time
		// to fill in the system parameters.
		PrepareSystemParameters(req);

		// Call the DspyImageOpen method on the display to initialise things.
		TqInt xres = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "System", "Resolution" ) [ 0 ];
		TqInt yres = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "System", "Resolution" ) [ 1 ];
		TqInt xmin = static_cast<TqInt>( CLAMP( CEIL( xres * QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "CropWindow" ) [ 0 ] ), 0, xres ) );
		TqInt xmax = static_cast<TqInt>( CLAMP( CEIL( xres * QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "CropWindow" ) [ 1 ] ), 0, xres ) );
		TqInt ymin = static_cast<TqInt>( CLAMP( CEIL( yres * QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "CropWindow" ) [ 2 ] ), 0, yres ) );
		TqInt ymax = static_cast<TqInt>( CLAMP( CEIL( yres * QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "CropWindow" ) [ 3 ] ), 0, yres ) );
		PtDspyError err = (*req.m_OpenMethod)(&req.m_imageHandle,
		                                      req.m_type.c_str(), req.m_name.c_str(),
		                                      xmax-xmin,
		                                      ymax-ymin,
		                                      req.m_customParams.size(),
		                                      &req.m_customParams[0],
		                                      req.m_formats.size(), &req.m_formats[0],
		                                      &req.m_flags);

		// Check for an error
		if( err != PkDspyErrorNone )
		{
			// The display did not successfully open, so clean up after it and leave the
			// request as invalid.
			Aqsis::log() << error << "Cannot open display \"" << req.m_name << "\" : ";
			switch(err)
			{
					case PkDspyErrorNoMemory:
					Aqsis::log() << error << "Out of memory" << std::endl;
					break;
					case PkDspyErrorUnsupported:
					Aqsis::log() << error << "Unsupported" << std::endl;
					break;
					case PkDspyErrorBadParams:
					Aqsis::log() << error << "Bad params" << std::endl;
					break;
					case PkDspyErrorNoResource:
					Aqsis::log() << error << "No resource" << std::endl;
					break;
					case PkDspyErrorUndefined:
					default:
					Aqsis::log() << error << "Undefined" << std::endl;
					break;
			}

			CloseDisplayLibrary(req);
			return;
		}
		else
			req.m_valid = true;

		// Now scan the returned format list to make sure that we pass the data in the order the display wants it.
		std::vector<PtDspyDevFormat>::iterator i;
		for(i=req.m_formats.begin(); i!=req.m_formats.end(); i++)
		{
			if(req.m_modeID & ( ModeRGB | ModeA | ModeZ) )
			{
				if( i->name == m_RedName )
					req.m_dataOffsets.push_back(Sample_Red);
				else if( i->name == m_GreenName )
					req.m_dataOffsets.push_back(Sample_Green);
				else if( i->name == m_BlueName )
					req.m_dataOffsets.push_back(Sample_Blue);
				else if( i->name == m_AlphaName )
					req.m_dataOffsets.push_back(Sample_Alpha);
				else if( i->name == m_ZName )
					req.m_dataOffsets.push_back(Sample_Depth);
			}
			else
			{
				// Scan through the generated names to find the ones specified, and use the index
				// of the found name as an offset into the data from the dataOffset passed in originally.
				TqUint iname;
				for(iname = 0; iname < req.m_AOVnames.size(); iname++)
				{
					if(i->name == req.m_AOVnames[iname])
					{
						req.m_dataOffsets.push_back(req.m_AOVOffset + iname );
						break;
					}
				}
				// If we got here, and didn't find it, add 0 as the offset, and issue an error.
				if( iname == req.m_AOVnames.size() )
				{
					Aqsis::log() << error << "Couldn't find format entry returned from display : " << i->name << std::endl;
					req.m_dataOffsets.push_back(req.m_AOVOffset);
				}
			}
		}

		// Determine how big each pixel is by summing the format type sizes.
		req.m_elementSize = 0;
		std::vector<PtDspyDevFormat>::iterator iformat;
		for(iformat = req.m_formats.begin(); iformat != req.m_formats.end(); iformat++)
		{
			TqInt type = iformat->type & PkDspyMaskType;
			switch( type )
			{
					case PkDspyFloat32:
					req.m_elementSize+=sizeof(float);
					break;
					case PkDspyUnsigned32:
					case PkDspySigned32:
					req.m_elementSize+=sizeof(long);
					break;
					case PkDspyUnsigned16:
					case PkDspySigned16:
					req.m_elementSize+=sizeof(short);
					break;
					case PkDspyUnsigned8:
					case PkDspySigned8:
					req.m_elementSize+=sizeof(char);
					break;
			}
		}

		//Aqsis::log() << warning << "Elementsize will be " << req.m_elementSize << std::endl;
		if( NULL != req.m_QueryMethod )
		{
			PtDspySizeInfo size;
			err = (*req.m_QueryMethod)(req.m_imageHandle, PkSizeQuery, sizeof(size), &size);
			PtDspyOverwriteInfo owinfo;
			owinfo.interactive = 0;
			owinfo.overwrite = 1;
			err = (*req.m_QueryMethod)(req.m_imageHandle, PkOverwriteQuery, sizeof(owinfo), &owinfo);
		}
	}
}

void CqDDManager::CloseDisplayLibrary( SqDisplayRequest& req )
{
	// Call the DspyImageClose method on the display to shut things down.
	// If there is a delayed close method, call it in preference.
	if( req.m_DelayCloseMethod)
		(*req.m_DelayCloseMethod)(req.m_imageHandle);
	else if( NULL != req.m_CloseMethod )
		(*req.m_CloseMethod)(req.m_imageHandle);

	if (req.m_DataBucket != 0)
	{
		free(req.m_DataBucket);
		req.m_DataBucket = 0;
	}
	if (req.m_DataRow != 0)
	{
		free(req.m_DataRow);
		req.m_DataRow = 0;
	}

	// Empty out the display request data
	req.m_CloseMethod = NULL;
	req.m_DataMethod = NULL;
	req.m_DelayCloseMethod = NULL;
	req.m_DriverHandle = 0;
	req.m_imageHandle = 0;
	req.m_OpenMethod = NULL;
	req.m_QueryMethod = NULL;

	/// \note We don't close the driver shared libraries here because doing so caused
	/// some problems with Win2K and FLTK. It seems that detaching from the drive DLL
	/// causes some important data to be altered and when a new window is opened it crashes.
	/// The cleanup of the drivers is left to when the CqDDManager instance closes down, and the
	/// CqSimplePlugin class gets destroyed, which will be at the end of the render, which is fine.
}

void CqDDManager::InitialiseDisplayNameMap()
{
	CqString strConfigFile("displays.ini");
	const CqString* displays = QGetRenderContext()->poptCurrent()->GetStringOption( "searchpath", "display" );
	if( displays )
		strConfigFile = displays[ 0 ] + "/" + strConfigFile;

	Aqsis::log() << info << "Loading display configuration from file \"" << strConfigFile << "\"" << std::endl;

	CqRiFile fileINI( strConfigFile.c_str(), "display" );
	if ( fileINI.IsValid() )
	{
		// On each line, read the first string, then the second and store them in the map.
		std::string strLine;
		std::istream& strmINI = static_cast<std::istream&>( fileINI );

		while ( std::getline( strmINI, strLine ) )
		{
			std::string strName, strDriverName;
			std::string::size_type iStartN = strLine.find_first_not_of( "\t " );
			std::string::size_type iEndN = strLine.find_first_of( "\t ", iStartN );
			std::string::size_type iStartD = strLine.find_first_not_of( "\t ", iEndN );
			std::string::size_type iEndD = strLine.find_first_of( "\t ", iStartD );
			if ( iStartN != std::string::npos && iEndN != std::string::npos &&
			        iStartD != std::string::npos )
			{
				strName = strLine.substr( iStartN, iEndN );
				strDriverName = strLine.substr( iStartD, iEndD );
				m_mapDisplayNames[ strName ] = strDriverName;
			}
		}
		m_fDisplayMapInitialised = true;
	}
	else
	{
		Aqsis::log() << error << "Could not find " << strConfigFile << " configuration file." << std::endl;
	}
}

/**
  Return the substring with the given index.
 
  The string \a s is conceptually broken into substrings that are separated by blanks
  or tabs. A continuous sequence of blanks/tabs counts as one individual separator.
  The substring with number \a idx is returned (0-based). If \a idx is higher than the
  number of substrings then an empty string is returned.
 
  \param s Input string.
  \param idx Index (0-based)
  \return Sub string with given index
*/
std::string CqDDManager::GetStringField( const std::string& s, int idx )
{
	int z = 1;   /* state variable  0=skip whitespace  1=skip chars  2=search end  3=end */
	std::string::const_iterator it;
	std::string::size_type start = 0;
	std::string::size_type end = 0;

	for ( it = s.begin(); it != s.end(); it++ )
	{
		char c = *it;

		if ( idx == 0 && z < 2 )
		{
			z = 2;
		}

		switch ( z )
		{
				case 0:
				if ( c != ' ' && c != '\t' )
				{
					idx--;
					end = start + 1;
					z = 1;
				}
				if ( idx > 0 )
					start++;
				break;
				case 1:
				if ( c == ' ' || c == '\t' )
				{
					z = 0;
				}
				start++;
				break;
				case 2:
				if ( c == ' ' || c == '\t' )
				{
					z = 3;
				}
				else
				{
					end++;
				}
				break;
		}
	}

	if ( idx == 0 )
		return s.substr( start, end - start );
	else
		return std::string( "" );

}


void ConstructMatrixParameter(const char* name, const CqMatrix* mats, TqInt count, UserParameter& parameter)
{
	// Allocate and fill in the name.
	char* pname = reinterpret_cast<char*>(malloc(strlen(name)+1));
	strcpy(pname, name);
	parameter.name = pname;
	// Allocate a 16 element float array.
	TqInt totallen = 16 * count * sizeof(RtFloat);
	RtFloat* pfloats = reinterpret_cast<RtFloat*>(malloc(totallen));
	TqInt i;
	for( i=0; i<count; i++)
	{
		const TqFloat* floats = mats[i].pElements();
		TqInt m;
		for(m=0; m<16; m++)
			pfloats[(i*16)+m]=floats[m];
	}
	parameter.value = reinterpret_cast<RtPointer>(pfloats);
	parameter.vtype = 'f';
	parameter.vcount = count * 16;
	parameter.nbytes = totallen;
}


void ConstructFloatsParameter(const char* name, const TqFloat* floats, TqInt count, UserParameter& parameter)
{
	// Allocate and fill in the name.
	char* pname = reinterpret_cast<char*>(malloc(strlen(name)+1));
	strcpy(pname, name);
	parameter.name = pname;
	// Allocate a float array.
	TqInt totallen = count * sizeof(RtFloat);
	RtFloat* pfloats = reinterpret_cast<RtFloat*>(malloc(totallen));
	// Then just copy the whole lot in one go.
	memcpy(pfloats, floats, totallen);
	parameter.value = reinterpret_cast<RtPointer>(pfloats);
	parameter.vtype = 'f';
	parameter.vcount = count;
	parameter.nbytes = totallen;
}


void ConstructIntsParameter(const char* name, const TqInt* ints, TqInt count, UserParameter& parameter)
{
	// Allocate and fill in the name.
	char* pname = reinterpret_cast<char*>(malloc(strlen(name)+1));
	strcpy(pname, name);
	parameter.name = pname;
	// Allocate a float array.
	TqInt totallen = count * sizeof(RtInt);
	RtInt* pints = reinterpret_cast<RtInt*>(malloc(totallen));
	// Then just copy the whole lot in one go.
	memcpy(pints, ints, totallen);
	parameter.value = reinterpret_cast<RtPointer>(pints);
	parameter.vtype = 'i';
	parameter.vcount = count;
	parameter.nbytes = totallen;
}


void ConstructStringsParameter(const char* name, const char** strings, TqInt count, UserParameter& parameter)
{
	// Allocate and fill in the name.
	char* pname = reinterpret_cast<char*>(malloc(strlen(name)+1));
	strcpy(pname, name);
	parameter.name = pname;
	// Allocate enough space for the string pointers, and the strings, in one big block,
	// makes it easy to deallocate later.
	TqInt totallen = count * sizeof(char*);
	TqInt i;
	for( i = 0; i < count; i++ )
		totallen += (strlen(strings[i])+1) * sizeof(char);
	char** pstringptrs = reinterpret_cast<char**>(malloc(totallen));
	char* pstrings = reinterpret_cast<char*>(&pstringptrs[count]);
	for( i = 0; i < count; i++ )
	{
		// Copy each string to the end of the block.
		strcpy(pstrings, strings[i]);
		pstringptrs[i] = pstrings;
		pstrings += strlen(strings[i])+1;
	}
	parameter.value = reinterpret_cast<RtPointer>(pstringptrs);
	parameter.vtype = 's';
	parameter.vcount = count;
	parameter.nbytes = totallen;
}


void CqDDManager::PrepareCustomParameters( std::map<std::string, void*>& mapParams, SqDisplayRequest& req )
{
	// Scan the map of extra parameters
	std::map<std::string, void*>::iterator param;
	for ( param = mapParams.begin(); param != mapParams.end(); param++ )
	{
		// First check if it is one of the recognised parameters that the renderer should handle.
		if(param->first.compare("quantize")==0)
		{
			// Extract the quantization information and store it with the display request.
			const RtFloat* floats = static_cast<float*>( param->second );
			req.m_QuantizeZeroVal = floats[0];
			req.m_QuantizeOneVal = floats[1];
			req.m_QuantizeMinVal = floats[2];
			req.m_QuantizeMaxVal = floats[3];
			req.m_QuantizeSpecified = true;
		}
		else if(param->first.compare("dither")==0)
		{
			// Extract the quantization information and store it with the display request.
			const RtFloat* floats = static_cast<float*>( param->second );
			req.m_QuantizeDitherVal = floats[0];
			req.m_QuantizeDitherSpecified = true;
		}
		else
		{
			// Otherwise, construct a UserParameter structure and fill in the details.
			SqParameterDeclaration Decl;
			try
			{
				Decl = QGetRenderContext() ->FindParameterDecl( param->first.c_str() );
			}
			catch( XqException e )
			{
				Aqsis::log() << error << e.strReason().c_str() << std::endl;
				return;
			}

			// Check the parameter type is uniform, not valid for non-surface requests otherwise.
			if( Decl.m_Class != class_uniform )
			{
				assert( false );
				continue;
			}

			UserParameter parameter;
			parameter.name = 0;
			parameter.value = 0;
			parameter.vtype = 0;
			parameter.vcount = 0;
			parameter.nbytes = 0;

			// Store the name
			char* pname = reinterpret_cast<char*>(malloc(Decl.m_strName.size()+1));
			strcpy(pname, Decl.m_strName.c_str());
			parameter.name = pname;

			switch ( Decl.m_Type )
			{
					case type_string:
					{
						const char** strings = static_cast<const char**>( param->second );
						ConstructStringsParameter(Decl.m_strName.c_str(), strings, Decl.m_Count, parameter);
					}
					break;

					case type_float:
					{
						const RtFloat* floats = static_cast<RtFloat*>( param->second );
						ConstructFloatsParameter(Decl.m_strName.c_str(), floats, Decl.m_Count, parameter);
					}
					break;

					case type_integer:
					{
						const RtInt* ints = static_cast<RtInt*>( param->second );
						ConstructIntsParameter(Decl.m_strName.c_str(), ints, Decl.m_Count, parameter);
					}
					break;
				default:
					break;
			}
			req.m_customParams.push_back(parameter);
		}
	}
}



void CqDDManager::PrepareSystemParameters( SqDisplayRequest& req )
{
	// Fill in "standard" parameters that the renderer must supply
	UserParameter parameter;

	// "NP"
	CqMatrix matWorldToScreen = QGetRenderContext() ->matSpaceToSpace( "world", "screen", NULL, NULL, QGetRenderContextI()->Time() );
	ConstructMatrixParameter("NP", &matWorldToScreen, 1, parameter);
	req.m_customParams.push_back(parameter);

	// "Nl"
	CqMatrix matWorldToCamera = QGetRenderContext() ->matSpaceToSpace( "world", "camera", NULL, NULL, QGetRenderContextI()->Time() );
	ConstructMatrixParameter("Nl", &matWorldToCamera, 1, parameter);
	req.m_customParams.push_back(parameter);

	// "near"
	TqFloat nearval = static_cast<TqFloat>( QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "Clipping" ) [ 0 ] );
	ConstructFloatsParameter("near", &nearval, 1, parameter);
	req.m_customParams.push_back(parameter);

	// "far"
	TqFloat farval = static_cast<TqFloat>( QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "Clipping" ) [ 1 ] );
	ConstructFloatsParameter("far", &farval, 1, parameter);
	req.m_customParams.push_back(parameter);

	// "OriginalSize"
	TqInt OriginalSize[2];
	OriginalSize[0] = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "System", "Resolution" ) [ 0 ];
	OriginalSize[1] = QGetRenderContext() ->poptCurrent()->GetIntegerOption( "System", "Resolution" ) [ 1 ];
	ConstructIntsParameter("OriginalSize", OriginalSize, 2, parameter);
	req.m_customParams.push_back(parameter);

	// "origin"
	TqInt origin[2];
	origin[0] = static_cast<TqInt>( CLAMP( CEIL( OriginalSize[0] * QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "CropWindow" ) [ 0 ] ), 0, OriginalSize[0] ) );
	origin[1] = static_cast<TqInt>( CLAMP( CEIL( OriginalSize[1] * QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "CropWindow" ) [ 2 ] ), 0, OriginalSize[1] ) );
	ConstructIntsParameter("origin", origin, 2, parameter);
	req.m_customParams.push_back(parameter);

	// "PixelAspectRatio"
	TqFloat PixelAspectRatio = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "PixelAspectRatio" ) [ 0 ];
	ConstructFloatsParameter("PixelAspectRatio", &PixelAspectRatio, 1, parameter);
	req.m_customParams.push_back(parameter);

	// "Software"
	char SoftwareName[ 80 ];
	const char* Software = SoftwareName;
	sprintf( SoftwareName, "%s %s (%s %s)", STRNAME, VERSION_STR, __DATE__, __TIME__ );
	ConstructStringsParameter("Software", &Software, 1, parameter);
	req.m_customParams.push_back(parameter);

	// "HostComputer"
	char HostComputerName[255];
	const char* HostComputer = HostComputerName;
#ifdef AQSIS_SYSTEM_WIN32

	WSADATA wsaData;
	WSAStartup( MAKEWORD( 2, 0 ), &wsaData );
#endif // AQSIS_SYSTEM_WIN32

	gethostname( HostComputerName, 255 );
#ifdef	AQSIS_SYSTEM_WIN32

	WSACleanup();
#endif

	ConstructStringsParameter("HostComputer", &HostComputer, 1, parameter);
	req.m_customParams.push_back(parameter);
}


END_NAMESPACE( Aqsis )

