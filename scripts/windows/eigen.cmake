######
## Eigen3 install on Windows
##

message( STATUS "EIGEN_SOURCE_DIR = " ${EIGEN_SOURCE_DIR} )
message( STATUS "EIGEN_BINARY_DIR = " ${EIGEN_BINARY_DIR} )

execute_process( COMMAND git clone https://github.com/eigenteam/eigen-git-mirror ${EIGEN_SOURCE_DIR} )

if ( EIGEN_RELEASE )
  execute_process( 
    COMMAND git checkout -b ${EIGEN_RELEASE}
    WORKING_DIRECTORY ${EIGEN_SOURCE_DIR}
    )
endif()

if ( NOT EXISTS ${EIGEN_BINARY_DIR} )
  file( MAKE_DIRECTORY ${EIGEN_BINARY_DIR} )
endif()

