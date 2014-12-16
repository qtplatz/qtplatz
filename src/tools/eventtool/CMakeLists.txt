
set( eventtool_SOURCES 
	    document.cpp
	    document.hpp
	    main.cpp
	    mainwindow.cpp
	    mainwindow.hpp
	    eventform.cpp
	    eventform.hpp
	    )

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

include_directories( ${QWT}/include )
link_directories( ${QWT}/lib )

find_package(Qt5 CONFIG REQUIRED Widgets Gui Svg PrintSupport )

find_package(Boost COMPONENTS program_options system )

add_executable( eventtool ${eventtool_SOURCES} ${Resources} )

target_link_libraries( eventtool LINK_PUBLIC
		       adportable
		       adwidgets
		       Qt5::Widgets
		       Qt5::Widgets
		       ${Boost_LIBRARIES} )

if( NOT WIN32 )
  target_link_libraries( eventtool LINK_PUBLIC 
    boost_system
    boost_program_options
    boost_thread
    boost_serialization
    boost_filesystem
    pthread )
endif()

