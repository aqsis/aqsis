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
		\brief Simple example display device manager.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef ___ddmanager_Loaded___
#define ___ddmanager_Loaded___

#include	<vector>

#include	"aqsis.h"
#include	"matrix.h"
#include	"ri.h"
#include	"iddmanager.h"
#include	"plugins.h"
#define		DSPY_INTERNAL
#include	"ndspy.h"
#undef		DSPY_INTERNAL

START_NAMESPACE( Aqsis )

//------------------------------------------------------------------------------
/** \brief Structure for packaging CqDDManager private static member variables
*
* This makes it easier for CqDDMnager to pass these variables to
* CqDisplayRequest::LoadDisplayLibrary() for initialization.
*/
struct SqDDMemberData
{	
	/** Constructor: initialize an SqDDMemberData structure
	 */ 
	SqDDMemberData(CqString strOpenMethod, CqString strQueryMethod, CqString strDataMethod,
			CqString strCloseMethod, CqString strDelayCloseMethod, char* redName, char* greenName,
			char* blueName, char* alphaName, char* zName) : 
				m_strOpenMethod(strOpenMethod),
				m_strQueryMethod(strQueryMethod),
				m_strDataMethod(strDataMethod),
				m_strCloseMethod(strCloseMethod),
				m_strDelayCloseMethod(strDelayCloseMethod),
				m_RedName(redName),
				m_GreenName(greenName),
				m_BlueName(blueName),
				m_AlphaName(alphaName),
				m_ZName(zName)
	{}
	
	//----------------------------------------------------------------
	// Member Data
	CqString m_strOpenMethod;
	CqString m_strQueryMethod;
	CqString m_strDataMethod;
	CqString m_strCloseMethod;
	CqString m_strDelayCloseMethod;

	char* m_RedName;
	char* m_GreenName;
	char* m_BlueName;
	char* m_AlphaName;
	char* m_ZName;
};
	
//---------------------------------------------------------------------
/** \class CqDisplayRequest
 * Base class for display requests.
 */
class CqDisplayRequest
{
	public:
		CqDisplayRequest()
		{}
		
		CqDisplayRequest(bool valid, const TqChar* name, const TqChar* type, const TqChar* mode,
				TqUlong modeHash, TqInt modeID, TqInt dataOffset, TqInt dataSize, TqFloat quantizeZeroVal, TqFloat quantizeOneVal, 
				TqFloat quantizeMinVal, TqFloat quantizeMaxVal, TqFloat quantizeDitherVal, bool quantizeSpecified, bool quantizeDitherSpecified) :
					m_valid(valid), m_name(name), m_type(type), m_mode(mode),
					m_modeHash(modeHash), m_modeID(modeID), m_AOVOffset(dataOffset), 
					m_AOVSize(dataSize), m_QuantizeZeroVal(quantizeZeroVal), m_QuantizeOneVal(quantizeOneVal),
					m_QuantizeMinVal(quantizeMinVal), m_QuantizeMaxVal(quantizeMaxVal), m_QuantizeDitherVal(quantizeDitherVal), m_QuantizeSpecified(quantizeSpecified), m_QuantizeDitherSpecified(quantizeDitherSpecified)
		{}
		virtual ~CqDisplayRequest(){}
		
		/* Query if this display uses the dispay mode variable (one of rgbaz)
		 * specified by the hash token.
		 */
		virtual bool ThisDisplayNeeds( const TqUlong& htoken, const TqUlong& rgb, const TqUlong& rgba,
									const TqUlong& Ci, const TqUlong& Oi, const TqUlong& Cs, const TqUlong& Os );
		/* Determine all of the environment variables this display uses
		 * by querying this display's mode hash.
		 */
		virtual	void ThisDisplayUses( TqInt& Uses );
		
		virtual void ClearDisplayParams();
		void LoadDisplayLibrary( SqDDMemberData& ddMemberData, CqSimplePlugin& dspyPlugin );
		void CloseDisplayLibrary();
		void ConstructStringsParameter(const char* name, const char** strings, TqInt count, UserParameter& parameter);
		void ConstructIntsParameter(const char* name, const TqInt* ints, TqInt count, UserParameter& parameter);
		void ConstructFloatsParameter(const char* name, const TqFloat* floats, TqInt count, UserParameter& parameter);
		void ConstructMatrixParameter(const char* name, const CqMatrix* mats, TqInt count, UserParameter& parameter);
		void PrepareCustomParameters( std::map<std::string, void*>& mapParams );
		void PrepareSystemParameters();
		
		/* Prepare a bucket for display, then send to the display device.
		 * We implement the standard functionality, but allow child classes
		 * to override.
		 */				 
		virtual void DisplayBucket(const IqBucket* pBucket);
		
		//----------------------------------------------
		// Pure virtual functions
		//----------------------------------------------		
		/* Does quantization, or in the case of DSM does the compression.
		 */				 
		virtual void FormatBucketForDisplay(const IqBucket* pBucket) = 0;
		/* Collapses a row of buckets into a scanline by copying the
		 * quantized data into a format readable by the display.  
		 * Used when the display wants scanline order.
		 * Return true if a full row is ready, false otherwise.
		 */
		virtual bool CollapseBucketsToScanlines(const IqBucket* pBucket) = 0;
		 /* Sends the data to the display.
		 */
		virtual void SendToDisplay(TqUint ymin, TqUint ymaxplus1) = 0;
		      
	protected:
		bool		m_valid;
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
		bool		m_QuantizeSpecified;
		bool		m_QuantizeDitherSpecified;		
		DspyImageOpenMethod			m_OpenMethod;
		DspyImageQueryMethod		m_QueryMethod;
		DspyImageDataMethod			m_DataMethod;
		DspyImageCloseMethod		m_CloseMethod;
		DspyImageDelayCloseMethod	m_DelayCloseMethod;	
		/// \todo Some of the instance data from SqDisplayRequest should be split out
		//  into a new structure, SqFormattedBucketData.
		//  Specifically, the stuff which deals with holding the data
		//  which has been copied out of the bucket and quantized: 
		unsigned char  *m_DataRow;    // A row of bucket's data
		unsigned char  *m_DataBucket; // A bucket's data
};

//---------------------------------------------------------------------
/** \class CqShallowDisplayRequest
 * Class representing most types of display requests, 
 * including tiff, bmp, framebuffer, etc.
 */
class CqShallowDisplayRequest : virtual public CqDisplayRequest
{
	public:
		CqShallowDisplayRequest() :
				CqDisplayRequest()
		{}
		
		CqShallowDisplayRequest(bool valid, const TqChar* name, const TqChar* type, const TqChar* mode,
				TqUlong modeHash, TqInt modeID, TqInt dataOffset, TqInt dataSize, TqFloat quantizeZeroVal, TqFloat quantizeOneVal, 
				TqFloat quantizeMinVal, TqFloat quantizeMaxVal, TqFloat quantizeDitherVal, bool quantizeSpecified, bool quantizeDitherSpecified) :
					CqDisplayRequest(valid, name, type, mode, modeHash,
							modeID, dataOffset, dataSize, quantizeZeroVal, quantizeOneVal,
							quantizeMinVal, quantizeMaxVal, quantizeDitherVal, quantizeSpecified, quantizeDitherSpecified)
		{}			
		
		virtual ~CqShallowDisplayRequest(){}
		
		/* Does quantization, or in the case of DSM does the compression.
		 */				 
		void FormatBucketForDisplay(const IqBucket* pBucket);
		/* Collapses a row of buckets into a scanline by copying the
		 * quantized data into a format readable by the display.  
		 * Used when the display wants scanline order.
		 * Return true if a full row is ready, false otherwise.
		 */
		bool CollapseBucketsToScanlines(const IqBucket* pBucket);
		/*
		 * Sends the data to the display.
		 * Invoked only when the bucket row data is full.
		 */
		void SendToDisplay(TqUint ymin, TqUint ymaxplus1);
	
	private:
		
		// Map for accumulating buckets to full rows:
		// The outer map has keys corresponding to the bucket row.
		// The inner vector stores an unsorted list of the buckets for a row.
		// The fact that it is unsorted may be an issue because the display needs to
		// receive data in sorted order. We may need to store identifying info to re-determine order.
		// Note: I want to use a boost:shared_array here, but my boost library does not have it.
		std::map<TqInt, std::vector<boost::shared_ptr<unsigned char> > > m_BucketDataMap;
};

//---------------------------------------------------------------------
/** \class CqDeepDisplayRequest
 * Class representing a DSM display request
 */
class CqDeepDisplayRequest : virtual public CqDisplayRequest
{
	public:
		CqDeepDisplayRequest() :
				CqDisplayRequest()
		{}
		
		CqDeepDisplayRequest(bool valid, const TqChar* name, const TqChar* type, const TqChar* mode,
				TqUlong modeHash, TqInt modeID, TqInt dataOffset, TqInt dataSize, TqFloat quantizeZeroVal, TqFloat quantizeOneVal, 
				TqFloat quantizeMinVal, TqFloat quantizeMaxVal, TqFloat quantizeDitherVal, bool quantizeSpecified, bool quantizeDitherSpecified) :
					CqDisplayRequest(valid, name, type, mode, modeHash,
							modeID, dataOffset, dataSize, quantizeZeroVal, quantizeOneVal,
							quantizeMinVal, quantizeMaxVal, quantizeDitherVal, quantizeSpecified, quantizeDitherSpecified)
		{}
		
		/* Does quantization, or in the case of DSM does the compression.
		 */				 
		virtual void FormatBucketForDisplay(const IqBucket* pBucket);
		/* Collapses a row of buckets into a scanline by copying the
		 * quantized data into a format readable by the display.  
		 * Used when the display wants scanline order.
		 * Return true if a full row is ready, false otherwise.
		 */
		virtual bool CollapseBucketsToScanlines(const IqBucket* pBucket);
		/*
		 * Sends the data to the display.
		 */
		virtual void SendToDisplay(TqUint ymin, TqUint ymaxplus1);
	
	private:
		
};

//---------------------------------------------------------------------
/** \class CqDDManagerSimple
 * Class providing display device management to the renderer.
 */

class CqDDManager : public IqDDManager
{
	public:
		CqDDManager() : m_fDisplayMapInitialised(false)
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
		virtual	TqInt	DisplayBucket( const IqBucket* pBucket );
		virtual	bool	fDisplayNeeds( const TqChar* var );
		virtual	TqInt	Uses();

	private:
		std::string	GetStringField( const std::string& s, int idx );
		void	InitialiseDisplayNameMap();
		std::vector< boost::shared_ptr<CqDisplayRequest> > m_displayRequests; ///< Array of requested display drivers. 
		bool	m_fDisplayMapInitialised;
		std::map<std::string, std::string>	m_mapDisplayNames;
		static SqDDMemberData m_MemberData;
		CqSimplePlugin m_DspyPlugin;
};


END_NAMESPACE( Aqsis )

#endif	// ___ddmanager_Loaded___

