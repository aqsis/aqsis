// Aqsis
// Copyright Î÷Î÷ 1997 - 2001, Paul C. Gregory
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
		\brief Declares the CqImagePixel class responsible for storing the results.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is imagebuffer.h included already?
#ifndef IMAGEPIXEL_H_INCLUDED 
//{
#define IMAGEPIXEL_H_INCLUDED 1

#include	"aqsis.h"

#include	<vector>
#ifdef	min
#define	__old_min__ min
#undef min
#endif
#ifdef	max
#define	__old_max__ max
#undef max
#endif
#ifndef AQSIS_SYSTEM_MACOSX
#include	<valarray>
#else
#include        "macosx/valarray"
#endif
#ifdef	__old_min__
#define	min __old_min__
#undef __old_min__
#endif
#ifdef	__old_max__
#define	max __old_max__
#undef __old_max__
#endif

#include	"bitvector.h"

#include	"renderer.h"
#include        "csgtree.h"
#include	"color.h"
#include	"vector2d.h"

START_NAMESPACE( Aqsis )

//-----------------------------------------------------------------------
/** Structure representing the information at a sample point in the image.
 */

class CqCSGTreeNode;

struct SqImageSample
{
    SqImageSample( TqInt NumData = 8 )
    {
        m_Data.resize( NumData );
    }
    enum {
        Flag_Occludes = 0x0001,
        Flag_Matte = 0x0002
    };

    CqColor Cs() const
    {
        return( CqColor( m_Data[0], m_Data[1], m_Data[2] ) );
    }

    void SetCs( const CqColor& col )
    {
        assert( m_Data.size() >= 3 );
        m_Data[0] = col.fRed();
        m_Data[1] = col.fGreen();
        m_Data[2] = col.fBlue();
    }

    CqColor Os() const
    {
        return( CqColor( m_Data[3], m_Data[4], m_Data[5] ) );
    }

    void SetOs( const CqColor& col )
    {
        assert( m_Data.size() >= 6 );
        m_Data[3] = col.fRed();
        m_Data[4] = col.fGreen();
        m_Data[5] = col.fBlue();
    }

    TqFloat Depth() const
    {
        return( m_Data[6] );
    }

    void SetDepth( TqFloat d )
    {
        assert( m_Data.size() >= 7 );
        m_Data[6] = d;
    }

    TqFloat Coverage() const
    {
        return( m_Data[7] );
    }

    void SetCoverage( TqFloat d )
    {
        assert( m_Data.size() >= 8 );
        m_Data[7] = d;
    }

    TqInt DataSize() const
    {
        return( m_Data.size() );
    }

    TqInt m_flags;
    std::valarray<TqFloat>	m_Data;
    boost::shared_ptr<CqCSGTreeNode>	m_pCSGNode;	///< Pointer to the CSG node this sample is part of, NULL if not part of a solid.
}
;


/** Structure to hold the info about a sample point.
 */
struct SqSampleData
{
    CqVector2D	m_Position;				///< Sample position
    CqVector2D	m_DofOffset;			///< Dof lens offset.
    TqInt		m_SubCellIndex;		///< Subcell index.
    TqFloat		m_Time;				///< Float sample time.
    TqFloat		m_DetailLevel;		///< Float level-of-detail sample.
};

//-----------------------------------------------------------------------
/** Storage class for all data relating to a single pixel in the image.
 */

class CqImagePixel
{
public:
    CqImagePixel();
    CqImagePixel( const CqImagePixel& ieFrom );
    virtual	~CqImagePixel();

    /** Get the number of horizontal samples in this pixel
     * \return The number of samples as an integer.
     */
    TqInt	XSamples() const
    {
        return ( m_XSamples );
    }
    /** Get the number of vertical samples in this pixel
     * \return The number of samples as an integer.
     */
    TqInt	YSamples() const
    {
        return ( m_YSamples );
    }
    void	AllocateSamples( TqInt XSamples, TqInt YSamples );
    void	InitialiseSamples( std::vector<CqVector2D>& vecSamples, TqBool fJitter = TqTrue );
    void	OffsetSamples(CqVector2D& vecPixel, std::vector<CqVector2D>& vecSamples)
	{
		// add in the pixel offset
		const TqInt numSamples = m_XSamples * m_YSamples;
		for ( TqInt i = 0; i < numSamples; i++ )
		{
			m_Samples[ i ].m_Position = vecSamples[ i ];
			m_Samples[ i ].m_Position += vecPixel;
		}
	}

    /** Get the approximate coverage of this pixel.
     * \return Float fraction of the pixel covered.
     */
    TqFloat	Coverage()
    {
        return ( m_Data.Coverage() );
    }
    void	SetCoverage( TqFloat c )
    {
        m_Data.SetCoverage( c );
    }
    /** Get the averaged color of this pixel
     * \return A color representing the averaged color at this pixel.
     * \attention Only call this after already calling FilterBucket().
     */
    CqColor	Color()
    {
        return ( m_Data.Cs() );
    }
    void	SetColor(const CqColor& col)
    {
        m_Data.SetCs( col );
    }
    /** Get the averaged opacity of this pixel
     * \return A color representing the averaged opacity at this pixel.
     * \attention Only call this after already calling FilterBucket().
     */
    CqColor	Opacity()
    {
        return ( m_Data.Os() );
    }
    void	SetOpacity(const CqColor& col)
    {
        m_Data.SetOs( col );
    }
    /** Get the averaged depth of this pixel
     * \return A float representing the averaged depth at this pixel.
     * \attention Only call this after already calling FilterBucket().
     */
    TqFloat	Depth()
    {
        return ( m_Data.Depth() );
    }
    void	SetDepth( TqFloat d )
    {
        m_Data.SetDepth( d );
    }
    /** Get a pointer to the sample data
     * \return A constant pointer to the sample data.
     */
    const TqFloat*	Data()
    {
        return ( &m_Data.m_Data[0] );
    }
    SqImageSample&	GetPixelSample()
    {
        return ( m_Data );
    }
    /** Get a count of data
     * \return A count of the samples on this pixel.
     */
    TqInt	DataSize()
    {
        return ( m_Data.m_Data.size() );
    }
    /** Get the maximum depth of this pixel
     * \return A float representing the maximum depth at this pixel.
     */
    TqFloat	MaxDepth()
    {
        return ( m_MaxDepth );
    }
    void	SetMaxDepth( TqFloat d )
    {
        m_MaxDepth = d;
    }
    /** Get the minimum depth of this pixel
     * \return A float representing the minimum depth at this pixel.
     */
    TqFloat	MinDepth()
    {
        return ( m_MinDepth );
    }
    void	SetMinDepth( TqFloat d )
    {
        m_MinDepth = d;
    }
    /** Get the id of the occlusion box that covers this pixel
     * \return The covering occlusion box's id.
     */
    TqInt	OcclusionBoxId()
    {
        return ( m_OcclusionBoxId );
    }
    void	SetOcclusionBoxId( TqInt id )
    {
        m_OcclusionBoxId = id;
    }
    /** Mark this pixel as needing its min and max Z  values recalculating
    */
    void	MarkForZUpdate()
    {
        m_NeedsZUpdate = true;
    }
    bool	NeedsZUpdating()
    {
        return m_NeedsZUpdate;
    }
    /** Scan through all the samples to find the min and max z values
    */
    void	UpdateZValues();

    /** Clear all sample information from this pixel.
     */
    void	Clear();
    /** Get a reference to the array of values for the specified sample.
     * \param m The horizontal index of the required sample point.
     * \param n The vertical index of the required sample point.
     * \return A Reference to a vector of SqImageSample data.
     */
    std::vector<SqImageSample>&	Values( TqInt index )
    {
        assert( index < m_XSamples*m_YSamples );
        return ( m_aValues[ index ] );
    }
    void	Combine();

    /** Get the sample data for the specified sample index.
     * \param The index of the required sample point.
     * \return A reference to the sample data.
     */
    const SqSampleData& SampleData( TqInt index )
    {
        assert( index < m_XSamples*m_YSamples );
        return ( m_Samples[index] );
    }

	/** Get the index of the sample that contains a dof offset that lies
	 *  in bounding-box number i.
	 * \param The index of the bounding box in question.
	 * \return The index of the sample that contains a dof offset in said bb.
	 */
	TqInt GetDofOffsetIndex(TqInt i)
	{
		return m_DofOffsetIndices[i];
	}

	/** Convert a coord in the unit square to one inside the unit circle.
	 *  used in generating dof sample positions.
	 */
	static void ProjectToCircle(CqVector2D& pos)
	{
		TqFloat r = pos.Magnitude();
		if( r == 0.0 )
			return;
			
		TqFloat adj = MAX(fabs(pos.x()), fabs(pos.y())) / r;
		pos.x(pos.x() * adj);
		pos.y(pos.y() * adj);
	}

private:
    TqInt	m_XSamples;						///< The number of samples in the horizontal direction.
    TqInt	m_YSamples;						///< The number of samples in the vertical direction.
    std::vector<std::vector<SqImageSample> > m_aValues;	///< Vector of vectors of sample point data.
    std::vector<SqSampleData> m_Samples;	///< A Vector of samples. Holds position, time, dof offset etc for each sample.
	std::vector<TqInt> m_DofOffsetIndices;	///< A mapping from dof bounding-box index to the sample that contains a dof offset in that bb.
    SqImageSample	m_Data;
    TqFloat m_MaxDepth;						///< The maximum depth of any sample in this pixel. used for occlusion culling
    TqFloat m_MinDepth;						///< The minimum depth of any sample in this pixel. used for occlusion culling
    TqInt m_OcclusionBoxId;					///< The CqOcclusionBox that covers this pixel
    TqBool m_NeedsZUpdate;					///< Whether or not the min/max depth values are up to date.
}
;




//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

//}  // End of #ifdef IMAGEPIXEL_H_INCLUDED
#endif
