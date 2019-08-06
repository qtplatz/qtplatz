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

#include "textfile.hpp"
#include "constants.hpp"
#include <multumcontrols/scanlaw.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adportable/textfile.hpp>
#include <fstream>
#include <cctype>

using namespace infitofspectrometer;

textfile::textfile()
{
}

bool
textfile::compile_header( adcontrols::MassSpectrum& ms, std::ifstream& in )
{
    std::streampos pos = in.tellg();

    int allElements(0);
    std::vector< int > arrayNum;
    std::vector< int > numTurns;
    std::vector< double > firstMass;
    std::vector< double > lastMass;

    std::string line;
    bool remain = false;
    while ( remain || adportable::textfile::getline( in, line ) ) {

		remain = false;

		if ( line.find( "All Elements" ) != line.npos ) {

            adportable::textfile::getline( in, line );
            allElements = atol( line.c_str() );

        } else if ( line.find( "Turn:" ) != line.npos ) {

            while ( adportable::textfile::getline( in, line ) && std::isdigit( line[0] ) )
                numTurns.push_back( std::atol( line.c_str() ) );
            remain = true;

        } else if ( line.find( "Array Num" ) != line.npos ) {

            while ( adportable::textfile::getline( in, line ) && std::isdigit( line[0] ) )
                arrayNum.push_back( std::atol( line.c_str() ) );
            remain = true;

        } else if ( line.find( "First m/z" ) != line.npos ) {

            while ( adportable::textfile::getline( in, line ) && std::isdigit( line[0] ) )
                firstMass.push_back( std::atof( line.c_str() ) );
            remain = true;

        } else if ( line.find( "Last m/z" ) != line.npos ) {

            while ( adportable::textfile::getline( in, line ) && std::isdigit( line[0] ) )
                lastMass.push_back( std::atof( line.c_str() ) );
            remain = true;

        } else if ( line.find( "Data" ) != line.npos ) {
            return setup( ms, numTurns, firstMass, lastMass );
        }
    }
    in.seekg( pos );
    return false;
}

// static
bool
textfile::setup( adcontrols::MassSpectrum& ms
                 , const std::vector<int>& nTurns
                 , const std::vector< double >& lMass
                 , const std::vector< double >& hMass )
{
    size_t nidx = 0;

    multumcontrols::infitof::ScanLaw law;
    adcontrols::MSProperty prop( ms.getMSProperty() );
    // prop.setDataInterpreterClsid( constants::dataInterpreter::spectrometer::utf8_name() );

    std::pair< double, double > massrange;
    if ( !lMass.empty() && !hMass.empty() ) {
        std::pair< double, double > massrange = std::make_pair( *std::min_element( lMass.begin(), lMass.end() )
                                                                , *std::max_element( hMass.begin(), hMass.end() ) );
        prop.setInstMassRange( massrange );
    }

    for ( int nlap: nTurns ) {

        adcontrols::SamplingInfo info;
        prop.setAcceleratorVoltage( law.kAcceleratorVoltage() );
        prop.setTDelay( law.tDelay() );
        info.setMode( nlap );
        prop.setSamplingInfo( info );

        if ( nidx == 0 ) {

            ms.setMSProperty( prop );

        } else {

            auto fms = std::make_shared< adcontrols::MassSpectrum >();
            fms->setMSProperty( prop );
            ms << std::move(fms);

        }
        ++nidx;
    }
    return true;
}
