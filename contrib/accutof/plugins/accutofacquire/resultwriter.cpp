/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "resultwriter.hpp"
#include "constants.hpp"
#include <acqrscontrols/counting_data_writer.hpp>
#include <acqrscontrols/threshold_result_accessor.hpp>
#include <acqrscontrols/u5303a/threshold_result.hpp>
#include <acqrscontrols/u5303a/waveform.hpp>
#include <acqrscontrols/constants.hpp>
#include <adfs/sqlite.hpp>
#include <adfs/filesystem.hpp>
#include <adacquire/datawriter.hpp>
#include <adacquire/samplesequence.hpp>
#include <adacquire/sampleprocessor.hpp>
#include <adacquire/task.hpp>
#include <adportable/debug.hpp>
#include <adportable/profile.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <fstream>

using namespace accutof::acquire;

ResultWriter::ResultWriter() : acquisition_active_( false )
{
}

ResultWriter::~ResultWriter()
{
}

ResultWriter&
ResultWriter::operator << ( std::shared_ptr< const acqrscontrols::u5303a::threshold_result >&& ptr )
{
#if !defined NDEBUG && 0
    static size_t count = 0;
    if ( count % 10 == 0 )
        ADDEBUG() << "ResultWriter << threshold_result";
#endif
    std::lock_guard< std::mutex > lock( mutex_ );
    cache0_.emplace_back( ptr );
    return *this;
}

#if 0
ResultWriter&
ResultWriter::operator << ( std::shared_ptr< const acqrscontrols::u5303a::waveform >&& ptr )
{
#if !defined NDEBUG
    static size_t count = 0;
    if ( count % 10 == 0 )
        ADDEBUG() << "ResultWriter << u5303a::waveform";
#endif
    std::lock_guard< std::mutex > lock( mutex_ );
    // cache1_.emplace_back( ptr );
    return *this;
}
#endif

ResultWriter&
ResultWriter::operator << ( std::shared_ptr< const acqrscontrols::ap240_threshold_result >&& ptr )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    cache2_.emplace_back( ptr );
    return *this;
}

namespace accutof { namespace acquire {

        /////////////////////////// AP240 ///////////////////////////
        template<>
        void
        ResultWriter::commitData_< acqrscontrols::ap240_threshold_result >( std::vector< std::shared_ptr< const acqrscontrols::ap240_threshold_result > >& cache )
        {
            auto accessor = std::make_shared< acqrscontrols::threshold_result_accessor_< acqrscontrols::ap240_threshold_result > >();

            do {
                std::lock_guard< std::mutex > lock( mutex_ );

                if ( cache.size() >= 3 ) {
                    auto it = std::next( cache.begin(), cache.size() - 2 );
                    std::move( cache.begin(), it, std::back_inserter( accessor->list ) );
                    cache.erase( cache.begin(), it );
                }

            } while ( 0 );

            if ( !accessor->list.empty() ) {
                //==== ap240 threshold count (digitizer) data to trigger/peak table  ===========================
                auto dataWriter = std::make_shared< acqrscontrols::counting_data_writer_< acqrscontrols::ap240_threshold_result > >( accessor );
                adacquire::task::instance()->handle_write( acqrscontrols::ap240::timecount_observer, dataWriter );
                //<==============================================================================================
            }

        }

        /////////////////////////// U5303A ///////////////////////////
        template<>
        void
        ResultWriter::commitData_< acqrscontrols::u5303a::threshold_result >(
            std::vector< std::shared_ptr< const acqrscontrols::u5303a::threshold_result > >& )
        {
            auto accessor = std::make_shared< acqrscontrols::threshold_result_accessor >();
            do {
                std::lock_guard< std::mutex > lock( mutex_ );
                auto& cache = cache0_;

                if ( cache.size() >= 3 ) {
                    auto it = std::next( cache.begin(), cache.size() - 2 );
                    std::move( cache.begin(), it, std::back_inserter( accessor->list ) );
                    cache.erase( cache.begin(), it );
                }
            } while ( 0 );

            if ( !accessor->list.empty() ) {
                //==== u5303a threshold count (digitizer) data to trigger/peak table  ===========================
                auto dataWriter = std::make_shared< acqrscontrols::counting_data_writer >( std::move( accessor ) );
                adacquire::task::instance()->handle_write( acqrscontrols::u5303a::timecount_observer, dataWriter );
                //<==============================================================================================
            }
        }

    }
}


void
ResultWriter::commitData()
{
    commitData_<>( cache0_ );
}

void
ResultWriter::fsmStop()
{
    acquisition_active_ = false;
}
