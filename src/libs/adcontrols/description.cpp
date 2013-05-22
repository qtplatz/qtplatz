// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
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

#include "description.hpp"
#include <ctime>

using namespace adcontrols;

Description::~Description()
{
}

Description::Description()
{
	time(&tv_sec_);
    tv_usec_ = 0;
}

Description::Description( const std::wstring& key, const std::wstring& text ) : key_(key), text_(text)
{
	time(&tv_sec_);
    tv_usec_ = 0;
}

Description::Description( const Description& t ) : tv_sec_(t.tv_sec_)
						                         , tv_usec_(t.tv_usec_)
												 , key_(t.key_)
												 , text_(t.text_)
												 , xml_(t.xml_)
{
}

