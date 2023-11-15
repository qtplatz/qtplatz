/**************************************************************************
** Copyright (C) 2010-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "queryform.hpp"
#include "document.hpp"
#include <QBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QStringList>

using namespace chemistry;

QueryForm::QueryForm( QWidget *parent ) : QWidget(parent)
                                        , semiColonCaptured_( false )
{
    resize( 200, 100 );

    auto vLayout = new QVBoxLayout( this );
    auto gridLayout = new QGridLayout();
    gridLayout->setHorizontalSpacing( 0 );

    //----------- row 0 ---------------------
    int row = 0;
    int col = 0;
    if ( auto edit = new QPlainTextEdit() ) {
        edit->setObjectName( "Query" );
        edit->setMaximumHeight( 32 );
        edit->installEventFilter( this );
        gridLayout->addWidget( new QLabel( tr("Query: ") ), row, col++ );
        gridLayout->addWidget( edit, row, col++, /*row span= */ 1, /* column span = */ 3 );
    }

    //----------- row 1 ---------------------
    ++row;
    col = 0;
    if ( auto edit = new QLineEdit( document::instance()->chemSpiderToken() ) ) {
        edit->setObjectName( "Token" );
        edit->setEchoMode( QLineEdit::PasswordEchoOnEdit );
        gridLayout->addWidget( new QLabel( tr("Token: ") ), row, col++ );
        gridLayout->addWidget( edit, row, col++, /*row span= */ 1, /* column span = */ 2 );
        connect( edit, &QLineEdit::editingFinished, [=](){ document::instance()->setChemSpiderToken( edit->text() ); } );
    }

    if ( auto button = new QPushButton( "invoke" ) ) {
        gridLayout->addWidget( button, row, gridLayout->columnCount() - 1 );
        connect( button, &QPushButton::pressed, this, [&](){
                if ( auto edit = findChild< QPlainTextEdit * >( "Query" ) )
                    emit trigger( edit->toPlainText() );
            });
    }

    //----------- row 2 ---------------------
    ++row;
    col = 1;
    if ( auto edit = new QTextEdit() ) {
        edit->setObjectName( "QueryResponse" );
        gridLayout->addWidget( edit, row, col, /*row span */ 1, /* column span */ 3 );
    }
    gridLayout->setColumnStretch( 1, 2 );

    vLayout->addLayout( gridLayout );
}

QueryForm::~QueryForm()
{
}

void
QueryForm::setSQL( const QString& t )
{
    if ( auto textEdit = findChild< QPlainTextEdit * >() )     {
        textEdit->clear();
        textEdit->insertPlainText( t );
    }
}

QString
QueryForm::sql() const
{
    if ( auto textEdit = findChild< QPlainTextEdit * >() )
        return textEdit->toPlainText();

    return QString();
}

bool
QueryForm::eventFilter( QObject * object, QEvent * event )
{
    auto textEdit = qobject_cast<QPlainTextEdit *>( object );

    if ( textEdit && event->type() == QEvent::KeyPress ) {
        if ( QKeyEvent * keyEvent = static_cast<QKeyEvent *>(event) ) {
            if ( keyEvent->key() == ';' )
                semiColonCaptured_ = true;
            else if ( keyEvent->key() == Qt::Key_Return && semiColonCaptured_ )
                emit trigger( textEdit->toPlainText() );
            else
                semiColonCaptured_ = false;
        }
    }
    return QWidget::eventFilter( object, event );
}
