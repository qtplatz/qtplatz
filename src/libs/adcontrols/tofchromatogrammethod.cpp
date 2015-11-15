/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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

#include "tofchromatogrammethod.hpp"
#include "serializer.hpp"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>

namespace adcontrols {

    class TofChromatogramMethod::impl {
    public:

        ~impl() {
        }
        
        impl() : mass_( 0 )
               , massWindow_( 0 )
               , time_( 0 )
               , timeWindow_( 0 )
               , algo_( ePeakAreaOnProfile ) {
        }        

        impl( const impl& t ) : mass_( t.mass_ )
                              , massWindow_( t.massWindow_ )             
                              , time_( t.time_ )
                              , timeWindow_( t.timeWindow_ )             
                              , algo_( t.algo_ ) {
        }
        
        std::string formula_;
        double mass_;
        double massWindow_;
        double time_;
        double timeWindow_;
        TofChromatogramMethod::eIntensityAlgorishm algo_;

    private:
        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int ) {
            using namespace boost::serialization;

            ar & BOOST_SERIALIZATION_NVP( formula_ );
            ar & BOOST_SERIALIZATION_NVP( mass_ );
            ar & BOOST_SERIALIZATION_NVP( massWindow_ );
            ar & BOOST_SERIALIZATION_NVP( time_ );
            ar & BOOST_SERIALIZATION_NVP( timeWindow_ );
            ar & BOOST_SERIALIZATION_NVP( algo_ );
            
        }
    };

    ////////// PORTABLE BINARY ARCHIVE //////////
    template<> void
    TofChromatogramMethod::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        ar & *impl_;
    }

    template<> void
    TofChromatogramMethod::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        ar & *impl_;
    }

    ///////// XML archive ////////
    template<> void
    TofChromatogramMethod::serialize( boost::archive::xml_woarchive& ar, const unsigned int )
    {
        ar & boost::serialization::make_nvp( "impl", *impl_ );
    }

    template<> void
    TofChromatogramMethod::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        ar & boost::serialization::make_nvp( "impl", *impl_ );
    }
    
}

using namespace adcontrols;

TofChromatogramMethod::TofChromatogramMethod() : impl_( new impl() )
{
}

TofChromatogramMethod::TofChromatogramMethod( const TofChromatogramMethod& t ) : impl_( new impl( *t.impl_ ) )
{
}

TofChromatogramMethod::~TofChromatogramMethod() 
{
    delete impl_;
}

const std::string&
TofChromatogramMethod::formula() const
{
    return impl_->formula_;
}

void
TofChromatogramMethod::setFormula( const std::string& formula )
{
    impl_->formula_ = formula;
}

double
TofChromatogramMethod::mass() const
{
    return impl_->mass_;
}

void
TofChromatogramMethod::setMass( double value )
{
    impl_->mass_ = value;
}

double
TofChromatogramMethod::massWindow() const
{
    return impl_->massWindow_;
}

void
TofChromatogramMethod::setMassWindow( double value )
{
    impl_->massWindow_ = value;
}

double
TofChromatogramMethod::time() const
{
    return impl_->time_;
}

void
TofChromatogramMethod::setTime( double seconds )
{
    impl_->time_ = seconds;
}

const double
TofChromatogramMethod::timeWindow() const
{
    return impl_->timeWindow_;
}

void
TofChromatogramMethod::setTimeWindow( double seconds )
{
    impl_->timeWindow_ = seconds;
}

TofChromatogramMethod::eIntensityAlgorishm
TofChromatogramMethod::intensityAlgorithm() const
{
    return impl_->algo_;
}

void
TofChromatogramMethod::setIntensityAlgorithm( eIntensityAlgorishm algo )
{
    impl_->algo_ = algo;
}


