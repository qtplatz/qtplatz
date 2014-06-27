/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** Commercial Usage
**
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://qt.nokia.com/contact.
**
**************************************************************************/

#include "versiondialog.h"
#include "../../version.h"
#include "coreconstants.h"
#include "icore.h"

#include <utils/qtcassert.h>

#include <QtCore/QDate>
#include <QtCore/QFile>
#include <QtCore/QSysInfo>

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextBrowser>

using namespace Core;
using namespace Core::Internal;
using namespace Core::Constants;

VersionDialog::VersionDialog(QWidget *parent)
    : QDialog(parent)
{
    // We need to set the window icon explicitly here since for some reason the
    // application icon isn't used when the size of the dialog is fixed (at least not on X11/GNOME)
    setWindowIcon(QIcon(":/core/images/qtcreator_logo_128.png"));

    setWindowTitle(tr("About QtPlatz"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    QGridLayout *layout = new QGridLayout(this);
    layout->setSizeConstraint(QLayout::SetFixedSize);

    QString version = QLatin1String(VERSION);
    version += QDate(2007, 25, 10).toString(Qt::SystemLocaleDate);

    QString ideRev;
#ifdef IDE_REVISION
     //: This gets conditionally inserted as argument %8 into the description string.
     ideRev = tr("From revision %1<br/>").arg(QString::fromLatin1(IDE_REVISION_STR).left(10));
#endif

     const QString description = tr(
        "<h3>QtPlatz %1</h3>"
        "Based on Qt %2 (%3 bit)<br/>"
        "<br/>"
        "Built on %4 at %5<br />"
        "<br/>"
        "%8"
        "<br/>"
        "Copyright 2010-%6 %7. All rights reserved.<br/>"
        "<br/>"
        "The program is provided AS IS with NO WARRANTY OF ANY KIND, "
        "INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A "
        "PARTICULAR PURPOSE.<br/>")
        .arg(version, QLatin1String(QT_VERSION_STR), QString::number(QSysInfo::WordSize), 
             QLatin1String(__DATE__), QLatin1String(__TIME__), QLatin1String("2014"), 
             (QLatin1String("MS-Cheminformatics LLC")), ideRev);

    QLabel *copyRightLabel = new QLabel(description);
    copyRightLabel->setWordWrap(true);
    copyRightLabel->setOpenExternalLinks(true);
    copyRightLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    QPushButton *closeButton = buttonBox->button(QDialogButtonBox::Close);
    QTC_ASSERT(closeButton, /**/);
    buttonBox->addButton(closeButton, QDialogButtonBox::ButtonRole(QDialogButtonBox::RejectRole | QDialogButtonBox::AcceptRole));
    connect(buttonBox , SIGNAL(rejected()), this, SLOT(reject()));

    QLabel *logoLabel = new QLabel;
    logoLabel->setPixmap(QPixmap(QLatin1String(":/core/images/qtcreator_logo_128.png")));
    layout->addWidget(logoLabel , 0, 0, 1, 1);
    layout->addWidget(copyRightLabel, 0, 1, 4, 4);
    layout->addWidget(buttonBox, 4, 0, 1, 5);
}
