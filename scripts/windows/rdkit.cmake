######
## RDKit install on Windows
##

message( STATUS "RDKIT_BINARY_DIR = " ${RDKIT_BINARY_DIR} )
message( STATUS "RDBASE = " ${RDBASE} )
if ( NOT RDBASE )
  mseeage( FATAL_ERROR "RDBASE=")
endif()
if ( NOT RDKIT_BINARY_DIR )
  mseeage( FATAL_ERROR "RDKIT_BINARY_DIR=")
endif()

if ( NOT EXISTS ${RDBASE} )
  execute_process( COMMAND git clone https://github.com/rdkit/rdkit ${RDBASE} )
endif()

if ( RDKIT_REVISION )
  execute_process( 
    COMMAND git checkout -b ${RDKIT_REVISION}
    WORKING_DIRECTORY ${RDBASE}
    )
endif()

if ( NOT EXISTS ${RDKIT_BINARY_DIR} )
  file( MAKE_DIRECTORY ${RDKIT_BINARY_DIR} )
endif()

