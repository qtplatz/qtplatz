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

#include "test_massspectrum.hpp"
// #include <adfs/adfs.hpp>
// #include <adfs/cpio.hpp>
// #include <adfs/sqlite.hpp>
// #include <adcontrols/massspectrum.hpp>
// #include <adportfolio/portfolio.hpp>
// #include <boost/filesystem.hpp>
// #include <boost/test/execution_monitor.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_parameters.hpp>
// #include <fstream>
// #include <functional>
// #include <iostream>

using boost::unit_test_framework::test_suite;
using namespace boost::unit_test;

BOOST_AUTO_TEST_CASE( adfs_massspectrum_create )
{
    boost::unit_test::unit_test_log.set_threshold_level( log_messages );
    // unit test framework can catch operating system signals
    BOOST_TEST_CHECKPOINT("About to create adfs massspectrum file!");
    BOOST_CHECK( test_adfs::massspectrum::test_create() == true );
}

BOOST_AUTO_TEST_CASE( adfs_massspectrum_read )
{
    // unit test framework can catch operating system signals
    BOOST_TEST_CHECKPOINT("About to read massspectrum file!");
    BOOST_CHECK( test_adfs::massspectrum::test_read() == true );
}

BOOST_AUTO_TEST_CASE( adfs_massspectrum_read_1 )
{
    // unit test framework can catch operating system signals
    BOOST_TEST_CHECKPOINT("About to read massspectrum file!");
    BOOST_CHECK( test_adfs::massspectrum::test_read( "waveform.adfs" ) == true );
}

BOOST_AUTO_TEST_CASE( adfs_massspectrum_read_2 )
{
    // unit test framework can catch operating system signals
    BOOST_TEST_CHECKPOINT("About to read massspectrum file!");
    BOOST_CHECK( test_adfs::massspectrum::test_read( "waveform2.adfs" ) == true );
}
