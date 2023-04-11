
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

target_link_libraries( adextension LINK_PUBLIC
  Qt::Core
  adacquire
  adportable
  ${Boost_LIBRARIES}
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
