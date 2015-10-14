/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@scienceliaison.com
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

#include "masterobserver.hpp"
#include "masterobserverevents.hpp"
#include "document.hpp"
#include <adportable/debug.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <algorithm>
#include <tuple>
#include <mutex>

namespace acquire {
    
    class MasterObserver::impl {
    public:
        impl() {
        }
        
        const char * objtext__ = "acquire.ms-cheminfo.com";

        const boost::uuids::uuid objid_ = boost::uuids::name_generator( adicontroller::SignalObserver::Observer::base_uuid() )( objtext__ );
        std::mutex mutex_;
        typedef std::tuple< adicontroller::SignalObserver::ObserverEvents *
                            , std::string
                            , adicontroller::SignalObserver::eUpdateFrequency
                            , bool > client_type;
        
        std::vector< client_type > clients_;

        MasterObserverEvents * masterObserverEvents() {
            static std::once_flag flag;
            std::call_once( flag, [this] () {
                    observerEvents_ = std::make_shared< MasterObserverEvents >( document::instance()->masterController() ); } );
            return observerEvents_.get();
        }
        
    private:
        std::shared_ptr< MasterObserverEvents > observerEvents_;
    };

}

using namespace acquire;

MasterObserver::MasterObserver() : impl_( new impl() )
{
}

MasterObserver::~MasterObserver()
{
    delete impl_;
}

bool
MasterObserver::addSibling( Observer * observer )
{
    observer->connect( impl_->masterObserverEvents(), adicontroller::SignalObserver::Realtime, "acquire::MasterObserver" );

    return adicontroller::SignalObserver::Observer::addSibling( observer );
}

const boost::uuids::uuid&
MasterObserver::objid() const
{
    return impl_->objid_;
}

const char *
MasterObserver::objtext() const
{
    return impl_->objtext__;
}


bool
MasterObserver::connect( so::ObserverEvents * cb, so::eUpdateFrequency frequency, const std::string& token )
{
    if ( cb ) {
        std::lock_guard< std::mutex > lock( impl_->mutex_ );
        impl_->clients_.push_back( std::make_tuple( cb, token, frequency, true ) );
        return true;
    }
    return false;
}
    
bool
MasterObserver::disconnect( so::ObserverEvents * cb )
{
    if ( cb ) {
        std::lock_guard< std::mutex > lock( impl_->mutex_ );
        auto it = std::find_if( impl_->clients_.begin(), impl_->clients_.end(), [cb]( const impl::client_type& a ){
                return std::get<0>(a) == cb;
            });
        if ( it != impl_->clients_.end() )
            impl_->clients_.erase( it );
        return true;
    }
    return false;
}

void
MasterObserver::dataChanged( adicontroller::SignalObserver::Observer * so, uint32_t pos )
{
    if ( so ) {
        std::lock_guard< std::mutex > lock( impl_->mutex_ );
        std::for_each( impl_->clients_.begin(), impl_->clients_.end(), [so,pos] ( const impl::client_type& a ) {
                std::get<0>( a )->onDataChanged( so, pos );
            });
    }
}


