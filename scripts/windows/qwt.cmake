######
## QWT install on Windows
set ( QWT_SOURCE_DIR ${BUILD_ROOT}/qwt-6.1 )
set ( QWT_BUILD_DIR ${QWT_SOURCE_DIR} )

add_custom_command(
  OUTPUT ${QWT_SOURCE_DIR}
  COMMAND svn checkout svn://svn.code.sf.net/p/qwt/code/branches/qwt-6.1
  COMMAND ${CMAKE_COMMAND} -E rename ${QWT_SOURCE_DIR}/qwtconfig.pri ${QWT_SOURCE_DIR}/qwtconfig.pri.orig
  WORKING_DIRECTORY ${BUILD_ROOT}
  )

add_custom_target( qwtconfig
  DEPENDS ${QWT_SOURCE_DIR}
  DEPENDS ${QWT_SOURCE_DIR}/qwtconfig.pri.orig
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/qwtconfig.cmake
  WORKING_DIRECTORY ${QWT_SOURCE_DIR}    
  )

add_custom_target( qwt
  DEPENDS ${QWT_SOURCE_DIR}
  DEPENDS ${QWT_SOURCE_DIR}/qwtconfig.pri.orig
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/qwtconfig.cmake
  COMMAND ${QMAKE} qwt.pro
  COMMAND nmake
  COMMAND nmake install
  WORKING_DIRECTORY ${QWT_SOURCE_DIR}  
  )
## <-- end QWT
######
