project(ImageViewer)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/cmake" )
find_package(QtCreator REQUIRED COMPONENTS Core)

find_package(Qt5 COMPONENTS SvgWidgets QUIET)
if (TARGET Qt5::SvgWidgets)
  set(SVG_WIDGETS Qt5::SvgWidgets)
endif()

add_qtc_plugin(ImageViewer
  DEPENDS OptionalSvg ${SVG_WIDGETS}
  PLUGIN_DEPENDS Core
  SOURCES
    exportdialog.cpp exportdialog.h
    imageview.cpp imageview.h
    imageviewer.cpp imageviewer.h
    imageviewerconstants.h
    imageviewerfile.cpp imageviewerfile.h
    imageviewerplugin.cpp imageviewerplugin.h
    multiexportdialog.cpp multiexportdialog.h
  EXPLICIT_MOC imageviewer.h
)

#target_link_libraries( ImageViewer LINK_PUBLIC adportable )

set_target_properties( ${PROJECT_NAME}
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}
  )
