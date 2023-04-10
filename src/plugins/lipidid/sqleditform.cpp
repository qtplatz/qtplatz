/**************************************************************************
** Copyright (C) 2010-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "sqleditform.hpp"
#include "document.hpp"
#include "sqledit.hpp"
#include <utils/styledbar.h>
#include <adportable/debug.hpp>
#include <QBoxLayout>
#include <QCompleter>
#include <QFile>
#include <QPushButton>
#include <QStringListModel>

using lipidid::SqlEditForm;

namespace {
    using lipidid::SqlEdit;
    struct SqlCompleter {
        QWidget * pThis_;
        SqlCompleter( QWidget * p ) : pThis_( p ) {}
        void operator()( SqlEdit * editor ) const  {
            if ( auto completer = new QCompleter( pThis_ ) ) {
                QStringList words ( "mols" );
                QFile file( ":/query/wordlist.txt" );
                if ( file.open( QFile::ReadOnly ) ) {
                    while ( !file.atEnd() ) {
                        QByteArray line = file.readLine();
                        if ( ! line.isEmpty() )
                            words << line.trimmed();
                    }
                }
                words.sort( Qt::CaseInsensitive );
                words.removeDuplicates();
                completer->setModel( new QStringListModel( words, completer ) );
                completer->setModelSorting( QCompleter::CaseInsensitivelySortedModel );
                completer->setCaseSensitivity( Qt::CaseInsensitive );
                completer->setWrapAround( false );
                editor->setCompleter( completer );
            }
        }
    };
}

SqlEditForm::SqlEditForm(QWidget *parent) : QWidget(parent)
                                          , semiColonCaptured_( false )
{
    auto vLayout = new QVBoxLayout( this );

    if ( auto editor = new SqlEdit() ) {
        editor->installEventFilter( this );
        vLayout->addWidget( editor );

        if ( auto toolBar = new Utils::StyledBar ) {
            toolBar->setProperty( "topBorder", true );
            toolBar->setSingleRow( true );
            //toolBar->setLightColored( false );
            QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
            toolBarLayout->setContentsMargins( {} );
            toolBarLayout->setSpacing( 2 );

            toolBarLayout->addWidget( new Utils::StyledSeparator );
            toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );

            toolBarLayout->addWidget( new Utils::StyledSeparator );

            if ( auto button = new QPushButton( "Execute Query" ) ) {
                toolBarLayout->addWidget( button );
                connect( button, &QPushButton::pressed, this, [editor,this]{ emit triggerQuery( editor->toPlainText() ); } );
            }
            vLayout->addWidget( toolBar );
        }
        SqlCompleter(this)( editor );
    }
    setSQL( "SELECT * FROM mols WHERE mass > 100 AND mass < 1200 ORDER BY mass DESC" );
}

SqlEditForm::~SqlEditForm()
{
}

void
SqlEditForm::setSQL( const QString& t )
{
    if ( auto textEdit = findChild< QPlainTextEdit * >() )     {
        textEdit->clear();
        textEdit->insertPlainText( t );
    }
}

QString
SqlEditForm::sql() const
{
    if ( auto textEdit = findChild< SqlEdit * >() )
        return textEdit->toPlainText();

    return QString();
}

bool
SqlEditForm::eventFilter( QObject * object, QEvent * event )
{
    if ( event->type() == QEvent::KeyPress ) {
        if ( QKeyEvent * keyEvent = static_cast<QKeyEvent *>(event) ) {
            if ( keyEvent->key() == ';' ) {
                semiColonCaptured_ = true;
            } else if ( keyEvent->key() == Qt::Key_Return && semiColonCaptured_ ) {
                semiColonCaptured_ = false;
                if ( auto editor = qobject_cast< QPlainTextEdit *>( object ) )
                    emit triggerQuery( editor->toPlainText() );
            } else {
                semiColonCaptured_ = false;
            }
        }
    }
    return QWidget::eventFilter( object, event );
}

void
SqlEditForm::setCompleter( QCompleter *completer )
{
    if ( auto textEditor = findChild< SqlEdit * >() ) {
        textEditor->setCompleter( completer );
    }
}

QCompleter *
SqlEditForm::completer() const
{
    if ( auto textEditor = findChild< SqlEdit * >() )
        return textEditor->completer();
    return nullptr;
}
