#
# VERSION_FILE     = . #Need to give some bogus input

version.target   = version.h
win32: version.commands = bash $${IDE_BUILD_TREE}/src/version.sh
else: version.commands = $${IDE_BUILD_TREE}/src/version.sh
version.depends = FORCE
version.output = version.h
version.clean += version.h
PRE_TARGETDEPS += version.h
QMAKE_EXTRA_TARGETS += version
system( bash $${IDE_BUILD_TREE}/src/version.sh )
