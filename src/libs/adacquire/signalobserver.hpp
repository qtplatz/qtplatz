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

#pragma once

#include "octet_array.hpp"
#include "constants.hpp"
#include "adacquire_global.hpp"
#include <adutils/acquiredconf_v3.hpp>
#include <boost/any.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <compiler/pragma_warning.hpp>

namespace boost { namespace uuids { struct uuid; } }

namespace adacquire {

    class SampleProcessor;

    namespace SignalObserver {

        class DataReadBuffer;
        class Observer;
        class ObserverEvents;

#if defined _MSC_VER
        ADACQUIRESHARED_TEMPLATE_EXPORT template class ADACQUIRESHARED_EXPORT std::weak_ptr < DataReadBuffer > ;
        ADACQUIRESHARED_TEMPLATE_EXPORT template class ADACQUIRESHARED_EXPORT std::weak_ptr < Observer > ;
        ADACQUIRESHARED_TEMPLATE_EXPORT template class ADACQUIRESHARED_EXPORT std::weak_ptr < ObserverEvents > ;
#endif
        // typedef std::vector < uint8_t > octet_array;

        struct ADACQUIRESHARED_EXPORT Description {

            enum axis { axisX, axisY };

            Description();
            Description( const Description& );
            Description( const char * trace_id
                         , eTRACE_METHOD
                         , eSPECTROMETER
                         , const wchar_t * axis_label_x
                         , const wchar_t * axis_label_y
                         , int axis_decimals_x
                         , int axis_decimals_y );

            void set_objtext( const char * objtext );
            void set_objid( const boost::uuids::uuid& );

            eTRACE_METHOD trace_method() const;
            void set_trace_method( eTRACE_METHOD );

            eSPECTROMETER spectrometer() const;
            void set_spectrometer( eSPECTROMETER );

            const char * trace_id() const;
            void set_trace_id( const std::string& );

            const wchar_t * trace_display_name() const;
            void set_trace_display_name( const std::wstring& );

            const wchar_t * axis_label( axis ) const;
            void set_axis_label( axis, const std::wstring& );

            int32_t axis_decimals( axis ) const;
            void set_axis_decimals( axis, int32_t );

            const adutils::v3::AcquiredConf::data& data() const;

        private:
            pragma_msvc_warning_push_disable_4251
            adutils::v3::AcquiredConf::data conf_;
            pragma_msvc_warning_pop
        };

        class ADACQUIRESHARED_EXPORT DataReadBuffer : public std::enable_shared_from_this< DataReadBuffer > {

            DataReadBuffer( const DataReadBuffer& ) = delete;
            void operator = ( const DataReadBuffer& ) = delete;

        public:
            virtual ~DataReadBuffer();
            DataReadBuffer();

            template<typename T> void load( const T& t ) {
                elapsed_time_ = t.elapsed_time();
                epoch_time_ = t.epoch_time();
                pos_ = t.pos();
                fcn_ = t.pn();
                ndata_ = 1; // number of data in this packet, so that always 1 for waveform
                events_ = t.well_known_events();
            }

            uint64_t& timepoint();
            uint64_t& elapsed_time();
            uint64_t& epoch_time();
            uint32_t& pos();       // data address (sequencial number for first data in this frame)
            uint32_t& fcn();       // function number for spectrum
            uint32_t& ndata();     // number of data in the buffer (for trace, spectrum should be always 1)
            uint32_t& events();    // well known events
            octet_array& xdata();    // serialized data array
            octet_array& xmeta();    // serialized meta data array
            uint64_t timepoint() const;
            uint64_t elapsed_time() const;
            uint64_t epoch_time() const;
            uint32_t pos() const;       // data address (sequencial number for first data in this frame)
            uint32_t fcn() const;       // function number for spectrum
            uint32_t ndata() const;     // number of data in the buffer (for trace, spectrum should be always 1)
            uint32_t events() const;    // well known events

            const octet_array& xdata() const;    // serialized data array
            const octet_array& xmeta() const;    // serialized meta data array

            const boost::any& data() const;
            void setData( boost::any );

        private:
            uint64_t elapsed_time_;  // ns
            uint64_t epoch_time_;  // ns
            uint32_t pos_;         // data address (sequencial number for first data in this frame)
            uint32_t fcn_;         // function number for spectrum
            uint32_t ndata_;       // number of data in the buffer (for trace, spectrum should be always 1)
            uint32_t events_;      // well known events
            octet_array xdata_;    // encoded data array
            octet_array xmeta_;
            pragma_msvc_warning_push_disable_4251
            boost::any any_;
            pragma_msvc_warning_pop
        };

        class ADACQUIRESHARED_EXPORT ObserverEvents : public std::enable_shared_from_this< ObserverEvents > {
        public:
            // Master observer tells you if new device is up or down
            void OnConfigChanged( uint32_t objId, eConfigStatus status );

            // OnUpdateData tells you 'new data' at data number 'pos' is now ready to read
            void OnUpdateData( uint32_t objId, long pos );

            // OnMethodChanged tells you data monitor parameter has changed at data number 'pos'
            void OnMethodChanged( uint32_t objId, long pos );

            // well known event at data number 'pos'
            void OnEvent( uint32_t objId, uint32_t event, long pos );

            // new c++ based interface, equivalent to OnUpdateData
            virtual void onDataChanged( Observer *, uint32_t pos ) = 0;
        };

        class ADACQUIRESHARED_EXPORT Observer : public std::enable_shared_from_this< Observer > {
            class impl;
            impl * impl_;
        protected:
            std::mutex& mutex();
        public:
            virtual ~Observer();
            Observer();

            static boost::uuids::uuid& base_uuid();

            /** \brief getDescription returns description
             */
            virtual const Description& description() const;
            virtual void setDescription( const Description& desc );

            virtual const boost::uuids::uuid& objid() const = 0;
            virtual const char * objtext() const = 0;

            /** \brief client can monitor real time events when make a connection, it is optional
             */
            virtual bool connect( ObserverEvents * cb, eUpdateFrequency frequency, const std::string& token );
            virtual bool disconnect( ObserverEvents * cb );

            virtual bool isActive() const;
            virtual void setIsActive( bool );

            /** getSblings returns Observers, which share time base and events.
             *
             * Top level 'Observer' object is responcible to issue events 'OnUpdateData', 'OnEvent',
             * so application does not need to hookup events for shiblings.
             */
            virtual std::vector< std::shared_ptr< Observer > > siblings() const;

            /** Instrument controller will add/remove sibling by changing method while running sequence
             *  the method name should be more like 'addChild', actually this method add a link as child -- not sibling to this
             */
            virtual bool addSibling( Observer * observer );
            virtual Observer * findObserver( const boost::uuids::uuid&, bool recursive );

            /** uptime returns micro seconds since start moniring,
             * this number never reset to zero while running
             */
            virtual uint64_t uptime() const = 0;

            virtual void uptime_range( uint64_t& oldest, uint64_t& newest ) const { oldest = 0; newest = 0; }

            virtual std::shared_ptr< DataReadBuffer > readData( uint32_t pos ) = 0;

            /** dataInterpreterClsid tells you object location that knows data array structure
             * turn into class object.
             */
            virtual const char * dataInterpreterClsid() const  = 0;

            virtual int32_t posFromTime( uint64_t usec ) const { return 0; }

            /** preparing stroage for the sample.
             * This method give a chance to store device or sample specific data onto storage database at a preparing_for_run timing
             */
            virtual bool prepareStorage( SampleProcessor& ) const = 0;

            /** finalize stroage for the sample.
             * This method give a chance to store device or sample specific data onto storage database before close dababase
             */
            virtual bool closingStorage( SampleProcessor& ) const = 0;

            /** if instrument has one or more calibration information, adcontroller retrive them though this
             * interface start with idx = 0 until return false;  all data will be set to datainterpreter
             * and also save into data file under /Calibration folder
             */
            virtual bool readCalibration( int32_t idx, octet_array& serialized, std::string& dataClass ) const;

            /** \brief set the process method as serialzied octet stream
             */
            virtual bool setProcessMethod( const std::string& dataClass, octet_array& serialized ) const;

            /** \brief get the process method as serialzied octet stream which stroed in the observer object
             */
            virtual bool processMethod( const std::string& dataClass, octet_array& serialized );
        };

    };

}
