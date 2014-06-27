#
# VERSION_FILE     = . #Need to give some bogus input

PRE_TARGETDEPS += version.h

contains(TEMPLATE, "vc.*") {
# CAUTION: This create version.h when qmake run, not build time
# due to qmake's PRE_TARGETDEPS could not be make it work with msbuild
  system( bash $$PWD/version.bat )
} else {
  version.commands = $$PWD/version.sh
  version.depends = FORCE
  version.output = version.h
  version.clean += version.h
  PRE_TARGETDEPS += version.h
  QMAKE_EXTRA_TARGETS += version
}
