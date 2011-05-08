/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "qreceiver_i.hpp"

#include <adinterface/eventlog_helper.hpp>
#include <qtwrapper/qstring.hpp>

using namespace adplugin;

QReceiver_i::QReceiver_i(QObject *parent) :
    QObject(parent)
{
}

QReceiver_i::~QReceiver_i()
{
}

void
QReceiver_i::message (::Receiver::eINSTEVENT msg, ::CORBA::ULong value)
{
    emit signal_message( msg, value );
}

void
QReceiver_i::log( const ::EventLog::LogMessage & log )
{
    TAO_OutputCDR cdr;
    cdr << log;
    QByteArray qarray( cdr.begin()->rd_ptr(), cdr.begin()->size() );
    emit signal_log( qarray );
}

void
QReceiver_i::shutdown (void)
{
    emit signal_shutdown();
}

void
QReceiver_i::debug_print (::CORBA::Long priority, ::CORBA::Long category, const char * text)
{
    emit signal_debug_print( priority, category, QString(text) );
}

