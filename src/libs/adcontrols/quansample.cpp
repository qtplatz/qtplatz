/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "quansample.hpp"
#include "serializer.hpp"
#include <adportable_serializer/portable_binary_oarchive.hpp>
#include <adportable_serializer/portable_binary_iarchive.hpp>
#include <adportable/date_time.hpp>
#include <adportable/iso8601.hpp>
#include <adportable/uuid.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/uuid/uuid_serialize.hpp>
#include <boost/json.hpp>
#include <algorithm>
#include <codecvt>
#include <locale>

namespace adcontrols {

    class QuanSample::impl {
    public:
        ~impl() {
        }
        impl() : uuid_( adportable::uuid()() )
               , rowid_(0)
               , sampleType_( SAMPLE_TYPE_UNKNOWN )
               , inletType_( Quan::Infusion )
               , level_(0)
               , istdId_(-1)
               , injVol_(0)
               , amountsAdded_( 1.0 )
               , istd_( 0 )
               , dataGeneration_( ASIS )
               , scan_range_( 0, -1 )
               , channel_(0) {
        }

        impl( const impl& t ) : uuid_( t.uuid_ )
                              , sequence_uuid_( t.sequence_uuid_ )
                              , rowid_( t.rowid_ )
                              , name_( t.name_ )
                              , dataType_( t.dataType_ )
                              , dataSource_( t.dataSource_ )
                              , sampleType_( t.sampleType_ )
                              , level_( t.level_ )
                              , istdId_( t.istdId_ )
                              , injVol_( t.injVol_ )
                              , amountsAdded_( t.amountsAdded_ )
                              , istd_( t.istd_ )
                              , results_( t.results_ )
                              , dataGeneration_( t.dataGeneration_ )
                              , scan_range_( t.scan_range_ )
                              , channel_( t.channel_ )
                              , description_( t.description_ )
                              , time_of_injection_( t.time_of_injection_ ) {
        }

        boost::uuids::uuid uuid_;                // own id
        boost::uuids::uuid sequence_uuid_;       // points to parement
        int32_t rowid_;                          // row number on sequence
        std::wstring name_;
        std::wstring dataType_;
        std::wstring dataSource_;                // fullpath for data file + "::" + data node
        // std::wstring dataGuid_;               // data guid on portfolio (for redisplay)
        QuanSampleType sampleType_;
        Quan::QuanInlet inletType_;                    // Infusion | Chromatogram
        int32_t level_;                          // 0 for UNK, otherwise >= 1
        int32_t istdId_;                         // id for istd sample (id for myself if this is ISTD)
        double injVol_;                          // conc. for infusion
        double amountsAdded_;                    // added amount for standard
        std::vector< quan::ISTD > istd_;         // index is correspoinding to ISTD id
        QuanResponses results_;
        QuanDataGeneration dataGeneration_;
        std::pair<int32_t,int32_t> scan_range_; // 0 := first spectrum, 1 := second spectrum, -1 := last spectrum
        int32_t channel_;                       // quan protocol id (channel
        std::wstring description_;
        std::string time_of_injection_;

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version ) {
            using namespace boost::serialization;

            ar & BOOST_SERIALIZATION_NVP( uuid_ );
            ar & BOOST_SERIALIZATION_NVP( sequence_uuid_ );
            ar & BOOST_SERIALIZATION_NVP( rowid_ );
            ar & BOOST_SERIALIZATION_NVP( name_ );
            ar & BOOST_SERIALIZATION_NVP( dataSource_ );
            ar & BOOST_SERIALIZATION_NVP( dataType_ );
            if ( version <= 3 ) {
                std::wstring dataGuid;
                ar & BOOST_SERIALIZATION_NVP( dataGuid ); // depricated
            }
            ar & BOOST_SERIALIZATION_NVP( sampleType_ );
            ar & BOOST_SERIALIZATION_NVP( level_ );
            ar & BOOST_SERIALIZATION_NVP( istdId_ );
            ar & BOOST_SERIALIZATION_NVP( injVol_ );
            ar & BOOST_SERIALIZATION_NVP( amountsAdded_ );
            ar & BOOST_SERIALIZATION_NVP( istd_ );
            ar & BOOST_SERIALIZATION_NVP( dataGeneration_ );
            ar & BOOST_SERIALIZATION_NVP( scan_range_ );
            ar & BOOST_SERIALIZATION_NVP( channel_ );
            ar & BOOST_SERIALIZATION_NVP( results_ );
            if ( version >= 2 )
                ar & BOOST_SERIALIZATION_NVP( description_ );

            if ( version >= 4 )
                ar & BOOST_SERIALIZATION_NVP( time_of_injection_ );
        }
    };
}

BOOST_CLASS_VERSION( adcontrols::QuanSample::impl, 4 )

namespace adcontrols {

    ////////// PORTABLE BINARY ARCHIVE //////////
    template<> void
    QuanSample::serialize( portable_binary_oarchive& ar, const unsigned int )
    {
        ar & *impl_;
    }

    template<> void
    QuanSample::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        if ( version >= 3 )
            ar & *impl_;
        else
            impl_->serialize( ar, version );

    }

    ///////// XML archive ////////
    template<> void
    QuanSample::serialize( boost::archive::xml_woarchive& ar, const unsigned int )
    {
        ar & boost::serialization::make_nvp("impl", *impl_);
    }

    template<> void
    QuanSample::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        if ( version >= 3 )
            ar & boost::serialization::make_nvp("impl", *impl_);
        else
            impl_->serialize( ar, version );
    }

    namespace quan {

        ////////// ISTD PORTABLE BINARY ARCHIVE //////////
        template<> void
        ISTD::serialize( portable_binary_oarchive& ar, const unsigned int ) {
            ar & id_;
            ar & amounts_;
        }

        template<> void
        ISTD::serialize( portable_binary_iarchive& ar, const unsigned int ) {
            ar & id_;
            ar & amounts_;
        }

        ///////// ISTD XML archive ////////
        template<> void
        ISTD::serialize( boost::archive::xml_woarchive& ar, const unsigned int ) {
            ar & BOOST_SERIALIZATION_NVP( id_ );
            ar & BOOST_SERIALIZATION_NVP( amounts_ );
        }

        template<> void
        ISTD::serialize( boost::archive::xml_wiarchive& ar, const unsigned int ) {
            ar & BOOST_SERIALIZATION_NVP( id_ );
            ar & BOOST_SERIALIZATION_NVP( amounts_ );
        }
    } // namespace quan
}


using namespace adcontrols;

QuanSample::~QuanSample()
{
}

QuanSample::QuanSample() : impl_( new impl() )
{
}

QuanSample::QuanSample( const QuanSample& t ) : impl_( new impl( *t.impl_ ) )
{
}

QuanSample&
QuanSample::operator = ( const QuanSample& t )
{
    impl_ = std::make_unique< impl >( *t.impl_ );
    return *this;
}

const wchar_t *
QuanSample::name() const
{
    return impl_->name_.c_str();
}

void
QuanSample::name( const wchar_t * v )
{
    impl_->name_ = v ? v : L"";
}

const wchar_t *
QuanSample::dataSource() const
{
    return impl_->dataSource_.c_str();
}

void
QuanSample::dataSource( const wchar_t * v )
{
    impl_->dataSource_ = v ? v : L"";
}


const wchar_t *
QuanSample::description() const
{
    return impl_->description_.c_str();
}

void
QuanSample::description( const wchar_t * v )
{
    impl_->description_ =  v ? v : L"";
}

QuanSample::QuanSampleType
QuanSample::sampleType() const
{
    return impl_->sampleType_;
}

void
QuanSample::sampleType( QuanSampleType v )
{
    impl_->sampleType_ = v;
}

int32_t
QuanSample::istdId() const
{
    return impl_->istdId_;
}

void
QuanSample::istdId( int32_t v )
{
    impl_->istdId_ = v;
}


int32_t
QuanSample::level() const
{
    return impl_->level_;
}

void
QuanSample::level( int32_t v )
{
    impl_->level_ = v;
}

double
QuanSample::injVol() const
{
    return impl_->injVol_;
}

void
QuanSample::injVol( double v )
{
    impl_->injVol_ = v;
}

double
QuanSample::addedAmounts() const
{
    return impl_->amountsAdded_;
}

void
QuanSample::addedAmounts( double v )
{
    impl_->amountsAdded_ = v;
}

const std::vector< quan::ISTD >&
QuanSample::istd() const
{
    return impl_->istd_;
}

void
QuanSample::istd( const std::vector< quan::ISTD > & istd )
{
    impl_->istd_ = istd;
    if ( ! impl_->istd_.empty() )
        std::sort( impl_->istd_.begin(), impl_->istd_.end(), []( const quan::ISTD& a, const quan::ISTD& b ){ return a.id_ < b.id_; });
}

QuanSample&
QuanSample::operator << ( const quan::ISTD& t )
{
    auto it = std::lower_bound( impl_->istd_.begin(), impl_->istd_.end(), t.id_, [](const quan::ISTD& a, uint32_t id){ return a.id_ < id; });
    if ( it->id_ == t.id_ )  // id must be unique
        *it = t;
    else
        impl_->istd_.insert( it, t );
    return *this;
}

QuanSample&
QuanSample::operator << ( const QuanResponse& t )
{
    impl_->results_ << t;
    return *this;
}

const boost::uuids::uuid&
QuanSample::sequence_uuid() const { return impl_->sequence_uuid_; }

int32_t QuanSample::row() const { return impl_->rowid_; }
void QuanSample::sequence_uuid( const boost::uuids::uuid& d, int32_t rowid ) { impl_->sequence_uuid_ = d; impl_->rowid_ = rowid; }

const boost::uuids::uuid&
QuanSample::uuid() const { return impl_->uuid_; }

QuanSample::QuanDataGeneration
QuanSample::dataGeneration() const
{
    return impl_->dataGeneration_;
}

void
QuanSample::dataGeneration( QuanDataGeneration v ) { impl_->dataGeneration_ = v; }

uint32_t
QuanSample::scan_range_first() const { return impl_->scan_range_.first; }

uint32_t
QuanSample::scan_range_second() const { return impl_->scan_range_.second; }

void
QuanSample::scan_range( int32_t first, int32_t second ) { impl_->scan_range_ = std::make_pair( first, second ); }

int32_t
QuanSample::channel() const  { return impl_->channel_; }

void
QuanSample::channel( int32_t t ) { impl_->channel_ = t; }

Quan::QuanInlet
QuanSample::inletType() const
{
    return impl_->inletType_;
}

void
QuanSample::inletType( Quan::QuanInlet v )
{
    impl_->inletType_ = v;
}

const QuanResponses&
QuanSample::results() const
{
    return impl_->results_;
}

QuanResponses&
QuanSample::results()
{
    return impl_->results_;
}

const wchar_t *
QuanSample::dataType() const
{
    return impl_->dataType_.c_str();
}

void
QuanSample::dataType( const wchar_t * v )
{
    impl_->dataType_ = v;
}

void
QuanSample::set_time_of_injection( std::chrono::time_point< std::chrono::system_clock, std::chrono::nanoseconds >&& tp )
{
    impl_->time_of_injection_ = adportable::date_time::to_iso< std::chrono::nanoseconds >( tp );
}

std::chrono::time_point< std::chrono::system_clock, std::chrono::nanoseconds >
QuanSample::time_of_injection() const
{
    if ( auto tp = adportable::iso8601::parse( impl_->time_of_injection_.begin(), impl_->time_of_injection_.end() ) )
        return *tp;
    return {};
}

std::string
QuanSample::time_of_injection_iso8601() const
{
    if ( impl_->time_of_injection_.empty() ) {
        return adportable::date_time::to_iso< std::chrono::nanoseconds >(
            std::chrono::time_point< std::chrono::system_clock, std::chrono::nanoseconds >{} );
    }
    return impl_->time_of_injection_;
}

bool
QuanSample::archive( std::ostream& os, const QuanSample& t )
{
    return internal::binSerializer().archive( os, t );
}

//static
bool
QuanSample::restore( std::istream& is, QuanSample& t )
{
    return internal::binSerializer().restore( is, t );
}

//static
bool
QuanSample::xml_archive( std::wostream& os, const QuanSample& t )
{
    return internal::xmlSerializer("QuanSample").archive( os, t );
}

//static
bool
QuanSample::xml_restore( std::wistream& is, QuanSample& t )
{
    return internal::xmlSerializer("QuanSample").restore( is, t );
}

QuanSample::operator boost::json::object () const
{
    std::wstring_convert< std::codecvt_utf8<wchar_t>, wchar_t> cvt;

    boost::json::array istds;

    for ( const auto& istd: impl_->istd_ ) {
        istds.emplace_back( boost::json::object{{ "id", istd.id_ }, { "amounts", istd.amounts_ }} );
    }
    return
        boost::json::object{
        { "uuid", boost::uuids::to_string( impl_->uuid_ ) }
        , { "sequence_uuid", boost::uuids::to_string( impl_->sequence_uuid_ ) }
        , { "rowid",          impl_->rowid_ }
        , { "name",           cvt.to_bytes( impl_->name_ ) }
        , { "dataType",       cvt.to_bytes( impl_->dataType_ ) }
        , { "dataSource",     cvt.to_bytes( impl_->dataSource_ ) }
        , { "sampleType",     int64_t( impl_->sampleType_ ) }
        , { "inletType",      int64_t( impl_->inletType_ ) }
        , { "level",          impl_->level_ }
        , { "istdId",         impl_->istdId_ }
        , { "injVol",         impl_->injVol_ }
        , { "amountsAdded",   impl_->amountsAdded_ }
        , { "istd",           istds }
        , { "dataGeneration", int64_t( impl_->dataGeneration_ ) }
        , { "results",        static_cast< boost::json::value >( impl_->results_ ).as_array() }
        , { "scan_range",     { impl_->scan_range_.first, impl_->scan_range_.second } } // array
        , { "channel",        impl_->channel_ }
    };
}
