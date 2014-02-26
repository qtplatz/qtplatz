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
#include <functional>
#include <vector>
#include <memory>

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
    };

	class U5303ASHARED_EXPORT method {
    public:
        std::string tba_;
    };

	class U5303ASHARED_EXPORT waveform : public std::enable_shared_from_this< waveform > {
		waveform( const waveform& );// = delete;
	public:
		waveform() : actualElements_(0), firstValidElement_(0) {}
        int64_t actualElements_;
        int64_t firstValidElement_;
        std::vector< int32_t > d_;
    };

    class U5303ASHARED_EXPORT digitizer {
    public:
        digitizer();
        ~digitizer();

        bool peripheral_initialize();
        bool peripheral_prepare_for_run( const method& );
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

#endif
