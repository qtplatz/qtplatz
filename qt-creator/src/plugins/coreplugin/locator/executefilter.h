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

#pragma once

#include "ilocatorfilter.h"

#include <utils/commandline.h>

#include <QQueue>
#include <QStringList>
#include <QTextCodec>

namespace Utils { class QtcProcess; }

namespace Core {
namespace Internal {

class ExecuteFilter : public Core::ILocatorFilter
{
    Q_OBJECT

    struct ExecuteData
    {
        Utils::CommandLine command;
        Utils::FilePath workingDirectory;
    };

public:
    ExecuteFilter();
    ~ExecuteFilter() override;
    QList<LocatorFilterEntry> matchesFor(QFutureInterface<LocatorFilterEntry> &future,
                                         const QString &entry) override;
    void accept(const LocatorFilterEntry &selection,
                QString *newText, int *selectionStart, int *selectionLength) const override;

private:
    void done();
    void readStandardOutput();
    void readStandardError();
    void runHeadCommand();

    void createProcess();
    void removeProcess();

    void saveState(QJsonObject &object) const final;
    void restoreState(const QJsonObject &object) final;

    QString headCommand() const;

    QQueue<ExecuteData> m_taskQueue;
    QStringList m_commandHistory;
    Utils::QtcProcess *m_process = nullptr;
    QTextCodec::ConverterState m_stdoutState;
    QTextCodec::ConverterState m_stderrState;
};

} // namespace Internal
} // namespace Core
