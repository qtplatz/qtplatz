/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "acqrscontrols_global.hpp"
#include "threshold_result.hpp"
#include <acqrscontrols/constants.hpp>
#include <memory>

namespace adcontrols {
    class threshold_action;
    class threshold_method;
    class MassSpectrometer;
    class MassSpectrum;
    class TofChromatogramsMethod;
    class TimeDigitalHistogram;
    class CountingMethod;
}

namespace acqrscontrols {

    class tdcbase {
    public:
        virtual ~tdcbase();
        tdcbase();

        enum SpectrumType { Raw, Profile, PeriodicHistogram, LongTermHistogram };
        
        static bool computeCountRate( const adcontrols::TimeDigitalHistogram& histogram
                                      , const adcontrols::CountingMethod&
                                      , std::vector< std::pair< size_t, size_t > >& );
        
        virtual void setCountingMethod( std::shared_ptr< const adcontrols::CountingMethod > );
        virtual std::shared_ptr< const adcontrols::CountingMethod > countingMethod() const;
        
        virtual bool setTofChromatogramsMethod( const adcontrols::TofChromatogramsMethod& a );
        virtual std::shared_ptr< const adcontrols::TofChromatogramsMethod > tofChromatogramsMethod() const;
        virtual void eraseTofChromatogramsMethod();

        virtual bool set_threshold_action( const adcontrols::threshold_action& ) = 0;
        virtual std::shared_ptr< const adcontrols::threshold_action > threshold_action() const = 0;

        virtual bool set_threshold_method( int channel, const adcontrols::threshold_method& ) = 0;
        virtual std::shared_ptr< const adcontrols::threshold_method > threshold_method( int channel ) const = 0;

        virtual std::pair< uint32_t, uint32_t > threshold_action_counts( int channel ) const = 0;
        virtual void set_threshold_action_counts( int channel, const std::pair< uint32_t, uint32_t >& ) const = 0;
        
        virtual void clear_histogram() = 0;

    protected:
        std::shared_ptr< const adcontrols::CountingMethod > countingMethod_;
        std::shared_ptr< const adcontrols::TofChromatogramsMethod > tofChromatogramsMethod_;
    };

}
