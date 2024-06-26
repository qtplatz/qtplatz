// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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

#include "estimatescanlaw.hpp"
#include "calibscanlaw.hpp"
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adportable/debug.hpp>
#include <QMenu>

using namespace infitofprocessor;
    
EstimateScanLaw::EstimateScanLaw()
{
}

EstimateScanLaw::~EstimateScanLaw()
{
}

bool
EstimateScanLaw::operator()( std::shared_ptr< adprocessor::dataprocessor > dp
                             , std::shared_ptr< const adcontrols::MassSpectrum > ms
                             , const std::vector< std::pair< int, int > >& range )
{
    CalibScanLaw()( dp, ms, ms->getMSProperty().instMassRange(), false );
    return true;
}
