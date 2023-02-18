// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include <QUrl>

namespace Utils {
namespace Internal {

QUrl filePathUrl(const QUrl &url);
QString normalizePathName(const QString &filePath);

} // Internal
} // Utils
