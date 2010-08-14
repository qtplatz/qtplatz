// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adplugin_global.h"
#include <QObject>
#include <adinterface/ReceiverS.h>

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

        virtual void message (::Receiver::eINSTEVENT msg, ::CORBA::ULong value );
        virtual void eventLog ( const ::Receiver::LogMessage & log );
        virtual void shutdown (void);
        virtual void debug_print (::CORBA::Long priority, ::CORBA::Long category, const char * text);
  
    signals:
        void signal_message( Receiver::eINSTEVENT msg, unsigned long value );
        void signal_eventLog( ACE_Message_Block * );
        void signal_shutdown();
        void signal_debug_print( unsigned long priority, unsigned long category, QString text );

    public slots:

    };

}


