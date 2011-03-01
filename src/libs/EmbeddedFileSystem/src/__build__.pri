INCLUDEPATH += $$PWD 

HEADERS += $$PWD/Efs.h \
	$$PWD/EfsDefs.h \
	$$PWD/EfsErrorCodes.h \
	$$PWD/EfsFile.h \
	$$PWD/EfsHeader.h \
	$$PWD/EfsFit.h \
	$$PWD/EfsConfig.h \
	$$PWD/EfsPosixFile.h \
	$$PWD/EfsStream.h \
	$$PWD/EfsWin32File.h \
	$$PWD/EfsNameKey.h \
	$$PWD/EfsCallback.h \
	$$PWD/EfsEncryptor.h \
	$$PWD/EfsBTreeController.h \
	$$PWD/FreesBTree.h

SOURCES += $$PWD/EfsWin32File.cpp \
	$$PWD/EfsImpl.cpp \
	$$PWD/EfsFit.cpp \
	$$PWD/EfsFile.cpp \
	$$PWD/EfsHeader.cpp \
	$$PWD/EfsStream.cpp \
	$$PWD/EfsPosixFile.cpp \
	$$PWD/EfsEncryptor.cpp \
	$$PWD/EfsNameKey.cpp

