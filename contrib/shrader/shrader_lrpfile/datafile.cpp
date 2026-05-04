/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@MS-Cheminformatics.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include "datafile.hpp"
#include "data_reader.hpp"
#include "export_to_adfs.hpp"
#include "../lrpfile/lrpfile.hpp"
#include "../lrpfile/lrptic.hpp"
#include "../lrpfile/msdata.hpp"
#include "datareader.hpp"
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/datasubscriber.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/processeddataset.hpp>
#include <adportable/debug.hpp>
#include <adportable/utf.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/portfolio.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/any.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>
#include <vector>
#include <filesystem>
#include <iostream>

namespace shrader {

    class datafile::impl {
    public:
        impl() : lrpfile_{0}
               , processedDataset_{0}
               , root_filename_{}
               , vChro_{}
            {};
        std::shared_ptr< shrader::lrpfile > lrpfile_;
		std::shared_ptr< adcontrols::ProcessedDataset > processedDataset_;
        std::wstring root_filename_;
        std::map< std::wstring, size_t > dataIds_;
        std::map< std::string, std::shared_ptr< adcontrols::Chromatogram > > vChro_;
        std::shared_ptr< adcontrols::DataReader > dataReader_;
        shrader::ticc_t ticc_;
    };
}

using namespace shrader;

datafile::~datafile()
{
}

datafile::datafile() : impl_( std::make_unique< impl >() )
{
}

//virtual
void
datafile::accept( adcontrols::dataSubscriber& sub )
{
    // AcquireDataset <LCMSDataset>
	sub.subscribe( *this );

    // subscribe processed dataset
	if ( impl_->processedDataset_ )
		sub.subscribe( *impl_->processedDataset_ );
}

boost::any
datafile::fetch( const std::string& foliumGuid, const std::string& dataType ) const
{
    return fetch( adportable::utf::to_wstring( foliumGuid ), adportable::utf::to_wstring( dataType ) );
}

// virtual
boost::any
datafile::fetch( const std::wstring& foliumGuid, const std::wstring& dataType ) const
{
    // ADDEBUG() << "############# " << __FUNCTION__ << " ############## " << foliumGuid << "\t" << dataType;
    {
        auto it = impl_->vChro_.find ( adportable::utf::to_utf8( foliumGuid ) );
        if ( it != impl_->vChro_.end() ) {
            return it->second;
        }
    }

    {
        auto it = impl_->dataIds_.find( foliumGuid );
        if ( it != impl_->dataIds_.end() ) {
            auto ms = std::make_shared< adcontrols::MassSpectrum >();
            if ( getSpectrum( 0, int(it->second), *ms, 0 ) )
                return ms;
        }
    }
	return {};
}

//virtual
adcontrols::datafile::factory_type
datafile::factory()
{
    ADDEBUG() << "############# " << __FUNCTION__ << " NOT IMPL ##############";
	return 0;
}

size_t
datafile::getFunctionCount() const
{
	return 1;
}

size_t
datafile::getSpectrumCount( int /* fcn */ ) const
{
    return impl_->lrpfile_->number_of_spectra();
}

//virtual
size_t
datafile::getChromatogramCount() const
{
	return 0;
}

bool
datafile::getTIC( int /* fcn */, adcontrols::Chromatogram& c ) const
{
    if ( not impl_->ticc_.empty() ) {
        c.resize( impl_->ticc_.size() );
        for ( size_t idx = 0; idx < impl_->ticc_.size(); ++idx ) {
            const auto& tic = impl_->ticc_.at( idx );
            c.setDatum( idx, { get<0>( tic ), get<1>( tic ) } );
        }
        c.setMinimumTime( std::get< 0 >( impl_->ticc_.front() ) );
        c.setMaximumTime( std::get< 0 >( impl_->ticc_.back() ) );
        c.set_time_of_injection_iso8601( impl_->lrpfile_->time_of_injection() );
        ADDEBUG() << "############# time of injection: " << c.time_of_injection();
        return true;
    }
	return false;
}

//virtual
bool
datafile::getSpectrum( int /* fcn*/, size_t idx, adcontrols::MassSpectrum& ms, uint32_t /* objid */) const
{
    ADDEBUG() << "############# " << __FUNCTION__ << " NOT IMPL ##############";  // V2 file -- no longer supported
    return false;
}

/////////////////////////

bool
datafile::_open( const std::filesystem::path& path, bool )
{
    if ( std::filesystem::exists( path ) ) {

        size_t fsize = std::filesystem::file_size( path );
        std::ifstream in( path, std::ios_base::binary );

        if (( impl_->lrpfile_ = std::make_shared< shrader::lrpfile >() )) {

            if ( impl_->lrpfile_->load( in, fsize ) ) {

                // impl_->lrpfile_->dump( std::cerr, 0 );

                impl_->ticc_ = impl_->lrpfile_->get_ticc();
                impl_->dataReader_ = std::make_shared< local::data_reader >( "lrpfile.1", 0, impl_->lrpfile_ );

                portfolio::Portfolio portfolio;
                portfolio.create_with_fullpath( path.string()  );
                if ( auto folder = portfolio.addFolder( L"Chromatograms" ) ) {
                    if ( auto chro = std::make_shared< adcontrols::Chromatogram >() ) {
                        getTIC( -1, *chro );
                        auto folium = folder.addFolium( "TIC.1" ).assign( chro, chro->dataClass() );
                        impl_->vChro_.emplace( folium.id<char>(),  chro );
                    }
                }

                if ( auto folder = portfolio.addFolder( L"Spectra" ) ) {
                }

                impl_->processedDataset_.reset( new adcontrols::ProcessedDataset );
                impl_->processedDataset_->xml( portfolio.xml() );

                return true;
            }
        }
    }
    return false;
}

size_t
datafile::posFromTime( double ) const
{
    ADDEBUG() << "############# " << __FUNCTION__ << " ##  NOT IMPL  ############";
	return 0;
}

double
datafile::timeFromPos( size_t ) const
{
    ADDEBUG() << "############# " << __FUNCTION__ << " ##  NOT IMPL  ############";
	return 0;
}

bool
datafile::getChromatograms( const std::vector< std::tuple<int, double, double> >&
                            , std::vector< adcontrols::Chromatogram >&
                            , std::function< bool (long curr, long total ) > /* progress */
                            , int begPos
                            , int endPos ) const
{
    ADDEBUG() << "############# " << __FUNCTION__ << " ##  NOT IMPL  ############";
    return false;
}

size_t
datafile::dataReaderCount() const
{
    ADDEBUG() << "############# " << __FUNCTION__ << " ##############";
    return 1;
}

const adcontrols::DataReader *
datafile::dataReader( size_t idx ) const
{
    ADDEBUG() << "############# " << __FUNCTION__ << " ############## NOT IMPL #######";
    return {};
}

const adcontrols::DataReader *
datafile::dataReader( const boost::uuids::uuid& ) const
{
    ADDEBUG() << "############# " << __FUNCTION__ << " ############## NOT IMPL #######";
    return {};
}

std::vector < std::shared_ptr< adcontrols::DataReader > >
datafile::dataReaders( bool allPossible ) const
{
    return { impl_->dataReader_ };
    // std::vector < std::shared_ptr< adcontrols::DataReader > > vec;
    // vec.emplace_back( std::make_shared< local::data_reader >( "traceid.1", 1, impl_->lrpfile_ ) );
    // ADDEBUG() << "############# " << __FUNCTION__ << " ############## " << vec.size();
    // return vec;
}


//static
bool
datafile::is_valid_datafile( const std::filesystem::path& path )
{
	if ( path.extension() == L".lrp" || path.extension() == L".LRP" )
		return true;
	return false;
}

std::shared_ptr< const lrpfile >
datafile::lrpfile() const
{
    return impl_->lrpfile_;
}

bool
datafile::export_rawdata( const adcontrols::datafile& db ) const
{
    auto sqlite = db.sqlite();
    export_to_adfs exporter( std::move( sqlite ) );
    return exporter( *this );
}
