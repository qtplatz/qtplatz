#QT       += gui svg printsupport
#greaterThan( QT_MAJOR_VERSION, 4 ): QT += widgets
#TARGET = adplot
#TEMPLATE = lib
project(adplot)

set(QT_REQUIRED ON)
set(CMAKE_PREFIX_PATH /opt/Qt5.3.2/5.3/gcc_64)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(QWT_ROOT /usr/local/qwt-6.1.2-svn)

find_package(Qt5Widgets)

include_directories( ${Qt5Core_INCLUDES} ${QWT_ROOT}/include )

if (Qt5_POSITION_INDEPENDENT_CODE)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--export-all-symbols")
add_definitions(-DADPLOT_LIBRARY)

#LIBS += -l$$qtLibraryTarget(adcontrols) \
#        -l$$qtLibraryTarget(adportable) \
#        -l$$qtLibraryTarget(qtwrapper)

add_library( adplot
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
