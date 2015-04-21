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

#include "mappedspectra.hpp"
#include "mappedspectrum.hpp"
#include <adcontrols/idaudit.hpp>
#include <boost/numeric/ublas/matrix.hpp>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>

namespace adcontrols {

    class MappedSpectra::impl {
    public:
        ~impl() {
        }
        
        impl( size_t i, size_t j ) : data_( i, j ) {
        }
        
        impl( impl& t ) : data_( t.data_ ) {
        }
        
        boost::numeric::ublas::matrix< MappedSpectrum > data_; // co-added spectral matrix
        idAudit ident_;            

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
    MappedSpectra::serialize( portable_binary_oarchive& ar, const unsigned int )
    {
        ar & *impl_;
    }

    template<> void
    MappedSpectra::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        ar & *impl_;
    }

    ///////// XML archive ////////
    template<> void
    MappedSpectra::serialize( boost::archive::xml_woarchive& ar, const unsigned int )
    {
        ar & BOOST_SERIALIZATION_NVP(*impl_);                        
    }

    template<> void
    MappedSpectra::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        ar & BOOST_SERIALIZATION_NVP(*impl_);            
    }
}

BOOST_CLASS_VERSION( adcontrols::MappedSpectra::impl, 1 )

using namespace adcontrols;

MappedSpectra::~MappedSpectra()
{
    delete impl_;
}

MappedSpectra::MappedSpectra() : impl_( new impl( 16, 16 ) )
{
}

MappedSpectra::MappedSpectra( size_t i, size_t j ) : impl_( new impl( i, j ) )
{
}

MappedSpectra::MappedSpectra( const MappedSpectra& t ) : impl_( new impl( *t.impl_ ) )
{
}

const adcontrols::idAudit&
MappedSpectra::ident() const
{
    return impl_->ident_;
}

MappedSpectra&
MappedSpectra::operator = ( const MappedSpectra& rhs )
{
    delete impl_;
    impl_ = new impl( *rhs.impl_ );
    return *this;
}

size_t
MappedSpectra::size1() const
{
    return impl_->data_.size1();
}

size_t
MappedSpectra::size2() const
{
    return impl_->data_.size2();
}

MappedSpectrum&
MappedSpectra::operator ()( size_t i, size_t j )
{
    return (impl_->data_)( i, j );
}

const MappedSpectrum&
MappedSpectra::operator ()( size_t i, size_t j ) const
{
    return (impl_->data_)( i, j );
}
            
MappedSpectra&
MappedSpectra::operator += ( const boost::numeric::ublas::matrix< uint16_t >& frame )
{
    if ( impl_->data_.size1() <= frame.size1() && impl_->data_.size2() <= frame.size2() ) {

        for ( size_t i = 0; i < impl_->data_.size1(); ++i ) {
            for ( size_t j = 0; j < impl_->data_.size2(); ++j ) {
                if ( auto raw = frame(i, j) ) {
                    ( impl_->data_ )( i, j ) << std::make_pair( raw, 1 );
                }
            }
        }
    }
    
    return *this;
}
