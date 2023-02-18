// Copyright (C) 2020 Uwe Kindler
// SPDX-License-Identifier: LicenseRef-Qt-Commercial

#pragma once

#include <QDebug>
#include <QPair>
#include <QPixmap>
#include <QStyle>
#include <QWidget>
#include <QtCore/QtGlobal>

QT_BEGIN_NAMESPACE
class QAbstractButton;
class QSplitter;
QT_END_NAMESPACE

#if defined(ADVANCEDDOCKINGSYSTEM_LIBRARY)
#  define ADS_EXPORT Q_DECL_EXPORT
#elif defined(ADVANCEDDOCKINGSYSTEM_STATIC_LIBRARY)
#  define ADS_EXPORT
#else
#  define ADS_EXPORT Q_DECL_IMPORT
#endif

//#define ADS_DEBUG_PRINT

// Define ADS_DEBUG_PRINT to enable a lot of debug output
#ifdef ADS_DEBUG_PRINT
#define ADS_PRINT(s) qDebug() << s
#else
#define ADS_PRINT(s)
#endif

// Set ADS_DEBUG_LEVEL to enable additional debug output and to enable layout
// dumps to qDebug and std::cout after layout changes
#define ADS_DEBUG_LEVEL 0

namespace ADS {

class DockSplitter;

enum DockWidgetArea {
    NoDockWidgetArea = 0x00,
    LeftDockWidgetArea = 0x01,
    RightDockWidgetArea = 0x02,
    TopDockWidgetArea = 0x04,
    BottomDockWidgetArea = 0x08,
    CenterDockWidgetArea = 0x10,

    InvalidDockWidgetArea = NoDockWidgetArea,
    OuterDockAreas = TopDockWidgetArea | LeftDockWidgetArea | RightDockWidgetArea
                     | BottomDockWidgetArea,
    AllDockAreas = OuterDockAreas | CenterDockWidgetArea
};
Q_DECLARE_FLAGS(DockWidgetAreas, DockWidgetArea)

enum eTitleBarButton { TitleBarButtonTabsMenu, TitleBarButtonUndock, TitleBarButtonClose };

/**
 * The different dragging states
 */
enum eDragState {
    DraggingInactive,      //!< DraggingInactive
    DraggingMousePressed,  //!< DraggingMousePressed
    DraggingTab,           //!< DraggingTab
    DraggingFloatingWidget //!< DraggingFloatingWidget
};

/**
 * The different icons used in the UI
 */
enum eIcon {
    TabCloseIcon,            //!< TabCloseIcon
    DockAreaMenuIcon,        //!< DockAreaMenuIcon
    DockAreaUndockIcon,      //!< DockAreaUndockIcon
    DockAreaCloseIcon,       //!< DockAreaCloseIcon
    FloatingWidgetCloseIcon, //!< FloatingWidgetCloseIcon

    IconCount, //!< just a delimiter for range checks
};

/**
 * For bitwise combination of dock wdget features
 */
enum eBitwiseOperator
{
    BitwiseAnd,
    BitwiseOr
};

namespace internal {
const char *const closedProperty = "close";
const char *const dirtyProperty = "dirty";

/**
 * Replace the from widget in the given splitter with the To widget
 */
void replaceSplitterWidget(QSplitter *splitter, QWidget *from, QWidget *to);

/**
 * This function walks the splitter tree upwards to hides all splitters
 * that do not have visible content
 */
void hideEmptyParentSplitters(DockSplitter *firstParentSplitter);

/**
 * Convenience class for QPair to provide better naming than first and
 * second
 */
class DockInsertParam : public QPair<Qt::Orientation, bool>
{
public:
    DockInsertParam(Qt::Orientation orientation, bool append)
        : QPair<Qt::Orientation, bool>(orientation, append)
    {}

    Qt::Orientation orientation() const { return this->first; }
    bool append() const { return this->second; }
    int insertOffset() const { return append() ? 1 : 0; }
};

/**
 * Returns the insertion parameters for the given dock area
 */
DockInsertParam dockAreaInsertParameters(DockWidgetArea area);

/**
 * Searches for the parent widget of the given type.
 * Returns the parent widget of the given widget or 0 if the widget is not
 * child of any widget of type T
 *
 * It is not safe to use this function in in DockWidget because only
 * the current dock widget has a parent. All dock widgets that are not the
 * current dock widget in a dock area have no parent.
 */
template<class T>
T findParent(const QWidget *widget)
{
    QWidget *parentWidget = widget->parentWidget();
    while (parentWidget) {
        T parentImpl = qobject_cast<T>(parentWidget);
        if (parentImpl) {
            return parentImpl;
        }
        parentWidget = parentWidget->parentWidget();
    }
    return 0;
}

/**
 * Creates a semi transparent pixmap from the given pixmap Source.
 * The Opacity parameter defines the opacity from completely transparent (0.0)
 * to completely opaque (1.0)
 */
QPixmap createTransparentPixmap(const QPixmap &source, qreal opacity);

/**
 * Helper function for settings flags in a QFlags instance.
 */
template<class T>
void setFlag(T &flags, typename T::enum_type flag, bool on = true)
{
    flags.setFlag(flag, on);
}

/**
 * Helper function for settings tooltips without cluttering the code with
 * tests for preprocessor macros
 */
template <class QObjectPtr>
void setToolTip(QObjectPtr obj, const QString &tip)
{
#ifndef QT_NO_TOOLTIP
    obj->setToolTip(tip);
#else
    Q_UNUSED(obj);
    Q_UNUSED(tip);
#endif
}

/**
 * Helper function to set the icon of a certain button.
 * Use this function to set the icons for the dock area and dock widget buttons.
 * The function first uses the CustomIconId to get an icon from the
 * IconProvider. You can register your custom icons with the icon provider, if
 * you do not want to use the default buttons and if you do not want to use
 * stylesheets.
 * If the IconProvider does not return a valid icon (icon is null), the function
 * fetches the given standard pixmap from the QStyle.
 * param[in] Button The button whose icons are to be set
 * param[in] StandardPixmap The standard pixmap to be used for the button
 * param[in] CustomIconId The identifier for the custom icon.
 */
void setButtonIcon(QAbstractButton *button, QStyle::StandardPixmap standarPixmap,
    ADS::eIcon CustomIconId);

enum eRepolishChildOptions
{
    RepolishIgnoreChildren,
    RepolishDirectChildren,
    RepolishChildrenRecursively
};

/**
 * Calls unpolish() / polish for the style of the given widget to update
 * stylesheet if a property changes
 */
void repolishStyle(QWidget *widget, eRepolishChildOptions options = RepolishIgnoreChildren);

} // namespace internal
} // namespace ADS
