
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

set( adplot_SOURCES 
  annotation.cpp 
  annotations.cpp 
  axis.cpp 
  baseline.cpp 
  chromatogramwidget.cpp 
  plot.cpp 
  peak.cpp 
  plotcurve.cpp 
  seriesdata.cpp 
  spectrumwidget.cpp 
  tracewidget.cpp 
  trace.cpp 
  traces.cpp 
  zoomer.cpp 
  picker.cpp 
  panner.cpp 
  spectrogramwidget.cpp 
  spectrogramdata.cpp 
  peakmarker.cpp 
  plot_stderror.cpp 
  adplotcurve.cpp 
  timingchart.cpp
)

add_library( adplot SHARED ${adplot_SOURCES} )

target_link_libraries( adplot LINK_PUBLIC
  adcontrols
  adportable
  qtwrapper
  Qt5::Widgets
  Qt5::Svg
  Qt5::PrintSupport
  ${QWT_LIBRARIES}
  )
