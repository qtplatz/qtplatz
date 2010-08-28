//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "qobserverevents_i.h"

using namespace adplugin;

QObserverEvents_i::QObserverEvents_i(QObject *parent) :
    QObject(parent)
{
}

void
QObserverEvents_i::OnUpdateData( CORBA::Long pos )
{
	emit signal_UpdateData( pos );
}

void
QObserverEvents_i::OnMethodChanged( CORBA::Long pos )
{
	emit signal_MethodChanged( pos );
}

void
QObserverEvents_i::OnEvent( CORBA::ULong event, CORBA::Long pos )
{
	emit signal_Event( event, pos );
}
