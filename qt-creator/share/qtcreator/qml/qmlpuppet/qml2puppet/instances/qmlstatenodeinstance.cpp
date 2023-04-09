/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "qmlstatenodeinstance.h"

#include "qmlpropertychangesnodeinstance.h"

#include <qmlprivategate.h>
#include <designersupportdelegate.h>

namespace QmlDesigner {
namespace Internal {

/**
  \class QmlStateNodeInstance

  QmlStateNodeInstance manages a QQuickState object.
  */

QmlStateNodeInstance::QmlStateNodeInstance(QObject *object) :
        ObjectNodeInstance(object)
{
}

QmlStateNodeInstance::Pointer
        QmlStateNodeInstance::create(QObject *object)
{
    Pointer instance(new QmlStateNodeInstance(object));

    instance->populateResetHashes();

    return instance;
}

void setAllNodesDirtyRecursive(QQuickItem *parentItem)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    Q_UNUSED(parentItem)
#else
    if (!parentItem)
        return;
    const QList<QQuickItem *> children = parentItem->childItems();
    for (QQuickItem *childItem : children)
        setAllNodesDirtyRecursive(childItem);
    DesignerSupport::addDirty(parentItem, QQuickDesignerSupport::Content);
#endif
}

void QmlStateNodeInstance::activateState()
{
    if (!QmlPrivateGate::States::isStateActive(object(), context())
            && nodeInstanceServer()->hasInstanceForObject(object())) {
        nodeInstanceServer()->setStateInstance(nodeInstanceServer()->instanceForObject(object()));
        QmlPrivateGate::States::activateState(object(), context());

        setAllNodesDirtyRecursive(nodeInstanceServer()->rootItem());
    }
}

void QmlStateNodeInstance::deactivateState()
{
    if (QmlPrivateGate::States::isStateActive(object(), context())) {
        nodeInstanceServer()->clearStateInstance();
        QmlPrivateGate::States::deactivateState(object());
    }
}

void QmlStateNodeInstance::setPropertyVariant(const PropertyName &name, const QVariant &value)
{
    if (name == "when")
        return;

    ObjectNodeInstance::setPropertyVariant(name, value);
}

void QmlStateNodeInstance::setPropertyBinding(const PropertyName &name, const QString &expression)
{
    if (name == "when")
        return;

    ObjectNodeInstance::setPropertyBinding(name, expression);
}

bool QmlStateNodeInstance::updateStateVariant(const ObjectNodeInstance::Pointer &target, const PropertyName &propertyName, const QVariant &value)
{
    return QmlPrivateGate::States::changeValueInRevertList(object(), target->object(), propertyName, value);
}

bool QmlStateNodeInstance::updateStateBinding(const ObjectNodeInstance::Pointer &target, const PropertyName &propertyName, const QString &expression)
{
    return QmlPrivateGate::States::updateStateBinding(object(), target->object(), propertyName, expression);
}

bool QmlStateNodeInstance::resetStateProperty(const ObjectNodeInstance::Pointer &target, const PropertyName &propertyName, const QVariant & resetValue)
{
    return QmlPrivateGate::States::resetStateProperty(object(), target->object(), propertyName, resetValue);
}

void QmlStateNodeInstance::reparent(const ObjectNodeInstance::Pointer &oldParentInstance,
                                    const PropertyName &oldParentProperty,
                                    const ObjectNodeInstance::Pointer &newParentInstance,
                                    const PropertyName &newParentProperty)
{
    ServerNodeInstance oldState = nodeInstanceServer()->activeStateInstance();

    ObjectNodeInstance::reparent(oldParentInstance,
                                 oldParentProperty,
                                 newParentInstance,
                                 newParentProperty);

    if (oldState.isValid())
        oldState.activateState();
}

} // namespace Internal
} // namespace QmlDesigner
