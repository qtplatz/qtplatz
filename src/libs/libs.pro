TEMPLATE  = subdirs
CONFIG   += ordered

SUBDIRS   = \
    aggregation \
    extensionsystem \
    utils \
    adportable \
    adextension \
    adinterface \
    xmlparser \
    portfolio \
    adutils \
    acewrapper \
    adcontrols \
    chromatogr \
    adfs \
    qtwrapper \
    adwplot \
    adplugin \
    adchem \
    adsequence \
    adorbmgr

# adfs : boost
# xmlparser : none
# adinterface : TAO
# acewrapper : boost, ACE
# adportable : boost, stl
# adutils : boost
# adwplot : QWT
# qtwrapper : QtCore
# adplugin (dll) : ACE, TAO
# adcontroller (dll) : adinterface, adportable, acewrapper, xmlparser, adplugin
# qtwidgets (dll) : adcontrols
# adborker (dll) : adinterface, adportable, acewrapper adcontrols portfolio
# adtxtfactory (dll) : adcontrols, adportable, xmlparser, portfolio
