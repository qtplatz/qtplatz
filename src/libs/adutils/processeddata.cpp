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
#include <compiler/workaround.h>
#include <compiler/disable_unused_parameter.h>

#include "processeddata.hpp"

#include <adcontrols/massspectrum.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/elementalcompositioncollection.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adportable/is_type.hpp>
#include <adportable/debug.hpp>

using namespace adutils;

ProcessedData::ProcessedData()
{
}

ProcessedData::value_type
ProcessedData::toVariant( boost::any & a )
{
	if ( adportable::a_type< MassSpectrumPtr >::is_a( a ) )
        return boost::any_cast< MassSpectrumPtr >( a );

	else if ( adportable::a_type< ChromatogramPtr >::is_a( a ) )
        return boost::any_cast< ChromatogramPtr >( a );

    else if ( adportable::a_type< ProcessMethodPtr >::is_a( a ) )
        return boost::any_cast< ProcessMethodPtr >( a );

    else if ( adportable::a_type< ElementalCompositionCollectionPtr >::is_a( a ) )
        return boost::any_cast< ElementalCompositionCollectionPtr >( a );

    else if ( adportable::a_type< MSCalibrateResultPtr >::is_a( a ) )
        return boost::any_cast< MSCalibrateResultPtr >( a );

    else if ( adportable::a_type< PeakResultPtr >::is_a( a ) )
        return boost::any_cast< PeakResultPtr >( a );

    else if ( adportable::a_type< MSPeakInfoPtr >::is_a( a ) )
        return boost::any_cast< MSPeakInfoPtr >( a );

    else if ( adportable::a_type< MassSpectraPtr >::is_a( a ) )
        return boost::any_cast< MassSpectraPtr >( a );

    else if ( adportable::a_type< SpectrogramClustersPtr >::is_a( a ) )
        return boost::any_cast< SpectrogramClustersPtr >( a );

    adportable::debug(__FILE__, __LINE__)
        << "ProcessedData::toVariant( " << a.type().name() << " ) -- return Nothing()";

    return Nothing();
}
