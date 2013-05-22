// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#include "adplugin.hpp"
#include <QMutex>
#include <QString>
#include <stdlib.h>
#include <acewrapper/constants.hpp>
#include <adportable/configuration.hpp>
#include <adportable/configloader.hpp>
#include <adportable/debug.hpp>
#include <string>
#include <vector>
#include <qtwrapper/qstring.hpp>
#include <QPluginLoader>
#include <QLibrary>
#include <QDir>
#include <QMessageBox>
#include "ifactory.hpp"
#include "imonitor.hpp"
#include "orbLoader.hpp"
#include <boost/smart_ptr.hpp>
#include <fstream>

#define BOOST_LIB_NAME boost_filesystem
#include <boost/config/auto_link.hpp>
#undef BOOST_LIB_NAME

#include <ace/Init_ACE.h>
#include <ace/Singleton.h>

#if defined ACE_WIN32
#  if defined _DEBUG || defined DEBUG
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
# if defined _DEBUG || defined DEBUG
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
