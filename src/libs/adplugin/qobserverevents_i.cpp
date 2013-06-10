/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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

#include "qobserverevents_i.hpp"
//#include "orbmanager.hpp"
#include <adportable/debug.hpp>

using namespace adplugin;

namespace adplugin {

	class ObserverEvents_i : public POA_SignalObserver::ObserverEvents {
		QObserverEvents_i& qobj_;
	public:
		ObserverEvents_i( QObserverEvents_i& qobj ) : qobj_( qobj ) {
		}
		// implements ObserverEvents
		void OnConfigChanged( CORBA::ULong objId, SignalObserver::eConfigStatus status ) {
			qobj_.onConfigChanged( objId, status );
		}
		void OnUpdateData( CORBA::ULong objId, CORBA::Long pos ) {
            qobj_.onUpdateData( objId, pos );
		}
		void OnMethodChanged( CORBA::ULong objId, CORBA::Long pos ) {
			qobj_.onMethodChanged( objId, pos );
		}
		void OnEvent( CORBA::ULong objId, CORBA::ULong event, CORBA::Long pos ) {
			qobj_.onEvent( objId, event, pos );
		}
	};

}

QObserverEvents_i::~QObserverEvents_i()
{
	if ( connected_ )
		disconnect();
	if ( impl_ ) 
		impl_->_remove_ref();
	if ( sink_ )
		sink_->_remove_ref();
}

QObserverEvents_i::QObserverEvents_i(QObject *parent) : QObject(parent)
                                                      , impl_( 0 )
                                                      , sink_( 0 ) 
                                                      , freq_( SignalObserver::Friquent )
                                                      , objId_(0) 
                                                      , connected_( false ) 
{
}

QObserverEvents_i::QObserverEvents_i( SignalObserver::Observer_ptr ptr
                                      , const std::string& token
                                      , SignalObserver::eUpdateFrequency freq 
                                      , QObject *parent)	 : QObject(parent)
                                                                 , impl_( SignalObserver::Observer::_duplicate(ptr) )
                                                                 , sink_( new ObserverEvents_i( *this ) ) 
                                                                 , token_( token ) 
                                                                 , freq_( freq )
                                                                 , objId_(0) 
{
    if ( ! CORBA::is_nil( impl_ ) ) {
        impl_->connect( sink_->_this(), freq_, token.c_str() );
        connected_ = true;
        objId_ = impl_->objId();
    }
}

void
QObserverEvents_i::disconnect()
{
    if ( ! CORBA::is_nil( impl_ ) && connected_ ) {
        connected_ = false;
        try {
            impl_->disconnect( sink_->_this() );
        } catch ( CORBA::Exception& ex ) {
            adportable::debug( __FILE__, __LINE__ ) << ex._info().c_str();
        }
        emit signal_OnClose();
    }
}

::SignalObserver::ObserverEvents *
QObserverEvents_i::_this (void)
{
    return sink_ ? sink_->_this() : 0;
}

void
QObserverEvents_i::onConfigChanged( CORBA::ULong objId, SignalObserver::eConfigStatus status )
{
    emit signal_ConfigChanged( objId, status );
}

void
QObserverEvents_i::onUpdateData( CORBA::ULong objId, CORBA::Long pos )
{
#if defined _DEBUG && 0 // emit UpdateData
    std::cout << "emit UpdateData(" << objId << ", " << pos << ")" << std::endl;
#endif
    emit signal_UpdateData( objId, pos );
}

void
QObserverEvents_i::onMethodChanged( CORBA::ULong objId, CORBA::Long pos )
{
    emit signal_MethodChanged( objId, pos );
}

void
QObserverEvents_i::onEvent( CORBA::ULong objId, CORBA::ULong event, CORBA::Long pos )
{
    emit signal_Event( objId, event, pos );
}

