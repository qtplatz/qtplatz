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

#include "description.hpp"
#include <acewrapper/timeval.hpp>

using namespace adcontrols;

Description::~Description()
{
}

Description::Description()
{
   acewrapper::gettimeofday(tv_sec_, tv_usec_);
}

Description::Description( const std::wstring& key, const std::wstring& text ) : key_(key), text_(text)
{
    acewrapper::gettimeofday(tv_sec_, tv_usec_);
}

Description::Description( const Description& t ) : tv_sec_(t.tv_sec_)
						                         , tv_usec_(t.tv_usec_)
												 , key_(t.key_)
												 , text_(t.text_)
												 , xml_(t.xml_)
{
}

