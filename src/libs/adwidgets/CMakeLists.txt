
project(adwidgets)

cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0028 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package( Qt5 REQUIRED PrintSupport Svg Widgets Gui Core )

include_directories(
  ${QWT_INCLUDE_DIR}
  ${Qt5PrintSupport_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  )

add_definitions(-DADWIDGETS_LIBRARY)
if ( MSVC )
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251" )
endif()

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

set( _sources "" )
if ( qwt_FOUND )
  set ( _sources pressuregauge.cpp )
endif()

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
  peaktable.cpp
  mschromatogramform.cpp
  mschromatogramwidget.cpp
  peakmethodform.cpp
  mspeaksummary.cpp
  mspeakwidget.cpp
  toftable.cpp        
  samplerunwidget.cpp
  consolewidget.cpp
  outputwidget.cpp
  scanlawdialog.cpp
  ${_sources}
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
  mschromatogramwidget.hpp  
  peakmethodform.hpp
  mspeaksummary.hpp
  mspeakwidget.hpp
  toftable.hpp
  samplerunwidget.hpp
  consolewidget.hpp
  outputwidget.hpp
  scanlawdialog.hpp
)

add_library( ${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_HEADERS} )

target_link_libraries( adwidgets LINK_PUBLIC
  adplugin
  adplugin_manager
  adlog
  adcontrols
  adportable	
  adprot
  qtwrapper
  ${QWT_LIBRARIES}
  Qt5::Core
  Qt5::Widgets
  Qt5::PrintSupport
  ${Boost_LIBRARIES}
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
