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

#include "addatafile.hpp"

#if defined WIN32 && defined _MSC_VER
#  if defined _DEBUG
#     pragma comment(lib, "acewrapperd.lib")
#     pragma comment(lib, "adcontrolsd.lib")
#     pragma comment(lib, "adfsd.lib")
#     pragma comment(lib, "adportabled.lib")
#     pragma comment(lib, "adutilsd.lib")
#     pragma comment(lib, "portfoliod.lib")
#     pragma comment(lib, "xmlparserd.lib")
#  else
#     pragma comment(lib, "acewrapper.lib")
#     pragma comment(lib, "adcontrols.lib")
#     pragma comment(lib, "adfs.lib")
#     pragma comment(lib, "adportable.lib")
#     pragma comment(lib, "adutils.lib")
#     pragma comment(lib, "portfolio.lib")
#     pragma comment(lib, "xmlparser.lib")
#  endif
#endif

#include "datafile_factory.hpp"

#define BOOST_LIB_NAME boost_filesystem
#include <boost/config/auto_link.hpp>


namespace adcontrols {
    class datafile_factory;
}

extern "C" {
    Q_DECL_EXPORT adcontrols::datafile_factory * datafile_factory();
}

adcontrols::datafile_factory *
datafile_factory()
{
    return new addatafile::datafile_factory();
}

