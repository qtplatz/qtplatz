/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qbs.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "jsextensions.h"

#include <language/scriptengine.h>
#include <tools/scripttools.h>

#include <QtCore/qmap.h>

#include <utility>

using InitializerMap = QMap<QString, void (*)(qbs::Internal::ScriptEngine *, JSValue)>;
static InitializerMap setupMap()
{
#define INITIALIZER_NAME(name) initializeJsExtension##name
#define ADD_JS_EXTENSION(name) \
    void INITIALIZER_NAME(name)(qbs::Internal::ScriptEngine *, JSValue); \
    map.insert(QStringLiteral(#name), &INITIALIZER_NAME(name))

    InitializerMap map;
    ADD_JS_EXTENSION(BinaryFile);
    ADD_JS_EXTENSION(Environment);
    ADD_JS_EXTENSION(File);
    ADD_JS_EXTENSION(FileInfo);
    ADD_JS_EXTENSION(Host);
    ADD_JS_EXTENSION(PkgConfig);
    ADD_JS_EXTENSION(Process);
    ADD_JS_EXTENSION(PropertyList);
    ADD_JS_EXTENSION(TemporaryDir);
    ADD_JS_EXTENSION(TextFile);
    ADD_JS_EXTENSION(Utilities);
    ADD_JS_EXTENSION(Xml);
    return map;
}

namespace qbs {
namespace Internal {

static InitializerMap &initializers()
{
    static InitializerMap theMap = setupMap();
    return theMap;
}

void JsExtensions::setupExtensions(ScriptEngine *engine, const QStringList &names,
                                   const JSValue &scope)
{
    for (const QString &name : names)
        initializers().value(name)(engine, scope);
}

JSValue JsExtensions::loadExtension(ScriptEngine *engine, const QString &name)
{
    if (!hasExtension(name))
        return {};

    ScopedJsValue extensionObj(engine->context(), engine->newObject());
    initializers().value(name)(engine, extensionObj);
    return getJsProperty(engine->context(), extensionObj, name);
}

bool JsExtensions::hasExtension(const QString &name)
{
    return initializers().contains(name);
}

QStringList JsExtensions::extensionNames()
{
    return initializers().keys();
}

} // namespace Internal
} // namespace qbs
