
if ( MathLink_FOUND )
  return()
endif()

find_package( arch )

if ( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
  find_path( _install_path NAMES "SystemFiles" PATHS /usr/local/Wolfram/Mathematica/${MathLink_FIND_VERSION_MAJOR}.${MathLink_FIND_VERSION_MINOR} )
  if ( NOT _install_path )
    message( FATAL_ERROR "MathLink: not found" )
    return()
  endif()
  message( STATUS "Mathematica installed on " ${_install_path} )

  if ( RTC_ARCH_X64 )
    set( MathLink_DeveloperKitDir "${_install_path}/SystemFiles/Links/MathLink/DeveloperKit/Linux-x86-64" )
    set( MathLink_LIBRARY_DIRS "${MathLink_DeveloperKitDir}/CompilerAdditions" "${_install_path}/SystemFiles/Libraries/Linux-x86-64" )
    set( WSTP_DeveloperKitDir "${_install_path}/SystemFiles/Links/WSTP/DeveloperKit/Linux-x86-64" )
    set( WSTP_LIBRARY_DIRS    "${WSTP_DeveloperKitDir}/CompilerAdditions" "${_install_path}/SystemFiles/Libraries/Linux-x86-64" )
  elseif ( RTC_ARCH_X86 )
    set( MathLink_DeveloperKitDir "${_install_path}/SystemFiles/Links/MathLink/DeveloperKit/Linux" )
    set( MathLink_LIBRARY_DIRS "${MathLink_DeveloperKitDir}/CompilerAdditions" "${_install_path}/SystemFiles/Libraries/Linux" )
    set( WSTP_DeveloperKitDir "${_install_path}/SystemFiles/Links/WSTP/DeveloperKit/Linux" )
    set( WSTP_LIBRARY_DIRS    "${WSTP_DeveloperKitDir}/CompilerAdditions" "${_install_path}/SystemFiles/Libraries/Linux" )
  endif()

  set( MathLink_SYSTEM_LIBRARIES pthread m rt dl )
  set( WSTP_SYSTEM_LIBRARIES pthread m rt dl )
endif()

set( MathLink_INCLUDE_DIRS "${MathLink_DeveloperKitDir}/CompilerAdditions" )
set( WSTP_INCLUDE_DIRS "${WSTP_DeveloperKitDir}/CompilerAdditions" )

find_program(MathLink_MPREP_EXECUTABLE 
  NAMES mprep
  PATHS ${MathLink_DeveloperKitDir}/CompilerAdditions
  )

find_program(WSTP_WSPREP_EXECUTABLE 
  NAMES wsprep
  PATHS ${WSTP_DeveloperKitDir}/CompilerAdditions
  )

if ( RTC_ARCH_X64 )
  find_library( MathLink_LIBRARY NAMES ML64i4 PATHS "${MathLink_DeveloperKitDir}/CompilerAdditions" )
  find_library( WSTP_LIBRARY NAMES WSTP64i4 PATHS "${WSTP_DeveloperKitDir}/CompilerAdditions" )
else()
  find_library( MathLink_LIBRARY NAMES ML32i4 PATHS "${MathLink_DeveloperKitDir}/CompilerAdditions" )
  find_library( WSTP_LIBRARY NAMES WSTP32i4 PATHS "${WSTP_DeveloperKitDir}/CompilerAdditions" )
endif()

set( MathLink_LIBRARIES ${MathLink_LIBRARY} uuid )
set( WSTP_LIBRARIES ${WSTP_LIBRARY} uuid )

# specify file extension of output file as optional second argument (default to .c)
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
   COMMAND  ${MathLink_MPREP_EXECUTABLE}
   ARGS     -o ${outfile} ${abs_infile}
   MAIN_DEPENDENCY ${infile})
endmacro (MathLink_ADD_TM)

macro (WSTP_ADD_TM infile)
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
   COMMAND  ${WSTP_WSPREP_EXECUTABLE}
   ARGS     -o ${outfile} ${abs_infile}
   MAIN_DEPENDENCY ${infile})
endmacro (WSTP_ADD_TM)

if( MathLink_INCLUDE_DIR AND MathLink_LIBRARIES )
  set( MathLink_FOUND TRUE )
endif()

