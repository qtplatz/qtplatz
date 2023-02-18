// Copyright (C) 2016 The Qt Company Ltd.
// Copyright (C) 2016 Falko Arps
// Copyright (C) 2016 Sven Klein
// Copyright (C) 2016 Giuliano Schneider
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "ioptionspage.h"

#include <coreplugin/icore.h>

#include <utils/aspects.h>
#include <utils/qtcassert.h>
#include <utils/stringutils.h>

#include <QCheckBox>
#include <QGroupBox>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QRegularExpression>

using namespace Utils;

namespace Core {

/*!
    \class Core::IOptionsPageProvider
    \inmodule QtCreator
    \internal
*/
/*!
    \class Core::IOptionsPageWidget
    \inmodule QtCreator
    \internal
*/

/*!
    \class Core::IOptionsPage
    \inheaderfile coreplugin/dialogs/ioptionspage.h
    \ingroup mainclasses
    \inmodule QtCreator

    \brief The IOptionsPage class is an interface for providing pages for the
    \uicontrol Options dialog (called \uicontrol Preferences on \macos).

    \image qtcreator-options-dialog.png
*/

/*!

    \fn Utils::Id Core::IOptionsPage::id() const

    Returns a unique identifier for referencing the options page.
*/

/*!
    \fn QString Core::IOptionsPage::displayName() const

    Returns the translated display name of the options page.
*/

/*!
    \fn Utils::Id Core::IOptionsPage::category() const

    Returns the unique id for the category that the options page should be displayed in. This id is
    used for sorting the list on the left side of the \uicontrol Options dialog.
*/

/*!
    \fn QString Core::IOptionsPage::displayCategory() const

    Returns the translated category name of the options page. This name is displayed in the list on
    the left side of the \uicontrol Options dialog.
*/

/*!
    Returns the category icon of the options page. This icon is displayed in the list on the left
    side of the \uicontrol Options dialog.
*/
QIcon IOptionsPage::categoryIcon() const
{
    return m_categoryIcon.icon();
}

/*!
    Sets the \a widgetCreator callback to create page widgets on demand. The
    widget will be destroyed on finish().
 */
void IOptionsPage::setWidgetCreator(const WidgetCreator &widgetCreator)
{
    m_widgetCreator = widgetCreator;
}

/*!
    Returns the widget to show in the \uicontrol Options dialog. You should create a widget lazily here,
    and delete it again in the finish() method. This method can be called multiple times, so you
    should only create a new widget if the old one was deleted.

    Alternatively, use setWidgetCreator() to set a callback function that is used to
    lazily create a widget in time.

    Either override this function in a derived class, or set a widget creator.
*/

QWidget *IOptionsPage::widget()
{
    if (!m_widget) {
        if (m_widgetCreator) {
            m_widget = m_widgetCreator();
        } else if (m_layouter) {
            m_widget = new QWidget;
            m_layouter(m_widget);
        } else {
            QTC_CHECK(false);
        }
    }
    return m_widget;
}

/*!
    Called when selecting the \uicontrol Apply button on the options page dialog.
    Should detect whether any changes were made and store those.

    Either override this function in a derived class, or set a widget creator.

    \sa setWidgetCreator()
*/

void IOptionsPage::apply()
{
    if (auto widget = qobject_cast<IOptionsPageWidget *>(m_widget)) {
        widget->apply();
    } else if (m_settings) {
        if (m_settings->isDirty()) {
            m_settings->apply();
            m_settings->writeSettings(ICore::settings());
         }
    }
}

/*!
    Called directly before the \uicontrol Options dialog closes. Here you should
    delete the widget that was created in widget() to free resources.

    Either override this function in a derived class, or set a widget creator.

    \sa setWidgetCreator()
*/

void IOptionsPage::finish()
{
    if (auto widget = qobject_cast<IOptionsPageWidget *>(m_widget))
        widget->finish();
    else if (m_settings)
        m_settings->finish();

    delete m_widget;
}

/*!
    Sets \a categoryIconPath as the path to the category icon of the options
    page.
*/
void IOptionsPage::setCategoryIconPath(const FilePath &categoryIconPath)
{
    m_categoryIcon = Icon({{categoryIconPath, Theme::PanelTextColorDark}}, Icon::Tint);
}

void IOptionsPage::setSettings(AspectContainer *settings)
{
    m_settings = settings;
}

void IOptionsPage::setLayouter(const std::function<void(QWidget *w)> &layouter)
{
    m_layouter = layouter;
}

/*!
    \fn void Core::IOptionsPage::setId(Utils::Id id)

    Sets the \a id of the options page.
*/

/*!
    \fn void Core::IOptionsPage::setDisplayName(const QString &displayName)

    Sets \a displayName as the display name of the options page.
*/

/*!
    \fn void Core::IOptionsPage::setCategory(Utils::Id category)

    Uses \a category to sort the options pages.
*/

/*!
    \fn void Core::IOptionsPage::setDisplayCategory(const QString &displayCategory)

    Sets \a displayCategory as the display category of the options page.
*/

/*!
    \fn void Core::IOptionsPage::setCategoryIcon(const Utils::Icon &categoryIcon)

    Sets \a categoryIcon as the category icon of the options page.
*/

static QList<IOptionsPage *> g_optionsPages;

/*!
    Constructs an options page with the given \a parent and registers it
    at the global options page pool if \a registerGlobally is \c true.
*/
IOptionsPage::IOptionsPage(QObject *parent, bool registerGlobally)
    : QObject(parent)
{
    if (registerGlobally)
        g_optionsPages.append(this);
}

/*!
    \internal
 */
IOptionsPage::~IOptionsPage()
{
    g_optionsPages.removeOne(this);
}

/*!
    Returns a list of all options pages.
 */
const QList<IOptionsPage *> IOptionsPage::allOptionsPages()
{
    return g_optionsPages;
}

/*!
    Is used by the \uicontrol Options dialog search filter to match \a regexp to this options
    page. This defaults to take the widget and then looks for all child labels, check boxes, push
    buttons, and group boxes. Should return \c true when a match is found.
*/
bool IOptionsPage::matches(const QRegularExpression &regexp) const
{
    if (!m_keywordsInitialized) {
        auto that = const_cast<IOptionsPage *>(this);
        QWidget *widget = that->widget();
        if (!widget)
            return false;
        // find common subwidgets
        for (const QLabel *label : widget->findChildren<QLabel *>())
            m_keywords << Utils::stripAccelerator(label->text());
        for (const QCheckBox *checkbox : widget->findChildren<QCheckBox *>())
            m_keywords << Utils::stripAccelerator(checkbox->text());
        for (const QPushButton *pushButton : widget->findChildren<QPushButton *>())
            m_keywords << Utils::stripAccelerator(pushButton->text());
        for (const QGroupBox *groupBox : widget->findChildren<QGroupBox *>())
            m_keywords << Utils::stripAccelerator(groupBox->title());

        m_keywordsInitialized = true;
    }
    for (const QString &keyword : std::as_const(m_keywords))
        if (keyword.contains(regexp))
            return true;
    return false;
}

static QList<IOptionsPageProvider *> g_optionsPagesProviders;

IOptionsPageProvider::IOptionsPageProvider(QObject *parent)
    : QObject(parent)
{
    g_optionsPagesProviders.append(this);
}

IOptionsPageProvider::~IOptionsPageProvider()
{
    g_optionsPagesProviders.removeOne(this);
}

const QList<IOptionsPageProvider *> IOptionsPageProvider::allOptionsPagesProviders()
{
    return g_optionsPagesProviders;
}

QIcon IOptionsPageProvider::categoryIcon() const
{
    return m_categoryIcon.icon();
}

} // Core
