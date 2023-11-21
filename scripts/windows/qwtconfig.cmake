# COMMAND sed 's/\([ \t]*QWT_CONFIG.*QwtDesigner\)/#\1/; s/\([ \t]*QWT_CONFIG.*QwtDll\)/#\1/' qwtconfig.pri.orig > qwtconfig.pri

file ( READ qwtconfig.pri config )
string ( REGEX REPLACE "(QWT_CONFIG[ \t]*\\+\\=[ \t]*QwtDesigner)"  "\#\\1" config "${config}" )
string ( REGEX REPLACE "(QWT_CONFIG[ \t]*\\+\\=[ \t]*QwtDll)"       "\#\\1 (cmake modified)" config "${config}" )
string ( REGEX REPLACE "(QWT_INSTALL_PREFIX[ \t]*\\=[ \t]*C:[^ \t]*)\n"   "\t\\1-qt-\$\$QT_VERSION\n" config "${config}" )
file ( WRITE qwtconfig.pri "${config}" )
