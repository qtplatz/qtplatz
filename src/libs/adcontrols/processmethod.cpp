/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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
#include <compiler/workaround.h>
#include <compiler/disable_unused_parameter.h>

#include "processmethod.hpp"
#include "centroidmethod.hpp"
#include "isotopemethod.hpp"
#include "elementalcompositionmethod.hpp"
#include "mscalibratemethod.hpp"
#include "msreferences.hpp"
#include "msreference.hpp"
#include "targetingmethod.hpp"
#include "peakmethod.hpp"

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/base_object.hpp>

#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>

using namespace adcontrols;

ProcessMethod::~ProcessMethod()
{
}

ProcessMethod::ProcessMethod()
{
}

ProcessMethod::ProcessMethod( const ProcessMethod& t ) : vec_( t.vec_ )
{
}

namespace adcontrols {

    template<> void DECL_EXPORT
    ProcessMethod::appendMethod( const ProcessMethod::value_type& v )
    {
	vec_.push_back( v );
    }

    template<> void DECL_EXPORT
    ProcessMethod::appendMethod( const adcontrols::CentroidMethod& v )
    {
	vec_.push_back( v );
    }
    
    template<> void DECL_EXPORT
    ProcessMethod::appendMethod( const IsotopeMethod& v )
    {
	vec_.push_back( v );
    }
    
    template<> void DECL_EXPORT
    ProcessMethod::appendMethod( const ElementalCompositionMethod& v )
    {
	vec_.push_back( v );
    }
    
    template<> void DECL_EXPORT
    ProcessMethod::appendMethod( const MSCalibrateMethod& v )
    {
	vec_.push_back( v );
    }
    
    template<> void DECL_EXPORT
    ProcessMethod::appendMethod( const TargetingMethod& v )
    {
	vec_.push_back( v );
    }

    template<> void DECL_EXPORT
    ProcessMethod::appendMethod( const PeakMethod& v )
    {
	vec_.push_back( v );
    }

}; // namespace adcontrols

//////
template<class T> struct method_finder {
    static const T * find( const ProcessMethod::vector_type& vec ) {
		auto it = std::find_if( vec.begin(), vec.end(), [&]( const ProcessMethod::value_type& t ){
			return typeid(T) == t.type();
		});
		if ( it != vec.end() )
			return &boost::get< T >( *it );
        return 0;
    }
};

namespace adcontrols {

    template<> DECL_EXPORT /* __declspec(dllexport) */ const adcontrols::CentroidMethod*
    ProcessMethod::find() const
    {
        return method_finder< CentroidMethod >::find( vec_ );
    }
    
    template<> DECL_EXPORT /* __declspec(dllexport) */ const adcontrols::IsotopeMethod*
    ProcessMethod::find() const
    {
        return method_finder< IsotopeMethod >::find( vec_ );
    }
    
    template<> DECL_EXPORT /* __declspec(dllexport) */ const adcontrols::ElementalCompositionMethod*
    ProcessMethod::find() const
    {
        return method_finder< ElementalCompositionMethod >::find( vec_ );
    }
    
    template<> DECL_EXPORT /* __declspec(dllexport) */ const adcontrols::MSCalibrateMethod*
    ProcessMethod::find() const
    {
        return method_finder< MSCalibrateMethod >::find( vec_ );
    }
    
    template<> DECL_EXPORT /* __declspec(dllexport) */ const adcontrols::TargetingMethod*
    ProcessMethod::find() const
    {
        return method_finder< TargetingMethod >::find( vec_ );
    }

    template<> DECL_EXPORT /* __declspec(dllexport) */ const adcontrols::PeakMethod*
    ProcessMethod::find() const
    {
        return method_finder< PeakMethod >::find( vec_ );
    }

}; // namespace adcontrols

///////////

const ProcessMethod::value_type&
ProcessMethod::operator [] ( int idx ) const
{
    return vec_[idx];
}

ProcessMethod::value_type&
ProcessMethod::operator [] ( int idx )
{
    return vec_[idx];
}

void
ProcessMethod::clear()
{
    vec_.clear();
}

size_t
ProcessMethod::size() const
{
    return vec_.size();
}

ProcessMethod::vector_type::iterator
ProcessMethod::begin()
{
    return vec_.begin();
}

ProcessMethod::vector_type::iterator
ProcessMethod::end()
{
    return vec_.end();
}

ProcessMethod::vector_type::const_iterator
ProcessMethod::begin() const
{
    return vec_.begin();
}

ProcessMethod::vector_type::const_iterator
ProcessMethod::end() const
{
    return vec_.end();
}

//////////////////// serialize /////////////////

namespace adcontrols {

    template<> void
    ProcessMethod::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
	(void)version;
	ar << boost::serialization::make_nvp( "ProcessMethod", vec_ );
    }
    
    template<> void
    ProcessMethod::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
	(void)version;
	ar >> boost::serialization::make_nvp("ProcessMethod", vec_);
    }
}; // namespace adcontrols

//////////////////// static ////////////////
bool
ProcessMethod::archive( std::ostream& os, const ProcessMethod& t )
{
    portable_binary_oarchive ar( os );
    ar << t;
    return true;
}

bool
ProcessMethod::restore( std::istream& is, ProcessMethod& t )
{
    portable_binary_iarchive ar( is );
    ar >> t;
    return true;
}
