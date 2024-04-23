/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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

#ifndef QBS_JSON_HELPER_H
#define QBS_JSON_HELPER_H

#include <tools/stlutils.h>

#include <QtCore/qjsonarray.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qjsonvalue.h>
#include <QtCore/qprocess.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qvariant.h>

#include <algorithm>
#include <iterator>

namespace qbs {
namespace Internal {

template<typename T> inline T fromJson(const QJsonValue &v);
template<> inline bool fromJson(const QJsonValue &v) { return v.toBool(); }
template<> inline int fromJson(const QJsonValue &v) { return v.toInt(); }
template<> inline QString fromJson(const QJsonValue &v) { return v.toString(); }
template<> inline QStringList fromJson(const QJsonValue &v)
{
    const QJsonArray &jsonList = v.toArray();
    return transformed<QStringList>(jsonList, [](const auto &v) { return v.toString(); });
}
template<> inline QVariantMap fromJson(const QJsonValue &v) { return v.toObject().toVariantMap(); }
template<> inline QProcessEnvironment fromJson(const QJsonValue &v)
{
    const QJsonObject obj = v.toObject();
    QProcessEnvironment env;
    for (auto it = obj.begin(); it != obj.end(); ++it)
        env.insert(it.key(), it.value().toString());
    return env;
}

template<typename T> inline void setValueFromJson(T &targetValue, const QJsonObject &data,
                                                  const char *jsonProperty)
{
    const auto it = data.find(QLatin1String(jsonProperty));
    if (it != data.end())
        targetValue = fromJson<T>(*it);
}

} // namespace Internal
} // namespace qbs

#endif // Include guard
