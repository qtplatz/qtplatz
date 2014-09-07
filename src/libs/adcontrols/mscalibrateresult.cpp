// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include "mscalibrateresult.hpp"
#include "msreference.hpp"
#include "msreferences.hpp"
#include "mscalibration.hpp"
#include "msassignedmass.hpp"
#include "msreference.hpp"
#include "serializer.hpp"

#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/scoped_ptr.hpp>
#include <boost/serialization/vector.hpp>

namespace adcontrols {

    class MSCalibrateResult::impl {
    public:
        impl() : tolerance_(0)
               , threshold_(0)
               , references_( new MSReferences )
               , calibration_( new MSCalibration ) 
               , assignedMasses_( new MSAssignedMasses ) 
               , mode_(0) {
        }

        impl::impl( const impl& t ) : tolerance_( t.tolerance_ )
                                    , threshold_( t.threshold_ )
                                    , references_( new MSReferences( *t.references_ ) )
                                    , calibration_( new MSCalibration( *t.calibration_ ) )
                                    , assignedMasses_( new MSAssignedMasses( *t.assignedMasses_ ) )
                                    , mode_( t.mode_ )
                                    , description_( t.description_ ) {
        }

        double tolerance_;
        double threshold_;
        boost::scoped_ptr< MSReferences > references_;
        boost::scoped_ptr< MSCalibration > calibration_;
        boost::scoped_ptr< MSAssignedMasses > assignedMasses_;

        int mode_;
        std::wstring description_;

        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version ) {
            using namespace boost::serialization;
            if ( version < 2 ) {
                ar & BOOST_SERIALIZATION_NVP(tolerance_);
                ar & BOOST_SERIALIZATION_NVP(threshold_);
                ar & BOOST_SERIALIZATION_NVP(references_);
                ar & BOOST_SERIALIZATION_NVP(calibration_);
                ar & BOOST_SERIALIZATION_NVP(assignedMasses_);
            } else if ( version >= 2 ) {
                ar & BOOST_SERIALIZATION_NVP(tolerance_);
                ar & BOOST_SERIALIZATION_NVP(threshold_);
                ar & boost::serialization::make_nvp("references", *references_);
                ar & boost::serialization::make_nvp("calibration", *calibration_);
                ar & boost::serialization::make_nvp("assignedMasses", *assignedMasses_);
                // trial for multi-turn calibration
                if ( version == 2 ) {
                    double tDelay;
                    ar & BOOST_SERIALIZATION_NVP(tDelay); // deprecated (only on version = 2 )
                    // tDelay has been implemented into MSCalibration class as t0_coeffs
                }
                if ( version >= 3 ) {
                    ar & BOOST_SERIALIZATION_NVP(mode_)
                        & BOOST_SERIALIZATION_NVP(description_)
                        ;
                }
            }
        }
    };
}

BOOST_CLASS_VERSION( adcontrols::MSCalibrateResult::impl, 3 )

namespace adcontrols {

    ////////// PORTABLE BINARY ARCHIVE //////////
    template<> ADCONTROLSSHARED_EXPORT void
    MSCalibrateResult::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        impl_->serialize( ar, version );
    }
    
    template<> ADCONTROLSSHARED_EXPORT void
    MSCalibrateResult::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        impl_->serialize( ar, version );
    }

    ///////// XML archive ////////
    template<> ADCONTROLSSHARED_EXPORT void
    MSCalibrateResult::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        impl_->serialize( ar, version );
    }

    template<> ADCONTROLSSHARED_EXPORT void
    MSCalibrateResult::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        impl_->serialize( ar, version );
    }
}


using namespace adcontrols;

MSCalibrateResult::~MSCalibrateResult()
{
}

MSCalibrateResult::MSCalibrateResult() : impl_( new impl )
{
}

MSCalibrateResult::MSCalibrateResult( const MSCalibrateResult& t ) : impl_( new impl(*t.impl_ ) )
{
}

const MSCalibrateResult&
MSCalibrateResult::operator = ( const MSCalibrateResult& t )
{
    impl_.reset( new impl( *t.impl_ ) );
	return *this;
}

double
MSCalibrateResult::tolerance() const
{
    return impl_->tolerance_;
}

void
MSCalibrateResult::tolerance( double v )
{
    impl_->tolerance_ = v;
}

double
MSCalibrateResult::threshold() const
{
    return impl_->threshold_;
}

void
MSCalibrateResult::threshold( double v )
{
    impl_->threshold_ = v;
}

const MSReferences&
MSCalibrateResult::references() const
{
    return *impl_->references_;
}

MSReferences&
MSCalibrateResult::references()
{
    return *impl_->references_;
}

void
MSCalibrateResult::references( const MSReferences& t )
{
    *impl_->references_ = t;
}

const MSCalibration&
MSCalibrateResult::calibration() const
{
    return *impl_->calibration_;
}

MSCalibration&
MSCalibrateResult::calibration()
{
    return *impl_->calibration_;
}

void
MSCalibrateResult::calibration( const MSCalibration& t )
{
    *impl_->calibration_ = t;
}


MSAssignedMasses&
MSCalibrateResult::assignedMasses()
{
    return *impl_->assignedMasses_;
}

const MSAssignedMasses&
MSCalibrateResult::assignedMasses() const
{
    return *impl_->assignedMasses_;
}

void
MSCalibrateResult::assignedMasses( const MSAssignedMasses& t )
{
    *impl_->assignedMasses_ = t;
}

int
MSCalibrateResult::mode() const
{
    return impl_->mode_;
}

void
MSCalibrateResult::mode( int mode )
{
    impl_->mode_ = mode;
}

const wchar_t *
MSCalibrateResult::description() const
{
    return impl_->description_.c_str();
}

void
MSCalibrateResult::description( const wchar_t * text )
{
    impl_->description_ = text ? text : L"";
}

////////////////  static //////////////////
bool
MSCalibrateResult::archive( std::ostream& os, const MSCalibrateResult& t )
{
    portable_binary_oarchive ar( os );
    ar << t;
    return true;
}

bool
MSCalibrateResult::restore( std::istream& is, MSCalibrateResult& t )
{
    portable_binary_iarchive ar( is );
    ar >> t;
    return true;
}

//static
bool
MSCalibrateResult::xml_archive( std::wostream& os, const MSCalibrateResult& t )
{
    return internal::xmlSerializer("MSCalibrateResult").archive( os, *t.impl_ );
}

//static
bool
MSCalibrateResult::xml_restore( std::wistream& is, MSCalibrateResult& t )
{
    return internal::xmlSerializer("MSCalibrateResult").restore( is, *t.impl_ );
}
