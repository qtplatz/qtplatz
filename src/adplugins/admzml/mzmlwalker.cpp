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
#include "accession.hpp"
#include "xmltojson.hpp"
#include "mzmlreader.hpp"
#include <adportable/debug.hpp>
#include <variant>
#include <pugixml.hpp>
#include <boost/json.hpp>
#include <QJsonDocument>

namespace {

    // helper for visitor
    template<class... Ts>
    struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;
    // end helper for visitor

    ///////////////////////////////////////////////////////////

    struct spectrumList {
        spectrumList() {}

        std::vector< std::shared_ptr< mzml::mzMLSpectrum > >
        operator()( const pugi::xml_node& node ) const {
            std::vector< std::shared_ptr< mzml::mzMLSpectrum > > vec;

            size_t count = node.attribute( "count" ).as_uint();

            for ( const auto node1: node.select_nodes( "spectrum" ) ) {
                // auto v = mzml::mzMLReader_< mzml::dataTypeSpectrum >{}( node1.node() );
                auto v = mzml::mzMLReader{}( node1.node() );
                std::visit( overloaded{
                        [](auto arg) { }
                            , [&](std::shared_ptr< mzml::mzMLSpectrum > arg) {vec.emplace_back( arg ); }
                            }, v);
            }
            return vec;
        }
    };

    ///////////////////////////////////////////////////////////

    struct chromatogramList {
        chromatogramList() {}

        std::vector< std::shared_ptr< mzml::mzMLChromatogram > >
        operator()( const pugi::xml_node& node ) const {
            std::vector< std::shared_ptr< mzml::mzMLChromatogram > > vec;

            size_t count = node.attribute( "count" ).as_uint();

            for ( const auto node1: node.select_nodes( "chromatogram" ) ) {
                //auto v = mzml::mzMLReader_< mzml::dataTypeChromatogram >{}( node1.node() );
                auto v = mzml::mzMLReader{}( node1.node() );
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

    node_only::node_only( const pugi::xml_node& node ) : node_( node )
    {
    }
    const pugi::xml_node& node_only::node() const
    {
        return node_;
    }
    std::string
    node_only::toString() const
    {
        return accession( node_ ).toString();
    }

    //-------
    fileDescription::fileDescription( const pugi::xml_node& node )
        : node_only( node.select_node( "fileDescription" ).node() )
    {
    }
    fileDescription& fileDescription::operator=( const fileDescription& t )
    {
        node_ = t.node_;
        return *this;
    }

    softwareList::softwareList( const pugi::xml_node& node )
        : node_only( node.select_node( "softwareList" ).node() )
    {
    }
    softwareList& softwareList::operator=( const softwareList& t )
    {
        node_ = t.node_;
        return *this;
    }

    instrumentConfigurationList::instrumentConfigurationList( const pugi::xml_node& node )
        : node_only( node.select_node( "instrumentConfigurationList" ).node() )
    {
    }
    instrumentConfigurationList& instrumentConfigurationList::operator=( const instrumentConfigurationList& t )
    {
        node_ = t.node_;
        return *this;
    }

    dataProcessingList::dataProcessingList( const pugi::xml_node& node )
        : node_only( node.select_node( "dataProcessingList" ).node() )
    {
    }
    dataProcessingList& dataProcessingList::operator = ( const dataProcessingList& t )
    {
        node_ = t.node_;
        return *this;
    }

    boost::json::array collect_names(const char* xpath, const pugi::xml_node& root) {
        boost::json::array result;
        for (auto n : root.select_nodes(xpath))
            result.emplace_back(n.node().attribute("name").value());
        return result;
    };

    void
    tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const fileDescription& t )
    {
        jv = {
            { "fileContent", collect_names(".//fileContent/cvParam", t.node()) },
            { "sourceFile", {
                    { "id",     t.node().select_node(".//sourceFile/@id").attribute().value() },
                    { "name",   t.node().select_node(".//sourceFile/@name").attribute().value() },
                    { "format", t.node().select_node(".//sourceFile/userParam/@value").attribute().value() },
                    { "cvParam", collect_names(".//sourceFile/cvParam", t.node()) }
                }}
        };
    }

    // <softwareList count="1">
    // 	<software id="LabSolutions" version="5.120">
    // 		<cvParam accession="MS:1001557" cvRef="MS" name="Shimadzu Corporation software" />
    // 	</software>
    // </softwareList>
    void
    tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const softwareList& t )
    {
        jv = {
            { "id",     t.node().select_node(".//software/@id").attribute().value() }
            , { "version",     t.node().select_node(".//software/@version").attribute().value() }
            , { "cvparam", collect_names( ".//software/cvParam", t.node()) }
            };
    }

    	// <instrumentConfiguration id="LCMS-8060">
		// <cvParam accession="MS:1000124" cvRef="MS" name="Shimadzu instrument model" />
		// <cvParam accession="MS:1000529" cvRef="MS" name="instrument serial number" value="O11105700580AE" />
		// <componentList count="3">
		// 	<source order="1">
		// 		<cvParam accession="MS:1000073" cvRef="MS" name="electrospray ionization" />
		// 	</source>
		// 	<analyzer order="2">
		// 		<cvParam accession="MS:1000081" cvRef="MS" name="quadrupole" />
		// 	</analyzer>
		// 	<detector order="3">
		// 		<cvParam accession="MS:1000026" cvRef="MS" name="detector type" />
		// 	</detector>
		// </componentList>
	    // </instrumentConfiguration>

    void
    tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const instrumentConfigurationList& t )
    {
        jv = {
            { "id",     t.node().select_node(".//instrumentConfiguration/@id").attribute().value() }
            , { "serialnumber",
                t.node().select_node( ".//instrumentConfiguration/cvParam[@accession='MS:1000529']").node().attribute("value").value() }
            , { "source", collect_names( ".//instrumentConfiguration/componentList/source/cvParam", t.node()) }
            , { "analyzer", collect_names( ".//instrumentConfiguration/componentList/analyzer/cvParam", t.node()) }
            , { "detector", collect_names( ".//instrumentConfiguration/componentList/detector/cvParam", t.node()) }
        };
    }

}

using namespace mzml;

mzMLWalker::~mzMLWalker()
{
}

mzMLWalker::mzMLWalker()
{
}

std::vector< data_t >
mzMLWalker::operator()( const pugi::xml_node& root_node )
{
    std::vector< data_t > var;

    if ( auto mzML = root_node.select_node( "mzML" ) ) {
        auto node = mzML.node();

        var.emplace_back( mzml::fileDescription( node ) );
        var.emplace_back( mzml::softwareList( node ) );
        var.emplace_back( mzml::instrumentConfigurationList( node ) );
        var.emplace_back( mzml::dataProcessingList( node ) );

        mzml::chromatograms_t chromatograms;

        if (  auto run = node.select_node( "run" ) ) {
            // ADDEBUG() << "run defaultInstrumentConfigurationRef=" << run.node().attribute( "defaultInstrumentConfigurationRef" ).value();
            // ADDEBUG() << "\t=" << run.node().attribute( "defaultInstrumentConfigurationRef" ).value();
            // ADDEBUG() << "\t=" << run.node().attribute( "defaultSourceFileRef" ).value();
            // ADDEBUG() << "\t=" << run.node().attribute( "id" ).value();

            if ( auto node1 = run.node().select_node( "spectrumList"  ) ) {
                var.emplace_back( spectrumList{}( node1.node() ) );
            }

            if ( auto node1 = run.node().select_node( "chromatogramList"  ) ) {
                var.emplace_back( chromatogramList{}( node1.node() ) );
            }
        }
    }
    return var;
}
