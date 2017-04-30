
project( adextension )

cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

add_definitions( -DADEXTENSION_LIBRARY )

set( adextension_SOURCES
  adextension.cpp
  adextension.hpp
  iacquire.cpp
  iacquire.hpp
  ibroker.cpp
  ibroker.hpp
  icontroller.cpp
  icontroller.hpp
  icontrollerimpl.hpp
  idataproc.cpp
  idataproc.hpp
  ieditorfactory.cpp
  ieditorfactory.hpp
  ieditorfactory_t.hpp    
  ilogger.cpp
  ilogger.hpp
  imonitorfactory.cpp
  imonitorfactory.hpp
  ipeptide.cpp
  ipeptide.hpp
  ipeptidehandler.cpp
  ipeptidehandler.hpp
  ireceiver.cpp
  ireceiver.hpp
  isequence.cpp
  isequence.hpp
  isequenceimpl.hpp
  isessionmanager.hpp
  isessionmanager.cpp
  iwidgetfactory.cpp
  iwidgetfactory.hpp
  receiverimpl.hpp
  receiverimpl.cpp
  adextension_global.hpp
  isnapshothandler.hpp
)

add_library( adextension SHARED ${adextension_SOURCES} )

set_property( TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14 )

target_link_libraries( adextension LINK_PUBLIC 
  Qt5::Core
  adicontroller
  adportable
  ${Boost_LIBRARIES}
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
