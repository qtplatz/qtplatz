/**************************************************************************
** Copyright (C) 2010-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2020 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include "aqmd3_global.hpp"
#include <aqmd3controls/identify.hpp>
#include <aqmd3controls/meta_data.hpp>
#include <aqmd3controls/method.hpp>
#include <aqmd3controls/waveform.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/any.hpp>
#include <cstdint>
#include <functional>
#include <vector>
#include <memory>

namespace adcontrols { namespace ControlMethod { class Method; } }
namespace adportable { class TimeSquaredScanLaw; }
namespace acqrscontrols { namespace u5303a { class method; class device_method; } }

namespace aqmd3 {

    class AqMD3;

    namespace detail { class task; }

	class AQMD3SHARED_EXPORT device_data {
    public:
        aqmd3controls::identify ident;
        aqmd3controls::meta_data meta;
    private:
        friend class boost::serialization::access;
        template<class Archive>
            void serialize( Archive& ar, const unsigned int ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( ident );
            ar & BOOST_SERIALIZATION_NVP( meta );
        }
    };

    ///////////
    class AQMD3SHARED_EXPORT digitizer {
    public:
        digitizer();
        ~digitizer();

        bool peripheral_initialize();
        bool peripheral_prepare_for_run( const adcontrols::ControlMethod::Method& );
        bool peripheral_prepare_for_run( const aqmd3controls::method& );
        bool peripheral_run();
        bool peripheral_stop();
        bool peripheral_trigger_inject();
        bool peripheral_terminate();
        bool peripheral_dark( size_t waitCount );

        void setScanLaw( std::shared_ptr< adportable::TimeSquaredScanLaw > );

        typedef void (command_handler_type)( const std::string, const std::string );

        typedef std::function< command_handler_type > command_reply_type;

        typedef bool (waveform_handler_type)( const aqmd3controls::waveform *
                                              , const aqmd3controls::waveform *
                                              , aqmd3controls::method& );

        typedef std::function< waveform_handler_type > waveform_reply_type;

        void connect_reply( command_reply_type ); // method,reply
        void disconnect_reply( command_reply_type );

        void connect_waveform( waveform_reply_type );
        void disconnect_waveform( waveform_reply_type );

        static bool readData( AqMD3&, const aqmd3controls::method&
                              , std::vector< std::shared_ptr< aqmd3controls::waveform > >& );
        static bool readData16( AqMD3&, const aqmd3controls::method&, aqmd3controls::waveform& );
        static bool readData32( AqMD3&, const aqmd3controls::method&, aqmd3controls::waveform&, const char * channel = "Channel1" );
    };


}
