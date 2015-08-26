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

#include <memory>
#include <string>
#include <vector>

#include "adicontroller_global.hpp"

namespace adicontroler {

    namespace SignalObserver {

        struct DataReadBuffer;
        class Observer;

#if defined _MSC_VER
        ADICONTROLLERSHARED_TEMPLATE_EXPORT template class ADICONTROLLERSHARED_EXPORT std::vector < int8_t > ;
        ADICONTROLLERSHARED_TEMPLATE_EXPORT template class ADICONTROLLERSHARED_EXPORT std::weak_ptr < DataReadBuffer > ;
        ADICONTROLLERSHARED_TEMPLATE_EXPORT template class ADICONTROLLERSHARED_EXPORT std::weak_ptr < Observer > ;
#endif

        enum wkEvent {
            wkEvent_Error           = 0x80000000
            , wkEvent_Warning         = 0x40000000  // instrument is warning state
            , wkEvent_Marker          = 0x20000000  // wireing to 'marker trigger in'
            , wkEvent_INJECT          = 0x10000000  // wireing to 'inject trigger in'
            , wkEvent_MethodStart     = 0x08000000  // wireing to 'method start in'
            , wkEvent_DataWarning     = 0x04000000  // data waring such as input over range.
            , wkEvent_DarkInProgress  = 0x02000000  // dark signal acquiring
            , wkEvent_AcqInProgress   = 0x01000000  // Data storing, INJ trigger disarmed
            , wkEvent_UserEventsMask  = 0x00ffffff  // wiring to 'event in/out' terminal box
        };

        enum eTRACE_METHOD {
            eTRACE_TRACE
            , eTRACE_SPECTRA
            , eDIAGNOSTIC  // events, LC flow/pressure profile, column oven temp profile etc.
        };

        enum eSPECTROMETER {
            eUnknownSpectrometer
            , eMassSpectrometer
            , eUVSpectrometer
            , eCDSpectrometer
            , eIRSpectrometer
            , eRamanSpectrometer
            , eFluorescenceSpectrometer 
        };

        enum eUpdateFrequency {
            Realtime
            , Frequent
            , Sometimes
            , HalfFull
            , WellKnownEventsOnly
        };

        enum eConfigStatus {
            Offline
            , Online
        };

        struct timeval {
            uint64_t sec;
            uint32_t usec;
        };

        struct ADICONTROLLERSHARED_EXPORT Description {

            enum axis { axisX, axisY };

            Description();
            Description( const Description& );

            eTRACE_METHOD trace_method() const;
            void set_trace_method( eTRACE_METHOD );

            eSPECTROMETER spectrometer() const;
            void set_spectrometer( eSPECTROMETER );

            const char * trace_id() const;
            void set_trace_id( const std::string& );

            const char * axis_label( axis ) const;
            void set_axis_label( axis, const std::string& );

            int32_t axis_decimals( axis ) const;
            void set_axis_decimals( axis, int32_t );

        private:
            eTRACE_METHOD trace_method_;
            eSPECTROMETER spectrometer_;
#if defined _MSC_VER
# pragma warning( push )
# pragma warning( disable: 4251 )
#endif            
            std::string trace_id_;  // unique name for the trace, can be used as 'data storage name'
            std::string trace_display_name_;
            std::string axis_x_label_;
            std::string axis_y_label_;
#if defined _MSC_VER
# pragma warning( pop )
#endif            
            int32_t axis_x_decimals_;
            int32_t axis_y_decimals_;
        };

        struct ADICONTROLLERSHARED_EXPORT DataReadBuffer : public std::enable_shared_from_this< DataReadBuffer > {

            DataReadBuffer();
            DataReadBuffer( const DataReadBuffer& );

            uint32_t uptime;      // time in usec
            uint32_t pos;         // data address (sequencial number for first data in this frame)
            uint32_t fcn;         // function number for spectrum
            uint32_t ndata;       // number of data in the buffer (for trace, spectrum should be always 1)
            uint32_t events;      // well known events
            std::vector<int8_t> xdata;         // encoded data array
            std::vector<int8_t> xmeta;
        };
    
        class ADICONTROLLERSHARED_EXPORT ObserverEvents {
        public:
            // Master observer tells you if new device is up or down
            void OnConfigChanged( uint32_t objId, eConfigStatus status );

            // OnUpdateData tells you 'new data' at data number 'pos' is now ready to read
            void OnUpdateData( uint32_t objId, long pos );

            // OnMethodChanged tells you data monitor parameter has changed at data number 'pos'
            void OnMethodChanged( uint32_t objId, long pos );

            // well known event at data number 'pos'
            void OnEvent( uint32_t objId, uint32_t event, long pos );
        };

        class ADICONTROLLERSHARED_EXPORT Observer : public std::enable_shared_from_this< Observer > {
        public:
            virtual ~Observer();
            
            /** \brief getDescription returns description
             */
            virtual const Description& description() = 0; // used be, this was getDescription
            virtual bool setDescription( const Description& desc ) = 0;

            virtual uint32_t objId() const = 0;
            virtual void setObjId( uint32_t objid ) = 0;  // used be this was assign_ObjId

            /** \brief client can monitor real time events when make a connection, it is optional
             */
            virtual bool connect( ObserverEvents * cb, eUpdateFrequency frequency, const std::string& token ) = 0;
            virtual bool disconnect( ObserverEvents * cb ) = 0;

            virtual bool isActive() const = 0;

            /** getSblings returns Observers, which share time base and events.
             *
             * Top level 'Observer' object is responcible to issue events 'OnUpdateData', 'OnEvent', 
             * so application does not need to hookup events for shiblings.
             */
            virtual std::vector< std::shared_ptr< Observer > > siblings() = 0;  // getSiblings

            /** Instrument controller will add/remove sibling by changing method while running sequence
             */
            virtual bool addSibling( Observer * observer ) = 0;
            virtual Observer * findObserver( uint32_t objId, bool recursive ) = 0;

            /** uptime returns micro seconds since start moniring, 
             * this number never reset to zero while running
             */
            virtual void uptime( uint64_t usec ) = 0;

            virtual void uptime_range( uint64_t& oldest, uint64_t& newest ) = 0;

            virtual bool readData( long pos, std::shared_ptr< DataReadBuffer >& dataReadBuffer ) = 0;

            /** dataInterpreterClsid tells you object location that knows data array structure
             * can turns into meaningfull object such as spectrum or chromatogram
             * this std::string can be CLSID/GUID for Microsoft COM, 
             * CORBA object name or internal C++ class name that is core software can handle
             */
            virtual std::string dataInterpreterClsid() const  = 0;
            
            virtual long posFromTime( uint64_t usec ) const = 0;

            /** if instrument has one or more calibration information, adcontroller retrive them though this
             * interface start with idx = 0 until return false;  all data will be set to datainterpreter 
             * and also save into data file under /Calibration folder
             */
            virtual bool readCalibration( uint32_t idx, std::vector<int8_t>& serialized, std::string& dataClass ) const = 0;

            /** \brief set the process method as serialzied octet stream
             */
            virtual bool setProcessMethod( std::string dataClass, std::vector<int8_t>& serialized ) const = 0;

            /** \brief get the process method as serialzied octet stream which stroed in the observer object
             */
            virtual bool processMethod( const std::string& dataClass, std::vector<int8_t>& serialized ) = 0;
        };
    
    };

}
