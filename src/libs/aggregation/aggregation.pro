TEMPLATE = lib
TARGET = Aggregation

include(../../qtplatz_library_rule.pri)

DEFINES += AGGREGATION_LIBRARY

HEADERS = aggregate.h \
    aggregation_global.h

SOURCES = aggregate.cpp

