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
		\brief Implements the default display devices for Aqsis.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#ifndef	___display_Loaded___
#define	___display_Loaded___

#include <aqsis.h>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Image.H>
#include <FL/fl_draw.H>

START_NAMESPACE( Aqsis )

/** FLTK Widget used to show a constantly updating image.
 *
 */
class Fl_FrameBuffer_Widget : public Fl_Widget 
{
public: 
	Fl_FrameBuffer_Widget(int x, int y, int imageW, int imageH, int depth, unsigned char* imageD) : Fl_Widget(x,y,imageW,imageH)
	{
		w = imageW;
		h = imageH;
		d = depth;
		image = imageD;
	}

	void draw(void) 
	{
		fl_draw_image(image,x(),y(),w,h,d,w*d); // draw image
	}
	
private:
	int w,h,d;
	unsigned char* image;
};

enum EqDisplayTypes 
{
	Type_File = 0,
	Type_Framebuffer,
	Type_ZFile,
	Type_ZFramebuffer,
	Type_Shadowmap,
};


struct SqDisplayInstance
{
	SqDisplayInstance() :
		m_ImageWidth(0),
		m_ImageHeight(0),
		m_PixelsProcessed(0),
		m_Channels(0),
		m_QuantizeZeroVal(0.0f),
		m_QuantizeOneVal(0.0f),
		m_QuantizeMinVal(0.0f),
		m_QuantizeMaxVal(0.0f),
		m_QuantizeDitherVal(0.0f),
		m_Compression(COMPRESSION_NONE), m_Quality(0),
		m_RenderWholeFrame(TqFalse),
		m_ImageType(Type_File),
		m_append(0),
		m_Data(0),
		m_theWindow(0),
		m_uiImageWidget(0),
		m_uiImage(0)
	{}
	TqInt		m_ImageWidth;
	TqInt		m_ImageHeight;
	TqInt		m_PixelsProcessed;
	TqInt		m_Channels;
	TqFloat		m_QuantizeZeroVal;
	TqFloat		m_QuantizeOneVal;
	TqFloat		m_QuantizeMinVal;
	TqFloat		m_QuantizeMaxVal;
	TqFloat		m_QuantizeDitherVal;
	uint16		m_Compression, m_Quality;
	TqBool		m_RenderWholeFrame;
	TqInt		m_ImageType;
	TqInt		m_append;
	TqFloat		m_matWorldToCamera[ 4 ][ 4 ];
	TqFloat		m_matWorldToScreen[ 4 ][ 4 ];

	void*		m_Data;

	Fl_Window*	m_theWindow;
	Fl_FrameBuffer_Widget* m_uiImageWidget;
	Fl_RGB_Image*	m_uiImage;
};


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	//	___display_Loaded___
