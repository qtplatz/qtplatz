/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "quick3dnodeinstance.h"
#include "qt5nodeinstanceserver.h"
#include "qt5informationnodeinstanceserver.h"

#ifdef QUICK3D_MODULE
#include <private/qquick3dobject_p.h>
#include <private/qquick3dnode_p.h>
#include <private/qquick3dnode_p_p.h>
#include <private/qquick3drepeater_p.h>
#include <private/qquick3dloader_p.h>
#if defined(QUICK3D_ASSET_UTILS_MODULE) && QT_VERSION > QT_VERSION_CHECK(6, 2, 0)
#include <private/qquick3druntimeloader_p.h>
#endif
#endif

namespace QmlDesigner {
namespace Internal {

Quick3DNodeInstance::Quick3DNodeInstance(QObject *node)
   : Quick3DRenderableNodeInstance(node)
{
}

void Quick3DNodeInstance::invokeDummyViewCreate() const
{
    QMetaObject::invokeMethod(m_dummyRootView, "createViewForNode",
                              Q_ARG(QVariant, QVariant::fromValue(object())));
}

Quick3DNodeInstance::~Quick3DNodeInstance()
{
}

void Quick3DNodeInstance::initialize(const ObjectNodeInstance::Pointer &objectNodeInstance,
                                     InstanceContainer::NodeFlags flags)
{
#ifdef QUICK3D_MODULE
    QObject *obj = object();
    auto repObj = qobject_cast<QQuick3DRepeater *>(obj);
    auto loadObj = qobject_cast<QQuick3DLoader *>(obj);
#if defined(QUICK3D_ASSET_UTILS_MODULE) && QT_VERSION > QT_VERSION_CHECK(6, 2, 0)
    auto runLoadObj = qobject_cast<QQuick3DRuntimeLoader *>(obj);
    if (repObj || loadObj || runLoadObj) {
#else
    if (repObj || loadObj) {
#endif
        if (auto infoServer = qobject_cast<Qt5InformationNodeInstanceServer *>(nodeInstanceServer())) {
            if (repObj) {
                QObject::connect(repObj, &QQuick3DRepeater::objectAdded,
                                 infoServer, &Qt5InformationNodeInstanceServer::handleDynamicAddObject);
#if defined(QUICK3D_ASSET_UTILS_MODULE) && QT_VERSION > QT_VERSION_CHECK(6, 2, 0)
            } else if (runLoadObj) {
                QObject::connect(runLoadObj, &QQuick3DRuntimeLoader::statusChanged,
                                 infoServer, &Qt5InformationNodeInstanceServer::handleDynamicAddObject);
#endif
            } else {
                QObject::connect(loadObj, &QQuick3DLoader::loaded,
                                 infoServer, &Qt5InformationNodeInstanceServer::handleDynamicAddObject);
            }
        }
    }

    Quick3DRenderableNodeInstance::initialize(objectNodeInstance, flags);
#else
    Q_UNUSED(objectNodeInstance)
    Q_UNUSED(flags)
#endif // QUICK3D_MODULE
}

QQuick3DNode *Quick3DNodeInstance::quick3DNode() const
{
#ifdef QUICK3D_MODULE
    return qobject_cast<QQuick3DNode *>(object());
#else
    return nullptr;
#endif
}

Quick3DNodeInstance::Pointer Quick3DNodeInstance::create(QObject *object)
{
    Pointer instance(new Quick3DNodeInstance(object));
    instance->populateResetHashes();
    return instance;
}

void Quick3DNodeInstance::setHiddenInEditor(bool b)
{
    ObjectNodeInstance::setHiddenInEditor(b);
#ifdef QUICK3D_MODULE
    QQuick3DNodePrivate *privateNode = QQuick3DNodePrivate::get(quick3DNode());
    if (privateNode)
        privateNode->setIsHiddenInEditor(b);
#endif
}

} // namespace Internal
} // namespace QmlDesigner

