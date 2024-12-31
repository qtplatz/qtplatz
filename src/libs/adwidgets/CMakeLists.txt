
project(adwidgets)

cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0028 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

if ( QT_VERSION_MAJOR EQUAL 6 )
  find_package( Qt${QT_VERSION_MAJOR} CONFIG REQUIRED SvgWidgets )
endif()

if ( FALSE )
  message( STATUS "### adwidgets ### QWT_INCLUDE_DIR: ${QWT_INCLUDE_DIR}" )
  message( STATUS "### adwidgets ### QtPrintSupport_INCLUDE_DIRS: ${QtPrintSupport_INCLUDE_DIRS}" )
  message( STATUS "### adwidgets ### QtSvg_INCLUDE_DIRS: ${QtSvg_INCLUDE_DIRS}" )
  message( STATUS "### adwidgets ### QtSql_INCLUDE_DIRS: ${QtSql_INCLUDE_DIRS}" )
  message( STATUS "### adwidgets ### QtWidgets_INCLUDE_DIRS: ${QtWidgets_INCLUDE_DIRS}" )
endif()

include_directories(
  "${QWT_INCLUDE_DIR}"
  "${Qt${QT_VERSION_MAJOR}PrintSupport_INCLUDE_DIRS}"
  "${Qt${QT_VERSION_MAJOR}Svg_INCLUDE_DIRS}"
  "${Qt${QT_VERSION_MAJOR}Sql_INCLUDE_DIRS}"
  "${Qt${QT_VERSION_MAJOR}Widgets_INCLUDE_DIRS}"
  "${Qt${QT_VERSION_MAJOR}OpenGL_INCLUDE_DIRS}"
  "${Qt${QT_VERSION_MAJOR}Multimedia_INCLUDE_DIRS}" # strange
  )

if ( QT_VERSION_MAJOR EQUAL 6 )
  include_directories(
    "${Qt6SvgWidgets_INCLUDE_DIRS}" # Qt6
    )
endif()

if ( rdkit_FOUND )
  include_directories( ${RDKit_INCLUDE_DIRS} )
endif()

add_definitions(-DADWIDGETS_LIBRARY)

file(GLOB UIs *.ui adcv/*.ui)
file(GLOB Resources *.qrc)

set( _sources "" )
if ( qwt_FOUND )
  set ( _sources pressuregauge.cpp )
endif()

if ( WIN32 )
  set_source_files_properties( moldraw.cpp PROPERTIES COMPILE_FLAGS -wd4305 )
endif()

add_library( ${PROJECT_NAME} SHARED
  adcv/contoursform.cpp                 adcv/contoursform.hpp
  adducts_type.cpp                      adducts_type.hpp
  adtracewidget.cpp                     adtracewidget.hpp
  adwidgets.cpp                         adwidgets.hpp
  adwidgets_global.hpp
  centroidwidget.cpp                    centroidwidget.hpp
  cgenform.cpp                          cgenform.hpp
  cherrypicker.cpp                      cherrypicker.hpp
  consolewidget.cpp                     consolewidget.hpp
  controlmethod/timedeventswidget.cpp   controlmethod/timedeventswidget.hpp
  controlmethod/timedtableview.cpp      controlmethod/timedtableview.hpp
  controlmethodtable.cpp                controlmethodtable.hpp
  controlmethodwidget.cpp               controlmethodwidget.hpp
  countingwidget.cpp                    countingwidget.hpp
  datareaderchoicedialog.cpp            datareaderchoicedialog.hpp
  delegatehelper.cpp                    delegatehelper.hpp
  dgprotocolform.cpp                    dgprotocolform.hpp
  dgwidget.cpp                          dgwidget.hpp
  doublespinbox.cpp                     doublespinbox.hpp
  dualspinslider.cpp                    dualspinslider.hpp
  filedialog.cpp                        filedialog.hpp
  findslopeform.cpp                     findslopeform.hpp
  hostaddrdialog.cpp                    hostaddrdialog.hpp
  htmlheaderview.hpp
  insttreeview.cpp                      insttreeview.hpp
  ionreactionform.cpp                   ionreactionform.hpp
  moldraw.cpp                           moldraw.hpp
  moltable.cpp                          moltable.hpp
  moltablecolumns.hpp
  moltablehelper.cpp                    moltablehelper.hpp
  moltableview.cpp                      moltableview.hpp
  molview.cpp                           molview.hpp
  mouserbuttonfilter.cpp                mouserbuttonfilter.hpp
  mscalibrateform.cpp                   mscalibrateform.hpp
  mscalibratesummarytable.cpp           mscalibratesummarytable.hpp
  mscalibratewidget.cpp                 mscalibratewidget.hpp
  mschromatogramform.cpp                mschromatogramform.hpp
  mschromatogramtable.cpp               mschromatogramtable.hpp
  mschromatogramwidget.cpp      	mschromatogramwidget.hpp
  mslockdialog.cpp              	mslockdialog.hpp
  mslockform.cpp                	mslockform.hpp
  mspeaksummary.cpp             	mspeaksummary.hpp
  mspeaktable.cpp               	mspeaktable.hpp
  mspeaktree.cpp                	mspeaktree.hpp
  mspeakwidget.cpp              	mspeakwidget.hpp
  msquantable.cpp               	msquantable.hpp
  msreferencedialog.cpp         	msreferencedialog.hpp
  msreferencetable.cpp          	msreferencetable.hpp
  mssimulatorform.cpp           	mssimulatorform.hpp
  mssimulatorwidget.cpp         	mssimulatorwidget.hpp
  mstoleranceform.cpp           	mstoleranceform.hpp
  outputwidget.cpp              	outputwidget.hpp
  peakmethodform.cpp            	peakmethodform.hpp
  peakdwidget.cpp                       peakdwidget.hpp
  peaktable.cpp                 	peaktable.hpp
  peptidetable.cpp              	peptidetable.hpp
  peptidewidget.cpp             	peptidewidget.hpp
  playercontrols.cpp            	playercontrols.hpp
  pressuregauge.hpp
  progresswnd.cpp               	progresswnd.hpp
  samplerunwidget.cpp           	samplerunwidget.hpp
  scanlawdialog.cpp
  scanlawdialog2.cpp            	scanlawdialog2.hpp
  scanlawform.cpp               	scanlawform.hpp
  spin_t.cpp                    	spin_t.hpp
  spinbox.cpp                   	spinbox.hpp
  spinslider.cpp                	spinslider.hpp
  tableview.cpp                 	tableview.hpp
  targetingadducts.cpp          	targetingadducts.hpp
  targetingform.cpp             	targetingform.hpp
  targetingtable.cpp            	targetingtable.hpp
  targetingwidget.cpp           	targetingwidget.hpp
  thresholdactionform.cpp       	thresholdactionform.hpp
  tofchromatogramsform.cpp      	tofchromatogramsform.hpp
  tofchromatogramswidget.cpp    	tofchromatogramswidget.hpp
  toftable.cpp                  	toftable.hpp
  utilities.cpp                 	utilities.hpp
  xchromatogramsform.cpp        	xchromatogramsform.hpp
  xchromatogramstable.cpp       	xchromatogramstable.hpp
  xchromatogramswidget.cpp      	xchromatogramswidget.hpp
  pugrestform.cpp                       pugrestform.hpp
  jstrestform.cpp                       jstrestform.hpp
  figsharerestform.cpp                  figsharerestform.hpp
  sfedelayform.cpp      sfedelayform.hpp
  sfedelaydialog.cpp    sfedelaydialog.hpp
  ${_sources}
  ${Resources}
)

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
  Qt::PrintSupport
  Qt::Svg
  Qt::Sql
  Qt::Widgets
  Qt::Core
  Qt::OpenGL
  Qt::Multimedia
  Qt::SvgWidgets
  ${RDKit_LIBRARIES}
  ${Boost_LIBRARIES}
)

runtime_install_path( _runtime_destination ${PROJECT_NAME} )
library_install_path( _library_destination ${PROJECT_NAME} )
archive_install_path( _archive_destination ${PROJECT_NAME} )

install( TARGETS ${PROJECT_NAME}
  EXPORT ${QTPLATZ_TARGETS}
  RUNTIME DESTINATION ${_runtime_destination} COMPONENT applications
  LIBRARY DESTINATION ${_library_destination} COMPONENT runtime_libraries
  ARCHIVE DESTINATION ${_archive_destination} COMPONENT libraries
  )
