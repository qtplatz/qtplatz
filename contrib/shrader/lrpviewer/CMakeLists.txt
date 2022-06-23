cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)

find_package( qwt )
include_directories(
	"${PROJECT_SOURCE_DIR}/src/libs"
	${QWT_INCLUDE_DIR}
	${QtPrintSupport_INCLUDE_DIRS}
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
	${QtPrintSupport_INCLUDE_DIRS}
	)

link_directories( ${QWT}/lib )

#find_package(Qt${QT_VERSION_MAJOR} CONFIG REQUIRED Widgets Gui Svg PrintSupport )

add_executable( lrpviewer ${lrpviewer_SOURCES} ${Resources} )

target_link_libraries( lrpviewer LINK_PUBLIC
                       lrpfile
		       adportable
		       adplot
		       adwidgets
		       ${QWT_LIBRARIES}
		       Qt::Widgets
		       Qt::Svg
		       Qt::PrintSupport
		       Qt::Widgets
		       ${Boost_LIBRARIES} )
