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

#include "quanresultwidget.hpp"
#include "quanresulttable.hpp"
#include "quanconnection.hpp"
#include "quandocument.hpp"
#include "quanmethod.hpp"
#include "quanquery.hpp"
#include <adportable/debug.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <utils/styledbar.h>
#include <QBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpacerItem>
#include <QStandardItemModel>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/format.hpp>

using namespace quan;

QuanResultWidget::QuanResultWidget(QWidget *parent) : QWidget(parent)
                                                    , currentIndex_( 0 )
{
    auto topLayout = new QVBoxLayout( this );
    topLayout->setMargin( 0 );
    topLayout->setSpacing( 0 );

    if ( auto toolBar = new Utils::StyledBar ) {
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 0 );
        toolBarLayout->setSpacing( 0 );

        toolBarLayout->addWidget( new Utils::StyledSeparator );

        if ( auto label = new QLabel ) {
            label->setText( tr("Results") );
            toolBarLayout->addWidget( label );
        }

        toolBarLayout->addWidget( new Utils::StyledSeparator );

        if ( auto pCombo = new QComboBox ) {
            pCombo->addItems( QStringList() << tr("All") << tr("Unknown") << tr("Standards") << tr("QC") << tr("Blank") );
            toolBarLayout->addWidget( pCombo );
            
            toolBarLayout->addWidget( new Utils::StyledSeparator );            
            toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum ) );

            connect( pCombo, static_cast< void(QComboBox::*)(int) >(&QComboBox::currentIndexChanged), this, &QuanResultWidget::handleIndexChanged );
        }

        toolBarLayout->addWidget( new Utils::StyledSeparator );
        toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );
        toolBarLayout->addWidget( new QLabel( tr("File:") ) );
        toolBarLayout->addWidget( new QLineEdit );
        
        topLayout->addWidget( toolBar ); // <-------- add to toolbar
    }
    if ( ( table_ = new QuanResultTable ) )
        topLayout->addWidget( table_ );

    // table_->setColumnHide( "uuid" );

    connect( table_, &QuanResultTable::onCurrentChanged, this, &QuanResultWidget::handleCurrentChanged );
}

void
QuanResultWidget::setConnection( QuanConnection * connection )
{
    connection_ = connection->shared_from_this();
    handleIndexChanged( 0 );
    if ( auto edit = findChild< QLineEdit * >() ) {
        edit->setText( QString::fromStdWString( connection->filepath() ) );
    }
}

void
QuanResultWidget::execQuery( const std::string& sqlString )
{
    qtwrapper::waitCursor wait;

    if ( auto conn = connection_.lock() ) {

        QSqlQuery sqlQuery( QString::fromStdString( sqlString ), conn->sqlDatabase() );
        
        table_->setQuery( sqlQuery, { "uuid" } );
    }
}

void
QuanResultWidget::handleIndexChanged( int idx )
{
    currentIndex_ = idx;
    if ( auto conn = connection_.lock() ) {

        std::string sql;
        conn->query()->buildQuery( sql, idx, conn->isCounting(), conn->isISTD(), {} );
        execQuery( sql );
        
        // CountingIndexChanged( idx );
        return;
    }
#if 0
    if ( idx == 0 ) { // All
        
        execQuery("SELECT QuanCompound.uuid, QuanResponse.id, QuanSample.name"
                  ", sampleType, QuanSample.level, QuanCompound.formula"
                  ", QuanCompound.mass AS \"exact mass\", QuanResponse.mass"
                  ", (QuanCompound.mass - QuanResponse.mass) * 1000 AS 'error(mDa)'"
                  ", intensity, QuanResponse.amount, QuanCompound.description, dataSource"
                  " FROM QuanSample, QuanResponse, QuanCompound"
                  " WHERE QuanSample.id = QuanResponse.idSample"
                  " AND QuanResponse.idCmpd = QuanCompound.uuid"
                  " ORDER BY QuanCompound.id, QuanSample.level");

    } else if ( idx == 1 ) { // Unknown
        
        execQuery("SELECT QuanCompound.uuid, QuanResponse.id, QuanSample.name"
                  ", sampleType, QuanCompound.formula, QuanCompound.mass AS \"exact mass\""
                  ", QuanResponse.mass"
                  ", (QuanCompound.mass - QuanResponse.mass) * 1000 AS 'error(mDa)'"
                  ", intensity, QuanResponse.amount, QuanCompound.description, dataSource"
                  " FROM QuanSample, QuanResponse, QuanCompound"
                  " WHERE QuanSample.id = QuanResponse.idSample"
                  " AND QuanResponse.idCmpd = QuanCompound.uuid"
                  " AND sampleType = 0 "
                  " ORDER BY QuanCompound.id");
    }
    else if ( idx == 2 ) { // Standard

        execQuery("SELECT QuanCompound.uuid, QuanResponse.id, QuanSample.name"
                  ", sampleType, QuanCompound.formula, QuanCompound.mass AS \"exact mass\""
                  ", QuanResponse.mass"
                  ", (QuanCompound.mass - QuanResponse.mass) * 1000 AS 'error(mDa)'"
                  ", intensity, QuanSample.level, QuanAmount.amount, QuanCompound.description, sampleType, dataSource"
                  " FROM QuanSample, QuanResponse, QuanCompound, QuanAmount"
                  " WHERE QuanSample.id = QuanResponse.idSample"
                  " AND QuanResponse.idCmpd = QuanCompound.uuid"
                  " AND sampleType = 1 "
                  " AND QuanAmount.idCompound = QuanCompound.id AND QuanAmount.level = QuanSample.level"
                  " ORDER BY QuanCompound.id, QuanSample.level");
    }
    else if ( idx == 3 ) { // QC
        execQuery("SELECT QuanCompound.uuid, QuanSample.name, sampleType, QuanCompound.formula"
                  ", QuanCompound.mass AS \"exact mass\", QuanResponse.mass "
                  ", (QuanCompound.mass - QuanResponse.mass) * 1000 AS 'error(mDa)'"
                  ", intensity, QuanSample.level, QuanAmount.amount, QuanCompound.description, sampleType, dataSource"
                  " FROM QuanSample, QuanResponse, QuanCompound, QuanAmount"
                  " WHERE QuanSample.id = QuanResponse.idSample"
                  " AND QuanResponse.idCmpd = QuanCompound.uuid"
                  " AND sampleType = 2"
                  " AND QuanAmount.idCompound = QuanCompound.id AND QuanAmount.level = QuanSample.level"
                  " ORDER BY QuanCompound.id, QuanSample.level");
        
    } else if ( idx == 4 ) { // Blank

        execQuery("SELECT QuanCompound.uuid, QuanSample.name, sampleType, QuanCompound.formula"
                  ", QuanCompound.mass AS \"exact mass\", QuanResponse.mass"
                  ", (QuanCompound.mass - QuanResponse.mass)*1000 AS 'error(mDa)'"
                  ", intensity, QuanSample.level, QuanAmount.amount, QuanCompound.description, sampleType, dataSource"
                  " FROM QuanSample, QuanResponse, QuanCompound, QuanAmount"
                  " WHERE QuanSample.id = QuanResponse.idSample"
                  " AND QuanResponse.idCmpd = QuanCompound.uuid"
                  " AND sampleType = 3"
                  " AND QuanAmount.idCompound = QuanCompound.id AND QuanAmount.level = QuanSample.level"
                  " ORDER BY QuanCompound.id, QuanSample.level");
    }
#endif
}

void
QuanResultWidget::handleCurrentChanged( const QModelIndex& index )
{
    if ( auto model = qobject_cast< const QSqlQueryModel * >( index.model() ) ) {
        int respId = model->record( index.row() ).value( "id" ).toInt();
        emit onResponseSelected( respId );
    }    
}

void
QuanResultWidget::setCompoundSelected( const std::set< boost::uuids::uuid >& uuids )
{
    if ( !uuids.empty() ) {
        if ( auto model = qobject_cast< QSqlQueryModel * >( table_->model() ) ) {
            std::ostringstream additionals;
            int count(0);
            for ( auto& uuid: uuids )
                additionals << boost::format( "%1% QuanCompound.uuid='%2%'" ) % ( count++ ? "OR" : "AND" ) % uuid;
            
            if ( auto conn = connection_.lock() ) {
                std::string sql;                
                if ( conn->query()->buildQuery( sql, currentIndex_, conn->isCounting(), conn->isISTD(), additionals.str() ) ) {
                    QSqlQuery sqlQuery( QString::fromStdString( sql ), conn->sqlDatabase() );
                    // ADDEBUG() << sql;
                    table_->setQuery( sqlQuery, { "uuid" } );
                }
            }
            return;
        }
    }
}
