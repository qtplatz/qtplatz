/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <QWidget>
#include <array>
#include <memory>
#include <mutex>

class QwtPlotMarker;

namespace adplot { class ChromatogramWidget; class PeakMarker; class TraceWidget; class SpectrumWidget; class SpanMarker; }
namespace adcontrols { class CountingMethod; class MassSpectrum; class MSPeakInfo;  class Trace; class TofChromatogramsMethod; class threshold_action; }
namespace acqrscontrols { namespace u5303a { class waveform; } }
namespace boost { namespace uuids { struct uuid; } }

namespace accutof { namespace acquire {

    //class waveform;

    class WaveformWnd : public QWidget {
        Q_OBJECT
    public:

        static constexpr size_t spViewCount = 2;
        static constexpr size_t channelCount = 1;
        constexpr static size_t closeupCount = 4;

        explicit WaveformWnd( QWidget * parent = 0 );
        ~WaveformWnd();

        void onInitialUpdate();
        void setData( const std::shared_ptr< const acqrscontrols::u5303a::waveform >& );
        void setMethod( const adcontrols::TofChromatogramsMethod& m );
        // void setMethod( const adcontrols::CountingMethod& m );
        void setSpanMarker( unsigned int row, unsigned int index /* 0 = time, 1 = window */, double );

        // bool longTermHistogramEnabled() const;
        // bool pkdSpectrumEnabled() const;
        // void setLongTermHistogramEnabled( bool );
        // void setPKDSpectrumEnabled( bool );
        void setAxis( int idView, int axis ); // 0: mass, 1: time

    public slots:
        void handle_threshold_method( int ch );
        void handle_threshold_action();
        void handle_method( const QString& );
        void handleScaleY( int, bool autoScale, double top, double bottom );
        void handleMolecules( const QString& );


    private slots:
        void handleDataChanged( const boost::uuids::uuid&, int idx );
        void handleTraceChanged( const boost::uuids::uuid& );
        void thresholdTraceChanged();

    private:
        void init();
        void fini();
        void setCountingRange( int row, const std::pair<double, double>& range );
        void handleDrawSettingChanged();

        adplot::ChromatogramWidget * tpw_;
        adplot::SpectrumWidget * spw_;
        adplot::SpectrumWidget * hpw_;
        size_t tickCount_;

        std::array< std::shared_ptr< adcontrols::MassSpectrum >, spViewCount > sp_;
        std::array< std::shared_ptr< adcontrols::Trace>, 2 > tp_;
        std::array< std::pair<bool, double>, channelCount > thresholds_;

        std::array< QwtPlotMarker *, channelCount > threshold_markers_;
        std::unique_ptr< adplot::SpanMarker > threshold_action_marker_;

        std::unique_ptr< adcontrols::threshold_action > threshold_action_;

        struct closeup {
            std::unique_ptr< adplot::SpectrumWidget > sp;
            std::unique_ptr< adplot::SpanMarker > marker;
            QString formula;
            bool enable;
        };
        std::array< closeup, closeupCount > closeups_;
        bool longTermHistogramEnabled_;
        bool pkdSpectrumEnabled_;
        // values for spectrum view title
        double elapsedTime_;
        uint32_t numberOfTriggersSinceInject_;
        std::mutex mutex_;
        std::vector< std::unique_ptr< QwtPlotMarker > > tof_markers_;
    };

}
}
