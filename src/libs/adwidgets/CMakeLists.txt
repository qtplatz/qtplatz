
project(adwidgets)

cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0028 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package( Qt5 CONFIG REQUIRED Core Widgets Svg PrintSupport )

include_directories(
  ${QWT_INCLUDE_DIR}
  ${Qt5PrintSupport_INCLUDE_DIRS}
  ${Qt5Svg_INCLUDE_DIRS}  
  ${Qt5Widgets_INCLUDE_DIRS}
  )

if ( rdkit_FOUND )
  include_directories( ${RDKit_INCLUDE_DIRS} )
endif()

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
  centroidform.cpp
  controlmethodtable.cpp
  controlmethodwidget.cpp
  consolewidget.cpp
  delegatehelper.cpp
  doublespinbox.cpp
  dualspinslider.cpp
  filedialog.cpp
  moltable.cpp
  mspeaktable.cpp
  msquantable.cpp
  mscalibrateform.cpp
  mschromatogramform.cpp
  mschromatogramwidget.cpp
  mspeaksummary.cpp
  mspeakwidget.cpp
  mssimulatorform.cpp
  mssimulatorwidget.cpp
  msreferencetable.cpp
  mscalibratewidget.cpp
  msreferencedialog.cpp
  mscalibratesummarytable.cpp
  mstoleranceform.cpp
  mslockform.cpp
  outputwidget.cpp
  peakmethodform.cpp
  peaktable.cpp
  peptidewidget.cpp
  peptidetable.cpp
  progresswnd.cpp
  samplerunwidget.cpp
  scanlawdialog.cpp
  spinbox.cpp
  spinslider.cpp
  targetingform.cpp
  tableview.cpp
  targetingwidget.cpp
  targetingtable.cpp
  targetingadducts.cpp
  toftable.cpp        
  ${_sources}
)

set( ${PROJECT_NAME}_HEADERS
  adwidgets.hpp
  adwidgets_global.hpp
  consolewidget.hpp
  controlmethodtable.hpp
  controlmethodwidget.hpp
  centroidform.hpp
  delegatehelper.hpp
  doublespinbox.hpp
  dualspinslider.hpp
  filedialog.hpp
  peptidewidget.hpp
  peptidetable.hpp
  moltable.hpp  
  mspeaktable.hpp
  msquantable.hpp
  mscalibrateform.hpp
  msreferencetable.hpp
  mscalibratewidget.hpp
  msreferencedialog.hpp
  mscalibratesummarytable.hpp
  mstoleranceform.hpp
  mslockform.hpp
  mschromatogramform.hpp
  mschromatogramwidget.hpp  
  mspeaksummary.hpp
  mspeakwidget.hpp
  mssimulatorform.hpp
  mssimulatorwidget.hpp
  outputwidget.hpp
  peaktable.hpp
  peakmethodform.hpp
  progresswnd.hpp
  pressuregauge.hpp
  scanlawdialog.hpp
  spin_t.hpp
  spinbox.hpp
  spinslider.hpp
  samplerunwidget.hpp
  toftable.hpp
  targetingform.hpp
  tableview.hpp
  targetingwidget.hpp
  targetingtable.hpp
  targetingadducts.hpp
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
  Qt5::Svg
  Qt5::PrintSupport
  ${Boost_LIBRARIES}
  ${RDKit_LIBRARIES}
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
