// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0 WITH Qt-GPL-exception-1.0
#include "generalhelper.h"

#ifdef QUICK3D_MODULE

#include "selectionboxgeometry.h"

#include <QtQuick3D/qquick3dobject.h>
#include <QtQuick3D/private/qquick3dorthographiccamera_p.h>
#include <QtQuick3D/private/qquick3dperspectivecamera_p.h>
#include <QtQuick3D/private/qquick3dcamera_p.h>
#include <QtQuick3D/private/qquick3dnode_p.h>
#include <QtQuick3D/private/qquick3dmodel_p.h>
#include <QtQuick3D/private/qquick3dviewport_p.h>
#include <QtQuick3D/private/qquick3ddefaultmaterial_p.h>
#include <QtQuick3D/private/qquick3dscenemanager_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendercontextcore_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderbuffermanager_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendermodel_p.h>
#include <QtQuick3DUtils/private/qssgbounds3_p.h>
#include <QtQuick3DUtils/private/qssgutils_p.h>
#include <QtQml/qqml.h>
#include <QtQuick/qquickwindow.h>
#include <QtQuick/qquickitem.h>
#include <QtCore/qmath.h>

#ifdef QUICK3D_PARTICLES_MODULE
#include <QtQuick3DParticles/private/qquick3dparticlemodelshape_p.h>
#include <QtQuick3DParticles/private/qquick3dparticleemitter_p.h>
#include <QtQuick3DParticles/private/qquick3dparticletrailemitter_p.h>
#include <QtQuick3DParticles/private/qquick3dparticleattractor_p.h>
#endif

#include <limits>

namespace QmlDesigner {
namespace Internal {

const QString _globalStateId = QStringLiteral("@GTS"); // global tool state
const QString _lastSceneIdKey = QStringLiteral("lastSceneId");
const QString _rootSizeKey = QStringLiteral("rootSize");

static const float floatMin = std::numeric_limits<float>::lowest();
static const float floatMax = std::numeric_limits<float>::max();
static const QVector3D maxVec = QVector3D(floatMax, floatMax, floatMax);
static const QVector3D minVec = QVector3D(floatMin, floatMin, floatMin);

GeneralHelper::GeneralHelper()
    : QObject()
{
    m_overlayUpdateTimer.setInterval(16);
    m_overlayUpdateTimer.setSingleShot(true);
    QObject::connect(&m_overlayUpdateTimer, &QTimer::timeout,
                     this, &GeneralHelper::overlayUpdateNeeded);

    m_toolStateUpdateTimer.setSingleShot(true);
    QObject::connect(&m_toolStateUpdateTimer, &QTimer::timeout,
                     this, &GeneralHelper::handlePendingToolStateUpdate);
}

void GeneralHelper::requestOverlayUpdate()
{
    // Restart the timer on each request in attempt to ensure there's one frame between the last
    // request and actual update.
    m_overlayUpdateTimer.start();
}

QString GeneralHelper::generateUniqueName(const QString &nameRoot)
{
    static QHash<QString, int> counters;
    int count = counters[nameRoot]++;
    return QStringLiteral("%1_%2").arg(nameRoot).arg(count);
}

// Resolves absolute model source path
QUrl GeneralHelper::resolveAbsoluteSourceUrl(const QQuick3DModel *sourceModel)
{
    if (!sourceModel)
        return {};

    const QUrl source = sourceModel->source();
    if (source.hasFragment()) {
        // Fragment is part of the url separated by '#', check if it is an index or primitive
        bool isNumber = false;
        source.fragment().toInt(&isNumber);
        // If it wasn't an index, then it was a primitive and we can return it as-is
        if (!isNumber)
            return source;
    }

    QQmlContext *context = qmlContext(sourceModel);
    return context ? context->resolvedUrl(source) : source;
}

void GeneralHelper::orbitCamera(QQuick3DCamera *camera, const QVector3D &startRotation,
                                const QVector3D &lookAtPoint, const QVector3D &pressPos,
                                const QVector3D &currentPos)
{
    QVector3D dragVector = currentPos - pressPos;

    if (dragVector.length() < 0.001f)
        return;

    camera->setEulerRotation(startRotation);
    QVector3D newRotation(-dragVector.y(), -dragVector.x(), 0.f);
    newRotation *= 0.5f; // Emprically determined multiplier for nice drag
    newRotation += startRotation;

    camera->setEulerRotation(newRotation);

    const QVector3D oldLookVector = camera->position() - lookAtPoint;
    QMatrix4x4 m = camera->sceneTransform();
    const float *dataPtr(m.data());
    QVector3D newLookVector(dataPtr[8], dataPtr[9], dataPtr[10]);
    newLookVector.normalize();
    newLookVector *= oldLookVector.length();

    camera->setPosition(lookAtPoint + newLookVector);
}

// Pans camera and returns the new look-at point
QVector3D GeneralHelper::panCamera(QQuick3DCamera *camera, const QMatrix4x4 startTransform,
                                   const QVector3D &startPosition, const QVector3D &startLookAt,
                                   const QVector3D &pressPos, const QVector3D &currentPos,
                                   float zoomFactor)
{
    QVector3D dragVector = currentPos - pressPos;

    if (dragVector.length() < 0.001f)
        return startLookAt;

    const float *dataPtr(startTransform.data());
    const QVector3D xAxis = QVector3D(dataPtr[0], dataPtr[1], dataPtr[2]).normalized();
    const QVector3D yAxis = QVector3D(dataPtr[4], dataPtr[5], dataPtr[6]).normalized();
    const QVector3D xDelta = -1.f * xAxis * dragVector.x();
    const QVector3D yDelta = yAxis * dragVector.y();
    const QVector3D delta = (xDelta + yDelta) * zoomFactor;

    camera->setPosition(startPosition + delta);
    return startLookAt + delta;
}

float GeneralHelper::zoomCamera([[maybe_unused]] QQuick3DViewport *viewPort,
                                QQuick3DCamera *camera,
                                float distance,
                                float defaultLookAtDistance,
                                const QVector3D &lookAt,
                                float zoomFactor,
                                bool relative)
{
    // Emprically determined divisor for nice zoom
    float multiplier = 1.f + (distance / 40.f);
    float newZoomFactor = relative ? qBound(.01f, zoomFactor * multiplier, 100.f)
                                   : zoomFactor;

    if (qobject_cast<QQuick3DOrthographicCamera *>(camera)) {
        // Ortho camera we can simply scale
        float orthoFactor = newZoomFactor;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        if (viewPort) {
            if (const QQuickWindow *w = viewPort->window())
                orthoFactor *= w->devicePixelRatio();
        }
#endif
        camera->setScale(QVector3D(orthoFactor, orthoFactor, orthoFactor));
    } else if (qobject_cast<QQuick3DPerspectiveCamera *>(camera)) {
        // Perspective camera is zoomed by moving camera forward or backward while keeping the
        // look-at point the same
        const QVector3D lookAtVec = (camera->position() - lookAt).normalized();
        const float newDistance = defaultLookAtDistance * newZoomFactor;
        camera->setPosition(lookAt + (lookAtVec * newDistance));
    }

    return newZoomFactor;
}

// Return value contains new lookAt point (xyz) and zoom factor (w)
QVector4D GeneralHelper::focusNodesToCamera(QQuick3DCamera *camera, float defaultLookAtDistance,
                                            const QVariant &nodes, QQuick3DViewport *viewPort,
                                            float oldZoom, bool updateZoom, bool closeUp)
{
    if (!camera)
        return QVector4D(0.f, 0.f, 0.f, 1.f);

    QList<QQuick3DNode *> nodeList;
    const QVariantList varNodes = nodes.value<QVariantList>();
    for (const auto &varNode : varNodes) {
        auto model = varNode.value<QQuick3DNode *>();
        if (model)
            nodeList.append(model);
    }

    // Get bounds
    QVector3D totalMinBound;
    QVector3D totalMaxBound;
    const qreal defaultExtent = 200.;

    if (!nodeList.isEmpty()) {
        static const float floatMin = std::numeric_limits<float>::lowest();
        static const float floatMax = std::numeric_limits<float>::max();
        totalMinBound = {floatMax, floatMax, floatMax};
        totalMaxBound = {floatMin, floatMin, floatMin};
    } else {
        const float halfExtent = defaultExtent / 2.f;
        totalMinBound = {-halfExtent, -halfExtent, -halfExtent};
        totalMaxBound = {halfExtent, halfExtent, halfExtent};
    }
    for (const auto node : std::as_const(nodeList)) {
        auto model = qobject_cast<QQuick3DModel *>(node);
        qreal maxExtent = defaultExtent;
        QVector3D center = node->scenePosition();
        if (model) {
            auto targetPriv = QQuick3DObjectPrivate::get(model);
            if (auto renderModel = static_cast<QSSGRenderModel *>(targetPriv->spatialNode)) {
                QWindow *window = static_cast<QWindow *>(viewPort->window());
                if (window) {
                    QSSGRef<QSSGRenderContextInterface> context;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                    context = QSSGRenderContextInterface::getRenderContextInterface(quintptr(window));
#else
                    context = targetPriv->sceneManager->rci;
#endif
                    if (!context.isNull()) {
                        QSSGBounds3 bounds;
                        auto geometry = qobject_cast<SelectionBoxGeometry *>(model->geometry());
                        if (geometry) {
                            bounds = geometry->bounds();
                        } else {
                            auto bufferManager = context->bufferManager();
#if QT_VERSION < QT_VERSION_CHECK(6, 3, 0)
                            bounds = renderModel->getModelBounds(bufferManager);
#else
                            bounds = bufferManager->getModelBounds(renderModel);
#endif
                        }

                        center = renderModel->globalTransform.map(bounds.center());
                        const QVector3D e = bounds.extents();
                        const QVector3D s = model->sceneScale();
                        qreal maxScale = qSqrt(qreal(s.x() * s.x() + s.y() * s.y() + s.z() * s.z()));
                        maxExtent = qSqrt(qreal(e.x() * e.x() + e.y() * e.y() + e.z() * e.z()));
                        maxExtent *= maxScale;

                        if (maxExtent < 0.0001)
                            maxExtent = defaultExtent;
                    }
                }
            }
        }
        float halfExtent = float(maxExtent / 2.);
        const QVector3D halfExtents {halfExtent, halfExtent, halfExtent};

        const QVector3D minBound = center - halfExtents;
        const QVector3D maxBound = center + halfExtents;

        for (int i = 0; i < 3; ++i) {
            totalMinBound[i] = qMin(minBound[i], totalMinBound[i]);
            totalMaxBound[i] = qMax(maxBound[i], totalMaxBound[i]);
        }
    }

    QVector3D extents = totalMaxBound - totalMinBound;
    QVector3D lookAt = totalMinBound + (extents / 2.f);
    float maxExtent = qMax(extents.x(), qMax(extents.y(), extents.z()));

    // Reset camera position to default zoom
    QMatrix4x4 m = camera->sceneTransform();
    const float *dataPtr(m.data());
    QVector3D newLookVector(dataPtr[8], dataPtr[9], dataPtr[10]);
    newLookVector.normalize();
    newLookVector *= defaultLookAtDistance;

    camera->setPosition(lookAt + newLookVector);

    float divisor = closeUp ? 900.f : 725.f;

    float newZoomFactor = updateZoom ? qBound(.01f, maxExtent / divisor, 100.f) : oldZoom;
    float cameraZoomFactor = zoomCamera(viewPort, camera, 0, defaultLookAtDistance, lookAt,
                                        newZoomFactor, false);

    return QVector4D(lookAt, cameraZoomFactor);
}

// This function can be used to synchronously focus camera on a node, which doesn't have to be
// a selection box for bound calculations to work. This is used to focus the view for
// various preview image generations, where doing things asynchronously is not good
// and recalculating bounds for every frame is not a problem.
void GeneralHelper::calculateNodeBoundsAndFocusCamera(
        QQuick3DCamera *camera, QQuick3DNode *node, QQuick3DViewport *viewPort,
        float defaultLookAtDistance, bool closeUp)
{
    QVector3D minBounds;
    QVector3D maxBounds;

    getBounds(viewPort, node, minBounds, maxBounds);

    QVector3D extents = maxBounds - minBounds;
    QVector3D lookAt = minBounds + (extents / 2.f);
    float maxExtent = qSqrt(qreal(extents.x()) * qreal(extents.x())
                          + qreal(extents.y()) * qreal(extents.y())
                          + qreal(extents.z()) * qreal(extents.z()));

    // Reset camera position to default zoom
    QMatrix4x4 m = camera->sceneTransform();
    const float *dataPtr(m.data());
    QVector3D newLookVector(dataPtr[8], dataPtr[9], dataPtr[10]);
    newLookVector.normalize();
    newLookVector *= defaultLookAtDistance;

    camera->setPosition(lookAt + newLookVector);

    // CloseUp divisor is used for icon generation, where we can allow some extreme models to go
    // slightly out of bounds for better results generally. The other divisor is used for other
    // previews, where the image is larger to begin with and we would also like some margin
    // between preview edge and the rendered model, so we can be more conservative with the zoom.
    // The divisor values are empirically selected to provide nice result.
    float divisor = closeUp ? 1250.f : 1050.f;
    float newZoomFactor = maxExtent / divisor;

    zoomCamera(viewPort, camera, 0, defaultLookAtDistance, lookAt, newZoomFactor, false);

    if (auto perspectiveCamera = qobject_cast<QQuick3DPerspectiveCamera *>(camera)) {
        // Fix camera near/far clips in case we are dealing with extreme zooms
        const float cameraDist = qAbs((camera->position() - lookAt).length());
        const float minDist = cameraDist - (maxExtent / 2.f);
        const float maxDist = cameraDist + (maxExtent / 2.f);
        if (minDist < perspectiveCamera->clipNear() || maxDist > perspectiveCamera->clipFar()) {
            perspectiveCamera->setClipNear(minDist * 0.99);
            perspectiveCamera->setClipFar(maxDist * 1.01);
        }

    }
}

// Aligns any cameras found in nodes list to a camera.
// Only position and rotation are copied, rest of the camera properties stay the same.
void GeneralHelper::alignCameras(QQuick3DCamera *camera, const QVariant &nodes)
{
    QList<QQuick3DCamera *> nodeList;
    const QVariantList varNodes = nodes.value<QVariantList>();
    for (const auto &varNode : varNodes) {
        auto cameraNode = varNode.value<QQuick3DCamera *>();
        if (cameraNode)
            nodeList.append(cameraNode);
    }

    for (QQuick3DCamera *node : std::as_const(nodeList)) {
        node->setPosition(camera->position());
        node->setRotation(camera->rotation());
    }
}

// Aligns the camera to the first camera in nodes list.
// Aligning means taking the position and XY rotation from the source camera. Rest of the properties
// remain the same, as this is used to align edit cameras, which have fixed Z-rot, fov, and clips.
// The new lookAt is set at same distance away as it was previously and scale isn't adjusted, so
// the zoom factor of the edit camera stays the same.
QVector3D GeneralHelper::alignView(QQuick3DCamera *camera, const QVariant &nodes,
                                   const QVector3D &lookAtPoint)
{
    float lastDistance = (lookAtPoint - camera->position()).length();
    const QVariantList varNodes = nodes.value<QVariantList>();
    QQuick3DCamera *cameraNode = nullptr;
    for (const auto &varNode : varNodes) {
        cameraNode = varNode.value<QQuick3DCamera *>();
        if (cameraNode)
            break;
    }

    if (cameraNode) {
        camera->setPosition(cameraNode->position());
        QVector3D newRotation = cameraNode->eulerRotation();
        newRotation.setZ(0.f);
        camera->setEulerRotation(newRotation);
    }

    QVector3D lookAt = camera->position() + camera->forward() * lastDistance;

    return lookAt;
}

bool GeneralHelper::fuzzyCompare(double a, double b)
{
    return qFuzzyCompare(a, b);
}

void GeneralHelper::delayedPropertySet(QObject *obj, int delay, const QString &property,
                                       const QVariant &value)
{
    QTimer::singleShot(delay, [obj, property, value]() {
        obj->setProperty(property.toLatin1().constData(), value);
    });
}

// Returns the first valid QQuick3DPickResult from view at (posX, PosY).
QQuick3DPickResult GeneralHelper::pickViewAt(QQuick3DViewport *view, float posX, float posY)
{
    if (!view)
        return QQuick3DPickResult();

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 1)
    // Make sure global picking is on
    view->setGlobalPickingEnabled(true);

    // With Qt 6.2+, select first suitable result from all picked objects
    auto pickResults = view->pickAll(posX, posY);
    for (auto pickResult : pickResults) {
        if (isPickable(pickResult.objectHit()))
            return pickResult;
    }
#else
    // With older Qt version we'll just pick the single object
    auto pickResult = view->pick(posX, posY);
    if (isPickable(pickResult.objectHit()))
        return pickResult;
#endif
    return QQuick3DPickResult();
}

QObject *GeneralHelper::resolvePick(QQuick3DNode *pickNode)
{
    if (pickNode) {
        // Check if the picked node actually specifies another object as the pick target
        QVariant componentVar = pickNode->property("_pickTarget");
        if (componentVar.isValid()) {
            auto componentObj = componentVar.value<QObject *>();
            if (componentObj)
                return componentObj;
        }
    }
    return pickNode;
}

bool GeneralHelper::isLocked(QQuick3DNode *node) const
{
    if (node) {
        QVariant lockValue = node->property("_edit3dLocked");
        return lockValue.isValid() && lockValue.toBool();
    }
    return false;
}

bool GeneralHelper::isHidden(QQuick3DNode *node) const
{
    if (node) {
        QVariant hideValue = node->property("_edit3dHidden");
        return hideValue.isValid() && hideValue.toBool();
    }
    return false;
}

bool GeneralHelper::isPickable(QQuick3DNode *node) const
{
    if (!node)
        return false;

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    // Instancing doesn't hide child nodes, so only check for instancing on the requested node
    if (auto model = qobject_cast<QQuick3DModel *>(node)) {
        if (model->instancing())
            return false;
    }
#endif

    QQuick3DNode *n = node;
    while (n) {
        if (!n->visible() || isLocked(n) || isHidden(n))
            return false;
        n = n->parentNode();
    }
    return true;
}

// Emitter gizmo model creation is done in C++ as creating dynamic properties and
// assigning materials to dynamically created models is lot simpler in C++
QQuick3DNode *GeneralHelper::createParticleEmitterGizmoModel(QQuick3DNode *emitter,
                                                             QQuick3DMaterial *material) const
{
#ifdef QUICK3D_PARTICLES_MODULE
    if (qobject_cast<QQuick3DParticleTrailEmitter *>(emitter) || !material)
        return nullptr;

    QQuick3DParticleModelShape *shape = nullptr;
    if (auto e = qobject_cast<QQuick3DParticleEmitter *>(emitter))
        shape = qobject_cast<QQuick3DParticleModelShape *>(e->shape());
    else if (auto a = qobject_cast<QQuick3DParticleAttractor *>(emitter))
        shape = qobject_cast<QQuick3DParticleModelShape *>(a->shape());

    if (shape && shape->delegate()) {
        if (auto model = qobject_cast<QQuick3DModel *>(
                    shape->delegate()->create(shape->delegate()->creationContext()))) {
            QQmlEngine::setObjectOwnership(model, QQmlEngine::JavaScriptOwnership);
            model->setProperty("_pickTarget", QVariant::fromValue(emitter));
            QQmlListReference matRef(model, "materials");
            matRef.append(material);
            return model;
        }
    }
#endif
    return nullptr;
}

void GeneralHelper::storeToolState(const QString &sceneId, const QString &tool, const QVariant &state,
                                   int delay)
{
    if (delay > 0) {
        QVariantMap sceneToolState;
        sceneToolState.insert(tool, state);
        m_toolStatesPending.insert(sceneId, sceneToolState);
        m_toolStateUpdateTimer.start(delay);
    } else {
        if (m_toolStateUpdateTimer.isActive())
            handlePendingToolStateUpdate();
        QVariant theState;
        // Convert JS arrays to QVariantLists for easier handling down the line
        // metaType().id() which only exist in Qt6 is the same as userType()
        if (state.userType() != QMetaType::QString && state.canConvert(QMetaType::QVariantList))
            theState = state.value<QVariantList>();
        else
            theState = state;
        QVariantMap &sceneToolState = m_toolStates[sceneId];
        if (sceneToolState[tool] != theState) {
            sceneToolState.insert(tool, theState);
            emit toolStateChanged(sceneId, tool, theState);
        }
    }
}

void GeneralHelper::setSceneEnvironmentColor(const QString &sceneId, const QColor &color)
{
    m_sceneEnvironmentColor[sceneId] = color;
}

QColor GeneralHelper::sceneEnvironmentColor(const QString &sceneId) const
{
    return m_sceneEnvironmentColor[sceneId];
}

void GeneralHelper::initToolStates(const QString &sceneId, const QVariantMap &toolStates)
{
    m_toolStates[sceneId] = toolStates;
}

void GeneralHelper::enableItemUpdate(QQuickItem *item, bool enable)
{
    if (item)
        item->setFlag(QQuickItem::ItemHasContents, enable);
}

QVariantMap GeneralHelper::getToolStates(const QString &sceneId)
{
    handlePendingToolStateUpdate();
    if (m_toolStates.contains(sceneId))
        return m_toolStates[sceneId];
    return {};
}

QString GeneralHelper::globalStateId() const
{
    return _globalStateId;
}

QString GeneralHelper::lastSceneIdKey() const
{
    return _lastSceneIdKey;
}

QString GeneralHelper::rootSizeKey() const
{
    return _rootSizeKey;
}

double GeneralHelper::brightnessScaler() const
{
    // Light brightness was rescaled in Qt6 from 100 -> 1.
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    return 100.;
#else
    return 1.;
#endif
}

void GeneralHelper::setMultiSelectionTargets(QQuick3DNode *multiSelectRootNode,
                                             const QVariantList &selectedList)
{
    // Filter selection to contain only topmost parent nodes in the selection
    m_multiSelDataMap.clear();
    m_multiSelNodes.clear();
    for (auto &connection : std::as_const(m_multiSelectConnections))
        disconnect(connection);
    m_multiSelectConnections.clear();
    m_multiSelectRootNode = multiSelectRootNode;
    QSet<QQuick3DNode *> selNodes;

    for (const auto &var : selectedList) {
        QQuick3DNode *node = nullptr;
        node = var.value<QQuick3DNode *>();
        if (node)
            selNodes.insert(node);
    }
    for (const auto selNode : std::as_const(selNodes)) {
        bool found = false;
        QQuick3DNode *parent = selNode->parentNode();
        while (parent) {
            if (selNodes.contains(parent)) {
                found = true;
                break;
            }
            parent = parent->parentNode();
        }
        if (!found) {
            m_multiSelDataMap.insert(selNode, {});
            m_multiSelNodes.append(QVariant::fromValue(selNode));
            m_multiSelectConnections.append(connect(selNode, &QObject::destroyed, [this]() {
                // If any multiselected node is destroyed, assume the entire selection is invalid.
                // The new selection should be notified by creator immediately after anyway.
                m_multiSelDataMap.clear();
                m_multiSelNodes.clear();
                for (auto &connection : std::as_const(m_multiSelectConnections))
                    disconnect(connection);
                m_multiSelectConnections.clear();
            }));
            m_multiSelectConnections.append(connect(selNode, &QQuick3DNode::sceneTransformChanged,
                                                    [this]() {
                // Reposition the multiselection root node if scene transform of any multiselected
                // node changes outside of drag (i.e. changes originating from creator side)
                if (!m_blockMultiSelectionNodePositioning)
                    resetMultiSelectionNode();
            }));
        }
    }

    resetMultiSelectionNode();
    m_blockMultiSelectionNodePositioning = false;
}

void GeneralHelper::resetMultiSelectionNode()
{
    for (auto it = m_multiSelDataMap.begin(); it != m_multiSelDataMap.end(); ++it)
        it.value() = {pivotScenePosition(it.key()), it.key()->scale(),
                      it.key()->rotation(), it.key()->sceneRotation()};

    m_multiSelNodeData = {};
    if (!m_multiSelDataMap.isEmpty()) {
        for (const auto &data : std::as_const(m_multiSelDataMap))
            m_multiSelNodeData.startScenePos += data.startScenePos;
        m_multiSelNodeData.startScenePos /= m_multiSelDataMap.size();
    }
    m_multiSelectRootNode->setPosition(m_multiSelNodeData.startScenePos);
    m_multiSelectRootNode->setRotation({});
    m_multiSelectRootNode->setScale({1.f, 1.f, 1.f});
}

void GeneralHelper::restartMultiSelection()
{
    resetMultiSelectionNode();
    m_blockMultiSelectionNodePositioning = true;
}

QVariantList GeneralHelper::multiSelectionTargets() const
{
    return m_multiSelNodes;
}

void GeneralHelper::moveMultiSelection(bool commit)
{
    // Move the multiselected nodes in global space by offset from multiselection start to scenePos
    QVector3D globalOffset = m_multiSelectRootNode->scenePosition() - m_multiSelNodeData.startScenePos;
    for (auto it = m_multiSelDataMap.constBegin(); it != m_multiSelDataMap.constEnd(); ++it) {
        QVector3D newGlobalPos = it.value().startScenePos + globalOffset;
        QMatrix4x4 m;
        if (it.key()->parentNode())
            m = it.key()->parentNode()->sceneTransform();
        it.key()->setPosition(m.inverted() * newGlobalPos);
    }
    m_blockMultiSelectionNodePositioning = !commit;
}

void GeneralHelper::scaleMultiSelection(bool commit)
{
    // Offset the multiselected nodes in global space according to scale factor and scale them by
    // the same factor.

    const QVector3D sceneScale = m_multiSelectRootNode->scale();
    const QVector3D unitVector {1.f, 1.f, 1.f};
    const QVector3D diffScale = sceneScale - unitVector;

    for (auto it = m_multiSelDataMap.constBegin(); it != m_multiSelDataMap.constEnd(); ++it) {
        const QVector3D newGlobalPos = m_multiSelNodeData.startScenePos
                + (it.value().startScenePos - m_multiSelNodeData.startScenePos) * sceneScale;
        QMatrix4x4 parentMat;
        if (it.key()->parentNode())
            parentMat = it.key()->parentNode()->sceneTransform().inverted();
        it.key()->setPosition(parentMat * newGlobalPos);

        QMatrix4x4 mat;
        mat.rotate(it.value().startSceneRot);

        auto scaleDim = [&](int dim) -> QVector3D {
            QVector3D dimScale;
            float diffScaleDim = diffScale[dim];
            dimScale[dim] = diffScaleDim;
            dimScale = (mat.inverted() * dimScale).normalized() * diffScaleDim;
            for (int i = 0; i < 3; ++i)
                dimScale[i] = qAbs(dimScale[i]);
            if (sceneScale[dim] < 1.0f)
                dimScale = -dimScale;
            return dimScale;
        };

        QVector3D finalScale = scaleDim(0) + scaleDim(1) + scaleDim(2) + unitVector;

        it.key()->setScale(finalScale * it.value().startScale);
    }
    m_blockMultiSelectionNodePositioning = !commit;
}

void GeneralHelper::rotateMultiSelection(bool commit)
{
    // Rotate entire selection around the multiselection node
    const QQuaternion sceneRotation = m_multiSelectRootNode->sceneRotation();
    QVector3D rotAxis;
    float rotAngle = 0;
    sceneRotation.getAxisAndAngle(&rotAxis, &rotAngle);

    for (auto it = m_multiSelDataMap.constBegin(); it != m_multiSelDataMap.constEnd(); ++it) {
        QVector3D globalOffset = it.value().startScenePos - m_multiSelNodeData.startScenePos;
        QVector3D newGlobalPos = m_multiSelNodeData.startScenePos + sceneRotation * globalOffset;
        QMatrix4x4 parentMat;
        if (it.key()->parentNode())
            parentMat = it.key()->parentNode()->sceneTransform().inverted();
        it.key()->setPosition(parentMat * newGlobalPos);
        it.key()->setRotation(it.value().startRot);
        it.key()->rotate(rotAngle, rotAxis, QQuick3DNode::SceneSpace);
    }
    m_blockMultiSelectionNodePositioning = !commit;
}

bool GeneralHelper::isMacOS() const
{
#ifdef Q_OS_MACOS
    return true;
#else
    return false;
#endif
}

void GeneralHelper::addRotationBlocks(const QSet<QQuick3DNode *> &nodes)
{
    m_rotationBlockedNodes.unite(nodes);
    emit rotationBlocksChanged();
}

void GeneralHelper::removeRotationBlocks(const QSet<QQuick3DNode *> &nodes)
{
    for (auto node : nodes)
        m_rotationBlockedNodes.remove(node);
    emit rotationBlocksChanged();
}

bool GeneralHelper::isRotationBlocked(QQuick3DNode *node) const
{
    return m_rotationBlockedNodes.contains(node);
}

void GeneralHelper::handlePendingToolStateUpdate()
{
    m_toolStateUpdateTimer.stop();
    auto sceneIt = m_toolStatesPending.constBegin();
    while (sceneIt != m_toolStatesPending.constEnd()) {
        const QVariantMap &sceneToolState = sceneIt.value();
        auto toolIt = sceneToolState.constBegin();
        while (toolIt != sceneToolState.constEnd()) {
            storeToolState(sceneIt.key(), toolIt.key(), toolIt.value());
            ++toolIt;
        }
        ++sceneIt;
    }
    m_toolStatesPending.clear();
}

// Calculate scene position of the node's pivot point, which in practice is just the position
// of the node without applying the pivot offset.
QVector3D GeneralHelper::pivotScenePosition(QQuick3DNode *node) const
{
    if (!node)
        return {};

    QQuick3DNode *parent = node->parentNode();
    if (!parent)
        return node->position();

    QMatrix4x4 localTransform;
    localTransform.translate(node->position());

    const QMatrix4x4 sceneTransform = parent->sceneTransform() * localTransform;

    return mat44::getPosition(sceneTransform);
}

// Calculate bounds for given node, including all child nodes.
// Returns true if the tree contains at least one Model node.
bool GeneralHelper::getBounds(QQuick3DViewport *view3D, QQuick3DNode *node, QVector3D &minBounds,
                              QVector3D &maxBounds)
{
    if (!node) {
        const float halfExtent = 100.f;
        minBounds = {-halfExtent, -halfExtent, -halfExtent};
        maxBounds = {halfExtent, halfExtent, halfExtent};
        return false;
    }

    QMatrix4x4 localTransform;
    auto nodePriv = QQuick3DObjectPrivate::get(node);
    auto renderNode = static_cast<QSSGRenderNode *>(nodePriv->spatialNode);

    if (renderNode) {
#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
        if (renderNode->flags.testFlag(QSSGRenderNode::Flag::TransformDirty))
            renderNode->calculateLocalTransform();
#else
        if (renderNode->isDirty(QSSGRenderNode::DirtyFlag::TransformDirty)) {
            renderNode->localTransform = QSSGRenderNode::calculateTransformMatrix(
                        node->position(), node->scale(), node->pivot(), node->rotation());
        }
#endif
        localTransform = renderNode->localTransform;
    }

    QVector3D localMinBounds = maxVec;
    QVector3D localMaxBounds = minVec;

    // Find bounds for children
    QVector<QVector3D> minBoundsVec;
    QVector<QVector3D> maxBoundsVec;
    const auto children = node->childItems();
    bool hasModel = false;
    for (const auto child : children) {
        if (auto childNode = qobject_cast<QQuick3DNode *>(child)) {
            QVector3D newMinBounds = minBounds;
            QVector3D newMaxBounds = maxBounds;
            bool childHasModel = getBounds(view3D, childNode, newMinBounds, newMaxBounds);
            // Ignore any subtrees that do not have Model in them as we don't need those
            // for visual bounds calculations
            if (childHasModel) {
                minBoundsVec << newMinBounds;
                maxBoundsVec << newMaxBounds;
                hasModel = true;
            }
        }
    }

    auto combineMinBounds = [](QVector3D &target, const QVector3D &source) {
        target.setX(qMin(source.x(), target.x()));
        target.setY(qMin(source.y(), target.y()));
        target.setZ(qMin(source.z(), target.z()));
    };
    auto combineMaxBounds = [](QVector3D &target, const QVector3D &source) {
        target.setX(qMax(source.x(), target.x()));
        target.setY(qMax(source.y(), target.y()));
        target.setZ(qMax(source.z(), target.z()));
    };
    auto transformCorner = [&](const QMatrix4x4 &m, QVector3D &minTarget, QVector3D &maxTarget,
            const QVector3D &corner) {
        QVector3D mappedCorner = m.map(corner);
        combineMinBounds(minTarget, mappedCorner);
        combineMaxBounds(maxTarget, mappedCorner);
    };
    auto transformCorners = [&](const QMatrix4x4 &m, QVector3D &minTarget, QVector3D &maxTarget,
            const QVector3D &minCorner, const QVector3D &maxCorner) {
        transformCorner(m, minTarget, maxTarget, minCorner);
        transformCorner(m, minTarget, maxTarget, maxCorner);
        transformCorner(m, minTarget, maxTarget, QVector3D(minCorner.x(), minCorner.y(), maxCorner.z()));
        transformCorner(m, minTarget, maxTarget, QVector3D(minCorner.x(), maxCorner.y(), minCorner.z()));
        transformCorner(m, minTarget, maxTarget, QVector3D(maxCorner.x(), minCorner.y(), minCorner.z()));
        transformCorner(m, minTarget, maxTarget, QVector3D(minCorner.x(), maxCorner.y(), maxCorner.z()));
        transformCorner(m, minTarget, maxTarget, QVector3D(maxCorner.x(), maxCorner.y(), minCorner.z()));
        transformCorner(m, minTarget, maxTarget, QVector3D(maxCorner.x(), minCorner.y(), maxCorner.z()));
    };

    // Combine all child bounds
    for (const auto &newBounds : std::as_const(minBoundsVec))
        combineMinBounds(localMinBounds, newBounds);
    for (const auto &newBounds : std::as_const(maxBoundsVec))
        combineMaxBounds(localMaxBounds, newBounds);

    if (qobject_cast<QQuick3DModel *>(node)) {
        if (auto renderModel = static_cast<QSSGRenderModel *>(renderNode)) {
            QWindow *window = static_cast<QWindow *>(view3D->window());
            if (window) {
                QSSGRef<QSSGRenderContextInterface> context;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                context = QSSGRenderContextInterface::getRenderContextInterface(quintptr(window));
#else
                context = QQuick3DObjectPrivate::get(node)->sceneManager->rci;
#endif
                if (!context.isNull()) {
                    auto bufferManager = context->bufferManager();
#if QT_VERSION < QT_VERSION_CHECK(6, 3, 0)
                    QSSGBounds3 bounds = renderModel->getModelBounds(bufferManager);
#else
                    QSSGBounds3 bounds = bufferManager->getModelBounds(renderModel);
#endif
                    QVector3D center = bounds.center();
                    QVector3D extents = bounds.extents();
                    QVector3D localMin = center - extents;
                    QVector3D localMax = center + extents;

                    combineMinBounds(localMinBounds, localMin);
                    combineMaxBounds(localMaxBounds, localMax);

                    hasModel = true;
                }
            }
        }
    } else {
        combineMinBounds(localMinBounds, {});
        combineMaxBounds(localMaxBounds, {});
    }

    if (localMaxBounds == minVec) {
        localMinBounds = {};
        localMaxBounds = {};
    }

    // Transform local space bounding box to parent space
    transformCorners(localTransform, minBounds, maxBounds, localMinBounds, localMaxBounds);

    return hasModel;
}

}
}

#endif // QUICK3D_MODULE
