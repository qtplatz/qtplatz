/**************************************************************************
** Copyright (C) 2010-2018 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2018 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "pkdavgwriter.hpp"
#include "constants.hpp"
#include <acqrscontrols/constants.hpp>
#include <acqrscontrols/counting_data_writer.hpp>
#include <acqrscontrols/pkd_counting_data_writer.hpp>
#include <acqrscontrols/threshold_result_accessor.hpp>
#include <acqrscontrols/u5303a/threshold_result.hpp>
#include <acqrscontrols/u5303a/waveform.hpp>
#include <acqrscontrols/waveform_accessor.hpp>
#include <adacquire/datawriter.hpp>
#include <adacquire/sampleprocessor.hpp>
#include <adacquire/samplesequence.hpp>
#include <adacquire/task.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/debug.hpp>
#include <adportable/profile.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <fstream>

using namespace accutof::acquire;

PKDAVGWriter::PKDAVGWriter()
{
}

PKDAVGWriter::~PKDAVGWriter()
{
}

PKDAVGWriter&
PKDAVGWriter::operator << ( std::array< std::shared_ptr< const acqrscontrols::u5303a::waveform >, 2 >&& ptr )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    cache_.emplace_back( ptr );

    return *this;
}

void
PKDAVGWriter::commitData()
{
    std::shared_ptr< acqrscontrols::waveform_accessor_< acqrscontrols::u5303a::waveform > > wforms, pkds;

    {
        std::lock_guard< std::mutex > lock( mutex_ );

        if ( cache_.size() >= 3 ) {
            wforms = std::make_shared< acqrscontrols::waveform_accessor_< acqrscontrols::u5303a::waveform > >();
            pkds = std::make_shared< acqrscontrols::waveform_accessor_< acqrscontrols::u5303a::waveform > >();

            std::for_each ( cache_.begin(), cache_.end(), [&]( const auto& a ){
                    wforms->list.emplace_back( std::move( a[ 0 ] ) );
                    if ( a[ 1 ] )
                        pkds->list.emplace_back( std::move( a[ 1 ] ) );
                });
            cache_.clear();
        }
    }

    if ( wforms && pkds ) {

        if ( auto tmp = std::make_shared< adacquire::SignalObserver::DataWriter >( wforms ) )
            adacquire::task::instance()->handle_write( acqrscontrols::u5303a::waveform_observer, tmp );

        if ( !pkds->list.empty() ) {
            if ( auto tmp = std::make_shared< adacquire::SignalObserver::DataWriter >( pkds ) )
                adacquire::task::instance()->handle_write( acqrscontrols::u5303a::pkd_observer, tmp );

            //==== u5303a threshold count (digitizer) data to trigger/peak table  ===========================
            if ( auto dataWriter = std::make_shared< acqrscontrols::pkd_counting_data_writer >( pkds ) )
                adacquire::task::instance()->handle_write( acqrscontrols::u5303a::pkd_observer, dataWriter );
        }

    };
}

void
PKDAVGWriter::fsmStop()
{
    commitData();
}
