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

//? Is .h included already?
#ifndef ___ddmanager_Loaded___
#define ___ddmanager_Loaded___

#include	<vector>

#include	"aqsis.h"
#include	"ri.h"
#include	"iddmanager.h"
#include	"plugins.h"
#define		DSPY_INTERNAL
#include	"ndspy.h"
#undef		DSPY_INTERNAL

START_NAMESPACE( Aqsis )


//---------------------------------------------------------------------
/** \class CqDDManagerSimple
 * Class providing display device management to the renderer.
 */

class CqDDManager : public IqDDManager
{
	public:
		CqDDManager() : m_fDisplayMapInitialised(TqFalse)
		{}
		virtual ~CqDDManager()
		{}

		// Overridden from IqDDManager

		virtual	TqInt	Initialise()
		{
			return ( 0 );
		}
		virtual	TqInt	Shutdown()
		{
			return ( 0 );
		}
		virtual	TqInt	AddDisplay( const TqChar* name, const TqChar* type, const TqChar* mode, TqInt modeID, TqInt dataOffset, TqInt dataSize, std::map<std::string, void*> mapOfArguments );
		virtual	TqInt	ClearDisplays();
		virtual	TqInt	OpenDisplays();
		virtual	TqInt	CloseDisplays();
		virtual	TqInt	DisplayBucket( IqBucket* pBucket );
		virtual	TqBool	fDisplayNeeds( const TqChar* var );
		virtual	TqInt	Uses();

	private:
		std::string	GetStringField( const std::string& s, int idx );
		void	InitialiseDisplayNameMap();

		struct SqDisplayRequest
		{
			TqBool		m_valid;
			std::string m_name;
			std::string m_type;
			std::string m_mode;
			TqUlong		m_modeHash;
			TqInt		m_modeID;
			TqInt		m_AOVOffset;
			TqInt		m_AOVSize;
			std::vector<UserParameter> m_customParams;
			void*		m_DriverHandle;
			PtDspyImageHandle m_imageHandle;
			PtFlagStuff	m_flags;
			std::vector<PtDspyDevFormat> m_formats;
			std::vector<TqInt>			m_dataOffsets;
			std::vector<std::string>	m_AOVnames;
			TqInt		m_elementSize;
			TqFloat		m_QuantizeZeroVal;
			TqFloat		m_QuantizeOneVal;
			TqFloat		m_QuantizeMinVal;
			TqFloat		m_QuantizeMaxVal;
			TqFloat		m_QuantizeDitherVal;
			TqBool		m_QuantizeSpecified;
			TqBool		m_QuantizeDitherSpecified;
			DspyImageOpenMethod			m_OpenMethod;
			DspyImageQueryMethod		m_QueryMethod;
			DspyImageDataMethod			m_DataMethod;
			DspyImageCloseMethod		m_CloseMethod;
			DspyImageDelayCloseMethod	m_DelayCloseMethod;
			unsigned char  *m_DataRow;    // A row of bucket's data
			unsigned char  *m_DataBucket; // A bucket's data
		};

		void	LoadDisplayLibrary( SqDisplayRequest& req );
		void	CloseDisplayLibrary( SqDisplayRequest& req );
		void	PrepareCustomParameters( std::map<std::string, void*>& mapParams, SqDisplayRequest& req );
		void	PrepareSystemParameters( SqDisplayRequest& req );

		std::vector<SqDisplayRequest>	m_displayRequests;		///< Array of requested display drivers.
		TqBool	m_fDisplayMapInitialised;
		std::map<std::string, std::string>	m_mapDisplayNames;

		static CqString m_strOpenMethod;
		static CqString m_strQueryMethod;
		static CqString m_strDataMethod;
		static CqString m_strCloseMethod;
		static CqString m_strDelayCloseMethod;
		CqSimplePlugin m_DspyPlugin;

		static char* m_RedName;
		static char* m_GreenName;
		static char* m_BlueName;
		static char* m_AlphaName;
		static char* m_ZName;
};


END_NAMESPACE( Aqsis )

#endif	// ___ddmanager_Loaded___

