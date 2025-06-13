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

#include <openssl/bio.h>
#include <openssl/evp.h>
#include "accession.hpp"
#include "xmlwalker.hpp"

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
            mzml::accession ac;
            for ( const auto param: node.select_nodes( "cvParam" ) )
                ac.assign( param.node().attribute("accession").value(), param.node().attribute("name").value() );

            std::string decoded;
            if ( auto binary = node.select_node( "binary" ) ) {
                std::string_view encoded = binary.node().child_value();
                if ( encoded.size() == length )
                    decoded  = base64_decode( encoded );
            }
            return { length, std::move( ac ), std::move( decoded ) };
        }
    };

    struct mzMLDatumBase {
        accession ac_;
        pugi::xml_node node_;
        mzMLDatumBase() {}
        mzMLDatumBase( const mzMLDatumBase& t ) : ac_(t.ac_), node_( t.node_ ) {}
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

        bool is_negative_scan() const {
            return node_.select_node( "cvParam[@accession='MS:1000129']" );
        }

        bool is_positive_scan() const {
            return node_.select_node( "cvParam[@accession='MS:1000130']" );
        }

        std::optional< int > ms_level() const {
            if ( auto ms_level = node_.select_node( "cvParam[@accession='MS:1000511']" ) )
                ms_level.attribute().as_int();
            return {};
        }

        std::optional< double > total_ion_current() const {
            if ( auto attr = node_.select_node( "cvParam[@accession='MS:1000285']" ) )
                return attr.attribute().as_double();
            return {};
        }

        std::optional< double > base_peak_mz() const {
            if ( auto attr = node_.select_node( "cvParam[@accession='MS:1000504']" ) )
                return attr.attribute().as_double();
            return {};
        }
        std::optional< double > base_peak_intensity() const {
            if ( auto attr = node_.select_node( "cvParam[@accession='MS:1000505']" ) )
                return attr.attribute().as_double();
            return {};
        }
        std::optional< double > scan_start_time() const {
            if ( auto scan = node_.select_node( "scanList/scan/cvParam[@accession='MS:1000016']" ) ) {
                return scan.node().attribute( "value" ).as_double();
            }
            return {};
        }

        std::optional< double > precursor() const {
            if ( auto param = node_.select_node( "precursorList/precursor/isolationWindow/cvParam[@accession='MS:1000827']" ) ) {
                return param.node().attribute( "value" ).as_double();
            }
            return {};
        }

        //--------------------->

        size_t selectedIonCount() const {
            return node_.select_node( "precursorList/precursor/selectedIonList/@count" ).attribute().as_uint();
        }

        std::optional< std::pair<double, double> > selectedIon() const {
            if ( auto intens = node_.select_node( "precursorList/precursor/selectedIonList/selectedIon/cvParam[@accession='MS:1000042']" ) ) {
                if ( auto mz = node_.select_node( "precursorList/precursor/selectedIonList/selectedIon/cvParam[@accession='MS:1000744']" ) ) {
                    return {{ mz.node().attribute( "value" ).as_double()
                                  , intens.node().attribute( "value" ).as_double()   }};
                }
            }
            return {};
        }

        std::optional< double > collision_energy() const {
            if ( auto ce = node_.select_node( "activation/cvParam[@accession='MS:1000045']" ) ) {
                return ce.node().attribute( "value" ).as_double();
            }
            return {};
        }

        bool is_increasing_mz_scan() const {
            return node_.select_node( "cvParam[@accession='MS:1000093]" );
        }
        bool is_linear() const {
            return node_.select_node( "cvParam[@accession='MS:1000095]" );
        }

        std::pair< double, double >
        basePeak() const {
            if ( auto intens = node_.select_node( "//cvParam[accession='MS:1000505']" ) ) {
                if ( auto mz = node_.select_node( "//cvParam[accession='MS:1000504']" ) ) {
                    return { mz.node().attribute( "value" ).as_double()
                             , intens.node().attribute( "value" ).as_double()   };
                }
            }
            return { 0, 0 };
        }
    };

    class mzMLSpectrum : public mzMLDatumBase {
        binaryDataArray mzArray_;
        binaryDataArray intensityArray_;
    public:
        mzMLSpectrum() {}
        mzMLSpectrum( binaryDataArray prime
                      , binaryDataArray secondi
                      , pugi::xml_node node ) : mzMLDatumBase( node )
                                              , mzArray_( prime )
                                              , intensityArray_( secondi ) {
        }
        size_t length() const { return mzArray_.size(); }

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
        ADDEBUG() << node.name() << " count=" << count << ", vec.size=" << vec.size();
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
            auto v = mzml::mzMLdatum< mzml::dataTypeSpectrum >{}( node1.node() );
            std::visit( mzml::overloaded{
                    [](auto arg) { std::cout << arg << ' '; }
                        , [&](std::shared_ptr< mzml::mzMLChromatogram > arg) {vec.emplace_back( arg ); }
                        }, v);
        }
        ADDEBUG() << node.name() << " count=" << count << ", vec.size=" << vec.size();
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
            ADDEBUG() << "fileDescription: " << fileDescription.toString();

            auto sourceFileListCount = node.select_node( "sourceFileList/@count" ).attribute().as_uint();
            for ( size_t i = 0; i < sourceFileListCount; ++i ) {
                for ( auto sourceFile : node.select_nodes( "sourceFileList/sourceFile" ) ) {
                    ADDEBUG() << mzml::accession(sourceFile.node()).toString();
                }
            }

            auto softwareListCount = node.select_node( "softwareList/@count" ).attribute().as_uint();
            for ( auto node1 : node.select_nodes( "softwareList/software" ) ) {
                ADDEBUG() << "software: " << node1.node().attribute("id").value() << ", " << node1.node().attribute("version").value();
                ADDEBUG() << mzml::accession(node1.node()).toString();
            }

            auto instrumentConfigurationListCount = node.select_node( "instrumentConfigurationList/@count" ).attribute().as_uint();
            for ( auto node1 : node.select_nodes( "instrumentConfigurationList/instrumentConfiguration" ) ) {
                ADDEBUG() << "instrumentConfiguration: " << node1.node().attribute( "id" ).value();
                ADDEBUG() << mzml::accession(node1.node()).toString();
            }

            auto componentListCount = node.select_node( "componentList/@count" ).attribute().as_uint();
            for ( auto node1 : node.select_nodes( "componentList/*" ) ) {
                ADDEBUG() << "component: " << node1.node().name() << ", order=" << node1.node().attribute( "order" ).value();
                ADDEBUG() << "\t" << mzml::accession( node1.node() ).toString();
            }

            auto dataProcessingListCount = node.select_node( "dataProcessingList/@count" ).attribute().as_uint();
            for ( auto node1 : node.select_nodes( "dataProcessingList/dataProcessing" ) ) {
                ADDEBUG() << "dataProcessing: " << node1.node().attribute("id").value();
                ADDEBUG() << "\t: " << node1.node().select_node( "processingMethod/@softwareRef").attribute().value();
                ADDEBUG() << "\t: " << mzml::accession( node1.node().select_node( "processingMethod").node() ).toString();
            }

            if (  auto run = node.select_node( "run" ) ) {
                ADDEBUG() << "run defaultInstrumentConfigurationRef=" << run.node().attribute( "defaultInstrumentConfigurationRef" ).value();
                ADDEBUG() << "\t=" << run.node().attribute( "defaultInstrumentConfigurationRef" ).value();
                ADDEBUG() << "\t=" << run.node().attribute( "defaultSourceFileRef" ).value();
                ADDEBUG() << "\t=" << run.node().attribute( "id" ).value();

                if ( auto spNode = run.node().select_node( "spectrumList"  ) ) {
                    auto vec = spectrumList{}( spNode.node() );
                    spectra_.insert(std::end(spectra_), std::begin(vec), std::end(vec));
                }
                if ( auto chroNode = run.node().select_node( "chromatogramList"  ) ) {
                    auto vec = chromatogramList{}( chroNode.node() );
                    chromatograms_.insert(std::end(chromatograms_), std::begin(vec), std::end(vec));
                }

                ADDEBUG() << "total " << std::make_pair( spectra_.size(), chromatograms_.size() ) << " spectra & chroamtograms";
                for ( const auto sp: spectra_ ) {
                    std::ostringstream o;
                    o << boost::format( "{}, {}, {}" ) << sp->id(), sp->index(), sp->length();
                    if ( auto value = sp->scan_start_time() )
                        o << std::format( ", scan start time: {}", *value );
                    if ( auto value = sp->precursor() )
                        o << std::format( ", precursor: {}", *value );
                    o << std::format( ", srmCount: {}", sp->selectedIonCount() );
                    if ( auto value = sp->selectedIon() )
                        o << std::format( ", selected ion: {},{}", value->first, value->second );
                    if ( auto value = sp->collision_energy() )
                        o << std::format( ", CE: {}", *value );

                    ADDEBUG() << o.str() << " BP:" << sp->basePeak();
                }
                for ( const auto cp: chromatograms_ ) {
                    ADDEBUG() << std::format( "{}, {}, {}", cp->id(), cp->index(), cp->length() ) << cp->ac().toString();
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
