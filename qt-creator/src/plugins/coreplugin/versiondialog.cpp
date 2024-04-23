// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "versiondialog.h"

#include "coreicons.h"
#include "coreplugintr.h"
#include "coreicons.h"
#include "icore.h"

#include <utils/algorithm.h>
#include <utils/appinfo.h>
#include <utils/hostosinfo.h>
#include <utils/layoutbuilder.h>
#include <utils/qtcassert.h>
#include <utils/stringutils.h>
#include <utils/utilsicons.h>

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>

using namespace Core;
using namespace Core::Internal;

VersionDialog::VersionDialog(QWidget *parent)
    : QDialog(parent)
{
    // We need to set the window icon explicitly here since for some reason the
    // application icon isn't used when the size of the dialog is fixed (at least not on X11/GNOME)
    if (Utils::HostOsInfo::isLinuxHost())
        setWindowIcon(Icons::QTCREATORLOGO_BIG.icon());

    setWindowTitle(Tr::tr("About %1").arg(QGuiApplication::applicationDisplayName()));

    auto logoLabel = new QLabel;
    logoLabel->setPixmap(Icons::QTCREATORLOGO_BIG.pixmap());

    auto copyRightLabel = new QLabel(ICore::aboutInformationHtml());
    copyRightLabel->setWordWrap(true);
    copyRightLabel->setOpenExternalLinks(true);
    copyRightLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    QPushButton *copyButton = buttonBox->addButton(Tr::tr("Copy and Close"),
                                                   QDialogButtonBox::ApplyRole);

    using namespace Layouting;
    Column {
        Row {
            Column { logoLabel, st },
            Column { copyRightLabel },
        },
        buttonBox,
    }.attachTo(this);

    layout()->setSizeConstraint(QLayout::SetFixedSize);

    connect(copyButton, &QPushButton::pressed, this, [this] {
        Utils::setClipboardAndSelection(ICore::aboutInformationCompact());
        accept();
    });
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

bool VersionDialog::event(QEvent *event)
{
    if (event->type() == QEvent::ShortcutOverride) {
        auto ke = static_cast<QKeyEvent *>(event);
        if (ke->key() == Qt::Key_Escape && !ke->modifiers()) {
            ke->accept();
            return true;
        }
    }
    return QDialog::event(event);
}
