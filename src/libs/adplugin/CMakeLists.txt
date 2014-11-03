#
# CMakeLists for adplugin shared library
#
cmake_policy(SET CMP0022 NEW)

set(Boost_USE_STATIC_LIBS ON)

find_package(Boost 1.56 COMPONENTS date_time system regex)
find_package(Qt5 5.3.2 CONFIG REQUIRED Widgets Core )
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

add_definitions(-DADPLUGIN_LIBRARY)

include_directories( ${Qt5Core_INCLUDE_DIRS}
		     ${Qt5Widgets_INCLUDE_DIRS} )

add_library( adplugin SHARED
    lifecycle.cpp
    manager.cpp
    lifecycleaccessor.cpp
    loader.cpp
    plugin.cpp
    orbfactory.cpp
    orbbroker.cpp
    orbservant.cpp
    plugin_ptr.cpp
    visitor.cpp
    widget_factory.cpp
)

#target_include_directories( adcontrols PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

target_link_libraries( adplugin LINK_PUBLIC
                                ${Boost_LIBRARIES}
								adportable
								qtwrapper
								xmlparser
								adlog
								Qt5::Widgets )

