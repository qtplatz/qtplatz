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

#include "qbrokersessionevent.hpp"
#include <qtwrapper/qstring.hpp>

QBrokerSessionEvent::QBrokerSessionEvent(QObject *parent) :
    QObject(parent)
{
}

QBrokerSessionEvent::~QBrokerSessionEvent()
{
}

void
QBrokerSessionEvent::message( const char * msg )
{
    emit signal_message( QString(msg) );
}

void
QBrokerSessionEvent::portfolio_created( const wchar_t * token )
{
    emit signal_portfolio_created( qtwrapper::qstring( token ) );
}

void
QBrokerSessionEvent::folium_added( const wchar_t * token, const wchar_t * path, const wchar_t * id )
{
    emit signal_folium_added( qtwrapper::qstring( token ), qtwrapper::qstring( path ), qtwrapper::qstring( id ) );
}