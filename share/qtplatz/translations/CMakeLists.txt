cmake_minimum_required(VERSION 2.8.11)
project( translations )

find_package( Qt5LinguistTools )

message( STATUS "Qt5LinguistTools: " ${Qt5LinguistTools_DIR} )

set( UPDATE_TRANSLATIONS OFF )

set( LANGUAGES cs de fr it ja pl ru sl uk zh_CN zh_TW )

foreach( lang ${LANGUAGES} )
  set( TS_FILE ${CMAKE_CURRENT_SOURCE_DIR}/qtplatz_${lang}.ts )
  set( ts_files ${ts_files} ${TS_FILE} )
  set_source_files_properties( ${TS_FILE} PROPERTIES OUTPUT_LOCATION ${CMAKE_CURRENT_BINARY_DIR} )
endforeach()

if ( UPDATE_TRANSLATIONS )
  file( GLOB_RECURSE files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/../../../src/*.cpp
    ${CMAKE_SOURCE_DIR}/../../../src/*.hpp
    ${CMAKE_SOURCE_DIR}/../../../src/*.ui )
  
  foreach( a ${files} )
    get_filename_component( file ${a} ABSOLUTE )
    list( APPEND sources ${file} )
  endforeach()
  QT5_CREATE_TRANSLATION( qm_files ${sources} ${ts_files} )
else()
  QT5_ADD_TRANSLATION( qm_files ${ts_files} )
endif()

foreach( qm_file ${qm_files} )
  message( "qm_file: " ${qm_file} )
endforeach()

file( WRITE phony.c "int main() { return 0; }" )

#add_custom_target( translations_target DEPENDS ${qm_files} )
add_executable( translations phony.c ${qm_files} )

#install( FILES ${qm_files} DESTINATION ${CMAKE_SOURCE_DIR}/translations )
install( FILES ${qm_files} DESTINATION share/qtplatz/translations COMPONENT translations )
set_directory_properties( PROPERTIES CLEAN_NO_CUSTOM true )

