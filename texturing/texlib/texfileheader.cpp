#include "texfileheader.cpp"

CqTexFileHeader::addStandardAttributes()
{
	addAttribute<TqInt>("width", 0);
	addAttribute<TqInt>("height", 0);
	addAttribute<boost::shared_ptr<CqChannelList> >("channels",
			boost::shared_ptr<CqChannelList>(new CqChannelList()));
}
