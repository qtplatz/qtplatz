/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "quanprocessor.hpp"
#include <adcontrols/quansample.hpp>
#include <adcontrols/quansequence.hpp>
#include <adcontrols/processmethod.hpp>
#include <adlog/logger.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/polfit.hpp>
#include <adwidgets/progresswnd.hpp>

#include <boost/filesystem.hpp>
#include <algorithm>
#include <numeric>
#include <set>
#include <vector>

namespace quan {

    struct QuanCalibrant {
        uint64_t idCompound_; // primary key for compound
        uint64_t uniqId_; // compound Id
        std::string formula_;
        std::set< uint64_t > level_;
        std::vector< double > x_;
        std::vector< double > y_;
        std::vector< double > coeffs_;
        void operator << ( const std::pair<double, double >& amount_response ) {
            x_.push_back( amount_response.second ); // intensity
            y_.push_back( amount_response.first );  // amount
        }
        QuanCalibrant() : uniqId_( 0 ), idCompound_( 0 ) {}
        QuanCalibrant( const QuanCalibrant& t ) : uniqId_( t.uniqId_ )
                                                , idCompound_( t.idCompound_ )
                                                , formula_( t.formula_ )
                                                , level_( t.level_ )
                                                , x_( t.x_ )
                                                , y_( t.y_ )
                                                , coeffs_( t.coeffs_ ) {
        }
    };

    struct QuanUnknown {
        uint64_t idResponse_; // QuanResponse primary id
        uint64_t uniqId_;
        double response_;
        QuanUnknown() : idResponse_(0), uniqId_(0), response_(0) {}
        QuanUnknown( const QuanUnknown& t ) : idResponse_( t.idResponse_ ), uniqId_( t.uniqId_ ), response_( t.response_ ) {
        }
    };

}

using namespace quan;

QuanProcessor::~QuanProcessor()
{
}

QuanProcessor::QuanProcessor()
{
}

QuanProcessor::QuanProcessor( const QuanProcessor& t ) : sequence_( t.sequence_ )
                                                       , que_( t.que_ )
                                                       , progress_( t.progress_ )
                                                       , progress_total_( t.progress_total_ )
                                                       , progress_count_( t.progress_count_ )
{
}

QuanProcessor::QuanProcessor( std::shared_ptr< adcontrols::QuanSequence >& s
                              , std::shared_ptr< adcontrols::ProcessMethod >& pm ) : sequence_( s )
                                                                                   , procmethod_( pm )
                                                                                   , progress_( adwidgets::ProgressWnd::instance()->addbar() )
                                                                                   , progress_total_(0)
                                                                                   , progress_count_(0)
{
    // combine per dataSource
    for ( auto it = sequence_->begin(); it != sequence_->end(); ++it )
        que_[ it->dataSource() ].push_back( *it );
    progress_total_ = std::accumulate( que_.begin(), que_.end(), 0, [] ( int n, const decltype(*que_.begin())& q ){ return n + int( q.second.size() ); } );
    (*progress_)(0, progress_total_);
}

adcontrols::QuanSequence *
QuanProcessor::sequence()
{
    return sequence_.get();
}

const adcontrols::QuanSequence *
QuanProcessor::sequence() const
{
    return sequence_.get();
}

const std::shared_ptr< adcontrols::ProcessMethod >&
QuanProcessor::procmethod() const
{
    return procmethod_;
}

QuanProcessor::iterator
QuanProcessor::begin()
{
    return que_.begin();
}

QuanProcessor::iterator
QuanProcessor::end()
{
    return que_.end();
}

QuanProcessor::const_iterator
QuanProcessor::begin() const
{
    return que_.begin();
}

QuanProcessor::const_iterator
QuanProcessor::end() const
{
    return que_.end();
}

void
QuanProcessor::complete( const adcontrols::QuanSample * )
{
    (*progress_)(++progress_count_, int( que_.size() ));
}

void
QuanProcessor::doCalibration()
{
    boost::filesystem::path database( sequence_->outfile() );
    if ( !boost::filesystem::exists( database ) )
        return;

    int nLevels = 1;
    int nReplicates = 1;
    adcontrols::QuanMethod::CalibEq eq = adcontrols::QuanMethod::idCalibLinear;

    if ( auto qm = procmethod_->find< adcontrols::QuanMethod >() ) {
        nLevels = qm->levels();
        nReplicates = qm->replicates();
        eq = qm->equation();
    }

    std::vector< std::pair< uint64_t, std::string > > compounds; // <uniqId, formula>

    adfs::filesystem fs;
    if ( fs.mount( database.wstring().c_str() ) ) {
        adfs::stmt sql( fs.db() );

        std::map< uint64_t, QuanCalibrant > calibrants;
        std::vector< QuanUnknown > unknowns;

        if ( sql.prepare( "\
SELECT QuanCompound.id, QuanResponse.formula, QuanResponse.intensity, QuanAmount.amount, QuanSample.level, QuanSample.sampleType \
FROM QuanSample,QuanResponse,QuanAmount,QuanCompound \
WHERE sampleType = 1 \
AND QuanSample.id = idSample \
AND QuanCompound.id = QuanResponse.idCompound \
AND QuanCompound.id = QuanAmount.idCompound \
AND QuanAmount.level = QuanSample.level \
ORDER BY QuanCompound.id" ) ) {

            while ( sql.step() == adfs::sqlite_row ) {
                int row = 0;
                uint64_t idCompound = sql.get_column_value< uint64_t >( row++ );     // QuanCompound.id
                std::string formula = sql.get_column_value< std::string >( row++ );  // QuanResponse.formula
                double intensity = sql.get_column_value< double >( row++ );          // QuanResponse.intensity
                double amount = sql.get_column_value< double >( row++ );             // QuanAmount.amount (standard amount added)
                uint64_t level = sql.get_column_value< uint64_t >( row++ );          // QuanSample.level
                uint64_t sampType = sql.get_column_value< uint64_t >( row++ );       // STD (1)

                QuanCalibrant& d = calibrants[ idCompound ];
                d.idCompound_ = idCompound;
                // d.uniqId_ = uniqId;
                d.level_.insert( level );
                d << std::make_pair( amount, intensity );
            }
        }

        for ( auto& calibrant: calibrants ) {
            auto& calib = calibrant.second;
            int levels = std::min( int( calib.level_.size() ), nLevels );
            std::vector< double > coeffs;
            double chisqr;
            if ( levels >= 2 ) {
                //adportable::polfit::fit( calib.second.x_.data(), calib.second.y_.data(), calib.second.x_.size(), 2, coeffs, chisqr, adportable::polfit::WEIGHTING_NONE );
                adportable::polfit::fit( calib.x_.data(), calib.y_.data(), calib.x_.size(), 2, coeffs );
            } else {
                double sum_x = std::accumulate( calib.x_.begin(), calib.x_.end(), 0.0, []( double a, double b ){ return a + b; } );
                double sum_y = std::accumulate( calib.y_.begin(), calib.y_.end(), 0.0, []( double a, double b ){ return a + b; });
                coeffs.push_back( sum_y / sum_x ); // one point, average
            }
            sql.begin();
            for ( auto& a : coeffs ) {
                if ( sql.prepare( "INSERT INTO QuanCalib (idCompound, coeffs) VALUES (?,?)" ) ) {

                    sql.bind( 1 ) = calib.idCompound_;
                    sql.bind( 2 ) = a;

                    if ( sql.step() != adfs::sqlite_done )
                        ADTRACE() << "sql error";
                }
            }
            sql.commit();
            calib.coeffs_ = coeffs;
        }

        if ( sql.prepare( "\
SELECT QuanResponse.id, QuanResponse.formula, QuanResponse.uniqId, QuanResponse.intensity \
FROM QuanSample, QuanResponse \
WHERE sampleType = 0 AND QuanSample.id = QuanResponse.idSample" ) ) {
            
            while ( sql.step() == adfs::sqlite_row ) {
                int row = 0;
                QuanUnknown unk;
                unk.idResponse_ = sql.get_column_value< uint64_t >( row++ );
                std::string formula = sql.get_column_value< std::string >( row++ );
                unk.uniqId_ = sql.get_column_value< uint64_t >( row++ );
                unk.response_ = sql.get_column_value< double >( row++ );
                unknowns.push_back( unk );
            }
        }
#if 0
        for ( auto& unk : unknowns ) {
            auto it = calibrants.find( unk.idCompound_ );
            if ( it != calibrants.end() && it->second.coeffs_.empty() ) {
                double est_a = adportable::polfit::estimate_y( it->second.coeffs_, unk.response_ );

                if ( sql.prepare( "UPDATE QuanResponse SET calibId = ?, amount = ? WHERE QuanResponse.id = ?" ) ) {
                    sql.bind( 1 ) = uint64_t( 0 );
                    sql.bind( 2 ) = est_a;
                    //sql.bind( 3 ) = unk.respId_;
                    if ( sql.step() != adfs::sqlite_row )
                        ADTRACE() << "sel error.";
                }
            }
        }
#endif
    }    
}

void
QuanProcessor::doQuantification()
{
}
