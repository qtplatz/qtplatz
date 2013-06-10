// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#ifndef QBROKERSESSIONEVENT_H
#define QBROKERSESSIONEVENT_H

#include "adplugin_global.h"

#include <QObject>
#include <adinterface/brokereventS.h>

class ACE_Message_Block;

class ADPLUGINSHARED_EXPORT QBrokerSessionEvent : public QObject, public POA_BrokerEventSink {
    Q_OBJECT
public:
    explicit QBrokerSessionEvent(QObject *parent = 0);
    ~QBrokerSessionEvent();
    
    virtual void message( const char * );
    virtual void portfolio_created( const wchar_t * token );
    virtual void folium_added( const wchar_t * token, const wchar_t * path, const wchar_t * folderId );

signals:
    void signal_message( const QString );
    void signal_portfolio_created( const QString );
    void signal_folium_added( const QString, const QString, const QString );

public slots:

};

#endif // QBROKERSESSIONEVENT_H
