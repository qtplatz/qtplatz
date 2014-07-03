/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#include "msquant.hpp"
#include "msqpeaks.hpp"
#include "msqpeak.hpp"
#include <adportable/polfit.hpp>

using namespace adcontrols;

MSQuant::~MSQuant()
{
}

MSQuant::MSQuant()
{
}

bool
MSQuant::operator()( MSQPeaks& peaks, std::function<void(MSQPeak&)> callback )
{
    std::map< std::wstring, std::vector< const MSQPeak * > > stdv;
    std::map< std::wstring, std::vector< double > > polynomials;

    std::for_each( peaks.begin(), peaks.end(), [&] ( const MSQPeak& pk ){
            if ( pk.isSTD() )
                stdv[ pk.componentId() ].push_back( &pk );
        });
    
    for ( auto& ref: stdv ) {
        std::vector< double > amounts, intens, coeffs;
        for ( auto t : ref.second ) {
            amounts.push_back( t->amount() );
            intens.push_back( t->intensity() );
        }
        if ( amounts.size() >= 2 )
            adportable::polfit::fit( intens.data(), amounts.data(), amounts.size(), 2, coeffs );
        else {
            coeffs.push_back( 0.0 ); // force though origin
            coeffs.push_back( amounts[ 0 ] / intens[ 0 ] ); // slope
        }
        polynomials[ ref.first ] = coeffs;
    }
    
    for ( auto& pk: peaks ) {
        if ( !pk.isSTD() && ! pk.componentId().empty() ) {
            auto it = polynomials.find( pk.componentId() );
            if ( it != polynomials.end() ) {
                double amount = adportable::polfit::estimate_y( it->second, pk.intensity() );
                pk.amount( amount );
                if ( callback )
                    callback( pk );
            }
        }
    }

    return true;
}
