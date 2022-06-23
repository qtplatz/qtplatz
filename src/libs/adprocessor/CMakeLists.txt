
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

add_definitions(-DADPROCESSOR_LIBRARY)

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

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib COMPONENT libraries )
