/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "variablemanager.h"

#include <utils/stringutils.h>

#include <QCoreApplication>
#include <QFileInfo>
#include <QMap>
#include <QDebug>

static const char kFilePathPostfix[] = ":FilePath";
static const char kPathPostfix[] = ":Path";
static const char kFileNamePostfix[] = ":FileName";
static const char kFileBaseNamePostfix[] = ":FileBaseName";

namespace Core {

class VMMapExpander : public Utils::AbstractMacroExpander
{
public:
    virtual bool resolveMacro(const QString &name, QString *ret)
    {
        bool found;
        *ret = Core::VariableManager::value(name.toUtf8(), &found);
        return found;
    }
};

class VariableManagerPrivate
{
public:
    QHash<QByteArray, VariableManager::StringFunction> m_map;
    VMMapExpander m_macroExpander;
    QMap<QByteArray, QString> m_descriptions;
};

/*!
    \class Core::VariableManager
    \brief The VariableManager class manages \QC wide variables, that a user
    can enter into many string settings. The variables are replaced by an actual value when the string
    is used, similar to how environment variables are expanded by a shell.

    \section1 Variables

    Variable names can be basically any string without dollar sign and braces,
    though it is recommended to only use 7-bit ASCII without special characters and whitespace.

    If there are several variables that contain different aspects of the same object,
    it is convention to give them the same prefix, followed by a colon and a postfix
    that describes the aspect.
    Examples of this are \c{CurrentDocument:FilePath} and \c{CurrentDocument:Selection}.

    When the variable manager is requested to replace variables in a string, it looks for
    variable names enclosed in %{ and }, like %{CurrentDocument:FilePath}.

    Environment variables are accessible using the %{Env:...} notation.
    For example, to access the SHELL environment variable, use %{Env:SHELL}.

    \note The names of the variables are stored as QByteArray. They are typically
    7-bit-clean. In cases where this is not possible, UTF-8 encoding is
    assumed.

    \section1 Providing Variable Values

    Plugins can register variables together with a description through registerVariable().
    A typical setup is to register variables in the Plugin::initialize() function.

    \code
    bool MyPlugin::initialize(const QStringList &arguments, QString *errorString)
    {
        [...]
        VariableManager::registerVariable(
            "MyVariable",
            tr("The current value of whatever I want."));
            []() -> QString {
                QString value;
                // do whatever is necessary to retrieve the value
                [...]
                return value;
            }
        );
        [...]
    }
    \endcode


    For variables that refer to a file, you should use the convenience function
    VariableManager::registerFileVariables().
    The functions take a variable prefix, like \c MyFileVariable,
    and automatically handle standardized postfixes like \c{:FilePath},
    \c{:Path} and \c{:FileBaseName}, resulting in the combined variables, such as
    \c{MyFileVariable:FilePath}.

    \section1 Providing and Expanding Parametrized Strings

    Though it is possible to just ask the variable manager for the value of some variable in your
    code, the preferred use case is to give the user the possibility to parametrize strings, for
    example for settings.

    (If you ever think about doing the former, think twice. It is much more efficient
    to just ask the plugin that provides the variable value directly, without going through
    string conversions, and through the variable manager which will do a large scale poll. To be
    more concrete, using the example from the Providing Variable Values section: instead of
    calling \c{VariableManager::value("MyVariable")}, it is much more efficient to just ask directly
    with \c{MyPlugin::variableValue()}.)

    \section2 User Interface

    If the string that you want to parametrize is settable by the user, through a QLineEdit or
    QTextEdit derived class, you should add a variable chooser to your UI, which allows adding
    variables to the string by browsing through a list. See Core::VariableChooser for more
    details.

    \section2 Expanding Strings

    Expanding variable values in strings is done by "macro expanders".
    Utils::AbstractMacroExpander is the base class for these, and the variable manager
    provides an implementation that expands \QC variables through
    VariableManager::macroExpander().

    There are several different ways to expand a string, covering the different use cases,
    listed here sorted by relevance:
    \list
    \li Using VariableManager::expandedString(). This is the most comfortable way to get a string
        with variable values expanded, but also the least flexible one. If this is sufficient for
        you, use it.
    \li Using the Utils::expandMacros() functions. These take a string and a macro expander (for which
        you would use the one provided by the variable manager). Mostly the same as
        VariableManager::expandedString(), but also has a variant that does the replacement inline
        instead of returning a new string.
    \li Using Utils::QtcProcess::expandMacros(). This expands the string while conforming to the
        quoting rules of the platform it is run on. Use this function with the variable manager's
        macro expander if your string will be passed as a command line parameter string to an
        external command.
    \li Writing your own macro expander that nests the variable manager's macro expander. And then
        doing one of the above. This allows you to expand additional "local" variables/macros,
        that do not come from the variable manager.
    \endlist

*/

static VariableManager *variableManagerInstance = 0;
static VariableManagerPrivate *d;

/*!
 * \internal
 */
VariableManager::VariableManager()
{
    d = new VariableManagerPrivate;
    variableManagerInstance = this;
}

/*!
 * \internal
 */
VariableManager::~VariableManager()
{
    variableManagerInstance = 0;
    delete d;
}

/*!
 * Returns the value of the given \a variable. If \a found is given, it is
 * set to true if the variable has a value at all, false if not.
 */
QString VariableManager::value(const QByteArray &variable, bool *found)
{
    if (variable.startsWith("Env:")) {
        QByteArray ba = qgetenv(variable.data() + 4);
        if (found)
            *found = !ba.isNull();
        return QString::fromLocal8Bit(ba);
    }
    if (found)
        *found = d->m_map.contains(variable);
    StringFunction f = d->m_map.value(variable);
    return f ? f() : QString();
}

/*!
 * Returns \a stringWithVariables with all variables replaced by their values.
 * See the VariableManager overview documentation for other ways to expand variables.
 *
 * \sa VariableManager
 * \sa macroExpander()
 */
QString VariableManager::expandedString(const QString &stringWithVariables)
{
    return Utils::expandMacros(stringWithVariables, macroExpander());
}

/*!
 * Returns a macro expander that is used to expand all variables from the variable manager
 * in a string.
 * See the VariableManager overview documentation for other ways to expand variables.
 *
 * \sa VariableManager
 * \sa expandedString()
 */
Utils::AbstractMacroExpander *VariableManager::macroExpander()
{
    return &d->m_macroExpander;
}

/*!
 * Makes the given string-valued \a variable known to the variable manager,
 * together with a localized \a description.
 *
 * \sa registerFileVariables(), registerIntVariable()
 */
void VariableManager::registerVariable(const QByteArray &variable,
    const QString &description, const StringFunction &value)
{
    d->m_descriptions.insert(variable, description);
    d->m_map.insert(variable, value);
}

/*!
 * Makes the given integral-valued \a variable known to the variable manager,
 * together with a localized \a description.
 *
 * \sa registerVariable(), registerFileVariables()
 */
void VariableManager::registerIntVariable(const QByteArray &variable,
    const QString &description, const VariableManager::IntFunction &value)
{
    registerVariable(variable, description,
        [value]() { return QString::number(value ? value() : 0); });
}

/*!
 * Convenience function to register several variables with the same \a prefix, that have a file
 * as a value. Takes the prefix and registers variables like \c{prefix:FilePath} and
 * \c{prefix:Path}, with descriptions that start with the given \a heading.
 * For example \c{registerFileVariables("CurrentDocument", tr("Current Document"))} registers
 * variables such as \c{CurrentDocument:FilePath} with description
 * "Current Document: Full path including file name."
 */
void VariableManager::registerFileVariables(const QByteArray &prefix,
    const QString &heading, const StringFunction &base)
{
    registerVariable(prefix + kFilePathPostfix,
         QCoreApplication::translate("Core::VariableManager", "%1: Full path including file name.").arg(heading),
         [base]() -> QString { QString tmp = base(); return tmp.isEmpty() ? QString() : QFileInfo(tmp).filePath(); });

    registerVariable(prefix + kPathPostfix,
         QCoreApplication::translate("Core::VariableManager", "%1: Full path excluding file name.").arg(heading),
         [base]() -> QString { QString tmp = base(); return tmp.isEmpty() ? QString() : QFileInfo(tmp).path(); });

    registerVariable(prefix + kFileNamePostfix,
         QCoreApplication::translate("Core::VariableManager", "%1: File name without path.").arg(heading),
         [base]() -> QString { QString tmp = base(); return tmp.isEmpty() ? QString() : QFileInfo(tmp).fileName(); });

    registerVariable(prefix + kFileBaseNamePostfix,
         QCoreApplication::translate("Core::VariableManager", "%1: File base name without path and suffix.").arg(heading),
         [base]() -> QString { QString tmp = base(); return tmp.isEmpty() ? QString() : QFileInfo(tmp).baseName(); });
}

/*!
 * Returns all registered variable names.
 *
 * \sa registerVariable()
 * \sa registerFileVariables()
 */
QList<QByteArray> VariableManager::variables()
{
    return d->m_descriptions.keys();
}

/*!
 * Returns the description that was registered for the \a variable.
 */
QString VariableManager::variableDescription(const QByteArray &variable)
{
    return d->m_descriptions.value(variable);
}

} // namespace Core
