/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qbs.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QBS_SOURCELOCATION_H
#define QBS_SOURCELOCATION_H

#include "qbs_export.h"

#include <QtCore/qdebug.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE
class QDataStream;
class QJsonObject;
class QString;
QT_END_NAMESPACE

namespace qbs {
namespace Internal { class PersistentPool; }

class QBS_EXPORT CodeLocation
{
    friend QBS_EXPORT bool operator==(const CodeLocation &cl1, const CodeLocation &cl2);
public:
    CodeLocation();
    explicit CodeLocation(const QString &aFilePath, int aLine = -1, int aColumn = -1,
                          bool checkPath = true);
    CodeLocation(const CodeLocation &other);
    CodeLocation(CodeLocation &&other) noexcept;
    CodeLocation &operator=(const CodeLocation &other);
    CodeLocation &operator=(CodeLocation &&other) noexcept;
    ~CodeLocation();

    QString filePath() const;
    int line() const;
    int column() const;

    bool isValid() const;
    QString toString() const;
    QJsonObject toJson() const;

    void load(Internal::PersistentPool &pool);
    void store(Internal::PersistentPool &pool) const;

private:
    class CodeLocationPrivate;
    QExplicitlySharedDataPointer<CodeLocationPrivate> d;
};

QBS_EXPORT bool operator==(const CodeLocation &cl1, const CodeLocation &cl2);
QBS_EXPORT bool operator!=(const CodeLocation &cl1, const CodeLocation &cl2);
QBS_EXPORT bool operator<(const CodeLocation &cl1, const CodeLocation &cl2);
inline auto qHash(const CodeLocation &cl) { return qHash(cl.toString()); }
QDebug operator<<(QDebug debug, const CodeLocation &location);

class QBS_EXPORT CodePosition
{
public:
    CodePosition(int line, int column) : m_line(line), m_column(column) {}

    CodePosition() = default;
    CodePosition(const CodePosition &other) = default;
    CodePosition(CodePosition &&other) = default;
    CodePosition &operator=(const CodePosition &other) = default;
    CodePosition &operator=(CodePosition &&other) = default;

    int line() const { return m_line; }
    void setLine(int newLine) { m_line = newLine; }

    int column() const { return m_column; }
    void setColumn(int newColumn) { m_column = newColumn; }

    void load(Internal::PersistentPool &pool);
    void store(Internal::PersistentPool &pool) const;

private:
    int m_line = 0;
    int m_column = 0;
};

QBS_EXPORT bool operator==(const CodePosition &pos1, const CodePosition &pos2);
QBS_EXPORT bool operator!=(const CodePosition &pos1, const CodePosition &pos2);
QBS_EXPORT bool operator<(const CodePosition &pos1, const CodePosition &pos2);
QBS_EXPORT bool operator>(const CodePosition &pos1, const CodePosition &pos2);
QBS_EXPORT bool operator<=(const CodePosition &pos1, const CodePosition &pos2);
QBS_EXPORT bool operator>=(const CodePosition &pos1, const CodePosition &pos2);
inline auto qHash(const CodePosition &pos)
{
    return QT_PREPEND_NAMESPACE(qHash)(pos.line()) ^ QT_PREPEND_NAMESPACE(qHash)(pos.column());
}

class QBS_EXPORT CodeRange
{
public:
    CodeRange(const CodePosition &start, const CodePosition &end);

    CodeRange() = default;
    CodeRange(const CodeRange &other) = default;
    CodeRange(CodeRange &&other) = default;
    CodeRange &operator=(const CodeRange &other) = default;
    CodeRange &operator=(CodeRange &&other) = default;

    const CodePosition &start() const & { return m_start; }
    const CodePosition &end() const & { return m_end; }
    CodePosition start() && { return std::move(m_start); }
    CodePosition end() && { return std::move(m_end); }

    bool contains(const CodePosition &pos) const;

    void load(Internal::PersistentPool &pool);
    void store(Internal::PersistentPool &pool) const;

private:
    CodePosition m_start;
    CodePosition m_end;
};

QBS_EXPORT bool operator==(const CodeRange &r1, const CodeRange &r2);
QBS_EXPORT bool operator!=(const CodeRange &r1, const CodeRange &r2);
QBS_EXPORT bool operator<(const CodeRange &r1, const CodeRange &r2);
inline auto qHash(const CodeRange &range) { return qHash(range.start()) ^ qHash(range.end()); }

using CodeLinksInFile = QHash<CodeRange, QList<CodeLocation>>;
using CodeLinks = QHash<QString, CodeLinksInFile>;

} // namespace qbs

#endif // QBS_SOURCELOCATION_H
