#Generate all files required for a corba server app.
# ADD_CORBA_SERVERS( foo_SRCS foo_HPPS file.idl ... ) 

MACRO(ADD_CORBA_SERVERS _sources _headers)

   FOREACH (_current_FILE ${ARGN})

      GET_FILENAME_COMPONENT(_tmp_FILE ${_current_FILE} ABSOLUTE)
      GET_FILENAME_COMPONENT(_basename ${_tmp_FILE} NAME_WE)

      SET(_server  ${CMAKE_CURRENT_BINARY_DIR}/${_basename}S.cpp)
      SET(_serverh ${CMAKE_CURRENT_BINARY_DIR}/${_basename}S.h ${CMAKE_CURRENT_BINARY_DIR}/${_basename}S.inl)

      SET(_client  ${CMAKE_CURRENT_BINARY_DIR}/${_basename}C.cpp)
      SET(_clienth ${CMAKE_CURRENT_BINARY_DIR}/${_basename}C.h ${CMAKE_CURRENT_BINARY_DIR}/${_basename}C.inl)

      IF (NOT HAVE_${_basename}_SERVER_RULE)
         SET(HAVE_${_basename}_SERVER_RULE ON)
	 # CMake atrocity: if none of these OUTPUT files is used in a target in the current CMakeLists.txt file,
	 # the ADD_CUSTOM_COMMAND is plainly ignored and left out of the make files.
         ADD_CUSTOM_COMMAND(OUTPUT ${_tserver} ${_server} ${_client} ${_tserverh} ${_serverh} ${_clienth}
           COMMAND ${TAO_IDL} ${IDLFLAGS} ${_tmp_FILE} -o ${CMAKE_CURRENT_BINARY_DIR} -I${CMAKE_CURRENT_SOURCE_DIR} ${IDL_INCLUDES}
           DEPENDS ${_tmp_FILE}
         )
     ENDIF (NOT HAVE_${_basename}_SERVER_RULE)

     set(${_sources} ${${_sources}} ${_server} ${_tserver} ${_client})
     set(${_headers} ${${_headers}} ${_serverh} ${_tserverh} ${_clienth})

     SET_SOURCE_FILES_PROPERTIES(${_server} ${_serverh} ${_tserver} ${_client} ${_tserverh} ${_clienth} PROPERTIES GENERATED true)
    ENDFOREACH (_current_FILE)

ENDMACRO(ADD_CORBA_SERVERS)
