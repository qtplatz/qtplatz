
IDE_SOURCE_TREE = $$PWD
isEqual(QT_MAJOR_VERSION, 5) {

  defineReplace(cleanPath) {
    return($$clean_path($$1))
  }

  defineReplace(targetPath) {
    return($$shell_path($$1))
  }

} else {

  error ( "Required Qt5." )

}

QTPLATZ_SOURCE_TREE = $$cleanPath( $$$$IDE_SOURCE_TREE/../../ )
QTPLATZ_BUILD_TREE = $$QTPLATZ_SOURCE_TREE
INCLUDEPATH += $${QTPLATZ_SOURCE_TREE}/src/libs
