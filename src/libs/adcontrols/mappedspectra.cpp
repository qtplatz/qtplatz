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
#include <boost/uuid/uuid.hpp>

namespace adcontrols {

    class MappedSpectra::impl {
    public:
        ~impl() {
        }
        
        impl( size_t i, size_t j ) : data_( i, j )
                                   , averageCount_( 0 )
                                   , dataReaderUuid_( { 0 } )
                                   , rowIds_( { 0, 0 } )
                                   , trigIds_( { 0, 0 } ) {
        }
        
        impl( impl& t ) : data_( t.data_ )
                        , averageCount_( t.averageCount_ )
                        , dataReaderUuid_( t.dataReaderUuid_ )
                        , rowIds_( t.rowIds_ )
                        , trigIds_( t.trigIds_ ) {
        }
        
        boost::numeric::ublas::matrix< MappedSpectrum > data_; // co-added spectral matrix
        idAudit ident_;
        uint32_t averageCount_;
        boost::uuids::uuid dataReaderUuid_;
        std::pair<int64_t, int64_t > rowIds_;
        std::pair<uint32_t, uint32_t > trigIds_;

    private:
        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( ident_ );
            ar & BOOST_SERIALIZATION_NVP( data_ );
            if ( version >= 2 ) {
                ar & BOOST_SERIALIZATION_NVP( averageCount_ );
                ar & BOOST_SERIALIZATION_NVP( dataReaderUuid_ );
                ar & BOOST_SERIALIZATION_NVP( rowIds_ );
                ar & BOOST_SERIALIZATION_NVP( trigIds_ );
            }
        }
    };

    ////////// PORTABLE BINARY ARCHIVE //////////
    template<> void
    MappedSpectra::serialize( portable_binary_oarchive& ar, const unsigned int )
    {
        ar & *impl_;
    }

    template<> void
    MappedSpectra::serialize( portable_binary_iarchive& ar, const unsigned int )
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
    MappedSpectra::serialize( boost::archive::xml_wiarchive& ar, const unsigned int )
    {
        ar & BOOST_SERIALIZATION_NVP(*impl_);            
    }
}

BOOST_CLASS_VERSION( adcontrols::MappedSpectra::impl, 2 )

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
MappedSpectra::average( const boost::numeric::ublas::matrix< uint16_t >& frame, std::function<double( uint16_t )> binary_to_time )
{
    if ( impl_->data_.size1() <= frame.size1() && impl_->data_.size2() <= frame.size2() ) {

        for ( size_t i = 0; i < impl_->data_.size1(); ++i ) {
            for ( size_t j = 0; j < impl_->data_.size2(); ++j ) {
                if ( auto raw = frame(i, j) ) {
                    ( impl_->data_ )( i, j ) << std::make_pair( binary_to_time( raw ), 1 );
                }
            }
        }
    }
    impl_->averageCount_++;
    return *this;
}

bool
MappedSpectra::sum_in_range( MappedSpectrum& sp, size_t x /* column */, size_t y /* row */, size_t w, size_t h )
{
    for ( size_t i = y; i < impl_->data_.size1() && i < ( y + h ); ++i ) {
        for ( size_t j = x; j < impl_->data_.size2() && j < ( x + w ); ++j ) {
            
            sp += (impl_->data_)( i, j );
            
        }
    }
    return true;
}

// for v3 format datafile support
void
MappedSpectra::setDataReaderUuid( const boost::uuids::uuid& uuid )
{
    impl_->dataReaderUuid_ = uuid;
}

const boost::uuids::uuid&
MappedSpectra::dataReaderUuid() const
{
    return impl_->dataReaderUuid_;
}

uint32_t
MappedSpectra::averageCount() const
{
    return impl_->averageCount_;
}

std::pair< int64_t, int64_t >
MappedSpectra::rowId() const
{
    return impl_->rowIds_;
}

void
MappedSpectra::setRowId( int64_t rowid, bool first )
{
    if ( first )
        impl_->rowIds_.first = rowid;
    else
        impl_->rowIds_.second = rowid;
}


std::pair< uint32_t, uint32_t >
MappedSpectra::trigId() const
{
    return impl_->trigIds_;
}

void
MappedSpectra::setTrigId( uint32_t trigid, bool first )
{
    if ( first )
        impl_->trigIds_.first = trigid;
    else
        impl_->trigIds_.second = trigid;
}

////
