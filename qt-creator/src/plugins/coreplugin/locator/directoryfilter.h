// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "basefilefilter.h"

#include <coreplugin/core_global.h>

#include <QString>
#include <QByteArray>
#include <QFutureInterface>
#include <QMutex>

namespace Core {

class CORE_EXPORT DirectoryFilter : public BaseFileFilter
{
    Q_OBJECT

public:
    DirectoryFilter(Utils::Id id);
    void restoreState(const QByteArray &state) override;
    bool openConfigDialog(QWidget *parent, bool &needsRefresh) override;
    void refresh(QFutureInterface<void> &future) override;

    void setIsCustomFilter(bool value);
    void setDirectories(const Utils::FilePaths &directories);
    void addDirectory(const Utils::FilePath &directory);
    void removeDirectory(const Utils::FilePath &directory);
    Utils::FilePaths directories() const;
    void setFilters(const QStringList &filters);
    void setExclusionFilters(const QStringList &exclusionFilters);

protected:
    void saveState(QJsonObject &object) const override;
    void restoreState(const QJsonObject &object) override;

private:
    void handleAddDirectory();
    void handleEditDirectory();
    void handleRemoveDirectory();
    void updateOptionButtons();
    void updateFileIterator();

    Utils::FilePaths m_directories;
    QStringList m_filters;
    QStringList m_exclusionFilters;
    // Our config dialog, uses in addDirectory and editDirectory
    // to give their dialogs the right parent
    class DirectoryFilterOptions *m_dialog = nullptr;
    mutable QMutex m_lock;
    Utils::FilePaths m_files;
    bool m_isCustomFilter = true;
};

} // namespace Core
