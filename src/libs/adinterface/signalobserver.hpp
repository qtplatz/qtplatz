/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <vector>
#include <algorithm>

namespace signalobserver {

    const unsigned long wkEvent_Error           = 0x80000000;
    const unsigned long wkEvent_Warning         = 0x40000000;  // instrument is warning state
    const unsigned long wkEvent_Marker          = 0x20000000;  // wireing to 'marker trigger in'
    const unsigned long wkEvent_INJECT          = 0x10000000;  // wireing to 'inject trigger in'
    const unsigned long wkEvent_MethodStart     = 0x08000000;  // wireing to 'method start in'
    const unsigned long wkEvent_DataWarning     = 0x04000000;  // data waring such as input over range.
    const unsigned long wkEvent_DarkInProgress  = 0x02000000;  // dark signal acquiring
    const unsigned long wkEvent_AcqInProgress   = 0x01000000;  // Recording data, INJ trigger disarmed
    const unsigned long wkEvent_UserEventsMask  = 0x00ffffff;  // wiring to 'event in/out' terminal box

    enum eTRACE_METHOD {
        eTRACE_TRACE             // Series of intensities such as common LC/GC detectors (UV/Vis, RI, FP, FID, ECD ...)
        , eTRACE_SPECTRA         // Series of spectra, e.g. LC/MS, Diode array detector
		, eDIAGNOSTIC            // events, LC flow/pressure profile, column oven temp profile etc.
        , eTRACE_IMAGE_TDC       // Series of 2-D image frames, in time domain (raw data from TDC array, such as MALPIX)
        , eTRACE_IMAGE_SPECTRA   // Serial of 2-D (surface giometric) imaging spectra
        , eTRACE_IMAGE_INTENSITY // Serias of 2-D image frames, in intensity domain, such as total ion count image map
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

    typedef std::vector< uint8_t > octet_array;

    struct DataReadBuffer {
        uint64_t uptime; // time in usec
        uint32_t pos;         // data address (sequencial number for first data in this frame)
        uint32_t fcn;         // function number for spectrum
        uint32_t ndata;       // number of data in the buffer (for trace, spectrum should be always 1)
        uint32_t events;      // well known events
        octet_array xdata;         // encoded data array
        octet_array xmeta;
        DataReadBuffer() : uptime(0), pos(0), fcn(0), ndata(0), events(0) {}
        DataReadBuffer( const DataReadBuffer& ) = delete;
        void operator = ( const DataReadBuffer& ) = delete;
    };

    struct Description {
        eTRACE_METHOD trace_method;
        eSPECTROMETER spectrometer;
        std::wstring trace_id;  // unique name for the trace, can be used as 'data storage name'
        std::wstring trace_display_name;
        std::wstring axis_x_label;
        std::wstring axis_y_label;
        int32_t axis_x_decimals;
        int32_t axis_y_decimals;
        Description() : trace_method( eTRACE_TRACE ), spectrometer( eUnknownSpectrometer )
                      , axis_x_decimals( 0 ), axis_y_decimals( 0 )
            {}
        Description( const Description& t ) : trace_method( t.trace_method )
                                            , spectrometer( t.spectrometer )
                                            , trace_id( t.trace_id )
                                            , trace_display_name( t.trace_display_name )
                                            , axis_x_decimals( t.axis_x_decimals )
                                            , axis_y_decimals( t.axis_y_decimals )
            {}
    };

    class Observer;

    typedef std::shared_ptr< Observer > ObserverPtr;
    typedef std::vector < ObserverPtr > Observers;

    class Observer : std::enable_shared_from_this< Observer > {
        Observer( const Observer& ) = delete;
        void operator = ( const Observer& ) = delete;
    public:
        Observer() : objId_( -1 ) {};
        virtual ~Observer() {}
        
        virtual const Description& getDescription() const      { return desc_; }
        
        virtual void setDescription( const Description& desc ) { desc_ = desc; }
        
        virtual uint32_t objId() const                         { return objId_; }
        
        virtual void assign_objId( uint32_t oid )              { objId_ = oid; }
        
        virtual bool isActive() const                          { return false; }

        virtual bool addSibling( Observer * observer )         {
            std::lock_guard< std::mutex > lock( mutex_ );
            siblings_.push_back( observer->shared_from_this() );
            return true;
        }

        /** find observer underneath
         */
        virtual Observer * findObserver( uint32_t objId, bool recursive ) {
            std::lock_guard< std::mutex > lock( mutex_ );
            auto it = std::find_if( siblings_.begin(), siblings_.end(), [=]( const ObserverPtr& o ){ return o->objId() == objId; });
            if ( it != siblings_.end() )
                return (*it).get();
            if ( recursive ) {
                for( auto& o: siblings_ )
                    if ( auto obs = o->findObserver( objId, true ) )
                        return obs;
            }
            return 0;
        }

        /** Current time in microseconds since hardware/firmware powered on.
         */
        virtual uint64_t uptime() const = 0;
        
        virtual void uptime_range( uint64_t& oldest, uint64_t& newest ) const = 0;
        
        virtual std::shared_ptr< DataReadBuffer > readData( uint32_t pos ) = 0;

        virtual const wchar_t * dataInterpreterClsid() const = 0;
        
        virtual int32_t posFromTime( uint64_t usec ) const = 0;

        /** Read hardware/firmware holding calibration that will be stored on file header by acquisition manager object
         *  such as 'adcontroller'
         */
		virtual bool readCalibration( int idx, octet_array& serialized, std::wstring& dataClass ) const = 0;
    protected:
        uint32_t objId_;
        Description desc_;
        Observers siblings_;
        std::mutex mutex_;
    };

}

