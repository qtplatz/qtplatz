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

#include "mzmlspectrum.hpp"
#include "binarydataarray.hpp"
#include "mzmldatumbase.hpp"
#include "xmltojson.hpp"
#include <boost/json.hpp>
#include <pugixml.hpp>

namespace mzml {

    class mzMLSpectrum::impl {
    public:
        impl() {}
        impl( binaryDataArray prime
              , binaryDataArray secondi ) : prime_( prime )
                                          , secondi_( secondi ) {
        }
        binaryDataArray prime_; // mz array
        binaryDataArray secondi_; // intensity array
        pugi::xml_node node_;

        bool is_profile_spectrum_;
        size_t ms_level_;
        double base_peak_intensity_;
        double base_peak_mz_;
        double highest_observed_mz_;
        double lowest_observed_mz_;

    };

    mzMLSpectrum::~mzMLSpectrum()
    {
    }

    mzMLSpectrum::mzMLSpectrum() : impl_( std::make_unique< impl >() )
    {
    }

    mzMLSpectrum::mzMLSpectrum( const mzMLSpectrum& t ) : impl_( std::make_unique< impl >( *t.impl_ ) )
    {
    }


    mzMLSpectrum::mzMLSpectrum( binaryDataArray prime
                                , binaryDataArray secondi
                                , pugi::xml_node node ) : mzMLDatumBase( node )
                                                        , impl_( std::make_unique< impl >( prime, secondi ) )
    {
#if 0
        for ( auto node: node_.select_nodes("scanList/scan") ) {
            scanlist_.emplace_back( node.node() );
        }
        for ( auto node: node_.select_nodes( "precursorList/precursor" ) ) {
            precursorlist_.emplace_back( node.node() );
        }
        {
            auto jv = mzml::to_value{}( node_.select_node( "scanList" ).node() );
            ADDEBUG() << QJsonDocument::fromJson( boost::json::serialize( jv ).c_str() )
                .toJson( QJsonDocument::Indented ).toStdString();
        }
        {
            auto jv = mzml::to_value{}( node_.select_node( "precursorList" ).node() );
            ADDEBUG() << QJsonDocument::fromJson( boost::json::serialize( jv ).c_str() )
                .toJson( QJsonDocument::Indented ).toStdString();
        }
#endif
    }

    size_t
    mzMLSpectrum::length() const
    {
        return impl_->prime_.size();
    }

    // std::optional< double > scan_start_time() const { return scan_.scan_start_time(); }

    boost::json::value
    mzMLSpectrum::to_value() const {
        // return mzml::to_value{}( node() );
        // ADDEBUG() << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";
        // ADDEBUG() << mzml::to_value{}( node_ );
        // ADDEBUG() << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
#if 0
        return boost::json::value{
            { "id", id() }
            , { "index", index() }
            , { "length", length() }
            , { "scan", boost::json::value_from( scanlist_ ) }
            , { "precursor", boost::json::value_from( precursorlist_ ) }
            , { "base_peak", { "mz", boost::json::value_from( ac().base_peak_mz() ) }
                , { "intensity", boost::json::value_from( ac().base_peak_intensity()) } }
            , { "is_positive_scan", boost::json::value_from( ac().is_positive_scan() ) }
        };
#endif
        return {};
    }
}
