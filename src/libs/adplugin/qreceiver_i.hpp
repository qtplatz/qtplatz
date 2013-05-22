// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#pragma once

#include "adplugin_global.h"
#include <QObject>

# include <adinterface/receiverS.h>

class ACE_Message_Block;

namespace adplugin {

    class message_block_ptr {
    public:
        ~message_block_ptr();
        message_block_ptr();
        explicit message_block_ptr( const ACE_Message_Block * mb );
        message_block_ptr( const message_block_ptr& );
    private:
        ACE_Message_Block * mb_;
    };

    class ADPLUGINSHARED_EXPORT QReceiver_i : public QObject
                                            , public POA_Receiver {
        Q_OBJECT
    public:
        explicit QReceiver_i(QObject *parent = 0);
        ~QReceiver_i();

        virtual void message(::Receiver::eINSTEVENT msg, ::CORBA::ULong value );
        virtual void log( const ::EventLog::LogMessage & log );
        virtual void shutdown(void);
        virtual void debug_print(::CORBA::Long priority, ::CORBA::Long category, const char * text);
  
    signals:
        void signal_message( unsigned long /* Receiver::eINSTEVENT */ msg, unsigned long value );
        void signal_log( QByteArray );
        void signal_shutdown();
        void signal_debug_print( unsigned long priority, unsigned long category, QString text );

    public slots:

    };

}


