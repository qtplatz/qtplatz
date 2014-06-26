TEMPLATE  = subdirs
CONFIG   += ordered

SUBDIRS = \
    libs \
    adplugins \
    app \
    plugins

app.depends = libs
plugins.depends = libs
