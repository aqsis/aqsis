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

#ifndef	AQSIS_NO_FLTK
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Image.H>
#include <FL/fl_draw.H>
#endif // AQSIS_NO_LTK

START_NAMESPACE( Aqsis )

/** FLTK Widget used to show a constantly updating image.
 *
 */
#ifndef	AQSIS_NO_FLTK
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
#endif // AQSIS_NO_FLTK

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
			m_filename(0),
			m_width(0),
			m_height(0),
			m_iFormatCount(0),
			m_format(PkDspyUnsigned8),
			m_entrySize(0),
			m_lineLength(0),
			m_compression(COMPRESSION_NONE), m_quality(90),
			m_hostname(0),
			m_RenderWholeFrame(TqFalse),
			m_imageType(Type_File),
			m_append(0),
			m_data(0)
#ifndef	AQSIS_NO_FLTK
			,
			m_theWindow(0),
			m_uiImageWidget(0),
			m_uiImage(0)
#endif // AQSIS_NO_FLTK
	{}
	char*		m_filename;
	TqInt		m_width;
	TqInt		m_height;
	TqInt		m_OriginalSize[2];
	TqInt		m_origin[2];
	TqInt		m_iFormatCount;
	TqInt		m_format;
	TqInt		m_entrySize;
	TqInt		m_lineLength;
	uint16		m_compression, m_quality;
	char*		m_hostname;
	TqBool		m_RenderWholeFrame;
	TqInt		m_imageType;
	TqInt		m_append;
	TqFloat		m_matWorldToCamera[ 4 ][ 4 ];
	TqFloat		m_matWorldToScreen[ 4 ][ 4 ];

	void*		m_data;
	unsigned char*	m_zfbdata;

#ifndef	AQSIS_NO_FLTK

	Fl_Window*	m_theWindow;
	Fl_FrameBuffer_Widget* m_uiImageWidget;
	Fl_RGB_Image*	m_uiImage;
#endif // AQSIS_NO_FLTK
};


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	//	___display_Loaded___
