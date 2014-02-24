/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "massspectrometer.hpp"
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/datainterpreter.hpp>
#include <adcontrols/mscalibrateresult.hpp>

namespace batchproc {

}

using namespace batchproc;

MassSpectrometerFactory::MassSpectrometerFactory() : spectrometer_( new MassSpectrometer(0) )
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

std::shared_ptr< adcontrols::MassSpectrometer >
MassSpectrometerFactory::create( const wchar_t * modelname, adcontrols::datafile * datafile ) const
{
    if ( std::wcscmp( L"batchproc::import", modelname ) == 0 )
        return std::make_shared< batchproc::MassSpectrometer >( datafile );

    return 0;
}

