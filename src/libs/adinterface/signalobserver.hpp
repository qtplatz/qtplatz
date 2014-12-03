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

#include <string>

namespace signalobserver {

    const unsigned long wkEvent_Error           = 0x80000000;
    const unsigned long wkEvent_Warning         = 0x40000000;  // instrument is warning state
    const unsigned long wkEvent_Marker          = 0x20000000;  // wireing to 'marker trigger in'
    const unsigned long wkEvent_INJECT          = 0x10000000;  // wireing to 'inject trigger in'
    const unsigned long wkEvent_MethodStart     = 0x08000000;  // wireing to 'method start in'
    const unsigned long wkEvent_DataWarning     = 0x04000000;  // data waring such as input over range.
    const unsigned long wkEvent_DarkInProgress  = 0x02000000;  // dark signal acquiring
    const unsigned long wkEvent_AcqInProgress   = 0x01000000;  // Data storing, INJ trigger disarmed
    const unsigned long wkEvent_UserEventsMask  = 0x00ffffff;  // wiring to 'event in/out' terminal box

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

    typedef std::vector< uint8_t > octet_array;

    struct DataReadBuffer {
        unsigned long long uptime; // time in usec
        unsigned long pos;         // data address (sequencial number for first data in this frame)
        unsigned long fcn;         // function number for spectrum
        unsigned long ndata;       // number of data in the buffer (for trace, spectrum should be always 1)
        unsigned long events;      // well known events
        octet_array xdata;         // encoded data array
        octet_array xmeta;
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
    };
#if 0
    class Observer {
        const Description& getDescription() const;
        boolean setDescription( const Description& desc );
        unsigned long objId();
        void assign_objId( in unsigned long oid );
        boolean isActive();
        Observers getSiblings();
        boolean addSibling( in Observer observer );
        Observer findObserver( in unsigned long objId, in boolean recursive );
        void uptime( out unsigned long long usec );
        void uptime_range( out unsigned long long oldest, out unsigned long long newest );
        boolean readData( in long pos, out DataReadBuffer dataReadBuffer );
        wstring dataInterpreterClsid();
        long posFromTime( in unsigned long long usec );
		boolean readCalibration( in unsigned long idx, out octet_array serialized, out wstring dataClass );
    };
#endif
}

