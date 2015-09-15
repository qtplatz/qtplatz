/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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

#include <adicontroller/signalobserver.hpp>
#include <workaround/boost/uuid/uuid.hpp>
#include <deque>
#include <ap240controls/waveform.hpp>
#include <ap240controls/method.hpp>

namespace ap240controls { class waveform; }
namespace ap240x = ap240controls;

namespace ap240controller {

    namespace so = adicontroller::SignalObserver;
    
    class WaveformObserver : public so::Observer {
        WaveformObserver( const WaveformObserver& ) = delete;
    public:
        WaveformObserver();
        virtual ~WaveformObserver();

        const boost::uuids::uuid& objid() const;
        const char * objtext() const;
        
        uint64_t uptime() const override;
        void uptime_range( uint64_t& oldest, uint64_t& newest ) const override;

        std::shared_ptr< so::DataReadBuffer > readData( uint32_t pos ) override;

        const char * dataInterpreterClsid() const override { return "ap240.digi"; }
        int32_t posFromTime( uint64_t usec ) const override;
        
        // WaveformObserver
        typedef std::pair< std::shared_ptr< ap240controls::waveform >, std::shared_ptr< ap240controls::waveform > > waveform_pair_t;
        typedef std::pair< std::shared_ptr< const ap240controls::waveform >, std::shared_ptr< const ap240controls::waveform > > const_waveform_pair_t;
        uint32_t operator << ( const_waveform_pair_t& );

    private:
        std::vector< std::shared_ptr< so::DataReadBuffer > > que_;
        const boost::uuids::uuid objid_;

    };
}
