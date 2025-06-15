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

#include "mzmlwalker.hpp"
#include "xmltojson.hpp"
#include "mzmlspectrum.hpp"
#include "mzmlchromatogram.hpp"
#include "mzmlreader.hpp"
#include <adportable/debug.hpp>
#include <variant>
#include <boost/json.hpp>
#include <QJsonDocument>

namespace {

    // helper for visitor
    template<class... Ts>
    struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;
    // end helper for visitor

    struct spectrumList {
        spectrumList() {}

        std::vector< std::shared_ptr< mzml::mzMLSpectrum > >
        operator()( const pugi::xml_node& node ) const {
            std::vector< std::shared_ptr< mzml::mzMLSpectrum > > vec;

            size_t count = node.attribute( "count" ).as_uint();

            for ( const auto node1: node.select_nodes( "spectrum" ) ) {
                auto v = mzml::mzMLReader< mzml::dataTypeSpectrum >{}( node1.node() );
                std::visit( overloaded{
                        [](auto arg) { }
                            , [&](std::shared_ptr< mzml::mzMLSpectrum > arg) {vec.emplace_back( arg ); }
                            }, v);
            }
            return vec;
        }
    };

    struct chromatogramList {
        chromatogramList() {}

        std::vector< std::shared_ptr< mzml::mzMLChromatogram > >
        operator()( const pugi::xml_node& node ) const {
            std::vector< std::shared_ptr< mzml::mzMLChromatogram > > vec;

            size_t count = node.attribute( "count" ).as_uint();

            for ( const auto node1: node.select_nodes( "chromatogram" ) ) {
                auto v = mzml::mzMLReader< mzml::dataTypeChromatogram >{}( node1.node() );
                std::visit( overloaded{
                        [](auto arg) { }
                            , [&](std::shared_ptr< mzml::mzMLChromatogram > arg) {vec.emplace_back( arg ); }
                            }, v);
            }
            return vec;
        }
    };

}

namespace mzml {
    class mzMLWalker::impl {
    public:
        std::vector< std::shared_ptr< mzml::mzMLChromatogram > > chromatograms_;
        std::vector< std::shared_ptr< mzml::mzMLSpectrum > > spectra_;
    };
}

using namespace mzml;

mzMLWalker::~mzMLWalker()
{
}

mzMLWalker::mzMLWalker() : impl_( std::make_unique< impl >() )
{
}

void
mzMLWalker::operator()( const pugi::xml_node& root_node )
{
    if ( auto mzML = root_node.select_node( "mzML" ) ) {
        auto node = mzML.node();

        ADDEBUG() << mzml::to_value{}( node.select_node( "fileDescription" ).node() );
        ADDEBUG() << mzml::to_value{}( node.select_node( "sourceFileList" ).node() );
        ADDEBUG() << mzml::to_value{}( node.select_node( "softwareList" ).node() );
        ADDEBUG() << mzml::to_value{}( node.select_node( "instrumentConfigurationList" ).node() );
        ADDEBUG() << mzml::to_value{}( node.select_node( "componentList" ).node() );
        ADDEBUG() << mzml::to_value{}( node.select_node( "dataProcessingList" ).node() );

        if (  auto run = node.select_node( "run" ) ) {
            ADDEBUG() << "run defaultInstrumentConfigurationRef=" << run.node().attribute( "defaultInstrumentConfigurationRef" ).value();
            ADDEBUG() << "\t=" << run.node().attribute( "defaultInstrumentConfigurationRef" ).value();
            ADDEBUG() << "\t=" << run.node().attribute( "defaultSourceFileRef" ).value();
            ADDEBUG() << "\t=" << run.node().attribute( "id" ).value();

            if ( auto node1 = run.node().select_node( "spectrumList"  ) ) {
                auto vec = spectrumList{}( node1.node() );
                impl_->spectra_.insert(std::end(impl_->spectra_), std::begin(vec), std::end(vec));
            }

            if ( auto node1 = run.node().select_node( "chromatogramList"  ) ) {
                auto vec = chromatogramList{}( node1.node() );
                impl_->chromatograms_.insert(std::end(impl_->chromatograms_), std::begin(vec), std::end(vec));
            }

            ADDEBUG() << "total " << std::make_pair( impl_->spectra_.size(), impl_->chromatograms_.size() ) << " spectra & chroamtograms";
            for ( const auto sp: impl_->spectra_ ) {
                if ( sp->length() > 0 ) {
                    ADDEBUG() << QJsonDocument::fromJson( boost::json::serialize( sp->to_value() ).c_str() )
                        .toJson( QJsonDocument::Compact ).toStdString();
                }
            }
            for ( const auto sp: impl_->spectra_ ) {
                if ( sp->length() == 0 )
                    ADDEBUG() << sp->to_value();
            }

            ADDEBUG() << "------------------------------ chromatograms ------------------------------";
            for ( const auto cp: impl_->chromatograms_ ) {
                ADDEBUG() << std::format( "chromatogram: id={}, indx={}, length={}, ", cp->id(), cp->index(), cp->length() ); // << cp->ac().toString();
            }
        }
        ADDEBUG() << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
    }
}
