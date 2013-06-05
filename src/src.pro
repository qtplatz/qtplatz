TEMPLATE  = subdirs
CONFIG   += ordered

SUBDIRS = \
    libs \
    adplugins \
    plugins \
    app

app.depends = libs
plugins.depends = libs
