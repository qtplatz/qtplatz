TEMPLATE  = subdirs
CONFIG   += ordered

SUBDIRS   = \
    aggregation \
    extensionsystem \
    utils \
    adinterface \
    adfs \
    xmlparser \
    adwplot \
    adportable \
    qtwrapper \
    portfolio \
    adutils \
    acewrapper \
    adplugin \
    adcontrols \
    qtwidgets \
    adtxtfactory \
    addatafile \
    chromatogr

# adfs : boost
# xmlparser : none
# adinterface : ACE TAO
# acewrapper : boost
# adportable : acewrapper
# adutils : boost
# adwplot : QWT
# qtwrapper : QtCore

# adplugin (dll) : ACE, TAO
# adcontroller (dll) : adinterface, adportable, acewrapper, xmlparser, adplugin
# qtwidgets (dll) : adcontrols
# adwidgets (dll) : adcontrols
# adborker (dll) : adinterface, adportable, acewrapper adcontrols portfolio
# adtxtfactory (dll) : adcontrols, adportable, xmlparser, portfolio
