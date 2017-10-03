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

#include <acqrscontrols/constants.hpp>
#include <QObject>
#include <atomic>
#include <array>
#include <mutex>
#include <memory>

namespace acqrscontrols { template< typename T > class threshold_result_; namespace ap240 { class waveform; class histogram; } }
namespace adcontrols { class threshold_method; class MassSpectrum; }

namespace ap240 {

    class iControllerImpl;

    typedef std::shared_ptr< acqrscontrols::threshold_result_< acqrscontrols::ap240::waveform > > threshold_result_ptr;
    typedef std::shared_ptr< const acqrscontrols::threshold_result_< acqrscontrols::ap240::waveform > > const_threshold_result_ptr;

    class tdcdoc : public QObject {

        Q_OBJECT

    public:
        ~tdcdoc();
        tdcdoc( QObject * parent = nullptr );

        bool set_threshold_method( int channel, const adcontrols::threshold_method& );
        std::shared_ptr< const adcontrols::threshold_method > threshold_method( int channel ) const;

        std::array< threshold_result_ptr, acqrscontrols::ap240::nchannels > handle_waveforms( std::array< std::shared_ptr< const acqrscontrols::ap240::waveform >, acqrscontrols::ap240::nchannels > );

        // strand required
        void appendHistogram( std::array< threshold_result_ptr, acqrscontrols::ap240::nchannels > results );
            
        std::shared_ptr< adcontrols::MassSpectrum > getHistogram( double resolution, int channel, size_t& trigCount, std::pair<uint64_t, uint64_t>& timeSinceEpoch ) const;

        void update_rate( size_t, const std::pair<uint64_t, uint64_t>& timeSinceEpoch );

        void clear_histogram();

        double trig_per_seconds() const;

        static void find_threshold_timepoints( const acqrscontrols::ap240::waveform& data
                                               , const adcontrols::threshold_method& method
                                               , std::vector< uint32_t >& elements
                                               , std::vector<double>& processed );

    private:
        std::array< std::shared_ptr< adcontrols::threshold_method >, 2 > threshold_methods_;
        std::array< std::shared_ptr< acqrscontrols::ap240::histogram >, 2 > histograms_;
        std::atomic< double > trig_per_seconds_;
        std::mutex mutex_;

    signals:
        // void getMethodFromUI( adcontrols::ControlMethod::Method& ) const;
        // void onControlMethodChanged();
        // void instStateChanged( int );
        // void dataChanged( int );
    };

}


