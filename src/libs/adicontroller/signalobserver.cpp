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
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <atomic>
#include <algorithm>
#include <chrono>
#include <mutex>

namespace adicontroller {

    namespace SignalObserver {

        Description::Description() : trace_method_( eTRACE_TRACE )
                                   , spectrometer_( eUnknownSpectrometer )
                                   , axis_x_decimals_( 2 )
                                   , axis_y_decimals_( 0 )
        {
        }

        Description::Description( const Description& t ) : trace_method_( t.trace_method_ )
                                                         , spectrometer_( t.spectrometer_ )
                                                         , trace_id_( t.trace_id_ )
                                                         , trace_display_name_( t.trace_display_name_ )
                                                         , axis_x_label_( t.axis_x_label_ )
                                                         , axis_y_label_( t.axis_y_label_ )
                                                         , axis_x_decimals_( t.axis_x_decimals_ )
                                                         , axis_y_decimals_( t.axis_y_decimals_ )
        {
        }

        eTRACE_METHOD
        Description::trace_method() const
        {
            return trace_method_;
        }

        void
        Description::set_trace_method( eTRACE_METHOD v )
        {
            trace_method_ = v;
        }
        
        eSPECTROMETER
        Description::spectrometer() const
        {
            return spectrometer_;
        }

        void
        Description::set_spectrometer( eSPECTROMETER v )
        {
            spectrometer_ = v;
        }
        
        const char *
        Description::trace_id() const
        {
            return trace_id_.c_str();
        }

        void
        Description::set_trace_id( const std::string& v )
        {
            trace_id_ = v;
        }

        const char *
        Description::trace_display_name() const
        {
            return trace_display_name_.c_str();
        }
        
        void
        Description::set_trace_display_name( const std::string& v )
        {
            trace_display_name_ = v;
        }
        
        const char *
        Description::axis_label( axis id ) const {
            if ( id == axisX )
                return axis_x_label_.c_str();
            else
                return axis_y_label_.c_str();
        }

        void
        Description::set_axis_label( axis id, const std::string& v ) {
            if ( id == axisX )
                axis_x_label_ = v;
            else
                axis_y_label_ = v;
        }
        
        int32_t
        Description::axis_decimals( axis id ) const
        {
            return id == axisX ? axis_x_decimals_ : axis_y_decimals_;
        }

        void
        Description::set_axis_decimals( axis id, int32_t v )
        {
            if ( id == axisX ) axis_x_decimals_ = v; else axis_y_decimals_ = v;
        }

        ///////
        class DataReadBuffer::impl {
        public:
            impl() : timepoint_( 0 )
                   , pos_( 0 )
                   , fcn_( 0 )
                   , ndata_( 0 )
                   , events_( 0 ) {
            }
            uint64_t timepoint_;   // time since epoch, in nanoseconds
            uint32_t pos_;         // data address (sequencial number for first data in this frame)
            uint32_t fcn_;         // function number for spectrum
            uint32_t ndata_;       // number of data in the buffer (for trace, spectrum should be always 1)
            uint32_t events_;      // well known events
            octet_array xdata_;
            octet_array xmeta_;
        };
        
        DataReadBuffer::DataReadBuffer() : impl_( new impl() )
        {
        }

        DataReadBuffer::~DataReadBuffer()
        {
            delete impl_;
        }
        
        uint64_t& DataReadBuffer::timepoint()  { return impl_->timepoint_; }
        uint32_t& DataReadBuffer::pos()        { return impl_->pos_; }
        uint32_t& DataReadBuffer::fcn()        { return impl_->fcn_; }
        uint32_t& DataReadBuffer::ndata()      { return impl_->ndata_; }
        uint32_t& DataReadBuffer::events()     { return impl_->events_; }
        octet_array& DataReadBuffer::xdata()   { return impl_->xdata_; }
        octet_array& DataReadBuffer::xmeta()   { return impl_->xmeta_; }     

        uint64_t DataReadBuffer::timepoint() const       { return impl_->timepoint_; } 
        uint32_t DataReadBuffer::pos() const             { return impl_->pos_; }       
        uint32_t DataReadBuffer::fcn() const             { return impl_->fcn_; }       
        uint32_t DataReadBuffer::ndata() const           { return impl_->ndata_; }     
        uint32_t DataReadBuffer::events() const          { return impl_->events_; }    
        const octet_array& DataReadBuffer::xdata() const { return impl_->xdata_; }     
        const octet_array& DataReadBuffer::xmeta() const { return impl_->xmeta_; }     

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

        uint32_t
        Observer::objId() const
        {
            return impl_->objId_;
        }

        void
        Observer::setObjId( uint32_t objid )
        {
            impl_->objId_ = objid;
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
        Observer::siblings()
        {
            std::lock_guard< std::mutex > lock( impl_->mutex_ );
            return impl_->siblings_;
        }

        bool
        Observer::addSibling( Observer * observer )
        {
            if ( observer ) {
                std::lock_guard< std::mutex > lock( impl_->mutex_ );
                impl_->siblings_.push_back( observer->shared_from_this() );
                return true;
            }
            return false;
        }

        Observer *
        Observer::findObserver( uint32_t objId, bool recursive )
        {
            std::lock_guard< std::mutex > lock( impl_->mutex_ );            

            auto it = std::find_if( impl_->siblings_.begin(), impl_->siblings_.end(), [objId]( const std::shared_ptr< Observer >& p ){
                    return p->objId() == objId; });

            if ( it != impl_->siblings_.end() )
                return it->get();

            if ( recursive ) {
                for ( auto& sibling: impl_->siblings_ ) {
                    if ( auto p = sibling->findObserver( objId, recursive ) )
                        return p;
                }
            }
            
            return 0;
        }

        Observer *
        Observer::findObserver( const boost::uuids::uuid& uuid, bool recursive )
        {
            std::lock_guard< std::mutex > lock( impl_->mutex_ );            

            auto it = std::find_if( impl_->siblings_.begin(), impl_->siblings_.end(), [uuid]( const std::shared_ptr< Observer >& p ){
                    return p->uuid() == uuid; });

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
