// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2024 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2024 MS-Cheminformatics LLC
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

#include "lockmass.hpp"
#include "annotation.hpp"
#include "annotations.hpp"
#include "chemicalformula.hpp"
#include "molecule.hpp"
#include "massspectrum.hpp"
#include "mspeakinfo.hpp"
#include "mspeakinfoitem.hpp"
#include "segment_wrapper.hpp"
#include <adportable/debug.hpp>
#include <adportable/polfit.hpp>
#include <adportable/float.hpp>
#include <adportable_serializer/portable_binary_archive.hpp>
#include <adportable_serializer/portable_binary_oarchive.hpp>
#include <adportable_serializer/portable_binary_iarchive.hpp>
#include <adportable/date_time.hpp>
#include <adportable/iso8601.hpp>
#include <boost/json.hpp> // #if BOOST_VERSION >= 107500
#include <adportable/json/extract.hpp>
#include <adportable/json_helper.hpp>
#include <cstring>
#include <chrono>

using namespace adcontrols;
using namespace adcontrols::lockmass;

mslock::mslock() : posix_time_( 0 )
{
}

mslock::mslock( const mslock& t ) : references_( t.references_ )
                                  , fitter_( t.fitter_ )
                                  , posix_time_( t.posix_time_ )
                                  , property_( t.property_ )
{
}

mslock::operator bool () const
{
    return !references_.empty();
}

mslock&
mslock::operator << ( const reference& t )
{
    auto it = std::find_if( references_.begin(), references_.end(), [t]( const reference& a ){
        return adportable::compare<double>::essentiallyEqual( t.exactMass(), a.exactMass() );
    });
    if ( it == references_.end() )
        references_.emplace_back( t );
    else
        *it = t;

	return *this;
}

mslock&
mslock::operator += ( const mslock& rhs )
{
    for ( auto a: rhs ) {
        if ( a.exactMass() > 1.0 ) {
            auto it = std::find_if( references_.begin(), references_.end(), [&]( const reference& b ){
                    return adportable::compare<double>::essentiallyEqual( a.exactMass(), b.exactMass() );
                });
            if ( it == references_.end() )
                references_.emplace_back( a );
            else
                *it = a;
        }
    }
    return *this;
}


reference::reference() : exactMass_(0)
                       , matchedMass_(0)
                       , time_(0)
{
}

reference::reference( const reference& t ) : formula_( t.formula_ )
                                           , exactMass_( t.exactMass_ )
                                           , matchedMass_( t.matchedMass_ )
                                           , time_( t.time_ )
{
}

reference::reference( const std::string& formula
                      , double exactMass
                      , double matchedMass
                      , double time ) : formula_( formula )
                                      , exactMass_( exactMass )
                                      , matchedMass_( matchedMass )
                                      , time_( time )
{
}

const std::string&
reference::formula() const
{
    return formula_;
}

double
reference::exactMass() const
{
    return exactMass_;
}

double
reference::matchedMass() const
{
    return matchedMass_;
}

double
reference::time() const
{
    return time_;
}

//////////////////////
void
mslock::clear()
{
    references_.clear();
}

size_t
mslock::size() const
{
    return references_.size();
}

bool
mslock::empty() const
{
    return references_.empty();
}

mslock::const_iterator
mslock::begin() const
{
    return references_.begin();
}

mslock::const_iterator
mslock::end() const
{
    return references_.end();
}

const std::vector< double >&
mslock::coeffs() const
{
    return fitter_.coeffs();
}

const lockmass::fitter&
mslock::fitter() const
{
    return fitter_;
}

int64_t
mslock::posix_time() const
{
    return posix_time_;
}

std::optional< std::string >
mslock::property() const
{
    if ( property_.empty() )
        return {};
    return property_;
}

void
mslock::setProperty( std::pair< std::string, std::string >&& keyValue )
{
    property_ = boost::json::serialize( boost::json::value{ keyValue.first, keyValue.second } );
}

///////////////////
#if 0
// static
bool
mslock::findReferences( mslock& lk,  const adcontrols::MassSpectrum& ms )
{
    static ChemicalFormula formulaParser;

    lk.clear();
    adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segs( ms );
    for ( auto& fms: segs ) {
        for ( const auto& a: fms.annotations() ) {
            if ( a.dataFormat() == annotation::dataFormula && a.index() >= 0 ) {
                const std::string& formula = a.text();
                double exactMass = formulaParser.getMonoIsotopicMass( formula );
                double matchedMass = fms.mass( a.index() );
                double time = fms.time( a.index() );
                lk << reference( formula, exactMass, matchedMass, time );
            }
        }
    }
    return true;
}
#endif

// static
bool
mslock::findReferences( mslock& lk,  const adcontrols::MassSpectrum& ms, int idx, int fcn )
{
    static ChemicalFormula formulaParser;

    if ( idx < 0 || fcn < 0 )
        return false;

    adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segs( ms );
    if ( size_t(fcn) >= segs.size() )
        return false;

    const double matchedMass = segs[ fcn ].mass( idx );
    const double matchedTime = segs[ fcn ].time( idx );

    const auto& annots = segs[ fcn ].annotations();

    auto pred1 = [&]( const auto& a){ return a.index() == idx; };

    auto it = std::find_if( annots.begin(), annots.end(), pred1 );
    std::optional< std::string > formula;
    std::optional< adcontrols::annotation::reference_molecule > refMol;

    while ( it != annots.end() ) {
        if ( it->dataFormat() == annotation::dataFormula ) {
            formula = it->text();
        } else if ( it->dataFormat() == annotation::dataJSON ) {
            auto value = adportable::json_helper::parse( it->text() );
            if ( value != boost::json::value{} ) {
                boost::system::error_code ec;
                if ( auto ptr = adportable::json_helper::find_pointer( value, "/refernce_molecule", ec ) ) {
                    refMol = boost::json::value_to< adcontrols::annotation::reference_molecule >( value );
                }
            }
        }
        it = std::find_if( ++it, annots.end(), pred1 );
    };

    if ( refMol ) {
        auto mol = adcontrols::ChemicalFormula::toMolecule( refMol->formula_, refMol->adduct_ );
        lk << reference( mol.formula(), mol.mass(), matchedMass, matchedTime );
        return true;
    }
    if ( formula ) {
        auto list = adcontrols::ChemicalFormula::split( *formula );
        double exactMass = formulaParser.getMonoIsotopicMass( list ).first;
        lk << reference( *formula, exactMass, matchedMass, matchedTime );
        return true;
    }
    return false;
}

bool
mslock::fit()
{
    fitter_.clear();

    posix_time_ = std::chrono::duration_cast< std::chrono::nanoseconds >( std::chrono::system_clock::now().time_since_epoch() ).count();

    if ( references_.size() == 1 ) {
        auto& ref = references_[0];
        double error = ref.matchedMass() - ref.exactMass();
        double relativeError = error / ref.matchedMass();

        fitter_ = std::vector< double >{  relativeError };
        return true;

    } else if ( references_.size() >= 2 ) {

        std::vector< double > x, y;

        for ( const auto& ref: references_ ) {
            double error = ref.matchedMass() - ref.exactMass();
            x.push_back( ref.matchedMass() );
            y.push_back( error );
        }
        return adportable::polfit::fit( x.data(), y.data(), x.size(), 2, fitter_.coeffs() );
    }

    return false;
}

bool
mslock::operator()( MassSpectrum& ms, bool applyToAll ) const
{
    if ( applyToAll ) {
        std::pair< double, double > range(1000000.0, 0.0);

        for ( auto& fms : adcontrols::segment_wrapper<>( ms ) ) {
            fitter_( fms );
            range.first = std::min( fms.mass( 0 ), range.first );
            range.second = std::max( fms.mass( fms.size() - 1 ), range.second );
        }

        range.first = range.first - ( range.second - range.first ) / 100;
        range.second = range.second + ( range.second - range.first ) / 100;

        ms.setAcquisitionMassRange( range.first, range.second );

        return true;

    } else
        return fitter_( ms );
}

bool
mslock::operator()( MSPeakInfo& info, bool applyToAll ) const
{
    if ( applyToAll ) {
        for ( auto& xinfo : adcontrols::segment_wrapper< MSPeakInfo >( info ) )
            fitter_( xinfo );
        return true;
    } else
        return fitter_( info );
}

fitter::fitter()
{
}

fitter::fitter( const fitter& t ) : coeffs_( t.coeffs_ )
{
}

fitter::fitter( std::vector< double >&& a )
{
    coeffs_ = std::move( a );
}

fitter::fitter( const std::array< double, 2 >& a )
{
    coeffs_.resize(2);
    std::copy( a.begin(), a.end(), coeffs_.begin() );
}


fitter::fitter( const std::vector< double >& a )
{
    coeffs_ = a;
}

fitter&
fitter::operator = ( std::vector< double >&& a )
{
    coeffs_ = std::move( a );
    return *this;
}

const std::vector< double >&
fitter::coeffs() const
{
    return coeffs_;
}

std::vector< double >&
fitter::coeffs()
{
    return coeffs_;
}

bool
fitter::operator()( MassSpectrum& ms ) const
{
    if ( coeffs_.empty() )
        return false;

    const double * masses = ms.getMassArray();

    if ( coeffs_.size() == 1 ) {
        // relative error correction
        for ( size_t i = 0; i < ms.size(); ++i ) {
            double mass = masses[ i ] - masses[ i ] * coeffs_[ 0 ];
            ms.setMass( i, mass );
        }

    } else {

        for ( size_t i = 0; i < ms.size(); ++i ) {
            double mass = masses[i] - adportable::polfit::estimate_y( coeffs_, masses[ i ] );
            ms.setMass( i, mass );
        }

    }

	return true;
}

bool
fitter::operator()( MSPeakInfo& pkInfo ) const
{
    if ( coeffs_.empty() )
        return false;

    if ( coeffs_.size() == 1 ) {
        // relative error correction
        for ( auto& item: pkInfo ) {
            double mass = item.mass() - item.mass() * coeffs_[ 0 ];
            item.assign_mass( mass );
        }

    } else {

        for ( auto& item: pkInfo ) {
            double mass = item.mass() - adportable::polfit::estimate_y( coeffs_, item.mass() );
            item.assign_mass( mass );
        }

    }

	return true;
}

void
fitter::clear()
{
    coeffs_.clear();
}

////////////////

namespace adcontrols {
    namespace lockmass {

        void
        tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const reference& t )
        {
            jv = {
                { "formula", t.formula_ }
                , { "exactMass", t.exactMass_ }
                , { "matchedMass", t.matchedMass_ }
                , { "time", t.time_ }
            };
        }

        reference
        tag_invoke( const boost::json::value_to_tag< reference >&, const boost::json::value& jv )
        {
            using namespace adportable::json;

            if ( jv.kind() == boost::json::kind::object ) {
                reference t;
                auto obj = jv.as_object();
                extract( obj, t.formula_         , "formula"               );
                extract( obj, t.exactMass_       , "exactMass"             );
                extract( obj, t.matchedMass_     , "matchedMass"           );
                extract( obj, t.time_            , "time"                  );
                return t;
            }
            return {};
        }

        ////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////
        void
        tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const fitter& t )
        {
            jv = {{ "coeffs", boost::json::value_from( t.coeffs_ ) }};
        }

        fitter
        tag_invoke( const boost::json::value_to_tag< fitter >&, const boost::json::value& jv )
        {
            using namespace adportable::json;

            if ( jv.kind() == boost::json::kind::object ) {
                fitter t;
                auto obj = jv.as_object();
                extract( obj, t.coeffs_, "coeffs" );
                return t;
            }
            return {};
        }

        ////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////
        void
        tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const mslock& t )
        {
            std::chrono::time_point< std::chrono::system_clock
                                     , std::chrono::nanoseconds > tp( std::chrono::nanoseconds( t.posix_time_ ) );
            auto dt = adportable::date_time::to_iso< std::chrono::microseconds >( tp );

            jv = {
                { "posix_time",  dt }
                , { "property",    t.property_ }
                , { "fitter",       boost::json::value_from( t.fitter_ ) }
                , { "references", boost::json::value_from( t.references_ ) }
            };
        }


        mslock
        tag_invoke( const boost::json::value_to_tag< mslock >&, const boost::json::value& jv )
        {
            if ( jv.kind() == boost::json::kind::object ) {
                mslock t;
                using namespace adportable::json;
                auto obj = jv.as_object();
                extract( obj, t.fitter_         , "fitter"               );
                extract( obj, t.references_     , "references"           );
                if ( obj.if_contains( "posix_time" ) ) {
                    std::string dt;
                    extract( obj, dt, "posix_time" );
                    if ( auto tp = adportable::iso8601::parse( dt.begin(), dt.end() ) ) {
                        t.posix_time_ = std::chrono::duration_cast< std::chrono::nanoseconds >( tp->time_since_epoch() ).count();
                    }
                }
                if ( obj.if_contains( "property" ) ) {
                    extract( obj, t.property_, "property" );
                }
                return t;
            }
            return {};
        }
    }
}

bool
mslock::archive( std::ostream& os, const mslock& t )
{
    os << boost::json::value_from( t ) << std::endl;
    return true;
}

bool
mslock::restore( std::istream& is, mslock& t )
{
    std::string s( std::istreambuf_iterator< char >( is ), {} );
    auto v = boost::json::parse( s );
    t = boost::json::value_to< mslock >( v );

    return true;
}
