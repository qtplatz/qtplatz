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

#include "task.hpp"
#include "oproxy.hpp"

using namespace adcontroller;

oProxy::~oProxy()
{
}

oProxy::oProxy( iTask& t ) : objref_(false)
                           , objId_(0) 
                           , task_( t )
{
}

void
oProxy::OnConfigChanged ( ::CORBA::ULong objId, ::SignalObserver::eConfigStatus status )
{
    (void)objId;
    (void)status;
}

void
oProxy::OnUpdateData ( ::CORBA::ULong objId, ::CORBA::Long pos )
{
    task_.observer_update_data( this->objId_, objId, pos );
}

void
oProxy::OnMethodChanged ( ::CORBA::ULong objId, ::CORBA::Long pos )
{
    task_.observer_update_method( this->objId_, objId, pos );
}

void
oProxy::OnEvent ( ::CORBA::ULong objId, ::CORBA::ULong events, ::CORBA::Long pos )
{
    task_.observer_update_event( this->objId_, objId, pos, events );
}

bool
oProxy::connect( const std::string& token )
{
    if ( objref_ )
        return impl_->connect( _this(), SignalObserver::Realtime, token.c_str() );
    return false;
}

bool
oProxy::disconnect()
{
    if ( objref_ )
        return impl_->disconnect( _this() );
    return false;
}

bool
oProxy::initialize()
{
    return true;
}

bool
oProxy::setInstrumentSession( Instrument::Session_ptr iSession )
{
    objref_ = false;
    
    CORBA::release( iSession_ );
    CORBA::release( impl_ );
    
    iSession_ = iSession;
    if ( ! CORBA::is_nil( iSession_ ) ) {
        impl_ = iSession_->getObserver();
        if ( ! CORBA::is_nil( impl_.in() ) ) {
            objref_ = true;
            impl_->assign_objId( objId_ );
        }
    }
    return objref_;
}

size_t
oProxy::populateObservers( unsigned long objid )
{
    if ( CORBA::is_nil( impl_.in() ) )
        return 0;
    
    size_t nsize = 0;
    SignalObserver::Observers_var vec = impl_->getSiblings();
    if ( ( vec.ptr() != 0) && ( nsize = vec->length() ) > 0 ) {
        for ( size_t i = 0; i < nsize; ++i )
            vec[i]->assign_objId( ++objid );
    }
    return nsize;
}

void
oProxy::setConfiguration( const adportable::Configuration& c )
{
    config_ = c;
    std::wstring id = c.attribute( L"id" );
}

unsigned long
oProxy::objId() const
{
    return objId_;
}

void
oProxy::objId( unsigned long objid )
{
    objId_ = objid;
}

SignalObserver::Observer_ptr
oProxy::getObject()
{
    return impl_.in();
}
