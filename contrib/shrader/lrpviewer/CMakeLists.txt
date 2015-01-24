cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)

find_package( qwt )
include_directories(
	"${PROJECT_SOURCE_DIR}/src/libs"
	${QWT_INCLUDE_DIR}
	${Qt5PrintSupport_INCLUDE_DIRS}
	)

set( lrpviewer_SOURCES 
  main.cpp
  mainwindow.cpp
  mainwindow.hpp
  )

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

include_directories(
	"${PROJECT_SOURCE_DIR}/src/libs"
	${QWT_INCLUDE_DIRS}
	${Qt5PrintSupport_INCLUDE_DIRS}
	)

link_directories( ${QWT}/lib )

find_package(Qt5 CONFIG REQUIRED Widgets Gui Svg PrintSupport )

#find_package(Boost COMPONENTS program_options system )

add_executable( lrpviewer ${lrpviewer_SOURCES} ${Resources} )

target_link_libraries( lrpviewer LINK_PUBLIC
                       lrpfile
		       adportable
		       adplot
		       adwidgets
		       ${QWT_LIBRARIES}
		       Qt5::Widgets
		       Qt5::Svg
		       Qt5::PrintSupport
		       Qt5::Widgets
		       ${Boost_LIBRARIES} )

