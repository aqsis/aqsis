#include "librib.h"
#include "bdec.h"

extern int yylex();
namespace librib
{
extern CqRibBinaryDecoder* BinaryDecoder;
};

int main(int argc, char* argv[])
{
	librib::BinaryDecoder = new librib::CqRibBinaryDecoder(stdin);
	while(yylex())
		{
		}
	
	delete(librib::BinaryDecoder);
	return 0;
}

