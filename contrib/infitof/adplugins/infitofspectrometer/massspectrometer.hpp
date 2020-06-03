// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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

#include "constants.hpp"
#include <adcontrols/massspectrometer.hpp>
#include <admtcontrols/scanlaw.hpp>
#include <adcontrols/massspectrometer_factory.hpp>
#include <adplugin/plugin.hpp>
#include <memory>
#include "infitofspectrometer_global.h"

namespace adcontrols { class datafile; class MSProperty; namespace ControlMethod { class Method; } }
namespace adfs { class filesystem; }
namespace boost { namespace uuids { struct uuid; } }
namespace admtcontrols { class ScanLaw; class OrbitProtocol; }

namespace infitofspectrometer {

    class InfiTofDataInterpreter;
    //class MultiTurnScanLaw;

    class MassSpectrometer : public adcontrols::MassSpectrometer {

        MassSpectrometer( const MassSpectrometer& t ) = delete; // non copyable

    public:
        MassSpectrometer();
        ~MassSpectrometer();

        // dataformat v3 interface
        void initialSetup( adfs::sqlite& dbf, const boost::uuids::uuid& ) override;
        void setAcceleratorVoltage( double, double ) override;
        bool assignMasses( adcontrols::MassSpectrum&, int64_t rowid ) const override;

        std::shared_ptr< adcontrols::ScanLaw > scanLaw( const adcontrols::MSProperty& ) const override;

        // dataformat v2 class name
        const wchar_t * name() const override;

        // dataformat v3 class uuid --> moved to qtplatz/contrib/infitof/libs/infitofcontrols/constants.hpp
        //static constexpr const char * clsid_text = "{90BB510B-5DC2-43AB-89EF-2E108E99EAAA}";
        //static constexpr const char * class_name = "InfiTOF"; // historical name, don't change

        //const char * objtext() const override;
        //const boost::uuids::uuid& objclsid() const override;
        const char * const massSpectrometerName() const override;
        const boost::uuids::uuid& massSpectrometerClsid() const override;

        const char * dataInterpreterText() const override;
        const boost::uuids::uuid& dataInterpreterUuid() const override;

        const adcontrols::ScanLaw * scanLaw() const override;

        void setMethod( const adcontrols::ControlMethod::Method& ) override;
        const adcontrols::ControlMethod::Method * method() const override;
        int mode( uint32_t protocolNumber ) const override;
        bool setMSProperty( adcontrols::MassSpectrum&, const adcontrols::ControlMethod::Method&, int ) const override;
        bool estimateScanLaw( const adcontrols::MSPeaks&, double& va, double& t0 ) const override;

        //--------- local -----------
        const adcontrols::ScanLaw * scanLaw( int64_t ) const;
    private:
        std::unique_ptr< admtcontrols::ScanLaw > scanLaw_;
        std::unique_ptr< adcontrols::ControlMethod::Method > method_;
        std::vector< admtcontrols::OrbitProtocol > protocols_;
        std::vector< std::pair< int64_t, std::unique_ptr< admtcontrols::ScanLaw > > > scanLaws_;
    };

}
