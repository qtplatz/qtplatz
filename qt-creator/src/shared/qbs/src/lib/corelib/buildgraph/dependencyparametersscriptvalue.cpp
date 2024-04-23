/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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
#include "dependencyparametersscriptvalue.h"

#include <language/preparescriptobserver.h>
#include <language/qualifiedid.h>
#include <language/scriptengine.h>
#include <tools/stringconstants.h>

namespace qbs {
namespace Internal {

static JSValue toScriptValue(ScriptEngine *engine, const QString &productName,
                             const QVariantMap &v, const QString &depName,
                             const QualifiedId &moduleName)
{
    JSValue obj = engine->newObject();
    bool objIdAddedToObserver = false;
    for (auto it = v.begin(); it != v.end(); ++it) {
        if (it.value().userType() == QMetaType::QVariantMap) {
            setJsProperty(engine->context(), obj, it.key(),
                          toScriptValue(engine, productName, it.value().toMap(),
                                        depName, QualifiedId(moduleName) << it.key()));
        } else {
            if (!objIdAddedToObserver) {
                objIdAddedToObserver = true;
                engine->observer()->addParameterObjectId(jsObjectId(obj), productName, depName,
                                                         moduleName);
            }
            const ScopedJsValue val(engine->context(), engine->toScriptValue(it.value()));
            engine->setObservedProperty(obj, it.key(), val);
        }
    }
    return obj;
}


static JSValue toScriptValue(ScriptEngine *scriptEngine, const QString &productName,
                             const QVariantMap &v, const QString &depName)
{
    return toScriptValue(scriptEngine, productName, v, depName, {});
}

JSValue dependencyParametersValue(const QString &productName, const QString &dependencyName,
                                  const QVariantMap &parametersMap, ScriptEngine *engine)
{
    return toScriptValue(engine, productName, parametersMap, dependencyName);
}

} // namespace Internal
} // namespace qbs
