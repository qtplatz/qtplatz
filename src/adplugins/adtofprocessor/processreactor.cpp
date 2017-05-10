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

#include "processreactor.hpp"
#include "addcontextmenu.hpp"
#include "estimatescanlaw.hpp"
#include "oncreate.hpp"
#include <adprocessor/processmediator.hpp>
#include <adspectrometer/massspectrometer.hpp>
#include <adportable/debug.hpp>
#include <boost/uuid/uuid_generators.hpp>

using namespace adtofprocessor;

ProcessReactor::ProcessReactor()
    : clsid_( boost::uuids::string_generator()( adspectrometer::MassSpectrometer::clsid_text ) )
    , onCreate_( std::make_unique< OnCreate >() )
    , addContextMenu_(std::make_unique< AddContextMenu >() )
    , estimateScanLaw_(std::make_unique< EstimateScanLaw >() )
{
}

ProcessReactor::~ProcessReactor()
{
}

ProcessReactor *
ProcessReactor::instance()
{
    static ProcessReactor __instance;
    return &__instance;
}

void
ProcessReactor::initialSetup()
{
    adprocessor::ProcessMediator::instance()->registerOnCreate( clsid_, *onCreate_ );
    adprocessor::ProcessMediator::instance()->registerAddContextMenu( clsid_, *addContextMenu_ );
    adprocessor::ProcessMediator::instance()->registerEstimateScanLaw( clsid_, *estimateScanLaw_ );
}

void
ProcessReactor::finalClose()
{
    adprocessor::ProcessMediator::instance()->unregister( clsid_ );
}
