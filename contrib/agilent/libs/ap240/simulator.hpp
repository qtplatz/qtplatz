/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#include <condition_variable>
#include <atomic>
#include <memory>
#include <vector>
#include <ap240spectrometer/waveform.hpp>
#include <ap240spectrometer/method.hpp>

namespace boost { namespace asio { class io_service; } }
namespace adinterface { class waveform_generator;  }

namespace ap240 {
    
    class simulator  {
    public:
        ~simulator();
        simulator();

        bool acquire( boost::asio::io_service& );
        bool waitForEndOfAcquisition();
        bool readData( ap240x::waveform& );
        void setup( const ap240x::method& );

        void protocol_handler( double, double );

        static simulator * instance();
    private:
        std::mutex mutex_;
        std::condition_variable cond_;
        std::mutex queue_;
        std::vector< std::pair< double, double > > ions_; // pair<mass, intensity>
        std::atomic<bool> hasWaveform_;
        std::atomic<bool> acqTriggered_;
        std::vector< std::shared_ptr< adinterface::waveform_generator > > waveforms_;
        double sampInterval_;
        double startDelay_;
        uint32_t nbrSamples_;
        uint32_t nbrWaveforms_;
        double exitDelay_;
        std::shared_ptr< ap240x::method > method_;
        static simulator * instance_;

        void post( adinterface::waveform_generator * );
    };

}


