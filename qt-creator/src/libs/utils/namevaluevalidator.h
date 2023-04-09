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

#pragma once

#include "utils_global.h"

#include "environmentfwd.h"

#include <QPersistentModelIndex>
#include <QTimer>
#include <QValidator>

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

namespace Utils {

class NameValueModel;

class QTCREATOR_UTILS_EXPORT NameValueValidator : public QValidator
{
    Q_OBJECT
public:
    NameValueValidator(QWidget *parent,
                       NameValueModel *model,
                       QTreeView *view,
                       const QModelIndex &index,
                       const QString &toolTipText);

    QValidator::State validate(QString &in, int &pos) const override;

    void fixup(QString &input) const override;

private:
    const QString m_toolTipText;
    NameValueModel *m_model;
    QTreeView *m_view;
    QPersistentModelIndex m_index;
    mutable QTimer m_hideTipTimer;
};
} // namespace Utils
