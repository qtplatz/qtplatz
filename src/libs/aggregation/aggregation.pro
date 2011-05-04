TEMPLATE = lib
TARGET = Aggregation

include(../../qtplatz_lib_dynamic.pri)

DEFINES += AGGREGATION_LIBRARY

HEADERS = aggregate.h \
    aggregation_global.h

SOURCES = aggregate.cpp

