/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include "peakresult.hpp"
#include "baselines.hpp"
#include "baseline.hpp"
#include "peaks.hpp"
#include "peak.hpp"
#include <adportable/portable_binary_iarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/string.hpp> 
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/scoped_ptr.hpp>

namespace adcontrols {

    template< typename T = PeakResult >
    class PeakResult_archive {
    public:
        template< class Archive >
        void serialize( Archive& ar, T& _, const unsigned int version ) {
            using namespace boost::serialization;            
            if ( version < 1 ) {
                boost::scoped_ptr< Baselines > baselines;
                boost::scoped_ptr< Peaks > peaks;
                ar & BOOST_SERIALIZATION_NVP( baselines );
                ar & BOOST_SERIALIZATION_NVP( peaks );
                if ( ! Archive::is_saving::value ) {
                    _.baselines_ = std::make_shared< Baselines >( *baselines );
                    _.peaks_ = std::make_shared< Peaks >( *peaks );
                }
                
            } else {
                ar & BOOST_SERIALIZATION_NVP(_.baselines_);
                ar & BOOST_SERIALIZATION_NVP(_.peaks_);
            }
        }
    };

    ////////// PORTABLE BINARY ARCHIVE //////////
    template<> void
    PeakResult::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        PeakResult_archive<>().serialize( ar, *this, version );
    }

    template<> void
    PeakResult::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        PeakResult_archive<>().serialize( ar, *this, version );
    }

    ///////// XML archive ////////
    template<> void
    PeakResult::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        PeakResult_archive<>().serialize( ar, *this, version );
    }

    template<> void
    PeakResult::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        PeakResult_archive<>().serialize( ar, *this, version );
    }
}

using namespace adcontrols;

PeakResult::~PeakResult()
{
}

PeakResult::PeakResult() : baselines_( std::make_shared< Baselines >() )
                         , peaks_( std::make_shared< Peaks >() ) 
{
}

PeakResult::PeakResult( const PeakResult& t ) : baselines_( std::make_shared< Baselines >( *t.baselines_ ) )
                                              , peaks_( std::make_shared< Peaks >( *t.peaks_ ) ) 
{
}

PeakResult::PeakResult( const Baselines& bs
                        , const Peaks& pks ) : baselines_( std::make_shared< Baselines >( bs ) )
                                             , peaks_( std::make_shared< Peaks >( pks ) ) 
{
}

void
PeakResult::clear()
{
    baselines_ = std::make_shared< Baselines >();
    peaks_ = std::make_shared< Peaks >();
}

const Baselines& 
PeakResult::baselines() const
{
	return * baselines_;
}

Baselines&
PeakResult::baselines()
{
	return * baselines_;
}

void
PeakResult::setBaselines( const Baselines& t )
{
    *baselines_ = t;
}

const Peaks&
PeakResult::peaks() const
{
	return * peaks_;
}

Peaks&
PeakResult::peaks()
{
	return * peaks_;
}

void
PeakResult::setPeaks( const Peaks& t )
{
    *peaks_ = t;
}

// ----- static -----
bool
PeakResult::archive( std::ostream& os, const PeakResult& t )
{
    portable_binary_oarchive ar( os );
    ar << t;
    return true;
}

bool
PeakResult::restore( std::istream& is, PeakResult& t )
{
    portable_binary_iarchive ar( is );
    ar >> t;
    return true;
}


