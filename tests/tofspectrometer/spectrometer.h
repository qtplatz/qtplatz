// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include <adcontrols/massspectrometer.h>

namespace tofspectrometer {

    class Spectrometer : public adcontrols::MassSpectrometer {
    public:
        ~Spectrometer();
        Spectrometer();

        static adcontrols::MassSpectrometer * instance();
        static void dispose();
        virtual void accept( adcontrols::Visitor& );
        virtual adcontrols::MassSpectrometer::factory_type factory();
        virtual const wchar_t * name() const;
        virtual const MassSpectrometer::ScanLaw& getScanLaw() const;
        virtual const adcontrols::DataInterpreter& getDataInterpreter() const;

    private:
        static Spectrometer * instance_;
        adcontrols::MassSpectrometer::ScanLaw * pScanLaw_;
        adcontrols::DataInterpreter * pInterpreter_;
    };

    /////////////////////////

    class MultiTurnScanLaw : public adcontrols::MassSpectrometer::ScanLaw {
    public:
        MultiTurnScanLaw( double timeCoefficient, double timeDelay, double acclVolt );
        double getMass( double secs, int nTurn ) const;
        double getTime( double mass, int nTurn ) const;
        double getMass( double secs, double fLength ) const;
        double getTime( double mass, double fLength ) const;
    private:
        double timeCoefficient_;
        double timeDelay_;
        double acclVoltage_;
    };
}


