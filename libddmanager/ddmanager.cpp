// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
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
		\brief Simple example display device manager.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/


#include	"aqsis.h"

#include	"sstring.h"
#include	"ddmanager.h"
#include	"rifile.h"
#include	"imagebuffer.h"
#include	"shaderexecenv.h"
#include	"logging.h"

START_NAMESPACE( Aqsis )


std::string PrepareCustomParameters( std::map<std::string, void*>& mapParams );

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
	req.m_name = name;
	req.m_type = type;
	req.m_mode = mode;
	req.m_modeHash = CqString::hash( mode );
	req.m_modeID = modeID;
	req.m_customParamsArgs = PrepareCustomParameters(mapOfArguments);

	m_displayRequests.push_back(req);

    return ( 0 );
}

TqInt CqDDManager::ClearDisplays()
{
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
    std::vector<SqDisplayRequest>::iterator i;
    for ( i = m_displayRequests.begin(); i != m_displayRequests.end(); i++ )
    {
        TqInt	xmin = pBucket->XOrigin();
        TqInt	ymin = pBucket->YOrigin();
        TqInt	xmaxplus1 = xmin + pBucket->Width();
        TqInt	ymaxplus1 = ymin + pBucket->Height();

		char* data = reinterpret_cast<char*>(malloc(i->m_elementSize * pBucket->Width() * pBucket->Height()));

        SqImageSample val( QGetRenderContext()->GetOutputDataTotalSize() );
		char* pdata = data;
        TqInt y;
        for ( y = ymin; y < ymaxplus1; y++ )
        {
            TqInt x;
            for ( x = xmin; x < xmaxplus1; x++ )
            {
				TqInt index = 0;
				std::vector<PtDspyDevFormat>::iterator iformat;
				for(iformat = i->m_formats.begin(); iformat != i->m_formats.end(); iformat++)
				{
					const TqFloat* pSamples = pBucket->Data( x, y );
					switch(iformat->type)
					{
						case PkDspyFloat32:
							reinterpret_cast<float*>(pdata)[0] = pSamples[i->m_dataOffsets[index]];
							pdata += sizeof(float);
							break;
						case PkDspyUnsigned32:
						case PkDspySigned32:
							reinterpret_cast<long*>(pdata)[0] = pSamples[i->m_dataOffsets[index]];
							pdata += sizeof(long);
							break;
						case PkDspyUnsigned16:
						case PkDspySigned16:
							reinterpret_cast<short*>(pdata)[0] = pSamples[i->m_dataOffsets[index]];
							pdata += sizeof(short);
							break;
						case PkDspyUnsigned8:
						case PkDspySigned8:
							reinterpret_cast<char*>(pdata)[0] = pSamples[i->m_dataOffsets[index]];
							pdata += sizeof(char);
							break;
					}
					index++;
				}
            }
        }

		free(data);
    }
    return ( 0 );
}

TqBool CqDDManager::fDisplayNeeds( const TqChar* var )
{
    static TqUlong rgb = CqString::hash( "rgb" );
    static TqUlong rgba = CqString::hash( "rgba" );
    static TqUlong Ci = CqString::hash( "Cs" );
    static TqUlong Oi = CqString::hash( "Os" );

    TqUlong htoken = CqString::hash( var );

	// Scan all registered displays to see if any of them need the variable specified.
	std::vector<SqDisplayRequest>::iterator i;
	for(i = m_displayRequests.begin(); i!=m_displayRequests.end(); i++)
    {
        TqBool usage = ( ( i->m_modeHash == rgba ) || ( i->m_modeHash == rgb ) );
        if ( ( htoken == Ci ) && usage )
            return ( TqTrue );
        else if ( ( htoken == Oi ) && usage )
            return ( TqTrue );
        else if ( ( i->m_modeHash == htoken ) )
            return ( TqTrue );
    }
    return ( TqTrue );
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
    if ( !m_fDisplayMapInitialised )
        InitialiseDisplayNameMap();

    // strDriverFileAndArgs: Second part of the ddmsock.ini line (e.g. "mydriver.exe --foo")
    CqString strDriverFileAndArgs = m_mapDisplayNames[ req.m_type ];
    // strDriverFile: Only the executable without arguments (e.g. "mydriver.exe")
    CqString strDriverFile = GetStringField( strDriverFileAndArgs, 0 );

	// Display type not found.
    if ( strDriverFile.empty() )
        throw( CqString( "Invalid display type \"" ) + CqString( req.m_type ) + CqString( "\"" ) );

    // Try to open the file to see if it's really there
    CqRiFile fileDriver( strDriverFile.c_str(), "display" );

    if ( !fileDriver.IsValid() )
        throw( CqString( "Error loading display driver [ " ) + strDriverFile + CqString( " ]" ) );

    CqString strDriverPathAndFile = fileDriver.strRealName();

	// Load the dynamic obejct and locate the relevant symbols.
    req.m_DriverHandle = req.m_DspyPlugin.SimpleDLOpen( &strDriverPathAndFile );
    if( req.m_DriverHandle != NULL )
    {
        req.m_OpenMethod = (DspyImageOpenMethod)req.m_DspyPlugin.SimpleDLSym( req.m_DriverHandle, &m_strOpenMethod );
        req.m_QueryMethod = (DspyImageQueryMethod)req.m_DspyPlugin.SimpleDLSym( req.m_DriverHandle, &m_strQueryMethod );
        req.m_DataMethod = (DspyImageDataMethod)req.m_DspyPlugin.SimpleDLSym( req.m_DriverHandle, &m_strDataMethod );
        req.m_CloseMethod = (DspyImageCloseMethod)req.m_DspyPlugin.SimpleDLSym( req.m_DriverHandle, &m_strCloseMethod );
        req.m_DelayCloseMethod = (DspyImageDelayCloseMethod)req.m_DspyPlugin.SimpleDLSym( req.m_DriverHandle, &m_strDelayCloseMethod );
    }

    if( NULL != req.m_OpenMethod )
    {
		TqFloat colorQuantOne = 255.0f;
		TqFloat depthQuantOne = 0.0f;
		// Get some key information about the render to be used when initialising the display.
	    const TqFloat* pQuant = QGetRenderContext() ->optCurrent().GetFloatOption( "Quantize", "Color" );
		if( pQuant )	colorQuantOne = pQuant[0];
	    pQuant = QGetRenderContext() ->optCurrent().GetFloatOption( "Quantize", "Depth" );
		if( pQuant )	depthQuantOne = pQuant[0];

		// Prepare the information and call the DspyImageOpen function in the display device.
		if(req.m_modeID & ( ModeRGB | ModeA | ModeZ) )
		{
			PtDspyDevFormat fmt;
			if( colorQuantOne == 255 )
				fmt.type = PkDspyUnsigned8;
			else if( colorQuantOne == 65535 )
				fmt.type = PkDspyUnsigned16;
			else if( colorQuantOne == 4294967295 )
				fmt.type = PkDspyUnsigned32;
			else fmt.type = PkDspyFloat32;
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
        
		// Call the DspyImageOpen method on the display to initialise things.
		PtDspyError err = (*req.m_OpenMethod)(&req.m_imageHandle, req.m_type.c_str(), req.m_name.c_str(), QGetRenderContext() ->pImage() ->iXRes(), QGetRenderContext() ->pImage() ->iYRes(), 0, NULL, req.m_formats.size(), &req.m_formats[0], &req.m_flags);

		// Now scan the returned format list to make sure that we pass the data in the order the display wants it.
		std::vector<PtDspyDevFormat>::iterator i;
		for(i=req.m_formats.begin(); i!=req.m_formats.end(); i++)
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

		// Determine how big each pixel is by summing the format type sizes.
		req.m_elementSize = 0;
		std::vector<PtDspyDevFormat>::iterator iformat;
		for(iformat = req.m_formats.begin(); iformat != req.m_formats.end(); iformat++)
		{
			switch(iformat->type)
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

		if( NULL != req.m_QueryMethod )
		{
			PtDspySizeInfo size;
			err = (*req.m_QueryMethod)(req.m_imageHandle, PkSizeQuery, sizeof(size), &size);
			PtDspyOverwriteInfo owinfo;
			err = (*req.m_QueryMethod)(req.m_imageHandle, PkOverwriteQuery, sizeof(owinfo), &owinfo);
		}
    }

	std::cerr << debug << "Load Display: " << strDriverPathAndFile.c_str() << std::endl;
}

void CqDDManager::CloseDisplayLibrary( SqDisplayRequest& req )
{
}

void CqDDManager::InitialiseDisplayNameMap()
{
    // Read in the configuration file.
    // Find the config file in the same place as the display drivers.
#ifdef AQSIS_SYSTEM_POSIX
    CqString ddmsock_path( "" );
    char* env = NULL;

    env = getenv( "AQSIS_CONFIG_PATH" );
    if ( env == NULL )
    {
#ifndef AQSIS_SYSTEM_MACOSX
        ddmsock_path = CONFIG_PATH;
#else
        ddmsock_path = ".";
#endif

    }
    else
    {
        ddmsock_path = env;
    }

    ddmsock_path.append( "/" );

    ddmsock_path.append( "displays.ini" );

    CqString strConfigFile = ddmsock_path;
#else
    CqString strConfigFile = "displays.ini";
#endif /* AQSIS_SYSTEM_POSIX */

    CqRiFile fileINI( strConfigFile.c_str(), "display" );
    if ( fileINI.IsValid() )
    {
        // On each line, read the first string, then the second and store them in the map.
        std::string strLine;
        std::istream& strmINI = static_cast<std::istream&>( fileINI );

        while ( std::getline( strmINI, strLine ) )
        {
            std::string strName, strDriverName;
            std::string::size_type iStartN = strLine.find_first_not_of( '\t' );
            std::string::size_type iEndN = strLine.find_first_of( '\t', iStartN );
            std::string::size_type iStartD = strLine.find_first_not_of( '\t', iEndN );
            std::string::size_type iEndD = strLine.find_first_of( '\t', iStartD );
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
        std::cerr << error << "Could not find ddmsock.ini file." << std::endl;
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
        case 0: if ( c != ' ' && c != '\t' )
            {
                idx--;
                end = start + 1;
                z = 1;
            }
            if ( idx > 0 ) start++;
            break;
        case 1: if ( c == ' ' || c == '\t' )
            {
                z = 0;
            }
            start++;
            break;
        case 2: if ( c == ' ' || c == '\t' )
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


std::string PrepareCustomParameters( std::map<std::string, void*>& mapParams )
{
    std::stringstream customParamArgs;
	std::stringstream customParamNames;
	std::stringstream customParamCounts;
	std::stringstream customParamStrings;
	std::stringstream customParamInts;
	std::stringstream customParamFloats;

	std::map<std::string, void*>::iterator param;
    for ( param = mapParams.begin(); param != mapParams.end(); param++ )
    {
        SqParameterDeclaration Decl;
        try
        {
            Decl = QGetRenderContext() ->FindParameterDecl( param->first.c_str() );
        }
        catch( XqException e )
        {
            std::cerr << error << e.strReason().c_str() << std::endl;
            return("");
        }

        // Check the parameter type is uniform, not valid for non-surface requests otherwise.
        if( Decl.m_Class != class_uniform )
        {
            assert( TqFalse );
            continue;
        }

		if( param != mapParams.begin() )
		{
			customParamNames << ",";
			customParamCounts << ",";
		}

		// Store the name
		customParamNames << "\"" << Decl.m_strName << "\"";
		TqInt i;
		const char** strings;
		const int* ints;
		const float* floats;
        switch ( Decl.m_Type )
        {
			case type_string:
				customParamCounts << 0 << "," << 0 << "," << Decl.m_Count;
				// If there are already some arguments of this type, then add a separating ','
				if(!customParamStrings.str().empty())
					customParamStrings << ",";
				strings = static_cast<const char**>( param->second );
				for( i = 0; i < Decl.m_Count; i++ )
				{
					customParamStrings << strings[i];
					if( i+1 != Decl.m_Count )
						customParamStrings << ",";
				}
				break;

			case type_float:
				customParamCounts << 0 << "," << Decl.m_Count << "," << 0;
				// If there are already some arguments of this type, then add a separating ','
				if(!customParamFloats.str().empty())
					customParamFloats << ",";
				floats = static_cast<float*>( param->second );
				for( i = 0; i < Decl.m_Count; i++ )
				{
					customParamFloats << floats[i];
					if( i+1 != Decl.m_Count )
						customParamFloats << ",";
				}
				break;

			case type_integer:
				customParamCounts << Decl.m_Count << "," << 0 << "," << 0;
				// If there are already some arguments of this type, then add a separating ','
				if(!customParamInts.str().empty())
					customParamInts << ",";
				ints = static_cast<int*>( param->second );
				for( i = 0; i < Decl.m_Count; i++ )
				{
					customParamInts << ints[i];
					if( i+1 != Decl.m_Count )
						customParamFloats << ",";
				}
				break;

			case type_point:
			case type_normal:
			case type_vector:
			case type_color:
				customParamCounts << 0 << "," << Decl.m_Count << "," << 0;
				// If there are already some arguments of this type, then add a separating ','
				if(!customParamFloats.str().empty())
					customParamFloats << ",";
				floats = static_cast<float*>( param->second );
				for( i = 0; i < Decl.m_Count * 3; i++ )
				{
					customParamFloats << floats[i];
					if( i+1 != Decl.m_Count )
						customParamFloats << ",";
				}
				break;

			case type_hpoint:
				customParamCounts << 0 << "," << Decl.m_Count << "," << 0;
				// If there are already some arguments of this type, then add a separating ','
				if(!customParamFloats.str().empty())
					customParamFloats << ",";
				floats = static_cast<float*>( param->second );
				for( i = 0; i < Decl.m_Count * 4; i++ )
				{
					customParamFloats << floats[i];
					if( i+1 != Decl.m_Count )
						customParamFloats << ",";
				}
				break;

			case type_matrix:
				customParamCounts << 0 << "," << Decl.m_Count << "," << 0;
				// If there are already some arguments of this type, then add a separating ','
				if(!customParamFloats.str().empty())
					customParamFloats << ",";
				floats = static_cast<float*>( param->second );
				for( i = 0; i < Decl.m_Count * 16; i++ )
				{
					customParamFloats << floats[i];
					if( i+1 != Decl.m_Count )
						customParamFloats << ",";
				}
				break;
        }
    }
	if(mapParams.size() > 0 )
	{
		customParamArgs << "--paramnames=" << customParamNames.str().c_str() << " ";
		customParamArgs << "--paramcounts=" << customParamCounts.str().c_str() << " ";
		if( !customParamInts.str().empty() )
			customParamArgs << "--paramints=" << customParamInts.str().c_str() << " ";
		if( !customParamFloats.str().empty() )
			customParamArgs << "--paramfloats=" << customParamFloats.str().c_str() << " ";
		if( !customParamStrings.str().empty() )
			customParamArgs << "--paramstrings=" << customParamStrings.str().c_str() << " ";
	}

	return(customParamArgs.str());
}


END_NAMESPACE( Aqsis )
