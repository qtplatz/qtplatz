
cmake_policy(SET CMP0022 NEW)

set(Boost_USE_STATIC_LIBS ON)

find_package(Boost 1.56 COMPONENTS date_time system wserialization serialization)

add_library( xmlparser
	     pugiwrapper.cpp pugixml.cpp )


