project( qtwrapper )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

include_directories(
  ${Qt5Widgets_INCLUDE_DIRS}
  )


add_library( qtwrapper
  qtwrapper.cpp
  application.cpp
  qstring.cpp
  xmlformatter.cpp
  progressbar.cpp
  font.cpp
  settings.cpp
)

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
