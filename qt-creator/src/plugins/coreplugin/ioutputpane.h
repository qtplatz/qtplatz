// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "core_global.h"

#include <utils/fancylineedit.h>
#include <utils/id.h>

#include <QObject>
#include <QList>
#include <QString>

QT_BEGIN_NAMESPACE
class QAction;
class QWidget;
QT_END_NAMESPACE

namespace Core {
class CommandButton;
class IContext;
class OutputWindow;

class CORE_EXPORT IOutputPane : public QObject
{
    Q_OBJECT

public:
    IOutputPane(QObject *parent = nullptr);
    ~IOutputPane() override;

    virtual QWidget *outputWidget(QWidget *parent) = 0;
    virtual QList<QWidget *> toolBarWidgets() const;
    virtual QString displayName() const = 0;
    virtual const QList<OutputWindow *> outputWindows() const { return {}; }
    virtual void ensureWindowVisible(OutputWindow *) { }

    virtual int priorityInStatusBar() const = 0;

    virtual void clearContents() = 0;
    virtual void visibilityChanged(bool visible);

    virtual void setFocus() = 0;
    virtual bool hasFocus() const = 0;
    virtual bool canFocus() const = 0;

    virtual bool canNavigate() const = 0;
    virtual bool canNext() const = 0;
    virtual bool canPrevious() const = 0;
    virtual void goToNext() = 0;
    virtual void goToPrev() = 0;

    void setFont(const QFont &font);
    void setWheelZoomEnabled(bool enabled);

    enum Flag { NoModeSwitch = 0, ModeSwitch = 1, WithFocus = 2, EnsureSizeHint = 4};
    Q_DECLARE_FLAGS(Flags, Flag)

public slots:
    void popup(int flags) { emit showPage(flags); }

    void hide() { emit hidePage(); }
    void toggle(int flags) { emit togglePage(flags); }
    void navigateStateChanged() { emit navigateStateUpdate(); }
    void flash() { emit flashButton(); }
    void setIconBadgeNumber(int number) { emit setBadgeNumber(number); }

signals:
    void showPage(int flags);
    void hidePage();
    void togglePage(int flags);
    void navigateStateUpdate();
    void flashButton();
    void setBadgeNumber(int number);
    void zoomInRequested(int range);
    void zoomOutRequested(int range);
    void resetZoomRequested();
    void wheelZoomEnabledChanged(bool enabled);
    void fontChanged(const QFont &font);

protected:
    void setupFilterUi(const QString &historyKey);
    QString filterText() const;
    bool filterUsesRegexp() const { return m_filterRegexp; }
    bool filterIsInverted() const { return m_invertFilter; }
    Qt::CaseSensitivity filterCaseSensitivity() const { return m_filterCaseSensitivity; }
    void setFilteringEnabled(bool enable);
    QWidget *filterWidget() const { return m_filterOutputLineEdit; }
    void setupContext(const char *context, QWidget *widget);
    void setZoomButtonsEnabled(bool enabled);

private:
    virtual void updateFilter();

    void filterOutputButtonClicked();
    void setCaseSensitive(bool caseSensitive);
    void setRegularExpressions(bool regularExpressions);
    Utils::Id filterRegexpActionId() const;
    Utils::Id filterCaseSensitivityActionId() const;
    Utils::Id filterInvertedActionId() const;

    Core::CommandButton * const m_zoomInButton;
    Core::CommandButton * const m_zoomOutButton;
    QAction *m_filterActionRegexp = nullptr;
    QAction *m_filterActionCaseSensitive = nullptr;
    QAction *m_invertFilterAction = nullptr;
    Utils::FancyLineEdit *m_filterOutputLineEdit = nullptr;
    IContext *m_context = nullptr;
    bool m_filterRegexp = false;
    bool m_invertFilter = false;
    Qt::CaseSensitivity m_filterCaseSensitivity = Qt::CaseInsensitive;
};

} // namespace Core

 Q_DECLARE_OPERATORS_FOR_FLAGS(Core::IOutputPane::Flags)
