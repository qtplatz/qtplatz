/**************************************************************************
** Copyright (C) 2014-2015 MS-Cheminformatics LLC, Toin, Mie Japan
** Author: Toshinobu Hondo, Ph.D.
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "outputwidget.hpp"
#include <QAbstractItemView>
#include <QBoxLayout>
#include <QCompleter>
#include <QMenu>
#include <QScrollBar>
#include <QTextEdit>
#include <QTextCharFormat>
#include <QTextBlock>
#include <functional>
#include <iostream>
#include <streambuf>
#include <string>
#include <mutex>
    
class OutputWidget::impl : public QTextEdit, public std::basic_streambuf < char > {
    Q_OBJECT
public:
    impl( std::ostream &stream ) : stream_( stream ), mainFrame_( 0 ), completer_(0) {
        old_buf_ = stream.rdbuf();
        stream.rdbuf(this);
        QTextCursor cursor = textCursor();
        cursor.movePosition( QTextCursor::Start );
        pframe_ = mainFrame_ = cursor.currentFrame();
        
        plainFormat_ = cursor.charFormat();
        plainFormat_.setFontPointSize( 10 );

        underlineFormat_ = plainFormat_;
        underlineFormat_.setFontUnderline( true );
        underlineFormat_.setForeground( QColor( "#990000" ) );

        frameFormat_.setBorderStyle( QTextFrameFormat::BorderStyle_Inset );
        frameFormat_.setBorder( 1 );
        frameFormat_.setMargin( 2 );
        frameFormat_.setPadding( 2 );
        
        connect( this, &impl::onText, this, &impl::handleText );
    }

    ~impl() {
        stream_.rdbuf( old_buf_ );
    }
        
protected:
    // this can be called from several threads
    virtual int_type overflow(int_type v) override {
        std::lock_guard< std::mutex > lock( mutex_ );
        linebuf_.push_back( v );
        if ( v == '\n' ) {
            emit onText( linebuf_ );
            linebuf_.clear();
        }
        return v;
    }
        
    virtual std::streamsize xsputn( const char *p, std::streamsize n ) override {
        size_t nchars = n;
        while ( nchars-- )
            overflow( *p++ );
        return n;
    }

    void keyPressEvent( QKeyEvent * e ) override;
    void insertCompletion( const QString& );
    QString textUnderCursor() const;
    void focusInEvent( QFocusEvent * ) override;
    void contextMenuEvent( QContextMenuEvent * ) override;
    void handleEnterKey();
    void handleBlockDeleted();
private:
    friend class OutputWidget;

    std::ostream& stream_;
    std::streambuf * old_buf_;
    QString linebuf_;
    QTextFrame * mainFrame_;
    QTextFrame * pframe_;
    QCompleter * completer_;
    QTextCharFormat plainFormat_;
    QTextCharFormat underlineFormat_;
    QTextFrameFormat frameFormat_;
    std::mutex mutex_;
signals:
    void onText( const QString& );
private slots:
    void handleText( const QString& text ) {
        auto cursor = pframe_->lastCursorPosition();
        cursor.insertText( text, plainFormat_ );
        setTextCursor( cursor );
        ensureCursorVisible();
    }
};


OutputWidget::~OutputWidget()
{
    delete impl_;
}

OutputWidget::OutputWidget( std::ostream& os, QWidget * parent ) : QWidget( parent )
                                                                 , impl_( new impl( os ) )
{
    auto layout = new QVBoxLayout( this );
    layout->setMargin( 0 );
    layout->setSpacing( 0 );
    layout->addWidget( impl_ );
}

void
OutputWidget::println( const QString& text, bool newframe )
{
    if ( newframe )
        impl_->pframe_ = impl_->mainFrame_->lastCursorPosition().insertFrame( impl_->frameFormat_ );

    auto cursor = impl_->pframe_->lastCursorPosition();

    cursor.insertText( text, impl_->underlineFormat_ );
    cursor.insertText( "\n", impl_->underlineFormat_ );
}

QString
OutputWidget::impl::textUnderCursor() const
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}

void
OutputWidget::impl::focusInEvent(QFocusEvent *e)
{
    if (completer_)
        completer_->setWidget( this );
    QTextEdit::focusInEvent(e);
}

void
OutputWidget::impl::keyPressEvent(QKeyEvent *e)
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
    }  else if ( (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) ) {
        handleEnterKey();
    }
}


void
OutputWidget::impl::contextMenuEvent( QContextMenuEvent * e )
{
    // std::vector< std::pair< QAction *, std::function<void()> > > items;
    // if ( QMenu * menu = createStandardContextMenu( e->globalPos() ) ) {
    //     menu->exec( e->globalPos() );
    // }
}

void
OutputWidget::impl::handleEnterKey()
{
}

void
OutputWidget::impl::handleBlockDeleted()
{
}

#include "outputwidget.moc"
