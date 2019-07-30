/**************************************************************************
** Copyright (C) 2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2019 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <adportable/timesquaredscanlaw.hpp>
#include <adcontrols/scanlaw.hpp>
#include <array>

namespace accutof { namespace spectrometer {

        //////////////
        class ScanLaw : public adcontrols::ScanLaw
                      , protected adportable::TimeSquaredScanLaw {
        public:
            ~ScanLaw();
            ScanLaw( double acceleratorVoltage = 7000, double tDelay = 0 );

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

            void setLength( int, double );
            double length( int ) const;
        };

    }
}
