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

#include "waveformwnd.hpp"
#include "document.hpp"
#include <ap240/digitizer.hpp>
#include <adplot/spectrumwidget.hpp>
#include <adplot/chromatogramwidget.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/trace.hpp>
#include <adportable/float.hpp>
#include <adportable/spectrum_processor.hpp>
#include <coreplugin/minisplitter.h>
#include <QSplitter>
#include <QBoxLayout>
#include <boost/format.hpp>
#include <chrono>

using namespace ap240;

WaveformWnd::WaveformWnd( QWidget * parent ) : QWidget( parent )
                                             , spw_( new adplot::SpectrumWidget )
                                             , tpw_( new adplot::ChromatogramWidget )
                                             , tp_( std::make_shared< adcontrols::Trace >() )
{
    init();
}

WaveformWnd::~WaveformWnd()
{
    fini();
    delete tpw_;
    delete spw_;
}

void
WaveformWnd::init()
{
    Core::MiniSplitter * splitter = new Core::MiniSplitter;
    do {
        splitter->addWidget( tpw_ );
        splitter->addWidget( spw_ );
        splitter->setStretchFactor( 0, 1 );
        splitter->setStretchFactor( 1, 3 );
        splitter->setOrientation( Qt::Vertical );
    } while(0);

    spw_->setAxis( adplot::SpectrumWidget::HorizontalAxisTime );
    spw_->setKeepZoomed( false );

    QBoxLayout * layout = new QVBoxLayout( this );
    layout->setMargin( 0 );
    layout->setSpacing( 2 );
    layout->addWidget( splitter );
}

void
WaveformWnd::fini()
{
}

void
WaveformWnd::onInitialUpdate()
{
    spw_->setKeepZoomed( false );
}

void
WaveformWnd::handle_waveform()
{
    if ( auto waveform = document::instance()->findWaveform() ) {
        double dbase(0), rms(0);
        double timestamp = waveform->meta_.initialXTimeSeconds; 
        do {
            // timed trace
            const int32_t * data = waveform->d_.data();
            double tic = adportable::spectrum_processor::tic( static_cast<unsigned int>(waveform->d_.size()), data, dbase, rms );

			tp_->push_back( waveform->serialnumber_, timestamp, tic );
            tpw_->setData( *tp_ );
        } while(0);

		// data interpreter need to be implementd --- TBD

        sp_ = std::make_shared< adcontrols::MassSpectrum >();
		sp_->setCentroid( adcontrols::CentroidNone );

        adcontrols::MSProperty prop = sp_->getMSProperty();
		adcontrols::MSProperty::SamplingInfo info( 0
                                                   , uint32_t( waveform->meta_.initialXOffset / waveform->meta_.xIncrement + 0.5 )
                                                   , uint32_t( waveform->d_.size() )
                                                   , waveform->meta_.actualAverages
                                                   , 0 );
        info.fSampInterval( waveform->meta_.xIncrement );
        prop.acceleratorVoltage( 3000 );
		prop.setSamplingInfo( info );
        using namespace adcontrols::metric;
        prop.setTimeSinceInjection( timestamp ); // seconds
        prop.setDataInterpreterClsid( "ap240" );

        
        // prop.setDeviceData(); TBA
        sp_->setMSProperty( prop );

		sp_->resize( waveform->d_.size() );
		int idx = 0;
		for ( auto y: waveform->d_ )
			sp_->setIntensity( idx++, y - dbase );
		spw_->setData( sp_, 0 );
        spw_->setKeepZoomed( true );
        //std::chrono::duration< uint64_t, std::pico > pico( waveform->timestamp_ );
        //typedef std::chrono::duration<double> seconds;
        spw_->setTitle( ( boost::format( "Time: %.3f RMS: %.3f" ) % waveform->meta_.initialXTimeSeconds % rms ).str() );
    }
}
