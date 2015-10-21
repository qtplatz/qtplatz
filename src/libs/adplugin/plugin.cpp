/**************************************************************************
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
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

#include <compiler/preamble.h>
#include "plugin.hpp"
#include "orbfactory.hpp"
#include "orbservant.hpp"
#include <adlog/logger.hpp>
#include <adportable/debug.hpp>

using namespace adplugin;

plugin::~plugin()
{
    ADTRACE() << "##### plugin dtor called #####";
}

plugin::plugin()
{
}

void
plugin::setConfig( const std::string& adpluginspec, const std::string& xml )
{
    clsid_ = adpluginspec; // full path name to <>.adplugin file
    spec_ = xml; // contents of .adplugin file
    // this data will be noticed via visitor pattern @ adplugin_manager::manager::data::install
}

std::shared_ptr< plugin >
plugin::pThis()
{
    try {
        return shared_from_this();
    } catch ( std::bad_weak_ptr& ) {
        // workaround
        ADDEBUG() << "adplugin::plugin bad weak_ptr found for: " << clsid_;
        return std::shared_ptr< plugin >( this );
    }
}

std::shared_ptr< const plugin >
plugin::pThis() const
{
    try {
        return shared_from_this();
    } catch ( std::bad_weak_ptr& ) {
        // workaround
        ADDEBUG() << "adplugin::plugin bad weak_ptr found for: " << clsid_;
        return 0;
        // return std::shared_ptr< const plugin >( this );
    }
}
