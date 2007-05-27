#include "tilearray.h"

namespace Aqsis
{

//------------------------------------------------------------------------------
// Implementation for CqTextureTile
//------------------------------------------------------------------------------

template<typename T>
CqTextureTile<T>::CqTextureTile<T>(T* data, TqUint width, TqUint height,
		TqUint topLeftX, TqUint topLeftY, TqUint samplesPerPixel)
	: m_data(data),
	m_width(width),
	m_height(height),
	m_topLeftX(topLeftX),
	m_topLeftY(topLeftY),
	m_samplesPerPixel(samplesPerPixel)
{ }

template<typename T>
void CqTextureTile<T>::setValue(const TqUint x, const TqUint y, const std::vector<TqFloat>& newValue)
{
	T* samplePtr = getSamplePtr(x,y);
	if(std::numeric_limits<T>::is_integer)
	{
		for(TqUint i = 0; i < m_samplesPerPixel; i++)
			samplePtr[i] = static_cast<T>(
					clamp<TqFloat>(newValue[i]*std::numeric_limits<T>::max(),
					std::numeric_limits<T>::min(), std::numeric_limits<T>::max()) );
	}
	else
	{
		for(TqUint i = 0; i < m_samplesPerPixel; i++)
			samplePtr[i] = static_cast<T>(newValue[i]);
	}
}

template<typename T>
CqTextureTile<T>::~CqTextureTile<T>()
{
	_TIFFfree(reinterpret_cast<tdata_t>(m_data));
}


//------------------------------------------------------------------------------
// Implementation for CqTextureTileArray
//------------------------------------------------------------------------------

template<typename T>
CqSampleVector<T> CqTextureTileArray<T>::getValue(const TqUint x, const TqUint y) const
{
	return getTileForIndex(x, y)->getValue();
}

template<typename T>
void CqTextureTileArray<T>::setValue(const TqUint x, const TqUint y, const std::vector<TqFloat>& newValue)
{
	/// \todo Implementation
}

template<typename T>
boost::intrusive_ptr<CqTextureTile<T> >& CqTextureTileArray<T>::getTileForIndex(const TqUint x, const TqUint y) const
{
	/// \todo Implementation
	return boost::intrusive_ptr<CqTextureTile<T> >(0);
}

template<typename T>
CqMemorySentry::TqMemorySize CqTextureTileArray<T>::zapMemory()
{
	/// \todo Implementation
	return 0;
}

template<typename T>
CqTextureTileArray<T>::~CqTextureTileArray<T>()
{
	/// \todo Implementation
}

//------------------------------------------------------------------------------
} // namespace Aqsis
