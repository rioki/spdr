// spdr - easy networking
// Copyright 2011 Sean Farrell

#include <iostream>
#include <sanity/trace.h>
#include <UnitTest++/UnitTest++.h>

int main()
{
    sanity::add_trace_target(sanity::DEBUG_TRACE, std::cerr);
	return UnitTest::RunAllTests();
}
