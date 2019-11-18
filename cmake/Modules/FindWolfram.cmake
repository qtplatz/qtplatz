# find AgMD2 -- Keysight U5303A driver

set ( Wolfram_FOUND FALSE )
set ( Wolfram_INCLUDE_DIRS "Wolfram_INCLUDE_DIR-NOTFOUND" )
set ( WSTP_FOUND FALSE )

find_path ( WOLFRAM_DIR "WolframEngine" PATHS "/usr/local/Wolfram" )

set ( WOLFRAM_VERSION "12.0" )

set ( WOLFRAM_SYSTEM_DIR "${WOLFRAM_DIR}/WolframEngine/${WOLFRAM_VERSION}/SystemFiles" )

find_package( Threads )
## WSTP Env.
set( WSTP_DeveloperKitDir "${WOLFRAM_SYSTEM_DIR}/Links/WSTP/DeveloperKit/Linux-x86-64" )
find_program( WSPREP "wsprep" PATHS "${WSTP_DeveloperKitDir}/CompilerAdditions" )

find_path( WSTP_INCLUDE_DIRS
  "wstp.h" PATHS "${WOLFRAM_SYSTEM_DIR}/Links/WSTP/DeveloperKit/Linux-x86-64/CompilerAdditions" )

find_library( _lib NAMES "WSTP64i4" PATHS "${WSTP_DeveloperKitDir}/CompilerAdditions" )
set ( WSTP_LIBRARY_DIRS    "${WOLFRAM_SYSTEM_DIR}/Libraries/Linux-x86-64" )
set ( WSTP_LIBRARIES ${_lib} uuid Threads::Threads ${CMAKE_DL_LIBS} rt )

## --------------
find_path( MathLink_INCLUDE_DIRS
  "mathlink.h" PATHS "${WOLFRAM_SYSTEM_DIR}/Links/MathLink/DeveloperKit/Linux-x86-64/CompilerAdditions" )

find_program( MPREP "mprep" PATHS "${WOLFRAM_SYSTEM_DIR}/Links/MathLink/DeveloperKit/Linux-x86-64/CompilerAdditions" )
set( MathLink_DeveloperKitDir "${WOLFRAM_SYSTEM_DIR}/Links/MathLink/DeveloperKit/Linux-x86-64" )
set( MathLink_LIBRARY_DIRS    "${WOLFRAM_SYSTEM_DIR}/Libraries/Linux-x86-64" )


## ------------------------------------------------------------------------
## --- MathLink is deprecated -- Use WSTP instead
## ------------------------------------------------------------------------
macro (MathLink_ADD_TM infile)
  get_filename_component(outfile ${infile} NAME_WE)
  get_filename_component(abs_infile ${infile} ABSOLUTE)
  get_filename_component(infile_ext ${infile} EXT)
  if( ${ARGC} EQUAL 1 )
   set(outfile ${CMAKE_CURRENT_BINARY_DIR}/${outfile}${infile_ext}.c)
 else()
   set(outfile ${CMAKE_CURRENT_BINARY_DIR}/${outfile}${infile_ext}${ARGV1})
 endif()
 add_custom_command(
   OUTPUT   ${outfile}
   COMMAND  ${MPREP}
   ARGS     -o ${outfile} ${abs_infile}
   MAIN_DEPENDENCY ${infile})
endmacro (MathLink_ADD_TM)

## ------------------------------------------------------------------------

macro ( WSTP_ADD_TM infile )
  get_filename_component( outfile ${infile} NAME_WE )
  get_filename_component( abs_infile ${infile} ABSOLUTE )
  get_filename_component( infile_ext ${infile} EXT )
  if( ${ARGC} EQUAL 1 )
    set(outfile ${CMAKE_CURRENT_BINARY_DIR}/${outfile}${infile_ext}.c)
  else()
    set(outfile ${CMAKE_CURRENT_BINARY_DIR}/${outfile}${infile_ext}${ARGV1})
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
