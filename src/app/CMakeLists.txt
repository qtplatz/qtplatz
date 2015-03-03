cmake_minimum_required (VERSION 2.8.11)
cmake_policy(SET CMP0028 NEW)

project (qtplatz)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package( Qt5 REQUIRED Network Core )

file(GLOB ${PROJECT_NAME}_Resources *.qrc)

include_directories(
  ${CMAKE_SOURCE_DIR}/src/shared/qtsingleapplication
  ${CMAKE_SOURCE_DIR}/src/shared/qtlockedfile
  ${CMAKE_SOURCE_DIR}/src
  ${Qt5Network_INCLUDE_DIRS}
  ${Qt5Core_INCLUDE_DIRS}
  )

if (WIN32)
  set( lockedfile_SOURCES
    ../shared/qtlockedfile/qtlockedfile_win.cpp
    )
else()
  set( lockedfile_SOURCES
    ../shared/qtlockedfile/qtlockedfile_unix.cpp
    )
endif()

set( SOURCES
  main.cpp 
  ../tools/qtcreatorcrashhandler/crashhandlersetup.cpp
  ../tools/qtcreatorcrashhandler/crashhandlersetup.h
  ../shared/qtsingleapplication/qtsingleapplication.cpp
  ../shared/qtsingleapplication/qtlocalpeer.cpp
  ../shared/qtlockedfile/qtlockedfile.cpp
  )

if ( WIN32 )
  add_executable( qtplatz WIN32
    ${SOURCES}  
    ${lockedfile_SOURCES}
    ${${PROJECT_NAME}_Resources}
    qtplatz.rc
    )
else()
  add_executable( qtplatz WIN32
    ${SOURCES}  
    ${lockedfile_SOURCES}
    ${${PROJECT_NAME}_Resources}
    qtplatz.rc
    )
endif()

target_link_libraries(
  qtplatz LINK_PUBLIC
  extensionsystem
  aggregation
  utils
  Qt5::Core
  Qt5::Network
)

install( TARGETS qtplatz RUNTIME DESTINATION bin COMPONENT applications )
