/**************************************************************************
** Copyright (C) 2013-2020 MS-Cheminformatics LLC
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

#include "datareader.hpp"
#include <adcontrols/datafile.hpp>
#include <adcontrols/datareader.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/massspectrum.hpp>
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
#include <boost/python.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <memory>
#include <mutex>

using namespace py_module;

DataReader::DataReader()
{
}

DataReader::DataReader( const DataReader& t ) : reader_( t.reader_ )
                                              , it_( t.it_ )
{
}

DataReader::DataReader( std::shared_ptr< const adcontrols::DataReader > t ) : reader_( t )
{
    if ( reader_ )
        it_ = reader_->begin();
}

DataReader::~DataReader()
{
}

boost::uuids::uuid
DataReader::objuuid() const
{
    return reader_ ? reader_->objuuid() : boost::uuids::uuid{{0}};
}

std::string
DataReader::objtext() const
{
    return reader_ ? reader_->objtext() : "";
}

std::string
DataReader::display_name() const
{
    return reader_ ? reader_->display_name() : "";
}

size_t
DataReader::size( int fcn ) const
{
    return reader_ ? reader_->size( fcn ) : 0;
}

std::shared_ptr< adcontrols::MassSpectrum >
DataReader::readSpectrum() const
{
    if ( reader_ ) {
        return reader_->readSpectrum( it_ );
    }
    return nullptr;
}

bool
DataReader::rewind()
{
    if ( reader_ ) {
        it_ = reader_->begin();
        return true;
    }
    return false;
}

bool
DataReader::next()
{
    if ( reader_ && it_ )
        return it_++ != reader_->end();
    return false;
}

int64_t
DataReader::rowid() const
{
    if ( reader_ && it_ )
        return it_->rowid();
    return -1;
}

int64_t
DataReader::epoch_time() const
{
    if ( reader_ && it_ )
        return it_->epoch_time();
    return -1;
}

int64_t
DataReader::elapsed_time() const
{
    if ( reader_ && it_ )
        return it_->elapsed_time();
    return -1;
}

double
DataReader::time_since_inject() const
{
    if ( reader_ && it_ )
        return it_->time_since_inject();
    return -1;
}

int
DataReader::protocol() const
{
    if ( reader_ && it_ )
        return it_->fcn();
    return -1;
}
