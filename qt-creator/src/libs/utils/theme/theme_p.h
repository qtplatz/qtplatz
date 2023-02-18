// Copyright (C) 2016 Thorben Kroeger <thorbenkroeger@gmail.com>.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "../utils_global.h"

#include "theme.h"

#include <QColor>
#include <QMap>

namespace Utils {

class QTCREATOR_UTILS_EXPORT ThemePrivate
{
public:
    ThemePrivate();

    QString id;
    QString fileName;
    QString displayName;
    QStringList preferredStyles;
    QString defaultTextEditorColorScheme;
    QVector<QPair<QColor, QString> > colors;
    QVector<QString> imageFiles;
    QVector<bool> flags;
    QMap<QString, QColor> palette;
};

QTCREATOR_UTILS_EXPORT void setCreatorTheme(Theme *theme);
QTCREATOR_UTILS_EXPORT void setThemeApplicationPalette();

} // namespace Utils
