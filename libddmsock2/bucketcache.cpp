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
		\brief Bucket data caching handler.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/


#include	"aqsis.h"

#include	<fstream>
#include	<map>
#include	"signal.h"
#include	<stdio.h>
#include	<stdlib.h>

#include	"renderer.h"
#include	"displaydriver.h"
#include	"ddmsock2.h"
#include	"imagebuffer.h"

#ifdef AQSIS_SYSTEM_WIN32

	#define unlink _unlink

#endif // AQSIS_SYSTEM_WIN32

START_NAMESPACE( Aqsis )

CqBucketDiskStore::~CqBucketDiskStore()
{
	CloseDown();
}


/**	\brief	Prepare the disk store file.

			The requested file will be created and truncated to 0. The standard header will be 
			written to the file and it will be left closed, ready for appending the bucket data.

	\param	name	The name of the file to use as the disk store, will be created.
					overwrites any existing file.
	\param	renderer	Pointer to the current renderer.
	\param	temp	Flag indicating if the file should be deleted when complete.

	\return	void
*/
void CqBucketDiskStore::PrepareFile(std::string& name, TqBool temp)
{
	m_fileName = name;
	m_Temporary = temp;

	std::ofstream file(name.c_str());
	if( file.is_open() )
	{
		std::vector<TqInt> counts;
		TqInt datasize = 5;
		counts.push_back(5);
		std::string desc("rgbaz/");
		std::map<std::string, CqRenderer::SqOutputDataEntry>& mapAOV=QGetRenderContext()->GetMapOfOutputDataEntries();
		std::map<std::string, CqRenderer::SqOutputDataEntry>::iterator iAOVEntry;
		for(iAOVEntry = mapAOV.begin(); iAOVEntry!=mapAOV.end(); iAOVEntry++)
		{
			desc.append(iAOVEntry->first);
			counts.push_back(iAOVEntry->second.m_NumSamples);
			datasize+=iAOVEntry->second.m_NumSamples;
		}
		TqInt len = sizeof(SqBucketDiskStoreHeader) + (counts.size()-1)*sizeof(TqInt) + desc.length();
		m_Header = reinterpret_cast<SqBucketDiskStoreHeader*>(calloc(len, 1));
		m_Header->m_DataValueCount = counts.size();
		m_Header->m_DataValueDescLen = desc.length();
		m_Header->m_DataValueSize = datasize;
		for(int i = 0; i < counts.size(); i++)
			m_Header->m_Data[i] = counts[i];
		char* headerDescription = reinterpret_cast<char*>(&m_Header->m_Data[m_Header->m_DataValueCount]);
		strncpy(headerDescription, desc.c_str(), desc.length() );
		m_Header->m_HeaderLength = len;

		file.write(reinterpret_cast<const char*>(m_Header), len);
		file.close();

		m_StoredBuckets = 0;
	}
	else
		throw("ERROR: Cannot create disk based bucket store");
}



/**	\brief	Store the given bucket data to the disk store.

			The PrepareFile function should already have been called to prepare the
			disk store for buckets.

	\param	bucket	Pointer to the bucket object containing the rendered data to store.
	\param	record_out	Optional output variable for a pointer to the bucket record area. 
						If this is NULL, the record memory is freed.
						If not, it is filled with the address of the memory area containing an 
						CqBucketDiskStore::SqBucketDiskStoreRecord structure. 
						It is then the responsibility of the calling function to free the record.
	\param	index_out	Optional output variable for the index in the bucket store. 
						If this is NULL, the index is not returned. If not, then the record index within
						the bucket store is returned and can be used in subsequent 
						CqBucketDiskStore::RetrieveBucketIndex calls.

	\return	TqLong		The record offset within the bucket store.
						The return value can be used in subsequent CqBucketDiskStore::RetrieveBucketOffset calls.

	
*/
TqLong CqBucketDiskStore::StoreBucket(IqBucket* bucket, std::map<TqInt, boost::condition*>& conditions, TqInt* index_out )
{
	TqLong seek_offset = -1;

	assert(!m_fileName.empty());
	
	std::ofstream file(m_fileName.c_str(), std::ios_base::out | std::ios_base::app | std::ios_base::binary);
	if( file.is_open() )
	{
		file.seekp(0, std::ios_base::end);
		seek_offset = file.tellp();

		SqBucketDiskStoreRecord* record;
		TqInt datalen = ( bucket->Width() * bucket->Height() * m_Header->m_DataValueSize * sizeof(float) );
		TqInt totallen = sizeof(SqBucketDiskStoreRecord) + datalen - sizeof(float);
		record = reinterpret_cast<SqBucketDiskStoreRecord*>(malloc(totallen));
		TqInt x, y;
		TqFloat* recordaddress = &record->m_Data[0];
		for(y = 0; y < bucket->Height(); y++)
		{
			for(x = 0; x < bucket->Width(); x++ )
			{
				const TqFloat* data = bucket->Data(x + bucket->XOrigin(), y + bucket->YOrigin());
				recordaddress[0] = data[0];	// r
				recordaddress[1] = data[1];	// g
				recordaddress[2] = data[2];	// b
                TqFloat a = ( data[3] + data[4] + data[5] ) / 3.0f;
                recordaddress[3] = a * data[7]; // a
				recordaddress[4] = data[6];	// z
				// Copy any AOV variables.
				TqInt i=5;
				for(; i<m_Header->m_DataValueSize; i++)
					recordaddress[i] = data[i+3];
				recordaddress += m_Header->m_DataValueSize;
			}
		}
		record->m_OriginX = bucket->XOrigin();
		record->m_OriginY = bucket->YOrigin();
		record->m_Height = bucket->Height();
		record->m_Width = bucket->Width();
		record->m_BucketLength = totallen;

		file.write(reinterpret_cast<const char*>(record), totallen);
		m_IndexTable.push_back(seek_offset);
		m_StoredBuckets++;
		TqInt index = m_IndexTable.size()-1;
		// Check if any sender threads are waiting on this bucket.
		std::map<TqInt, boost::condition*>::iterator waiting;
		if((waiting = conditions.find(index)) != conditions.end() )
			CacheBucket(record, index);
		else
			free(record);
		if(index_out!=NULL)
			*index_out = m_IndexTable.size()-1;

		file.close();
	}
	return(seek_offset);
}


/**	\brief	Retrive an indexed bucket from the store.

			Buckets are stored in the disk store in the order they are rendered, so passing
			incrementing values for this will retrieve the buckets in the order they were rendered.

	\param	index	The index of the bucket within the disk store.

	\return	A pointer to an area of memory storing a CqBucketDiskStore::SqBucketDiskStoreRecord. It is the responsibility 
			of the calling function to free the memory pointed to by the record pointer.
			NULL is returned if the bucket cannot be found.

	
*/
CqBucketDiskStore::SqBucketDiskStoreRecord* CqBucketDiskStore::RetrieveBucketIndex(TqInt index)
{
	// First check if the index is in the memory cache.
	boost::mutex::scoped_lock lk(m_MemoryCacheMutex);
	if( m_MemoryCache.find(index) != m_MemoryCache.end() )
	{
		//std::cerr << debug << "Bucket found in cache: " << index << std::endl;
		// Lock the cache entry so that it doesn't get freed while it is in use.
		m_MemoryCache[index].second++;
		return( m_MemoryCache[index].first );
	}
	else
	{
		// Check if the index is available.
		if(index < m_IndexTable.size() )
		{
			SqBucketDiskStoreRecord* record = RetrieveBucketOffset(m_IndexTable[index]);
			if( record )
			{
				CacheBucket(record, index);

				//std::cerr << debug << "Bucket memory cache size: " << m_MemoryCacheSize << std::endl;
			}
			return(record);
		}
		else
			return(0);
	}
}


/**	\brief	Private function. Retrive a bucket from the store at the specified offset.

			The offset must be valid, hence it is only called via the alternative retriveal
			functions, where the proper offset is calculated from more appropriate specifications.

	\param	offset	The offset within the file to retrieve the bucket from.

	\return	A pointer to an area of memory storing a CqBucketDiskStore::SqBucketDiskStoreRecord. It is the responsibility 
			of the calling function to free the memory pointed to by the record pointer.
			NULL is returned if the bucket cannot be found.

	
*/
CqBucketDiskStore::SqBucketDiskStoreRecord* CqBucketDiskStore::RetrieveBucketOffset(TqLong offset)
{
	std::ifstream file(m_fileName.c_str(), std::ios_base::in | std::ios_base::binary);
	if( file.is_open() )
	{
		file.seekg(offset, std::ios_base::beg);
		// Just check we were able to get there...
		TqLong seek_offset = file.tellg();
		if( seek_offset != offset )
			return(0);

		// Read the length
		TqLong len;
		file.read(reinterpret_cast<char*>(&len), sizeof(TqLong));

		// Allocate enough space for the whole bucket data.
		SqBucketDiskStoreRecord* record = reinterpret_cast<SqBucketDiskStoreRecord*>(calloc(len, 1));
		if(!record)
			throw("ERROR: Out of memory reading bucket from disk store!");
		record->m_BucketLength = len;
		file.read(reinterpret_cast<char*>(&record->m_OriginX), len-sizeof(TqLong));
		file.close();

		return(record);
	}
	return(0);
}


/**	\brief	Shutdown the bucket cache, deleting the temporary file and cleaning up any data relevant to the current pass.

	This function should be called after each complete frame to clear up the current cache in preparation for reinitialising
	for the next render using PrepareFile.


	\return	void

	
*/
void CqBucketDiskStore::CloseDown()
{
	if( m_Temporary )
	{
		if(unlink(m_fileName.c_str()) < 0)
			std::cerr << error << "Could not delete temporary bucket store " << m_fileName.c_str() << std::endl;
	}
	free(m_Header);

	m_IndexTable.clear();
	
	// Free any entries we have in the memory cache
	boost::mutex::scoped_lock lk(m_MemoryCacheMutex);
	std::map<TqInt, std::pair<SqBucketDiskStoreRecord*, TqInt> >::iterator i;
	for( i = m_MemoryCache.begin(); i != m_MemoryCache.end(); i++ )
		free(i->second.first);

	m_MemoryCache.clear();
	m_MemoryCacheSize = 0;
	m_MemoryCacheReferences.clear();
}


void CqBucketDiskStore::ReleaseBucket(SqBucketDiskStoreRecord* record, TqInt index)
{
	// Check if the bucket is in the cache.
	boost::mutex::scoped_lock lk(m_MemoryCacheMutex);
	std::map<TqInt, std::pair<SqBucketDiskStoreRecord*, TqInt> >::iterator i;
	if( ( i = m_MemoryCache.find(index) ) != m_MemoryCache.end() && i->second.first == record )
		// If so, and it is valid, then mark it as no longer protected.
		i->second.second--;
}


void CqBucketDiskStore::CacheBucket(SqBucketDiskStoreRecord* record, TqInt index)
{
	boost::mutex::scoped_lock lk(m_MemoryCacheMutex);
	// Add the bucket to the most recently referenced list and add it to the memory cache.
	if( ( m_MemoryCacheSize + record->m_BucketLength ) > 1024 * 50 )
	{
		// Free the least most recently used cache entry, that isn't locked.
		std::deque<TqInt>::reverse_iterator i;
		for( i = m_MemoryCacheReferences.rbegin(); i != m_MemoryCacheReferences.rend(); i++ )
		{
			// Check if it is locked.
			if( m_MemoryCache[*i].second == 0)
			{
				SqBucketDiskStoreRecord* oldrec = m_MemoryCache[*i].first;
				if( oldrec )
				{
					m_MemoryCacheSize -= oldrec->m_BucketLength;
					free(oldrec);
				}
				m_MemoryCache.erase(m_MemoryCache.find(*i));
				m_MemoryCacheReferences.erase(i.base());
			}
		}
	}
	// Create a new memory cache entry for this record, keyed by it's index.
	m_MemoryCache[index] = std::pair<SqBucketDiskStoreRecord*, TqBool>(record, 1);
	// Clear any previous entries in the reference table for this index.
	std::remove(m_MemoryCacheReferences.begin(), m_MemoryCacheReferences.end(), index);
	// And store the index at the top, as the most recently referenced record.
	m_MemoryCacheReferences.push_front(index);
	// Increase the cache memory use by the size of this new bucket record.
	m_MemoryCacheSize += record->m_BucketLength;
}

END_NAMESPACE( Aqsis )
