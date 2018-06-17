
project( adio )

add_library(
  ${PROJECT_NAME}
  dgprotocols.cpp
  dgprotocols.hpp
  dgprotocol.hpp
  ioconfig.cpp
  ioconfig.hpp
  ioeventsequence.cpp
  ioeventsequence.hpp
  iosequence.cpp
  iosequence.hpp
  )

target_include_directories( ${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  ARCHIVE DESTINATION lib COMPONENT libraries
  )

install( FILES
  dgprotocol.hpp
  dgprotocols.hpp
  ioconfig.hpp
  ioeventsequence.hpp
  iosequence.hpp
  DESTINATION include/adio COMPONENT headers
  )
