######
## maeparser install on Windows
##

if ( NOT EXISTS ${MAEPARSER_SOURCE_DIR} )
  execute_process( COMMAND  git clone https://github.com/schrodinger/maeparser ${MAEPARSER_SOURCE_DIR} )
endif()

if ( NOT EXISTS ${MAEPARSER_BINARY_DIR} )
  file( MAKE_DIRECTORY ${MAEPARSER_BINARY_DIR} )
endif()

configure_file(
  ${CURRENT_SOURCE_DIR}/maeparser-bootstrap.bat.in
  ${MAEPARSER_BINARY_DIR}/bootstrap.bat
  )
