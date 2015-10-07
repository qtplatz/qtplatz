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

#ifndef WAVEFORMWND_HPP
#define WAVEFORMWND_HPP

#include <QWidget>
#include <array>
#include <memory>

class QwtPlotMarker;

namespace adplot { class ChromatogramWidget; class TraceWidget; class SpectrumWidget; }
namespace adcontrols { class MassSpectrum; class Trace; }
namespace acqrscontrols { namespace u5303a { class waveform; } }
namespace boost { namespace uuids { struct uuid; } }

namespace u5303a {

    //class waveform;

    class WaveformWnd : public QWidget {
        Q_OBJECT
    public:
        explicit WaveformWnd( QWidget * parent = 0 );
        ~WaveformWnd();
        
        void onInitialUpdate();
        void setData( const std::shared_ptr< const acqrscontrols::u5303a::waveform >& );

    public slots:
        void handle_waveform();
        void handle_threshold_method( int ch );
        void handle_method( const QString& );
        void dataChanged( const boost::uuids::uuid&, int idx );

    private:
        void init();
        void fini();
        adplot::ChromatogramWidget * tpw_;
        adplot::SpectrumWidget * spw_;
        adplot::SpectrumWidget * hpw_;
        size_t tickCount_;

        std::array< std::shared_ptr< adcontrols::MassSpectrum >, 2 > sp_;

        std::array< std::shared_ptr< adcontrols::Trace>, 2 > tp_;
        std::array< std::pair<bool, double>, 2 > thresholds_;
        std::array< QwtPlotMarker *, 2 > threshold_markers_;
    };

}

#endif // WAVEFORMWND_HPP
