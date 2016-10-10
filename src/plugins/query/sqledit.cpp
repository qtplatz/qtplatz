/**************************************************************************
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#include "sqledit.hpp"
#include <adportable/debug.hpp>
#include <QAbstractItemView>
#include <QCompleter>
#include <QContextMenuEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QScrollBar>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QTextCharFormat>
#include <QTextBlock>
#include <QColor>
#include <qmargins.h>
#include <array>
#include <cstring>
#include <functional>
#include <fstream>
#include <regex>
#include <map>

using namespace query;

SqlEdit::~SqlEdit()
{
}

SqlEdit::SqlEdit(QWidget *parent) : QPlainTextEdit(parent)
                                  , completer_(0)
{
}

void
SqlEdit::setCompleter(QCompleter *completer)
{
    if (completer_)
        QObject::disconnect(completer_, 0, this, 0);

    completer_ = completer;

    if (!completer_)
        return;

    completer_->setWidget(this);
    completer_->setCompletionMode(QCompleter::PopupCompletion);
    completer_->setCaseSensitivity(Qt::CaseInsensitive);
    connect( completer_, static_cast<void(QCompleter::*)(const QString&)>(&QCompleter::activated), this, &SqlEdit::insertCompletion );
}

QCompleter *
SqlEdit::completer() const
{
    return completer_;
}

void
SqlEdit::insertCompletion(const QString& completion)
{
    if (completer_->widget() != this)
        return;
    QTextCursor tc = textCursor();
    int extra = completion.length() - completer_->completionPrefix().length();
    tc.movePosition(QTextCursor::StartOfWord, QTextCursor::MoveAnchor );
    tc.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor );
    tc.insertText(completion);
    setTextCursor(tc);
}

QString
SqlEdit::textUnderCursor() const
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}

void
SqlEdit::focusInEvent(QFocusEvent *e)
{
    if (completer_)
        completer_->setWidget( this );
    QPlainTextEdit::focusInEvent(e);
}

void
SqlEdit::keyPressEvent(QKeyEvent *e)
{
    if ( completer_ && completer_->popup()->isVisible() ) {
        // The following keys are forwarded by the completer to the widget
       switch (e->key()) {
       case Qt::Key_Enter:
       case Qt::Key_Return:
       case Qt::Key_Escape:
       case Qt::Key_Tab:
       case Qt::Key_Backtab:
            e->ignore();
            return; // let the completer do default behavior
       default:
           break;
       }
    }

    bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E); // CTRL+E
    if ( !completer_ || !isShortcut ) // do not process the shortcut when we have a completer
        QPlainTextEdit::keyPressEvent( e );

    const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (!completer_ || (ctrlOrShift && e->text().isEmpty()))
        return;

    static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
    bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = textUnderCursor();

    if (!isShortcut && (hasModifier || e->text().isEmpty()
                        || completionPrefix.length() < 2 || eow.contains(e->text().right(1)))) {
        completer_->popup()->hide();
        return;
    }

    if (completionPrefix != completer_->completionPrefix()) {
        completer_->setCompletionPrefix( completionPrefix );
        completer_->popup()->setCurrentIndex( completer_->completionModel()->index( 0, 0 ) );
    }
    QRect cr = cursorRect();
    cr.setWidth( completer_->popup()->sizeHintForColumn( 0 )
                 + completer_->popup()->verticalScrollBar()->sizeHint().width() );
    completer_->complete( cr ); // popup it up!
}

void
SqlEdit::contextMenuEvent( QContextMenuEvent * e )
{
    std::vector< std::pair< QAction *, std::function<void()> > > items;
    if ( QMenu * menu = createStandardContextMenu( e->globalPos() ) ) {
        menu->addAction( tr( "Add Summary Table" ), this, SLOT( addSummaryTable() ) );
        menu->exec( e->globalPos() );
    }
}

void
SqlEdit::addSummaryTable()
{
    QTextCursor cursor = textCursor();
    cursor.insertText( QString( QChar::ObjectReplacementCharacter ) );
}

