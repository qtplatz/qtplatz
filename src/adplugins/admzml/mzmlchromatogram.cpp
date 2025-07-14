// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2025 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2025 MS-Cheminformatics LLC
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

#include "mzmlchromatogram.hpp"
#include "binarydataarray.hpp"
#include "xmltojson.hpp"
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/descriptions.hpp>
#include <adportable/debug.hpp>
#include <algorithm>
#include <pugixml.hpp>

namespace mzml {

    class mzMLChromatogram::impl {
    public:
        binaryDataArray prime_;
        binaryDataArray secondi_;
    public:
        impl() {}
        impl( binaryDataArray prime
              , binaryDataArray secondi ) : prime_( prime )
                                          , secondi_( secondi ) {
        }
    };


    mzMLChromatogram::~mzMLChromatogram()
    {
    }

    mzMLChromatogram::mzMLChromatogram() : impl_( std::make_unique< impl >() )
    {
    }

    mzMLChromatogram::mzMLChromatogram( binaryDataArray prime
                                        , binaryDataArray secondi
                                        , pugi::xml_node node) : mzMLDatumBase( node )
                                                               , impl_( std::make_unique< impl >( prime, secondi ) )
    {
    }

    size_t
    mzMLChromatogram::length() const
    {
        return std::min( impl_->prime_.length(), impl_->secondi_.length() ); // held in base class
    }

    std::pair< const binaryDataArray&, const binaryDataArray& >
    mzMLChromatogram::dataArrays() const
    {
        return { impl_->prime_, impl_->secondi_ };
    }

    boost::json::value
    mzMLChromatogram::to_value() const
    {
        return mzml::to_value{}( node() );
    }
}


namespace {
    // helper for visitor
    template<class... Ts>
    struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;
    // end helper for visitor
}

using namespace mzml;

std::shared_ptr< adcontrols::Chromatogram >
mzMLChromatogram::toChromatogram( const mzMLChromatogram& t )
{
    auto sp = std::make_shared< adcontrols::Chromatogram >();

    sp->resize( t.length() );
    sp->timeArray().resize( t.length() );

    auto [times,intensities] = t.dataArrays();

    std::visit([&](auto arg){
        std::transform(arg, arg + t.length(), sp->timeArray().begin(),
                       [](auto v) { return static_cast<double>(v); });
    }, times.data() );

    std::visit([&](auto arg){
        for ( size_t i = 0; i < t.length(); ++i )
            sp->setIntensity( i, *arg++ );
    }, intensities.data() );

    sp->setMinimumTime( sp->timeArray().front() );
    sp->setMaximumTime( sp->timeArray().back() );

    sp->addDescription( { "id", t.node().select_node( "@id" ).attribute().value() } );
    sp->addDescription( { "metadata", boost::json::serialize( mzml::to_value{}(t.node() ) ) } );
    sp->set_display_name( t.node().select_node( "@id" ).attribute().value() );

    return sp;
}
