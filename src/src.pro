TEMPLATE  = subdirs
CONFIG   += ordered

SUBDIRS = \
    libs \
    adplugins \
    app \
    plugins \
    tools

app.depends = libs
plugins.depends = libs
tools.depends = libs
