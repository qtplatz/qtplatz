// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "mslockmethod.hpp"
#include "msfinder.hpp"
#include "moltable.hpp"
#include <adportable/is_equal.hpp>
#include <adportable/unique_ptr.hpp>
#include <boost/serialization/nvp.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/archive/archive_exception.hpp>
#include <boost/exception/all.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>


namespace adcontrols {

    template< typename T = MSLockMethod >
    class MSLockMethod_archive {
    public:
        template< class Archive >
        void serialize( Archive& ar, T& _, const unsigned int version ) {
            if ( version == 0 ) {
                std::wstring xmlDataClass, xmlReferences;
                ar & BOOST_SERIALIZATION_NVP(_.enabled_);
                ar & BOOST_SERIALIZATION_NVP(_.toleranceMethod_);
                ar & BOOST_SERIALIZATION_NVP(_.algorithm_);
                ar & BOOST_SERIALIZATION_NVP(_.toleranceDa_);
                ar & BOOST_SERIALIZATION_NVP(_.tolerancePpm_);
                ar & BOOST_SERIALIZATION_NVP(_.peakIntensityThreshold_);
                ar & BOOST_SERIALIZATION_NVP(xmlDataClass); // throw away
                ar & BOOST_SERIALIZATION_NVP(xmlReferences); // throw away
            } else {
                ar & BOOST_SERIALIZATION_NVP(_.enabled_);
                ar & BOOST_SERIALIZATION_NVP(_.toleranceMethod_);
                ar & BOOST_SERIALIZATION_NVP(_.algorithm_);
                ar & BOOST_SERIALIZATION_NVP(_.toleranceDa_);
                ar & BOOST_SERIALIZATION_NVP(_.tolerancePpm_);
                ar & BOOST_SERIALIZATION_NVP(_.peakIntensityThreshold_);
                ar & BOOST_SERIALIZATION_NVP( *_.molecules_ );
            }
        }
    };

    template<> ADCONTROLSSHARED_EXPORT void MSLockMethod::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        MSLockMethod_archive<>().serialize( ar, *this, version );
    }
    
    template<> ADCONTROLSSHARED_EXPORT void MSLockMethod::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        MSLockMethod_archive<>().serialize( ar, *this, version );
    }
    
    template<> ADCONTROLSSHARED_EXPORT void MSLockMethod::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        MSLockMethod_archive<>().serialize( ar, *this, version );
    }
    
    template<> ADCONTROLSSHARED_EXPORT void MSLockMethod::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        MSLockMethod_archive<>().serialize( ar, *this, version );
    }    
}

using namespace adcontrols;

MSLockMethod::~MSLockMethod(void)
{
}

MSLockMethod::MSLockMethod() : enabled_( false )
                             , enablePeakThreshold_( false )
                             , toleranceMethod_( idToleranceDaltons )
                             , algorithm_( idFindLargest )
                             , toleranceDa_( 0.2 )
                             , tolerancePpm_( 10.0 )
                             , peakIntensityThreshold_( 10000.0 )
                             , molecules_( std::make_unique< moltable >() )
{
}

MSLockMethod::MSLockMethod(const MSLockMethod & t) : enabled_( t.enabled_ )
                                                   , enablePeakThreshold_( t.enablePeakThreshold_ )
                                                   , toleranceMethod_( t.toleranceMethod_)
                                                   , algorithm_( t.algorithm_ )
                                                   , toleranceDa_( t.toleranceDa_ )
                                                   , tolerancePpm_( t.tolerancePpm_ )
                                                   , peakIntensityThreshold_( t.peakIntensityThreshold_ )
                                                   , molecules_( std::make_unique< moltable >( *t.molecules_ ))
{
}

MSLockMethod&
MSLockMethod::operator=(const MSLockMethod& t)
{
    enabled_ = t.enabled_;
    enablePeakThreshold_ = t.enablePeakThreshold_;
    toleranceMethod_ = t.toleranceMethod_;
    algorithm_ = t.algorithm_;
    toleranceDa_ = t.toleranceDa_;
    tolerancePpm_ = t.tolerancePpm_;
    peakIntensityThreshold_ = t.peakIntensityThreshold_;
    molecules_ = std::make_unique< moltable >( *t.molecules_ );
    return *this;
}

bool
MSLockMethod::enabled() const
{
    return enabled_;
}

void
MSLockMethod::setEnabled( bool t )
{
    enabled_ = t;
}

bool
MSLockMethod::enablePeakThreshold() const
{
    return enablePeakThreshold_;
}

void
MSLockMethod::setEnablePeakThreshold( bool t )
{
    enablePeakThreshold_ = t;
}

idFindAlgorithm
MSLockMethod::algorithm() const
{
    return algorithm_;
}

void
MSLockMethod::setAlgorithm( idFindAlgorithm t )
{
    algorithm_ = t;
}

idToleranceMethod
MSLockMethod::toleranceMethod() const
{
    return toleranceMethod_;
}

void
MSLockMethod::setToleranceMethod( idToleranceMethod t )
{
    toleranceMethod_ = t;
}

double
MSLockMethod::tolerance( idToleranceMethod t ) const
{
    return t == idToleranceDaltons ? toleranceDa_ : tolerancePpm_;
}

void
MSLockMethod::setTolerance( idToleranceMethod t, double value )
{
    if ( t == idToleranceDaltons )
        toleranceDa_ = value;
    else 
        tolerancePpm_ = value;
}

double
MSLockMethod::peakIntensityThreshold() const
{
    return peakIntensityThreshold_;
}

void
MSLockMethod::setPeakIntensityThreshold( double value )
{
    peakIntensityThreshold_ = value;
}

const moltable&
MSLockMethod::molecules() const
{
    return *molecules_;
}

moltable&
MSLockMethod::molecules()
{
    return *molecules_;
}

void
MSLockMethod::setMolecules( const moltable& mols )
{
    molecules_ = std::make_unique< moltable >( mols );
}
