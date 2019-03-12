/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef WAVEFORMWND_HPP
#define WAVEFORMWND_HPP

#include <QWidget>
#include <array>
#include <memory>

class QwtPlotMarker;

namespace adplot { class ChromatogramWidget; class PeakMarker; class TraceWidget; class SpectrumWidget; class SpanMarker; }
namespace adcontrols { class MassSpectrum; class MSPeakInfo;  class Trace; class TofChromatogramsMethod; class threshold_action; }
namespace acqrscontrols { namespace acquire { class waveform; } }
namespace boost { namespace uuids { struct uuid; } }

namespace acquire {

    //class waveform;

    class WaveformWnd : public QWidget {
        Q_OBJECT
    public:

        static constexpr size_t spViewCount = 2;
        static constexpr size_t channelCount = 1;

        explicit WaveformWnd( QWidget * parent = 0 );
        ~WaveformWnd();

        void onInitialUpdate();
        void setMethod( const adcontrols::TofChromatogramsMethod& m );

    public slots:
        void handle_threshold_action( const QJsonDocument& );
        void handle_threshold_method( const QJsonDocument& );
        void handle_threshold_level( double mV );
    private slots:
        void dataChanged( const boost::uuids::uuid&, int idx );

    private:
        void init();
        void fini();
        adplot::ChromatogramWidget * tpw_;
        adplot::SpectrumWidget * spw_;
        adplot::SpectrumWidget * hpw_;
        adplot::SpectrumWidget * pkw_;
        size_t tickCount_;

        std::array< std::shared_ptr< adcontrols::MassSpectrum >, spViewCount > sp_;
        std::array< std::shared_ptr< adcontrols::Trace>, 2 > tp_;

        std::array< std::pair<bool, double>, channelCount > thresholds_;
        std::array< QwtPlotMarker *, channelCount > threshold_markers_;

        std::unique_ptr< adplot::SpanMarker > threshold_action_marker_;
        std::array< std::array< std::unique_ptr< adplot::SpanMarker >, 8 >, spViewCount > histogram_window_markers_;
        std::array< QString, 2 > hpwTitles_;
    };

}

#endif // WAVEFORMWND_HPP
