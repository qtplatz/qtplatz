/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <adcontrols/quanmethod.hpp>
#include <adcontrols/quansample.hpp>
#include <adcontrols/quansequence.hpp>
#include <adcontrols/processmethod.hpp>
#include <adlog/logger.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/polfit.hpp>
#include <adportable/debug.hpp>
#include <adwidgets/progresswnd.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
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
    progress_total_ = std::accumulate( que_.begin(), que_.end(), 0, [] ( int n, decltype(*que_.begin())& q ){ return n + int( q.second.size() ); } );
    progress_->setRange( 0, progress_total_ );
}

QuanProcessor::QuanProcessor( std::shared_ptr< adcontrols::QuanSequence > s
                              , std::shared_ptr< adcontrols::ProcessMethod > pm
                              , size_t nThreads ) : sequence_( s )
                                                  , procmethod_( pm )
                                                  , progress_( adwidgets::ProgressWnd::instance()->addbar() )
                                                  , progress_total_(0)
                                                  , progress_count_(0)
{
    // combine per number-of-threads
    size_t n(0);
    for ( auto it = sequence_->begin(); it != sequence_->end(); ++it ) {
        auto ident = ( boost::wformat( L"processor_%d" ) % ( n++ % nThreads ) ).str();
        que_[ ident ].push_back( *it );
    }
    progress_total_ = std::accumulate( que_.begin(), que_.end(), 0, [] ( int n, decltype(*que_.begin())& q ){ return n + int( q.second.size() ); } );
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

    if ( qM->isCounting() ) {
        doCountingCalibration( db );
        return;
    }

    adcontrols::QuanCalibrations results;
    std::map< uint64_t, adcontrols::QuanCalibration > calibrants;

    adfs::stmt sql( db );

    sql.begin();

    if ( !QuanDataWriter::insert_table( sql, results.ident(), "Create QuanCalib a.k.a. ResultSet" ) )
        return;

    int nLevels = qM->levels(); // must be 1 or larger
    adcontrols::QuanMethod::CalibEq eq = qM->equation();
    int order = qM->polynomialOrder(); // if CalibEq >= isCalibLinear, otherwise taking an average

    std::map< uint64_t, std::set< int > > levels;
    std::string query =
        "SELECT QuanCompound.id"
        ", QuanCompound.uuid"
        ", QuanCompound.idTable"
        ", QuanResponse.id"
        ", QuanResponse.formula"
        ", QuanResponse.intensity"
        ", QuanAmount.amount"
        ", QuanSample.level"
        " FROM QuanSample, QuanResponse, QuanCompound, QuanAmount"
        " WHERE QuanSample.id = QuanResponse.idSample"
        " AND QuanResponse.idCmpd = QuanCompound.uuid"
        " AND QuanAmount.idCompound = QuanCompound.id AND QuanAmount.level = QuanSample.level"
        " AND sampleType = 1"
        " ORDER BY QuanCompound.id";

    if ( sql.prepare( query ) ) {

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

        switch( eq ) {
        case adcontrols::QuanMethod::idCalibOnePoint:
            do {
                map.second.fit( 1 ); // single point, average 
            } while (0);
            break;
        case adcontrols::QuanMethod::idCalibLinear_origin:
            do {
                map.second.fit( 2, true ); // force zero for 1'st term
            } while(0);
            break;
        case adcontrols::QuanMethod::idCalibLinear:
            map.second.fit( 2 );
            break;
        case adcontrols::QuanMethod::idCalibPolynomials:
            map.second.fit( std::min( order + 1, nLevels ) );
            break;
        }

        results << map.second;

        if ( sql.prepare( "INSERT INTO QuanCalib (uuid, idCompound, idTable, idCmpd, idMethod, n, min_x, max_x, chisqr, a, b, c, d, e, f)"
                          " VALUES (:uuid, ?, ?, ?, ?, :n, :min_x, :max_x, :chisqr, :a, :b, :c, :d, :e, :f)" ) ) {
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
    auto qM = procmethod_->find< adcontrols::QuanMethod >();
    if ( !qM )
        return;

    if ( qM->isCounting() ) {
        doCountingQuantification( db );
        return;
    }
    
    bool isCounting = qM->isCounting();
    
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

    std::string query = 
        "SELECT QuanResponse.id, QuanSample.id, QuanCompound.formula, intensity, a, b, c, d, e, f"
        " FROM QuanResponse, QuanCompound, QuanSample, QuanCalib"
        " WHERE QuanResponse.idCmpd = QuanCompound.uuid"
        " AND QuanSample.id = QuanResponse.idSample"
        " AND QuanSample.sampleType = 0"
        " AND QuanResponse.idCmpd = QuanCalib.idCmpd";
    
    if ( sql.prepare( query ) ) {
        
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
            if ( unk.second.coeffs.size() == 1 ) {
                unk.second.amount = unk.second.coeffs[ 0 ] * unk.second.intensity;
            } else {
                unk.second.amount = adportable::polfit::estimate_y( unk.second.coeffs, unk.second.intensity );
            }
            if ( sql.prepare( "UPDATE QuanResponse SET amount = ? WHERE id = ?" ) ) {
                sql.bind( 1 ) = unk.second.amount;
                sql.bind( 2 ) = unk.first;
                if ( sql.step() != adfs::sqlite_done )
                    ADERROR() << "sql error";
            }
        }
    }
    sql.commit();

}

void
QuanProcessor::doCountingCalibration( adfs::sqlite& db )
{
    auto qM = procmethod_->find< adcontrols::QuanMethod >();
    if ( !qM )
        return;

    bool isISTD = qM->isInternalStandard();

    adcontrols::QuanCalibrations results;
    std::map< uint64_t, adcontrols::QuanCalibration > calibrants;

    adfs::stmt sql( db );

    sql.begin();

    if ( !QuanDataWriter::insert_table( sql, results.ident(), "Create QuanCalib a.k.a. ResultSet" ) )
        return;

    int nLevels = qM->levels(); // must be 1 or larger
    adcontrols::QuanMethod::CalibEq eq = qM->equation();
    int order = qM->polynomialOrder(); // if CalibEq >= isCalibLinear, otherwise taking an average

    std::map< uint64_t, std::set< int > > levels;
    std::string query;
    if ( isISTD ) {
        query = 
            "SELECT id, uuid, idTable, formula, r1.CR/r2.CR, amount, level, r1.idSample FROM "
            "(SELECT QuanCompound.id"
            ", QuanCompound.uuid"
            ", QuanCompound.idTable"
            ", QuanResponse.formula"
            ", QuanResponse.intensity / QuanResponse.trigCounts as CR"  // <== CountRate
            ", QuanAmount.amount"
            ", QuanSample.level"
            ", idSample"
            " FROM QuanSample, QuanResponse, QuanCompound, QuanAmount"
            " WHERE QuanSample.id = QuanResponse.idSample"
            " AND QuanResponse.idCmpd = QuanCompound.uuid"
            " AND QuanAmount.idCompound = QuanCompound.id AND QuanAmount.level = QuanSample.level"
            " AND sampleType = 1 AND isISTD=0 ) r1"
            " LEFT JOIN"
            "(SELECT idSample,QuanResponse.intensity/QuanResponse.trigCounts as CR" // <== CountRate
            " FROM QuanSample,QuanResponse,QuanCompound, QuanAmount"
            " WHERE QuanSample.id = QuanResponse.idSample"
            " AND QuanResponse.idCmpd = QuanCompound.uuid"
            " AND QuanAmount.idCompound = QuanCompound.id AND QuanAmount.level = QuanSample.level"
            " AND sampleType = 1 AND isISTD=1 ) r2"            
            " ON r1.idSample=r2.idSample ORDER BY r1.id";
    } else {
        query = "SELECT QuanCompound.id"
            ", QuanCompound.uuid"
            ", QuanCompound.idTable"
            ", QuanResponse.formula"
            ", QuanResponse.intensity * 60000 / QuanResponse.trigCounts"
            ", QuanAmount.amount"
            ", QuanSample.level"
            " FROM QuanSample, QuanResponse, QuanCompound, QuanAmount"
            " WHERE QuanSample.id = QuanResponse.idSample"
            " AND QuanResponse.idCmpd = QuanCompound.uuid"
            " AND QuanAmount.idCompound = QuanCompound.id AND QuanAmount.level = QuanSample.level"
            " AND sampleType = 1"
            " ORDER BY QuanCompound.id";        
    }

    if ( sql.prepare( query ) ) {

        ADDEBUG() << query;

        while ( sql.step() == adfs::sqlite_row ) {
            int row = 0;
            try {
                uint64_t idCompound = sql.get_column_value< uint64_t >( row++ );     // QuanCompound.id
                boost::uuids::uuid idCmpd = sql.get_column_value< boost::uuids::uuid >( row++ );  // QuanCompound.idCmpd
                boost::uuids::uuid idTable = sql.get_column_value< boost::uuids::uuid >( row++ ); // QuanCompound.idCmpdTable
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

                ADDEBUG() << "add calib: " << formula << "\t" << amount << "\t" << intensity;

            } catch ( std::bad_cast& ex ) {
                BOOST_THROW_EXCEPTION( ex );
            }
        }
    }
    
    for ( auto& map : calibrants ) {

        switch( eq ) {
        case adcontrols::QuanMethod::idCalibOnePoint:
            do {
                map.second.fit( 1 ); // single point, average
            } while (0);
            break;
        case adcontrols::QuanMethod::idCalibLinear_origin:
            do {
                map.second.fit( 2, true ); // force zero for 1'st term
            } while(0);
            break;
        case adcontrols::QuanMethod::idCalibLinear:
            map.second.fit( 2 );
            break;
        case adcontrols::QuanMethod::idCalibPolynomials:
            map.second.fit( std::min( order + 1, nLevels ) );
            break;
        }

        results << map.second;
        
        if ( sql.prepare( "INSERT INTO QuanCalib "
                          "(uuid, idCompound, idTable, idCmpd, idMethod, n, min_x, max_x, chisqr, a, b, c, d, e, f)"
                          "VALUES"
                          "(:uuid, ?, ?, ?, ?, :n, :min_x, :max_x, :chisqr, :a, :b, :c, :d, :e, :f)" ) ) {
            int row = 1;
            sql.bind( row++ ) = results.ident().uuid();        // :uuid
            sql.bind( row++ ) = map.first;                     // ?idCompound
            sql.bind( row++ ) = map.second.uuid_cmpd_table();  // ?idCmpdTable
            sql.bind( row++ ) = map.second.uuid_cmpd();        // ?idCmpd
            sql.bind( row++ ) = qM->ident().uuid();            // ?idMethod
            sql.bind( row++ ) = uint64_t( map.second.size() ); // :n
            if ( eq == adcontrols::QuanMethod::idCalibOnePoint ) {
                sql.bind( row++ ) = 0.0;  // :min_x
                sql.bind( row++ ) = double( map.second.max_x() );  // :max_x
            } else {
                sql.bind( row++ ) = double( map.second.min_x() );  // :min_x
                sql.bind( row++ ) = double( map.second.max_x() );  // :max_x                
            }
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
QuanProcessor::doCountingQuantification( adfs::sqlite& db )
{
    bool isISTD( false );

    if ( auto qM = procmethod_->find< adcontrols::QuanMethod >() )
        isISTD = qM->isInternalStandard();
    
    adfs::stmt sql( db );

    struct unknown {
        uint64_t idSamp;
        std::string formula;
        double intensity;
        double amount;
        int64_t calibid;
        unknown() : idSamp(0), intensity(0), amount(0) {}
        unknown( const unknown& t ) : formula( t.formula), intensity( t.intensity ), amount( t.amount ) {}
    };

    std::map< uint64_t, unknown > unknowns;
    std::map< boost::uuids::uuid, std::pair< int64_t, std::vector< double > > > calib;
    if ( sql.prepare( "SELECT idCmpd,id,a,b,c,d,e,f FROM QuanCalib" ) ) {
        while ( sql.step() == adfs::sqlite_row ) {
            int row = 0;
            boost::uuids::uuid idCmpd  = sql.get_column_value< boost::uuids::uuid >( row++ );
            int64_t calibid = sql.get_column_value< int64_t >( row++ );
            calib[ idCmpd ].first = calibid;
            for ( int i = 0; i < 5; ++i ) {
                if ( sql.is_null_column( row ) )
                    break;
                calib[ idCmpd ].second.emplace_back( sql.get_column_value< double >( row++ ) );
            }
        }
    }
    
    std::string query;
    if ( isISTD ) {
        query =
            "SELECT idCmpd, id, r1.idSample, r1.formula, r1.CountRate/r2.CountRate FROM"
            " (SELECT QuanResponse.idCmpd, QuanResponse.id, idSample, sampleType, QuanCompound.formula"
            " , intensity * 60000 / trigCounts as CountRate"
            "  FROM QuanSample, QuanResponse, QuanCompound"
            "  WHERE QuanSample.id = idSample"
            "  AND QuanResponse.idCmpd = QuanCompound.uuid"
            "  AND isISTD=0 AND sampleType=0) r1"
            " LEFT JOIN"
            " (SELECT idSample, QuanCompound.formula,intensity * 60000 / trigCounts as 'CountRate'"
            "  FROM QuanResponse,QuanCompound"
            "  WHERE QuanResponse.idCmpd=QuanCompound.uuid AND isISTD=1) r2"
            " ON r1.idSample=r2.idSample ORDER BY r1.idSample";
    } else {
        query = "SELECT QuanResponse.idCmpd, QuanResponse.id, QuanSample.id, QuanCompound.formula"
            ", QuanResponse.intensity * 60000 / QuanResponse.trigCounts"
            " FROM QuanResponse, QuanCompound, QuanSample"
            " WHERE QuanResponse.idCmpd = QuanCompound.uuid"
            " AND QuanSample.id = QuanResponse.idSample"
            " AND QuanSample.sampleType = 0";
    }

    // unknown values
    if ( sql.prepare( query ) ) {
        
        while ( sql.step() == adfs::sqlite_row ) {
            int row = 0;
            boost::uuids::uuid idCmpd = sql.get_column_value< boost::uuids::uuid >( row++ );
            uint64_t idResp = sql.get_column_value< uint64_t >( row++ ); // QuanResponse.id
            unknown& u = unknowns[ idResp ];
            u.idSamp = sql.get_column_value< uint64_t >( row++ ); // QuanSample.id
            u.formula = sql.get_column_value< std::string >( row++ ); // 
            u.intensity = sql.get_column_value< double >( row++ ); // QuanResponse.id

            auto it = calib.find( idCmpd );
            if ( it != calib.end() ) {
                auto& coeffs = it->second.second;
                u.calibid = it->second.first;
                if ( coeffs.size() == 1 ) {
                    u.amount = coeffs[ 0 ] * u.intensity;
                } else {
                    u.amount = adportable::polfit::estimate_y( coeffs, u.intensity );
                }
            }
            
        }
    }

    sql.begin();
    for ( auto& unk : unknowns ) {
        if ( sql.prepare( "UPDATE QuanResponse SET amount = ?, calibid = ? WHERE id = ?" ) ) {
            sql.bind( 1 ) = unk.second.amount;
            sql.bind( 2 ) = unk.second.calibid;
            sql.bind( 3 ) = unk.first;
            if ( sql.step() != adfs::sqlite_done )
                ADERROR() << "sql error";
        }
    }
    sql.commit();

}
