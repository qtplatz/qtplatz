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
#include "sqledit.hpp"
#include <adportable/debug.hpp>
#include <QAbstractItemView>
#include <QApplication>
#include <QCompleter>
#include <QScrollBar>
#include <QStringList>
#include <QPlainTextEdit>
#include <QBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QStyledItemDelegate>

using namespace query;

QueryForm::QueryForm(QWidget *parent) : QWidget(parent)
                                      , semiColonCaptured_( false )
{
    resize( 200, 100 );

    auto vLayout = new QVBoxLayout( this );
    auto gridLayout = new QGridLayout();

    if ( auto textEditor = new SqlEdit() ) {
        textEditor->installEventFilter( this );
        vLayout->addWidget( textEditor );
    }

    if ( auto combo = new QComboBox() ) {
        combo->setObjectName( "tableList" );
        gridLayout->addWidget( combo, 1, 0, 1, 1 );
        connect( combo, static_cast< void(QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged)
                 , this, &QueryForm::on_comboBox_currentIndexChanged );
    }

    if ( auto button = new QPushButton( "View History" ) ) {
        gridLayout->addWidget( button, 1, 1, 1, 1 );
        connect( button, &QPushButton::clicked, this, [&](){ emit showHistory(); });
    }
/*
    if ( auto combo = new QComboBox() ) {
        combo->setObjectName( "history" );
        combo->setSizeAdjustPolicy( QComboBox::AdjustToMinimumContentsLength );
        gridLayout->addWidget( combo, 1, 1, 1, 1 );
        connect( combo, static_cast< void(QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged)
                 , this, &QueryForm::on_history_currentIndexChanged );
    }
*/
    if ( auto button = new QPushButton( "execute query" ) ) {
        gridLayout->addWidget( button, 1, 2, 1, 1 );
        connect( button, &QPushButton::pressed, this, &QueryForm::on_pushButton_pressed );
    }
    
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

void
QueryForm::setTableList( const QList< QString >& list )
{
    if ( auto combo = findChild< QComboBox * >( "tableList" ) ) {
        combo->clear();
        combo->addItems( list );
    }
}

// void
// QueryForm::setSqlHistory( const QStringList& list )
// {
//     if ( auto combo = findChild< QComboBox * >( "history" ) ) {
//         combo->clear();
//         combo->addItems( list );
//     }
// }

QString
QueryForm::sql() const
{
    if ( auto textEdit = findChild< QPlainTextEdit * >() )
        return textEdit->toPlainText();

    return QString();
}

void 
QueryForm::on_plainTextEdit_textChanged()
{
}

void 
QueryForm::on_pushButton_pressed()
{
    if ( auto textEdit = findChild< QPlainTextEdit * >() )
        emit triggerQuery( textEdit->toPlainText() );
}

void 
QueryForm::on_comboBox_currentIndexChanged( const QString& itemText )
{
    if ( itemText == "{Counting}" )
        setSQL( QString( "SELECT ROUND(peak_time, 9) AS time, COUNT(*), protocol  FROM peak,trigger WHERE id=idTrigger GROUP BY time ORDER BY time" ) );
    else
        setSQL( QString( "SELECT * FROM %1" ).arg( itemText ));
}

void 
QueryForm::on_history_currentIndexChanged( const QString& itemText )
{
    if ( auto combo = findChild< QComboBox * >( "history" ) )
        setSQL( itemText );
}

bool
QueryForm::eventFilter( QObject * object, QEvent * event )
{
    auto textEdit = qobject_cast<QPlainTextEdit *>( object );

    if ( textEdit && event->type() == QEvent::KeyPress ) {
        if ( QKeyEvent * keyEvent = static_cast<QKeyEvent *>(event) ) {
            if ( keyEvent->key() == ';' ) {
                semiColonCaptured_ = true;
            } else if ( keyEvent->key() == Qt::Key_Return && semiColonCaptured_ ) {
                emit triggerQuery( textEdit->toPlainText() );
            } else {
                semiColonCaptured_ = false;
            }
        }
    }
    return QWidget::eventFilter( object, event );
}

void
QueryForm::setCompleter( QCompleter *completer )
{
    if ( auto textEditor = findChild< SqlEdit * >() ) {
        textEditor->setCompleter( completer );
    }
}

QCompleter *
QueryForm::completer() const
{
    if ( auto textEditor = findChild< SqlEdit * >() )
        return textEditor->completer();
    return nullptr;
}

