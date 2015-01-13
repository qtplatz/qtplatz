
project( adextension )

cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

add_definitions( -DADEXTENSION_LIBRARY )

set( adextension_SOURCES
  adextension.cpp
  isequence.cpp
  ieditorfactory.cpp
  imonitorfactory.cpp
  ipeptide.cpp
  ipeptidehandler.cpp
  ilogger.cpp
  ibroker.cpp
  icontroller.cpp
  idataproc.cpp
  iwidgetfactory.cpp
  adextension.hpp
  adextension_global.hpp
  isnapshothandler.hpp
  isequence.hpp
  ieditorfactory.hpp
  imonitorfactory.hpp
  ipeptide.hpp
  ipeptidehandler.hpp
  ilogger.hpp
  ibroker.hpp
  icontroller.hpp
  idataproc.hpp
  iwidgetfactory.hpp
)

add_library( adextension SHARED ${adextension_SOURCES} )

target_link_libraries( adextension LINK_PUBLIC 
  Qt5::Core
  )
