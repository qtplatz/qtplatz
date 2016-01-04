/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#ifndef MASSSPECTROMETER_HPP
#define MASSSPECTROMETER_HPP

#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/datasubscriber.hpp>
#include <memory>

namespace adcontrols {
    class datafile;
    class MassSpectrometer;
    class MSCalibrateResult;
    class MSProperty; 
    class ScanLaw;
    class LCMSDataset;
}

namespace adspectrometer {

    class DataInterpreter;
    class import_continuum_massarray;

    class MassSpectrometer : public adcontrols::MassSpectrometer
		                   , public adcontrols::dataSubscriber
                           , public std::enable_shared_from_this< MassSpectrometer > {
        MassSpectrometer( const MassSpectrometer& t ) = delete; // non copyable
    public:
        virtual ~MassSpectrometer();
        MassSpectrometer( adcontrols::datafile * datafile );

        // dataSubscriber
        bool subscribe( const adcontrols::LCMSDataset& ) override;
        // end dataSubscriber

        const wchar_t * name() const override;
        const adcontrols::ScanLaw& getScanLaw() const override;
        //const adcontrols::DataInterpreter& getDataInterpreter() const override;
		std::shared_ptr<adcontrols::ScanLaw> scanLaw( const adcontrols::MSProperty& ) const override;
		void setCalibration( int mode, const adcontrols::MSCalibrateResult& ) override;
        const std::shared_ptr< adcontrols::MSCalibrateResult > getCalibrateResult( size_t idx ) const override;
        const adcontrols::MSCalibration * findCalibration( int mode ) const override;
        //-----------
        
        const import_continuum_massarray& continuum_massarray() const;
        void continuum_massarray( const import_continuum_massarray& );

    private:
        std::shared_ptr< import_continuum_massarray > continuum_massarray_;
        const adcontrols::LCMSDataset* accessor_;
        bool load_continuum_massarray();
    };

}
#endif // MASSSPECTROMETER_HPP
