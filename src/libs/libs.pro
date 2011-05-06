TEMPLATE  = subdirs
CONFIG   += ordered

SUBDIRS   = \
    adfs \
    xmlparser \
    qtwrapper \
    adportable \
    adutils \
    adcontrols \
    qtwidgets \
    adinterface \
    acewrapper \
    adbroker \
    adcontroller \
    addatafile \
    adplugin \
    adtxtfactory \
    adwidgets \
    adwplot \
    aggregation \
    extensionsystem \
    portfolio \
    utils

#   adcontroller
#    interface
adbroker.depends = adinterface
