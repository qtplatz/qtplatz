// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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
/*
#define BOOST_LIB_NAME boost_iostreams
#include <boost/config/auto_link.hpp>
#undef BOOST_LIB_NAME

#define BOOST_LIB_NAME boost_serialization
#include <boost/config/auto_link.hpp>
#undef BOOST_LIB_NAME

#define BOOST_LIB_NAME boost_wserialization
#include <boost/config/auto_link.hpp>
#undef BOOST_LIB_NAME

#define BOOST_LIB_NAME boost_system
#include <boost/config/auto_link.hpp>
#undef BOOST_LIB_NAME
*/
#include "ace/Init_ACE.h"

#if defined ACE_WIN32
#  if defined _DEBUG
#     pragma comment(lib, "QtCored4.lib")
#     pragma comment(lib, "ACEd.lib")
#     pragma comment(lib, "acewrapperd.lib")
#     pragma comment(lib, "adportabled.lib")
#  else
#     pragma comment(lib, "QtCore4.lib")
#     pragma comment(lib, "ACE.lib")
#     pragma comment(lib, "acewrapper.lib")
#     pragma comment(lib, "adportable.lib")
#  endif
#endif
