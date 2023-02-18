// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "utils_global.h"

#include <QDialog>

QT_BEGIN_NAMESPACE
class QCheckBox;
QT_END_NAMESPACE

namespace Utils {

class FilePath;

class QTCREATOR_UTILS_EXPORT RemoveFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RemoveFileDialog(const FilePath &filePath, QWidget *parent = nullptr);
    ~RemoveFileDialog() override;

    void setDeleteFileVisible(bool visible);
    bool isDeleteFileChecked() const;

private:
    QCheckBox *m_deleteFileCheckBox;
};

} // namespace Utils
