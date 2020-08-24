
project( addtwo )

WSTP_ADD_TM(addtwo.tm .cxx)

include_directories( ${WSTP_INCLUDE_DIRS} )

link_directories( ${WSTP_LIBRARY_DIRS} )   # /usr/local/Wolfram/Mathematica/10.4/SystemFiles/Libraries/Linux-x86-64

message( STATUS "LIBRARY_DIRS: " "${WSTP_LIBRARY_DIRS}" )
message( STATUS "LIBRARIES: "    "${WSTP_LIBRARIES}" )

add_executable ( ${PROJECT_NAME} addtwo.tm.cxx addtwo.cxx )

target_link_libraries( ${PROJECT_NAME} ${WSTP_LIBRARIES} ${WSTP_SYSTEM_LIBRARIES} m )
