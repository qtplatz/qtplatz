/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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

#include "samplinginfo.hpp"
#include "metric/prefix.hpp"
#include "massspectrometer.hpp"
#include "metric/prefix.hpp"
#include <adportable/base64.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <boost/exception/all.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/archive/archive_exception.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace adcontrols {
    
    template<typename T = SamplingInfo >
    class SamplingInfo_archive {
    public:
        template<class Archive>
        void serialize( Archive& ar, T& _, const unsigned int version ) {
            if ( version >= 6 ) {
                ar & BOOST_SERIALIZATION_NVP( _.nSamplingDelay_ );
                ar & BOOST_SERIALIZATION_NVP( _.nSamples_ );
                ar & BOOST_SERIALIZATION_NVP( _.nAverage_ );
                ar & BOOST_SERIALIZATION_NVP( _.mode_ );
                ar & BOOST_SERIALIZATION_NVP( _.fsampInterval_ );
                ar & BOOST_SERIALIZATION_NVP( _.horPos_ );
                ar & BOOST_SERIALIZATION_NVP( _.delayTime_ );
            } else {
                uint32_t sampInterval;
                ar & BOOST_SERIALIZATION_NVP( sampInterval );
                ar & BOOST_SERIALIZATION_NVP( _.nSamplingDelay_ );
                ar & BOOST_SERIALIZATION_NVP( _.nSamples_ );
                ar & BOOST_SERIALIZATION_NVP( _.nAverage_ );
                if ( version >= 3 )
                    ar & BOOST_SERIALIZATION_NVP( _.mode_ );
                if ( version >= 4 ) {
                    uint32_t padding( 0 );
                    ar & BOOST_SERIALIZATION_NVP( padding );
                    ar & BOOST_SERIALIZATION_NVP( _.fsampInterval_ );
                }
                if ( version >= 5 ) {
                    ar & BOOST_SERIALIZATION_NVP( _.horPos_ );
                    ar & BOOST_SERIALIZATION_NVP( _.delayTime_ );
                }
                if ( sampInterval )
                    _.fsampInterval_ = sampInterval * 1.0e-12; // ps -> s
            }
        }
        
    };

    ////////// SamplingInfo ///////////
    template<> ADCONTROLSSHARED_EXPORT void SamplingInfo::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        SamplingInfo_archive<>().serialize( ar, *this, version );
    }
    
    template<> ADCONTROLSSHARED_EXPORT void SamplingInfo::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        SamplingInfo_archive<>().serialize( ar, *this, version );
    }
    
    template<> ADCONTROLSSHARED_EXPORT void SamplingInfo::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        SamplingInfo_archive<>().serialize( ar, *this, version );
    }
    
    template<> ADCONTROLSSHARED_EXPORT void SamplingInfo::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        SamplingInfo_archive<>().serialize( ar, *this, version );
    }    

}

using namespace adcontrols;

SamplingInfo::SamplingInfo( double interval
                            , uint32_t ndelay
                            , uint32_t nsamples
                            , uint32_t navgr
                            , uint32_t _mode )   : nSamplingDelay_( ndelay )
                                                 , nSamples_( nsamples )  
                                                 , nAverage_( navgr )
                                                 , mode_( _mode )
                                                 , fsampInterval_( interval )
                                                 , horPos_( 0.0 )
                                                 , delayTime_( 0.0 )
{
    assert( interval < 1.0e-6 );  // 1us limit, for security due to originally this was 'ps' in integer.
}

SamplingInfo::SamplingInfo() : nSamplingDelay_( 0 )
                             , nSamples_( 0 )
                             , nAverage_( 0 )
                             , mode_( 0 )
                             , fsampInterval_( 0.0 )
                             , horPos_( 0.0 )
                             , delayTime_( 0.0 )
{
}

void
SamplingInfo::fSampInterval( double v )
{
    fsampInterval_ = v;
}

double
SamplingInfo::fSampInterval() const
{
    return fsampInterval_;
}

void
SamplingInfo::horPos( double v )
{
    horPos_ = v;
}

double
SamplingInfo::horPos() const
{
    return horPos_;
}

void 
SamplingInfo::setDelayTime( double v )
{
    delayTime_ = v;
}

double
SamplingInfo::delayTime() const
{
    return delayTime_;
}

double
SamplingInfo::fSampDelay() const
{
	return nSamplingDelay_ * fsampInterval_;
}

size_t
SamplingInfo::numberOfTriggers() const
{
    return nAverage_;
}

//----------
void
SamplingInfo::setNumberOfTriggers( size_t value )
{
    nAverage_ = static_cast< uint32_t >(value);
}

uint32_t
SamplingInfo::mode() const
{
    return mode_;
}

void
SamplingInfo::setMode( uint32_t value )
{
    mode_ = value;
}

uint32_t
SamplingInfo::nSamples() const
{
    return nSamples_;
}

void
SamplingInfo::setNSamplingDelay( uint32_t value )
{
    nSamplingDelay_ = value;
}

uint32_t
SamplingInfo::nSamplingDelay() const
{
    return nSamplingDelay_;
}
