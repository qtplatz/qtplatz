/**************************************************************************
** Copyright (C) 2010-2018 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2018 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
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

#include "test_process_method.hpp"

#include <adcontrols/processmethod.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/msreferences.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/xml_serializer.hpp>
#include <boost/exception/all.hpp>
#include <boost/test/unit_test.hpp>
#include <fstream>
#include <iostream>

namespace test {

    adcontrols::MSReferences&
    make_msreferences( adcontrols::MSReferences& refs )
    {
        using namespace adcontrols;
        refs << MSReference( L"H2O(C2H4)2", true, L"+H" )
             << MSReference( L"H2O(C2H4)3", true, L"+H" )
             << MSReference( L"H2O(C2H4)4", true, L"+H" )
             << MSReference( L"H2O(C2H4)5", true, L"+H" );
        return refs;
    }
    
    void
    make_process_method( adcontrols::ProcessMethod& m )
    {
        using namespace adcontrols;

        MSReferences refs;
        make_msreferences( refs );

        m << CentroidMethod();
        m << ElementalCompositionMethod();
        m << IsotopeMethod();

        auto m3 = MSCalibrateMethod();
        m3.references( refs );
        m << m3;

        m << MSChromatogramMethod();
        m << MSLockMethod();
        m << PeakMethod();
        m << QuanCompounds();
        m << QuanMethod();
        m << TargetingMethod();
        m << CentroidMethod();
    }

    bool
    write_process_method( const adcontrols::ProcessMethod& m, const std::string& name )
    {
        std::ofstream fo( name );
        return adportable::binary::serialize<>()( m, fo );
    }

    bool
    read_process_method( adcontrols::ProcessMethod& m, const std::string& name )
    {
        BOOST_TEST_CHECKPOINT("read_process_method!");
        std::ifstream fi( name );
        if ( !fi.fail() )
            return adportable::binary::deserialize<>()( m, fi );
        return false;
    }

}

using namespace test;

bool
process_method::test()
{
    BOOST_TEST_CHECKPOINT("process_method::test!");

    adcontrols::ProcessMethod m, t;
    make_process_method( m );

    write_process_method( m, "processmethod1.bin" );
    read_process_method( t, "processmethod1.bin" );

    std::wofstream xml( "processmethod1.xml" );

    return adportable::xml::serialize<>()( t, xml );
}
