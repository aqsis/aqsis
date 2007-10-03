#include "texfileheader.cpp"

CqTexFileHeader::addStandardAttributes()
{
	setAttribute<TqInt>("width", 100);
	setAttribute<TqInt>("height", 100);
	setAttribute<CqChannelList>("channels", CqChannelList());
}
