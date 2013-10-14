/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#include "cpio.hpp"
#include <adfs/cpio.hpp>
#include <adportable/is_type.hpp>
#include <adutils/processeddata.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/baselines.hpp>
#include <adcontrols/baseline.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/peak.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/elementalcompositioncollection.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adcontrols/msassignedmass.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>

using namespace addatafile;

cpio::cpio()
{
}

bool
cpio::save( adfs::file& dbf, const boost::any& a )
{
    using namespace adcontrols;

  	if ( adportable::a_type< adutils::MassSpectrumPtr >::is_a( a ) ) {

        typedef MassSpectrum T;

        dbf.dataClass( T::dataClass() );
        const std::shared_ptr< T > p = boost::any_cast< std::shared_ptr<T> >( a );
        return adfs::cpio< T >::out( *p, dbf );

    } else if ( adportable::a_type< adutils::ChromatogramPtr >::is_a( a ) ) {

        typedef Chromatogram T;

        dbf.dataClass( T::dataClass() );
        const std::shared_ptr< T > p = boost::any_cast< std::shared_ptr<T> >( a );
        return adfs::cpio< T >::out( *p, dbf );

    } else if ( adportable::a_type< adutils::ProcessMethodPtr >::is_a( a ) ) {

        typedef ProcessMethod T;

        dbf.dataClass( T::dataClass() );
        const std::shared_ptr< T > p = boost::any_cast< std::shared_ptr<T> >( a );
        return adfs::cpio< T >::out( *p, dbf );

    } else if ( adportable::a_type< adutils::ElementalCompositionCollectionPtr >::is_a( a ) ) {

        typedef ElementalCompositionCollection T;

        dbf.dataClass( T::dataClass() );
        const std::shared_ptr< T > p = boost::any_cast< std::shared_ptr<T> >( a );
        return adfs::cpio< T >::out( *p, dbf );

    } else if ( adportable::a_type< adutils::MSCalibrateResultPtr >::is_a( a ) ) {

        typedef MSCalibrateResult T;

        dbf.dataClass( T::dataClass() );
        const std::shared_ptr< T > p = boost::any_cast< std::shared_ptr<T> >( a );
        return adfs::cpio< T >::out( *p, dbf );

    } else if ( adportable::a_type< adutils::PeakResultPtr >::is_a( a ) ) {

        typedef PeakResult T;

        dbf.dataClass( T::dataClass() );
        const std::shared_ptr< T > p = boost::any_cast< std::shared_ptr<T> >( a );
        return adfs::cpio< T >::out( *p, dbf );

    } else if ( adportable::a_type< adutils::MSPeakInfoPtr >::is_a( a ) ) {

        typedef MSPeakInfo T;

        dbf.dataClass( T::dataClass() );
        const std::shared_ptr< T > p = boost::any_cast< std::shared_ptr<T> >( a );
        return adfs::cpio< T >::out( *p, dbf );
    }
    return false;
}
