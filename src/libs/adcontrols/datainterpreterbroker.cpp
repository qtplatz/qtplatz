// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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

#include "datainterpreterbroker.hpp"
#include "datainterpreter.hpp"

#include <algorithm>
#include <map>
#include <string>
#include <adportable/string.hpp>
#include <adportable/debug.hpp>

using namespace adcontrols;

namespace adcontrols {

    class DataInterpreterBroker::impl {
    public:
        std::map< std::string, DataInterpreterFactory * > factories_;
    };
}

DataInterpreterBroker *
DataInterpreterBroker::instance()
{
    static DataInterpreterBroker __instance__;
    return &__instance__;
}

DataInterpreterBroker::DataInterpreterBroker() : impl_( new impl() )
{
}

DataInterpreterBroker::~DataInterpreterBroker()
{
    delete impl_;
}

bool
DataInterpreterBroker::register_factory( DataInterpreterFactory * factory, const std::wstring& name )
{
    return false;
}
