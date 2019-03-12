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
#include <ads54j/constants.hpp>
#include <ads54j/counting_data_writer.hpp>
#include <ads54j/pkd_result.hpp>
#include <ads54j/pkd_result_accessor.hpp>
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

using namespace ads54j;

ResultWriter::ResultWriter() : acquisition_active_( false )
{
}

ResultWriter::~ResultWriter()
{
}

ResultWriter&
ResultWriter::operator << ( std::shared_ptr< const pkd_result >&& ptr )
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

namespace ads54j {

    template<>
    void
    ResultWriter::commitData_< pkd_result >( std::vector< std::shared_ptr< const pkd_result > >& )
    {
    }
}


void
ResultWriter::commitData()
{

    auto accessor = std::make_shared< ads54j::pkd_result_accessor >();
    do {
        std::lock_guard< std::mutex > lock( mutex_ );
        auto& cache = cache0_;

        if ( cache.size() >= 3 ) {
            auto it = std::next( cache.begin(), cache.size() - 2 );
            std::move( cache.begin(), it, std::back_inserter( accessor->list ) );
            cache.erase( cache.begin(), it );
        }
    } while ( 0 );

    // ADDEBUG() << "########### commit data ######### list.size=" << accessor->list.size();

    if ( !accessor->list.empty() ) {
        //==== threshold count data to trigger/peak table  ===========================
        auto dataWriter = std::make_shared< ads54j::counting_data_writer >( accessor );
        adacquire::task::instance()->handle_write( ads54j::tdc_observer, dataWriter );
        //<==============================================================================================
    }
}

void
ResultWriter::fsmStop()
{
    acquisition_active_ = false;
}
