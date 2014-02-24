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

#include "adplugin_global.h"
#include "widget_factory.hpp"
#include "plugin_ptr.hpp"
#include "plugin.hpp"
#include "manager.hpp"
#include <adportable/utf.hpp>
#include <QObject>

using namespace adplugin;

widget_factory::widget_factory()
{
}

widget_factory::~widget_factory()
{
}

QWidget *
widget_factory::create( const char * wiid, const char * clsid, QWidget * parent )
{
    std::wstring wiidw = adportable::utf::to_wstring( reinterpret_cast< const unsigned char * >( wiid ) );
    return widget_factory::create( wiidw.c_str(), clsid, parent );
}

QWidget *
widget_factory::create( const wchar_t * wiid, const char * clsid, QWidget * parent )
{
    std::vector< adplugin::plugin_ptr > vec;

    adplugin::manager::instance()->select_iids( ".*\\.adplugins\\.widget_factory\\..*", vec );
    
    for ( const adplugin::plugin_ptr& d: vec ) {

        if ( clsid && std::string( clsid ) != d->clsid() )
            continue;

        widget_factory * factory = d->query_interface< adplugin::widget_factory >();
        QWidget * pw = factory->create_widget( wiid, parent );
        if ( pw )
            return pw;

    }
    return 0;
}
