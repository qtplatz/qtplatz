IDE_SOURCE_TREE = $$PWD

isEqual(QT_MAJOR_VERSION, 5) {

  defineReplace(cleanPath) {
    return($$clean_path($$1))
  }

  defineReplace(targetPath) {
    return($$shell_path($$1))
  }

} else { # qt5

  defineReplace(cleanPath) {
    win32:1 ~= s|\\\\|/|g
    contains(1, ^/.*):pfx = /
    else:pfx =
    segs = $$split(1, /)
    out =
    for(seg, segs) {
        equals(seg, ..):out = $$member(out, 0, -2)
        else:!equals(seg, .):out += $$seg
    }
    return($$join(out, /, $$pfx))
  }

  defineReplace(targetPath) {
    return($$replace(1, /, $$QMAKE_DIR_SEP))
  }

} # qt5

QTPLATZ_SOURCE_TREE = $$cleanPath( $$$$IDE_SOURCE_TREE/.. )
PROVIDER = MS-Cheminformatics

