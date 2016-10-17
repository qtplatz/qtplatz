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

#include "queryquerywidget.hpp"
#include "queryconnection.hpp"
#include "queryconstants.hpp"
#include "querydocument.hpp"
#include "queryquery.hpp"
#include "queryqueryform.hpp"
#include "queryresulttable.hpp"
#if QT5_CHARTS
# include "charts/chartview.hpp"
#endif
#include "plotdialog.hpp"
#include <adportable/profile.hpp>
#include <adportable/debug.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <qtwrapper/progresshandler.hpp>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/progressmanager/progressmanager.h>
#include <utils/styledbar.h>
#include <QCompleter>
#include "qwt/chartview.hpp"
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSplitter>
#include <QSqlError>
#include <QStringListModel>
#include <QStackedWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QStandardItemModel>
#include <boost/filesystem.hpp>
#include <boost/exception/all.hpp>
#include <fstream>
#include <algorithm>

using namespace query;

#if QT5_CHARTS
QT_CHARTS_USE_NAMESPACE;
#endif

QueryQueryWidget::~QueryQueryWidget()
{
}

QueryQueryWidget::QueryQueryWidget(QWidget *parent) : QWidget(parent)
                                                    , layout_( new QGridLayout )
                                                    , form_( new QueryQueryForm )
                                                    , table_( new QueryResultTable )
{
    qRegisterMetaType < std::shared_ptr< QueryQuery > >();

    auto topLayout = new QVBoxLayout( this );
    topLayout->setMargin( 0 );
    topLayout->setSpacing( 0 );
    topLayout->addLayout( layout_ );

    connect( QueryDocument::instance(), &QueryDocument::onConnectionChanged, this, &QueryQueryWidget::handleConnectionChanged );
    connect( QueryDocument::instance(), &QueryDocument::onHistoryChanged, this, [this](){ form_->setSqlHistory( QueryDocument::instance()->sqlHistory() ); } );
    connect( form_.get(), &QueryQueryForm::triggerQuery, this, &QueryQueryWidget::handleQuery );
    connect( table_.get(), &QueryResultTable::plot, this, &QueryQueryWidget::handlePlot );

    if ( auto toolBar = new Utils::StyledBar ) {
        
        layout_->addWidget( toolBar );
        
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 2 );
        toolBarLayout->setSpacing( 2 );
        
        if ( auto btnOpen = new QToolButton ) {
            btnOpen->setDefaultAction( Core::ActionManager::instance()->command( Constants::FILE_OPEN )->action() );
            btnOpen->setToolTip( tr("Open result file...") );
            toolBarLayout->addWidget( btnOpen );

            auto edit = new QLineEdit;
            edit->setReadOnly( true );
            edit->setObjectName( Constants::editQueryFilename );
            toolBarLayout->addWidget( edit );
        }
    }

    if ( QSplitter * splitter = new QSplitter ) {
        splitter->setOrientation( Qt::Vertical );

        splitter->addWidget( form_.get() );

        if ( auto hsplitter = new QSplitter ) {
            hsplitter->setOrientation( Qt::Horizontal );
            hsplitter->addWidget( table_.get() );
#if QT5_CHARTS            
            if ( auto chartView = new charts::ChartView )
                hsplitter->addWidget( chartView );
#else
            if ( auto chartView = new qwt::ChartView )
                hsplitter->addWidget( chartView );
#endif
            if ( auto w = hsplitter->widget( 1 ) )
                w->hide();

            splitter->addWidget( hsplitter );
        }

        layout_->addWidget( splitter );

        splitter->setStretchFactor( 0, 0 );
        splitter->setStretchFactor( 1, 2 );
    }

    form_->setSqlHistory( QueryDocument::instance()->sqlHistory() );
}

void
QueryQueryWidget::handleConnectionChanged()
{
    ADDEBUG() << "set file: " << QueryDocument::instance()->connection()->filepath();

    if ( auto edit = findChild< QLineEdit * >( Constants::editQueryFilename ) ) {
        edit->setText( QString::fromStdWString( QueryDocument::instance()->connection()->filepath() ) );
    }

    if ( auto conn = QueryDocument::instance()->connection() ) {
        if ( auto form = findChild< QueryQueryForm * >() ) {
            QStringList tables;
            auto query = conn->sqlQuery( "SELECT name FROM sqlite_master WHERE type='table'" );
            while ( query.next() ) 
                tables << query.value( 0 ).toString();
            tables.push_back( "sqlite_master" );

            form->setTableList( tables );

            form->setSqlHistory( QueryDocument::instance()->sqlHistory() );            
            
            QStringList words ( tables );

            if ( QCompleter * completer = new QCompleter( this ) ) {
                QFile file( ":/query/wordlist.txt" );
                if ( file.open( QFile::ReadOnly ) ) {
                    while ( !file.atEnd() ) {
                        QByteArray line = file.readLine();
                        if ( ! line.isEmpty() )
                            words << line.trimmed();
                    }
                }

                for ( auto& table: tables ) 
                    words << table;
                
                query = conn->sqlQuery( "SELECT objuuid FROM AcquiredConf" );
                while ( query.next() ) 
                    words << query.value( 0 ).toString(); // guid

                words.sort( Qt::CaseInsensitive );
                words.removeDuplicates();

                completer->setModel( new QStringListModel( words, completer ) );
                completer->setModelSorting( QCompleter::CaseInsensitivelySortedModel );
                completer->setCaseSensitivity( Qt::CaseInsensitive );
                completer->setWrapAround( false );
                form->setCompleter( completer );
            }
        }
    }

    executeQuery();
}

void
QueryQueryWidget::executeQuery()
{
    if ( auto connection = QueryDocument::instance()->connection() ) {
        form_->setSQL( "SELECT * FROM sqlite_master WHERE type='table'" );

        auto query = connection->sqlQuery( form_->sql() );
        table_->setQuery( query );
        //table_->setDatabae( connection->sqlDatabase() );
    }
}

void
QueryQueryWidget::handleQuery( const QString& sql )
{
    if ( auto connection = QueryDocument::instance()->connection() ) {

        auto query = connection->sqlQuery( sql );

        auto sqlError = query.lastError();
        if ( sqlError.type() != QSqlError::NoError ) {
            QMessageBox::information( this
                                      , tr( "QtPlatz/Query" )
                                      , sqlError.driverText() + "\n" + sqlError.databaseText() );
        } else {
            QueryDocument::instance()->addSqlHistory( sql );
        }
        
        table_->setQuery( query );
    }
}

void
QueryQueryWidget::handlePlot()
{
#if QT5_CHARTS
    typedef charts::ChartView ChartView_t;
#else
    typedef qwt::ChartView ChartView_t;
#endif
    
    if ( auto chart = findChild< ChartView_t * >() ) {

        PlotDialog dlg( this );
        dlg.setModel( table_->model() );

        if ( dlg.exec() ) {

            if ( dlg.clearExisting() )
                chart->clear();

            
            auto type = dlg.chartType();
            auto plots = dlg.plots();
            for( auto& plot: plots ) {

                auto title = std::get< 0 >( plot );
                int iX = std::get< 1 >( plot );
                int iY = std::get< 2 >( plot );

                auto xtitle = table_->model()->headerData( iX, Qt::Horizontal ).toString();
                auto ytitle = table_->model()->headerData( iY, Qt::Horizontal ).toString();
                
                chart->setData( table_->model(), title, iX, iY, xtitle, ytitle, type );
            }
            
            if ( auto chart = findChild< ChartView_t * >() )
                chart->show();
        }
    }
}
