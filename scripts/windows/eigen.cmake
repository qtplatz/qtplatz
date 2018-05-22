######
## Eigen3 install on Windows
##
set ( EIGEN_BUILD_DIR ${BUILD_ROOT}/eigen.build )
if ( NOT EXISTS ${EIGEN_BUILD_DIR} )
  file( MAKE_DIRECTORY ${EIGEN_BUILD_DIR} )
endif()

add_custom_command(
  OUTPUT ${EIGEN_SOURCE_DIR}
  COMMAND git clone https://github.com/eigenteam/eigen-git-mirror ${EIGEN_SOURCE_DIR}
  )

add_custom_command(
  OUTPUT ${EIGEN_BUILD_DIR}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${EIGEN_BUILD_DIR}
  )

add_custom_target( eigen
  DEPENDS ${EIGEN_SOURCE_DIR} ${EIGEN_BUILD_DIR}
  COMMAND cmake -DCMAKE_CXX_FLAGS="/MP" ${EIGEN_SOURCE_DIR}
  COMMAND cmake --build . --config Release
  COMMAND cmake --build . --target install
  WORKING_DIRECTORY ${EIGEN_BUILD_DIR}
  )
## End Eigen3 install.
######

