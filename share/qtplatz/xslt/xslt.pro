include(../../../qtplatz.pri)

XSLFILES = quan-html.xsl

updatexslt.input = XSLFILES
updatexslt.output = $$IDE_DATA_PATH/xslt/${QMAKE_FILE_BASE}.xsl
isEmpty(vcproj):updatexslt.variable_out = PRE_TARGETDEPS
updatexslt.commands = $$QMAKE_COPY ${QMAKE_FILE_IN} ${QMAKE_FILE_OUT}
updatexslt.name = COPY ${QMAKE_FILE_IN}
updatexslt.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += updatexslt
isEmpty(vcproj) {
    QMAKE_LINK = @: IGNORE THIS LINE
    OBJECTS_DIR =
    win32:CONFIG -= embed_manifest_exe
} else {
    CONFIG += console
    PHONY_DEPS = .
    phony_src.input = PHONY_DEPS
    phony_src.output = phony.c
    phony_src.variable_out = GENERATED_SOURCES
    phony_src.commands = echo int main() { return 0; } > phony.c
    phony_src.name = CREATE phony.c
    phony_src.CONFIG += combine
    QMAKE_EXTRA_COMPILERS += phony_src
}

xslfiles.files = $$XSLFILES
xslfiles.path = $$QTC_PREFIX/share/qtplatz/xslt
xslfiles.CONFIG += no_check_exist
xslfiles.CONFIG += no_link
INSTALLS += xslfiles
