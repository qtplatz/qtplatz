/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <adportable/profile.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <qtwrapper/progresshandler.hpp>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/progressmanager/progressmanager.h>
#include <utils/styledbar.h>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QGridLayout>
#include <QToolButton>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QStandardItemModel>
#include <boost/filesystem.hpp>
#include <boost/exception/all.hpp>
#include <fstream>
#include <algorithm>

using namespace query;

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
    connect( form_.get(), &QueryQueryForm::triggerQuery, this, &QueryQueryWidget::handleQuery );
    
    if ( auto toolBar = new Utils::StyledBar ) {

        layout_->addWidget( toolBar );
        
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 2 );
        toolBarLayout->setSpacing( 2 );

        if ( auto btnOpen = new QToolButton ) {
            btnOpen->setDefaultAction( Core::ActionManager::instance()->command( Constants::FILE_OPEN )->action() );
            btnOpen->setToolTip( tr("Open result file...") );
            toolBarLayout->addWidget( btnOpen );
        }

        auto edit = new QLineEdit;
        edit->setObjectName( Constants::editQueryFilename );
        toolBarLayout->addWidget( edit );
    } // end toolbar
    
    layout_->addWidget( form_.get() );
    layout_->addWidget( table_.get() );
    //layout_->setRowStretch( 1, 0 );
    //layout_->setRowStretch( 2, 1 );
    //bool rcode = connect( this, &QueryQueryWidget::onQueryData, &QueryQueryWidget::handleQueryData );
}

void
QueryQueryWidget::handleConnectionChanged()
{
    if ( auto edit = findChild< QLineEdit * >( Constants::editQueryFilename ) )
        edit->setText( QString::fromStdWString( QueryDocument::instance()->connection()->filepath() ) );

    if ( auto conn = QueryDocument::instance()->connection() ) {
        if ( auto form = findChild< QueryQueryForm * >() ) {
            QList< QString > tables, sublist;
            auto query = conn->sqlQuery( "SELECT * FROM sqlite_master WHERE type='table'" );
            while ( query.next() )
                tables.push_back( query.value( 1 ).toString() );
            form->setTableList( tables );

            query = conn->sqlQuery( "SELECT * FROM AcquiredConf" );
            while ( query.next() )
                sublist.push_back( query.value( 0 ).toString() );
            form->setSubList( sublist );
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
        table_->setQuery( query );
    }
}

