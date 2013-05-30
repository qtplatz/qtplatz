TEMPLATE  = subdirs
CONFIG   += ordered

SUBDIRS = \
    libs \
    app \
    servants \
    plugins

app.depends = libs
plugins.depends = libs
