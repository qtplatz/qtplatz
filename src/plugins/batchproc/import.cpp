/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "import.hpp"
#include <adportable/debug.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <thread>
#include <chrono>

using namespace batchproc;

import::import()
{
}

import::import( int row
                , const std::wstring& source_file
                , const std::wstring& destination_file
                , std::function< bool(int, int, int) > progress ) : rowId_( row )
                                                                  , source_file_( source_file )
                                                                  , destination_file_( destination_file )
                                                                  , progress_( progress )
                                                                  , datafile_(0)
                                                                  , accessor_(0)
{
    datafile_ = adcontrols::datafile::open( source_file, true );
}

import::~import()
{
    if ( datafile_ )
        adcontrols::datafile::close( datafile_ );
}

bool
import::operator()()
{
    if ( datafile_ ) {

        datafile_->accept( *this );
        if ( tic_.empty() )
            return false;

        const adcontrols::Chromatogram& chro = *tic_[0];
        size_t nSpectra = chro.size();

        for ( int i = 0; i < nSpectra; ++i ) {

            if ( progress_( rowId_, i, chro.size() ) ) {
                progress_( rowId_, 0, 0 ); // canceled
                return false;
            }

            adcontrols::MassSpectrum ms;
            accessor_->getSpectrum( 0, i, ms );
            // std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
        }

        progress_( rowId_, nSpectra, nSpectra ); // completed
        return true;
    }
    progress_( rowId_, 0, 0 ); // 
    return false;
}

bool
import::subscribe( const adcontrols::LCMSDataset& data )
{
    accessor_ = &data;
    size_t nfcn = data.getFunctionCount();
    for ( size_t i = 0; i < nfcn; ++i ) {
        auto c = std::make_shared< adcontrols::Chromatogram >();
        if ( data.getTIC( i, *c ) )
            tic_.push_back( c );
    }
    return true;
}
