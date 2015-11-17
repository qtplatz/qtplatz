cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)

find_package( Qt5 CONFIG REQUIRED Widgets )

project( qtwrapper )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

include_directories(
  ${Qt5Core_INCLUDE_DIRS}  
  ${Qt5Widgets_INCLUDE_DIRS}
  )

add_library( qtwrapper
  application.cpp
  make_widget.hpp
  font.cpp
  progresshandler.hpp
  qtwrapper.cpp
  qstring.cpp
  settings.cpp
  waitcursor.cpp
  waitcursor.hpp
  xmlformatter.cpp
)

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
