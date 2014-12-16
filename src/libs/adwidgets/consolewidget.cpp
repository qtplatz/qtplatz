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

#include "consolewidget.hpp"
#include "outputwidget.hpp"
#include <QSplitter>
#include <QApplication>
#include <QBoxLayout>
#include <QCompleter>
#include <QFile>
#include <QLineEdit>
#include <QStringListModel>
#include <iostream>

namespace adwidgets {
    
    class ConsoleWidget::impl {
        impl( const impl& ) = delete;
        ConsoleWidget& operator = ( const ConsoleWidget& ) = delete;
    public:
        ~impl() {
        }
        impl( std::ostream& os ) : consoleOut_( os ), completer_( 0 ) {
            connect( &lineEdit_, &QLineEdit::returnPressed, [this] (){ handleReturnPressed(); } );
        }
        QAbstractItemModel * modelFromFile(const QString& fileName);
        OutputWidget consoleOut_;
        QLineEdit lineEdit_;
        QCompleter * completer_;
        bool hasSession_;
    private:
        void handleReturnPressed() {
            QString text = lineEdit_.text();
            consoleOut_.println( text );
            lineEdit_.clear();
        }
        void create_session( const std::vector< std::string >& );
    };
}

using namespace adwidgets;

ConsoleWidget::~ConsoleWidget()
{
    delete impl_;
}

ConsoleWidget::ConsoleWidget( std::ostream& os, QWidget * parent ) : QWidget( parent )
                                                                    , impl_( new impl(os) )
{
    if ( auto splitter = new QSplitter ) {

        splitter->addWidget( &impl_->consoleOut_ );
        splitter->addWidget( &impl_->lineEdit_ );

        splitter->setOrientation( Qt::Vertical );

        if ( auto layout = new QVBoxLayout( this ) ) {
            layout->setMargin( 0 );
            layout->setSpacing( 0 );
            layout->addWidget( splitter );
        }
    }

    impl_->completer_ = new QCompleter( this );
    impl_->completer_->setModel( impl_->modelFromFile( ":/adwidgets/wordlist.txt" ) );
    impl_->completer_->setModelSorting( QCompleter::CaseInsensitivelySortedModel );
    impl_->completer_->setCaseSensitivity( Qt::CaseInsensitive );
    impl_->completer_->setWrapAround( false );
    impl_->consoleOut_.setCompleter( impl_->completer_ );
}

QAbstractItemModel *
ConsoleWidget::impl::modelFromFile( const QString& fileName )
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
        return new QStringListModel( completer_ );

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QStringList words;

    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        if (!line.isEmpty())
            words << line.trimmed();
    }

    QApplication::restoreOverrideCursor();

    return new QStringListModel(words, completer_);
}

void
ConsoleWidget::impl::create_session( const std::vector< std::string >& args )
{
}
