
project( adextension )

cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

add_definitions( -DADEXTENSION_LIBRARY )

set( adextension_SOURCES
  adextension.cpp
  adextension.hpp
  isequence.cpp
  isequence.hpp
  isequenceimpl.hpp
  ieditorfactory.cpp
  ieditorfactory.hpp
  ieditorfactory_t.hpp    
  imonitorfactory.cpp
  imonitorfactory.hpp
  ipeptide.cpp
  ipeptide.hpp
  ipeptidehandler.cpp
  ipeptidehandler.hpp
  ilogger.cpp
  ilogger.hpp
  ibroker.cpp
  ibroker.hpp
  icontroller.cpp
  icontroller.hpp
  icontrollerimpl.hpp
  idataproc.cpp
  idataproc.hpp
  iwidgetfactory.cpp
  iwidgetfactory.hpp
  adextension_global.hpp
  isnapshothandler.hpp
)

add_library( adextension SHARED ${adextension_SOURCES} )

target_link_libraries( adextension LINK_PUBLIC 
  Qt5::Core
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
