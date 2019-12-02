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

#include "multumcontrols_global.hpp"
#include <adportable/timesquaredscanlaw.hpp>
#include <adcontrols/scanlaw.hpp>
#include <array>

namespace multumcontrols {

    namespace infitof { }

    //////////////
    class MULTUMCONTROLSSHARED_EXPORT ScanLaw : public adcontrols::ScanLaw, protected adportable::TimeSquaredScanLaw {
        double gateOffset_;
        std::array< double, 7 > dimension_;
    public:
        ~ScanLaw();
        ScanLaw( double acceleratorVoltage
                 , double tDelay
                 , double _1, double _2, double _3, double _4, double _5, double _6, double _7 );

        ScanLaw( const ScanLaw& t );
        ScanLaw& operator = ( const ScanLaw& );

        // TimeSquaredScanLaw
        double tDelay() const override;
        double kAcceleratorVoltage() const override;
        double acceleratorVoltage( double mass, double time, int mode, double tDelay ) override;
        void setAcceleratorVoltage( double ) override;
        void setTDelay( double ) override;

        double acceleratorVoltage( double mass, double time, double flength, double tDelay );

        // adcontrols::ScanLaw
        double getMass( double t, int mode ) const override;
        double getTime( double m, int mode ) const override;
        double getMass( double t, double fLength ) const override;
        double getTime( double m, double fLength ) const override;
        double fLength( int type ) const override;

        // multumcontrols::ScanLaw
        double orbital_period( double mass ) const;
        double go_around_threshold_time( double mass ) const;
        double gate_through_threshold_time( double mass ) const;
        double entry_through_threshold_time( double mass ) const;
        double exit_through_threshold_time( double mass, int nTurns ) const;

        double orbital_length() const;
        double linear_length() const;
        double go_around_threshold_length() const;
        double gate_through_threshold_length() const;
        double entry_through_threshold_length() const;
        double exit_through_threshold_length( int nTurns ) const;

        double number_of_turns( double exit_delay, double exact_mass ) const;

        void setGateOffsetLength( double );
        double gateOffsetLength() const;

        void setLength( int, double );
        double length( int ) const;
    };

    namespace infitof {

        class MULTUMCONTROLSSHARED_EXPORT ScanLaw : public multumcontrols::ScanLaw {
        public:
            ~ScanLaw();
            ScanLaw();
            ScanLaw( double kAcceleratorVoltage, double tDelay );
        };

    };
}
