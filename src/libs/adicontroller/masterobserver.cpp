// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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

#include "masterobserver.hpp"
#include <adportable/debug.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace adicontroller {

    class MasterObserver::impl {
    public:
        std::string objtext_;
        boost::uuids::uuid uuid_;
        std::mutex mutex_;
        typedef std::tuple< adicontroller::SignalObserver::ObserverEvents *
                            , std::string
                            , adicontroller::SignalObserver::eUpdateFrequency
                            , bool > client_type;
        
        std::vector< client_type > clients_;
        
        impl( const char * objtext ) : objtext_( objtext ? objtext : "" )
                                     , uuid_( { 0 } ) {
        }

        ~impl()  {
        }
        
    };
    
}

using namespace adicontroller;

MasterObserver::MasterObserver( const char * objtext ) : impl_( new impl( objtext ) )
{
    if ( objtext ) {
        impl_->uuid_ = boost::uuids::name_generator( adicontroller::SignalObserver::Observer::base_uuid() )( objtext );
        so::Description desc;
        desc.set_objid( impl_->uuid_ );
        desc.set_objtext( objtext );
        setDescription( desc );
    }
}

MasterObserver::~MasterObserver()
{
    delete impl_;
}

bool
MasterObserver::connect( so::ObserverEvents * cb, so::eUpdateFrequency freq, const std::string& token )
{
    if ( cb ) {
        std::lock_guard< std::mutex > lock( impl_->mutex_ );
        impl_->clients_.push_back( std::make_tuple( cb, token, freq, true ) );
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

const boost::uuids::uuid&
MasterObserver::objid() const
{
    return impl_->uuid_;
}

const char *
MasterObserver::objtext() const
{
    return impl_->objtext_.c_str();
}

uint64_t
MasterObserver::uptime() const
{
    return 0;
}

std::shared_ptr< so::DataReadBuffer >
MasterObserver::readData( uint32_t pos )
{
    return 0;
}

const char *
MasterObserver::dataInterpreterClsid() const
{
    return 0;
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

bool
MasterObserver::prepareStorage( SampleProcessor& sp ) const
{
    for ( auto observer : siblings() ) {
        observer->prepareStorage( sp );
    }
    return true;
}

bool 
MasterObserver::closingStorage( SampleProcessor& sp ) const
{
    for ( auto observer : siblings() ) {
        observer->closingStorage( sp );
    }
    return true;
}
