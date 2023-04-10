// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
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

#pragma once

#include <QtGlobal>
#include <aggregation/aggregate.h>
#include <extensionsystem/pluginmanager.h>

namespace qtwrapper {

    struct plugin_manager {
#if QTC_VERSION >= 0x08'00'00
        template <typename T> static QList<T *> getObjects() {
            QReadLocker lock( ExtensionSystem::PluginManager::listLock() );
            QList<T *> results;
            auto all = ExtensionSystem::PluginManager::allObjects();
            foreach (QObject *obj, all) {
                if ( auto result = qobject_cast<T *>(obj) )
                    results += result;
            }
            return results;
        }
#else
        template <typename T> static QList<T *> getObjects() {
            return ExtensionSystem::PluginManager::getObjects< T >();
        }
#endif
    };
}
