// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@scienceliaison.com
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

#pragma once

#include <adplugin/constants.hpp>

#if defined WIN32
#  if defined _DEBUG
#    define QTWIDGETS_NAME "/MS-Cheminformatics/qtwidgetsd.dll"
#  else
#    define QTWIDGETS_NAME "/MS-Cheminformatics/qtwidgets.dll"
#  endif
#elif defined __linux__
#  define QTWIDGETS_NAME   "/MS-Cheminformatics/libqtwidgets.so"
#elif defined __APPLE__
# if defined DEBUG
#  define QTWIDGETS_NAME   "/MS-Cheminformatics/libqtwidgets_debug.dylib"
# else
#  define QTWIDGETS_NAME   "/MS-Cheminformatics/libqtwidgets.dylib"
# endif
#endif
