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

#include "msqpeak.hpp"
#include "msqpeaks.hpp"
#include "massspectrum.hpp"
#include "msproperty.hpp"
#include "descriptions.hpp"
#include "description.hpp"
#include "annotation.hpp"
#include "annotations.hpp"

using namespace adcontrols;

static std::wstring empty_wstring;

MSQPeaks::~MSQPeaks()
{
}

MSQPeaks::MSQPeaks()
{
}

MSQPeaks::MSQPeaks( const MSQPeaks & t ) : vec_( t.vec_ )
{
}

void
MSQPeaks::clear()
{
    vec_.clear();
    ident_.clear();
}

size_t
MSQPeaks::size() const
{
    return vec_.size();
}

MSQPeaks::iterator_type
MSQPeaks::begin()
{
    return vec_.begin();
}

MSQPeaks::iterator_type
MSQPeaks::end()
{
    return vec_.end();
}

MSQPeaks::const_iterator_type
MSQPeaks::begin() const
{
    return vec_.begin();
}

MSQPeaks::const_iterator_type
MSQPeaks::end() const
{
    return vec_.end();
}

MSQPeaks::iterator_type
MSQPeaks::find( const std::wstring& dataGuid, int idx, int fcn )
{
    return std::find_if( vec_.begin(), vec_.end(), [=]( value_type& t ){
            return t.dataGuid() == dataGuid && t.fcn() == unsigned(fcn) && t.idx() == unsigned(idx);
        });
}

bool
MSQPeaks::erase( const std::wstring& profGuid )
{
    bool modified( false );

    typedef decltype(*ident_.begin()) ident_value_type;
    auto idit = std::find_if( ident_.begin(), ident_.end(), [=] ( ident_value_type& t ){ return t.second.first == profGuid; } );
    if ( idit != ident_.end() ) {
        modified = true;
        const std::wstring& dataGuid = idit->first;
        auto it = std::remove_if( vec_.begin(), vec_.end(), [=] ( value_type& t ){ return t.dataGuid() == dataGuid; } );
        if ( it != vec_.end() )
            vec_.erase( it, vec_.end() );
        ident_.erase( idit );
    }
    return modified;
}

MSQPeaks::iterator_type
MSQPeaks::erase( iterator_type it )
{
    return vec_.erase( it );
}

MSQPeaks::iterator_type
MSQPeaks::erase( iterator_type first, iterator_type last )
{
    return vec_.erase( first, last );
}

MSQPeaks&
MSQPeaks::operator << ( const MSQPeak& v )
{
    vec_.push_back( v );
    return *this;
}

const MSQPeak&
MSQPeaks::operator [] ( size_t idx ) const
{
    return vec_[ idx ];
}

const std::wstring&
MSQPeaks::parentGuid( const std::wstring& dataGuid ) const
{
    auto it = ident_.find( dataGuid );
    if ( it != ident_.end() )
        return it->second.first;
    return empty_wstring;
}

const std::wstring&
MSQPeaks::dataSource( const std::wstring& dataGuid ) const
{
    auto it = ident_.find( dataGuid );
    if ( it != ident_.end() )
        return it->second.second;
    return empty_wstring;
}

void
MSQPeaks::setData( const MassSpectrum& ms, const std::wstring& dataGuid, const std::wstring& profGuid, const std::wstring& dataSource )
{
    if ( ms.isCentroid() ) {

        adcontrols::segment_wrapper< const MassSpectrum > segs( ms );
        uint32_t fcn = 0;

        for ( auto& fms: segs ) {

            std::string protcol_text;
            auto& descs = fms.getDescriptions();
            auto it = std::find_if( descs.begin(), descs.end(), []( const adcontrols::Description& d ){return d.key() == L"acquire.protocol.label";});
            if ( it != descs.end() )
                prot_texts_[ dataGuid ][ fcn ] = it->text();

            ident_[ dataGuid ] = std::make_pair( profGuid, dataSource );

            const auto& annots = fms.get_annotations();
            auto& prop = fms.getMSProperty();

            for ( uint32_t idx = 0; idx < fms.size(); ++idx ) {

                MSQPeak pk( dataGuid, idx, fcn, this );

                pk.time( fms.getTime(idx) );
                pk.mass( fms.getMass(idx) );
                pk.intensity( fms.getIntensity(idx) );
                pk.mode( prop.mode() );

                auto it = std::find_if( annots.begin(), annots.end(), [=]( const adcontrols::annotation& a ){ return a.index() == int(idx); });
                while ( it != annots.end() ) {
                    if ( it->dataFormat() == adcontrols::annotation::dataText ) {
                        pk.description( it->text() );
                    } else if ( it->dataFormat() == adcontrols::annotation::dataFormula ) {
                        pk.formula( it->text() );
                    }
                    it = std::find_if( it + 1, annots.end(), [=]( const adcontrols::annotation& a ){ return a.index() == int(idx); });                    
                }
                vec_.push_back( pk );
            }
            ++fcn;
        }
    }
}

