// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include "qtwidgets2.hpp"

#if defined WIN32
# if defined _DEBUG
#     pragma comment(lib, "adportabled.lib")
#     pragma comment(lib, "adcontrolsd.lib")
#     pragma comment(lib, "adplugind.lib")
#     pragma comment(lib, "qtwrapperd.lib")
#     pragma comment(lib, "xmlparserd.lib")
# else
#     pragma comment(lib, "adportable.lib")
#     pragma comment(lib, "adcontrols.lib")
#     pragma comment(lib, "adplugin.lib")
#     pragma comment(lib, "qtwrapper.lib")
#     pragma comment(lib, "xmlparser.lib")
# endif
#endif


