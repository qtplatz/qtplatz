cmake_minimum_required (VERSION 2.8.11)
project (app)

add_executable( qtplatz 
  main.cpp 
  ../tools/qtcreatorcrashhandler/crashhandlersetup.cpp
  ../tools/qtcreatorcrashhandler/crashhandlersetup.h
)

target_link_libraries( qtplatz
  LINK_PUBLIC
  ExtensionSystem
  Aggregation
  Utils
)


