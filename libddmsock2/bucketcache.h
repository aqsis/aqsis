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

//? Is context.h included already?
#ifndef BUCKETCACHE_H_INCLUDED
#define BUCKETCACHE_H_INCLUDED 1

#include	<vector>
#include	<string>

START_NAMESPACE( Aqsis )

struct IqBucket;

/**
 * Data format.
 *	Header
 *		int			-	Header length (in bytes).
 *		int			-	Number of data values per pixel.
 *		int			-	Size of each pixel (in bytes).
 *		int			-	Length of data values names string (including terminating '/').
 *		int[]		-	Array of element counts for each data value, i.e. 3 for "Os" (r,g,b), 1 for "s".
 *						The array length is equal to the number of data values.
 *		char[]		-	String describing the data values stored, each entry separated by '/'.
 *						i.e. "Os/s/t/N/"
 *	Buckets...
 *		long		-	Total length of bucket (in bytes).
 *		int, int	-	Bucket position in x,y
 *		int, int	-	Bucket size in x, y
 *		float[]		-	Bucket data. Size is "size of each pixel" * "bucket width" * "bucket height"
 */


//---------------------------------------------------------------------
/** \class CqBucketDiskStore
 * Class responsible for managing the on disk bucket storage.
 */

class CqBucketDiskStore
{
	public:
		CqBucketDiskStore()	: m_Header(NULL), m_Temporary(TqTrue)	{}
		~CqBucketDiskStore();

		//---------------------------------------------------------------------
		/** \struct SqBucketDiskStoreHeader
		 * A structure that describes the header of the bucket disk store.
		 */

		struct SqBucketDiskStoreHeader
		{
			TqInt	m_HeaderLength;
			TqInt	m_DataValueCount, m_DataValueSize, m_DataValueDescLen;
			TqInt	m_Data[1];
		};


		//---------------------------------------------------------------------
		/** \struct SqBucketDiskStoreRecord
		 * A structure used to reference information stored in the disk based bucket store.
		 */

		struct SqBucketDiskStoreRecord
		{
			TqLong	m_BucketLength;	
			TqInt	m_OriginX, m_OriginY;
			TqInt	m_Width, m_Height;
			TqFloat	m_Data[1];
		};

		void	PrepareFile(std::string& name, TqBool temp = TqTrue);
		void	CloseDown();
		TqLong	StoreBucket(IqBucket* bucket, SqBucketDiskStoreRecord** record_out = NULL, TqInt* index_out = NULL);
		SqBucketDiskStoreRecord* RetrieveBucketOrigin(TqInt originx, TqInt originy);
		SqBucketDiskStoreRecord* RetrieveBucketIndex(TqInt index);

		TqInt	GetDataValueSize()
		{
			if(m_Header)
				return(m_Header->m_DataValueSize);
			else
				return(0);
		}

	private:
		SqBucketDiskStoreRecord* RetrieveBucketOffset(TqLong offset);
		
		std::string	m_fileName;
		SqBucketDiskStoreHeader* m_Header;
		TqInt m_StoredBuckets;
		std::map<TqInt, std::map<TqInt, TqLong> >	m_OriginTable;
		std::vector<TqLong>							m_IndexTable;
		TqBool	m_Temporary;			///< Flag indicating the file should be deleted when done.
};

END_NAMESPACE( Aqsis )

#endif	// BUCKETCACHE_H_INCLUDED
