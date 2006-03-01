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
#include    "imagepixel.h"
#include    "bucket.h"
#include	"kdtree.h"
#include	"occlusion.h"
#include	"clippingvolume.h"

START_NAMESPACE( Aqsis )


// This struct is used to hold info about a mpg that is used when rendering the mpg.
// It caches the info for use by multiple samples.
struct SqMpgSampleInfo
{
	CqColor		m_Colour;
	CqColor		m_Opacity;
	TqBool		m_Occludes;		// whether the opacity is full.
	TqBool		m_IsOpaque;		// whether the mpg can use the faster StoreOpaqueSample routine that assumes a few things.
};

// This struct holds info about a grid that can be cached and used for all its mpgs.
struct SqGridInfo
{
	TqFloat			m_ShadingRate;
	TqFloat			m_ShutterOpenTime;
	TqFloat			m_ShutterCloseTime;
	const TqFloat*	        m_LodBounds;
	TqBool			m_IsMatte;
	TqBool			m_IsCullable;
	TqBool			m_UsesDataMap;
};

// Enumeration of the type of rendering order of the buckets (experimental)
enum EqBucketOrder {
        Bucket_Horizontal = 0,
        Bucket_Vertical,
        Bucket_ZigZag,
        Bucket_Circle,
        Bucket_Random
};


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
 
  \see CqBucket, CqSurface, CqRenderer
 */

class CqImageBuffer
{
	public:
		CqImageBuffer() :
				m_fQuit( TqFalse ),
				m_fDone( TqTrue ),
				m_iXRes( 0 ),
				m_iYRes( 0 ),
				m_cXBuckets( 0 ),
				m_cYBuckets( 0 ),
				m_XBucketSize( 0 ),
				m_YBucketSize( 0 ),
				m_PixelXSamples( 0 ),
				m_PixelYSamples( 0 ),
				m_FilterXWidth( 0 ),
				m_FilterYWidth( 0 ),
				m_CropWindowXMin( 0 ),
				m_CropWindowYMin( 0 ),
				m_CropWindowXMax( 0 ),
				m_CropWindowYMax( 0 ),
				m_DisplayMode( ModeRGB ),
				m_CurrentBucketCol( 0 ),
				m_CurrentBucketRow( 0 ),
				m_MaxEyeSplits(10)
		{}
		virtual	~CqImageBuffer();

		CqVector2D	BucketPosition() const;
		CqVector2D	BucketPosition(TqInt x, TqInt y) const;
		CqVector2D	BucketSize() const;
		CqVector2D	BucketSize( TqInt x, TqInt y) const;

		/** Get the horizontal resolution of this image.
		 * \return Integer horizontal resolution.
		 */
		TqInt	iXRes() const
		{
			return ( m_iXRes );
		}
		/** Get the vertical resolution of this image.
		 * \return Integer vertical resolution.
		 */
		TqInt	iYRes() const
		{
			return ( m_iYRes );
		}
		/** Get the minimum horizontal pixel to render.
		 * \return Integer minimum pixel index.
		 */
		TqInt	CropWindowXMin() const
		{
			return ( m_CropWindowXMin );
		}
		/** Get the minimum vertical pixel to render.
		 * \return Integer minimum pixel index.
		 */
		TqInt	CropWindowYMin() const
		{
			return ( m_CropWindowYMin );
		}
		/** Get the maximum horizontal pixel to render.
		 * \return Integer maximum pixel index.
		 */
		TqInt	CropWindowXMax() const
		{
			return ( m_CropWindowXMax );
		}
		/** Get the maximum vertical pixel to render.
		 * \return Integer maximum pixel index.
		 */
		TqInt	CropWindowYMax() const
		{
			return ( m_CropWindowYMax );
		}
		/** Get the number of buckets in the horizontal direction.
		 * \return Integer horizontal bucket count.
		 */
		TqInt	cXBuckets() const
		{
			return ( m_cXBuckets );
		}
		/** Get the number of buckets in the vertical direction.
		 * \return Integer vertical bucket count.
		 */
		TqInt	cYBuckets() const
		{
			return ( m_cYBuckets );
		}
		/** Get the horizontal bucket size.
		 * \return Integer horizontal bucket size.
		 */
		TqInt	XBucketSize() const
		{
			return ( m_XBucketSize );
		}
		/** Get the vertical bucket size.
		 * \return Integer vertical bucket size.
		 */
		TqInt	YBucketSize() const
		{
			return ( m_YBucketSize );
		}
		/** Get the number of horizontal samples per pixel.
		 * \return Integer sample count.
		 */
		TqInt	PixelXSamples() const
		{
			return ( m_PixelXSamples );
		}
		/** Get the number of vertical samples per pixel.
		 * \return Integer sample count.
		 */
		TqInt	PixelYSamples() const
		{
			return ( m_PixelYSamples );
		}
		/** Get the width of the pixel filter in the horizontal direction.
		 * \return Integer filter width, in pixels.
		 */
		TqFloat	FilterXWidth() const
		{
			return ( m_FilterXWidth );
		}
		/** Get the width of the pixel filter in the vertical direction.
		 * \return Integer filter width, in pixels.
		 */
		TqFloat	FilterYWidth() const
		{
			return ( m_FilterYWidth );
		}
		/** Get the near clipping distance.
		 * \return Float distance from the camera that objects must be to be visible.
		 */
		TqFloat	ClippingNear() const
		{
			return ( m_ClippingNear );
		}
		/** Get the far clipping distance.
		 * \return Float distance from the camera that objects will be clipped from view.
		 */
		TqFloat	ClippingFar() const
		{
			return ( m_ClippingFar );
		}
		/** Get the display.
		 * \return Integer display mode as a member of enum Mode.
		 */
		TqInt	DisplayMode() const
		{
			return ( m_DisplayMode );
		}
		/** Get the column index of the bucket currently being processed.
		 * \return Integer bucket index.
		 */
		TqInt	CurrentBucketCol() const
		{
			return ( m_CurrentBucketCol );
		}
		/** Get the row index of the bucket currently being processed.
		 * \return Integer bucket index.
		 */
		TqInt	CurrentBucketRow() const
		{
			return ( m_CurrentBucketRow );
		}
		/** Move to the next bucket to process.
		 */
    		TqBool NextBucket(EqBucketOrder order);

		/** Get a pointer to the current bucket
		 */
		CqBucket& CurrentBucket()
		{
			return( m_Buckets[CurrentBucketRow()][CurrentBucketCol()] );
		}
		/** Get a pointer to the bucket at position x,y in the grid.
		 */
		CqBucket& Bucket( TqInt x, TqInt y)
		{
			return( m_Buckets[y][x] );
		}

		void	DeleteImage();
		void	SaveImage( const char* strName );

		void	PostSurface( const boost::shared_ptr<CqSurface>& pSurface );
		TqBool	CullSurface( CqBound& Bound, const boost::shared_ptr<CqSurface>& pSurface );
		TqBool	OcclusionCullSurface( const boost::shared_ptr<CqSurface>& pSurface );
		void	AddMPG( CqMicroPolygon* pmpgNew );
		TqBool	PushMPGForward( CqMicroPolygon* pmpg, TqInt Col, TqInt Row );
		TqBool	PushMPGDown( CqMicroPolygon*, TqInt Col, TqInt Row );
		void	RenderMPGs( long xmin, long xmax, long ymin, long ymax );
		void	RenderMicroPoly( CqMicroPolygon* pMPG, long xmin, long xmax, long ymin, long ymax );
		void	RenderSurfaces( long xmin, long xmax, long ymin, long ymax, TqBool fImager, enum EqFilterDepth filterdepth, CqColor zThreshold );
		void	RenderImage();
		void	StoreExtraData( CqMicroPolygon* pMPG, SqImageSample& sample);

		void	RenderMPG_MBOrDof( CqMicroPolygon* pMPG, long xmin, long xmax, long ymin, long ymax, TqBool IsMoving, TqBool UsingDof );
		void	RenderMPG_Static( CqMicroPolygon* pMPG, long xmin, long xmax, long ymin, long ymax );



		/** Get completion status of this rendered image.
		    * \return bool indicating finished or not.
		    */
		TqBool	fDone() const
		{
			return ( m_fDone );
		}

		virtual	void	SetImage();
		virtual	void	Quit();
		virtual	void	Release();

		// Callbacks to overridden image buffer class to allow display/processing etc.
		virtual	void	BucketComplete()
		{}
		virtual	void	ImageComplete()
		{}
		virtual	TqBool	IsCurrentBucketEmpty();


	private:
		TqBool	m_fQuit;			///< Set by system if a quit has been requested.
		TqBool	m_fDone;			///< Set when the render of this image has completed.

		TqInt	m_iXRes;			///< Integer horizontal image resolution.
		TqInt	m_iYRes;			///< Integer vertical image resolution.
		TqInt	m_cXBuckets;		///< Integer horizontal bucket count.
		TqInt	m_cYBuckets;		///< Integer vertical bucket count.
		TqInt	m_XBucketSize;		///< Integer horizontal bucket size.
		TqInt	m_YBucketSize;		///< Integer vertical bucket size.
		TqInt	m_PixelXSamples;	///< Integer horizontal sample per pixel count.
		TqInt	m_PixelYSamples;	///< Integer vertical sample per pixel count.
		TqFloat	m_FilterXWidth;		///< Integer horizontal pixel filter width in pixels.
		TqFloat	m_FilterYWidth;		///< Integer vertical pixel filter width in pixels.
		TqInt	m_CropWindowXMin;	///< Integer minimum horizontal pixel to render.
		TqInt	m_CropWindowYMin;	///< Integer minimum vertical pixel to render.
		TqInt	m_CropWindowXMax;	///< Integer maximum horizontal pixel to render.
		TqInt	m_CropWindowYMax;	///< Integer maximum vertical pixel to render.
		TqFloat	m_ClippingNear;		///< Near clipping distance.
		TqFloat	m_ClippingFar;		///< Far clipping distance.
		TqInt	m_DisplayMode;		///< Integer display mode, a member of the enum Mode.

		std::vector<std::vector<CqBucket> >	m_Buckets; ///< Array of bucket storage classes (row/col)
		TqInt	m_CurrentBucketCol;	///< Column index of the bucket currently being processed.
		TqInt	m_CurrentBucketRow;	///< Row index of the bucket currently being processed.
		TqInt	m_MaxEyeSplits;	        ///< Max Eye Splits by default 10

		SqMpgSampleInfo m_CurrentMpgSampleInfo;

		SqGridInfo m_CurrentGridInfo;

		void CacheGridInfo( CqMicroPolyGridBase* pGrid );
};



//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

//}  // End of #ifdef IMAGEBUFFER_H_INCLUDED
#endif


