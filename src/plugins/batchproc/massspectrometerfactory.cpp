/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "massspectrometerfactory.hpp"
#include "datainterpreter.hpp"
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/datainterpreter.hpp>
#include <adcontrols/mscalibrateresult.hpp>

namespace batchproc {

    class MassSpectrometer : public adcontrols::MassSpectrometer {
    public:
        virtual ~MassSpectrometer(void) {
        }

        MassSpectrometer(void) : interpreter_( std::make_shared< DataInterpreter >() ) {
        }

        MassSpectrometer( const MassSpectrometer& t ) : interpreter_( t.interpreter_ ) {
        }
        
        const wchar_t * name() const override {
            return L"batchproc::import";
        }

        const adcontrols::ScanLaw& getScanLaw() const override {
            throw std::bad_cast();
        }

        const adcontrols::DataInterpreter& getDataInterpreter() const override {
            return *interpreter_;
        }

		std::shared_ptr<adcontrols::ScanLaw> scanLaw( const adcontrols::MSProperty& ) const override {
            return 0;
        }

		void setCalibration( int mode, const adcontrols::MSCalibrateResult& ) override {
        }

        const std::shared_ptr< adcontrols::MSCalibrateResult > getCalibrateResult( size_t idx ) const override {
            return 0;
        }

        const adcontrols::MSCalibration * findCalibration( int mode ) const override {
            return 0;
        }

    private:
        std::shared_ptr< DataInterpreter > interpreter_;
    };
}

using namespace batchproc;

MassSpectrometerFactory::MassSpectrometerFactory() : spectrometer_( new MassSpectrometer() )
{
}

const wchar_t * 
MassSpectrometerFactory::name() const
{
    return L"batchproc::import";
}

adcontrols::MassSpectrometer *
MassSpectrometerFactory::get( const wchar_t * modelname )
{
    if ( std::wcscmp( L"batchproc::import", modelname ) == 0 )
        return spectrometer_.get();

    return 0;
}




