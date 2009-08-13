// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
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

#include	<aqsis/aqsis.h>
#include	<aqsis/math/matrix.h>
#include	<aqsis/ri/ri.h>
#include	"iddmanager.h"
#include	<aqsis/util/plugins.h>
#define		DSPY_INTERNAL
#include	<aqsis/ri/ndspy.h>
#undef		DSPY_INTERNAL

namespace Aqsis {

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
	               CqString strCloseMethod, CqString strDelayCloseMethod,
		       const char* redName, const char* greenName, const char* blueName,
		       const char* alphaName, const char* zName) :
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

	const char* m_RedName;
	const char* m_GreenName;
	const char* m_BlueName;
	const char* m_AlphaName;
	const char* m_ZName;
};

//---------------------------------------------------------------------
/** \class CqDisplayRequest
 * Base class for display requests.
 *
 * \todo <b>Code review</b>: This class could be tweaked to use a more
 * RAII-like style.
 */
class CqDisplayRequest : public IqDisplayRequest
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
				m_QuantizeMinVal(quantizeMinVal), m_QuantizeMaxVal(quantizeMaxVal), m_QuantizeDitherVal(quantizeDitherVal), m_QuantizeSpecified(quantizeSpecified), m_QuantizeDitherSpecified(quantizeDitherSpecified),
				m_isLoaded(false)
		{}
		virtual ~CqDisplayRequest();

		/* Query if this display uses the dispay mode variable (one of rgbaz)
		 * specified by the hash token.
		 */
		virtual bool ThisDisplayNeeds( const TqUlong& htoken, const TqUlong& rgb, const TqUlong& rgba,
		                               const TqUlong& Ci, const TqUlong& Oi, const TqUlong& Cs, const TqUlong& Os );
		/* Determine all of the environment variables this display uses
		 * by querying this display's mode hash.
		 */
		virtual	void ThisDisplayUses( TqInt& Uses );

		void LoadDisplayLibrary( SqDDMemberData& ddMemberData, CqSimplePlugin& dspyPlugin, TqInt dspNo, TqInt width, TqInt height );
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
		virtual void DisplayBucket( const CqRegion& DRegion, const IqChannelBuffer* pBuffer);

		//----------------------------------------------
		// Pure virtual functions
		//----------------------------------------------
		// Overridden from IqDisplayRequest
		virtual std::string& 	name();
		virtual const std::string& 	name() const;
		virtual bool isLoaded() const;
		/* Does quantization, or in the case of DSM does the compression.
		 */
		virtual void FormatBucketForDisplay( const CqRegion& DRegion, const IqChannelBuffer* pBuffer);
		/* Collapses a row of buckets into a scanline by copying the
		 * quantized data into a format readable by the display.
		 * Used when the display wants scanline order.
		 * Return true if a full row is ready, false otherwise.
		 */
		virtual bool CollapseBucketsToScanlines(const CqRegion& DRegion);
		/* Sends the data to the display.
		*/
		virtual void SendToDisplay(TqInt ymin, TqInt ymaxplus1);

	protected:
		bool			m_valid;
		std::string 	m_name;
		std::string 	m_type;
		std::string 	m_mode;
		TqUlong			m_modeHash;
		TqInt			m_modeID;
		TqInt			m_AOVOffset;
		TqInt			m_AOVSize;
		TqInt			m_width;
		TqInt			m_height;
		std::vector<UserParameter> m_customParams;
		void*			m_DriverHandle;
		PtDspyImageHandle m_imageHandle;
		PtFlagStuff		m_flags;
		std::vector<PtDspyDevFormat> m_formats;
		std::map<std::string, std::pair<std::string, TqInt> > m_bufferMap;
		TqInt			m_elementSize;
		TqFloat			m_QuantizeZeroVal;
		TqFloat			m_QuantizeOneVal;
		TqFloat			m_QuantizeMinVal;
		TqFloat			m_QuantizeMaxVal;
		TqFloat			m_QuantizeDitherVal;
		bool			m_QuantizeSpecified;
		bool			m_QuantizeDitherSpecified;
		DspyImageOpenMethod			m_OpenMethod;
		DspyImageQueryMethod		m_QueryMethod;
		DspyImageDataMethod			m_DataMethod;
		DspyImageCloseMethod		m_CloseMethod;
		DspyImageDelayCloseMethod	m_DelayCloseMethod;
		bool			m_isLoaded;

		/// \todo Some of the instance data from SqDisplayRequest
		//  should be split out into a new structure,
		//  SqFormattedBucketData.
		//  Specifically, the stuff which deals with holding the data
		//  which has been copied out of the bucket and quantized:
		unsigned char  *m_DataRow;    // A row of bucket's data
		unsigned char  *m_DataBucket; // A bucket's data

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
		virtual void FormatBucketForDisplay( const CqRegion& DRegion, const IqChannelBuffer* pBuffer);
		/* Collapses a row of buckets into a scanline by copying the
		 * quantized data into a format readable by the display.
		 * Used when the display wants scanline order.
		 * Return true if a full row is ready, false otherwise.
		 */
		virtual bool CollapseBucketsToScanlines(const CqRegion& DRegion );
		/*
		 * Sends the data to the display.
		 */
		virtual void SendToDisplay(TqInt ymin, TqInt ymaxplus1);

	private:

};

//---------------------------------------------------------------------
/** \class CqDDManagerSimple
 * Class providing display device management to the renderer.
 */

class CqDDManager : public IqDDManager
{
	public:
		CqDDManager() : m_Uses(0)
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

		virtual TqInt	numDisplayRequests();
		virtual boost::shared_ptr<IqDisplayRequest>	displayRequest(TqInt index);

		virtual	TqInt	AddDisplay( const TqChar* name, const TqChar* type, const TqChar* mode, TqInt modeID, TqInt dataOffset, TqInt dataSize, std::map<std::string, void*> mapOfArguments );
		virtual	TqInt	ClearDisplays();
		virtual	TqInt	OpenDisplays(TqInt width, TqInt height);
		virtual	TqInt	CloseDisplays();
		virtual	TqInt	DisplayBucket( const CqRegion& DRegion, const IqChannelBuffer* pBucket );
		virtual	bool	fDisplayNeeds( const TqChar* var );
		virtual	TqInt	Uses();

	private:
		std::string	GetStringField( const std::string& s, int idx );
		std::vector< boost::shared_ptr<CqDisplayRequest> > m_displayRequests; ///< Array of requested display drivers.
		static SqDDMemberData m_MemberData;
		CqSimplePlugin m_DspyPlugin;
		TqInt 	m_Uses;
};


//----------------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------------

std::string& 	CqDisplayRequest::name()
{
	return m_name;
}

const std::string& 	CqDisplayRequest::name() const
{
	return m_name;
}

bool CqDisplayRequest::isLoaded() const
{
	return m_isLoaded;
}

TqInt CqDDManager::numDisplayRequests()
{
	return m_displayRequests.size();
}

boost::shared_ptr<IqDisplayRequest> CqDDManager::displayRequest(TqInt index)
{
	return m_displayRequests[index];
}

} // namespace Aqsis

#endif	// ___ddmanager_Loaded___

