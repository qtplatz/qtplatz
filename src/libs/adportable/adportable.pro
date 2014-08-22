#-------------------------------------------------
#
# Project created by QtCreator 2010-07-04T06:28:41
#
#-------------------------------------------------

QT       -= core gui

TARGET = adportable
TEMPLATE = lib
CONFIG += staticlib

include(../../qtplatzstaticlib.pri)
include(../../boost.pri)

SOURCES += base64.cpp \
           configloader.cpp \
           configuration.cpp \
           constants.cpp \
           ConvertUTF.c \
           date_string.cpp \
           debug.cpp \
           fft.cpp \
           polfit.cpp \
           portable_binary_oarchive.cpp \
           portable_binary_iarchive.cpp \
           posix_path.cpp \
           serialport.cpp \
           spectrum_processor.cpp \
           string.cpp \
           timesquaredscanlaw.cpp \
           utf.cpp \
           lifecycleframe.cpp \
           timer.cpp \
           profile.cpp \
           textfile.cpp \
           bzip2.cpp \
           debug_core.cpp \
           sgfilter.cpp \
           uuid.cpp

win32 {
   SOURCES += protocollifecycle.cpp
}

HEADERS += array_wrapper.hpp \
           base64.hpp \
           binary_search.hpp \
           configloader.hpp \
           configuration.hpp \
           constants.hpp \
           ConvertUTF.h \
           date_string.hpp \
           debug.hpp \
           fft.hpp \
           float.hpp \
           formula_parser.hpp \
           is_equal.hpp \
           moment.hpp \
           polfit.hpp \
           portable_binary_archive.hpp \
           portable_binary_oarchive.hpp \
           portable_binary_iarchive.hpp \
           posix_path.hpp \
           protocollifecycle.hpp \
           safearray.hpp \
           serialport.hpp \
           serializer.hpp \
           xml_serializer.hpp \
           spectrum_processor.hpp \
           string.hpp \
           timesquaredscanlaw.hpp \
           utf.hpp \
           lifecycleframe.hpp \
           timer.hpp \
           profile.hpp \
           textfile.hpp \
           bzip2.hpp \
           debug_core.hpp \
           sgfilter.hpp \
    uuid.hpp

