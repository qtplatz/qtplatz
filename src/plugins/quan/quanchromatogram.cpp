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

using namespace quan;

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
{
}
        
void
QuanChromatogram::append( uint32_t pos, double time, double value )
{
    indecies_.push_back( pos );
    ( *chromatogram_ ) << std::make_pair( time, value );
}

void
QuanChromatogram::identify( const adcontrols::QuanCompounds& cmpds )
{
    if ( peakinfo_ ) {

        auto& peaks = peakinfo_->peaks();

        for ( auto& cmpd : cmpds ) {

            double refSeconds = cmpd.tR();
            double tolerance = ( msrange_.second - msrange_.first ) * 10;
            double lMass = cmpd.mass() - tolerance;
            double uMass = cmpd.mass() + tolerance;

            if ( lMass < cmpd.mass() && cmpd.mass() < uMass ) {
                
                auto it = std::find_if( peaks.begin(), peaks.end(), [refSeconds] ( const adcontrols::Peak& pk ) {
                        return (pk.peakTime() - pk.peakWidth()) < refSeconds && refSeconds < ( pk.peakTime() + pk.peakWidth() ); } );
                
                if ( it != peaks.end() ) {
                    
                    for ( auto next = it; next != peaks.end() &&
                          ( ( ( next->peakTime() - next->peakWidth() ) < refSeconds ) &&
                              ( refSeconds < ( next->peakTime() + next->peakWidth() ) ) ); ++next ) {
                        
                        if ( std::abs( it->peakTime() - refSeconds ) > std::abs( next->peakTime() - refSeconds ) )
                            it = next; // take closest one
                        
                    }
                    
                    it->formula( cmpd.formula() );
                    auto text = adcontrols::ChemicalFormula::formatFormula( adportable::utf::to_wstring( cmpd.formula() ) );
                    it->name( text );
                    
                }
                
            }
        }
        
    }
}

