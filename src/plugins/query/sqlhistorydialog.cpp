/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "sqlhistorydialog.hpp"
#include "ui_sqlhistorydialog.h"
#include <QTextCursor>
#include <QTextCharFormat>
#include <QTextEdit>
#include <QTextFrame>
#include <QTextFrameFormat>

using namespace query;

SqlHistoryDialog::SqlHistoryDialog(QWidget *parent) : QDialog( parent, Qt::Tool )
                                                    , ui(new Ui::SqlHistoryDialog)
                                                    , size_( 0 )
                                                    , mainFrame_( 0 )
                                                    , plainFormat_( std::make_unique< QTextCharFormat >() )
                                                    , headingFormat_( std::make_unique< QTextCharFormat >() )
                                                    , tagFormat_( std::make_unique< QTextCharFormat >() )
                                                    , underlineFormat_( std::make_unique< QTextCharFormat >() )
                                                    , frameFormat_( std::make_unique< QTextFrameFormat >() )
{
    ui->setupUi(this);
    connect( ui->buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );

    ui->textEdit->setReadOnly( true );
    
    plainFormat_->setFontPointSize( 10 );
    
    (*headingFormat_) = *plainFormat_;
    headingFormat_->setFontWeight( QFont::Bold );
    headingFormat_->setFontPointSize( 16 );
    
    (*tagFormat_) = *plainFormat_;
    tagFormat_->setForeground( QColor( "#990000" ) );
    tagFormat_->setFontUnderline( true );
    
    (*underlineFormat_) = *plainFormat_;
    underlineFormat_->setFontUnderline( true );
    
    frameFormat_->setBorderStyle( QTextFrameFormat::BorderStyle_Inset );
    frameFormat_->setBorder( 1 );
    frameFormat_->setMargin( 10 );
    frameFormat_->setPadding( 4 );
}

SqlHistoryDialog::~SqlHistoryDialog()
{
    delete ui;
}

QTextEdit *
SqlHistoryDialog::textEdit()
{
    return ui->textEdit;
}

void
SqlHistoryDialog::appendSql( const QString& text )
{
    auto cursor = ui->textEdit->textCursor();
    auto pframe = cursor.currentFrame();

    QString tag = QString( "%1" ).arg( size_ );
    cursor.insertText( tag, *tagFormat_ );
    size_++;

    {
        auto frame = cursor.insertFrame( *frameFormat_ ); // insert frame
        connect( frame, &QTextFrame::destroyed, [](){ /* handle block deleted */ } );
        cursor.insertText( text, *plainFormat_ );
    }
    
    cursor = mainFrame_->lastCursorPosition();

    ui->textEdit->setTextCursor( cursor );
}

void
SqlHistoryDialog::appendSql( const QStringList& list )
{
    size_ = 0;
    ui->textEdit->clear();

    auto cursor = ui->textEdit->textCursor();
    mainFrame_ = cursor.currentFrame();
    
    for ( auto& text: list )
        appendSql( text );

    ui->textEdit->setTextCursor( cursor );
    ui->textEdit->ensureCursorVisible();
}
