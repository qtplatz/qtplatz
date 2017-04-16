
project(adplot)

cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package( Qt5 CONFIG REQUIRED PrintSupport Svg Core Widgets Gui )
find_package( qwt )

include_directories(
  ${QWT_INCLUDE_DIRS}
  ${Qt5Svg_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
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
  axis.cpp
  axis.hpp 
  baseline.cpp
  baseline.hpp
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
)

add_library( adplot SHARED ${adplot_SOURCES} )
set_property( TARGET adplot PROPERTY CXX_STANDARD 14 )  # for std::make_unique on Mac

target_link_libraries( adplot LINK_PUBLIC
  adcontrols
  adportable
  qtwrapper
  ${QWT_LIBRARIES}
  Qt5::Svg
  Qt5::PrintSupport
  Qt5::Widgets
  Qt5::Core
  )

if( APPLE )
  install( TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin COMPONENT applications
    LIBRARY DESTINATION Contents/PlugIns COMPONENT libraries
    ARCHIVE DESTINATION Contents/PlugIns COMPONENT libraries )
else()
  install( TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin COMPONENT applications
    LIBRARY DESTINATION lib/qtplatz COMPONENT runtime_libraries
    ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
endif()
