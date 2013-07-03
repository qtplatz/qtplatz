#
# VERSION_FILE     = . #Need to give some bogus input

version.target   = version.h
win32: version.commands = bash version.sh
else: version.commands = ./version.sh
QMAKE_EXTRA_TARGETS += version
PRE_TARGETDEPS += version.h
QMAKE_CLEAN += version.h