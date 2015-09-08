/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "ap240_global.hpp"
#include <adcontrols/threshold_method.hpp>
#include <ap240spectrometer/method.hpp>
#include <ap240spectrometer/waveform.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <cstdint>
#include <functional>
#include <iostream>
#include <vector>
#include <memory>

namespace adcontrols { namespace ControlMethod { class Method; } }
namespace adportable { class TimeSquaredScanLaw; }

#if defined _MSC_VER
# pragma warning(disable:4251)
#endif

namespace ap240 {

    namespace detail { class task; struct device_ap240; }

	class AP240SHARED_EXPORT device_data {
    public:
        ap240x::identify ident;
        ap240x::metadata meta;
    private:
        friend class boost::serialization::access;
        template<class Archive>
            void serialize( Archive& ar, const unsigned int ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( ident );
            ar & BOOST_SERIALIZATION_NVP( meta );           
        }
    };
#if 0
	class AP240SHARED_EXPORT metadata_archive {
    public:
        metadata_archive( const ap240x::identify& id ) {
        }
        ap240x::identify ident_;
        std::vector< ap240x::metadata > meta_;
        std::shared_ptr< ap240x::method > method_;
    private:        
        friend class boost::serialization::access;
        template<class Archive>
            void serialize( Archive& ar, const unsigned int ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( ident_ );
            ar & BOOST_SERIALIZATION_NVP( meta_ );           
        }
    };
#endif
    class AP240SHARED_EXPORT digitizer {
    public:
        digitizer();
        ~digitizer();

        bool peripheral_initialize();
        bool peripheral_prepare_for_run( const ap240x::method& );
        bool peripheral_run();
        bool peripheral_stop();
        bool peripheral_trigger_inject();
        bool peripheral_terminate();
        void setScanLaw( std::shared_ptr< adportable::TimeSquaredScanLaw > );

        typedef std::function< void( const std::string, const std::string ) > command_reply_type;
        typedef std::function< bool( const ap240x::waveform *, const ap240x::waveform *, ap240x::method& ) > waveform_reply_type;

        void connect_reply( command_reply_type ); // method,reply
        void disconnect_reply( command_reply_type );

        void connect_waveform( waveform_reply_type );
        void disconnect_waveform( waveform_reply_type );
        //-------
        // bool findDevice();
        // bool initial_setup();
        // bool acquire();
        // bool stop();
        enum result_code { success, error_timeout, error_overload, error_io_read, error_stopped };
        result_code waitForEndOfAcquisition( size_t timeout );
    };

}

//BOOST_CLASS_VERSION( ap240::method, 2 )
//BOOST_CLASS_VERSION( adcontrols::threshold_method, 2 )
//BOOST_CLASS_VERSION( ap240::metadata, 1 )
//BOOST_CLASS_VERSION( ap240::identify, 1 )

#endif
