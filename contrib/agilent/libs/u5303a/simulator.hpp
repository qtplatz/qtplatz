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

#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include <condition_variable>
#include <atomic>
#include <memory>
#include <vector>

namespace boost { namespace asio { class io_service; } }
namespace adacquire { class waveform_simulator;  }
namespace acqrscontrols { namespace u5303a { class waveform; class method; } }
namespace dgpio { class pio; }

namespace u5303a {
    
    //class waveform;
    //class method;

    class simulator  {
        simulator();
        simulator( const simulator& ) = delete;
        simulator& operator = ( const simulator& ) = delete;

    public:
        ~simulator();

        bool acquire();
        bool waitForEndOfAcquisition();
        bool readData( acqrscontrols::u5303a::waveform& );
        bool readDataPkdAvg( acqrscontrols::u5303a::waveform&, acqrscontrols::u5303a::waveform& );
        void setup( const acqrscontrols::u5303a::method& );
        void touchup( std::vector< std::shared_ptr< acqrscontrols::u5303a::waveform > >&
                      , const acqrscontrols::u5303a::method& );

        void protocol_handler( double, double );
        int protocol_number() const;

        static simulator * instance();
    private:
        std::mutex mutex_;
        std::condition_variable cond_;
        std::mutex queue_;
        std::vector< std::pair< double, double > > ions_; // pair<mass, intensity>
        std::atomic<bool> hasWaveform_;
        std::atomic_flag acqTriggered_;
        std::vector< std::shared_ptr< adacquire::waveform_simulator > > waveforms_;
        std::unique_ptr< dgpio::pio > pio_;
        double sampInterval_;
        double startDelay_;
        uint32_t nbrSamples_;
        uint32_t nbrWaveforms_;
        double exitDelay_;
        std::shared_ptr< acqrscontrols::u5303a::method > method_;

        int32_t protocolIndex_;
        int32_t protocolReplicates_;

		void post( std::shared_ptr< adacquire::waveform_simulator >& );
        void next_protocol();
    };

}

#endif // SIMULATOR_HPP
