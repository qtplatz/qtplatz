// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include "adcontrols_global.h"
#include <memory>
#include <string>
#include <vector>

namespace adcontrols {

    class Visitor;
    class DataInterpreter;
	class MSProperty;

    namespace massspectrometer {

        class ADCONTROLSSHARED_EXPORT ScanLaw {
        public:
            virtual double getMass( double secs, int mode ) const = 0;
            virtual double getTime( double mass, int mode ) const = 0;
            virtual double getMass( double secs, double fLength ) const = 0;
            virtual double getTime( double mass, double fLength ) const = 0;
            virtual double fLength( int mode ) const = 0;
        };

        const double kATOMIC_MASS_CONSTANT = 1.66054020e-27; // [kg/u]
        const double kELEMENTAL_CHARGE    = 1.60217733e-19; // [C]
        const double kTimeSquaredCoeffs   = 2.0 * kELEMENTAL_CHARGE / kATOMIC_MASS_CONSTANT;
    }
        
    class ADCONTROLSSHARED_EXPORT MassSpectrometer {
    public:
        MassSpectrometer(void) {}
        virtual ~MassSpectrometer(void) {}
        
        // class ScanLaw {
        // public:
        //     virtual double getMass( double secs, int mode ) const = 0;
        //     virtual double getTime( double mass, int mode ) const = 0;
        //     virtual double getMass( double secs, double fLength ) const = 0;
        //     virtual double getTime( double mass, double fLength ) const = 0;
        //     virtual double fLength( int mode ) const = 0;
        // };

        //virtual void accept( Visitor& ) = 0;
        virtual const wchar_t * name() const = 0;
        virtual const massspectrometer::ScanLaw& getScanLaw() const = 0;
        virtual const DataInterpreter& getDataInterpreter() const = 0;
		virtual std::shared_ptr<massspectrometer::ScanLaw> scanLaw( const adcontrols::MSProperty& ) const = 0;
        
        static const MassSpectrometer& get( const wchar_t * dataInterpreterClsid );
		static const MassSpectrometer& get( const char * dataInterpreterClsid );
        static std::vector< std::wstring > get_model_names();
    };

}

