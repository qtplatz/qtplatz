/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>
#include <atlbase.h>

// it seems that x64 version of CompassXtract does not read ESI/FTICR data collectly.  Only 32bit version works.
#if defined (_M_X64) || defined (__amd64__)
# import "C:/Program Files/Bruker Daltonik/CompassXtract/CompassXtractMS.dll"
#else
# import "C:/Program Files (x86)/Bruker Daltonik/CompassXtract/CompassXtractMS.dll"
#endif

namespace po = boost::program_options;

class bdfconv {
public:
    static bool convert( const std::string& filename );

	struct filename_d {
		static boost::filesystem::path find( const boost::filesystem::path& path ) {
            if ( boost::filesystem::is_directory( path ) && path.extension() == L".d" )
                return path;
            return "";
		}
	};

	struct date_t {
		static boost::posix_time::ptime ptime( const SYSTEMTIME & st ) {
			try {
				boost::gregorian::date d( st.wYear, st.wMonth, st.wDay );
				boost::posix_time::time_duration td( st.wHour, st.wMinute, st.wSecond );
				return boost::posix_time::ptime( d, td );
			} catch ( boost::gregorian::bad_day_of_month& ) {
				return boost::posix_time::ptime();
			}
		}
	};
};

int
main( int ac, char * av[] )
{
    po::variables_map vm;
    {
        po::options_description description( "bdfconv" );
        
        description.add_options()
            ( "help,h",    "Display this help message" )
            ( "version,v", "Display the version number" )
            ( "input-files", po::value< std::vector< std::string > >(), "Input files" )
            ;

        po::positional_options_description p;
        p.add( "input-files", -1 );
        
        po::store( po::command_line_parser( ac, av ).options( description ).positional( p ).run(), vm );
        po::notify(vm);
        if ( vm.count( "help" ) )
            std::cout << description;
    }

    ::CoInitialize( 0 );

    if ( vm.count( "input-files" ) ) {
        std::vector< std::string > files = vm[ "input-files" ].as<std::vector< std::string > >();
        for ( std::string file : files ) {
            std::cout << "input: " << file << std::endl;
            bdfconv::convert( file );
        }
    }

    ::CoUninitialize();
}

bool
bdfconv::convert( const std::string& filename )
{
    auto path = filename_d::find( filename );
    if ( path.empty() )
        return false;

    EDAL::IMSAnalysis2Ptr pAnalysis;
    HRESULT hr;
	if ( ( hr = pAnalysis.CreateInstance( L"EDAL.MSAnalysis" ) ) == S_OK ) {
		try  {
			if ( ( hr = pAnalysis->Open( _bstr_t( path.wstring().c_str() ) ) ) == S_OK ) {

                _bstr_t operatorName = pAnalysis->GetOperatorName();
                _bstr_t analysisName = pAnalysis->GetAnalysisName();
                _bstr_t dateIso      = pAnalysis->GetAnalysisDateTimeIsoString();
				_bstr_t analysisDesc = pAnalysis->GetAnalysisDescription();

                SYSTEMTIME st;
				VariantTimeToSystemTime( pAnalysis->GetAnalysisDateTime(), &st );
				std::string creationDate = boost::posix_time::to_simple_string( date_t::ptime( st ) );
                
                std::cout << "Analysis:     " << analysisName << std::endl;
                std::cout << "Operator:     " << operatorName << std::endl;
                std::cout << "Date/TimeIso: " << dateIso << std::endl;
                std::cout << "Date/Time:    " << creationDate << std::endl;
                std::cout << "Description:  " << analysisDesc << std::endl;

                if ( EDAL::IMSSpectrumCollectionPtr pSpectra = pAnalysis->GetMSSpectrumCollection() ) {
                    if ( int nItem = pSpectra->Count ) {
                        for ( int i = 1; i <= nItem; ++i ) {
                            if ( EDAL::IMSSpectrum2Ptr p = pSpectra->GetItem( i ) ) {
                                std::cout << "#" << i;
                                if ( p->HasSpecType( EDAL::SpectrumType_Profile ) ) {
                                    std::cout << " has profile;";
                                }
                                if ( p->HasSpecType( EDAL::SpectrumType_Line ) ) {
                                    std::cout << " has centroid;";
                                }
                                std::cout << std::endl;
                            }
                        }
                    }
                }
                
            }
        } catch(_com_error& ex ) {
            std::cerr << ex.ErrorMessage() << std::endl;
			return false;
        }
    }    
    return true;
}
