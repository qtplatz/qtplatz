/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "mschromatogrammethod.hpp"
#include "moltable.hpp"
#include "serializer.hpp"
#include <adportable/utf.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/utility.hpp>

#include <array>
#include <adportable/float.hpp>


namespace adcontrols {
    namespace depricated {
        // for loading old file
        struct value_type {
            bool enable;
            bool msref;
            double mass;
            std::string formula;
            std::wstring memo;
            value_type() : enable( true ), msref( false ), mass( 0 ) {}
            value_type( const value_type& t ) : enable( t.enable ), msref( t.msref ), mass( t.mass ), formula( t.formula ), memo( t.memo ) {
            }
        };

    }
}

namespace boost {
    namespace serialization {

        using namespace adcontrols;

        template <class Archive >
        void serialize( Archive& ar, adcontrols::depricated::value_type& p, const unsigned int ) {
            ar & BOOST_SERIALIZATION_NVP( p.enable );
            ar & BOOST_SERIALIZATION_NVP( p.msref );
            ar & BOOST_SERIALIZATION_NVP( p.mass );
            ar & BOOST_SERIALIZATION_NVP( p.formula );
            ar & BOOST_SERIALIZATION_NVP( p.memo );
        }
    }
}


namespace adcontrols {

    namespace {
        enum DataSource { Profile, Centroid }; // deprecated
    }

    class MSChromatogramMethod::impl {
    public:
        DataSource deprecated_dataSource_;
        WidthMethod widthMethod_;
        std::vector< double > width_;
        std::pair< double, double > mass_limits_; // lower, upper

        moltable molecules_;
        bool enable_lockmass_;
        double tolerance_;
        bool enableAutoTargeting_;
        std::string dataReaderId_;
        double peakWidthForChromatogram_;

        friend class boost::serialization::access;
        template<class Archive>
        void serialize( Archive& ar, const unsigned int version ) {
            using namespace boost::serialization;

            if ( version <= 4 ) {
                ar & BOOST_SERIALIZATION_NVP( deprecated_dataSource_ );
                ar & BOOST_SERIALIZATION_NVP( widthMethod_ );
                ar & boost::serialization::make_nvp( "width0", width_[ 0 ] );
                ar & boost::serialization::make_nvp( "width1", width_[ 1 ] );
                ar & BOOST_SERIALIZATION_NVP( mass_limits_ );
                if ( version == 4 ) {
                    std::vector< depricated::value_type > formulae;
                    ar & BOOST_SERIALIZATION_NVP( formulae );
                    ar & BOOST_SERIALIZATION_NVP( enable_lockmass_ );
                    ar & BOOST_SERIALIZATION_NVP( tolerance_ );
                    for ( auto& f: formulae ) {
                        moltable::value_type mol;
                        mol.enable() = f.enable;
                        mol.setIsMSRef( f.msref );
                        mol.formula() = f.formula;
                        mol.set_description( adportable::utf::to_utf8( f.memo ) );
                        molecules_ << mol;
                    }

                }
            } else if ( version >= 5 ) {
                ar & BOOST_SERIALIZATION_NVP( deprecated_dataSource_ );
                ar & BOOST_SERIALIZATION_NVP( widthMethod_ );
                ar & BOOST_SERIALIZATION_NVP( width_ );
                ar & BOOST_SERIALIZATION_NVP( mass_limits_ );
                ar & BOOST_SERIALIZATION_NVP( molecules_ );
                ar & BOOST_SERIALIZATION_NVP( enable_lockmass_ );
                ar & BOOST_SERIALIZATION_NVP( tolerance_ );
            }
            if ( version >= 6 ) {
                ar & BOOST_SERIALIZATION_NVP( enableAutoTargeting_ );
                ar & BOOST_SERIALIZATION_NVP( peakWidthForChromatogram_ );
                ar & BOOST_SERIALIZATION_NVP( dataReaderId_ );
            }
        }

        impl() : widthMethod_( widthInDa )
               , width_( 2 )
               , mass_limits_( -1, -1 )
               , enable_lockmass_( false )
               , tolerance_( 0.020 )
               , enableAutoTargeting_( false )
               , dataReaderId_( "" )
               , peakWidthForChromatogram_( 1.0 ) {

            width_[ widthInDa ] = 0.002;
            width_[ widthInRP ] = 100000;

        }

        impl( const impl& t ) : widthMethod_( t.widthMethod_ )
                              , width_( t.width_)
                              , mass_limits_( t.mass_limits_ )
                              , molecules_( t.molecules_ )
                              , enable_lockmass_( t.enable_lockmass_ )
                              , tolerance_( t.tolerance_ )
                              , enableAutoTargeting_( t.enableAutoTargeting_ )
                              , dataReaderId_( t.dataReaderId_ )
                              , peakWidthForChromatogram_( t.peakWidthForChromatogram_ ) {
        }
    };
}

BOOST_CLASS_VERSION( adcontrols::MSChromatogramMethod::impl, 6 )

namespace adcontrols {

    ////////// PORTABLE BINARY ARCHIVE //////////
    template<> void
    MSChromatogramMethod::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        try {
            ar & *impl_; // write v4 format
        } catch ( std::exception& ) {
            BOOST_THROW_EXCEPTION( serializer_error() << info( "serialize 'adcontrols::MSChromatogramMethod' to portable_binary_oarchive" ) );
        }
    }

    template<> void
    MSChromatogramMethod::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        try {
            if ( version <= 2 )
                impl_->serialize( ar, version );
            else
                ar & *impl_;
        } catch ( std::exception& ) {
            BOOST_THROW_EXCEPTION( serializer_error() << info( "serialize 'adcontrols::MSChromatogramMethod' to portable_binary_iarchive" ) );
        }
    }

    ///////// XML archive ////////
    template<> void
    MSChromatogramMethod::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        try {
            ar & boost::serialization::make_nvp( "impl", *impl_ );
        } catch ( std::exception& ) {
            BOOST_THROW_EXCEPTION( serializer_error() << info( "serialize 'adcontrols::MSChromatogramMethod ' to xml_woarchive" ) );
        }
    }

    template<> void
    MSChromatogramMethod::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        try {
            if ( version <= 2 )
                impl_->serialize( ar, version );
            else
                ar & boost::serialization::make_nvp( "impl", *impl_ );
        } catch ( std::exception& ) {
            BOOST_THROW_EXCEPTION( serializer_error() << info( "serialize 'adcontrols::MSChromatogramMethod ' to xml_wiarchive" ) );
        }
    }
}


using namespace adcontrols;

MSChromatogramMethod::~MSChromatogramMethod()
{
}

MSChromatogramMethod::MSChromatogramMethod() : impl_( new impl() )
{
}

MSChromatogramMethod::MSChromatogramMethod( const MSChromatogramMethod& t ) : impl_( new impl( *t.impl_ ) )
{
}

MSChromatogramMethod&
MSChromatogramMethod::operator = ( const MSChromatogramMethod& t )
{
    if ( impl_ != t.impl_ ) {  // can't copy self
        delete impl_;
        impl_ = new impl( *t.impl_ );
    }
    return *this;
}

bool
MSChromatogramMethod::operator == ( const MSChromatogramMethod& t ) const
{
    if ( impl_->dataReaderId_ == t.impl_->dataReaderId_ &&
         impl_->widthMethod_ == t.impl_->widthMethod_ ) {
        for ( int i = 0; i < int(impl_->width_.size()); ++i )
            if ( !adportable::compare<double>::essentiallyEqual( impl_->width_[ i ], t.impl_->width_[ i ] ) )
                return false;
        if ( adportable::compare<double>::essentiallyEqual( impl_->mass_limits_.first, t.impl_->mass_limits_.first ) &&
             adportable::compare<double>::essentiallyEqual( impl_->mass_limits_.second, t.impl_->mass_limits_.second ) )
            return true;
    }
    return false;
}

const std::string&
MSChromatogramMethod::dataReader() const
{
    return impl_->dataReaderId_;
}

void
MSChromatogramMethod::setDataReader( const std::string& name )
{
    impl_->dataReaderId_ = name;
}


MSChromatogramMethod::WidthMethod
MSChromatogramMethod::widthMethod() const
{
    return impl_->widthMethod_;
}

void
MSChromatogramMethod::widthMethod( MSChromatogramMethod::WidthMethod method )
{
    impl_->widthMethod_ = method;
}

double
MSChromatogramMethod::width( WidthMethod method ) const
{
    //assert( size_t(method) < width_.size() );
    return impl_->width_[ method ];
}

void
MSChromatogramMethod::width( double value, WidthMethod method )
{
    //assert( size_t(method) < width_.size() );
    impl_->width_[ method ] = value;
}

double
MSChromatogramMethod::lower_limit() const
{
    return impl_->mass_limits_.first;
}

double
MSChromatogramMethod::upper_limit() const
{
    return impl_->mass_limits_.second;
}

void
MSChromatogramMethod::lower_limit( double v )
{
    impl_->mass_limits_.first = v;
}

void
MSChromatogramMethod::upper_limit( double v )
{
    impl_->mass_limits_.second = v;
}


double
MSChromatogramMethod::width_at_mass( double mass ) const
{
    if ( impl_->widthMethod_ == widthInRP )
        return mass / impl_->width_[ impl_->widthMethod_ ];
    else
        return impl_->width_[ impl_->widthMethod_ ];
}

#if 0
const std::vector< MSChromatogramMethod::value_type >&
MSChromatogramMethod::targets() const
{
    return impl_->formulae_;
}

void
MSChromatogramMethod::targets( const std::vector< value_type >& f )
{
    impl_->formulae_ = f;
}
#endif

bool
MSChromatogramMethod::lockmass() const
{
    return impl_->enable_lockmass_;
}

void
MSChromatogramMethod::setLockmass( bool enable )
{
    impl_->enable_lockmass_ = enable;
}

double
MSChromatogramMethod::tolerance() const
{
    return impl_->tolerance_;
}

void
MSChromatogramMethod::setTolerance( double value )
{
    impl_->tolerance_ = value;
}

bool
MSChromatogramMethod::enableAutoTargeting() const
{
    return impl_->enableAutoTargeting_;
}

void
MSChromatogramMethod::setEnableAutoTargeting( bool f )
{
    impl_->enableAutoTargeting_ = f;
}

const moltable&
MSChromatogramMethod::molecules() const
{
    return impl_->molecules_;
}

moltable&
MSChromatogramMethod::molecules()
{
    return impl_->molecules_;
}

void
MSChromatogramMethod::setMolecules( const moltable& t )
{
    impl_->molecules_ = t;
}

double
MSChromatogramMethod::peakWidthForChromatogram() const
{
    return impl_->peakWidthForChromatogram_;
}

void
MSChromatogramMethod::setPeakWidthForChromatogram( double value )
{
    impl_->peakWidthForChromatogram_ = value;
}
