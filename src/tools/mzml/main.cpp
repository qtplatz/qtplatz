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
#include <fstream>
#include <iomanip>
#include <iostream>
#include <ratio>
#include <numeric>
#include <QApplication>
#include <QMessageBox>
#include <QString>
#include <QStringList>

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

struct cvList {
    std::string tab_;
    cvList( const std::string& tab ) : tab_( tab ) {}
    void operator()( const pugi::xml_node& node ) const {
        std::cout << tab_ << node.name() << std::endl;
    }
};

struct sourceFileList {
    std::string tab_;
    sourceFileList( const std::string& tab ) : tab_( tab ) {}
    void operator()( const pugi::xml_node& node ) const {
        for ( auto file: node.select_nodes( "sourceFile" ) ) {
            std::string id = file.node().attribute("id").value();
            std::string location = file.node().attribute("location").value();
            std::string name = file.node().attribute("name").value();
            mzml::accession ac;
            for ( auto param: file.node().select_nodes( "cvParam" ) ) {
                ac.assign( param.node().attribute("accession").value(), param.node().attribute("name").value() );
            }
            std::cout << tab_ << node.name()
                      << std::format( "\t{}: \'{}/{}\' {}", id, location, name, ac.toString() )
                      << std::endl;
            for ( const auto node1: node.select_nodes(
                      "*[not(self::cvParam)]"
                      "[not(self::offset)]"
                      "[not(self::fileContent)]"
                      "[not(self::sourceFile)]" ) ) {
                ADDEBUG() << tab_ << node1.node().name();
            }
        }
    }
};

struct fileDescription {
    std::string tab_;
    fileDescription( const std::string&  tab ) : tab_( tab ) {}
    void operator()( const pugi::xml_node& node ) const {
        mzml::accession ac;
        if ( auto content = node.select_node( "fileContent" ) ) {
            for (auto param : content.node().select_nodes("./cvParam")) {
                ac.assign( param.node().attribute("accession").value(), param.node().attribute("name").value() );
            }
        }
        std::cout << tab_ << node.name() << "\tContent: [" << ac.toString() << "]" << std::endl;
        for ( const auto node1: node.select_nodes( "*[not(self::cvParam)][not(self::offset)][not(self::fileContent)]" ) ) {
            if ( node1.node().name() == std::string( "sourceFileList" ) ) {
                sourceFileList{ tab_ + "\t" }( node1.node() );
            }
        }
    }
};

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
        accession ac_;
        std::string decoded_;
    public:
        binaryDataArray( size_t length = 0
                         , const mzml::accession& ac = {}
                         , const std::string& decoded = {} ) : encodedLength_( length )
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
                std::string encoded = binary.node().child_value();
                if ( encoded.size() == length )
                    decoded  = base64_decode( encoded );
            }
            return { length, ac, decoded };
        }
    };

    enum dataType { dataTypeSpectrum, dataTypeChromatogram };

    template< dataType T >
    struct mzMLdatum {
        std::string tab_;
        mzMLdatum( const std::string&  tab ) : tab_( tab ) {}

        std::pair< const pugi::xml_node, const pugi::xml_node > getArrayNodes( const pugi::xml_node ) const;

        void operator()( const pugi::xml_node& node ) const {
            auto defaultArrayLength = node.attribute("defaultArrayLength").as_uint();
            auto id = node.attribute( "id" ).value();
            auto index = node.attribute( "index" ).value();
            mzml::accession ac;
            for ( const auto param: node.select_nodes( "cvParam" ) )
                ac.assign( param.node().attribute("accession").value(), param.node().attribute("name").value() );

            ADDEBUG() << std::format( "length={}, id={}, index={}", defaultArrayLength, id, index );
            size_t count{0};
            if ( auto vecCount = node.select_node( "binaryDataArrayList/@count" ) ) {
                count = vecCount.attribute().as_uint();
            }
            std::pair< binaryDataArray, binaryDataArray > sp;
            if ( count == 2 ) {
                auto nodes = getArrayNodes( node );
                sp = { binaryDataArray::make_instance( std::get<0>( nodes ) )
                       , binaryDataArray::make_instance( std::get<1>( nodes ) ) };
            }

            if ( std::get<0>(sp).accession().is_32bit() ) {
                if ( std::get<1>(sp).accession().is_32bit() ) {
                    auto ptr1 = std::get< const float *>(std::get<0>(sp).data());
                    auto ptr2 = std::get< const float *>(std::get<1>(sp).data());
                    for ( size_t i = 0; i < std::get<0>(sp).length(); ++i ) {
                        ADDEBUG() << std::make_pair( *ptr1++, *ptr2++ );
                    }
                }
            }

            for ( const auto node1: node.select_nodes( "*[not(self::cvParam)]" ) ) {
                if ( node1.node().name() == std::string( "scanList" ) ) {
                    size_t count = node1.node().attribute( "count" ).as_uint();
                    // ADDEBUG() << "\t" << node1.node().name() << "\tcount=" << count;
                } else if ( node1.node().name() == std::string( "precursorList" ) ) {
                    size_t count = node1.node().attribute( "count" ).as_uint();
                    // ADDEBUG() << "\t" << node1.node().name() << "\tcount=" << count;
                } else if ( node1.node().name() == std::string( "productList" ) ) {
                    size_t count = node1.node().attribute( "count" ).as_uint();
                    // ADDEBUG() << "\t" << node1.node().name() << "\tcount=" << count;
                }
            }
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
    std::string tab_;
    spectrumList( const std::string&  tab ) : tab_( tab ) {}
    void operator()( const pugi::xml_node& node ) const {
        size_t count = node.attribute( "count" ).as_uint();
        ADDEBUG() << node.name() << " count=" << count;

        for ( const auto node1: node.select_nodes( "spectrum" ) ) {
            mzml::mzMLdatum< mzml::dataTypeSpectrum >{ tab_ + "\t" }( node1.node() );
        }
    }
};

struct chromatogramList {
    std::string tab_;
    chromatogramList( const std::string&  tab ) : tab_( tab ) {}
    void operator()( const pugi::xml_node& node ) const {
        size_t count = node.attribute( "count" ).as_uint();
        // ADDEBUG() << node.name() << " count=" << count;

        for ( const auto node1: node.select_nodes( "*[not(self::cvParam)]"
                                                   "[not(self::offset)]"  ) ) {
            if ( node1.node().name() == std::string( "chromatogram" ) ) {
                handle_chromatogram( node1.node() );
            } else {
                mzml::xmlWalker{ tab_ + "<cL>\t" }( node1.node() );
            }
        }
    }
    void handle_chromatogram( const pugi::xml_node& node ) const {
        mzml::mzMLdatum< mzml::dataTypeChromatogram >{ tab_ + "\t" }( node );
    }
};


struct mzMLWalker {
    std::string tab_;

    mzMLWalker( std::string tab ) : tab_( tab ) {}

    void operator()( const pugi::xml_node& node ) const {
        for ( auto node1: node.select_nodes( "./*[not(self::cvParam)][not(self::offset)]" ) ) {
            if ( node1.node().name() == std::string( "cvList" )) {
                cvList{ tab_ + "\t"}( node1.node() );
            } else if ( node1.node().name() == std::string( "softwareList" )) {
                mzml::xmlWalker{tab_ + "<softwareList>\t"}( node1.node() );
            } else if ( node1.node().name() == std::string( "instrumentConfigurationList" )) {
                mzml::xmlWalker{tab_ + "<instrumentConfigurationList>\t"}( node1.node() );
            } else if ( node1.node().name() == std::string( "dataProcessingList" )) {
                mzml::xmlWalker{tab_ + "<dataProcessingList>\t"}( node1.node() );
            } else if ( node1.node().name() == std::string( "fileDescription" )) {
                fileDescription{tab_ + "\t"}( node1.node() );
            } else if ( node1.node().name() == std::string( "spectrumList" )) {
                spectrumList{tab_ + "\t"}( node1.node() );
            } else if ( node1.node().name() == std::string( "chromatogramList" )) {
                chromatogramList{tab_ + "\t"}( node1.node() );
            } else {
                mzml::accession ac;
                for (auto param : node.select_nodes("./cvParam")) {
                    std::string accession = param.node().attribute("accession").value();
                    std::string name = param.node().attribute("name").value();
                    ac.assign( accession, name );
                }
                if ( ac.empty() )
                    std::cout << tab_ << node1.node().name() << std::endl;
                else
                    std::cout << tab_ << node1.node().name() << "\t[" << ac.toString() << "]" << std::endl;
                mzMLWalker{ tab_ + "<>\t" }( node1.node() );
            }
        }
    };
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

            for ( auto node: doc.select_nodes( "/indexedmzML/*" ) ) {
                mzMLWalker{"\t"}( node.node() );
            }

            ADDEBUG() << "########################### end spectrumList ####################";
        }
        // auto top = doc.select_node( "/indexedmzML" ).node();

        // pugi::xpath_query query("//ns:spectrum", pugi::xpath_variable_set().add("ns", "http://psi.hupo.org/ms/mzml"));
        // for (auto node : query.evaluate_node_set(doc)) {
        // }

    }

    return 0;
}
