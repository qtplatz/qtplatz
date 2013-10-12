// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include "copyin_visitor.hpp"
#include <boost/variant.hpp>
#include <boost/any.hpp>
#include <adutils/processeddata.hpp>
#include <adfs/adfs.hpp>
#include <adfs/cpio.hpp>

#include <adcontrols/massspectrum.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/elementalcompositioncollection.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adcontrols/msassignedmass.hpp>

#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>

namespace addatafile { namespace detail {

    struct copyin : public boost::static_visitor<bool> {
        adfs::file& file_;
        copyin( adfs::file& f ) : file_( f ) {}

        template<typename T>  bool operator ()( T& ) const { 
            throw boost::bad_any_cast();
        }
    };

}
}


using namespace addatafile;
using namespace addatafile::detail;

bool
copyin_visitor::apply( boost::any& a, adfs::file& dbf )
{
    adutils::ProcessedData::value_type value = adutils::ProcessedData::toVariant( a );
    return boost::apply_visitor( addatafile::detail::copyin(dbf), value );
}

namespace addatafile {
    namespace detail {

	template<> bool
	copyin::operator ()( adutils::ProcessedData::Nothing& ) const
	{
	    // nothing to do here.
	    return true;
	}       
	
	template<> bool
	copyin::operator ()( adcontrols::MassSpectrumPtr& p ) const
	{
		file_.dataClass( p->dataClass() );
	    return adfs::cpio< adcontrols::MassSpectrum >::copyin( *p, file_ );
	}       
	
	template<> bool
	copyin::operator ()( adcontrols::ProcessMethodPtr& p ) const
	{
		file_.dataClass( p->dataClass() );
	    return adfs::cpio< adcontrols::ProcessMethod >::copyin( *p, file_ );
	} 
    
	template<> bool
	copyin::operator ()( adutils::ElementalCompositionCollectionPtr& p ) const
	{
		file_.dataClass( p->dataClass() );
	    return adfs::cpio< adcontrols::ElementalCompositionCollection >::copyin( *p, file_ );
	}       
	
	template<> bool
	copyin::operator ()( adcontrols::ChromatogramPtr& p ) const
	{
		file_.dataClass( p->dataClass() );
	    return adfs::cpio< adcontrols::Chromatogram >::copyin( *p, file_ );
	} 

	template<> bool
	copyin::operator ()( adcontrols::PeakResultPtr& p ) const
	{
		file_.dataClass( p->dataClass() );
		return adfs::cpio< adcontrols::PeakResult >::copyin( *p, file_ );
	} 
	
	template<> bool
	copyin::operator ()( adcontrols::MSCalibrateResultPtr& p ) const
	{
		file_.dataClass( p->dataClass() );
	    return adfs::cpio< adcontrols::MSCalibrateResult >::copyin( *p, file_ );
	} 
    } // namespace detail
} // namespace addatafile


