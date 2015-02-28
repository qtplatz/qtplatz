/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#ifndef DIGITIZER_HPP
#define DIGITIZER_HPP

#include "u5303a_global.hpp"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <functional>
#include <vector>
#include <memory>

namespace adcontrols { class ControlMethod; }

#if defined _MSC_VER
# pragma warning(disable:4251)
#endif

namespace u5303a {

    namespace detail { class task; }

    class U5303ASHARED_EXPORT identify {
    public:
        identify();
        identify( const identify& );
        std::string Identifier;
        std::string Revision;
        std::string Vendor;
        std::string Description;
        std::string InstrumentModel;
        std::string FirmwareRevision;
    private:
        friend class boost::serialization::access;
        template<class Archive>
            void serialize( Archive& ar, const unsigned int version ) {
            using namespace boost::serialization;
            (void)version;
            ar & BOOST_SERIALIZATION_NVP( Identifier );
            ar & BOOST_SERIALIZATION_NVP( Revision );
            ar & BOOST_SERIALIZATION_NVP( Vendor );
            ar & BOOST_SERIALIZATION_NVP( Description );
            ar & BOOST_SERIALIZATION_NVP( InstrumentModel );
            ar & BOOST_SERIALIZATION_NVP( FirmwareRevision );
        }
    };

	class U5303ASHARED_EXPORT method {
    public:
        method()
            : front_end_range( 2.0 )        // 1V,2V range
            , front_end_offset( 0.0 )       // [-0.5V,0.5V], [-1V,1V] offset
            , ext_trigger_level( 1.0 )      // external trigger threshold
			, samp_rate( 3.2e9 )			// sampling rate (3.2GS/s)
            , nbr_of_s_to_acquire( 100000 ) // from 1 to 480,000 samples
            , nbr_of_averages( 512 )		// number of averages minus one. >From 0 to 519,999 averages in steps of 8. For instance 0,7,15
            , delay_to_first_s( 0 )         // from 0 to 16,000,000 "blocks". Each block shifts by 10ns. 
            , invert_signal( 0 )            // 0-> no inversion , 1-> signal inverted
            , nsa( 0x0 ) {                  // bit[31]->enable, bits[11:0]->threshold
        }
        double front_end_range;
        double front_end_offset;
        double ext_trigger_level;
		double samp_rate; // HZ
        long nbr_of_s_to_acquire;
        long nbr_of_averages;
        long delay_to_first_s;
        long invert_signal;
        long nsa;
    private:
        friend class boost::serialization::access;
        template<class Archive>
            void serialize( Archive& ar, const unsigned int version ) {
            using namespace boost::serialization;
            (void)version;
            ar & BOOST_SERIALIZATION_NVP( front_end_range );
            ar & BOOST_SERIALIZATION_NVP( front_end_offset );
            ar & BOOST_SERIALIZATION_NVP( ext_trigger_level );
            ar & BOOST_SERIALIZATION_NVP( samp_rate );
            ar & BOOST_SERIALIZATION_NVP( nbr_of_s_to_acquire );
            ar & BOOST_SERIALIZATION_NVP( nbr_of_averages );
            ar & BOOST_SERIALIZATION_NVP( delay_to_first_s );
            ar & BOOST_SERIALIZATION_NVP( invert_signal );
            ar & BOOST_SERIALIZATION_NVP( nsa );
        }
    };

	class U5303ASHARED_EXPORT waveform : public std::enable_shared_from_this< waveform > {
		waveform( const waveform& ); // = delete;
		void operator = ( const waveform& ); // = delete;
	public:
		waveform() : actualElements_(0)
            , firstValidElement_(0)
            , wellKnownEvents_(0)
            , serialnumber_(0)
            , timestamp_(0)
            , actualAverages(0)
            , actualRecords(0)
            , initialXOffset(0)
            , xIncrement(0)
            , scaleFactor(0)
            , scaleOffset(0)
            , numPointsPerRecord(0) {
        }
        method method_;
        uint32_t serialnumber_;
        uint32_t wellKnownEvents_;
        uint64_t timestamp_;
        int64_t actualElements_;
        int64_t firstValidElement_;
        int32_t actualAverages;
        int64_t actualRecords;
        double initialXOffset;
        double xIncrement;
        double scaleFactor;
        double scaleOffset;
        int64_t numPointsPerRecord;
        std::vector< int32_t > d_;
    };

    class U5303ASHARED_EXPORT digitizer {
    public:
        digitizer();
        ~digitizer();

        bool peripheral_initialize();
        bool peripheral_prepare_for_run( const adcontrols::ControlMethod& );
        bool peripheral_run();
        bool peripheral_stop();
        bool peripheral_trigger_inject();
        bool peripheral_terminate();

        typedef std::function< void( const std::string, const std::string ) > command_reply_type;
        typedef std::function< void( const waveform * ) > waveform_reply_type;

        void connect_reply( command_reply_type ); // method,reply
        void disconnect_reply( command_reply_type );

        void connect_waveform( waveform_reply_type );
        void disconnect_waveform( waveform_reply_type );
    };

}

BOOST_CLASS_VERSION( u5303a::method, 1 )

#endif
