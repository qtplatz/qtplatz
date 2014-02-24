/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "logger.hpp"
#include "logging_handler.hpp"
#include <adportable/string.hpp>
#include <boost/system/error_code.hpp>

using namespace adlog;

logger::logger( const char * file, int line, int pri ) : pri_( pri )
                                                       , line_(line)
                                                       , file_(file == 0 ? "" : file)
{
}

logger::~logger()
{
    logging_handler::instance()->appendLog( pri_, o_.str(), file_, line_ );
}

template<> logger&
logger::operator << ( const std::wstring& text )
{
    o_ << adportable::string::convert( text );
    return *this;
}

template<> logger&
logger::operator << ( const boost::system::error_code& error )
{
    o_ << error.message();
    return *this;
}

logger&
logger::operator << ( const wchar_t * text )
{
	o_ << adportable::string::convert( text );
	return *this;
}
