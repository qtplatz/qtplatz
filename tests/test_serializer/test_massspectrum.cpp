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

#include "test_massspectrum.hpp"

#include <adcontrols/massspectrum.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/xml_serializer.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/filesystem/path.hpp>
#include <fstream>
#include <iostream>

namespace test {

    void
    make_massspectrum( adcontrols::MassSpectrum& m )
    {
    }

    bool
    write_massspectrum( const adcontrols::MassSpectrum& m, const std::string& name )
    {
        std::ofstream fo( name );
        return adportable::binary::serialize<>()( m, fo );
    }

    bool
    read_massspectrum( adcontrols::MassSpectrum& m, const std::string& name )
    {
        std::ifstream fi( name );
        if ( !fi.fail() )
            return adportable::binary::deserialize<>()( m, fi );
        return false;
    }

}

using namespace test;

bool
massspectrum::test()
{
    BOOST_TEST_CHECKPOINT("massspectrum::test!");
    constexpr const char * const file = "massspectrum1.bin";
    boost::filesystem::path xmlfile = boost::filesystem::path( file ).replace_extension( "xml" );

    adcontrols::MassSpectrum m, t;
    make_massspectrum( m );

    BOOST_CHECK( write_massspectrum( m, file ) );
    BOOST_CHECK( read_massspectrum( t, file ) );

    std::wofstream xml( xmlfile.string().c_str() );

    return adportable::xml::serialize<>()( t, xml );
}
