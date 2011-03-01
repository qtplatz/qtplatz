INCLUDEPATH += $$PWD 

# Input
HEADERS += $$PWD/JInputStream.h \
	   $$PWD/JBufferString.h \
	   $$PWD/JBufferedStream.h \
           $$PWD/JByteInputStream.h \
           $$PWD/JFileInputStream.h \
	   $$PWD/JBufferedInputStream.h \
	   $$PWD/JBufferedInputStream.inl \
	   $$PWD/JMappedInputStream.h \
	   $$PWD/JOutputStream.h \
           $$PWD/JByteOutputStream.h \
           $$PWD/JFileOutputStream.h \
	   $$PWD/JBufferedOutputStream.h \
	   $$PWD/JBufferedOutputStream.inl \
	   $$PWD/JMappedOutputStream.h \
	   $$PWD/JFileStream.h \
	   $$PWD/JTempFileStream.h \
	   $$PWD/StringStreamWrapper.h \
	   $$PWD/StringStreamWrapper.inl \
	   $$PWD/JStream.h \
	   $$PWD/JUtils.h \
	   $$PWD/SmartPtr.h \
	   $$PWD/SmartPtr.inl \
	   $$PWD/PlatformTypes.h \
	   $$PWD/JStreamWrapper.h

SOURCES += $$PWD/JInputStream.cpp \
	   $$PWD/JBufferString.cpp \
	   $$PWD/JBufferedStream.cpp \
           $$PWD/JByteInputStream.cpp \
           $$PWD/JFileInputStream.cpp \
           $$PWD/JMappedInputStream.cpp \
	   $$PWD/JOutputStream.cpp \
           $$PWD/JByteOutputStream.cpp \
           $$PWD/JFileOutputStream.cpp \
           $$PWD/JMappedOutputStream.cpp \
	   $$PWD/JFileStream.cpp \
	   $$PWD/JTempFileStream.cpp \
	   $$PWD/JUtils.cpp

