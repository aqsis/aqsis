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

#include	<vector>

#include	"specific.h"

#include	"bitvector.h"
#include	"micropolygon.h"
#include	"irenderer.h"
#include	"ri.h"
#include	"sstring.h"
#include	"scene.h"
#include	"surface.h"
#include	"color.h"
#include	"vector2d.h"

#define		_qShareName	CORE
#include	"share.h"

START_NAMESPACE(Aqsis)


//-----------------------------------------------------------------------
/** Structure representing the information at a sample point in the image.
 */

struct SqImageValue
{
				SqImageValue()	{}
				/** Data constructor
				 * \param col The color at this sample point.
				 * \param opac The opacity at this sample point.
				 * \param depth the depth of the sample point.
				 */
				SqImageValue(CqColor& col,
							 CqColor& opac=gColWhite,
							 TqFloat depth=FLT_MAX) : 
								m_colColor(col), m_colOpacity(opac), m_Depth(depth) {}

	CqColor	m_colColor;		///< The color at this sampel point.
	CqColor	m_colOpacity;		///< The opacity value of this sample point.
	TqFloat		m_Depth;			///< The depth of the sample point.
	TqFloat		m_Coverage;
};

//-----------------------------------------------------------------------
/** Storage class for all data relating to a single pixel in the image.
 */

class _qShareC	CqImageElement
{
	public:
	_qShareM					CqImageElement();
	_qShareM					CqImageElement(const CqImageElement& ieFrom);
	_qShareM	virtual			~CqImageElement();

								/** Get the number of horizontal samples in this pixel
								 * \return The number of samples as an integer.
								 */
	_qShareM	TqInt			XSamples() const	{return(m_XSamples);}
								/** Get the number of vertical samples in this pixel
								 * \return The number of samples as an integer.
								 */
	_qShareM	TqInt			YSamples() const	{return(m_YSamples);}
	_qShareM	void			AllocateSamples(TqInt XSamples, TqInt YSamples);
	_qShareM	void			InitialiseSamples(CqVector2D& vecPixel, TqBool fJitter=TqTrue);
								/** Get the approximate coverage of this pixel.
								 * \return Float fraction of the pixel covered.
								 */
	_qShareM	TqFloat			Coverage()			{return(m_Coverage);}
								/** Get the averaged color of this pixel
								 * \return A color representing the averaged color at this pixel.
								 * \attention Only call this after already calling Combine().
								 */
	_qShareM	CqColor&		Color()	{return(m_colColor);}
								/** Get the averaged depth of this pixel
								 * \return A float representing the averaged depth at this pixel.
								 * \attention Only call this after already calling Combine().
								 */
	_qShareM	TqFloat			Depth()		{return(m_Depth);}
								/** Clear all sample information from this pixel.
								 */
	_qShareM	void			Clear();
								/** Get a reference to the array of values for the specified sample.
								 * \param m The horizontal index of the required sample point.
								 * \param n The vertical index of the required sample point.
								 * \return A Reference to a vector of SqImageElement data.
								 */
	_qShareM	std::vector<SqImageValue>&	Values(TqInt m, TqInt n)	
											{
												assert(m<m_XSamples);
												assert(n<m_XSamples);
												return(m_aValues[n*m_XSamples+m]);
											}
	_qShareM	void			Combine();
								/** Get the 2D sample position of the specified sample index.
								 * \param m The horizontal index of the required sample point.
								 * \param n The vertical index of the required sample point.
								 * \return A 2d vector representing the sample position. 
								 *  The position is in pixels and fractions thereof indexed from 0,0 at the top left of the image.
								 */
	_qShareM	CqVector2D&		SamplePoint(TqInt m, TqInt n)
													{
														assert(m<m_XSamples);
														assert(n<m_XSamples);
														return(m_avecSamples[n*m_XSamples+m]);
													}
								/** Get the frame time associated with the specified sample.
								 * \param m The horizontal index of the required sample point.
								 * \param n The vertical index of the required sample point.
								 * \return A float time between the shutter open and close times.
								 */
	_qShareM	TqFloat			SampleTime(TqInt m, TqInt n)
													{
														assert(m<m_XSamples);
														assert(n<m_XSamples);
														return(m_aTimes[n*m_XSamples+m]);
													}


	_qShareM	CqImageElement&	operator=(const CqImageElement& ieFrom);

	private:
			TqInt				m_XSamples;			///< The number of samples in the horizontal direction.
			TqInt				m_YSamples;			///< The number of samples in the vertical direction.
			std::vector<SqImageValue>* m_aValues;	///< Pointer to an array of vectors or sample point data.
			CqVector2D*			m_avecSamples;		///< Pointer to an array of sample positions.
			std::vector<TqFloat> m_aTimes;			///< A vector of float sample times for the sample points.
			TqFloat				m_Coverage;			///< The approximate coverage, just the ratio of sample hits to misses.
			CqColor			m_colColor;		///< The averaged color of this pixel.
			TqFloat				m_Depth;			///< The averaged depth of this pixel.
};


//-----------------------------------------------------------------------
/** The main image and related data, also responsible for processing the rendering loop.
 */

class _qShareC	CqImageBuffer
{
	public:
	_qShareM					CqImageBuffer()	:
												m_fDone(TqTrue),
												m_fQuit(TqFalse),
												m_iXRes(0),
												m_iYRes(0),
												m_cXBuckets(0),
												m_cYBuckets(0),
												m_XBucketSize(0),
												m_YBucketSize(0),
												m_PixelXSamples(0),
												m_PixelYSamples(0),
												m_FilterXWidth(0),
												m_FilterYWidth(0),
												m_aSurfaces(0),
												m_aScene(0),
												m_pieImage(0),
												m_CurrBucket(0),
												m_CropWindowXMin(0),
												m_CropWindowYMin(0),
												m_CropWindowXMax(0),
												m_CropWindowYMax(0),
												m_DisplayMode(ModeRGB)
												{}
	_qShareM	virtual			~CqImageBuffer();

	_qShareM			TqInt	Bucket(TqInt X, TqInt Y, TqInt& Xb, TqInt& Yb) const;
	_qShareM			TqInt	Bucket(TqInt X, TqInt Y) const;
	_qShareM			CqVector2D	Position(TqInt iBucket) const;
	_qShareM			CqVector2D	Size(TqInt iBucket) const;

								/** Get the horizontal resolution of this image.
								 * \return Integer horizontal resolution.
								 */
	_qShareM			TqInt	iXRes() const			{return(m_iXRes);}
								/** Get the vertical resolution of this image.
								 * \return Integer vertical resolution.
								 */
	_qShareM			TqInt	iYRes() const			{return(m_iYRes);}
								/** Get the minimum horizontal pixel to render.
								 * \return Integer minimum pixel index.
								 */
	_qShareM			TqInt	CropWindowXMin() const	{return(m_CropWindowXMin);}
								/** Get the minimum vertical pixel to render.
								 * \return Integer minimum pixel index.
								 */
	_qShareM			TqInt	CropWindowYMin() const	{return(m_CropWindowYMin);}
								/** Get the maximum horizontal pixel to render.
								 * \return Integer maximum pixel index.
								 */
	_qShareM			TqInt	CropWindowXMax() const	{return(m_CropWindowXMax);}
								/** Get the maximum vertical pixel to render.
								 * \return Integer maximum pixel index.
								 */
	_qShareM			TqInt	CropWindowYMax() const	{return(m_CropWindowYMax);}
								/** Get the number of buckets in the horizontal direction.
								 * \return Integer horizontal bucket count.
								 */
	_qShareM			TqInt	cXBuckets() const		{return(m_cXBuckets);}
								/** Get the number of buckets in the vertical direction.
								 * \return Integer vertical bucket count.
								 */
	_qShareM			TqInt	cYBuckets() const		{return(m_cYBuckets);}
								/** Get the horizontal bucket size.
								 * \return Integer horizontal bucket size.
								 */
	_qShareM			TqInt	XBucketSize() const		{return(m_XBucketSize);}
								/** Get the vertical bucket size.
								 * \return Integer vertical bucket size.
								 */
	_qShareM			TqInt	YBucketSize() const		{return(m_YBucketSize);}
								/** Get the number of horizontal samples per pixel.
								 * \return Integer sample count.
								 */
	_qShareM			TqInt	PixelXSamples() const	{return(m_PixelXSamples);}
								/** Get the number of vertical samples per pixel.
								 * \return Integer sample count.
								 */
	_qShareM			TqInt	PixelYSamples() const	{return(m_PixelYSamples);}
								/** Get the width of the pixel filter in the horizontal direction.
								 * \return Integer filter width, in pixels.
								 */
	_qShareM			TqInt	FilterXWidth() const	{return(m_FilterXWidth);}
								/** Get the width of the pixel filter in the vertical direction.
								 * \return Integer filter width, in pixels.
								 */
	_qShareM			TqInt	FilterYWidth() const	{return(m_FilterYWidth);}
								/** Get the display.
								 * \return Integer display mode as a member of enum Mode.
								 */
	_qShareM			TqInt	DisplayMode() const		{return(m_DisplayMode);}

	_qShareM			void	ClearBucket();
	_qShareM			void	DeleteImage();
	_qShareM			void	SaveImage(const char* strName);

	_qShareM			TqBool	Pixel(TqInt iXPos, TqInt iYPos, TqInt iBucket, CqImageElement*& pie);
	_qShareM			void	FilterPixel(TqInt X, TqInt Y, TqInt iBucket,SqImageValue& val);
	_qShareM			void	ExposePixel(SqImageValue& Pixel);
	_qShareM			void	QuantizePixel(SqImageValue& Pixel);

	_qShareM			void	AddSurfacePointer(CqBasicSurface* pSurface);
	_qShareM			void	PostSurface(CqBasicSurface* pSurface);
	_qShareM			TqBool	CullSurface(CqBound& Bound, CqBasicSurface* pSurface);
	_qShareM			void	InitialiseSurfaces(CqScene& Scene);
	_qShareM			void	AddMPG(CqMicroPolygonBase* pmpgNew);
	_qShareM			void	AddGrid(CqMicroPolyGridBase* pgridNew);
	_qShareM			void	RenderMPGs(TqInt iBucket, long xmin, long xmax, long ymin, long ymax);
	_qShareM			void	RenderMicroPoly(CqMicroPolygonBase* pMPG, TqInt iBucket, long xmin, long xmax, long ymin, long ymax);
	_qShareM			void	RenderSurfaces(TqInt iBucket, long xmin, long xmax, long ymin, long ymax);
	_qShareM			void	RenderImage();
								/** Get comlpetion status of this rendered image.
								 * \return bool indicating finished or not.
								 */
	_qShareM			TqBool	fDone() const		{return(m_fDone);}

	_qShareM	virtual	void	SetImage();
	_qShareM	virtual	void	AllocateSamples();
	_qShareM	virtual	void	InitialiseSamples(TqInt iBucket);
	_qShareM	virtual	void	Quit();
	_qShareM	virtual	void	Release();


	// Callbacks to overridden image buffer class to allow display/processing etc.
	_qShareM	virtual	void	BucketComplete(TqInt iBucket)	{}
	_qShareM	virtual	void	ImageComplete()					{}

	private:
			TqBool				m_fQuit;			///< Set by system if a quit has been requested.
			TqBool				m_fDone;			///< Set when the render of this image has completed.

			TqInt				m_iXRes;			///< Integer horizontal image resolution.
			TqInt				m_iYRes;			///< Integer vertical image resolution.
			TqInt				m_cXBuckets;		///< Integer horizontal bucket count.
			TqInt				m_cYBuckets;		///< Integer vertical bucket count.
			TqInt				m_XBucketSize;		///< Integer horizontal bucket size.
			TqInt				m_YBucketSize;		///< Imteger vertical bucket count.
			TqInt				m_PixelXSamples;	///< Integer horizontal sample per pixel count.
			TqInt				m_PixelYSamples;	///< Integer vertical sample per pixel count.
			TqInt				m_FilterXWidth;		///< Integer horizontal pixel filter width in pixels.
			TqInt				m_FilterYWidth;		///< Integer vertical pixel filter width in pixels.
			TqInt				m_CropWindowXMin;	///< Integer minimum horizontal pixel to render.
			TqInt				m_CropWindowYMin;	///< Integer minimum vertical pixel to render.
			TqInt				m_CropWindowXMax;	///< Integer maximum horizontal pixel to render.
			TqInt				m_CropWindowYMax;	///< Integer maximum vertical pixel to render.
			TqInt				m_DisplayMode;		///< Integer display mode, a member of the enum Mode.

			TqInt				m_CurrBucket;		///< Running currently being processed buckt index.

			CqImageElement*	m_pieImage;				///< Array of image element classes repesenting the pixels of the current bucket.
			std::vector<std::vector<CqMicroPolygonBase*> >	m_aampgWaiting;		///< Vector of vectors of waiting micropolygons in each bucket
			std::vector<std::vector<CqMicroPolyGridBase*> >	m_aagridWaiting;	///< Vector of vectors of waiting micropolygrids in each bucket
			std::vector<CqBasicSurface*>*	m_aScene;							///< Vector of lists of scene surfaces for each bucket.
			CqList<CqBasicSurface>*	m_aSurfaces;		///< Vector of lists of split surfaces for each bucket.
			std::vector<std::vector<TqFloat> >	m_aaFilterValues;	///< Vector of vector of filter weights precalculated fro each jittered sample point in each pixel.
};


//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

//}  // End of #ifdef IMAGEBUFFER_H_INCLUDED
#endif
