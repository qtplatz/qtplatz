/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com
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


#if defined _MSC_VER
# pragma warning(push)
# pragma warning(disable:4996)
#endif

#include "signalobserver.hpp"
#include <adutils/acquiredconf.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <atomic>
#include <algorithm>
#include <chrono>
#include <mutex>

namespace adacquire {

    namespace SignalObserver {
        
        Description::Description()
        {
            conf_.objid = { {0} };
            conf_.pobjid = { {0} };
            conf_.spectrometer = eUnknownSpectrometer;
			conf_.trace_method = eTRACE_SPECTRA;
            conf_.axis_decimals_x = 2;
            conf_.axis_decimals_y = 0;
        }
        
        Description::Description( const Description& t ) : conf_( t.conf_ )
        {
        }

        Description::Description( const char * trace_id
                                  , eTRACE_METHOD trace_method
                                  , eSPECTROMETER spectrometer
                                  , const wchar_t * axis_label_x
                                  , const wchar_t * axis_label_y
                                  , int axis_decimals_x
                                  , int axis_decimals_y ) {
            conf_.trace_method = trace_method;
            conf_.spectrometer = spectrometer;
            conf_.axis_label_x = axis_label_x;
            conf_.axis_label_y = axis_label_y;
            conf_.axis_decimals_x = axis_decimals_x;
            conf_.axis_decimals_y = axis_decimals_y;
            conf_.objid = { {0} };
            conf_.pobjid = { {0} };
        }

        void
        Description::set_objtext( const char * objtext )
        {
            conf_.objtext = objtext;
        }
        
        void
        Description::set_objid( const boost::uuids::uuid& objid )
        {
            conf_.objid = objid;
        }
        
        eTRACE_METHOD
        Description::trace_method() const
        {
            return eTRACE_METHOD( conf_.trace_method );
        }
        
        void
        Description::set_trace_method( eTRACE_METHOD v )
        {
            conf_.trace_method = int(v);
        }
        
        eSPECTROMETER
        Description::spectrometer() const
        {
            return eSPECTROMETER( conf_.spectrometer );
        }

        void
        Description::set_spectrometer( eSPECTROMETER v )
        {
            conf_.spectrometer = int( v );
        }
        
        const char *
        Description::trace_id() const
        {
            return conf_.trace_id.c_str();
        }
        
        void
        Description::set_trace_id( const std::string& v )
        {
            conf_.trace_id = v;
        }

        const wchar_t *
        Description::trace_display_name() const
        {
            return conf_.trace_display_name.c_str();
        }
        
        void
        Description::set_trace_display_name( const std::wstring& v )
        {
            conf_.trace_display_name = v;
        }
        
        const wchar_t *
        Description::axis_label( axis id ) const
        {
            if ( id == axisX )
                return conf_.axis_label_x.c_str();
            else
                return conf_.axis_label_y.c_str();
        }

        void
        Description::set_axis_label( axis id, const std::wstring& v )
        {
            if ( id == axisX )
                conf_.axis_label_x = v;
            else
                conf_.axis_label_y = v;
        }
        
        int32_t
        Description::axis_decimals( axis id ) const
        {
            return id == axisX ? conf_.axis_decimals_x : conf_.axis_decimals_y;
        }

        void
        Description::set_axis_decimals( axis id, int32_t v )
        {
            if ( id == axisX )
                conf_.axis_decimals_x = v;
            else
                conf_.axis_decimals_y = v;
        }

        const adutils::v3::AcquiredConf::data&
        Description::data() const
        {
            return conf_;
        }

        ///////
        DataReadBuffer::DataReadBuffer() : elapsed_time_( 0 ) //: impl_( new impl() )
                                         , epoch_time_( 0 )
                                         , pos_( 0 )
                                         , fcn_( 0 )
                                         , ndata_( 0 )
                                         , events_( 0 )
        {
        }

        DataReadBuffer::~DataReadBuffer()
        {
            //delete impl_;
        }
        
        uint64_t& DataReadBuffer::timepoint()    { return epoch_time_; }
        uint64_t& DataReadBuffer::epoch_time()   { return epoch_time_; }
        uint64_t& DataReadBuffer::elapsed_time() { return elapsed_time_; }
        uint32_t& DataReadBuffer::pos()          { return pos_; }
        uint32_t& DataReadBuffer::fcn()          { return fcn_; }
        uint32_t& DataReadBuffer::ndata()        { return ndata_; }
        uint32_t& DataReadBuffer::events()       { return events_; }
        octet_array& DataReadBuffer::xdata()     { return xdata_; }
        octet_array& DataReadBuffer::xmeta()     { return xmeta_; }     
        
        uint64_t DataReadBuffer::timepoint() const       { return epoch_time_; }
        uint64_t DataReadBuffer::epoch_time() const      { return epoch_time_; }
        uint64_t DataReadBuffer::elapsed_time() const    { return elapsed_time_; }
        uint32_t DataReadBuffer::pos() const             { return pos_; }       
        uint32_t DataReadBuffer::fcn() const             { return fcn_; }       
        uint32_t DataReadBuffer::ndata() const           { return ndata_; }     
        uint32_t DataReadBuffer::events() const          { return events_; }    
        const octet_array& DataReadBuffer::xdata() const { return xdata_; }     
        const octet_array& DataReadBuffer::xmeta() const { return xmeta_; }
        const boost::any& DataReadBuffer::data() const   { return any_; }
        void DataReadBuffer::setData( boost::any d )     { any_ = d; }

        ///////
        
        class Observer::impl {
        public:
            impl() : objId_( 0 )
                   , isActive_( false ) {
            }

            std::atomic< uint32_t > objId_;
            std::atomic< bool > isActive_;
            std::chrono::steady_clock::time_point tp_origin_;
            std::chrono::steady_clock::time_point tp_last_event_;
            Description description_;
            std::vector< std::shared_ptr< Observer > > siblings_;
            std::mutex mutex_;
        };
        
        Observer::~Observer()
        {
            delete impl_;
        }

        Observer::Observer() : impl_( new impl() )
        {
        }

        std::mutex&
        Observer::mutex()
        {
            return impl_->mutex_;
        }

        bool
        Observer::connect( ObserverEvents * cb, eUpdateFrequency frequency, const std::string& token )
        {
            return false;
        }

        bool
        Observer::disconnect( ObserverEvents * cb )
        {
            return false;
        }

        const Description&
        Observer::description() const
        {
            return impl_->description_;
        }
        
        void
        Observer::setDescription( const Description& v )
        {
            impl_->description_ = v;
        }

        bool
        Observer::isActive() const
        {
            return impl_->isActive_;
        }

        void
        Observer::setIsActive( bool v )
        {
            impl_->isActive_ = v;
        }

        std::vector< std::shared_ptr< Observer > >
        Observer::siblings() const
        {
            std::lock_guard< std::mutex > lock( impl_->mutex_ );
            return impl_->siblings_;
        }

        bool
        Observer::addSibling( Observer * observer )
        {
            if ( observer ) {
                assert( observer->objid() != boost::uuids::uuid( { {0} } ) );
                std::lock_guard< std::mutex > lock( impl_->mutex_ );
                impl_->siblings_.push_back( observer->shared_from_this() );
                return true;
            }
            return false;
        }

        Observer *
        Observer::findObserver( const boost::uuids::uuid& uuid, bool recursive )
        {
            std::lock_guard< std::mutex > lock( impl_->mutex_ );            

            auto it = std::find_if( impl_->siblings_.begin(), impl_->siblings_.end(), [uuid]( const std::shared_ptr< Observer >& p ){
                    return p->objid() == uuid; });

            if ( it != impl_->siblings_.end() )
                return it->get();

            if ( recursive ) {
                for ( auto& sibling: impl_->siblings_ ) {
                    if ( auto p = sibling->findObserver( uuid, recursive ) )
                        return p;
                }
            }
            
            return 0;
        }

        bool
        Observer::readCalibration( int32_t idx, octet_array& serialized, std::string& dataClass ) const
        {
            return false;
        }

        bool
        Observer::setProcessMethod( const std::string& dataClass, octet_array& serialized ) const
        {
            return false;
        }

        bool
        Observer::processMethod( const std::string& dataClass, octet_array& serialized )
        {
            return false;
        }

        // static
        boost::uuids::uuid&
        Observer::base_uuid()
        {
            static boost::uuids::uuid uuidx = boost::uuids::string_generator()( "{6AE63365-1A4D-4504-B0CD-38AE86309F83}" );
            return uuidx; // 
        }
        
    };

}


