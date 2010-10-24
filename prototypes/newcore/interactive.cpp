// Copyright (C) 2001, Paul C. Gregory and the other authors and contributors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the software's owners nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// (This is the New BSD license)

#include <iostream>
#include <fstream>
#include <cstdlib>

#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Double_Window.H>
#include <FL/fl_draw.H>

#include <aqsis/riutil/ricxxutil.h>

#include "api.h"
#include "displaymanager.h"

using namespace Aqsis;


//------------------------------------------------------------------------------
class FltkDisplay : public Aqsis::Display
{
    public:
        FltkDisplay(std::vector<uchar>& image)
            : m_image(image)
        { }

        virtual bool open(const std::string& fileName, const V2i& imageSize,
                          const V2i& tileSize, const VarSpec& varSpec)
        {
            m_imageSize = imageSize;
            m_tileSize = tileSize;
            return true;
        }

        virtual bool writeTile(const V2i& pos, void* data)
        {
            int nchans = 3;
            int tileRowSize = nchans*m_tileSize.x;
            // Clamp output tile size to extent of image.
            V2i outTileEnd = min(pos + m_tileSize, m_imageSize);
            V2i outTileSize = outTileEnd - pos;
            int outTileRowSize = sizeof(uchar)*nchans*outTileSize.x;
            // Copy data
            const uchar* src = (const uchar*)data;
            uchar* dest = &m_image[0] + nchans*(m_imageSize.x * pos.y + pos.x);
            for(int i = 0; i < outTileSize.y; ++i)
            {
                std::memcpy(dest, src, outTileRowSize);
                dest += nchans*m_imageSize.x;
                src += tileRowSize;
            }
            return true;
        }

        virtual bool close()
        {
            return true;
        }

    private:
        V2i m_imageSize;
        V2i m_tileSize;
        std::vector<uchar>& m_image;
};



//------------------------------------------------------------------------------
class InteractiveRender : public Fl_Widget
{
    public:
        InteractiveRender(int x, int y, V2i imageSize,
                          Ri::RendererServices& renderer)
            : Fl_Widget(x,y, imageSize.x,imageSize.y, 0),
            m_prev_x(0),
            m_prev_y(0),
            m_theta(0),
            m_phi(0),
            m_dist(5),
            m_imageSize(V2i(0)),
            m_image(),
            m_renderer(renderer),
            m_display(m_image)
        {
            setImageSize(imageSize);

            Ri::Renderer& ri = renderer.firstFilter();
            // Set up the display
            Display* disp = &m_display;
            ri.Display("Ci.tif", "__Display_instance__", "rgb",
                       ParamListBuilder()("int instance",
                                          reinterpret_cast<int*>(&disp)));
            // Kick off initial render
            renderImage();
        }

        virtual void draw()
        {
            fl_draw_image(&m_image[0], x(),y(), m_imageSize.x, m_imageSize.y);
        }

        virtual void resize(int x, int y, int w, int h)
        {
            setImageSize(V2i(w,h));
            renderImage();
            Fl_Widget::resize(x,y, w,h);
        }

        virtual int handle(int event)
        {
            switch(event)
            {
                case FL_FOCUS:
                    return 1;
                case FL_UNFOCUS:
                    return 1;
                case FL_KEYDOWN:
                    {
//                        int key = Fl::event_key();
//                        switch(key)
//                        {
//                            case '1':
//                        }
                    }
                    break;
                case FL_MOUSEWHEEL:
                    {
                        m_dist *= std::pow(0.9, -Fl::event_dy());
                        renderImage();
                        return 1;
                    }
                    break;
                case FL_PUSH:
                    {
                        m_prev_x = Fl::event_x();
                        m_prev_y = Fl::event_y();
                    }
                    return 1;
                case FL_DRAG:
                    {
                        m_theta += 0.5*(m_prev_x - Fl::event_x());
                        m_phi   += 0.5*(m_prev_y - Fl::event_y());
                        m_prev_x = Fl::event_x();
                        m_prev_y = Fl::event_y();
                        renderImage();
                        return 1;
                    }
                    break;
            }
            return Fl_Widget::handle(event);
        }

    private:
        void setImageSize(V2i imageSize)
        {
            m_imageSize = imageSize;
            m_image.assign(3*prod(m_imageSize), 0);
        }

        void renderImage()
        {
            Ri::Renderer& ri = m_renderer.firstFilter();

            ri.FrameBegin(1);

            ri.Format(m_imageSize.x, m_imageSize.y, 1);

            // Viewing transformation
            float fov = 90;
            ri.Projection("perspective",
                          ParamListBuilder()("float fov", &fov));
            ri.Translate(0, 0, m_dist);
            ri.Rotate(m_phi, 1, 0, 0);
            ri.Rotate(m_theta, 0, 1, 0);

            ri.WorldBegin();
            ri.ReadArchive("retained_model", 0, ParamListBuilder());
            ri.WorldEnd();
            ri.FrameEnd();

            damage(FL_DAMAGE_ALL);
        }

        int m_prev_x;
        int m_prev_y;

        float m_theta;
        float m_phi;
        float m_dist;
        V2i m_imageSize;
        std::vector<uchar> m_image;
        Ri::RendererServices& m_renderer;
        FltkDisplay m_display;
};


//------------------------------------------------------------------------------
class RenderWindow : public Fl_Double_Window
{
    public:
        RenderWindow(int w, int h, const char* title,
                     Ri::RendererServices& renderer)
            : Fl_Double_Window(w, h, title)
        {
            m_renderWidget = new InteractiveRender(x(), y(), V2i(w,h),
                                                   renderer);
            resizable(m_renderWidget);
            end();
        }

    private:
        InteractiveRender* m_renderWidget;
};


//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    typedef std::vector<std::string> StringVec;
    namespace po = boost::program_options;
    // optional options
    po::options_description optionsDesc("options");
    optionsDesc.add_options()
        ("help,h", "help message")
        ("stats,s", po::value<int>()->default_value(0),
         "frame statistics verbosity")
        ("threads,t", po::value<int>()->default_value(-1),
         "number of threads to use (-1 = auto)")
        ("opts", po::value<std::string>(),
         "frame render options")
    ;
    // options + positional parameters
    po::options_description allArgs("All arguments");
    allArgs.add(optionsDesc);
    allArgs.add_options()
        ("rib_files", po::value<StringVec>(), "RIB files to render")
    ;
    po::positional_options_description params;
    params.add("rib_files", -1);
    // Parse options
    po::variables_map opts;
    po::store(po::command_line_parser(argc, argv)
              .options(allArgs).positional(params).run(), opts);
    po::notify(opts);

    if(opts.count("help") || opts.count("rib_files") == 0)
    {
        std::cout
            << "Usage: " << argv[0] << " [options] model.rib\n\n"
            << optionsDesc;
        return 0;
    }

    boost::shared_ptr<Ri::RendererServices> renderer(createRenderer());

    // Command line options
    Ri::Renderer& ri = renderer->firstFilter();
    ri.Option("statistics", ParamListBuilder()
              ("int endofframe", &opts["stats"].as<int>()));
    ri.Option("limits", ParamListBuilder()
              ("int threads", &opts["threads"].as<int>()));

    // Open model file.
    const StringVec& ribFiles = opts["rib_files"].as<StringVec>();
    const char* modelFileName = ribFiles[0].c_str();
    std::ifstream modelFile(modelFileName, std::ios::in | std::ios::binary);
    if(!modelFile)
    {
        std::cerr << "Error - could not open \"" << modelFileName << "\"\n";
        return 0;
    }

    // Default hard-coded frame options.
    ri.PixelSamples(2, 2);
    ri.PixelFilter(renderer->getFilterFunc("sinc"), 3, 3);
    ri.ShadingRate(1);
    int eyesplits = 6;
    ri.Option("limits", ParamListBuilder()("int eyesplits", &eyesplits));

    if(opts.count("opts"))
    {
        const char* optFileName = opts["opts"].as<std::string>().c_str();
        std::ifstream optFile(optFileName, std::ios::in | std::ios::binary);
        if(!optFile)
        {
            std::cerr << "Error - could not open \"" << optFileName << "\"\n";
            return 0;
        }
        renderer->parseRib(optFile, optFileName);
    }

    ri.ArchiveBegin("retained_model", ParamListBuilder());
    renderer->parseRib(modelFile, modelFileName);
    ri.ArchiveEnd();

    Fl::lock();

    RenderWindow win(640, 480, "Aqsis-2.0 interactive demo", *renderer);
    win.show();
    while(Fl::wait());
//    {
//        if(Fl::thread_message())
//        {
//        }
//    }

    return 0;
}

