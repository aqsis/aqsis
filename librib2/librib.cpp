#include "librib.h"

#include <iostream>
#include <cassert>

extern int yyparse();

namespace librib
{

std::istream* ParseInputStream = &std::cin;
std::string ParseStreamName = "stdin";
RendermanInterface* ParseCallbackInterface = 0;
std::ostream* ParseErrorStream = &std::cerr;
unsigned int ParseLineNumber;
bool ParseSucceeded = true;

bool Parse(std::istream& InputStream, const std::string StreamName, RendermanInterface& CallbackInterface, std::ostream& ErrorStream)
{
	ParseInputStream = &InputStream;
	ParseStreamName = StreamName;
	ParseCallbackInterface = &CallbackInterface;
	ParseErrorStream = &ErrorStream;
	ParseLineNumber = 1;
	ParseSucceeded = true;
	
	yyparse();
	
	return ParseSucceeded;
}

void ResetParser()
{
	ParseInputStream = &std::cin;
	ParseStreamName = "stdin";
	ParseCallbackInterface = 0;
	ParseErrorStream = &std::cerr;
	ParseLineNumber = 1;
	ParseSucceeded = true;
}

}; // namespace librib

