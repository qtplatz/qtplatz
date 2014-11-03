
project(adplot)

if ( WIN32 )
  set( CMAKE_PREFIX_PATH "C:/Qt/Qt5.3.2/5.3/msvc2013_64_opengl" )
  set( QWT "C:/Qwt-6.1.2-svn" )
else()
  set( CMAKE_PREFIX_PATH /opt/Qt5.3.2/5.3/gcc_64 )
  set( QWT "/opt/local/qwt-6.1.2-svn" )
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# Find the QtWidgets library
find_package(Qt5Widgets 5.3.2 CONFIG REQUIRED Core Widgets Gui Svg)

include_directories( ${QWT}/include
		      $ENV{QTDIR}/include/QtSvg
#		      ${Qt5Svg_INCLUDE_DIRS}
		      ${Qt5Widgets_INCLUDE_DIRS}
		      )
		     
link_directories( ${QWT}/lib )

#if (Qt5_POSITION_INDEPENDENT_CODE)
#  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
#endif()

if ( NOT WIN32 )
  set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--export-all-symbols")
endif()

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

set(LINK_LIBRARY optimized qwt debug qwtd )

add_library( adplot ${adplot_SOURCES} )
target_link_libraries( adplot LINK_PUBLIC adcontrols adportable qtwrapper ${LINK_LIBRARY} )
