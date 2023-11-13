
project(adprocessor)

cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0028 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

include_directories(
  ${CMAKE_SOURCE_DIR}/src/adplugins
  ${QWT_INCLUDE_DIR}
  ${QtWidgets_INCLUDE_DIRS}
  )

add_definitions( -DADPROCESSOR_LIBRARY )

add_library( ${PROJECT_NAME} SHARED
  dataprocessor.hpp
  dataprocessor.cpp
  mschromatogramextractor.hpp
  mschromatogramextractor_accumulate.hpp
  xchromatogram.hpp
  mschromatogramextractor_v2.cpp
  mschromatogramextractor_v2.hpp
  mschromatogramextractor_v3.cpp
  mschromatogramextractor_v3.hpp
  processmediator.hpp
  processmediator.cpp
  scanlawextractor.cpp
  scanlawextractor.hpp
  autotargeting.cpp
  autotargeting.hpp
  autotargetingcandidates.cpp
  autotargetingcandidates.hpp
  mslocker.cpp
  mslocker.hpp
  noise_filter.cpp noise_filter.hpp
  jcb2009_processor.cpp jcb2009_processor.hpp
  jcb2009_helper.cpp jcb2009_helper.hpp
  centroid_processor.cpp centroid_processor.hpp
  jcb2009_summarizer.cpp jcb2009_summarizer.hpp
  generator_property.cpp generator_property.hpp
  extract_by_generator_property.cpp  extract_by_generator_property.hpp
  )

target_link_libraries( adprocessor LINK_PUBLIC
  adcontrols
  adfs
  adutils
  portfolio
  ${Boost_LIBRARIES}
  Qt::Widgets
  Qt::Core
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
