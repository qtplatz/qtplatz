//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "adplugin.h"
#include <QMutex>
#include <QString>
#include <stdlib.h>
#include <acewrapper/constants.h>
#include <adportable/configuration.h>
#include <adportable/configloader.h>
#include <adportable/debug.h>
#include <string>
#include <vector>
#include <qtwrapper/qstring.h>
#include <QPluginLoader>
#include <QLibrary>
#include <QDir>
#include <QMessageBox>
#include "ifactory.h"
#include "imonitor.h"
#include "orbLoader.h"
#include <boost/smart_ptr.hpp>
#include <fstream>

#pragma warning (disable: 4996)
#include <ace/Init_ACE.h>
#include <ace/Singleton.h>
#pragma warning (default: 4996)

#if defined ACE_WIN32
#  if defined _DEBUG
#     pragma comment(lib, "TAO_Utilsd.lib")
#     pragma comment(lib, "TAO_PId.lib")
#     pragma comment(lib, "TAO_PortableServerd.lib")
#     pragma comment(lib, "TAO_AnyTypeCoded.lib")
#     pragma comment(lib, "TAOd.lib")
#     pragma comment(lib, "ACEd.lib")
#  else
#     pragma comment(lib, "TAO_Utils.lib")
#     pragma comment(lib, "TAO_PI.lib")
#     pragma comment(lib, "TAO_PortableServer.lib")
#     pragma comment(lib, "TAO_AnyTypeCode.lib")
#     pragma comment(lib, "TAO.lib")
#     pragma comment(lib, "ACE.lib")
#  endif
#endif

#if defined WIN32
# if defined _DEBUG
#     pragma comment(lib, "adinterfaced.lib")
#     pragma comment(lib, "adportabled.lib")
#     pragma comment(lib, "acewrapperd.lib")
#     pragma comment(lib, "qtwrapperd.lib")
#     pragma comment(lib, "xmlparserd.lib")
# else
#     pragma comment(lib, "adinterface.lib")
#     pragma comment(lib, "adportable.lib")
#     pragma comment(lib, "acewrapper.lib")
#     pragma comment(lib, "qtwrapper.lib")
#     pragma comment(lib, "xmlparser.lib")
# endif
#endif

//////////////////////////////////////
////////////////////////////////////////
