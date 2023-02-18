// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "saveitemsdialog.h"

#include <coreplugin/diffservice.h>
#include <coreplugin/idocument.h>

#include <utils/filepath.h>
#include <utils/fsengine/fileiconprovider.h>
#include <utils/hostosinfo.h>
#include <utils/layoutbuilder.h>

#include <extensionsystem/pluginmanager.h>

#include <QCheckBox>
#include <QDebug>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include <QPushButton>
#include <QTreeWidget>

Q_DECLARE_METATYPE(Core::IDocument*)

using namespace Core;
using namespace Core::Internal;

SaveItemsDialog::SaveItemsDialog(QWidget *parent, const QList<IDocument *> &items)
    : QDialog(parent)
    , m_msgLabel(new QLabel(tr("The following files have unsaved changes:")))
    , m_treeWidget(new QTreeWidget)
    , m_saveBeforeBuildCheckBox(new QCheckBox(tr("Automatically save all files before building")))
    , m_buttonBox(new QDialogButtonBox)
{
    resize(457, 200);
    setWindowTitle(tr("Save Changes"));

    m_treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_treeWidget->setTextElideMode(Qt::ElideLeft);
    m_treeWidget->setIndentation(0);
    m_treeWidget->setRootIsDecorated(false);
    m_treeWidget->setUniformRowHeights(true);
    m_treeWidget->setHeaderHidden(true);
    m_treeWidget->setColumnCount(2);

    // QDialogButtonBox's behavior for "destructive" is wrong, the "do not save" should be left-aligned
    const QDialogButtonBox::ButtonRole discardButtonRole = Utils::HostOsInfo::isMacHost()
                                                               ? QDialogButtonBox::ResetRole
                                                               : QDialogButtonBox::DestructiveRole;
    if (DiffService::instance()) {
        m_diffButton = m_buttonBox->addButton(tr("&Diff"), discardButtonRole);
        connect(m_diffButton, &QAbstractButton::clicked, this, &SaveItemsDialog::collectFilesToDiff);
    }
    m_buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Save);
    QPushButton *discardButton = m_buttonBox->addButton(tr("Do &Not Save"), discardButtonRole);
    m_treeWidget->setFocus();

    m_saveBeforeBuildCheckBox->setVisible(false);

    using namespace Utils::Layouting;
    // clang-format off
    Column {
        m_msgLabel,
        m_treeWidget,
        m_saveBeforeBuildCheckBox,
        m_buttonBox
    }.attachTo(this);
    // clang-format on

    for (IDocument *document : items) {
        QString visibleName;
        QString directory;
        Utils::FilePath filePath = document->filePath();
        if (filePath.isEmpty()) {
            visibleName = document->fallbackSaveAsFileName();
        } else {
            directory = filePath.absolutePath().toUserOutput();
            visibleName = filePath.fileName();
        }
        QTreeWidgetItem *item = new QTreeWidgetItem(m_treeWidget,
                                                    QStringList()
                                                        << visibleName
                                                        << QDir::toNativeSeparators(directory));
        if (!filePath.isEmpty())
            item->setIcon(0, Utils::FileIconProvider::icon(filePath));
        item->setData(0, Qt::UserRole, QVariant::fromValue(document));
    }

    m_treeWidget->resizeColumnToContents(0);
    m_treeWidget->selectAll();
    if (Utils::HostOsInfo::isMacHost())
        m_treeWidget->setAlternatingRowColors(true);
    adjustButtonWidths();
    updateButtons();

    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_buttonBox->button(QDialogButtonBox::Save),
            &QAbstractButton::clicked,
            this,
            &SaveItemsDialog::collectItemsToSave);
    connect(discardButton, &QAbstractButton::clicked, this, &SaveItemsDialog::discardAll);
    connect(m_treeWidget, &QTreeWidget::itemSelectionChanged, this, &SaveItemsDialog::updateButtons);

    m_buttonBox->button(QDialogButtonBox::Save)->setDefault(true);
}

void SaveItemsDialog::setMessage(const QString &msg)
{
    m_msgLabel->setText(msg);
}

void SaveItemsDialog::updateButtons()
{
    int count = m_treeWidget->selectedItems().count();
    QPushButton *saveButton = m_buttonBox->button(QDialogButtonBox::Save);
    bool buttonsEnabled = true;
    QString saveText = tr("&Save");
    QString diffText = tr("&Diff && Cancel");
    if (count == m_treeWidget->topLevelItemCount()) {
        saveText = tr("&Save All");
        diffText = tr("&Diff All && Cancel");
    } else if (count == 0) {
        buttonsEnabled = false;
    } else {
        saveText = tr("&Save Selected");
        diffText = tr("&Diff Selected && Cancel");
    }
    saveButton->setEnabled(buttonsEnabled);
    saveButton->setText(saveText);
    if (m_diffButton) {
        m_diffButton->setEnabled(buttonsEnabled);
        m_diffButton->setText(diffText);
    }
}

void SaveItemsDialog::adjustButtonWidths()
{
    // give save button a size that all texts fit in, so it doesn't get resized
    // Mac: make cancel + save button same size (work around dialog button box issue)
    QStringList possibleTexts;
    possibleTexts << tr("Save") << tr("Save All");
    if (m_treeWidget->topLevelItemCount() > 1)
        possibleTexts << tr("Save Selected");
    int maxTextWidth = 0;
    QPushButton *saveButton = m_buttonBox->button(QDialogButtonBox::Save);
    for (const QString &text : std::as_const(possibleTexts)) {
        saveButton->setText(text);
        int hint = saveButton->sizeHint().width();
        if (hint > maxTextWidth)
            maxTextWidth = hint;
    }
    if (Utils::HostOsInfo::isMacHost()) {
        QPushButton *cancelButton = m_buttonBox->button(QDialogButtonBox::Cancel);
        int cancelButtonWidth = cancelButton->sizeHint().width();
        if (cancelButtonWidth > maxTextWidth)
            maxTextWidth = cancelButtonWidth;
        cancelButton->setMinimumWidth(maxTextWidth);
    }
    saveButton->setMinimumWidth(maxTextWidth);
}

void SaveItemsDialog::collectItemsToSave()
{
    m_itemsToSave.clear();
    const QList<QTreeWidgetItem *> items = m_treeWidget->selectedItems();
    for (const QTreeWidgetItem *item : items) {
        m_itemsToSave.append(item->data(0, Qt::UserRole).value<IDocument*>());
    }
    accept();
}

void SaveItemsDialog::collectFilesToDiff()
{
    m_filesToDiff.clear();
    const QList<QTreeWidgetItem *> items = m_treeWidget->selectedItems();
    for (const QTreeWidgetItem *item : items) {
        if (auto doc = item->data(0, Qt::UserRole).value<IDocument*>())
            m_filesToDiff.append(doc->filePath().toString());
    }
    reject();
}

void SaveItemsDialog::discardAll()
{
    m_treeWidget->clearSelection();
    collectItemsToSave();
}

QList<IDocument*> SaveItemsDialog::itemsToSave() const
{
    return m_itemsToSave;
}

QStringList SaveItemsDialog::filesToDiff() const
{
    return m_filesToDiff;
}

void SaveItemsDialog::setAlwaysSaveMessage(const QString &msg)
{
    m_saveBeforeBuildCheckBox->setText(msg);
    m_saveBeforeBuildCheckBox->setVisible(true);
}

bool SaveItemsDialog::alwaysSaveChecked()
{
    return m_saveBeforeBuildCheckBox->isChecked();
}
