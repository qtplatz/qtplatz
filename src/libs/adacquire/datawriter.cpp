/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "datawriter.hpp"
#include "signalobserver.hpp"
#include <adfs/file.hpp>
#include <adfs/filesystem.hpp>
#include <adportable/debug.hpp>
#include <adutils/acquiredconf_v3.hpp>
#include <adutils/acquireddata_v3.hpp>
#include <compiler/boost/workaround.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <atomic>
#include <algorithm>
#include <chrono>
#include <mutex>

using namespace adacquire::SignalObserver;

uint32_t DataWriter::idCounter_ = 0;

DataWriter::DataWriter( std::shared_ptr< DataAccess >&& accessor ) : accessor_( accessor )
                                                                   , myId_( ++idCounter_ )
{
    accessor_->rewind();
}

DataWriter::~DataWriter()
{
}

void
DataWriter::rewind()
{
    accessor_->rewind();
}

bool
DataWriter::next()
{
    return accessor_->next();
}

uint64_t
DataWriter::epoch_time() const
{
    return accessor_->epoch_time();
}

uint64_t
DataWriter::elapsed_time() const
{
    return accessor_->elapsed_time();
}

uint64_t
DataWriter::pos() const
{
    return accessor_->pos();
}

uint32_t
DataWriter::fcn() const
{
    return accessor_->fcn();
}

uint32_t
DataWriter::ndata() const
{
    return uint32_t ( accessor_->ndata() );
}

uint32_t
DataWriter::events() const
{
    return accessor_->events();
}

size_t
DataWriter::xdata( std::string& data ) const
{
    return accessor_->xdata( data );
}

size_t
DataWriter::xmeta( std::string& data ) const
{
    return accessor_->xmeta( data );
}

// virtual
bool
DataWriter::write( adfs::filesystem& fs, const boost::uuids::uuid& objId ) const
{
#if !defined NDEBUG && 0
    ADDEBUG() << "#### DataWriter BASE WRITER ##### " << objId;
#endif
    std::string xdata, xmeta;
    this->xdata ( xdata );
    this->xmeta ( xmeta );
    if ( ! adutils::v3::AcquiredData::insert ( fs.db(), objId
                                               , this->elapsed_time()
                                               , this->epoch_time()
                                               , this->pos()
                                               , this->fcn()
                                               , this->ndata()
                                               , this->events()
                                               , xdata
                                               , xmeta )  ) {
        ADDEBUG() << "AcquiredData::insert failed";
    }
    return true;
}

//////////////////
DataAccess::~DataAccess()
{
}

DataAccess::DataAccess()
{
}

void
DataAccess::rewind()
{
}

bool
DataAccess::next()
{
    return false;
}

size_t
DataAccess::ndata() const
{
    return 0;
}     // number of data in the buffer

uint64_t
DataAccess::elapsed_time() const
{
    return 0;
}

uint64_t
DataAccess::epoch_time() const
{
    return 0;
}

uint64_t
DataAccess::pos() const
{
    return 0;
}       // data address (sequencial number for first data in this frame)

uint32_t
DataAccess::fcn() const
{
    return 0;
}       // function number for spectrum

uint32_t
DataAccess::events() const
{
    return 0;
}

size_t
DataAccess::xdata( std::string& ) const
{
    return 0;
}

size_t
DataAccess::xmeta( std::string& ) const
{
    return 0;
}

std::optional< std::pair< uint64_t, uint64_t > >
DataAccess::pos_range() const
{
    return {};
}
