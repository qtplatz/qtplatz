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
#include "accession.hpp"
#include "xmlwalker.hpp"
#include "xmltojson.hpp"
#include <boost/json.hpp>

namespace po = boost::program_options;

struct cvParamList {
    bool operator()( const std::vector< std::string >& files ) const {
        std::map< std::string, std::string > cvParam;
        for ( auto file: files ) {
            pugi::xml_document doc;
            if ( auto result = doc.load_file( file.c_str() ) ) {
                for (auto param : doc.select_nodes("//cvParam")) {
                    std::string name = param.node().attribute("name").value();
                    cvParam[ param.node().attribute("accession").value() ] = param.node().attribute("name").value();
                }
            }
        }

        std::cout << "\nnamespace mzml {" << std::endl
                  << "\tenum Accession {" << std::endl;

        for ( const auto& param: cvParam ) {
            std::string tag = param.first;
            std::replace( tag.begin(), tag.end(), ':', '_');
            if ( param == *cvParam.begin() )
                std::cout << std::format( "\t {}", tag) << std::endl;
            else
                std::cout << std::format( "\t, {}", tag) << std::endl;
        }
        std::cout << std::format( "\t, MS_MAX") << std::endl
                  << "\t};" << std::endl
                  << "} // namespace" << std::endl;

        std::cout << "\nnamespace mzml {" << std::endl
                  << "\tconst std::array< std::tuple< std::string, Accession, std::string >, MS_MAX > accession_list = {{" << std::endl;

        for ( const auto& param: cvParam ) {
            std::string tag = param.first;
            std::replace( tag.begin(), tag.end(), ':', '_');
            std::cout << "\t,{" << std::format( "\t\"{}\",\t{},\t\"{}\"\t", param.first, tag, param.second) << "}" << std::endl;
        }
        std::cout << "\t}};" << std::endl
                  << "} // namespace" << std::endl;
        return 0;
    }
};

std::string get_full_xpath(pugi::xml_node node) {
    std::string path;
    while (node && node.type() == pugi::node_element) {
        std::string name = node.name();
        // Optional: include index if parent has multiple with same name
        int index = 1;
        pugi::xml_node sibling = node.previous_sibling(name.c_str());
        while (sibling) {
            ++index;
            sibling = sibling.previous_sibling(name.c_str());
        }
        path = "/" + name + "[" + std::to_string(index) + "]" + path;
        node = node.parent();
    }
    return path.empty() ? "/" : path;
}


namespace mzml {

    // helper for visitor
    template<class... Ts>
    struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;
    // end helper for visitor

    typedef std::variant< const float *, const double * > data_ptr;

    class binaryDataArray {
        size_t encodedLength_;
        std::string decoded_;
        accession ac_;
    public:
        binaryDataArray( size_t length = 0
                         , mzml::accession&& ac = {}
                         , std::string&& decoded = {} ) : encodedLength_( length )
                                                             , ac_( ac )
                                                             , decoded_( decoded ) {
        }

        binaryDataArray( const binaryDataArray& t ) : encodedLength_( t.encodedLength_ )
                                                    , ac_( t.ac_ )
                                                    , decoded_( t.decoded_ ) {
        }

        operator bool () const { return not decoded_.empty(); }
        const accession& accession() const { return ac_; }
        size_t size() const { return decoded_.size(); }
        size_t length() const {
            if ( ac_.is_32bit() ) {
                return size() / sizeof(float);
            } else if ( ac_.is_64bit() ) {
                return size() / sizeof(double);
            }
            return size();
        }

        data_ptr data() const {
            if ( ac_.is_32bit() )
                return reinterpret_cast< const float * >( decoded_.data() );
            if ( ac_.is_64bit() )
                return reinterpret_cast< const double * >( decoded_.data() );
            return {};
        }

        static binaryDataArray make_instance( const pugi::xml_node& node ) {
            size_t length = node.attribute( "encodedLength" ).as_uint();
            mzml::accession ac(node);

            std::string decoded;
            if ( auto binary = node.select_node( "binary" ) ) {
                std::string_view encoded = binary.node().child_value();
                if ( encoded.size() == length )
                    decoded  = base64_decode( encoded );
            }
            return { length, std::move( ac ), std::move( decoded ) };
        }
    };

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


    struct mzMLDatumBase {
        accession ac_;
        pugi::xml_node node_;
        mzMLDatumBase() {}
        mzMLDatumBase( const mzMLDatumBase& t ) : ac_(t.ac_)
                                                , node_( t.node_ ) {}
        mzMLDatumBase( pugi::xml_node node ) : ac_( node )
                                             , node_( node ) {
        }
        std::string_view id() const {
            return node_.attribute( "id" ).value();
        }
        size_t index() const {
            return node_.attribute( "index" ).as_uint();
        }
        const mzml::accession& ac() const { return ac_; }

        bool is_negative_scan() const { return ac_.is_negative_scan(); }
        bool is_positive_scan() const { return ac_.is_positive_scan(); }
        std::optional< int > ms_level() const { return ac_.ms_level(); }
        std::optional< double > total_ion_current() const { return ac_.total_ion_current(); }
        std::optional< double > base_peak_mz() const { return ac_.base_peak_mz(); }
        std::optional< double > base_peak_intensity() const { return ac_.base_peak_intensity(); }

        std::pair< double, double >
        base_peak() const {
            if ( auto intens = node_.select_node( "cvParam[@accession='MS:1000505']" ) ) {
                if ( auto mz = node_.select_node( "cvParam[@accession='MS:1000504']" ) ) {
                    return { mz.node().attribute( "value" ).as_double()
                             , intens.node().attribute( "value" ).as_double()   };
                }
            }
            return { 0, 0 };
        }
    };

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

    class mzMLSpectrum : public mzMLDatumBase {
        binaryDataArray mzArray_;
        binaryDataArray intensityArray_;
        std::vector< mzml::scan > scanlist_;
        std::vector< mzml::precursor > precursorlist_;
        bool is_profile_spectrum_;
        size_t ms_level_;
        double base_peak_intensity_;
        double base_peak_mz_;
        double highest_observed_mz_;
        double lowest_observed_mz_;
    public:
        mzMLSpectrum() : is_profile_spectrum_( false )
                       , ms_level_(0)
                       , base_peak_intensity_(0)
                       , base_peak_mz_(0)
                       , highest_observed_mz_(0)
                       , lowest_observed_mz_(0) {
        }
        mzMLSpectrum( const mzMLSpectrum& t ) : mzArray_( t.mzArray_ )
                                              , intensityArray_( t.intensityArray_ )
                                              , scanlist_( t.scanlist_ )
                                              , precursorlist_( t.precursorlist_ )
                                              , is_profile_spectrum_( t.is_profile_spectrum_ )
                                              , ms_level_( t.ms_level_ )
                                              , base_peak_intensity_( t.base_peak_intensity_ )
                                              , base_peak_mz_( t.base_peak_mz_ )
                                              , highest_observed_mz_( t.highest_observed_mz_ )
                                              , lowest_observed_mz_( t.lowest_observed_mz_ ) {
        }
        mzMLSpectrum( binaryDataArray prime
                      , binaryDataArray secondi
                      , pugi::xml_node node ) : mzMLDatumBase( node )
                                              , mzArray_( prime )
                                              , intensityArray_( secondi ) {
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
        }
        size_t length() const { return mzArray_.size(); }

        // std::optional< double > scan_start_time() const { return scan_.scan_start_time(); }

        boost::json::value to_value() const {
            // ADDEBUG() << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";
            // ADDEBUG() << mzml::to_value{}( node_ );
            // ADDEBUG() << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
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
        }
    };

    class mzMLChromatogram : public mzMLDatumBase {
        binaryDataArray timeArray_;
        binaryDataArray intensityArray_;
        accession ac_;
        pugi::xml_node node_;
    public:
        mzMLChromatogram() {}
        mzMLChromatogram(   binaryDataArray prime
                          , binaryDataArray secondi
                            , pugi::xml_node node ) : mzMLDatumBase( node )
                                                    , timeArray_( prime )
                                                    , intensityArray_( secondi ) {
        }
        size_t length() const { return timeArray_.size(); }
        const pugi::xml_node& node() const { return node_; }

    };

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
            // for ( size_t i = 0; i < sourceFileListCount; ++i ) {
            //     for ( auto sourceFile : node.select_nodes( "sourceFileList/sourceFile" ) ) {
            //         ADDEBUG() << mzml::accession(sourceFile.node()).toString();
            //     }
            // }

            auto softwareListCount = node.select_node( "softwareList/@count" ).attribute().as_uint();
            ADDEBUG() << mzml::to_value{}( node.select_node( "softwareList" ).node() );
            // for ( auto node1 : node.select_nodes( "softwareList/software" ) ) {
            //     ADDEBUG() << "software: " << node1.node().attribute("id").value() << ", " << node1.node().attribute("version").value();
            //     ADDEBUG() << mzml::accession(node1.node()).toString();
            // }

            auto instrumentConfigurationListCount = node.select_node( "instrumentConfigurationList/@count" ).attribute().as_uint();
            ADDEBUG() << mzml::to_value{}( node.select_node( "instrumentConfigurationList" ).node() );
            // for ( auto node1 : node.select_nodes( "instrumentConfigurationList/instrumentConfiguration" ) ) {
            //     ADDEBUG() << "instrumentConfiguration: " << node1.node().attribute( "id" ).value();
            //     ADDEBUG() << mzml::accession(node1.node()).toString();
            // }

            auto componentListCount = node.select_node( "componentList/@count" ).attribute().as_uint();
            ADDEBUG() << mzml::to_value{}( node.select_node( "componentList" ).node() );
            // for ( auto node1 : node.select_nodes( "componentList/*" ) ) {
            //     ADDEBUG() << "component: " << node1.node().name() << ", order=" << node1.node().attribute( "order" ).value();
            //     ADDEBUG() << "\t" << mzml::accession( node1.node() ).toString();
            // }

            auto dataProcessingListCount = node.select_node( "dataProcessingList/@count" ).attribute().as_uint();
            ADDEBUG() << mzml::to_value{}( node.select_node( "dataProcessingList" ).node() );
            // for ( auto node1 : node.select_nodes( "dataProcessingList/dataProcessing" ) ) {
            //     ADDEBUG() << "dataProcessing: " << node1.node().attribute("id").value();
            //     ADDEBUG() << "\t: " << node1.node().select_node( "processingMethod/@softwareRef").attribute().value();
            //     ADDEBUG() << "\t: " << mzml::accession( node1.node().select_node( "processingMethod").node() ).toString();
            // }
            return;

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
                for ( const auto sp: spectra_ ) {
                    if ( sp->length() > 0 ) {
                        // ADDEBUG() << sp->to_value();
                        ADDEBUG() << QJsonDocument::fromJson( boost::json::serialize( sp->to_value() ).c_str() )
                            .toJson( QJsonDocument::Compact ).toStdString();
                    }
                }
#if 0
                for ( const auto sp: spectra_ ) {
                    if ( sp->length() == 0 )
                        ADDEBUG() << sp->to_value();
                }
                ADDEBUG() << "------------------------------ chromatograms ------------------------------";
                for ( const auto cp: chromatograms_ ) {
                    ADDEBUG() << std::format( "chromatogram: id={}, indx={}, length={}, ", cp->id(), cp->index(), cp->length() ) << cp->ac().toString();
                }
#endif
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
