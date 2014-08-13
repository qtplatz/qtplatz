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

#include "quanreportwidget.hpp"
#include "quanconnection.hpp"
#include "quanconstants.hpp"
#include "quandocument.hpp"
#include "quanquery.hpp"
#include "quanqueryform.hpp"
#include "quanresulttable.hpp"
#include <adportable/profile.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <coreplugin/actionmanager/actionmanager.h>
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

using namespace quan;

QuanReportWidget::~QuanReportWidget()
{
}

QuanReportWidget::QuanReportWidget(QWidget *parent) : QWidget(parent)
                                                    , layout_( new QGridLayout )
                                                    , form_( new QuanQueryForm )
                                                    , table_( new QuanResultTable )
{
    auto topLayout = new QVBoxLayout( this );
    topLayout->setMargin( 0 );
    topLayout->setSpacing( 0 );
    topLayout->addLayout( layout_ );

    connect( QuanDocument::instance(), &QuanDocument::onConnectionChanged, this, &QuanReportWidget::handleConnectionChanged );
    connect( form_.get(), &QuanQueryForm::triggerQuery, this, &QuanReportWidget::handleQuery );
    
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
        edit->setObjectName( Constants::editQuanFilename );
        toolBarLayout->addWidget( edit );
    } // end toolbar
    
    layout_->addWidget( form_.get() );
    layout_->addWidget( table_.get() );
    //layout_->setRowStretch( 1, 0 );
    //layout_->setRowStretch( 2, 1 );
}

void
QuanReportWidget::handleConnectionChanged()
{
    if ( auto edit = findChild< QLineEdit * >( Constants::editQuanFilename ) )
        edit->setText( QString::fromStdWString( QuanDocument::instance()->connection()->filepath() ) );
    executeQuery();
}

void
QuanReportWidget::executeQuery()
{
    if ( auto connection = QuanDocument::instance()->connection() ) {
        form_->setSQL(
            "SELECT dataSource, row, level, formula, mass, intensity, sampleType FROM QuanSample,QuanResponse \
WHERE QuanSample.id = idSample AND formula like '%' ORDER BY formula" );
        std::wstring sql = form_->sql().toStdWString();
        if ( auto query = connection->query() ) {
            if ( query->prepare( sql ) ) {
                table_->prepare( *query );
                while ( query->step() == adfs::sqlite_row ) {
                    table_->addRecord( *query );
                }
            }
        }
    }
}

void
QuanReportWidget::handleQuery( const QString& sql )
{
    if ( auto connection = QuanDocument::instance()->connection() ) {
        if ( auto query = connection->query() ) {
            
            qtwrapper::waitCursor wait;

            std::wstring wsql = sql.toStdWString();

            wsql.erase( std::remove( wsql.begin(), wsql.end(), '\\' ) );
            
            if ( query->prepare( wsql ) ) {
                table_->prepare( *query );
                while ( query->step() == adfs::sqlite_row ) {
                    table_->addRecord( *query );
                }
            }
        }
    }
}
