/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#include "massspectrometerfactory.hpp"
#include "datainterpreter.hpp"
#include "massspectrometer.hpp"
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/datainterpreter.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adplugin/visitor.hpp>
#include <mutex>

using namespace adspectrometer;

// adplugin is managed by ref_count
std::shared_ptr< MassSpectrometerFactory > MassSpectrometerFactory::instance_ = 0;

static std::once_flag flag;

MassSpectrometerFactory *
MassSpectrometerFactory::instance()
{
    //std::call_once( flag, [] () { instance_ = std::make_shared< MassSpectrometerFactory >(); } );
    return 0; // instance_.get();
}

