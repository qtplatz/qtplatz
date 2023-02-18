// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "utils_global.h"

#include <QList>
#include <QString>

#include <functional>

QT_BEGIN_NAMESPACE
class QJsonValue;
QT_END_NAMESPACE

namespace Utils {

// Create a usable settings key from a category,
// for example Editor|C++ -> Editor_C__
QTCREATOR_UTILS_EXPORT QString settingsKey(const QString &category);

// Return the common prefix part of a string list:
// "C:\foo\bar1" "C:\foo\bar2"  -> "C:\foo\bar"
QTCREATOR_UTILS_EXPORT QString commonPrefix(const QStringList &strings);

// Return the common path of a list of files:
// "C:\foo\bar1" "C:\foo\bar2"  -> "C:\foo"
QTCREATOR_UTILS_EXPORT QString commonPath(const QStringList &files);

// On Linux/Mac replace user's home path with ~
// Uses cleaned path and tries to use absolute path of "path" if possible
// If path is not sub of home path, or when running on Windows, returns the input
QTCREATOR_UTILS_EXPORT QString withTildeHomePath(const QString &path);

// Removes first unescaped ampersand in text
QTCREATOR_UTILS_EXPORT QString stripAccelerator(const QString &text);
// Quotes all ampersands
QTCREATOR_UTILS_EXPORT QString quoteAmpersands(const QString &text);

QTCREATOR_UTILS_EXPORT bool readMultiLineString(const QJsonValue &value, QString *out);

// Compare case insensitive and use case sensitive comparison in case of that being equal.
QTCREATOR_UTILS_EXPORT int caseFriendlyCompare(const QString &a, const QString &b);

class QTCREATOR_UTILS_EXPORT AbstractMacroExpander
{
public:
    virtual ~AbstractMacroExpander() {}
    // Not const, as it may change the state of the expander.
    //! Find an expando to replace and provide a replacement string.
    //! \param str The string to scan
    //! \param pos Position to start scan on input, found position on output
    //! \param ret Replacement string on output
    //! \return Length of string part to replace, zero if no (further) matches found
    virtual int findMacro(const QString &str, int *pos, QString *ret);
    //! Provide a replacement string for an expando
    //! \param name The name of the expando
    //! \param ret Replacement string on output
    //! \return True if the expando was found
    virtual bool resolveMacro(const QString &name, QString *ret, QSet<AbstractMacroExpander *> &seen) = 0;
private:
    bool expandNestedMacros(const QString &str, int *pos, QString *ret);
};

QTCREATOR_UTILS_EXPORT void expandMacros(QString *str, AbstractMacroExpander *mx);
QTCREATOR_UTILS_EXPORT QString expandMacros(const QString &str, AbstractMacroExpander *mx);

QTCREATOR_UTILS_EXPORT int parseUsedPortFromNetstatOutput(const QByteArray &line);

template<typename T>
T makeUniquelyNumbered(const T &preferred, const std::function<bool(const T &)> &isOk)
{
    if (isOk(preferred))
        return preferred;
    int i = 2;
    T tryName = preferred + QString::number(i);
    while (!isOk(tryName))
        tryName = preferred + QString::number(++i);
    return tryName;
}

template<typename T, typename Container>
T makeUniquelyNumbered(const T &preferred, const Container &reserved)
{
    const std::function<bool(const T &)> isOk
            = [&reserved](const T &v) { return !reserved.contains(v); };
    return makeUniquelyNumbered(preferred, isOk);
}

QTCREATOR_UTILS_EXPORT QString formatElapsedTime(qint64 elapsed);

/* This function is only necessary if you need to match the wildcard expression against a
 * string that might contain path separators - otherwise
 * QRegularExpression::wildcardToRegularExpression() can be used.
 * Working around QRegularExpression::wildcardToRegularExpression() taking native separators
 * into account and handling them to disallow matching a wildcard characters.
 */
QTCREATOR_UTILS_EXPORT QString wildcardToRegularExpression(const QString &original);

QTCREATOR_UTILS_EXPORT QString languageNameFromLanguageCode(const QString &languageCode);


#ifdef QT_WIDGETS_LIB

// Feeds the global clipboard and, when present, the primary selection
QTCREATOR_UTILS_EXPORT void setClipboardAndSelection(const QString &text);

#endif

QTCREATOR_UTILS_EXPORT QString chopIfEndsWith(QString str, QChar c);
QTCREATOR_UTILS_EXPORT QStringView chopIfEndsWith(QStringView str, QChar c);

} // namespace Utils
