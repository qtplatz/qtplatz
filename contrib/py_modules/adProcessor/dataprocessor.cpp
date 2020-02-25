/**************************************************************************
** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2019 MS-Cheminformatics LLC
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
#include "datareader.hpp"
#include <adcontrols/datafile.hpp>
#include <adcontrols/datareader.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrometer_factory.hpp>
#include <adcontrols/massspectrometerbroker.hpp>
#include <adportfolio/portfolio.hpp>
#include <adprocessor/dataprocessor.hpp>
#include <adfs/filesystem.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin_manager/loader.hpp>
#include <adplugin_manager/manager.hpp>
#include <adportable/debug.hpp>
#include <adportable/utf.hpp>
#include <adprocessor/dataprocessor.hpp>
#include <boost/filesystem.hpp>
#include <compiler/boost/workaround.hpp>
#include <boost/python.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <memory>
#include <mutex>

using namespace py_module;

dataProcessor::~dataProcessor()
{
}

dataProcessor::dataProcessor() : processor_( std::make_shared< adprocessor::dataprocessor >() )
{
    static std::once_flag flag;
    std::call_once( flag, []{ adplugin::manager::standalone_initialize(); } );
}

bool
dataProcessor::open( const std::wstring& filename )
{
    std::wstring emsg;
    return processor_->open( filename, emsg );

#if 0
    if ( file_ )
        delete file_;
    raw_ = nullptr;

    if ( ( file_ = adcontrols::datafile::open( filename ) ) ) {
        file_->accept( *this );
    }

    if ( raw_ ) {
        for ( auto reader: raw_->dataReaders() )
            ADDEBUG() << reader->objtext() << ", " << reader->display_name() << ", " << reader->objuuid();
    }

    return file_ != nullptr;
#endif
}

std::vector< boost::python::tuple >
dataProcessor::dataReaderTuples() const
{
    std::vector< boost::python::tuple > a;
    if ( processor_ ) {
        if ( auto raw = processor_->rawdata() ) {
            for ( auto reader: raw->dataReaders() ) {
                a.emplace_back( boost::python::make_tuple( reader->objtext()
                                                           , reader->display_name()
                                                           , reader->objuuid() )   );
            }
        }
    }
    return a;
}

std::vector< std::shared_ptr< py_module::DataReader > >
dataProcessor::dataReaders() const
{
    std::vector< std::shared_ptr< DataReader > > a;
    if ( processor_ ) {
        if ( auto raw = processor_->rawdata() ) {
            for ( auto& reader: raw->dataReaders() )
                a.emplace_back( std::make_shared< DataReader >( reader ) );
        }
    }
    return a;
}

std::shared_ptr< py_module::DataReader >
dataProcessor::dataReader( const std::string& str ) const
{
    auto uuid = boost::uuids::string_generator()( str );
    if ( processor_ ) {
        if ( auto raw = processor_->rawdata() ) {
            for ( auto& reader: raw->dataReaders() ) {
                if ( reader->objuuid() == uuid )
                    return std::make_shared< py_module::DataReader >( reader );
            }
        }
    }
    return nullptr;
}

std::wstring
dataProcessor::filename() const
{
    if ( processor_ )
        return processor_->filename();
    return {};
}

std::string
dataProcessor::xml() const
{
    if ( processor_ )
        return processor_->portfolio().xml();
    return {};
}

folder
dataProcessor::root() const
{
    // return folder( processor_->fs()->root() );
    if ( processor_ && processor_->fs() )
        return processor_->fs()->findFolder( L"/Processed" );
    return {};
}
