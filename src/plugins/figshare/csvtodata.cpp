/**************************************************************************
** Copyright (C) 2014-2024 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "csvtodata.hpp"
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/chromatogram.hpp>

using namespace figshare;

// using variant_type = std::variant< std::shared_ptr< adcontrols::MassSpectrum >
//                                    , std::shared_ptr< adcontrols::Chromatogram >
//                                    , std::shared_ptr< QwtSeriesData< QPointF > >
//                                    >;

variant_type
csv::toData::operator ()( const std::vector< adportable::csv::list_string_type >& vlist ) const
{
    if ( not vlist.empty() ) {
        const auto& alist = vlist.at(0);
        if ( alist.size() >= 2
             && alist.at(0).first.type() == typeid(double) && alist.at(1).first.type() == typeid(double) ) {
            // assume mass spectrum
            auto ms = std::make_shared< adcontrols::MassSpectrum >();
            ms->resize( vlist.size() );
            ms->setCentroid( adcontrols::CentroidNative );

            size_t n = 0;
            for (const auto& alist: vlist ) {
                ms->setValue( n++, { boost::get<double>( alist.at(0).first ), boost::get<double>(alist.at(1).first) } );
            }
            return ms;
        } else {
            ADDEBUG() << "============= toData error alist.size: " << alist.size();
            ADDEBUG() << "============= toData error alist.type 0: " << alist.at(0).first.type().name();
            ADDEBUG() << "============= toData error alist.type 1: " << alist.at(1).first.type().name();
        }
    }
    return {};
}
