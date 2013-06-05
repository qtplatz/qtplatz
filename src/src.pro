TEMPLATE  = subdirs
CONFIG   += ordered

SUBDIRS = \
    libs \
    app \
    adplugins \
    plugins

app.depends = libs
plugins.depends = libs
