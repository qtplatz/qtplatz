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

#include "dataprocessor.hpp"
#include <adcontrols/datafile.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrometer_factory.hpp>
#include <adcontrols/massspectrometerbroker.hpp>
#include <adfs/filesystem.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin_manager/loader.hpp>
#include <adplugin_manager/manager.hpp>
#include <adportable/debug.hpp>
#include <adportable/utf.hpp>
#include <adprocessor/dataprocessor.hpp>

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <memory>

namespace test_dataprocessor {

    class dataSubscriber : public adcontrols::dataSubscriber {
    public:
        const adcontrols::LCMSDataset * raw_;
        bool hasProcessedDataset;
    public:
        virtual ~dataSubscriber() {}
        dataSubscriber() : raw_( 0 )
                         , hasProcessedDataset( false ) {
        }

        virtual bool subscribe( const adcontrols::LCMSDataset& raw ) override {
            raw_ = &raw;
            return true;
        }
        virtual bool subscribe( const adcontrols::ProcessedDataset& ) override {
            hasProcessedDataset = true;
            ADDEBUG() << __FUNCTION__ << " hasProcessedDataset = " << hasProcessedDataset;
            return true;
        }
        virtual void notify( adcontrols::dataSubscriber::idError, const std::string& json ) override {
            ADDEBUG() << __FUNCTION__ << " Error: " << json;
        }
    };
}

using namespace test_dataprocessor;

bool
dataprocessor::init()
{
    adplugin::manager::standalone_initialize();
    return true;
}

bool
dataprocessor::test0( const boost::filesystem::path& datapath )
{
    if ( auto file = adcontrols::datafile::open( datapath.wstring(), false ) ) {

        auto fs = std::make_unique< adfs::filesystem >();
        BOOST_REQUIRE( fs->mount( datapath ) );

        dataSubscriber subscriber;
        //file->accept( subscriber );

        //BOOST_REQUIRE( subscriber.raw_ != nullptr );
        //BOOST_CHECK( subscriber.hasProcessedDataset );

        // if ( subscriber.raw_ ) {
        //     BOOST_CHECK( subscriber.raw_->dataReaderCount() > 0 );
        // }
        delete file;

        return true;
    }
    return false;
}

bool
dataprocessor::test1( const boost::filesystem::path& datapath )
{
    if ( auto file = std::unique_ptr< adcontrols::datafile >( adcontrols::datafile::open( datapath.wstring(), false ) ) ) {

        auto fs = std::make_unique< adfs::filesystem >();
        BOOST_REQUIRE( fs->mount( datapath ) );
        dataSubscriber subscriber;

        file->accept( subscriber );
#if 0

        BOOST_REQUIRE( subscriber.raw_ != nullptr );
        BOOST_CHECK( subscriber.hasProcessedDataset );

        if ( subscriber.raw_ ) {
            BOOST_CHECK( subscriber.raw_->dataReaderCount() > 0 );
        }
#endif
        return true;
    }
    return false;
}

bool
dataprocessor::test2( const boost::filesystem::path& datapath )
{
    BOOST_TEST_CHECKPOINT("dataprocessor::test!");
    if ( auto dp = std::make_unique< adprocessor::dataprocessor >() ) {
        std::wstring errmsg;
        BOOST_REQUIRE( dp->open( datapath.wstring(), errmsg ) );
        BOOST_TEST_CHECKPOINT("dataprocessor::test!");
        ADDEBUG() << "file: " << datapath.string() << " open success";
    }
    return true;
}
