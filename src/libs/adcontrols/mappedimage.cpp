/**************************************************************************
** Copyright (C) 2014-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2014-2015 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
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

#include "mappedimage.hpp"
#include <adcontrols/idaudit.hpp>
#include <adcontrols/mappedspectra.hpp>
#include <adcontrols/mappedspectrum.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <adportable/debug.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <numeric>
#include <ratio>

namespace adcontrols {

    class MappedImage::impl {
    public:
        ~impl() {}
        impl( size_t i, size_t j ) : data_( i, j )
                                   , tof_range_( std::make_pair( 0, 0 ) )
                                   , z_( std::numeric_limits<double>::min() )
                                   , mergeCount_( 0 ) {
            data_.clear();
        }
        impl( impl& t ) : data_( t.data_ )
                        , tof_range_( std::make_pair( 0, 0 ) )
                        , z_( t.z_ )
                        , mergeCount_( t.mergeCount_ ) {
        }            

        adcontrols::idAudit ident_;
        boost::numeric::ublas::matrix< double > data_;
        boost::uuids::uuid data_origin_uuid_;
        std::pair< uint32_t, uint32_t > tof_range_;
        double z_;
        size_t mergeCount_;

    private:
            
        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( ident_ );
            ar & BOOST_SERIALIZATION_NVP( data_ );
        }
    };

    ////////// PORTABLE BINARY ARCHIVE //////////
    template<> void
    MappedImage::serialize( portable_binary_oarchive& ar, const unsigned int )
    {
        ar & *impl_;
    }

    template<> void
    MappedImage::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        ar & *impl_;
    }

    ///////// XML archive ////////
    template<> void
    MappedImage::serialize( boost::archive::xml_woarchive& ar, const unsigned int )
    {
        ar & BOOST_SERIALIZATION_NVP(*impl_);                        
    }

    template<> void
    MappedImage::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        ar & BOOST_SERIALIZATION_NVP(*impl_);            
    }
}

BOOST_CLASS_VERSION( adcontrols::MappedImage::impl, 1 )

using namespace adcontrols;

MappedImage::~MappedImage()
{
    delete impl_;
}

MappedImage::MappedImage() : impl_( new impl( 16, 16 ) )
{
}

MappedImage::MappedImage( size_t i, size_t j ) : impl_( new impl( i, j ) )
{
}

MappedImage::MappedImage( const MappedImage& t ) : impl_( new impl( *t.impl_ ) )
{
}

const adcontrols::idAudit&
MappedImage::ident() const
{
    return impl_->ident_;
}

MappedImage&
MappedImage::operator = ( const MappedImage& rhs )
{
    delete impl_;
    impl_ = new impl( *rhs.impl_ );
    return *this;
}

size_t
MappedImage::size1() const
{
    return impl_->data_.size1();
}

size_t
MappedImage::size2() const
{
    return impl_->data_.size2();
}

double&
MappedImage::operator ()( size_t i, size_t j )
{
    return (impl_->data_)( i, j );
}

const double
MappedImage::operator ()( size_t i, size_t j ) const
{
    return (impl_->data_)( i, j );
}

MappedImage::operator const boost::numeric::ublas::matrix< double >& () const
{
    return impl_->data_;
}

double
MappedImage::max_z() const
{
    return impl_->z_;
}

size_t
MappedImage::mergeCount() const
{
    return impl_->mergeCount_;
}

void
MappedImage::setMergeCount( size_t count )
{
    impl_->mergeCount_ = count;
}

bool
MappedImage::merge( const boost::numeric::ublas::matrix<uint16_t>& frame, unsigned low, unsigned high )
{
    if ( frame.size1() != impl_->data_.size1() || frame.size2() != impl_->data_.size2() ) {
        // dimension mismatch
        impl_->data_.resize( frame.size1(), frame.size2() );
        impl_->data_.clear();
        impl_->mergeCount_ = 0;
    }

    for ( size_t i = 0; i < frame.size1(); ++i ) {
        for ( size_t j = 0; j < frame.size2(); ++j ) {

            if ( auto tof = frame(i, j) ) { // has hit
                if ( low <= tof && tof <= high ) {
                    impl_->data_( i, j ) += 1.0;
                    impl_->z_ = std::max( impl_->z_, impl_->data_( i, j ) );
                }
            }
            
        }
    }
    impl_->mergeCount_++;
    return true;
}

bool
MappedImage::merge( const MappedSpectra& map, double tof, double width )
{
    if ( map.size1() != impl_->data_.size1() || map.size2() != impl_->data_.size2() ) {
        // dimension mismatch
        impl_->data_.resize( map.size1(), map.size2() );
        impl_->data_.clear();
        impl_->mergeCount_ = 0;
    }

    double t0 = tof - width / 2;
    double t1 = tof + width / 2;

    for ( size_t i = 0; i < map.size1(); ++i ) {
        for ( size_t j = 0; j < map.size2(); ++j ) {
            
            auto& sp = map( i, j );

            if ( sp.size() > 0 ) {
                auto beg = std::lower_bound( sp.begin(), sp.end(), t0, []( const MappedSpectrum::datum_type& a, double b ){ return a.first < b; } );
                if ( beg != sp.end() ) {
                    auto end = std::lower_bound( sp.begin(), sp.end(), t1, []( const MappedSpectrum::datum_type& a, double b ){ return a.first < b; } );
                    size_t d = std::accumulate( beg, end, size_t(0), []( size_t a, const MappedSpectrum::datum_type& b ){ return a + b.second; } );
                    impl_->data_( i, j ) += d;
                    impl_->z_ = std::max( impl_->z_, impl_->data_( i, j ) );
                }
            }
        }
    }
    
    impl_->mergeCount_ += map.averageCount();
    return true;
}

bool
MappedImage::merge( const MappedSpectra& map )
{
    if ( map.size1() != impl_->data_.size1() || map.size2() != impl_->data_.size2() ) {
        // dimension mismatch
        impl_->data_.resize( map.size1(), map.size2() );
        impl_->data_.clear();
        impl_->mergeCount_ = 0;
    }

    for ( size_t i = 0; i < map.size1(); ++i ) {
        for ( size_t j = 0; j < map.size2(); ++j ) {
            
            auto& sp = map( i, j );
            
            if ( sp.size() > 0 ) {
                size_t d = std::accumulate( sp.begin(), sp.end(), size_t(0), []( size_t a, const MappedSpectrum::datum_type& b ){ return a + b.second; } );
                impl_->data_( i, j ) += d;
                impl_->z_ = std::max( impl_->z_, impl_->data_( i, j ) );
            }
        }
    }

    impl_->mergeCount_ += map.averageCount();    
    return true;
}

