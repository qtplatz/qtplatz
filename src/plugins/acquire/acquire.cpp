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

#include "acquire.hpp"

# include <ace/Service_Config.h>
# include <ace/Process_Manager.h>

#if defined WIN32 && defined _MSC_VER
#  if defined _DEBUG
#     pragma comment(lib, "adcontrollerd.lib")
#     pragma comment(lib, "adcontrolsd.lib")   // static
#     pragma comment(lib, "adutilsd.lib")      // static
#     pragma comment(lib, "adinterfaced.lib")  // static
#     pragma comment(lib, "adportabled.lib")   // staitc
#     pragma comment(lib, "adwplotd.lib")
#     pragma comment(lib, "acewrapperd.lib")   // static
#     pragma comment(lib, "qtwrapperd.lib")    // static
#     pragma comment(lib, "xmlparserd.lib")   // static
#     pragma comment(lib, "adplugind.lib")     // dll
#     pragma comment(lib, "qwtd.lib")
#  else
#     pragma comment(lib, "adcontroller.lib")
#     pragma comment(lib, "adcontrols.lib")    // static
#     pragma comment(lib, "adutils.lib")       // static
#     pragma comment(lib, "adinterface.lib")   // static
#     pragma comment(lib, "adportable.lib")    // static
#     pragma comment(lib, "adwplot.lib")
#     pragma comment(lib, "acewrapper.lib")    // static
#     pragma comment(lib, "qtwrapper.lib")     // static
#     pragma comment(lib, "xmlparser.lib")    // static
#     pragma comment(lib, "adplugin.lib")      // dll
#     pragma comment(lib, "qwt.lib")
#  endif
#endif

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



acquire::acquire()
{
}
