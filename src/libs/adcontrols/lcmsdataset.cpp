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

#include "lcmsdataset.hpp"
#include <adcontrols/datareader.hpp>
#include <adcontrols/massspectrometer.hpp>

using namespace adcontrols;

std::vector < std::shared_ptr< adcontrols::DataReader > >
LCMSDataset::dataReaders( bool allPossible ) const
{
    return {}; // std::vector < std::shared_ptr< adcontrols::DataReader > >();
}

adcontrols::MSFractuation *
LCMSDataset::msFractuation() const
{
    if ( auto reader = dataReaders().front() )
        if ( auto spectrometer = reader->massSpectrometer() )
            return spectrometer->msFractuation();
    return nullptr;
}
