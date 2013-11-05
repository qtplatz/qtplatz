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
#include <map>

namespace adcontrols {

    class DataInterpreter;
	class MSProperty;
    class MSCalibrateResult;

    const double kATOMIC_MASS_CONSTANT = 1.66054020e-27; // [kg/u]
    const double kELEMENTAL_CHARGE    = 1.60217733e-19; // [C]
    const double kTimeSquaredCoeffs   = 2.0 * kELEMENTAL_CHARGE / kATOMIC_MASS_CONSTANT;
    
    class ADCONTROLSSHARED_EXPORT ScanLaw {
    public:
        virtual double getMass( double secs, int mode ) const = 0;
        virtual double getTime( double mass, int mode ) const = 0;
        virtual double getMass( double secs, double fLength ) const = 0;
        virtual double getTime( double mass, double fLength ) const = 0;
        virtual double fLength( int mode ) const = 0;
    };

    class ADCONTROLSSHARED_EXPORT TimeSquaredScanLaw : public ScanLaw {
    public:
        TimeSquaredScanLaw( const TimeSquaredScanLaw& t );
        TimeSquaredScanLaw( double kAcceleratorVoltage = 3000, double tDelay = 0, double fLength = 1.0 );

        double getMass( double secs, int mode ) const override;
        double getTime( double mass, int mode ) const override;
        double getMass( double secs, double fLength ) const override;
        double getTime( double mass, double fLength ) const override;
        double fLength( int mode ) const override;

    protected:
        double kAcceleratorVoltage_;
        double tDelay_;
        const double kTimeSquaredCoeffs_;
        const double fLength_;
    };
        
    class ADCONTROLSSHARED_EXPORT MassSpectrometer {
    public:
        MassSpectrometer(void);
        MassSpectrometer( const MassSpectrometer& );
        virtual ~MassSpectrometer(void);
        
        virtual const wchar_t * name() const;
        virtual const ScanLaw& getScanLaw() const;
        virtual const DataInterpreter& getDataInterpreter() const;
		virtual std::shared_ptr<ScanLaw> scanLaw( const adcontrols::MSProperty& ) const;

		virtual void setCalibration( int mode, const adcontrols::MSCalibrateResult& );

        virtual const std::shared_ptr< adcontrols::MSCalibrateResult > findCalibration( int mode ) const;
        virtual const std::shared_ptr< adcontrols::MSCalibrateResult > calibration( size_t idx ) const;

        static std::shared_ptr< MassSpectrometer > create( const wchar_t * dataInterpreterClsid );

        static const MassSpectrometer& get( const wchar_t * dataInterpreterClsid );
		static const MassSpectrometer& get( const char * dataInterpreterClsid );
        static std::vector< std::wstring > get_model_names();

    protected:

#if defined _MSC_VER
# pragma warning(disable:4251)
#endif
        const MassSpectrometer * instance_;
        std::map< int, std::shared_ptr< adcontrols::MSCalibrateResult > > mode_calib_map_;
        std::shared_ptr< ScanLaw > scanLaw_;
    };

}

