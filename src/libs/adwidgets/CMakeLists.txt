
project(adwidgets)

cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0028 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package( Qt5 CONFIG REQUIRED Core Widgets Svg Sql PrintSupport OpenGL Multimedia )

include_directories(
  ${QWT_INCLUDE_DIR}
  ${Qt5PrintSupport_INCLUDE_DIRS}
  ${Qt5Svg_INCLUDE_DIRS}
  ${Qt5Sql_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  ${Qt5OpenGL_INCLUDE_DIRS}
  ${Qt5Multimedia_INCLUDE_DIRS}
  )

if ( rdkit_FOUND )
  include_directories( ${RDKit_INCLUDE_DIRS} )
endif()

add_definitions(-DADWIDGETS_LIBRARY)
if ( MSVC )
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251" )
endif()

file(GLOB UIs *.ui adcv/*.ui)
file(GLOB Resources *.qrc)

set( _sources "" )
if ( qwt_FOUND )
  set ( _sources pressuregauge.cpp )
endif()

set( ${PROJECT_NAME}_SOURCES
  adducts_type.cpp
  adducts_type.hpp
  adcv/contoursform.cpp
  adcv/contoursform.hpp
  adwidgets.cpp
  centroidwidget.cpp
  cherrypicker.cpp
  cgenform.cpp
  controlmethodtable.cpp
  controlmethodwidget.cpp
  consolewidget.cpp
  datareaderchoicedialog.cpp
  delegatehelper.cpp
  doublespinbox.cpp
  dualspinslider.cpp
  dgwidget.cpp
  dgwidget.hpp
  dgprotocolform.cpp
  dgprotocolform.hpp
  filedialog.cpp
  findslopeform.cpp
  hostaddrdialog.cpp
  hostaddrdialog.hpp
  htmlheaderview.hpp
  insttreeview.cpp
  mslockdialog.cpp
  moldraw.hpp
  moldraw.cpp
  moltable.cpp
  molview.cpp
  molview.hpp
  mouserbuttonfilter.cpp
  mspeaktable.cpp
  mspeaktree.cpp
  msquantable.cpp
  mscalibrateform.cpp
  mschromatogramtable.cpp
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
  playercontrols.cpp
  progresswnd.cpp
  moltablehelper.cpp
  moltableview.cpp
  samplerunwidget.cpp
  scanlawdialog.cpp
  scanlawdialog2.cpp
  scanlawform.cpp
  spinbox.cpp
  spinslider.cpp
  targetingform.cpp
  tableview.cpp
  targetingwidget.cpp
  targetingtable.cpp
  targetingadducts.cpp
  toftable.cpp
  tofchromatogramsform.cpp
  tofchromatogramswidget.cpp
  adtracewidget.cpp
  thresholdactionform.cpp
  controlmethod/timedeventswidget.cpp
  controlmethod/timedtableview.cpp
  countingwidget.cpp
  xchromatogramsform.cpp
  xchromatogramstable.cpp
  xchromatogramswidget.cpp
  utilities.cpp
  spin_t.cpp
  spin_t.hpp
  ${_sources}
)

set( ${PROJECT_NAME}_HEADERS
  adwidgets.hpp
  adwidgets_global.hpp
  centroidwidget.hpp
  cherrypicker.hpp
  cgenform.hpp
  consolewidget.hpp
  controlmethodtable.hpp
  controlmethodwidget.hpp
  datareaderchoicedialog.hpp
  delegatehelper.hpp
  doublespinbox.hpp
  dualspinslider.hpp
  filedialog.hpp
  findslopeform.hpp
  insttreeview.hpp
  peptidewidget.hpp
  peptidetable.hpp
  mslockdialog.hpp
  moltable.hpp
  mouserbuttonfilter.hpp
  mspeaktable.hpp
  mspeaktree.hpp
  msquantable.hpp
  mscalibrateform.hpp
  msreferencetable.hpp
  mscalibratewidget.hpp
  msreferencedialog.hpp
  mscalibratesummarytable.hpp
  mstoleranceform.hpp
  mslockform.hpp
  mschromatogramform.hpp
  mschromatogramtable.hpp
  mschromatogramwidget.hpp
  mspeaksummary.hpp
  mspeakwidget.hpp
  mssimulatorform.hpp
  mssimulatorwidget.hpp
  outputwidget.hpp
  peaktable.hpp
  peakmethodform.hpp
  playercontrols.hpp
  progresswnd.hpp
  pressuregauge.hpp
  moltablehelper.hpp
  moltableview.hpp
#  scanlawdialog.hpp
  scanlawdialog2.hpp
  scanlawform.hpp
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
  tofchromatogramsform.hpp
  tofchromatogramswidget.hpp
  adtracewidget.hpp
  thresholdactionform.hpp
  controlmethod/timedeventswidget.hpp
  controlmethod/timedtableview.hpp
  countingwidget.hpp
  xchromatogramsform.hpp
  xchromatogramstable.hpp
  xchromatogramswidget.hpp
  utilities.hpp
  )

if ( WIN32 )
  set_source_files_properties( moldraw.cpp PROPERTIES COMPILE_FLAGS -wd4305 )
endif()

add_library( ${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_HEADERS} )

target_link_libraries( adwidgets LINK_PUBLIC
  adchem
  adplugin
  adplugin_manager
  adlog
  adcontrols
  admethods
  adprot
  qtwrapper
  adportable
  ${QWT_LIBRARIES}
  Qt5::PrintSupport
  Qt5::Svg
  Qt5::Sql
  Qt5::Widgets
  Qt5::Core
  Qt5::OpenGL
  ${RDKit_LIBRARIES}
  ${Boost_LIBRARIES}
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib COMPONENT libraries
  )
