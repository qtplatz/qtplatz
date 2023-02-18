// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include <QtGlobal>
#include <QtQuick/qquickwindow.h>

#include "nodeinstanceserver.h"

QT_BEGIN_NAMESPACE
class QQuickItem;
class QQmlEngine;
class QQuickDesignerSupport;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
class QQuickRenderControl;
class QRhi;
class QRhiTexture;
class QRhiRenderBuffer;
class QRhiTextureRenderTarget;
class QRhiRenderPassDescriptor;
#endif
QT_END_NAMESPACE

namespace QmlDesigner {

class Qt5NodeInstanceServer : public NodeInstanceServer
{
    Q_OBJECT
public:
    Qt5NodeInstanceServer(NodeInstanceClientInterface *nodeInstanceClient);
    ~Qt5NodeInstanceServer() override;

    QQuickView *quickView() const override;
    QQuickWindow *quickWindow() const override;
    QQmlView *declarativeView() const override;
    QQuickItem *rootItem() const override;
    void setRootItem(QQuickItem *item) override;

    QQmlEngine *engine() const override;
    void refreshBindings() override;

    QQuickDesignerSupport *designerSupport();

    void createScene(const CreateSceneCommand &command) override;
    void clearScene(const ClearSceneCommand &command) override;
    void reparentInstances(const ReparentInstancesCommand &command) override;

    QImage grabWindow() override;
    QImage grabItem(QQuickItem *item) override;
    bool renderWindow() override;

    static QQuickItem *parentEffectItem(QQuickItem *item);

protected:
    void initializeView() override;
    void resizeCanvasToRootItem() override;
    void resetAllItems();
    void setupScene(const CreateSceneCommand &command) override;
    QList<QQuickItem*> allItems() const;
    bool rootIsRenderable3DObject() const;

    struct RenderViewData {
        QPointer<QQuickWindow> window = nullptr;
        QQuickItem *rootItem = nullptr;
        QQuickItem *contentItem = nullptr;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        bool bufferDirty = true;
        QQuickRenderControl *renderControl = nullptr;
        QRhi *rhi = nullptr;
        QRhiTexture *texture = nullptr;
        QRhiRenderBuffer *buffer = nullptr;
        QRhiTextureRenderTarget *texTarget = nullptr;
        QRhiRenderPassDescriptor *rpDesc = nullptr;
#endif
    };

    virtual bool initRhi(RenderViewData &viewData);
    virtual QImage grabRenderControl(RenderViewData &viewData);

private:
    RenderViewData m_viewData;
    std::unique_ptr<QQuickDesignerSupport> m_designerSupport;
    QQmlEngine *m_qmlEngine = nullptr;
};

} // QmlDesigner
