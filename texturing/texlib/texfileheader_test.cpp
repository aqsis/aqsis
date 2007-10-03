#include "texfileheader.h"

BOOST_AUTO_TEST_CASE(CqTexFileHeader_findAttribute)
{
	Aqsis::CqTexFileHeader header;

	// add and read back an attribute via the findAttributePtr() interface
	header.setAttribute<TqInt>("asdf", 42);
	const TqInt* value = header.findAttributePtr<TqInt>("asdf");
	BOOST_REQUIRE(value != 0);
	BOOST_CHECK_EQUAL(*value, 42);

	BOOST_CHECK_EQUAL(header.findAttributePtr<TqInt>("not_present"), 0);
}

BOOST_AUTO_TEST_CASE(CqTexFileHeader_operator_idx)
{
	Aqsis::CqTexFileHeader header;

	// add and read back an attribute via the findAttribute() interface
	header.setAttribute<TqInt>("asdf", 42);
	BOOST_CHECK_EQUAL(header.findAttribute<TqInt>("asdf"), 42);

	// throw when accessing an attribute which isn't present.
	BOOST_CHECK_THROW(header.findAttribute("not_present"), Aqsis::XqInternal);
}

BOOST_AUTO_TEST_CASE(CqTexFileHeader_defaults)
{
	Aqsis::CqTexFileHeader header;

	// should throw, since we shouldn't be able to change the type of "width"
	// from int to float, & "width" is a default attribute.
	BOOST_CHECK_THROW(header.setAttribute<TqFloat>("width", 1.0f), Aqsis::XqInternal);

	// Check that the appropriate default channels are present.
}
