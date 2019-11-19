# find AgMD2 -- Keysight U5303A driver

set ( Wolfram_FOUND FALSE )
set ( Wolfram_INCLUDE_DIRS "Wolfram_INCLUDE_DIR-NOTFOUND" )
set ( WSTP_FOUND FALSE )

set ( WOLFRAM_VERSION "12.0" )

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  
  find_path ( WOLFRAM_DIR "Contents/Resources/Wolfram Player.app" PATHS "/Applications/Wolfram Engine.app" )
  if ( WOLFRAM_DIR )
    find_path( WOLFRAM_SYSTEM_DIR "Links" PATHS "${WOLFRAM_DIR}/Contents/Resources/Wolfram Player.app/Contents/SystemFiles" )
  endif()
  if ( WOLFRAM_SYSTEM_DIR )
    find_path( WSTP_DeveloperKitDir "CompilerAdditions" PATHS "${WOLFRAM_SYSTEM_DIR}/Links/WSTP/DeveloperKit/MacOSX-x86-64" )
  endif()
  find_library( COREFOUNDATION_LIBRARY CoreFoundation )
  set ( WSTP_LIBRARY_DIRS    "${WSTP_DeveloperKitDir}/CompilerAdditions" )
  set ( WSTP_LIBRARIES WSTPi4 ${COREFOUNDATION_LIBRARY} )

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  
  find_path ( WOLFRAM_DIR "WolframEngine" PATHS "/usr/local/Wolfram" )
  if ( WOLFRAM_DIR )
    set ( WOLFRAM_SYSTEM_DIR "${WOLFRAM_DIR}/WolframEngine/${WOLFRAM_VERSION}/SystemFiles" )    
  endif()
  if ( WOLFRAM_SYSTEM_DIR )
    find_path ( WSTP_DeveloperKitDir "CompilerAdditions" PATHS "${WOLFRAM_SYSTEM_DIR}/Links/WSTP/DeveloperKit/Linux-x86-64" )
  endif()
  set ( WSTP_LIBRARY_DIRS    "${WOLFRAM_SYSTEM_DIR}/Libraries/Linux-x86-64/CompilerAdditions" )
  set ( WSTP_LIBRARIES WSTPi4 )  
  set ( WSTP_SYSTEM_LIBRARIES Threads::Threads rt stdc++ ${CMAKE_DL_LIBS} m )
endif()

find_package( Threads )

find_program( WSPREP "wsprep" PATHS "${WSTP_DeveloperKitDir}/CompilerAdditions" )
find_path( WSTP_INCLUDE_DIRS "wstp.h" PATHS "${WSTP_DeveloperKitDir}/CompilerAdditions" )

## ------------------------------------------------------------------------
macro ( WSTP_ADD_TM infile )
  get_filename_component( outfile ${infile} NAME_WE )
  get_filename_component( abs_infile ${infile} ABSOLUTE )
  get_filename_component( infile_ext ${infile} EXT )
  if( ${ARGC} EQUAL 1 )
    set( outfile ${CMAKE_CURRENT_BINARY_DIR}/${outfile}${infile_ext}.cxx )
  else()
    set( outfile ${CMAKE_CURRENT_BINARY_DIR}/${outfile}${infile_ext}${ARGV1} )
  endif()
  add_custom_command(
    OUTPUT   ${outfile}
    COMMAND  ${WSPREP}
    ARGS     -o ${outfile} ${abs_infile}
    MAIN_DEPENDENCY ${infile})
endmacro (WSTP_ADD_TM)

if ( WSPREP )
  set( Wolfram_FOUND TRUE )
  set( WSTP_FOUND TRUE )
endif()

message( STATUS "## WOLFRAM_DIR: " ${WOLFRAM_DIR} )
message( STATUS "## WOLFRAM_SYSTEM_DIR: " ${WOLFRAM_SYSTEM_DIR} )
message( STATUS "## WSTP_DeveloperKitDir: " ${WSTP_DeveloperKitDir} )
