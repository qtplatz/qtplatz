
project( adio )

add_definitions(-DADIO_LIBRARY)

add_library(
  ${PROJECT_NAME} SHARED
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

target_link_libraries( ${PROJECT_NAME}
  ${Boost_LIBRARIES}
)

install( TARGETS ${PROJECT_NAME}
  EXPORT ${QTPLATZ_TARGETS}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT runtime_libraries
  ARCHIVE DESTINATION lib COMPONENT libraries )

install( FILES
  dgprotocol.hpp
  dgprotocols.hpp
  ioconfig.hpp
  ioeventsequence.hpp
  iosequence.hpp
  DESTINATION include/adio COMPONENT headers
  )
