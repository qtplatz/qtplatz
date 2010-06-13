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

#include "openeditorsview.h"
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/editorview.h>
#include <coreplugin/editormanager/openeditorsmodel.h>
#include <coreplugin/icore.h>

#include <coreplugin/coreconstants.h>
#include <coreplugin/filemanager.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <utils/qtcassert.h>

#include <QtCore/QTimer>
#include <QtGui/QMenu>
#include <QtGui/QPainter>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>
#include <QtGui/QHeaderView>
#include <QtGui/QKeyEvent>
#ifdef Q_WS_MAC
#include <qmacstyle_mac.h>
#endif

Q_DECLARE_METATYPE(Core::IEditor*)

using namespace DataAnalysis;
using namespace DataAnalysis::Internal;


OpenEditorsDelegate::OpenEditorsDelegate(QObject *parent)
 : QStyledItemDelegate(parent)
{
}

void OpenEditorsDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
           const QModelIndex &index) const
{
    if (option.state & QStyle::State_MouseOver) {
        if ((QApplication::mouseButtons() & Qt::LeftButton) == 0)
            pressedIndex = QModelIndex();
        QBrush brush = option.palette.alternateBase();
        if (index == pressedIndex)
            brush = option.palette.dark();
        painter->fillRect(option.rect, brush);
    }


    QStyledItemDelegate::paint(painter, option, index);

    if (index.column() == 1 && option.state & QStyle::State_MouseOver) {
        QIcon icon((option.state & QStyle::State_Selected) ? ":/core/images/closebutton.png"
               : ":/core/images/darkclosebutton.png");

        QRect iconRect(option.rect.right() - option.rect.height(),
                       option.rect.top(),
                       option.rect.height(),
                       option.rect.height());

        icon.paint(painter, iconRect, Qt::AlignRight | Qt::AlignVCenter);
    }

}

////
// OpenEditorsWidget
////

OpenEditorsWidget::OpenEditorsWidget()
{
    m_ui.setupUi(this);
    setWindowTitle(tr("Open Datafiles"));
	setWindowIcon(QIcon(Core::Constants::ICON_DIR));
    setFocusProxy(m_ui.editorList);
    m_ui.editorList->viewport()->setAttribute(Qt::WA_Hover);
    m_ui.editorList->setItemDelegate((m_delegate = new OpenEditorsDelegate(this)));
    m_ui.editorList->header()->hide();
    m_ui.editorList->setIndentation(0);
    m_ui.editorList->setTextElideMode(Qt::ElideMiddle);
    m_ui.editorList->setFrameStyle(QFrame::NoFrame);
    m_ui.editorList->setAttribute(Qt::WA_MacShowFocusRect, false);
	Core::EditorManager *em = Core::EditorManager::instance();
    m_ui.editorList->setModel(em->openedEditorsModel());
    m_ui.editorList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_ui.editorList->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_ui.editorList->header()->setStretchLastSection(false);
    m_ui.editorList->header()->setResizeMode(0, QHeaderView::Stretch);
    m_ui.editorList->header()->setResizeMode(1, QHeaderView::Fixed);
    m_ui.editorList->header()->resizeSection(1, 16);
    m_ui.editorList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_ui.editorList->installEventFilter(this);

    connect(em, SIGNAL(currentEditorChanged(Core::IEditor*)),
            this, SLOT(updateCurrentItem(Core::IEditor*)));
    connect(m_ui.editorList, SIGNAL(clicked(QModelIndex)),
            this, SLOT(handleClicked(QModelIndex)));
    connect(m_ui.editorList, SIGNAL(pressed(QModelIndex)),
            this, SLOT(handlePressed(QModelIndex)));

    connect(m_ui.editorList, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(contextMenuRequested(QPoint)));
}

OpenEditorsWidget::~OpenEditorsWidget()
{
}

void OpenEditorsWidget::updateCurrentItem(Core::IEditor *editor)
{
    if (!editor) {
        m_ui.editorList->clearSelection();
        return;
    }
	Core::EditorManager *em = Core::EditorManager::instance();
    m_ui.editorList->setCurrentIndex(em->openedEditorsModel()->indexOf(editor));
    m_ui.editorList->selectionModel()->select(m_ui.editorList->currentIndex(),
                                              QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    m_ui.editorList->scrollTo(m_ui.editorList->currentIndex());
}

bool OpenEditorsWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_ui.editorList && event->type() == QEvent::KeyPress
            && m_ui.editorList->currentIndex().isValid()) {
        QKeyEvent *ke = static_cast<QKeyEvent*>(event);
        if ((ke->key() == Qt::Key_Return
                || ke->key() == Qt::Key_Enter)
                && ke->modifiers() == 0) {
            activateEditor(m_ui.editorList->currentIndex());
            return true;
        } else if ((ke->key() == Qt::Key_Delete
                   || ke->key() == Qt::Key_Backspace)
                && ke->modifiers() == 0) {
            closeEditor(m_ui.editorList->currentIndex());
        }
    }
    return false;
}

void OpenEditorsWidget::handlePressed(const QModelIndex &index)
{
    if (index.column() == 0) {
        activateEditor(index);
    } else if (index.column() == 1) {
        m_delegate->pressedIndex = index;
    }
}

void OpenEditorsWidget::handleClicked(const QModelIndex &index)
{
    if (index.column() == 1) { // the funky close button
        closeEditor(index);

        // work around a bug in itemviews where the delegate wouldn't get the QStyle::State_MouseOver
        QPoint cursorPos = QCursor::pos();
        QWidget *vp = m_ui.editorList->viewport();
        QMouseEvent e(QEvent::MouseMove, vp->mapFromGlobal(cursorPos), cursorPos, Qt::NoButton, 0, 0);
        QCoreApplication::sendEvent(vp, &e);
    }
}

void OpenEditorsWidget::activateEditor(const QModelIndex &index)
{
    m_ui.editorList->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
	Core::EditorManager::instance()->activateEditor(index);
}

void OpenEditorsWidget::closeEditor(const QModelIndex &index)
{
	Core::EditorManager::instance()->closeEditor(index);
    // work around selection changes
	updateCurrentItem(Core::EditorManager::instance()->currentEditor());
}

void OpenEditorsWidget::contextMenuRequested(QPoint pos)
{
    const QModelIndex index = m_ui.editorList->indexAt(pos);
    QMenu contextMenu;
    QAction *closeEditor = contextMenu.addAction(
            index.isValid() ?  tr("Close %1").arg(index.data().toString())
                            :  tr("Close Datum"));
    QAction *closeOtherEditors = contextMenu.addAction(
            index.isValid() ? tr("Close All Data Except %1").arg(index.data().toString())
                            : tr("Close Other Data"));
    QAction *closeAllEditors = contextMenu.addAction(tr("Close All Data"));

    if (!index.isValid()) {
        closeEditor->setEnabled(false);
        closeOtherEditors->setEnabled(false);
    }

	if (Core::EditorManager::instance()->openedEditors().isEmpty())
        closeAllEditors->setEnabled(false);

    QAction *action = contextMenu.exec(m_ui.editorList->mapToGlobal(pos));
    if (action == 0)
        return;
    if (action == closeEditor)
		Core::EditorManager::instance()->closeEditor(index);
    else if (action == closeAllEditors)
		Core::EditorManager::instance()->closeAllEditors();
    else if (action == closeOtherEditors)
		Core::EditorManager::instance()->closeOtherEditors(index.data(Qt::UserRole).value<Core::IEditor*>());
}

///
// OpenEditorsViewFactory
///

Core::NavigationView 
OpenEditorsViewFactory::createWidget()
{
	Core::NavigationView n;
    n.widget = new OpenEditorsWidget();
    return n;
}

QString
OpenEditorsViewFactory::displayName()
{
    return OpenEditorsWidget::tr("Open Data");
}

QKeySequence
OpenEditorsViewFactory::activationSequence()
{
    return QKeySequence(Qt::ALT + Qt::Key_O);
}

OpenEditorsViewFactory::OpenEditorsViewFactory()
{
}

OpenEditorsViewFactory::~OpenEditorsViewFactory()
{
}
