// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "checkablemessagebox.h"

#include "hostosinfo.h"
#include "qtcassert.h"
#include "qtcsettings.h"
#include "utilstr.h"

#include <QApplication>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPointer>
#include <QPushButton>
#include <QStyle>
#include <QTextEdit>

/*!
    \class Utils::CheckableMessageBox
    \inmodule QtCreator

    \brief The CheckableMessageBox class implements a message box suitable for
    questions with a \uicontrol {Do not ask again} or \uicontrol {Do not show again}
    checkbox.

    Emulates the QMessageBox API with
    static conveniences. The message label can open external URLs.
*/

static const char kDoNotAskAgainKey[] = "DoNotAskAgain";

namespace Utils {

static QtcSettings *theSettings;

static void prepare(QMessageBox::Icon icon,
                    const QString &title,
                    const QString &text,
                    const CheckableDecider &decider,
                    QMessageBox::StandardButtons buttons,
                    QMessageBox::StandardButton defaultButton,
                    QMap<QMessageBox::StandardButton, QString> buttonTextOverrides,
                    const QString &msg,
                    QMessageBox &msgBox)
{
    msgBox.setWindowTitle(title);
    msgBox.setIcon(icon);
    msgBox.setText(text);
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setTextInteractionFlags(Qt::LinksAccessibleByKeyboard | Qt::LinksAccessibleByMouse);

#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    if (HostOsInfo::isMacHost()) {
        // Message boxes on macOS cannot display links.
        // If the message contains a link, we need to disable native dialogs.
        if (text.contains("<a "))
            msgBox.setOptions(QMessageBox::Option::DontUseNativeDialog);

            // Workaround for QTBUG-118241, fixed in Qt 6.6.1
#if QT_VERSION < QT_VERSION_CHECK(6, 6, 1)
        if (!buttonTextOverrides.isEmpty())
            msgBox.setOptions(QMessageBox::Option::DontUseNativeDialog);
#endif // QT_VERSION < QT_VERSION_CHECK(6, 6, 1)
    }
#endif

    if (decider.shouldAskAgain) {
        msgBox.setCheckBox(new QCheckBox);
        msgBox.checkBox()->setChecked(false);
        msgBox.checkBox()->setText(msg);
    }

    msgBox.setStandardButtons(buttons);
    msgBox.setDefaultButton(defaultButton);
    for (auto it = buttonTextOverrides.constBegin(); it != buttonTextOverrides.constEnd(); ++it)
        msgBox.button(it.key())->setText(it.value());
}

static QMessageBox::StandardButton exec(
    QWidget *parent,
    QMessageBox::Icon icon,
    const QString &title,
    const QString &text,
    const CheckableDecider &decider,
    QMessageBox::StandardButtons buttons,
    QMessageBox::StandardButton defaultButton,
    QMessageBox::StandardButton acceptButton,
    QMap<QMessageBox::StandardButton, QString> buttonTextOverrides,
    const QString &msg)
{
    if (decider.shouldAskAgain) {
        if (!decider.shouldAskAgain())
            return acceptButton;
    }

    QMessageBox msgBox(parent);
    prepare(icon, title, text, decider, buttons, defaultButton, buttonTextOverrides, msg, msgBox);
    msgBox.exec();

    QMessageBox::StandardButton clickedBtn = msgBox.standardButton(msgBox.clickedButton());

    if (decider.doNotAskAgain && msgBox.checkBox()->isChecked()
        && (acceptButton == QMessageBox::NoButton || clickedBtn == acceptButton))
        decider.doNotAskAgain();
    return clickedBtn;
}

static void show(QWidget *parent,
                 QMessageBox::Icon icon,
                 const QString &title,
                 const QString &text,
                 CheckableDecider decider,
                 QMessageBox::StandardButtons buttons,
                 QMessageBox::StandardButton defaultButton,
                 QMessageBox::StandardButton acceptButton,
                 QMap<QMessageBox::StandardButton, QString> buttonTextOverrides,
                 const QString &msg,
                 QObject *guard,
                 const std::function<void(QMessageBox::StandardButton choosenBtn)> &callback)
{
    if (decider.shouldAskAgain && !decider.shouldAskAgain()) {
        if (callback) {
            QMetaObject::invokeMethod(
                guard ? guard : qApp,
                [callback, acceptButton] { callback(acceptButton); },
                Qt::QueuedConnection);
        }

        return;
    }

    QMessageBox *msgBox = new QMessageBox(parent);
    prepare(icon, title, text, decider, buttons, defaultButton, buttonTextOverrides, msg, *msgBox);

    std::optional<QPointer<QObject>> guardPtr;
    if (guard)
        guardPtr = guard;

    QObject::connect(msgBox,
                     &QMessageBox::finished,
                     [guardPtr, msgBox, callback, decider, acceptButton] {
                         QMessageBox::StandardButton clickedBtn = msgBox->standardButton(
                             msgBox->clickedButton());

                         if (decider.doNotAskAgain && msgBox->checkBox()->isChecked()
                             && (acceptButton == QMessageBox::NoButton
                                 || clickedBtn == acceptButton)) {
                             decider.doNotAskAgain();
                         }

                         if (callback && (!guardPtr || *guardPtr))
                             callback(clickedBtn);

                         msgBox->deleteLater();
                     });

    msgBox->show();
}

CheckableDecider::CheckableDecider(const Key &settingsSubKey)
{
    QTC_ASSERT(theSettings, return);
    shouldAskAgain = [settingsSubKey] {
        theSettings->beginGroup(kDoNotAskAgainKey);
        bool shouldNotAsk = theSettings->value(settingsSubKey, false).toBool();
        theSettings->endGroup();
        return !shouldNotAsk;
    };
    doNotAskAgain = [settingsSubKey] {
        theSettings->beginGroup(kDoNotAskAgainKey);
        theSettings->setValue(settingsSubKey, true);
        theSettings->endGroup();
    };
}

CheckableDecider::CheckableDecider(bool *storage)
{
    shouldAskAgain = [storage] { return !*storage; };
    doNotAskAgain = [storage] { *storage = true; };
}

QMessageBox::StandardButton CheckableMessageBox::question(
    QWidget *parent,
    const QString &title,
    const QString &question,
    const CheckableDecider &decider,
    QMessageBox::StandardButtons buttons,
    QMessageBox::StandardButton defaultButton,
    QMessageBox::StandardButton acceptButton,
    QMap<QMessageBox::StandardButton, QString> buttonTextOverrides,
    const QString &msg)
{
    return exec(parent,
                QMessageBox::Question,
                title,
                question,
                decider,
                buttons,
                defaultButton,
                acceptButton,
                buttonTextOverrides,
                msg.isEmpty() ? msgDoNotAskAgain() : msg);
}

void CheckableMessageBox::question_async(
    QWidget *parent,
    const QString &title,
    const QString &question,
    const CheckableDecider &decider,
    QObject *guard,
    std::function<void(QMessageBox::StandardButton choosenBtn)> callback,
    QMessageBox::StandardButtons buttons,
    QMessageBox::StandardButton defaultButton,
    QMessageBox::StandardButton acceptButton,
    QMap<QMessageBox::StandardButton, QString> buttonTextOverrides,
    const QString &msg)
{
    show(parent,
         QMessageBox::Question,
         title,
         question,
         decider,
         buttons,
         defaultButton,
         acceptButton,
         buttonTextOverrides,
         msg.isEmpty() ? msgDoNotAskAgain() : msg,
         guard,
         callback);
}

QMessageBox::StandardButton CheckableMessageBox::information(
    QWidget *parent,
    const QString &title,
    const QString &text,
    const CheckableDecider &decider,
    QMessageBox::StandardButtons buttons,
    QMessageBox::StandardButton defaultButton,
    QMap<QMessageBox::StandardButton, QString> buttonTextOverrides,
    const QString &msg)
{
    return exec(parent,
                QMessageBox::Information,
                title,
                text,
                decider,
                buttons,
                defaultButton,
                defaultButton,
                buttonTextOverrides,
                msg.isEmpty() ? msgDoNotShowAgain() : msg);
}

void CheckableMessageBox::information_async(
    QWidget *parent,
    const QString &title,
    const QString &text,
    const CheckableDecider &decider,
    QObject *guard,
    std::function<void(QMessageBox::StandardButton choosenBtn)> callback,
    QMessageBox::StandardButtons buttons,
    QMessageBox::StandardButton defaultButton,
    QMap<QMessageBox::StandardButton, QString> buttonTextOverrides,
    const QString &msg)
{
    show(parent,
         QMessageBox::Information,
         title,
         text,
         decider,
         buttons,
         defaultButton,
         defaultButton,
         buttonTextOverrides,
         msg.isEmpty() ? msgDoNotShowAgain() : msg,
         guard,
         callback);
}

/*!
    Resets all suppression settings for doNotAskAgainQuestion()
    so all these message boxes are shown again.
 */
void CheckableMessageBox::resetAllDoNotAskAgainQuestions()
{
    QTC_ASSERT(theSettings, return);
    theSettings->beginGroup(kDoNotAskAgainKey);
    theSettings->remove(Key());
    theSettings->endGroup();
}

/*!
    Returns whether any message boxes from doNotAskAgainQuestion() are suppressed
    in the settings.
*/
bool CheckableMessageBox::hasSuppressedQuestions()
{
    QTC_ASSERT(theSettings, return false);
    theSettings->beginGroup(kDoNotAskAgainKey);
    const bool hasSuppressed = !theSettings->childKeys().isEmpty()
                               || !theSettings->childGroups().isEmpty();
    theSettings->endGroup();
    return hasSuppressed;
}

/*!
    Returns the standard \uicontrol {Do not ask again} check box text.
*/
QString CheckableMessageBox::msgDoNotAskAgain()
{
    return Tr::tr("Do not &ask again");
}

/*!
    Returns the standard \uicontrol {Do not show again} check box text.
*/
QString CheckableMessageBox::msgDoNotShowAgain()
{
    return Tr::tr("Do not &show again");
}

void CheckableMessageBox::initialize(QtcSettings *settings)
{
    theSettings = settings;
}

} // namespace Utils
