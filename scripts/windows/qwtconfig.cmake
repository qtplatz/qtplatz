
# COMMAND sed 's/\([ \t]*QWT_CONFIG.*QwtDesigner\)/#\1/; s/\([ \t]*QWT_CONFIG.*QwtDll\)/#\1/' qwtconfig.pri.orig > qwtconfig.pri

file ( READ qwtconfig.pri.orig config )
string ( REGEX REPLACE "(QWT_CONFIG[ \t]*\+\=[ \t]*QwtDesigner)"  "\# \\1" config "${config}" )
string ( REGEX REPLACE "(QWT_CONFIG[ \t]*\+\=[ \t]*QwtDll)"  "\# \\1 (cmake modified)" config "${config}" )
file ( WRITE qwtconfig.pri "${config}" )

