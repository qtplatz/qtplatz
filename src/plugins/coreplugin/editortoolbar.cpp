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

#include "editortoolbar.h"

#include <coreplugin/coreconstants.h>
#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/icore.h>

#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/editormanager_p.h>
#include <coreplugin/editormanager/documentmodel.h>
#include <coreplugin/actionmanager/actionmanager.h>

#include <utils/hostosinfo.h>
#include <utils/qtcassert.h>

#include <QDir>
#include <QApplication>
#include <QComboBox>
#include <QVBoxLayout>
#include <QToolButton>
#include <QMenu>
#include <QClipboard>

enum {
    debug = false
};

namespace Core {

struct EditorToolBarPrivate
{
    explicit EditorToolBarPrivate(QWidget *parent, EditorToolBar *q);

    QComboBox *m_editorList;
    QToolButton *m_closeEditorButton;
    QToolButton *m_lockButton;
    QAction *m_goBackAction;
    QAction *m_goForwardAction;
    QToolButton *m_backButton;
    QToolButton *m_forwardButton;
    QToolButton *m_splitButton;
    QAction *m_horizontalSplitAction;
    QAction *m_verticalSplitAction;
    QAction *m_splitNewWindowAction;
    QToolButton *m_closeSplitButton;

    QWidget *m_activeToolBar;
    QWidget *m_toolBarPlaceholder;
    QWidget *m_defaultToolBar;

    bool m_isStandalone;
};

EditorToolBarPrivate::EditorToolBarPrivate(QWidget *parent, EditorToolBar *q) :
    m_editorList(new QComboBox(q)),
    m_closeEditorButton(new QToolButton),
    m_lockButton(new QToolButton),
    m_goBackAction(new QAction(QIcon(QLatin1String(Constants::ICON_PREV)), EditorManager::tr("Go Back"), parent)),
    m_goForwardAction(new QAction(QIcon(QLatin1String(Constants::ICON_NEXT)), EditorManager::tr("Go Forward"), parent)),
    m_splitButton(new QToolButton),
    m_horizontalSplitAction(new QAction(QIcon(QLatin1String(Constants::ICON_SPLIT_HORIZONTAL)), EditorManager::tr("Split"), parent)),
    m_verticalSplitAction(new QAction(QIcon(QLatin1String(Constants::ICON_SPLIT_VERTICAL)), EditorManager::tr("Split Side by Side"), parent)),
    m_splitNewWindowAction(new QAction(EditorManager::tr("Open in New Window"), parent)),
    m_closeSplitButton(new QToolButton),
    m_activeToolBar(0),
    m_toolBarPlaceholder(new QWidget),
    m_defaultToolBar(new QWidget(q)),
    m_isStandalone(false)
{
}

/*!
  Mimic the look of the text editor toolbar as defined in e.g. EditorView::EditorView
  */
EditorToolBar::EditorToolBar(QWidget *parent) :
        Utils::StyledBar(parent), d(new EditorToolBarPrivate(parent, this))
{
    QHBoxLayout *toolBarLayout = new QHBoxLayout(this);
    toolBarLayout->setMargin(0);
    toolBarLayout->setSpacing(0);
    toolBarLayout->addWidget(d->m_defaultToolBar);
    d->m_toolBarPlaceholder->setLayout(toolBarLayout);
    d->m_toolBarPlaceholder->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    d->m_defaultToolBar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    d->m_activeToolBar = d->m_defaultToolBar;

    d->m_lockButton->setAutoRaise(true);
    d->m_lockButton->setEnabled(false);

    connect(d->m_goBackAction, SIGNAL(triggered()), this, SIGNAL(goBackClicked()));
    connect(d->m_goForwardAction, SIGNAL(triggered()), this, SIGNAL(goForwardClicked()));

    d->m_editorList->setProperty("hideicon", true);
    d->m_editorList->setProperty("notelideasterisk", true);
    d->m_editorList->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    d->m_editorList->setMinimumContentsLength(20);
    d->m_editorList->setModel(DocumentModel::model());
    d->m_editorList->setMaxVisibleItems(40);
    d->m_editorList->setContextMenuPolicy(Qt::CustomContextMenu);

    d->m_closeEditorButton->setAutoRaise(true);
    d->m_closeEditorButton->setIcon(QIcon(QLatin1String(Constants::ICON_BUTTON_CLOSE)));
    d->m_closeEditorButton->setEnabled(false);
    d->m_closeEditorButton->setProperty("showborder", true);

    d->m_toolBarPlaceholder->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    d->m_backButton = new QToolButton(this);
    d->m_backButton->setDefaultAction(d->m_goBackAction);

    d->m_forwardButton= new QToolButton(this);
    d->m_forwardButton->setDefaultAction(d->m_goForwardAction);

    if (Utils::HostOsInfo::isMacHost()) {
        d->m_horizontalSplitAction->setIconVisibleInMenu(false);
        d->m_verticalSplitAction->setIconVisibleInMenu(false);
        d->m_splitNewWindowAction->setIconVisibleInMenu(false);
    }

    d->m_splitButton->setIcon(QIcon(QLatin1String(Constants::ICON_SPLIT_HORIZONTAL)));
    d->m_splitButton->setToolTip(tr("Split"));
    d->m_splitButton->setPopupMode(QToolButton::InstantPopup);
    d->m_splitButton->setProperty("noArrow", true);
    QMenu *splitMenu = new QMenu(d->m_splitButton);
    splitMenu->addAction(d->m_horizontalSplitAction);
    splitMenu->addAction(d->m_verticalSplitAction);
    splitMenu->addAction(d->m_splitNewWindowAction);
    d->m_splitButton->setMenu(splitMenu);

    d->m_closeSplitButton->setAutoRaise(true);
    d->m_closeSplitButton->setIcon(QIcon(QLatin1String(Constants::ICON_CLOSE_SPLIT_BOTTOM)));

    QHBoxLayout *toplayout = new QHBoxLayout(this);
    toplayout->setSpacing(0);
    toplayout->setMargin(0);
    toplayout->addWidget(d->m_backButton);
    toplayout->addWidget(d->m_forwardButton);
    toplayout->addWidget(d->m_lockButton);
    toplayout->addWidget(d->m_editorList);
    toplayout->addWidget(d->m_closeEditorButton);
    toplayout->addWidget(d->m_toolBarPlaceholder, 1); // Custom toolbar stretches
    toplayout->addWidget(d->m_splitButton);
    toplayout->addWidget(d->m_closeSplitButton);

    setLayout(toplayout);

    // this signal is disconnected for standalone toolbars and replaced with
    // a private slot connection
    connect(d->m_editorList, SIGNAL(activated(int)), this, SIGNAL(listSelectionActivated(int)));

    connect(d->m_editorList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(listContextMenu(QPoint)));
    connect(d->m_lockButton, SIGNAL(clicked()), this, SLOT(makeEditorWritable()));
    connect(d->m_closeEditorButton, SIGNAL(clicked()), this, SLOT(closeEditor()), Qt::QueuedConnection);
    connect(d->m_horizontalSplitAction, SIGNAL(triggered()),
            this, SIGNAL(horizontalSplitClicked()), Qt::QueuedConnection);
    connect(d->m_verticalSplitAction, SIGNAL(triggered()),
            this, SIGNAL(verticalSplitClicked()), Qt::QueuedConnection);
    connect(d->m_splitNewWindowAction, SIGNAL(triggered()),
            this, SIGNAL(splitNewWindowClicked()), Qt::QueuedConnection);
    connect(d->m_closeSplitButton, SIGNAL(clicked()),
            this, SIGNAL(closeSplitClicked()), Qt::QueuedConnection);


    connect(ActionManager::command(Constants::CLOSE), SIGNAL(keySequenceChanged()),
            this, SLOT(updateActionShortcuts()));
    connect(ActionManager::command(Constants::GO_BACK), SIGNAL(keySequenceChanged()),
            this, SLOT(updateActionShortcuts()));
    connect(ActionManager::command(Constants::GO_FORWARD), SIGNAL(keySequenceChanged()),
            this, SLOT(updateActionShortcuts()));

    updateActionShortcuts();
}

EditorToolBar::~EditorToolBar()
{
    delete d;
}

void EditorToolBar::removeToolbarForEditor(IEditor *editor)
{
    QTC_ASSERT(editor, return);
    disconnect(editor->document(), SIGNAL(changed()), this, SLOT(checkDocumentStatus()));

    QWidget *toolBar = editor->toolBar();
    if (toolBar != 0) {
        if (d->m_activeToolBar == toolBar) {
            d->m_activeToolBar = d->m_defaultToolBar;
            d->m_activeToolBar->setVisible(true);
        }
        d->m_toolBarPlaceholder->layout()->removeWidget(toolBar);
        toolBar->setVisible(false);
        toolBar->setParent(0);
    }
}

void EditorToolBar::setCloseSplitEnabled(bool enable)
{
    d->m_closeSplitButton->setVisible(enable);
}

void EditorToolBar::setCloseSplitIcon(const QIcon &icon)
{
    d->m_closeSplitButton->setIcon(icon);
}

void EditorToolBar::closeEditor()
{
    IEditor *current = EditorManager::currentEditor();
    if (!current)
        return;

    if (d->m_isStandalone)
        EditorManager::closeEditor(current);
    emit closeClicked();
}

void EditorToolBar::addEditor(IEditor *editor)
{
    QTC_ASSERT(editor, return);
    connect(editor->document(), SIGNAL(changed()), this, SLOT(checkDocumentStatus()));
    QWidget *toolBar = editor->toolBar();

    if (toolBar && !d->m_isStandalone)
        addCenterToolBar(toolBar);

    updateDocumentStatus(editor->document());
}

void EditorToolBar::addCenterToolBar(QWidget *toolBar)
{
    QTC_ASSERT(toolBar, return);
    toolBar->setVisible(false); // will be made visible in setCurrentEditor
    d->m_toolBarPlaceholder->layout()->addWidget(toolBar);

    updateToolBar(toolBar);
}

void EditorToolBar::updateToolBar(QWidget *toolBar)
{
    if (!toolBar)
        toolBar = d->m_defaultToolBar;
    if (d->m_activeToolBar == toolBar)
        return;
    toolBar->setVisible(true);
    d->m_activeToolBar->setVisible(false);
    d->m_activeToolBar = toolBar;
}

void EditorToolBar::setToolbarCreationFlags(ToolbarCreationFlags flags)
{
    d->m_isStandalone = flags & FlagsStandalone;
    if (d->m_isStandalone) {
        EditorManager *em = EditorManager::instance();
        connect(em, SIGNAL(currentEditorChanged(Core::IEditor*)), SLOT(updateEditorListSelection(Core::IEditor*)));

        disconnect(d->m_editorList, SIGNAL(activated(int)), this, SIGNAL(listSelectionActivated(int)));
        connect(d->m_editorList, SIGNAL(activated(int)), this, SLOT(changeActiveEditor(int)));
        d->m_splitButton->setVisible(false);
        d->m_closeSplitButton->setVisible(false);
    }
}

void EditorToolBar::setCurrentEditor(IEditor *editor)
{
    IDocument *document = editor ? editor->document() : 0;
    d->m_editorList->setCurrentIndex(DocumentModel::rowOfDocument(document));

    // If we never added the toolbar from the editor,  we will never change
    // the editor, so there's no need to update the toolbar either.
    if (!d->m_isStandalone)
        updateToolBar(editor ? editor->toolBar() : 0);

    updateDocumentStatus(document);
}

void EditorToolBar::updateEditorListSelection(IEditor *newSelection)
{
    if (newSelection)
        d->m_editorList->setCurrentIndex(DocumentModel::rowOfDocument(newSelection->document()));
}

void EditorToolBar::changeActiveEditor(int row)
{
    EditorManager::activateEditorForEntry(DocumentModel::entryAtRow(row));
}

void EditorToolBar::listContextMenu(QPoint pos)
{
    DocumentModel::Entry *entry = DocumentModel::entryAtRow(
                d->m_editorList->currentIndex());
    QString fileName = entry ? entry->fileName() : QString();
    QString shortFileName = entry ? QFileInfo(fileName).fileName() : QString();
    QMenu menu;
    QAction *copyPath = menu.addAction(tr("Copy Full Path to Clipboard"));
    QAction *copyFileName = menu.addAction(tr("Copy File Name to Clipboard"));
    menu.addSeparator();
    if (fileName.isEmpty() || shortFileName.isEmpty()) {
        copyPath->setEnabled(false);
        copyFileName->setEnabled(false);
    }
    EditorManager::addSaveAndCloseEditorActions(&menu, entry);
    menu.addSeparator();
    EditorManager::addNativeDirAndOpenWithActions(&menu, entry);
    QAction *result = menu.exec(d->m_editorList->mapToGlobal(pos));
    if (result == copyPath)
        QApplication::clipboard()->setText(QDir::toNativeSeparators(fileName));
    if (result == copyFileName)
        QApplication::clipboard()->setText(shortFileName);
}

void EditorToolBar::makeEditorWritable()
{
    if (IDocument *current = EditorManager::currentDocument())
        Internal::EditorManagerPrivate::makeFileWritable(current);
}

void EditorToolBar::setCanGoBack(bool canGoBack)
{
    d->m_goBackAction->setEnabled(canGoBack);
}

void EditorToolBar::setCanGoForward(bool canGoForward)
{
    d->m_goForwardAction->setEnabled(canGoForward);
}

void EditorToolBar::updateActionShortcuts()
{
    d->m_closeEditorButton->setToolTip(ActionManager::command(Constants::CLOSE)->stringWithAppendedShortcut(EditorManager::tr("Close Document")));
    d->m_goBackAction->setToolTip(ActionManager::command(Constants::GO_BACK)->action()->toolTip());
    d->m_goForwardAction->setToolTip(ActionManager::command(Constants::GO_FORWARD)->action()->toolTip());
    d->m_closeSplitButton->setToolTip(ActionManager::command(Constants::REMOVE_CURRENT_SPLIT)->stringWithAppendedShortcut(tr("Remove Split")));
}

void EditorToolBar::checkDocumentStatus()
{
    IDocument *document = qobject_cast<IDocument *>(sender());
    QTC_ASSERT(document, return);
    DocumentModel::Entry *entry = DocumentModel::entryAtRow(
                d->m_editorList->currentIndex());

    if (entry && entry->document && entry->document == document)
        updateDocumentStatus(document);
}

void EditorToolBar::updateDocumentStatus(IDocument *document)
{
    d->m_closeEditorButton->setEnabled(document != 0);

    if (!document) {
        d->m_lockButton->setIcon(QIcon());
        d->m_lockButton->setEnabled(false);
        d->m_lockButton->setToolTip(QString());
        d->m_editorList->setToolTip(QString());
        return;
    }

    d->m_editorList->setCurrentIndex(DocumentModel::rowOfDocument(document));

    if (document->filePath().isEmpty()) {
        d->m_lockButton->setIcon(QIcon());
        d->m_lockButton->setEnabled(false);
        d->m_lockButton->setToolTip(QString());
    } else if (document->isFileReadOnly()) {
        d->m_lockButton->setIcon(DocumentModel::lockedIcon());
        d->m_lockButton->setEnabled(true);
        d->m_lockButton->setToolTip(tr("Make Writable"));
    } else {
        d->m_lockButton->setIcon(DocumentModel::unlockedIcon());
        d->m_lockButton->setEnabled(false);
        d->m_lockButton->setToolTip(tr("File is writable"));
    }
    d->m_editorList->setToolTip(
            document->filePath().isEmpty()
            ? document->displayName()
            : QDir::toNativeSeparators(document->filePath()));
}

void EditorToolBar::setNavigationVisible(bool isVisible)
{
    d->m_goBackAction->setVisible(isVisible);
    d->m_goForwardAction->setVisible(isVisible);
    d->m_backButton->setVisible(isVisible);
    d->m_forwardButton->setVisible(isVisible);
}

} // Core
