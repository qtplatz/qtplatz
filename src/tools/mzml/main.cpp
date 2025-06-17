/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <adplugins/admzml/mzmlspectrum.hpp>
#include <adplugins/admzml/mzmlchromatogram.hpp>
#include <adplugins/admzml/xmltojson.hpp>
#include "accession.hpp"
#include "cvparamlist.hpp"

#include <pugixml.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/folder.hpp>
#include <adfs/fs.hpp>
#include <adfs/sqlite.hpp>
#include <adplugin/plugin.hpp>
#include <adportable/base64.hpp>
#include <adportable/debug.hpp>
#include <adportable/utf.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/portfolio.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <algorithm>
#include <iostream>
#include <memory>
#include <numeric>
#include <sstream>
#include <type_traits>
#include <variant>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>

#include <openssl/bio.h>
#include <openssl/evp.h>
// #include "accession.hpp"
#include "xmlwalker.hpp"

#include <boost/json.hpp>

namespace po = boost::program_options;

namespace mzml {

    // helper for visitor
    template<class... Ts>
    struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;
    // end helper for visitor

    // typedef std::variant< const float *, const double * > data_ptr;
    struct isolationWindow {
        double target_mz_;
        double lower_offset_;
        double upper_offset_;
        isolationWindow() : target_mz_(0), lower_offset_(0), upper_offset_(0) {};
    };

    void
    tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const isolationWindow& t )
    {
        jv = boost::json::value{ {  "target_mz", t.target_mz_ }
                                 , { "lower_offset", t.lower_offset_ }
                                 , { "upper_offset", t.upper_offset_ } };
    }

    struct selectedIon {
        double mz;
        double intensity;
        selectedIon( double _m = 0, double _i = 0) : mz( _m ), intensity( _i ) {}
    };

    void
    tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const selectedIon& t )
    {
        jv = boost::json::value{ { "mz", t.mz }, { "intensity", t.intensity } };
    }

    // inside of precursor
    struct activation {
        double collision_energy;
        std::string accession;
        std::string name;
        activation( double _1 = 0, std::string _2 = {}, std::string _3 = {} ) : collision_energy( _1 )
                                                                              , accession( _2 )
                                                                              , name( _3 ) {}
    };

    void
    tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const activation& t )
    {
        jv = boost::json::value{ { "collision_energy", t.collision_energy }
                                 , { "accession", t.accession }
                                 , { "name", t.name } };
    }

    class precursor {
        pugi::xml_node node_;
    public:
        operator bool () const { return node_ && node_.name() == std::string("precursor"); }
        pugi::xml_node node() const { return node_; };
        precursor() {}
        precursor( pugi::xml_node node ) : node_( node ) {}

        isolationWindow isolationWindow() const {
            struct isolationWindow w;
            if ( auto node = node_.select_node( "isolationWindow" ) ) {
                if ( auto node1 = node.node().select_node( "cvParam[@accession='MS:1000827']" ) ) {
                    w.target_mz_ = node1.node().attribute( "value" ).as_double();
                }
                if ( auto node1 = node.node().select_node( "cvParam[@accession='MS:1000828']" ) ) {
                    w.lower_offset_ = node1.node().attribute( "value" ).as_double();
                }
                if ( auto node1 = node.node().select_node( "cvParam[@accession='MS:1000829']" ) ) {
                    w.upper_offset_ = node1.node().attribute( "value" ).as_double();
                }
            }
            return w;
        }

        size_t selectedIonListCount() const {
            if ( auto node = node_.select_node( "selectedIonList/@count" ) )
                return node.attribute().as_uint();
            return 0;
        }

        std::vector< selectedIon >
        selectedIon() const {
            std::vector< mzml::selectedIon > v;
            for ( auto node: node_.select_nodes( "selectedIonList/selectedIon" ) ) {
                // node.node().print( std::cout );
                if ( auto node1 = node.node().select_node( "cvParam[@accession='MS:1000744']" ) ) { // m/z
                    if ( auto node2 = node.node().select_node( "cvParam[@accession='MS:1000042']" ) ) { // intensity
                        v.emplace_back( node1.node().attribute( "value" ).as_double()
                                        , node2.node().attribute( "value" ).as_double() );
                    }
                }
            }
            if ( not v.empty() )
                return v;
            return {};
        }

        mzml::activation activation() const {
            mzml::activation a;
            if ( auto node = node_.select_node( "activation/cvParam[@accession='MS:1000045']" ) )  {
                a.collision_energy = node.node().attribute( "value" ).as_double();
            }
            if ( auto node = node_.select_node( "activation/cvParam[@accession!='MS:1000045']" ) )  {
                a.name = node.node().attribute( "name" ).value();
                a.accession = node.node().attribute( "accession" ).value();
            }
            return a;
        }
    };

    void
    tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const precursor& t )
    {
        // jv = mzml::to_value{}( t.node() );
        jv = boost::json::value{
            { "isolationWindow", boost::json::value_from ( t.isolationWindow() ) }
            , { "selectedIon", boost::json::value_from ( t.selectedIon() ) }
            , { "activation", boost::json::value_from( t.activation() ) } };
    }

    struct scanWindow {
        double lower_limit_;
        double upper_limit_;
        scanWindow( double _1 = 0, double _2 = 0 ) : lower_limit_( _1 ), upper_limit_( _2 ) {}
    };

    class scan {
    public:
        double scan_start_time_;
        std::vector< mzml::scanWindow > scan_window_;
        scan( const scan& t ) : scan_start_time_( t.scan_start_time_ )
                              , scan_window_( t.scan_window_ ) {
        }

        scan( pugi::xml_node scan ) {
            if ( auto node = scan.select_node( "cvParam[@accession='MS:1000016']" ) ) {
                scan_start_time_ = node.node().attribute( "value" ).as_double();
                for ( auto window: scan.select_nodes( "scanWindowList/scanWindow" ) ) {
                    if ( auto ulimit = window.node().select_node( "cvParam[@accession='MS:1000500']" ) ) {
                        if ( auto llimit = window.node().select_node( "cvParam[@accession='MS:1000501']" ) ) {
                            scan_window_.emplace_back( llimit.node().attribute( "value" ).as_double()
                                                       , ulimit.node().attribute( "value" ).as_double() );
                        }
                    }
                    // ADDEBUG() << "#### scan: " << boost::json::value_from( *this );
                }
            }

        }
    };

    void
    tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const mzml::scanWindow& t )
    {
        jv = boost::json::value{{ "lower_limit", t.lower_limit_ }, { "upper_limit", t.upper_limit_ } };
    }

    void
    tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const mzml::scan& t )
    {
        jv = boost::json::value{{ "scan_start_time", t.scan_start_time_ },
                                { "scanWindow", boost::json::value_from( t.scan_window_ ) }};
    }

    enum dataType { dataTypeSpectrum, dataTypeChromatogram };

    using datum_variant_t = std::variant< std::shared_ptr< mzMLSpectrum >
                                          , std::shared_ptr< mzMLChromatogram > >;


    template< dataType T >
    struct mzMLdatum {
        using mzMLDatumType = std::conditional< T == dataTypeSpectrum, mzMLSpectrum, mzMLChromatogram >::type;

        mzMLdatum() {}

        std::pair< const pugi::xml_node, const pugi::xml_node > getArrayNodes( const pugi::xml_node ) const;

        datum_variant_t
        operator()( const pugi::xml_node& node ) const {
            // ADDEBUG() << std::format( "length={}, id={}, index={}", defaultArrayLength, id, index );
            size_t count = node.select_node( "binaryDataArrayList/@count" ).attribute().as_uint();
            if ( count == 2 ) {
                auto nodes = getArrayNodes( node );

                auto prime = binaryDataArray::make_instance( std::get<0>( nodes ) );
                auto secondi = binaryDataArray::make_instance( std::get<1>( nodes ) );

                if ( prime.length() == secondi.length()) {
                    return std::make_shared< mzMLDatumType >( prime, secondi, node );
                }
            }
            return {};
        }
    };

    template<>
    std::pair< const pugi::xml_node, const pugi::xml_node >
    mzMLdatum< dataTypeSpectrum >::getArrayNodes( const pugi::xml_node node ) const {
        if (auto intens= node.select_node("binaryDataArrayList/binaryDataArray[cvParam[@accession='MS:1000515']]")) {
            if (auto mz= node.select_node("binaryDataArrayList/binaryDataArray[cvParam[@accession='MS:1000514']]")) {
                return { mz.node(), intens.node() };
            }
        }
        return {};
    }

    template<>
    std::pair< const pugi::xml_node, const pugi::xml_node >
    mzMLdatum< dataTypeChromatogram >::getArrayNodes( const pugi::xml_node node ) const {

        if (auto intens= node.select_node("binaryDataArrayList/binaryDataArray[cvParam[@accession='MS:1000515']]")) {
            if (auto time = node.select_node("binaryDataArrayList/binaryDataArray[cvParam[@accession='MS:1000595']]")) {
                return { time.node(), intens.node()};
            }
        }
        return {};
    }
}

struct spectrumList {
    spectrumList() {}

    std::vector< std::shared_ptr< mzml::mzMLSpectrum > >
    operator()( const pugi::xml_node& node ) const {
        std::vector< std::shared_ptr< mzml::mzMLSpectrum > > vec;
        size_t count = node.attribute( "count" ).as_uint();

        for ( const auto node1: node.select_nodes( "spectrum" ) ) {
            auto v = mzml::mzMLdatum< mzml::dataTypeSpectrum >{}( node1.node() );
            std::visit( mzml::overloaded{
                    [](auto arg) { std::cout << arg << ' '; }
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
            auto v = mzml::mzMLdatum< mzml::dataTypeChromatogram >{}( node1.node() );
            std::visit( mzml::overloaded{
                    [](auto arg) { std::cout << arg << ' '; }
                        , [&](std::shared_ptr< mzml::mzMLChromatogram > arg) {vec.emplace_back( arg ); }
                        }, v);
        }
        return vec;
    }
};

struct mzMLWalker {
    std::vector< std::shared_ptr< mzml::mzMLChromatogram > > chromatograms_;
    std::vector< std::shared_ptr< mzml::mzMLSpectrum > > spectra_;

    mzMLWalker() {}

    void operator()( const pugi::xml_node& indexedmzMLNode ) {
        ADDEBUG() << "=========================================>";
        if ( auto mzML = indexedmzMLNode.select_node( "mzML" ) ) {
            auto node = mzML.node();

            auto cvList = node.select_node( "cvList" );
            mzml::accession fileDescription( node.select_node( "fileDescription/fileContent" ).node() );
            ADDEBUG() << "============ fileDescription ===============>";
            ADDEBUG() << mzml::to_value{}( node.select_node( "fileDescription" ).node() );

            auto sourceFileListCount = node.select_node( "sourceFileList/@count" ).attribute().as_uint();
            ADDEBUG() << mzml::to_value{}( node.select_node( "sourceFileList" ).node() );

            auto softwareListCount = node.select_node( "softwareList/@count" ).attribute().as_uint();
            ADDEBUG() << mzml::to_value{}( node.select_node( "softwareList" ).node() );

            auto instrumentConfigurationListCount = node.select_node( "instrumentConfigurationList/@count" ).attribute().as_uint();
            ADDEBUG() << mzml::to_value{}( node.select_node( "instrumentConfigurationList" ).node() );

            auto componentListCount = node.select_node( "componentList/@count" ).attribute().as_uint();
            ADDEBUG() << mzml::to_value{}( node.select_node( "componentList" ).node() );

            auto dataProcessingListCount = node.select_node( "dataProcessingList/@count" ).attribute().as_uint();
            ADDEBUG() << mzml::to_value{}( node.select_node( "dataProcessingList" ).node() );

            if (  auto run = node.select_node( "run" ) ) {
                ADDEBUG() << "run defaultInstrumentConfigurationRef=" << run.node().attribute( "defaultInstrumentConfigurationRef" ).value();
                ADDEBUG() << "\t=" << run.node().attribute( "defaultInstrumentConfigurationRef" ).value();
                ADDEBUG() << "\t=" << run.node().attribute( "defaultSourceFileRef" ).value();
                ADDEBUG() << "\t=" << run.node().attribute( "id" ).value();

                if ( auto node1 = run.node().select_node( "spectrumList"  ) ) {
                    auto vec = spectrumList{}( node1.node() );
                    spectra_.insert(std::end(spectra_), std::begin(vec), std::end(vec));
                }

                if ( auto node1 = run.node().select_node( "chromatogramList"  ) ) {
                    auto vec = chromatogramList{}( node1.node() );
                    chromatograms_.insert(std::end(chromatograms_), std::begin(vec), std::end(vec));
                }

                ADDEBUG() << "total " << std::make_pair( spectra_.size(), chromatograms_.size() ) << " spectra & chroamtograms";

                // make_scan_indices
                for ( const auto sp: spectra_ ) {
                    std::string id;
                    size_t index{0};
                    mzml::accession ac( sp->node() );

                    if ( auto scan = sp->node().select_node( "./scanList[1]/scan" ) ) {
                        id = sp->node().attribute( "id" ).value();
                        index = sp->node().attribute( "id" ).as_uint();
                        double scan_start_time = scan.node().select_node( "cvParam[@accession='MS:1000016']" ).node().attribute("value").as_double();
                        double selected_ion_mz{0}, ce{0};
                        if ( auto node =
                             sp->node().select_node( "./precursorList[1]/precursor/selectedIonList/selectedIon/cvParam[@accession='MS:1000744']" ) ) {
                            selected_ion_mz = node.node().attribute( "value" ).as_double();
                        }
                        if ( auto node =
                             sp->node().select_node( "./precursorList[1]/precursor/activation/cvParam[@accession='MS:1000045']" ) ) {
                            ce = node.node().attribute( "value" ).as_double();
                        }
                        ADDEBUG() << std::make_tuple( id, index, sp->length(), scan_start_time, selected_ion_mz, ce, ac.to_string( ac.ion_polarity() ) )
                                  << ", ms_level=" << *ac.ms_level();
                    }
                    // ADDEBUG() << mzml::to_value{}( sp->node().select_node(".//scan").node() );
                }
            }
            ADDEBUG() << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
        }
    }
};


int
main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    po::variables_map vm;
    po::options_description description( "adimport" );
    {
        description.add_options()
            ( "help,h",      "Display this help message" )
            ( "args",            po::value< std::vector< std::string > >(),  "input files" )
            ( "output,o",        po::value< std::string >()->default_value( "output.adfs" ), "import from text file to adfs" )
            ( "endian",     "Host endian check" )
            ( "cvparam",     "List all cvParam" )
            ;
        po::positional_options_description p;
        p.add( "args",  -1 );
        po::store( po::command_line_parser( argc, argv ).options( description ).positional(p).run(), vm );
        po::notify(vm);
    }

    if ( vm.count( "endian" ) ) {
        uint32_t u32 = 0xbeefdead;
        uint8_t * u8 = reinterpret_cast< uint8_t *>(&u32);
        ADDEBUG() << std::format( "{:08x}\t{:02x},{:02x},{:02x},{:02x}", u32, u8[0],u8[1],u8[2], u8[3] );
        ADDEBUG() << (( u8[0] == 0xad && u8[3] == 0xbe ) ? "host is little endian" : "host is big endian");
        return 0;
    }

    if ( vm.count( "help" ) || not vm.count( "args") ) {
        std::cout << description;
        return 0;
    }

    if ( vm.count( "cvparam" ) ) {
        return cvParamList{}(  vm[ "args" ].as< std::vector< std::string > >() );
    }

    for ( auto file: vm[ "args" ].as< std::vector< std::string > >() ) {
        pugi::xml_document doc;
        if ( auto result = doc.load_file( file.c_str() ) ) {

            if ( auto node = doc.select_node( "/indexedmzML" ) )
                mzMLWalker{}( node.node() );
        }
    }

    return 0;
}
