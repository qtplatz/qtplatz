
project(adplot)

cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

#find_package( Qt${QT_VERSION_MAJOR} CONFIG REQUIRED PrintSupport Svg Core Widgets Gui )
find_package( qwt )

include_directories(
  ${QWT_INCLUDE_DIRS}
  ${QtSvg_INCLUDE_DIRS}
  ${QtWidgets_INCLUDE_DIRS}
  )

add_definitions(-DADPLOT_LIBRARY)
if ( MSVC )
  add_definitions(-D_SCL_SECURE_NO_WARNINGS -wd4996 )
endif()

set( adplot_SOURCES
  annotation.cpp
  annotation.hpp
  annotations.cpp
  annotations.hpp
  axisextent.cpp
  axisextent.hpp
  baseline.cpp
  baseline.hpp
  boundingrect.cpp boundingrect.hpp
  chartview.cpp
  chartview.hpp
  chromatogramwidget.cpp
  chromatogramwidget.hpp
  plot.cpp
  plot.hpp
  peak.cpp
  peak.hpp
  plotcurve.cpp
  plotcurve.hpp
  seriesdata.cpp
  seriesdata.hpp
  spectrumwidget.cpp
  spectrumwidget.hpp
  tracewidget.cpp
  tracewidget.hpp
  trace.cpp
  trace.hpp
  traces.cpp
  traces.hpp
  zoomer.cpp
  zoomer.hpp
  picker.cpp
  picker.hpp
  panner.cpp
  panner.hpp
  rangemarker.cpp
  rangemarker.hpp
  spanmarker.cpp
  spanmarker.hpp
  spectrogramwidget.cpp
  spectrogramwidget.hpp
  spectrogramdata.cpp
  spectrogramdata.hpp
  peakmarker.cpp
  peakmarker.hpp
  plot_stderror.cpp
  plot_stderror.hpp
  adplotcurve.cpp
  adplotcurve.hpp
  timingchart.cpp
  timingchart.hpp
  xyseriesdata.cpp
  xyseriesdata.hpp
  # ${CMAKE_SOURCE_DIR}/../qwt-6.1/src/qwt_plot_axis.cpp
)

add_library( adplot SHARED ${adplot_SOURCES} )

target_link_libraries( adplot LINK_PUBLIC
  adcontrols
  adportable
  qtwrapper
  ${QWT_LIBRARIES}
  Qt::Svg
  Qt::PrintSupport
  Qt::Widgets
  Qt::Core
  )

runtime_install_path( _runtime_destination ${PROJECT_NAME} )
library_install_path( _library_destination ${PROJECT_NAME} )
archive_install_path( _archive_destination ${PROJECT_NAME} )

if( APPLE )
  install( TARGETS ${PROJECT_NAME}
    EXPORT ${QTPLATZ_TARGETS}
    RUNTIME DESTINATION ${_runtime_destination} COMPONENT applications
    LIBRARY DESTINATION Contents/PlugIns COMPONENT libraries
    ARCHIVE DESTINATION Contents/PlugIns COMPONENT libraries )
else()
  install( TARGETS ${PROJECT_NAME}
    EXPORT ${QTPLATZ_TARGETS}
    RUNTIME DESTINATION ${_runtime_destination} COMPONENT applications
    LIBRARY DESTINATION ${_library_destination} COMPONENT runtime_libraries
    ARCHIVE DESTINATION ${_archive_destination} COMPONENT libraries
    )
endif()
