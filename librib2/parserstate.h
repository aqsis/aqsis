#ifndef PARSERSTATE_H
#define PATSERSTATE_H

#include <map>
#include "bdec.h"

namespace librib
{

/** Class for storing the state of the parser to be restored when processing
 *  DelayedReadArchive and friends. This is not pretty, ideally we would have
 *  a full OO parser/scanner, doing this in bison/flex is much ugglier. All of
 *  this mess can dissapear if migrated to a Boost/Spirit parser in future.
 */

class CqRIBParserState
{
	public:

		FILE *m_pParseInputFile;
		std::string m_ParseStreamName;
		CqRibBinaryDecoder *m_pBinaryDecoder;
		std::ostream* m_pParseErrorStream;

		RendermanInterface* m_pParseCallbackInterface;
		unsigned int m_ParseLineNumber;
		std::string m_ArchivePath;

		RtArchiveCallback m_pArchiveCallback;
		bool m_ParseSucceeded;
		bool m_fRequest;
		bool m_fParams;
		bool m_fRecovering;

		void *m_pYY_STATE;
};

/// Retrieve the current state of the parser internal variables
CqRIBParserState GetParserState();
/// Retrieve the current state of the parser internal variables
void SetParserState( CqRIBParserState& state );

}

#endif // PARSERSTATE_H
