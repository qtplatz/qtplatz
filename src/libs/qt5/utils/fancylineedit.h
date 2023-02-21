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

#ifndef FANCYLINEEDIT_H
#define FANCYLINEEDIT_H

#include "utils_global.h"
#include "completinglineedit.h"

#include <QAbstractButton>

QT_BEGIN_NAMESPACE
class QEvent;
QT_END_NAMESPACE

namespace Utils {

class FancyLineEditPrivate;

class QTCREATOR_UTILS_EXPORT IconButton: public QAbstractButton
{
    Q_OBJECT
    Q_PROPERTY(float iconOpacity READ iconOpacity WRITE setIconOpacity)
    Q_PROPERTY(bool autoHide READ hasAutoHide WRITE setAutoHide)
    Q_PROPERTY(QPixmap pixmap READ pixmap WRITE setPixmap)
public:
    explicit IconButton(QWidget *parent = 0);
    void paintEvent(QPaintEvent *event);
    void setPixmap(const QPixmap &pixmap) { m_pixmap = pixmap; update(); }
    QPixmap pixmap() const { return m_pixmap; }
    float iconOpacity() { return m_iconOpacity; }
    void setIconOpacity(float value) { m_iconOpacity = value; update(); }
    void animateShow(bool visible);

    void setAutoHide(bool hide) { m_autoHide = hide; }
    bool hasAutoHide() const { return m_autoHide; }

    QSize sizeHint() const;

protected:
    void keyPressEvent(QKeyEvent *ke);
    void keyReleaseEvent(QKeyEvent *ke);

private:
    float m_iconOpacity;
    bool m_autoHide;
    QPixmap m_pixmap;
};

class QTCREATOR_UTILS_EXPORT FancyLineEdit : public CompletingLineEdit
{
    Q_OBJECT
    Q_ENUMS(Side)

    // Validation.
    Q_PROPERTY(QString initialText READ initialText WRITE setInitialText DESIGNABLE true)
    Q_PROPERTY(QColor errorColor READ errorColor WRITE setErrorColor DESIGNABLE true)

public:
    enum Side {Left = 0, Right = 1};

    explicit FancyLineEdit(QWidget *parent = 0);
    ~FancyLineEdit();

    QPixmap buttonPixmap(Side side) const;
    void setButtonPixmap(Side side, const QPixmap &pixmap);

    QMenu *buttonMenu(Side side) const;
    void setButtonMenu(Side side, QMenu *menu);

    void setButtonVisible(Side side, bool visible);
    bool isButtonVisible(Side side) const;
    QAbstractButton *button(Side side) const;

    void setButtonToolTip(Side side, const QString &);
    void setButtonFocusPolicy(Side side, Qt::FocusPolicy policy);

    // Set whether tabbing in will trigger the menu.
    void setMenuTabFocusTrigger(Side side, bool v);
    bool hasMenuTabFocusTrigger(Side side) const;

    // Set if icon should be hidden when text is empty
    void setAutoHideButton(Side side, bool h);
    bool hasAutoHideButton(Side side) const;


    // Completion

    // Enable a history completer with a history of entries.
    void setHistoryCompleter(const QString &historyKey);
    // Sets a completer that is not a history completer.
    void setSpecialCompleter(QCompleter *completer);


    // Filtering

    // Enables fitering
    void setFiltering(bool on);


    //  Validation

    enum State { Invalid, DisplayingInitialText, Valid };

    State state() const;
    bool isValid() const;
    QString errorMessage() const;

    QString initialText() const;
    void setInitialText(const QString &);

    QColor errorColor() const;
    void setErrorColor(const  QColor &);

    // Trigger an update (after changing settings)
    void triggerChanged();

    static QColor textColor(const QWidget *w);
    static void setTextColor(QWidget *w, const QColor &c);

protected slots:
    // Custom behaviour can be added here.
    virtual void handleChanged(const QString &) {}

signals:
    void buttonClicked(Utils::FancyLineEdit::Side side);
    void leftButtonClicked();
    void rightButtonClicked();

    void filterChanged(const QString &);

    void validChanged();
    void validChanged(bool validState);
    void validReturnPressed();

private slots:
    void iconClicked();
    void onTextChanged(const QString &);
    void onEditingFinished();

protected:
    void resizeEvent(QResizeEvent *e);

    virtual bool validate(const QString &value, QString *errorMessage) const;
    virtual QString fixInputString(const QString &string);

private:
    // Unimplemented, to force the user to make a decision on
    // whether to use setHistoryCompleter() or setSpecialCompleter().
    void setCompleter(QCompleter *);

    void updateMargins();
    void updateButtonPositions();
    friend class FancyLineEditPrivate;

    FancyLineEditPrivate *d;
};

} // namespace Utils

#endif // FANCYLINEEDIT_H
