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

#ifndef QBROKERSESSIONEVENT_H
#define QBROKERSESSIONEVENT_H

#include "adplugin_global.h"

#include <QObject>

# pragma warning(disable:4996)
# pragma warning(disable:4805)

# include <adinterface/brokereventS.h>

class ACE_Message_Block;

class ADPLUGINSHARED_EXPORT QBrokerSessionEvent : public QObject, public POA_BrokerEventSink {
    Q_OBJECT
public:
    explicit QBrokerSessionEvent(QObject *parent = 0);
    ~QBrokerSessionEvent();
    
    virtual void message( const char * );

signals:

public slots:

};

#endif // QBROKERSESSIONEVENT_H
