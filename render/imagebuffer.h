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
		\brief Declares the CqImageBuffer class responsible for rendering the primitives and storing the results.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is imagebuffer.h included already?
#ifndef IMAGEBUFFER_H_INCLUDED 
//{
#define IMAGEBUFFER_H_INCLUDED 1

#include	"aqsis.h"

#include	<vector>

#include	"bitvector.h"
#include	"micropolygon.h"
#include	"renderer.h"
#include	"ri.h"
#include	"sstring.h"
#include	"surface.h"
#include	"color.h"
#include	"vector2d.h"

#define		_qShareName	CORE
#include	"share.h"

START_NAMESPACE( Aqsis )


//-----------------------------------------------------------------------
/** Structure representing the information at a sample point in the image.
 */

class CqCSGTreeNode;

struct SqImageSample
{
	SqImageSample()
	{}
	/** Data constructor
	 * \param col The color at this sample point.
	 * \param opac The opacity at this sample point.
	 * \param depth the depth of the sample point.
	 */
	SqImageSample( CqColor& col,
	               CqColor& opac = gColWhite,
	               TqFloat depth = FLT_MAX ) :
			m_flags( 0 ), m_colColor( col ), m_colOpacity( opac ), m_Depth( depth )
	{}

	enum {
		Flag_Occludes = 0x0001,
		Flag_Matte    = 0x0002
	};

	TqInt   m_flags;
	CqColor	m_colColor;		///< The color at this sample point.
	CqColor	m_colOpacity;		///< The opacity value of this sample point.
	TqFloat	m_Depth;			///< The depth of the sample point.
	TqFloat	m_Coverage;
	CqCSGTreeNode*	m_pCSGNode;	///< Pointer to the CSG node this sample is part of, NULL if not part of a solid.
}
;

//-----------------------------------------------------------------------
/** Storage class for all data relating to a single pixel in the image.
 */

class _qShareC	CqImagePixel
{
	public:
		_qShareM	CqImagePixel();
		_qShareM	CqImagePixel( const CqImagePixel& ieFrom );
		_qShareM	virtual	~CqImagePixel();

		/** Get the number of horizontal samples in this pixel
		 * \return The number of samples as an integer.
		 */
		_qShareM	TqInt	XSamples() const
		{
			return ( m_XSamples );
		}
		/** Get the number of vertical samples in this pixel
		 * \return The number of samples as an integer.
		 */
		_qShareM	TqInt	YSamples() const
		{
			return ( m_YSamples );
		}
		_qShareM	void	AllocateSamples( TqInt XSamples, TqInt YSamples );
		_qShareM	void	InitialiseSamples( CqVector2D& vecPixel, TqBool fJitter = TqTrue );
		/** Get the approximate coverage of this pixel.
		 * \return Float fraction of the pixel covered.
		 */
		_qShareM	TqFloat	Coverage()
		{
			return ( m_Coverage );
		}
		_qShareM	void	SetCoverage( TqFloat c )
		{
			m_Coverage = c;
		}
		/** Get the averaged color of this pixel
		 * \return A color representing the averaged color at this pixel.
		 * \attention Only call this after already calling FilterBucket().
		 */
		_qShareM	CqColor&	Color()
		{
			return ( m_colColor );
		}
		/** Get the averaged opacity of this pixel
		 * \return A color representing the averaged opacity at this pixel.
		 * \attention Only call this after already calling FilterBucket().
		 */
		_qShareM	CqColor&	Opacity()
		{
			return ( m_colOpacity );
		}
		/** Get the averaged depth of this pixel
		 * \return A float representing the averaged depth at this pixel.
		 * \attention Only call this after already calling FilterBucket().
		 */
		_qShareM	TqFloat	Depth()
		{
			return ( m_Depth );
		}
		_qShareM	void	SetDepth( TqFloat d )
		{
			m_Depth = d;
		}
		/** Get the maximum depth of this pixel
		 * \return A float representing the maximum depth at this pixel.
		 */
		_qShareM	TqFloat	MaxDepth()
		{
			return ( m_MaxDepth );
		}
		_qShareM	void	SetMaxDepth( TqFloat d )
		{
			m_MaxDepth = d;
		}
		/** Get the minimum depth of this pixel
		 * \return A float representing the minimum depth at this pixel.
		 */
		_qShareM	TqFloat	MinDepth()
		{
			return ( m_MinDepth );
		}
		_qShareM	void	SetMinDepth( TqFloat d )
		{
			m_MinDepth = d;
		}
		/** Get the id of the occlusion box that covers this pixel
		 * \return The covering occlusion box's id.
		 */
		_qShareM	TqInt	OcclusionBoxId()
		{
			return ( m_OcclusionBoxId );
		}
		_qShareM	void	SetOcclusionBoxId( TqInt id )
		{
			m_OcclusionBoxId = id;
		}
		/** Mark this pixel as needing its min and max Z  values recalculating
		*/
		_qShareM	void	MarkForZUpdate()
		{
			m_NeedsZUpdate = true;
		}
		_qShareM	bool	NeedsZUpdating()
		{
			return m_NeedsZUpdate;
		}
		/** Scan through all the samples to find the min and max z values
		*/
		_qShareM	void	UpdateZValues();
		
		/** Clear all sample information from this pixel.
		 */
		_qShareM	void	Clear();
		/** Get a reference to the array of values for the specified sample.
		 * \param m The horizontal index of the required sample point.
		 * \param n The vertical index of the required sample point.
		 * \return A Reference to a vector of SqImageSample data.
		 */
		_qShareM	std::vector<SqImageSample>&	Values( TqInt m, TqInt n )
		{
			assert( m < m_XSamples );
			assert( n < m_YSamples );
			return ( m_aValues[ n * m_XSamples + m ] );
		}
		_qShareM	void	Combine();
		/** Get the 2D sample position of the specified sample index.
		 * \param m The horizontal index of the required sample point.
		 * \param n The vertical index of the required sample point.
		 * \return A 2d vector representing the sample position. 
		 *  The position is in pixels and fractions thereof indexed from 0,0 at the top left of the image.
		 */
		_qShareM	CqVector2D&	SamplePoint( TqInt m, TqInt n )
		{
			assert( m < m_XSamples );
			assert( n < m_YSamples );
			return ( m_avecSamples[ n * m_XSamples + m ] );
		}
		/** Get the filter weight index of the appropriate subcell.
		 * Subcell dimensions are inverted subpixel dimensions, producing a square subcell matrix.
		 * \param m The horizontal index of the required sample point.
		 * \param n The vertical index of the required sample point.
		 * \return The integer index of the subcell.
		 *  The position is in pixels and fractions thereof indexed from 0,0 at the top left of the image.
		 */
		_qShareM	TqInt	SubCellIndex( TqInt m, TqInt n )
		{
			assert( m < m_XSamples );
			assert( n < m_YSamples );
			return ( m_aSubCellIndex[ n * m_XSamples + m ] );
		}
		/** Get the frame time associated with the specified sample.
		 * \param m The horizontal index of the required sample point.
		 * \param n The vertical index of the required sample point.
		 * \return A float time between the shutter open and close times.
		 */
		_qShareM	TqFloat	SampleTime( TqInt m, TqInt n )
		{
			assert( m < m_XSamples );
			assert( n < m_YSamples );
			return ( m_aTimes[ n * m_XSamples + m ] );
		}
		/** Get the detail level associated with the specified sample.
		 * \param m The horizontal index of the required sample point.
		 * \param n The vertical index of the required sample point.
		 * \return A float detail level.
		 */
		_qShareM	TqFloat	SampleLevelOfDetail( TqInt m, TqInt n )
		{
			assert( m < m_XSamples );
			assert( n < m_YSamples );
			return ( m_aDetailLevels[ n * m_XSamples + m ] );
		}

		//	_qShareM	CqImagePixel&	operator=(const CqImagePixel& ieFrom);

	private:
		TqInt	m_XSamples;						///< The number of samples in the horizontal direction.
		TqInt	m_YSamples;						///< The number of samples in the vertical direction.
		std::vector<std::vector<SqImageSample> > m_aValues;	///< Vector of vectors of sample point data.
		std::vector<CqVector2D>	m_avecSamples;				///< Vector of sample positions.
		std::vector<TqInt>	m_aSubCellIndex;				///< Vector of subcell indices.
		std::vector<TqFloat> m_aTimes;						///< A vector of float sample times for the sample points.
		std::vector<TqFloat> m_aDetailLevels;					///< A vector of float level-of-detail samples for the sample points.
		TqFloat	m_Coverage;						///< The approximate coverage, just the ratio of sample hits to misses.
		CqColor	m_colColor;						///< The averaged color of this pixel.
		CqColor	m_colOpacity;					///< The averaged opacity of this pixel.
		TqFloat	m_Depth;						///< The averaged depth of this pixel.
		TqFloat m_MaxDepth;						///< The maximum depth of any sample in this pixel. used for occlusion culling
		TqFloat m_MinDepth;						///< The minimum depth of any sample in this pixel. used for occlusion culling
		TqInt m_OcclusionBoxId;					///< The CqOcclusionBox that covers this pixel
		TqBool m_NeedsZUpdate;					///< Whether or not the min/max depth values are up to date.
}
;

//-----------------------------------------------------------------------
/** Class holding data about a particular bucket.
 */

class CqBucket : public IqBucket
{
	public:
		CqBucket()
		{}
		CqBucket( const CqBucket& From )
		{
			*this = From;
		}
		virtual ~CqBucket()
		{}

		CqBucket& operator=( const CqBucket& From )
		{
			m_ampgWaiting = From.m_ampgWaiting;
			m_agridWaiting = From.m_agridWaiting;

			return ( *this );
		}

		// Overridden from IqBucket
		virtual	TqInt	XSize() const
		{
			return ( m_XSize );
		}
		virtual	TqInt	YSize() const
		{
			return ( m_YSize );
		}
		virtual	TqInt	XOrigin() const
		{
			return ( m_XOrigin );
		}
		virtual	TqInt	YOrigin() const
		{
			return ( m_YOrigin );
		}
		virtual	TqInt	XFWidth() const
		{
			return ( m_XFWidth );
		}
		virtual	TqInt	YFWidth() const
		{
			return ( m_YFWidth );
		}
		virtual	TqInt	XPixelSamples() const
		{
			return ( m_XPixelSamples );
		}
		virtual	TqInt	YPixelSamples() const
		{
			return ( m_YPixelSamples );
		}

		virtual	CqColor Color( TqInt iXPos, TqInt iYPos );
		virtual	CqColor Opacity( TqInt iXPos, TqInt iYPos );
		virtual	TqFloat Coverage( TqInt iXPos, TqInt iYPos );
		virtual	TqFloat Depth( TqInt iXPos, TqInt iYPos );
		virtual	TqFloat MaxDepth( TqInt iXPos, TqInt iYPos );

		static	void	InitialiseBucket( TqInt xorigin, TqInt yorigin, TqInt xsize, TqInt ysize, TqInt xfwidth, TqInt yfwidth, TqInt xsamples, TqInt ysamples, TqBool fJitter = TqTrue );
		static	void	InitialiseFilterValues();
		static	void	Clear();
		static	TqBool	ImageElement( TqInt iXPos, TqInt iYPos, CqImagePixel*& pie );
		static	void	CombineElements();
		void	FilterBucket();
		void	ExposeBucket();
		void	QuantizeBucket();

		/** Add a GPRim to the list of deferred GPrims.
		 */
		void	AddGPrim( CqBasicSurface* pGPrim );

		/** Add an MPG to the list of deferred MPGs.
		 */
		void	AddMPG( CqMicroPolygonBase* pmpgNew )
		{
			m_ampgWaiting.push_back( pmpgNew );
		}
		/** Add a Micropoly grid to the list of deferred grids.
		 */
		void	AddGrid( CqMicroPolyGridBase* pgridNew )
		{
			m_agridWaiting.push_back( pgridNew );
		}
		/** Get a pointer to the top GPrim in the list of deferred GPrims.
		 */
		CqBasicSurface* pTopSurface()
		{
			return ( m_aGPrims.pFirst() );
		}
		/** Get a reference to the vetor of deferred MPGs.
		 */
		std::vector<CqMicroPolygonBase*>& aMPGs()
		{
			return ( m_ampgWaiting );
		}
		/** Get a reference to the vetor of deferred grids.
		 */
		std::vector<CqMicroPolyGridBase*>& aGrids()
		{
			return ( m_agridWaiting );
		}

	private:
		static	TqInt	m_XSize;
		static	TqInt	m_YSize;
		static	TqInt	m_XFWidth;
		static	TqInt	m_YFWidth;
		static	TqInt	m_XMax;
		static	TqInt	m_YMax;
		static	TqInt	m_XPixelSamples;
		static	TqInt	m_YPixelSamples;
		static	TqInt	m_XOrigin;
		static	TqInt	m_YOrigin;
		static	std::vector<CqImagePixel>	m_aieImage;
		static	std::vector<TqFloat>	m_aFilterValues;				///< Vector of filter weights precalculated.

		std::vector<CqMicroPolygonBase*> m_ampgWaiting;			///< Vector of vectors of waiting micropolygons in this bucket
		std::vector<CqMicroPolyGridBase*> m_agridWaiting;		///< Vector of vectors of waiting micropolygrids in this bucket
		CqList<CqBasicSurface>	m_aGPrims;						///< Vector of lists of split surfaces for this bucket.
}
;


//-----------------------------------------------------------------------
/**
  The main image and related data, also responsible for processing the rendering loop.
 
  Before the image can be rendered the image buffer has to be initialised by calling the
  SetImage() method. The parameters for the creation of the buffer are read from the
  current options (this includes things like the image resolution, bucket size,
  number of pixel samples, etc.).
  
  After the buffer is initialized the surfaces (gprims) that are to be rendered 
  can be added to the buffer. This is done by calling PostSurface() for each gprim.
  (note: before calling this method the gprim has to be transformed into camera space!)
  All the gprims that can be culled at this point (i.e. CullSurface() returns true) 
  won't be stored inside the buffer. If a gprim can't be culled it is assigned to
  the first bucket that touches its bound.
 
  Once all the gprims are posted to the buffer the image can be rendered by calling
  RenderImage(). Now all buckets will be processed one after another. 
 
  \see CqBucket, CqBasicSurface, CqRenderer
 */

class _qShareC	CqImageBuffer
{
	public:
		_qShareM	CqImageBuffer() :
				m_fQuit( TqFalse ),
				m_fDone( TqTrue ),
				m_iXRes( 0 ),
				m_iYRes( 0 ),
				m_cXBuckets( 0 ),
				m_cYBuckets( 0 ),
				m_XBucketSize( 0 ),
				m_YBucketSize( 0 ),
				m_iCurrentBucket( 0 ),
				m_PixelXSamples( 0 ),
				m_PixelYSamples( 0 ),
				m_FilterXWidth( 0 ),
				m_FilterYWidth( 0 ),
				m_CropWindowXMin( 0 ),
				m_CropWindowYMin( 0 ),
				m_CropWindowXMax( 0 ),
				m_CropWindowYMax( 0 ),
				m_DisplayMode( ModeRGB )
		{}
		_qShareM	virtual	~CqImageBuffer();

		_qShareM	TqInt	Bucket( TqInt X, TqInt Y, TqInt& Xb, TqInt& Yb ) const;
		_qShareM	TqInt	Bucket( TqInt X, TqInt Y ) const;
		_qShareM	CqVector2D	Position( TqInt iBucket ) const;
		_qShareM	CqVector2D	Size( TqInt iBucket ) const;

		/** Get the horizontal resolution of this image.
		 * \return Integer horizontal resolution.
		 */
		_qShareM	TqInt	iXRes() const
		{
			return ( m_iXRes );
		}
		/** Get the vertical resolution of this image.
		 * \return Integer vertical resolution.
		 */
		_qShareM	TqInt	iYRes() const
		{
			return ( m_iYRes );
		}
		/** Get the minimum horizontal pixel to render.
		 * \return Integer minimum pixel index.
		 */
		_qShareM	TqInt	CropWindowXMin() const
		{
			return ( m_CropWindowXMin );
		}
		/** Get the minimum vertical pixel to render.
		 * \return Integer minimum pixel index.
		 */
		_qShareM	TqInt	CropWindowYMin() const
		{
			return ( m_CropWindowYMin );
		}
		/** Get the maximum horizontal pixel to render.
		 * \return Integer maximum pixel index.
		 */
		_qShareM	TqInt	CropWindowXMax() const
		{
			return ( m_CropWindowXMax );
		}
		/** Get the maximum vertical pixel to render.
		 * \return Integer maximum pixel index.
		 */
		_qShareM	TqInt	CropWindowYMax() const
		{
			return ( m_CropWindowYMax );
		}
		/** Get the number of buckets in the horizontal direction.
		 * \return Integer horizontal bucket count.
		 */
		_qShareM	TqInt	cXBuckets() const
		{
			return ( m_cXBuckets );
		}
		/** Get the number of buckets in the vertical direction.
		 * \return Integer vertical bucket count.
		 */
		_qShareM	TqInt	cYBuckets() const
		{
			return ( m_cYBuckets );
		}
		/** Get the horizontal bucket size.
		 * \return Integer horizontal bucket size.
		 */
		_qShareM	TqInt	XBucketSize() const
		{
			return ( m_XBucketSize );
		}
		/** Get the vertical bucket size.
		 * \return Integer vertical bucket size.
		 */
		_qShareM	TqInt	YBucketSize() const
		{
			return ( m_YBucketSize );
		}
		/** Get the number of horizontal samples per pixel.
		 * \return Integer sample count.
		 */
		_qShareM	TqInt	PixelXSamples() const
		{
			return ( m_PixelXSamples );
		}
		/** Get the number of vertical samples per pixel.
		 * \return Integer sample count.
		 */
		_qShareM	TqInt	PixelYSamples() const
		{
			return ( m_PixelYSamples );
		}
		/** Get the width of the pixel filter in the horizontal direction.
		 * \return Integer filter width, in pixels.
		 */
		_qShareM	TqInt	FilterXWidth() const
		{
			return ( m_FilterXWidth );
		}
		/** Get the width of the pixel filter in the vertical direction.
		 * \return Integer filter width, in pixels.
		 */
		_qShareM	TqInt	FilterYWidth() const
		{
			return ( m_FilterYWidth );
		}
		/** Get the near clipping distance.
		 * \return Float distance from the camera that objects must be to be visible.
		 */
		_qShareM	TqFloat	ClippingNear() const
		{
			return ( m_ClippingNear );
		}
		/** Get the far clipping distance.
		 * \return Float distance from the camera that objects will be clipped from view.
		 */
		_qShareM	TqFloat	ClippingFar() const
		{
			return ( m_ClippingFar );
		}
		/** Get the display.
		 * \return Integer display mode as a member of enum Mode.
		 */
		_qShareM	TqInt	DisplayMode() const
		{
			return ( m_DisplayMode );
		}
		/** Get the index of the bucket currently being processed.
		 * \return Integer bucket index.
		 */
		_qShareM	TqInt	iCurrentBucket() const
		{
			return ( m_iCurrentBucket );
		}
		/** Set the current bucket index.
		 * \param iBucket Integer index of the bucket being processed.
		 */
		_qShareM	void	SetiCurrentBucket( TqInt iBucket )
		{
			m_iCurrentBucket = iBucket;
		}

		_qShareM	void	DeleteImage();
		_qShareM	void	SaveImage( const char* strName );

		_qShareM	void	PostSurface( CqBasicSurface* pSurface );
		_qShareM	TqBool	CullSurface( CqBound& Bound, CqBasicSurface* pSurface );
		_qShareM	TqBool	OcclusionCullSurface( TqInt iBucket, CqBasicSurface* pSurface );
		_qShareM	void	AddMPG( CqMicroPolygonBase* pmpgNew );
		_qShareM	void	RenderMPGs( TqInt iBucket, long xmin, long xmax, long ymin, long ymax );
		_qShareM	void	RenderMicroPoly( CqMicroPolygonBase* pMPG, TqInt iBucket, long xmin, long xmax, long ymin, long ymax );
		_qShareM	void	RenderSurfaces( TqInt iBucket, long xmin, long xmax, long ymin, long ymax );
		_qShareM	void	RenderImage();
		/** Get completion status of this rendered image.
		 * \return bool indicating finished or not.
		 */
		_qShareM	TqBool	fDone() const
		{
			return ( m_fDone );
		}

		_qShareM	virtual	void	SetImage();
		_qShareM	virtual	void	Quit();
		_qShareM	virtual	void	Release();


		// Callbacks to overridden image buffer class to allow display/processing etc.
		_qShareM	virtual	void	BucketComplete( TqInt iBucket )
		{}
		_qShareM	virtual	void	ImageComplete()
		{}

	private:
		TqBool	m_fQuit;			///< Set by system if a quit has been requested.
		TqBool	m_fDone;			///< Set when the render of this image has completed.

		TqInt	m_iXRes;			///< Integer horizontal image resolution.
		TqInt	m_iYRes;			///< Integer vertical image resolution.
		TqInt	m_cXBuckets;		///< Integer horizontal bucket count.
		TqInt	m_cYBuckets;		///< Integer vertical bucket count.
		TqInt	m_XBucketSize;		///< Integer horizontal bucket size.
		TqInt	m_YBucketSize;		///< Integer vertical bucket size.
		TqInt	m_iCurrentBucket;	///< Index of the bucket currently being processed.
		TqInt	m_PixelXSamples;	///< Integer horizontal sample per pixel count.
		TqInt	m_PixelYSamples;	///< Integer vertical sample per pixel count.
		TqInt	m_FilterXWidth;		///< Integer horizontal pixel filter width in pixels.
		TqInt	m_FilterYWidth;		///< Integer vertical pixel filter width in pixels.
		TqInt	m_CropWindowXMin;	///< Integer minimum horizontal pixel to render.
		TqInt	m_CropWindowYMin;	///< Integer minimum vertical pixel to render.
		TqInt	m_CropWindowXMax;	///< Integer maximum horizontal pixel to render.
		TqInt	m_CropWindowYMax;	///< Integer maximum vertical pixel to render.
		TqFloat	m_ClippingNear;		///< Near clipping distance.
		TqFloat	m_ClippingFar;		///< Far clipping distance.
		TqInt	m_DisplayMode;		///< Integer display mode, a member of the enum Mode.

		std::vector<CqBucket>	m_aBuckets;
};


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

//}  // End of #ifdef IMAGEBUFFER_H_INCLUDED
#endif
