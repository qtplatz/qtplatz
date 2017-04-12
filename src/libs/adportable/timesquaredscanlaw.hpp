// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
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

namespace adportable {

    // doi = "10.1103/RevModPhys.88.035009"
    const double kATOMIC_MASS_CONSTANT = 1.660538921e-27; // [kg/u]
    const double kELEMENTAL_CHARGE    = 1.60217733e-19; // [C]

    const double kTimeSquaredCoeffs   = 2.0 * kELEMENTAL_CHARGE / kATOMIC_MASS_CONSTANT;

    class TimeSquaredScanLaw {
    public:
        TimeSquaredScanLaw( const TimeSquaredScanLaw& t );
        TimeSquaredScanLaw( double kAcceleratorVoltage = 3000, double tDelay = 0, double fLength = 1.0 );
        TimeSquaredScanLaw& operator = ( const TimeSquaredScanLaw& );
        
        virtual double getMass( double secs, int mode ) const;
        virtual double getTime( double mass, int mode ) const;
        virtual double getMass( double secs, double fLength ) const;
        virtual double getTime( double mass, double fLength ) const;
        virtual double fLength( int mode ) const;

        virtual double tDelay() const { return tDelay_; }
        virtual double kAcceleratorVoltage() const { return kAcceleratorVoltage_; }
        virtual double acceleratorVoltage( double mass, double time, int mode, double tDelay );
        static double acceleratorVoltage( double mass, double time, double flength, double tDelay );
        
    protected:
        double kAcceleratorVoltage_;
        double tDelay_;
        const double kTimeSquaredCoeffs_;
        const double fLength_;
    };

}
