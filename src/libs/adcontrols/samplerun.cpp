/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
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

#if defined _MSC_VER
#pragma warning(disable:4996)
#endif

#include "samplerun.hpp"
#include "adcontrols_global.h"
#include "serializer.hpp"
#include "idaudit.hpp"
#include <adportable/debug.hpp>
#include <adportable/profile.hpp>
#include <adportable/uuid.hpp>
#include <adportable/utf.hpp>
#include <adportable/date_string.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <adportable/split_filename.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <cstdint>
#include <memory>
#include <vector>

namespace adcontrols {

    class SampleRun::impl {
        
    public:
        ~impl() {
        }

        impl() : methodTime_( 60.0 )
               , replicates_( 999 )
               , dataDirectory_( adportable::profile::user_data_dir<wchar_t>() + L"/data/"
                                 + adportable::date_string::wstring( boost::posix_time::second_clock::local_time().date() ) )
               , filePrefix_( L"RUN_0001" )
               , runCount_( 0 )
               , runNumber_( 0 ) {

            std::ostringstream os;

            os << "<p>ID: " << ident_.uuid() << "</p>" << std::endl;
            os << "<p>Created: " << ident_.dateCreated() << "</p>"
               << "<p>Computer: <i>'" << adportable::utf::to_utf8( ident_.idComputer() ) << "'</i></p>"
               << "<p>by <i>" << adportable::utf::to_utf8( ident_.nameCreatedBy() ) << "</i></p>" << std::endl;

            description_ = os.str();
            runNumber_ = findLastRunNumber();
            filePrefix_ = make_name( runNumber_ );
        }

        impl( const impl& t ) : ident_( t.ident_ )
                              , methodTime_( t.methodTime_ )
                              , replicates_( t.replicates_ )
                              , dataDirectory_( t.dataDirectory_ )
                              , filePrefix_( t.filePrefix_ )
                              , description_( t.description_ )
                              , runCount_( t.runCount_ )
                              , runNumber_( t.runNumber_ ) {
        }
        
        size_t findLastRunNumber() {
            boost::filesystem::path dir( dataDirectory_ );
            
            boost::filesystem::path prefix = adportable::split_filename::prefix<wchar_t>( filePrefix_ );    
            
            size_t lastRunNumber(0);
            if ( boost::filesystem::exists( dir ) && boost::filesystem::is_directory( dir ) ) {
                using boost::filesystem::directory_iterator;
                for ( directory_iterator it( dir ); it != directory_iterator(); ++it ) {
                    boost::filesystem::path fname = (*it);
                    if ( fname.extension().string() == ".adfs" || fname.extension().string() == ".adfs~" ) {
                        lastRunNumber = std::max( int(lastRunNumber), adportable::split_filename::trailer_number_int( fname.stem().wstring() ) );
                    }
                }
            }
            return lastRunNumber;
        }

        std::wstring make_name( size_t n ) {
            boost::filesystem::path prefix = adportable::split_filename::prefix<wchar_t>( filePrefix_ );
            std::wostringstream o;
            o << prefix.wstring() << std::setw( 4 ) << std::setfill( L'0' ) << ( n );
            return o.str();
        }
        
        idAudit ident_;
        double methodTime_;
        size_t replicates_;
        std::wstring dataDirectory_;
        std::wstring filePrefix_;
        std::string description_;

        // exclude from archive
        size_t runCount_;
        size_t runNumber_;

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( ident_ );
            ar & BOOST_SERIALIZATION_NVP( methodTime_ );
            ar & BOOST_SERIALIZATION_NVP( replicates_ );
            ar & BOOST_SERIALIZATION_NVP( dataDirectory_ );
            ar & BOOST_SERIALIZATION_NVP( filePrefix_ );
            ar & BOOST_SERIALIZATION_NVP( description_ );
        }
    };
}

BOOST_CLASS_VERSION( adcontrols::SampleRun::impl, 1 )

namespace adcontrols {
    ////////// PORTABLE BINARY ARCHIVE //////////
    template<> void
    SampleRun::serialize( portable_binary_oarchive& ar, const unsigned int )
    {
        ar & *impl_;
    }

    template<> void
    SampleRun::serialize( portable_binary_iarchive& ar, const unsigned int )
    {
        ar & *impl_;        
    }

    ///////// XML archive ////////
    template<> ADCONTROLSSHARED_EXPORT void
    SampleRun::serialize( boost::archive::xml_woarchive& ar, const unsigned int )
    {
        ar & boost::serialization::make_nvp("impl", *impl_);
    }

    template<> ADCONTROLSSHARED_EXPORT void
    SampleRun::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        ar & boost::serialization::make_nvp("impl", *impl_);
    }
}

using namespace adcontrols;

SampleRun::~SampleRun()
{
    ADDEBUG() << "############ DTOR SampleRun ################";
}

SampleRun::SampleRun() : impl_( new impl() )
{
}

SampleRun::SampleRun( const SampleRun& t ) : impl_( new impl( *t.impl_ ) )
{
}

const idAudit&
SampleRun::ident() const
{
    return impl_->ident_;
}

double
SampleRun::methodTime() const
{
    return impl_->methodTime_;
}

void
SampleRun::methodTime( double v ) const
{
    impl_->methodTime_ = v;
}

size_t
SampleRun::replicates() const
{
    return impl_->replicates_;
}

void
SampleRun::replicates( size_t v )
{
    impl_->replicates_ = v;
}

const wchar_t *
SampleRun::dataDirectory() const
{
    return impl_->dataDirectory_.c_str();
}

void
SampleRun::dataDirectory( const wchar_t * v )
{
    setDataDirectory( v ? v : L"" );
}

void
SampleRun::setDataDirectory( const std::wstring& dir )
{
    impl_->dataDirectory_ = dir;
    impl_->runNumber_ = impl_->findLastRunNumber() + 1;
    impl_->filePrefix_ = impl_->make_name( impl_->runNumber_ );
}

const wchar_t *
SampleRun::filePrefix() const // RUN_
{
    return impl_->filePrefix_.c_str();
}

void
SampleRun::setFilePrefix( const std::wstring& file )
{
    impl_->filePrefix_ = file;
    impl_->runNumber_  = adportable::split_filename::trailer_number_int( file );

    ADDEBUG() << "############ setFilePrefix(" << file << ") run# " << impl_->runNumber_ << " ###################";
}

const char * // utf8
SampleRun::description() const
{
    return impl_->description_.c_str();
}

void
SampleRun::description( const char * t )
{
    impl_->description_ = t ? t : "";
}

std::wstring
SampleRun::runname() const
{
    return impl_->filePrefix_;
}

std::wstring
SampleRun::filename( const wchar_t * extension ) const
{
    boost::filesystem::path dir( impl_->dataDirectory_ );
    boost::filesystem::path path( dir / impl_->filePrefix_ );
    path.replace_extension( extension );
    return path.wstring();
}

size_t
SampleRun::operator ++ ()
{
    impl_->runNumber_++;

    boost::filesystem::path dir( impl_->dataDirectory_ );
    
    bool exists( false );
    do {
        boost::filesystem::path path = dir / ( impl_->make_name( impl_->runNumber_ ) + L".adfs" );
        if ( exists = boost::filesystem::exists( path ) )
            impl_->runNumber_++;
    } while ( exists );

    impl_->filePrefix_ = impl_->make_name( impl_->runNumber_ );

    ADDEBUG() << "############ increment (" << impl_->filePrefix_ << ") run# " << impl_->runNumber_ << " ###################";

    return ++(impl_->runCount_);
}

size_t
SampleRun::runCount() const
{
    return impl_->runCount_;
}

//static
bool
SampleRun::archive( std::ostream& os, const SampleRun& t )
{
    return internal::binSerializer().archive( os, t );
}

//static
bool
SampleRun::restore( std::istream& is, SampleRun& t )
{
    return internal::binSerializer().restore( is, t );
}

//static
bool
SampleRun::xml_archive( std::wostream& os, const SampleRun& t )
{
    return internal::xmlSerializer("SampleRun").archive( os, *t.impl_ );
}

//static
bool
SampleRun::xml_restore( std::wistream& is, SampleRun& t )
{
    return internal::xmlSerializer("SampleRun").restore( is, *t.impl_ );
}
