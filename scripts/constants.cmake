
set ( BOOST_VERSION	"1_67_0" )

set ( qmake_hints
  "C:/Qt/5.11.0/msvc2017_64/bin"
  "C:/Qt/5.10.1/msvc2017_64/bin"
  "C:/Qt/5.9.2/msvc2017_64/bin"
  )

find_program ( QMAKE qmake HINTS ${qmake_hints} )
