# find AgMD2 -- Keysight U5303A driver

set ( Wolfram_FOUND FALSE )
set ( Wolfram_INCLUDE_DIRS "Wolfram_INCLUDE_DIR-NOTFOUND" )
set ( WSTP_FOUND FALSE )

find_path ( WOLFRAM_DIR "WolframEngine" PATHS "/usr/local/Wolfram" )

set ( WOLFRAM_VERSION "12.0" )

set ( WOLFRAM_SYSTEM_DIR "${WOLFRAM_DIR}/WolframEngine/${WOLFRAM_VERSION}/SystemFiles" )

find_program( WSCC  "wscc" PATHS "${WOLFRAM_SYSTEM_DIR}/Links/WSTP/DeveloperKit/Linux-x86-64/CompilerAdditions" )

find_program( MPREP "mprep" PATHS "${WOLFRAM_SYSTEM_DIR}/Links/MathLink/DeveloperKit/Linux-x86-64/CompilerAdditions" )

find_path( WSTP_INCLUDE_DIRS
  "wstp.h" PATHS "${WOLFRAM_SYSTEM_DIR}/Links/WSTP/DeveloperKit/Linux-x86-64/CompilerAdditions" )

find_path( MathLink_INCLUDE_DIRS
  "mathlink.h" PATHS "${WOLFRAM_SYSTEM_DIR}/Links/MathLink/DeveloperKit/Linux-x86-64/CompilerAdditions" )

set( MathLink_DeveloperKitDir "${WOLFRAM_SYSTEM_DIR}/Links/MathLink/DeveloperKit/Linux-x86-64" )
set( MathLink_LIBRARY_DIRS    "${WOLFRAM_SYSTEM_DIR}/Libraries/Linux-x86-64" )

set( WSTP_DeveloperKitDir     "${WOLFRAM_SYSTEM_DIR}/Links/WSTP/DeveloperKit/Linux-x86-64" )
set( WSTP_LIBRARY_DIRS        "${WOLFRAM_SYSTEM_DIR}/Libraries/Linux-x86-64" )

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
    COMMAND  ${WSCC}
    ARGS     -o ${outfile} ${abs_infile}
    MAIN_DEPENDENCY ${infile})
endmacro (WSTP_ADD_TM)

if ( WSCC )
  set( Wolfram_FOUND TRUE )
  set( WSTP_FOUND TRUE )
endif()
