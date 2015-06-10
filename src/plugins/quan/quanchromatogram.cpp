/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "quanchromatogram.hpp"
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/peak.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/quancompound.hpp>
#include <adcontrols/quancompounds.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adportable/utf.hpp>
#include <adportable/float.hpp>

namespace quan {
    namespace quanchromatogram {
        namespace adc = adcontrols;        

        struct tRComp {

            bool operator () ( const adc::Peak& pk, double refTime ) { 
                return ( pk.peakTime() - pk.peakWidth() ) < refTime;  }
            bool operator () ( double refTime, const adc::Peak& pk ) { return 
                refTime < ( pk.peakTime() + pk.peakWidth() );  }
            
            bool operator () ( const adc::QuanCompound& cmpd, double refTime ) { return cmpd.tR() < refTime; }
            bool operator () ( double refTime, const adc::QuanCompound& cmpd ) { return refTime < cmpd.tR(); }
        };

    }
}

using namespace quan;
using namespace quan::quanchromatogram;

QuanChromatogram::QuanChromatogram( uint32_t fcn
                                    , uint32_t candidate_index
                                    , const std::string& formula
                                    , double exactMass
                                    , double matchedMass
                                    , const std::pair< double, double >& range ) : fcn_( fcn )
                                                                                 , candidate_index_( candidate_index )
                                                                                 , formula_( formula )
                                                                                 , exactMass_( exactMass )
                                                                                 , matchedMass_( matchedMass )
                                                                                 , msrange_( range )
                                                                                 , chromatogram_( std::make_shared< adcontrols::Chromatogram >() )
                                                                                 , peakinfo_( std::make_shared< adcontrols::PeakResult >() )
                                                                                 , peakId_( -1 )
                                                                                 , count_( 0 )
{
}
        
void
QuanChromatogram::append( uint32_t pos, double time, double value )
{
    if ( count_++ == 0 && pos > 0 )
        return;  // ignore first data after chromatogram condition change
    
    indecies_.push_back( pos );
    ( *chromatogram_ ) << std::make_pair( time, value );
}

bool
QuanChromatogram::identify( const adcontrols::QuanCompounds& cmpds, const std::string& formula )
{
    if ( peakinfo_ ) {
        using adcontrols::QuanCompound;
        namespace adc = adcontrols;

        // auto p = std::make_pair( std::lower_bound( cmpds.begin(), cmpds.end(), massrange_.first, [=](const QuanCompound& a, double m) { return a.mass() < m;} )
        //                          , std::upper_bound( cmpds.begin(), cmpds.end(), massrange_.second, [=](double m, const QuanCompound& a) { return m < a.mass(); } ) );

        auto cmpd = std::find_if( cmpds.begin(), cmpds.end(), [formula]( const QuanCompound& a ){ return a.formula() == formula; } );
        if ( cmpd == cmpds.end() )
            return false;

        double refSeconds = cmpd->tR();
        if ( adportable::compare<double>::essentiallyEqual( refSeconds, 0.0 ) )
            return false;  // no time specified

        auto& peaks = peakinfo_->peaks();
        if ( peaks.size() == 0 )
            return false;

        // find first occurance that refSeconds is grater than peak start, wich can be out of scope
        auto pk = std::lower_bound( peaks.begin(), peaks.end(), refSeconds
                                    , [] ( const adc::Peak& a, double t ) { return a.peakTime() < ( t - a.peakTime() - a.peakWidth() / 2 ); } );
        
        if ( pk == peaks.end() )
            return false;

        while ( ( pk + 1 ) != peaks.end() ) {
            if ( std::abs( pk->peakTime() - refSeconds ) > std::abs( ( pk + 1 )->peakTime() - refSeconds ) )
                ++pk;
            else
                break;
        }
        if ( pk->startTime() < refSeconds && refSeconds < pk->endTime() ) {
            pk->formula( formula.c_str() );
            pk->name( adc::ChemicalFormula::formatFormula( adportable::utf::to_wstring( pk->formula() ) ) );
            peakId_ = pk->peakId();
        }
    }
    return is_identified();
}

bool
QuanChromatogram::is_identified() const
{
    return peakId_ != uint32_t(-1);
}

uint32_t
QuanChromatogram::identfied_peakid() const
{
    return peakId_;
}

const adcontrols::Peak *
QuanChromatogram::find_peak( uint32_t peakId ) const
{
    if ( peakinfo_ ) {
        auto it = peakinfo_->peaks().find_peakId( peakId );
        if ( it != peakinfo_->peaks().end() )
            return &( *it );
    }
    return 0;
}

std::vector< adcontrols::Peak * >
QuanChromatogram::peaks()
{
    std::vector< adcontrols::Peak* > vec;

    if ( peakinfo_ ) {
        for ( auto& pk: peakinfo_->peaks() )
            vec.push_back( &pk );
    }
    return vec;
}

uint32_t
QuanChromatogram::pos_from_peak( const adcontrols::Peak& pk ) const
{
    if ( pk.topPos() < indecies_.size() ) {
        
        return indecies_[ pk.topPos() ];
        
    }
    return (-1);
}

void
QuanChromatogram::setReferenceDataGuid( const std::wstring& dataGuid, uint32_t idx, uint32_t fcn )
{
    dataGuids_.push_back( std::make_tuple( dataGuid, idx, fcn ) );
}

void
QuanChromatogram::setDataGuid( const std::wstring& dataGuid )
{
    dataGuid_ = dataGuid;
}

const std::wstring&
QuanChromatogram::dataGuid() const
{
    return dataGuid_;
}
