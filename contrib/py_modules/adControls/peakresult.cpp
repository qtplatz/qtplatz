/**************************************************************************
** Copyright (C) 2020 MS-Cheminformatics LLC
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
#include <adcontrols/peak.hpp>
#include <adcontrols/peaks.hpp>
#include <adportable/debug.hpp>
#include <adportable/xml_serializer.hpp>
#include <memory>

using namespace py_module;

namespace py_module {

    boost::python::dict baselines_getitem( const adcontrols::Baselines& self, int index )
    {
        if ( index >= 0 && index < self.size() ) {
            const auto& bs = self.begin() + index;

            boost::python::dict d;
            d[ "baseId" ]           = bs->baseId();
            d[ "manuallyModified" ] = bs->isManuallyModified();
            d[ "startPos" ]         = bs->startPos();
            d[ "stopPos" ]          = bs->stopPos();
            d[ "startHeight" ]      = bs->startHeight();
            d[ "stopHeight" ]       = bs->startHeight();
            d[ "startTime" ]        = double( bs->startTime() );
            d[ "stopTime" ]         = double( bs->stopTime() );
            return d;
        }
        PyErr_SetString(PyExc_IndexError, "index out of range");
        throw boost::python::error_already_set();;
    }

    boost::python::dict peaks_getitem( const adcontrols::Peaks& self, int index )
    {
        boost::python::dict d;
        if ( index >= 0 && index < self.size() ) {
            const auto it = self.begin() + index;

            d[ "startPos" ]          = it->startPos();
            d[ "topPos" ]           = it->topPos();
            d[ "endPos" ]           = it->endPos();
            d[ "startTime" ]        = double( it->startTime() );
            d[ "peakTime" ]         = double( it->peakTime() );
            d[ "endTime" ]          = double( it->endTime() );
            d[ "startHeight" ]      = it->startHeight();
            d[ "peakHeight" ]       = it->peakHeight();
            d[ "endHeight" ]        = it->endHeight();
            d[ "peakArea" ]         = it->peakArea();
            d[ "capacityFactor" ]   = it->capacityFactor();
            d[ "peakWidth" ]        = it->peakWidth();
            d[ "peakAmount" ]       = it->peakAmount();
            d[ "percentArea" ]      = it->percentArea();
            d[ "percentHeight" ]    = it->percentHeight();
            d[ "manuallyModified" ] = it->isManuallyModified();
            d[ "asymmetry"        ] = boost::python::make_tuple( it->asymmetry().asymmetry(), it->asymmetry().startTime(), it->asymmetry().endTime() );

            boost::python::dict res;
            res[ "value" ]          = it->resolution().resolution();
            res[ "baseline" ]       = boost::python::make_tuple( it->resolution().baselineStartTime()
                                                               , it->resolution().baselineStartHeight()
                                                               , it->resolution().baselineEndTime()
                                                               , it->resolution().baselineEndHeight() );
            d[ "resolution"  ]      = res;

            boost::python::dict ntp;
            ntp[ "value" ]          = it->theoreticalPlate().ntp();
            ntp[ "baseline" ]       = boost::python::make_tuple( it->theoreticalPlate().baselineStartTime(), it->theoreticalPlate().baselineStartHeight()
                                                             , it->theoreticalPlate().baselineEndTime(), it->theoreticalPlate().baselineEndHeight() );
            ntp[ "peak" ]           = boost::python::make_tuple( it->theoreticalPlate().peakTopTime(), it->theoreticalPlate().peakTopHeight() );
            d[ "ntp" ]              = ntp;

            boost::python::dict ret;
            ret[ "algorithm" ]      = int( it->retentionTime().algorithm() );
            ret[ "threshold" ]      = boost::python::make_tuple( it->retentionTime().threshold(0), it->retentionTime().threshold(1) );
            ret[ "boundary" ]       = boost::python::make_tuple( it->retentionTime().boundary(0), it->retentionTime().boundary(1) );

            double a, b, c;
            it->retentionTime().eq( a, b, c );
            ret[ "Eq" ]             = boost::python::make_tuple( a, b, c );
            d[ "retentionTime" ]    = ret;
            return d;
        }
        PyErr_SetString(PyExc_IndexError, "index out of range");
        throw boost::python::error_already_set();
    }
}
