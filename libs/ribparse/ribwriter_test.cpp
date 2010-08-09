#include <iostream>

#include <aqsis/util/logging.h>
#include <aqsis/util/logging_streambufs.h>

#include "ribwriter.h"
#include "ribsema.h"

using namespace Aqsis;


class ParamListBuilder
{
    private:
        std::vector<Ri::Param> m_paramStorage;

    public:
        template<typename T, size_t size>
        void push_back(const Ri::TypeSpec& spec, const char* name,
                       T (&data)[size])
        {
            m_paramStorage.push_back(
                    Ri::Param(spec, name, Ri::Array<T>(data, size)));
        }

        operator Ri::ParamList()
        {
            if(m_paramStorage.empty())
                return Ri::ParamList();
            return Ri::ParamList(&m_paramStorage[0], m_paramStorage.size());
        }
};


int main(int argc, char* argv[])
{
    bool useGzip = false;
    bool useBinary = false;
    bool interpolate = false;
    if(argc > 1)
    {
        char* c = argv[1];
        if(*c == '-')
        {
            ++c;
            while(*c)
            {
                switch(*c)
                {
                    case 'a':
                        interpolate = true;
                        break;
                    case 'b':
                        useBinary = true;
                        break;
                    case 'z':
                        useGzip = true;
                        break;
                }
                ++c;
            }
        }
    }

    // Colourization of errors:
    Aqsis::color_level_buf colorLevel(Aqsis::log());
    // Faster buffering, but log colourization then fails:
    //std::ios_base::sync_with_stdio(false);

    boost::shared_ptr<Ri::RendererServices> services =
        createRibWriter(std::cout, interpolate, useBinary, useGzip);
    // Drop frames
    ParamListBuilder frameDropParams;
    int desiredFrames[] = {1, 3, 4, 10};
    frameDropParams.push_back(Ri::TypeSpec(Ri::TypeSpec::Integer), "frames", desiredFrames);
    services->addFilter("framedrop", frameDropParams);
    // Validate interface
    services->addFilter("validate");

    services->parseRib(std::cin, "stdin", services->firstFilter());

    return 0;
}

// vi: set et:
