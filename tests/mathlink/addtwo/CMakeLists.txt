
project( addtwo )

#MathLink_ADD_TM(addtwo.tm .c)
WSTP_ADD_TM(addtwo.tm .c)

include_directories( ${WSTP_INCLUDE_DIRS} )

link_directories( ${WSTP_LIBRARY_DIRS} )   # /usr/local/Wolfram/Mathematica/10.4/SystemFiles/Libraries/Linux-x86-64

message( STATUS "LIBRARY_DIRS: " "${WSTP_LIBRARY_DIRS}" )
message( STATUS "LIBRARIES: "    "${WSTP_LIBRARIES}" )

add_executable ( ${PROJECT_NAME} addtwo.tm.c addtwo.c)

target_link_libraries( ${PROJECT_NAME} ${WSTP_LIBRARIES} ${WSTP_SYSTEM_LIBRARIES} )
