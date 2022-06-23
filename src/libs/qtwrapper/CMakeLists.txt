cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)

#find_package( Qt${QT_VERSION_MAJOR} CONFIG REQUIRED Widgets )

project( qtwrapper )

add_definitions(-DQTWRAPPER_LIBRARY)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

include_directories(
  ${QtCore_INCLUDE_DIRS}
  ${QtWidgets_INCLUDE_DIRS}
  )

add_library( ${PROJECT_NAME} SHARED
  application.cpp
  make_widget.hpp
  font.cpp
  progresshandler.hpp
  qtwrapper.cpp
  settings.cpp
  waitcursor.cpp
  waitcursor.hpp
  xmlformatter.cpp
)

target_link_libraries( ${PROJECT_NAME}
  Qt::Core
  Qt::Widgets
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT runtime_libraries
  ARCHIVE DESTINATION lib COMPONENT libraries )
