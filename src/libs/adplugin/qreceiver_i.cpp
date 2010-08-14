//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "qreceiver_i.h"
#include <adinterface/ReceiverS.h>

using namespace adplugin;

QReceiver_i::QReceiver_i(QObject *parent) :
    QObject(parent)
{
}

void
QReceiver_i::message (::Receiver::eINSTEVENT msg, ::CORBA::ULong value)
{
    emit signal_message( msg, value );
}

void
QReceiver_i::eventLog ( const ::Receiver::LogMessage & log)
{
    emit signal_eventLog( log );
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

