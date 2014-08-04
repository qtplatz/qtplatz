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
#include "quandatawriter.hpp"
#include <adcontrols/quancalibrations.hpp>
#include <adcontrols/quansample.hpp>
#include <adcontrols/quansequence.hpp>
#include <adcontrols/processmethod.hpp>
#include <adlog/logger.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/polfit.hpp>
#include <adwidgets/progresswnd.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/exception/all.hpp>
#include <algorithm>
#include <numeric>
#include <set>
#include <vector>

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
    progress_->setRange( 0, progress_total_ );
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
    (*progress_)(); // ++progress_count_, progress_total_);
}

void
QuanProcessor::doCalibration( adfs::sqlite& db )
{
    auto qM = procmethod_->find< adcontrols::QuanMethod >();
    if ( !qM )
        return;

    adcontrols::QuanCalibrations results;
    std::map< uint64_t, adcontrols::QuanCalibration > calibrants;

    adfs::stmt sql( db );

    sql.begin();

    if ( !QuanDataWriter::insert_table( sql, results.ident(), "Create QuanCalib a.k.a. ResultSet" ) )
        return;

    int nLevels = qM->levels(); // must be 1 or larger
    // int nReplicates = qM->replicates(); // must be 1 or larger
    // adcontrols::QuanMethod::CalibEq eq = qM->equation();
    // int order = qM->polynomialOrder(); // if CalibEq >= isCalibLinear, otherwise taking an average


    std::map< uint64_t, std::set< int > > levels;

    if ( sql.prepare( "\
SELECT QuanCompound.id\
, QuanCompound.uuid \
, QuanCompound.idTable \
, QuanResponse.id \
, QuanResponse.formula \
, QuanResponse.intensity \
, QuanAmount.amount \
, QuanSample.level \
FROM QuanSample, QuanResponse, QuanCompound, QuanAmount \
WHERE QuanSample.id = QuanResponse.idSample \
AND QuanResponse.idCmpd = QuanCompound.uuid \
AND QuanAmount.idCompound = QuanCompound.id AND QuanAmount.level = QuanSample.level \
AND sampleType = 1 \
ORDER BY QuanCompound.id" ) ) {

        while ( sql.step() == adfs::sqlite_row ) {
            int row = 0;
            try {
                uint64_t idCompound = sql.get_column_value< uint64_t >( row++ );     // QuanCompound.id
                boost::uuids::uuid idCmpd = sql.get_column_value< boost::uuids::uuid >( row++ );  // QuanCompound.idCmpd
                boost::uuids::uuid idTable = sql.get_column_value< boost::uuids::uuid >( row++ ); // QuanCompound.idCmpdTable
                uint64_t idResp = sql.get_column_value< uint64_t >( row++ );
                (void)idResp;
                std::string formula = sql.get_column_value< std::string >( row++ );  // QuanResponse.formula
                double intensity = sql.get_column_value< double >( row++ );          // QuanResponse.intensity
                double amount = sql.get_column_value< double >( row++ );             // QuanAmount.amount (standard amount added)
                uint64_t level = sql.get_column_value< uint64_t >( row++ );          // QuanSample.level
                levels[ idCompound ].insert( level );  // count how meny levels are actually exists

                auto it = calibrants.find( idCompound );
                if ( it == calibrants.end() ) {
                    adcontrols::QuanCalibration& d = calibrants[ idCompound ];
                    d.uuid_cmpd( idCmpd );
                    d.uuid_cmpd_table( idTable );
                    d.formula( formula.c_str() );
                }
                adcontrols::QuanCalibration& d = calibrants[ idCompound ];
                // verify
                if ( d.uuid_cmpd() != idCmpd || d.uuid_cmpd_table() != idTable )
                    ADERROR() << "wrong calibrant data selected";
                d << std::make_pair( amount, intensity );
            }
            catch ( std::bad_cast& ex ) {
                BOOST_THROW_EXCEPTION( ex );
            }
        }
    }

    for ( auto& map : calibrants ) {
        map.second.fit( nLevels );
        results << map.second;

        if ( sql.prepare( "\
INSERT INTO QuanCalib (uuid, idCompound, idTable, idCmpd, idMethod, n, min_x, max_x, chisqr, a, b, c, d, e, f) \
VALUES (:uuid, ?, ?, ?, ?, :n, :min_x, :max_x, :chisqr, :a, :b, :c, :d, :e, :f)" ) ) {
            int row = 1;
            sql.bind( row++ ) = results.ident().uuid();        // :uuid
            sql.bind( row++ ) = map.first;                     // ?idCompound
            sql.bind( row++ ) = map.second.uuid_cmpd_table();  // ?idCmpdTable
            sql.bind( row++ ) = map.second.uuid_cmpd();        // ?idCmpd
            sql.bind( row++ ) = qM->ident().uuid();            // ?idMethod
            sql.bind( row++ ) = uint64_t( map.second.size() ); // :n
            sql.bind( row++ ) = double( map.second.min_x() );  // :min_x
            sql.bind( row++ ) = double( map.second.max_x() );  // :max_x
            sql.bind( row++ ) = double( map.second.chisqr() ); // chisqr
            size_t i = 0;
            while ( i < map.second.nTerms() )
                sql.bind( row++ ) = map.second.coefficients()[ i++ ];                     // a..f
            while ( i++ < 5 )
                sql.bind( row++ ) = adfs::null();                                         // a..f

            if ( sql.step() != adfs::sqlite_done )
                ADERROR() << "sql error";
        }
    }

    sql.commit();
}

void
QuanProcessor::doQuantification( adfs::sqlite& db )
{
    adfs::stmt sql( db );

    struct unknown {
        uint64_t idSamp;
        std::string formula;
        double intensity;
        double amount;
        std::vector< double > coeffs;
        unknown() : idSamp(0), intensity(0), amount(0) {}
        unknown( const unknown& t ) : formula( t.formula), intensity( t.intensity ), amount( t.amount ), coeffs( t.coeffs ) {}
    };

    std::map< uint64_t, unknown > unknowns;

    if ( sql.prepare( "\
SELECT QuanResponse.id, QuanSample.id, QuanCompound.formula, intensity, a, b, c, d, e, f \
FROM QuanResponse, QuanCompound, QuanSample, QuanCalib \
WHERE QuanResponse.idCmpd = QuanCompound.uuid \
AND QuanSample.id = QuanResponse.idSample \
AND QuanSample.sampleType = 0 \
AND QuanResponse.idCmpd = QuanCalib.idCmpd" ) ) {
        
        while ( sql.step() == adfs::sqlite_row ) {
            int row = 0;
            uint64_t idResp = sql.get_column_value< uint64_t >( row++ ); // QuanResponse.id
            unknown& u = unknowns[ idResp ];

            u.idSamp = sql.get_column_value< uint64_t >( row++ ); // QuanSample.id
            u.formula = sql.get_column_value< std::string >( row++ ); // 
            u.intensity = sql.get_column_value< double >( row++ ); // QuanResponse.id
            for ( int i = 0; i < 5; ++i ) {
                if ( sql.is_null_column( row ) )
                    break;
                u.coeffs.push_back( sql.get_column_value< double >( row++ ) );
            }
        }
    }

    sql.begin();
    for ( auto& unk : unknowns ) {
        if ( !unk.second.coeffs.empty() ) {
            unk.second.amount = adportable::polfit::estimate_y( unk.second.coeffs, unk.second.intensity );
            if ( sql.prepare( "UPDATE QuanResponse SET amount = :amount WHERE id = :id" ) ) {
                sql.bind( 1 ) = unk.second.amount;
                sql.bind( 2 ) = unk.first;
                if ( sql.step() != adfs::sqlite_done )
                    ADERROR() << "sql error";
            }
        }
    }
    sql.commit();

}
