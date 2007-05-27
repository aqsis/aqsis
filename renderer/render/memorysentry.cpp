#include "memorysentry.h"

#include "logging.h"

namespace Aqsis
{

//------------------------------------------------------------------------------
// Implementation for CqMemorySentry
//------------------------------------------------------------------------------
CqMemorySentry::CqMemorySentry(const TqMemorySize maxMemory)
	: m_maxMemory(maxMemory)
{
}

//------------------------------------------------------------------------------
void CqMemorySentry::registerAsManaged(const boost::shared_ptr<CqMemoryMonitored>& managedObject)
{
	m_managedList.push_back(boost::weak_ptr<CqMemoryMonitored>(managedObject));
}

//------------------------------------------------------------------------------
void CqMemorySentry::incrementTotalMemory(const TqMemorySize numBytes)
{
	m_totalMemory += numBytes;
	if(m_totalMemory > m_maxMemory)
	{
		/// \todo: Walk through the object list zapping some memory.
		Aqsis::log() << warning << "Exceeded global memory for textures.\n";
	}
}


//------------------------------------------------------------------------------
// Implementation for CqMemoryMonitored
//------------------------------------------------------------------------------
CqMemoryMonitored::CqMemoryMonitored(CqMemorySentry& memorySentry)
	: m_memorySentry(memorySentry)
{
}

//------------------------------------------------------------------------------
CqMemorySentry& CqMemoryMonitored::getMemorySentry() const
{
	return m_memorySentry;
}

//------------------------------------------------------------------------------
// Virtual destructor empty implementation.
CqMemoryMonitored::~CqMemoryMonitored()
{ }

//------------------------------------------------------------------------------

} // namespace Aqsis
