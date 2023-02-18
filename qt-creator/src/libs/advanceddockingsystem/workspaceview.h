// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial

#pragma once

#include "workspacemodel.h"

#include <utils/itemviews.h>

#include <QAbstractTableModel>

namespace ADS {

class DockManager;
class WorkspaceDialog;

class WorkspaceView : public Utils::TreeView
{
    Q_OBJECT

public:
    explicit WorkspaceView(QWidget *parent = nullptr);

    void createNewWorkspace();
    void deleteSelectedWorkspaces();
    void cloneCurrentWorkspace();
    void renameCurrentWorkspace();
    void resetCurrentWorkspace();
    void switchToCurrentWorkspace();

    void importWorkspace();
    void exportCurrentWorkspace();

    QString currentWorkspace();
    WorkspaceModel *workspaceModel();
    void selectActiveWorkspace();
    void selectWorkspace(const QString &workspaceName);

    QStringList selectedWorkspaces() const;

signals:
    void workspaceActivated(const QString &workspace);
    void workspacesSelected(const QStringList &workspaces);
    void workspaceSwitched();

private:
    void showEvent(QShowEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    void deleteWorkspaces(const QStringList &workspaces);

    static WorkspaceDialog *castToWorkspaceDialog(QWidget *widget);

    DockManager *m_manager;
    WorkspaceModel m_workspaceModel;
};

} // namespace ADS
