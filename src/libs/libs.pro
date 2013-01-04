TEMPLATE  = subdirs
CONFIG   += ordered

SUBDIRS   = \
    aggregation \
    extensionsystem \
    utils \
    adportable \
    adextension \
    adinterface \
    adcontrols \
    adtxtfactory \
    adfs \
    addatafile \
    adwplot \
    adplugin \
    adutils \
    xmlparser \
    qtwrapper \
    portfolio \
    acewrapper \
    qtwidgets \
    chromatogr \
    adchem \
    adsequence

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
