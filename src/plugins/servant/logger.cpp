/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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
#include <adlog/logger.hpp>
#include <adportable/debug.hpp>
#include <qcolor.h>
#include <boost/format.hpp>

using namespace servant;

Logger::~Logger()
{
}

Logger::Logger(QObject *parent) :  adextension::iLogger(parent)
{
}

void
Logger::appendLog( const std::string& text, bool richText )
{
    emit onLogging( QString::fromStdString( text ), richText );
}

void
Logger::operator ()( int pri, const std::string& text, const std::string& file, int line, const std::chrono::system_clock::time_point& tp )
{
    QString loc = (file.empty() ? "" : (boost::format("%s(%4d):") % file % line).str().c_str() );

    if ( pri < adlog::LOG_ERR )
		emit onLogging( QString( "<font color=red>%1&nbsp;&nbsp;%2</font>" ).arg( loc, text.c_str() ), true );
    else if ( pri < adlog::LOG_WARNING )
        emit onLogging( QString( "<span style='background-color: yellow'>%1&nbsp;&nbsp;%2</span>" ).arg( loc, text.c_str() ), true );
    else if ( pri < adlog::LOG_NOTICE )
        emit onLogging( QString( "<font color=blue>%1&nbsp;&nbsp;%2</font>" ).arg( loc, text.c_str() ), true );
    else
        emit onLogging( QString("%1\t%2").arg( loc, text.c_str() ), false );
}
