// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#ifdef QUICK3D_MODULE

#include <QColor>
#include <QHash>
#include <QMatrix4x4>
#include <QObject>
#include <QPointer>
#include <QQuaternion>
#include <QTimer>
#include <QUrl>
#include <QVariant>
#include <QVector3D>
#include <QtQuick3D/private/qquick3dpickresult_p.h>

QT_BEGIN_NAMESPACE
class QQuick3DCamera;
class QQuick3DNode;
class QQuick3DViewport;
class QQuick3DMaterial;
class QQuickItem;
QT_END_NAMESPACE

namespace QmlDesigner {
namespace Internal {

class GeneralHelper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isMacOS READ isMacOS CONSTANT)

public:
    GeneralHelper();

    Q_INVOKABLE void requestOverlayUpdate();
    Q_INVOKABLE QString generateUniqueName(const QString &nameRoot);
    Q_INVOKABLE QUrl resolveAbsoluteSourceUrl(const QQuick3DModel *sourceModel);

    Q_INVOKABLE void orbitCamera(QQuick3DCamera *camera, const QVector3D &startRotation,
                                 const QVector3D &lookAtPoint, const QVector3D &pressPos,
                                 const QVector3D &currentPos);
    Q_INVOKABLE QVector3D panCamera(QQuick3DCamera *camera, const QMatrix4x4 startTransform,
                                    const QVector3D &startPosition, const QVector3D &startLookAt,
                                    const QVector3D &pressPos, const QVector3D &currentPos,
                                    float zoomFactor);
    Q_INVOKABLE float zoomCamera(QQuick3DViewport *viewPort, QQuick3DCamera *camera, float distance,
                                 float defaultLookAtDistance, const QVector3D &lookAt,
                                 float zoomFactor, bool relative);
    Q_INVOKABLE QVector4D focusNodesToCamera(QQuick3DCamera *camera, float defaultLookAtDistance,
                                             const QVariant &nodes, QQuick3DViewport *viewPort,
                                             float oldZoom, bool updateZoom = true,
                                             bool closeUp = false);
    Q_INVOKABLE void calculateNodeBoundsAndFocusCamera(QQuick3DCamera *camera, QQuick3DNode *node,
                                                       QQuick3DViewport *viewPort,
                                                       float defaultLookAtDistance, bool closeUp);
    Q_INVOKABLE void alignCameras(QQuick3DCamera *camera, const QVariant &nodes);
    Q_INVOKABLE QVector3D alignView(QQuick3DCamera *camera, const QVariant &nodes,
                                    const QVector3D &lookAtPoint);
    Q_INVOKABLE bool fuzzyCompare(double a, double b);
    Q_INVOKABLE void delayedPropertySet(QObject *obj, int delay, const QString &property,
                                        const QVariant& value);
    Q_INVOKABLE QQuick3DPickResult pickViewAt(QQuick3DViewport *view, float posX, float posY);
    Q_INVOKABLE QObject *resolvePick(QQuick3DNode *pickNode);

    Q_INVOKABLE bool isLocked(QQuick3DNode *node) const;
    Q_INVOKABLE bool isHidden(QQuick3DNode *node) const;
    Q_INVOKABLE bool isPickable(QQuick3DNode *node) const;
    Q_INVOKABLE QQuick3DNode *createParticleEmitterGizmoModel(QQuick3DNode *emitter,
                                                              QQuick3DMaterial *material) const;

    Q_INVOKABLE void storeToolState(const QString &sceneId, const QString &tool,
                                    const QVariant &state, int delayEmit = 0);
    void initToolStates(const QString &sceneId, const QVariantMap &toolStates);
    Q_INVOKABLE void enableItemUpdate(QQuickItem *item, bool enable);
    Q_INVOKABLE QVariantMap getToolStates(const QString &sceneId);
    QString globalStateId() const;
    QString lastSceneIdKey() const;
    QString rootSizeKey() const;

    Q_INVOKABLE double brightnessScaler() const;

    Q_INVOKABLE void setMultiSelectionTargets(QQuick3DNode *multiSelectRootNode,
                                              const QVariantList &selectedList);
    Q_INVOKABLE void resetMultiSelectionNode();
    Q_INVOKABLE void restartMultiSelection();
    Q_INVOKABLE QVariantList multiSelectionTargets() const;
    Q_INVOKABLE void moveMultiSelection(bool commit);
    Q_INVOKABLE void scaleMultiSelection(bool commit);
    Q_INVOKABLE void rotateMultiSelection(bool commit);

    void setSceneEnvironmentColor(const QString &sceneId, const QColor &color);
    Q_INVOKABLE QColor sceneEnvironmentColor(const QString &sceneId) const;

    bool isMacOS() const;

    void addRotationBlocks(const QSet<QQuick3DNode *> &nodes);
    void removeRotationBlocks(const QSet<QQuick3DNode *> &nodes);
    Q_INVOKABLE bool isRotationBlocked(QQuick3DNode *node) const;

signals:
    void overlayUpdateNeeded();
    void toolStateChanged(const QString &sceneId, const QString &tool, const QVariant &toolState);
    void hiddenStateChanged(QQuick3DNode *node);
    void lockedStateChanged(QQuick3DNode *node);
    void rotationBlocksChanged();

private:
    void handlePendingToolStateUpdate();
    QVector3D pivotScenePosition(QQuick3DNode *node) const;
    bool getBounds(QQuick3DViewport *view3D, QQuick3DNode *node, QVector3D &minBounds,
                   QVector3D &maxBounds);

    QTimer m_overlayUpdateTimer;
    QTimer m_toolStateUpdateTimer;
    QHash<QString, QVariantMap> m_toolStates;
    QHash<QString, QVariantMap> m_toolStatesPending;
    QHash<QString, QColor> m_sceneEnvironmentColor;
    QSet<QQuick3DNode *> m_rotationBlockedNodes;

    struct MultiSelData {
        QVector3D startScenePos;
        QVector3D startScale;
        QQuaternion startRot;
        QQuaternion startSceneRot;
    };

    QHash<QQuick3DNode *, MultiSelData> m_multiSelDataMap;
    QVariantList m_multiSelNodes;
    MultiSelData m_multiSelNodeData;
    QQuick3DNode *m_multiSelectRootNode = nullptr;
    QList<QMetaObject::Connection> m_multiSelectConnections;
    bool m_blockMultiSelectionNodePositioning = false;
};

}
}

#endif // QUICK3D_MODULE
