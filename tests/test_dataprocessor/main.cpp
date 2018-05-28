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
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include "dataprocessor.hpp"
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_parameters.hpp>

using boost::unit_test_framework::test_suite;
using namespace boost::unit_test;

namespace test_dataprocessor {
    constexpr const char * const __datapath = "data/mouse/100ppmN2O_CO2_50ul_0001.adfs";
    constexpr const char * const __datapath_nofile = "data/mouse/100ppmN2O_CO2_50ul_0001xxx.adfs";
}

BOOST_AUTO_TEST_CASE( data_open0 )
{
    BOOST_CHECK( test_dataprocessor::dataprocessor::init() == true );
    
    boost::filesystem::path dpath = boost::filesystem::path( getenv( "HOME" ) ) / test_dataprocessor::__datapath;
    
    // unit test framework can catch operating system signals
    BOOST_TEST_CHECKPOINT("About to read data file!");
    BOOST_CHECK( test_dataprocessor::dataprocessor::test0( dpath ) == true );
}

BOOST_AUTO_TEST_CASE( data_open1 )
{
    BOOST_CHECK( test_dataprocessor::dataprocessor::init() == true );
    
    boost::filesystem::path dpath = boost::filesystem::path( getenv( "HOME" ) ) / test_dataprocessor::__datapath;
    
    // unit test framework can catch operating system signals
    BOOST_TEST_CHECKPOINT("About to read data file!");
    BOOST_CHECK( test_dataprocessor::dataprocessor::test1( dpath ) == true );
}

#if 0
BOOST_AUTO_TEST_CASE( data_open2 )
{
    boost::filesystem::path dpath = boost::filesystem::path( getenv( "HOME" ) ) / test_dataprocessor::__datapath;
    
    // unit test framework can catch operating system signals
    BOOST_TEST_CHECKPOINT("About to read data file!");
    BOOST_CHECK( test_dataprocessor::dataprocessor::test2( dpath ) == true );
}
#endif
