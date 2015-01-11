
project(adwidgets)

cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

#find_package( Qt5 REQUIRED PrintSupport Svg Widgets Gui Core )

include_directories(
  ${QWT_INCLUDE_DIR}
  ${Qt5PrintSupport_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  )

#link_directories( ${QWT}/lib )
		     
#if (Qt5_POSITION_INDEPENDENT_CODE)
#  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
#endif()

#if ( NOT WIN32 )
#  set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--export-all-symbols")
#endif()

add_definitions(-DADWIDGETS_LIBRARY)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

set( adwidgets_SOURCES 
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
	peaktable.hpp
	samplerunwidget.cpp
	samplerunwidget.hpp
	consolewidget.cpp
	consolewidget.hpp
	outputwidget.cpp
	outputwidget.hpp
	)

add_library( adwidgets SHARED ${adwidgets_SOURCES} )

target_link_libraries( adwidgets LINK_PUBLIC
  adplugin
  adlog
  adcontrols
  adportable	
  adprot
  qtwrapper
  optimized ${QWT_LIBRARY} debug ${QWT_LIBRARY_DEBUG}
  Qt5::Widgets
  Qt5::PrintSupport
  ${Boost_LIBRARIES}
  )
