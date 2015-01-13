project( acquire )
cmake_minimum_required( VERSION 2.6 )
set ( CMAKE_BUILD_TYPE Release )
add_definitions ( -Wall )
find_package ( Qt4 REQUIRED )
include ( ${QT_USE_FILE} )
include_directories (
	${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
	${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR} ${QT_QTSVG_INCLUDE_DIR} ${QT_QTPRINTSUPPORT_INCLUDE_DIR}
	)
set ( acquire_HDRS
	acquire_global.h
	\
	)

set ( acquire_SRCS
	\
	)

set ( acquire_RSCS
	\
	)
QT4_ADD_RESOURCES(RSCS ${acquire_RSCS})

add_library ( acquire SHARED ${acquire_SRCS} ${UIS} ${RSCS} ${TRS} ${MOCS} )
target_link_libraries ( acquire  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTSVG_LIBRARY} ${QT_QTPRINTSUPPORT_LIBRARY} )
