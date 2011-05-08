// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "copyin_visitor.hpp"
#include <boost/variant.hpp>
#include <boost/any.hpp>
#include <adutils/processeddata.hpp>
#include <adfs/adfs.hpp>
#include <adfs/cpio.hpp>

#include <adcontrols/massspectrum.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/elementalcompositioncollection.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adcontrols/msassignedmass.hpp>

# pragma warning( disable: 4996 )
# include <boost/archive/binary_oarchive.hpp>
# include <boost/archive/binary_iarchive.hpp>
# pragma warning( default: 4996 )

namespace addatafile { namespace detail {

    struct copyin : public boost::static_visitor<bool> {
        adfs::folium& folium_;
        copyin( adfs::folium& f ) : folium_( f ) {}

        template<typename T>  bool operator ()( T& t ) const { 
            throw boost::bad_any_cast();
        }
    };

}
}


using namespace addatafile;
using namespace addatafile::detail;

bool
copyin_visitor::apply( boost::any& a, adfs::folium& dbf )
{
    return boost::apply_visitor( addatafile::detail::copyin(dbf), adutils::ProcessedData::toVariant( a ) );
}

template<> bool
copyin::operator ()( adutils::ProcessedData::Nothing& ) const
{
    // nothing to do here.
    return true;
}       

template<> bool
copyin::operator ()( adcontrols::MassSpectrumPtr& p ) const
{
    return adfs::cpio< adcontrols::MassSpectrum >::copyin( *p, folium_ );
}       

template<> bool
copyin::operator ()( adcontrols::ProcessMethodPtr& p ) const
{
    return adfs::cpio< adcontrols::ProcessMethod >::copyin( *p, folium_ );
} 


template<> bool
copyin::operator ()( adutils::ElementalCompositionCollectionPtr& p ) const
{
    return adfs::cpio< adcontrols::ElementalCompositionCollection >::copyin( *p, folium_ );
}       


template<> bool
copyin::operator ()( adcontrols::ChromatogramPtr& p ) const
{
    return adfs::cpio< adcontrols::Chromatogram >::copyin( *p, folium_ );
} 


template<> bool
copyin::operator ()( adcontrols::MSCalibrateResultPtr& p ) const
{
    return adfs::cpio< adcontrols::MSCalibrateResult >::copyin( *p, folium_ );
} 


