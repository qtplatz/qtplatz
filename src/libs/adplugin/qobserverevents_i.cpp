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

#include "qobserverevents_i.h"
#include "orbmanager.h"

using namespace adplugin;

QObserverEvents_i::~QObserverEvents_i()
{
    OnClose();
    adplugin::ORBManager::instance()->deactivate( this->_this() );
}

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
    if ( ! CORBA::is_nil( impl_.in() ) )
        impl_->disconnect( this->_this() );
}

void
QObserverEvents_i::OnUpdateData( CORBA::ULong objId, CORBA::Long pos )
{
#if defined _DEBUG
    std::cout << "emit UpdateData(" << objId << ", " << pos << ")" << std::endl;
#endif
	emit signal_UpdateData( objId, pos );
}

void
QObserverEvents_i::OnMethodChanged( CORBA::ULong objId, CORBA::Long pos )
{
	emit signal_MethodChanged( objId, pos );
}

void
QObserverEvents_i::OnEvent( CORBA::ULong objId, CORBA::ULong event, CORBA::Long pos )
{
	emit signal_Event( objId, event, pos );
}
