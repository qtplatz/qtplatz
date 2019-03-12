/**************************************************************************
** Copyright (C) 2014-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include <cstdint>
#include <future>
#include <vector>

class QRect;

namespace adacquire {
    namespace SignalObserver { class Observer; }
    namespace Instrument { class Session; }
}

namespace adcontrols {
    namespace ControlMethod { class Method; }
    class SampleRun;
    class TofChromatogramsMethod;
}


namespace acquire {

    class task {
        ~task();
        task();
        task( const task& ) = delete;
        const task& operator = ( const task& ) = delete;
    public:
        static task * instance();
        bool initialize();
        bool finalize();

        void onDataChanged( adacquire::SignalObserver::Observer *, uint32_t pos );
        void instInitialize( adacquire::Instrument::Session * session );
        void post( std::vector< std::future<bool> >& futures );

        void setCellSelectionEnabled( bool );
        void setCellSelection( const QRect& );
        void setHistogramWindowEnabled( bool );
        void setHistogramWindow( double delay, double window );
        void setDeviceDelay( double delay );
        void setHistogramClearCycleEnabled( bool );
        void setHistogramClearCycle( uint32_t );
        void setRecording( bool );
        bool isRecording() const;

        void sample_started();  // autosampler start
        void sample_injected(); // data acquisition start, being time zero
        void sample_stopped();  // data close, if ( injected ) { post process start } else { delete data }

        void setTofChromatogramsMethod( const adcontrols::TofChromatogramsMethod& );
        uint64_t injectTimeSinceEpoch() const;
        uint64_t upTimeSinceEpoch() const;

        void start_polling( uint32_t interval );

    private:
        class impl;
        impl * impl_;
    };
}
