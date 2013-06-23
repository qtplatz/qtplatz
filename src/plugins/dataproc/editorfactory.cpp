/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include "editorfactory.hpp"
#include <adplugin/manager.hpp>
#include <qtwrapper/qstring.hpp>
#include <adplugin/widget_factory.hpp>
#include <adportable/utf.hpp>

using namespace dataproc;

EditorFactory::EditorFactory( const adportable::Configuration& config
                              , const std::wstring& path ) : path_( path )
                                                           , config_( config )
{
}

QWidget *
EditorFactory::createEditor( QWidget * parent )
{
        std::string wiid = adportable::utf::to_utf8( config_._interface() );
        QWidget * pw = adplugin::widget_factory::create( wiid.c_str(), 0, parent );
        return pw;
        // return adplugin::manager::widget_factory( config_, path_.c_str(), parent );
    return 0;
}

QString
EditorFactory::title() const
{
    return qtwrapper::qstring::copy( config_.title() );
}
