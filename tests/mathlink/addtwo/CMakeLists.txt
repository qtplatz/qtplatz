
MathLink_ADD_TM(addtwo.tm .c)

include_directories( ${MathLink_INCLUDE_DIR} )

link_directories( ${MathLink_LIBRARY_DIRS} ) # /usr/local/Wolfram/Mathematica/10.4/SystemFiles/Libraries/Linux-x86-64 )

add_executable (addtwo addtwo.tm.c addtwo.c)

target_link_libraries( addtwo ${MathLink_LIBRARIES} ${MathLink_SYSTEM_LIBRARIES} )
