//------------------------------------------------------------------------------
/**
 *    @file     file.h
 *    @author   Andrew J. Bromage
 *    @brief    Implement an Aqsis file.
 *
 *    Last change by:   $Author: pseudonym $
 *    Last change date: $Date: 2005/09/06 07:29:52 $
 */ 
//------------------------------------------------------------------------------


#ifndef    ___file_Loaded___
#define    ___file_Loaded___

#include "aqsis.h"
#include <boost/operators.hpp>
#include <string>
#include <iostream>

START_NAMESPACE( Aqsis )

class CqFileTag;


/// Class to represent an Aqsis file.
/**
    A CqFile is a chunk-based file.

    \seealso CqFile::Chunk
    \seealso CqFileTag
 */
class CqFile
{
public:
    /// Typedef to represent the id of a CqFile::Chunk.
    typedef unsigned long ChunkId;

    /// Helper struct to represent the "open" mode.
    struct Open {};

    /// Helper struct to represent the "create" mode.
    struct Create {};

    /// A handle to the chunk of a CqFile.
    class ChunkHandle {
        /// Get the data associated with the chunk.
        /**
            \return A pointer to the data.
         */
        const void* data() const;

        /// Set the data associated with the chunk.
        /**
            \param p_data A pointer to the data.
            \param p_length The length of the data.
         */
        void setData(const void* p_data, unsigned p_length);

        /// Get the id of this chunk.
        /**
            \return The id of the chunk.
         */
        ChunkId chunk() const;

        /// Get the tag of this chunk.
        /**
            \return The tag of the chunk.
         */
        CqFileTag& tag() const;

        /// Open constructor.
        /**
            Reads a chunk into the handle.
            \param p_openMode Opens an existing chunk.
            \param p_file The file to open.
            \param p_chunk The id of the chunk.
         */
        ChunkHandle(Open p_openMode, CqFile& p_file, ChunkId p_chunk);

        /// Create constructor.
        /**
            Creates a new chunk.
            \param p_createMode Creates a new chunk.
            \param p_file The file in which the chunk will be created.
            \param p_tag The type of the chunk being created.
         */
        ChunkHandle(Create p_createMode, CqFile& p_file,
                    const CqFileTag& p_tag);

        /// Destructor
        ~ChunkHandle();

    private:
        CqFile& m_file;     ///< A reference to the underlying CqFile.
        ChunkId m_id;       ///< The id of the chunk.
    };

    /// Open constructor
    /**
        Opens an existing CqFile.
        \param p_openMode Opens an existing file.
        \param p_fileName The name of the file to open.
     */
    CqFile(Open p_openMode, const std::string& p_fileName);

    /// Create constructor
    /**
        Creates a new CqFile.
        \param p_createMode Creates a new file.
        \param p_fileName The name of the file to create.
     */
    CqFile(Create p_createMode, const std::string& p_fileName);

    /// Destructor
    ~CqFile();

private:
};



/// Class to represent file tags.
/**
    Tags are used to represent the "type" of a file and/or a file chunk.

    \seealso CqFile
    \seealso CqFile::ChunkHandle
 */
class CqFileTag
    : public boost::equality_comparable1<CqFileTag>
{
public:
    /// Constructor
    /**
      \param p_tag A string tag.  Must be at least 4 chars long.
     */
    explicit CqFileTag(const char* p_tag)
    {
        m_tag = static_cast<unsigned char>(p_tag[0]) << 0
              | static_cast<unsigned char>(p_tag[1]) << 8
              | static_cast<unsigned char>(p_tag[2]) << 16
              | static_cast<unsigned char>(p_tag[3]) << 24;
    }

    /// Constructor
    /**
      \param p_tag An unsigned integer tag.
     */
    explicit CqFileTag(unsigned p_tag)
        : m_tag(p_tag)
    {
    }

    /// Friend declaration for equality testing.
    friend bool operator==(const CqFileTag& p_lhs, const CqFileTag& p_rhs);

    /// Friend declaration for operator<<.
    friend std::ostream& operator<<(std::ostream& p_os, const CqFileTag& p_tag);

private:
    friend class CqFile;

    /// Dereference operator.
    char operator[](int p_offset) const
    {
        return (m_tag >> (p_offset * 8)) & 0xff;
    }

    unsigned long m_tag;
};


/// Equality test for CqFileTag.
inline bool
operator==(const CqFileTag& p_lhs, const CqFileTag& p_rhs)
{
    return p_lhs.m_tag == p_rhs.m_tag;
}


/// Output operator test for CqFileTag.
inline std::ostream&
operator<<(std::ostream& p_os, const CqFileTag& p_tag)
{
    p_os << static_cast<char>((p_tag.m_tag >> 0) & 0xff)
         << static_cast<char>((p_tag.m_tag >> 8) & 0xff)
         << static_cast<char>((p_tag.m_tag >> 16) & 0xff)
         << static_cast<char>((p_tag.m_tag >> 24) & 0xff);
    return p_os;
}


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif    //    ___file_Loaded___

// vim: ts=4:sts=4:expandtab
