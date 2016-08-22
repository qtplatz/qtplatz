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
#include <QStringList>
#include <QPlainTextEdit>
#include <QBoxLayout>
#include <QComboBox>
#include <QPushButton>

using namespace chemistry;

QueryForm::QueryForm(QWidget *parent) :  QWidget(parent)
                                      , semiColonCaptured_( false )
{
    resize( 200, 100 );

    auto vLayout = new QVBoxLayout( this );
    auto gridLayout = new QGridLayout();

    if ( auto textEditor = new QPlainTextEdit() ) {
        textEditor->installEventFilter( this );
        textEditor->setMaximumHeight( 80 );
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(textEditor->sizePolicy().hasHeightForWidth());
        textEditor->setSizePolicy(sizePolicy);
        textEditor->setMaximumSize(QSize(16777215, 80));
        gridLayout->addWidget( textEditor, 0, 0, /*row span= */ 1, /* column span = */ 4 );
    }

    if ( auto combo = new QComboBox() ) {
        combo->setObjectName( "tableList" );
        gridLayout->addWidget( combo, 1, 0, 1, 1 );

        // connect( combo, static_cast< void(QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, &QueryForm::on_comboBox_currentIndexChanged );
    }

    if ( auto combo = new QComboBox() ) {
        combo->setObjectName( "subList" );
        gridLayout->addWidget( combo, 1, 1, 1, 1 );
        // connect( combo, static_cast<void( QComboBox::* )( const QString& )>( &QComboBox::currentIndexChanged ), this, &QueryForm::on_subList_currentIndexChanged );
    }

    if ( auto button = new QPushButton( "execute query" ) ) {
        gridLayout->addWidget( button, 1, 2, 1, 1 );
        // connect( button, &QPushButton::pressed, this, &QueryForm::on_pushButton_pressed );
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

// void
// QueryForm::setTableList( const QList< QString >& list )
// {
//     if ( auto combo = findChild< QComboBox * >( "tableList" ) ) {
//         combo->clear();
//         combo->addItems( list );
//     }
// }

// void
// QueryForm::setSubList( const QList< QString >& list )
// {
//     if ( auto combo = findChild< QComboBox * >( "subList" ) ) {
//         combo->clear();
//         combo->addItem( "" );
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

// void 
// QueryForm::on_plainTextEdit_textChanged()
// {
// }

// void 
// QueryForm::on_pushButton_pressed()
// {
//     if ( auto textEdit = findChild< QPlainTextEdit * >() )
//         emit triggerQuery( textEdit->toPlainText() );
// }

// void 
// QueryForm::on_comboBox_currentIndexChanged( const QString& itemText )
// {
//     QString subItem;
//     if ( auto combo = findChild< QComboBox * >( "subList" ) )
//         subItem = combo->currentText();

//     if ( itemText == "AcquiredData" ) {
//         if ( subItem.isEmpty() )
//             setSQL( QString( "SELECT * FROM %1 ORDER BY npos" ).arg( itemText ));
//         else
//             setSQL( QString( "SELECT * FROM %1 WHERE objuuid = '%2' ORDER BY npos" ).arg( itemText, subItem ));
//     } else {
//         setSQL( QString( "SELECT * FROM %1" ).arg( itemText ));
//     }
// }

// void 
// QueryForm::on_subList_currentIndexChanged( const QString& itemText )
// {
//     if ( auto combo = findChild< QComboBox * >( "tableList" ) ) {
//         if ( combo->currentText() == "AcquiredData" ) {
//             on_comboBox_currentIndexChanged( combo->currentText() );
//         }
//     }
// }

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
