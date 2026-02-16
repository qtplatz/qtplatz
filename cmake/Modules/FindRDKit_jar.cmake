# FindRDKit_jar.cmake

find_path( rdkit_SOURCE_DIR NAMES "Code/RDGeneral" PATHS "${CMAKE_SOURCE_DIR}/../rdkit" )

if ( NOT rdkit_SOURCE_DIR )
  set ( RDKit_JAR_FOUND FALSE )
  message( STATUS "## rdkit_SOURCE_DIR=${rdkit_SOURCE_DIR} could not be found in ${CMAKE_SOURCE_DIR}/../rdkit" )
  return()
endif()

find_path( rdkit_JAR_DIR NAMES "org.RDKit.jar" PATHS "${rdkit_SOURCE_DIR}/Code/JavaWrappers/gmwrapper" )

if ( NOT rdkit_JAR_DIR )
  set ( RDKit_JAR_FOUND FALSE )
  message( STATUS "## org.RDKit.jar file not found in ${rdkit_SOURCE_DIR}/Code/JavaWrappers/gmwrapper" )
  return()
endif()

if ( rdkit_JAR_DIR )
  file( GLOB RDKit_JAR_FILES "${rdkit_JAR_DIR}/*.jar")
  file( GLOB RDKit_JNILIBS   "${rdkit_JAR_DIR}/*.jnilib")
#  message( STATUS "########################## rdkit_JAR_FILES=${RDKit_JAR_FILES} #####################" )
#  message( STATUS "########################## rdkit_JAR_JNILIBS=${RDKit_JNILIBS} #####################" )
  set ( RDKit_JAR_FOUND TRUE )
else()
  set ( RDKit_JAR_FOUND FALSE )
endif()
