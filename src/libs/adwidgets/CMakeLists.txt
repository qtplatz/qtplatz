
project(adwidgets)

cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

#find_package( Qt5 REQUIRED PrintSupport Svg Widgets Gui Core )

include_directories(
  ${QWT_INCLUDE_DIR}
  ${Qt5PrintSupport_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  )

#if (Qt5_POSITION_INDEPENDENT_CODE)
#  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
#endif()

#if ( NOT WIN32 )
#  set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--export-all-symbols")
#endif()

add_definitions(-DADWIDGETS_LIBRARY)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

set( ${PROJECT_NAME}_SOURCES
  adwidgets.cpp
  targetingform.cpp
  tableview.cpp
  targetingwidget.cpp
  targetingtable.cpp
  targetingadducts.cpp
  peptidewidget.cpp
  peptidetable.cpp
  delegatehelper.cpp
  controlmethodtable.cpp
  controlmethodwidget.cpp
  centroidform.cpp
  mspeaktable.cpp
  msquantable.cpp
  mscalibrateform.cpp
  msreferencetable.cpp
  mscalibratewidget.cpp
  msreferencedialog.cpp
  mscalibratesummarytable.cpp
  mstoleranceform.cpp
  mslockform.cpp
  progresswnd.cpp
  spinslider.cpp
  dualspinslider.cpp
  filedialog.cpp
  pressuregauge.cpp
  peaktable.cpp
  mschromatogramform.cpp
  peakmethodform.cpp
  mspeaksummary.cpp
  mspeakwidget.cpp
  toftable.cpp        
  samplerunwidget.cpp
  consolewidget.cpp
  outputwidget.cpp
)

set( ${PROJECT_NAME}_HEADERS
  adwidgets.hpp
  adwidgets_global.hpp
  targetingform.hpp
  spin_t.hpp
  tableview.hpp
  targetingwidget.hpp
  targetingtable.hpp
  targetingadducts.hpp
  peptidewidget.hpp
  peptidetable.hpp
  delegatehelper.hpp
  controlmethodtable.hpp
  controlmethodwidget.hpp
  centroidform.hpp
  mspeaktable.hpp
  msquantable.hpp
  mscalibrateform.hpp
  msreferencetable.hpp
  mscalibratewidget.hpp
  msreferencedialog.hpp
  mscalibratesummarytable.hpp
  mstoleranceform.hpp
  mslockform.hpp
  progresswnd.hpp
  spinslider.hpp
  dualspinslider.hpp
  filedialog.hpp
  pressuregauge.hpp
  peaktable.hpp
  mschromatogramform.hpp
  peakmethodform.hpp
  mspeaksummary.hpp
  mspeakwidget.hpp
  toftable.hpp
  samplerunwidget.hpp
  consolewidget.hpp
  outputwidget.hpp
)

add_library( ${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_HEADERS} )

target_link_libraries( adwidgets LINK_PUBLIC
  adplugin
  adlog
  adcontrols
  adportable	
  adprot
  qtwrapper
  ${QWT_LIBRARIES}
  Qt5::Widgets
  Qt5::PrintSupport
  ${Boost_LIBRARIES}
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT apprications
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
