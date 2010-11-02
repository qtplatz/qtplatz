//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "qobserverevents_i.h"

using namespace adplugin;

QObserverEvents_i::QObserverEvents_i(QObject *parent) : QObject(parent)
                                                      , freq_( SignalObserver::Friquent )
													  , objId_(0) 
{
}

QObserverEvents_i::QObserverEvents_i( SignalObserver::Observer_ptr ptr
									 , const std::wstring& token
									 , SignalObserver::eUpdateFrequency freq 
									 , QObject *parent)	 : impl_( SignalObserver::Observer::_duplicate(ptr) )
									                     , token_( token ) 
														 , freq_( freq )
														 , objId_(0) 
														 , QObject(parent)
{
	if ( ! CORBA::is_nil( impl_.in() ) ) {
		impl_->connect( this->_this(), freq_, token.c_str() );
        objId_ = impl_->objId();
	}
}

void
QObserverEvents_i::OnClose()
{
	if ( ! CORBA::is_nil( impl_.in() ) ) {
		impl_->disconnect( this->_this() );
    }
}

void
QObserverEvents_i::OnUpdateData( CORBA::Long pos )
{
	emit signal_UpdateData( objId_, pos );
}

void
QObserverEvents_i::OnMethodChanged( CORBA::Long pos )
{
	emit signal_MethodChanged( objId_, pos );
}

void
QObserverEvents_i::OnEvent( CORBA::ULong event, CORBA::Long pos )
{
	emit signal_Event( objId_, event, pos );
}
