/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
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

#include <vector>
#include <chromatogr/stack.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/baselines.hpp>

namespace adcontrols {
    class PeakMethod;
}

namespace chromatogr {

    /**
       \brief Integrator class process input data (usually from ADC) stream and process it
       as chromatography data for find peaks and baseline determination.
    */
    class Integrator {
        Integrator( const Integrator& ) = delete;
        Integrator& operator = ( const Integrator& ) = delete;
    public:
        Integrator(void);
        virtual ~Integrator(void);

    private:
        class impl;
        impl * impl_;
        
    public:
        /**
           \brief Set sampling interval for input data if data has contenious constant time interval, otherwise set zero.
        */
        void samping_interval( double /* seconds */ );

        /**
           \brief Set peak width (seconds) for first elute compounds in your interst.
        */        
        void minimum_width(double /* seconds */);

        /**
           \brief Set baseline start recognition threshold.
        */        
        void slope_sensitivity(double /* uV / second */);

        /**
           \brief drift give a hint to software for determine baseline either v-to-v or perpendicular dropping.
         */
        void drift(double /* uV / second */);
        
        double currentTime() const;

        void timeOffset( double peaktime );

        /**
           \brief add a new data intensity (ADC value).  Sampling interval must be set in advance.
        */
        void operator << ( double v );  // analogue input
        
        /**
           \brief add data point as pair of (time(seconds), intensity) for non time constant acquisition
        */
        void operator << ( const std::pair<double, double >& v );

        /**
           \brief close out data stream, and fix all detected peaks and baselines
        */
		void close( const class adcontrols::PeakMethod&, adcontrols::Peaks&, adcontrols::Baselines& );
        
        /**
           \brief Stop integration during data input when this flag set true.  
           Set this flags will ignore from peak detection process for all incoming data by '<<' operator.
         */
        void offIntegration( bool f );

        /**
           \brief 
         */
        bool offIntegration() const;
    };

}
