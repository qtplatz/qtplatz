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
#include <qtwrapper/waitcursor.hpp>
#include <utils/styledbar.h>
#include <QBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QSpacerItem>
#include <QStandardItemModel>
#include <QLineEdit>
#include <boost/lexical_cast.hpp>
#include <workaround/boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/format.hpp>

using namespace quan;

QuanResultWidget::QuanResultWidget(QWidget *parent) :  QWidget(parent)
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

    table_->setColumnHide( "uuid" );

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
        if ( auto query = conn->query() ) {

            if ( query->prepare( sqlString ) ) {

                table_->prepare( *query );
                while ( query->step() == adfs::sqlite_row ) {
                    table_->addRecord( *query );
                }                
            }

        }
    }
}

void
QuanResultWidget::CountingIndexChanged( int idx )
{
    bool isISTD = QuanDocument::instance()->quanMethod().isInternalStandard();
    std::string query;
    if ( isISTD ) {

        bool hasIsCounting( true );
        
        // if ( auto conn = connection_.lock() ) {
        //     if ( auto query = conn->query() ) {
        //         if ( query->prepare( "PRAGMA table_info( 'QuanCompound')" ) )
                    
        //     }
        // }
        
        query = hasIsCounting ? 
            "SELECT t1.uuid as 'uuid'"
            ", t1.id as id"
            ", t1.idSample as idSample"
            ", t1.name as name"
            ", t1.sampleType as sampletype"
            ", t1.level as level"
            ", t1.formula as formula"
            ", t1.isCounting as isCounting"
            ", t1.mass as mass"
            ", t1.error as 'error(mDa)'"
            ", t1.CountRate"
            ", t2.formula as formula"
            ", t2.CountRate as CountRate"
            ", t1.CountRate/t2.CountRate AS 'Ratio'"
            ", amount"
            ", trigCounts,dataSource"
            " FROM "
            "(SELECT QuanCompound.uuid, QuanResponse.id, QuanSample.name,idSample"
            ", sampleType, QuanSample.level, QuanCompound.formula"
            ", QuanCompound.isCounting"
            ", QuanCompound.mass AS 'exact mass', QuanResponse.mass"
            ", (QuanCompound.mass - QuanResponse.mass) * 1000 AS 'error'"
            ", intensity * 1000 / trigCounts as 'CountRate', trigCounts, QuanResponse.amount, QuanCompound.description, dataSource"
            " FROM QuanSample, QuanResponse, QuanCompound"
            " WHERE QuanSample.id = idSample"
            " AND QuanResponse.idCmpd = QuanCompound.uuid %1%) t1"
            " LEFT JOIN"
            " (SELECT idSample, intensity * 1000 / trigCounts as 'CountRate',QuanResponse.formula,QuanResponse.mass"
            " FROM QuanResponse,QuanCompound"
            " WHERE QuanResponse.idCmpd=QuanCompound.uuid AND isISTD=1) t2"
            " ON t1.idSample=t2.idSample ORDER BY t1.idSample"
            :
            "SELECT t1.uuid as 'uuid'"
            ", t1.id as id"
            ", t1.idSample as idSample"
            ", t1.name as name"
            ", t1.sampleType as sampletype"
            ", t1.level as level"
            ", t1.formula as formula"
            ", t1.mass as mass"
            ", t1.error as 'error(mDa)'"
            ", t1.CountRate"
            ", t2.formula as formula"
            ", t2.CountRate as CountRate"
            ", t1.CountRate/t2.CountRate AS 'Ratio'"
            ", amount"
            ", trigCounts,dataSource"
            " FROM "
            "(SELECT QuanCompound.uuid, QuanResponse.id, QuanSample.name,idSample"
            ", sampleType, QuanSample.level, QuanCompound.formula"
            ", QuanCompound.mass AS 'exact mass', QuanResponse.mass"
            ", (QuanCompound.mass - QuanResponse.mass) * 1000 AS 'error'"
            ", intensity * 1000 / trigCounts as 'CountRate', trigCounts, QuanResponse.amount, QuanCompound.description, dataSource"
            " FROM QuanSample, QuanResponse, QuanCompound"
            " WHERE QuanSample.id = idSample"
            " AND QuanResponse.idCmpd = QuanCompound.uuid %1%) t1"
            " LEFT JOIN"
            " (SELECT idSample, intensity * 1000 / trigCounts as 'CountRate',QuanResponse.formula,QuanResponse.mass"
            " FROM QuanResponse,QuanCompound"
            " WHERE QuanResponse.idCmpd=QuanCompound.uuid AND isISTD=1) t2"
            " ON t1.idSample=t2.idSample ORDER BY t1.idSample";
        
    } else {
        query =
            "SELECT QuanCompound.uuid, QuanResponse.id, QuanSample.name"
            ", sampleType, QuanCompound.formula, QuanCompound.mass AS \"exact mass\""
            ", QuanResponse.mass"
            ", (QuanCompound.mass - QuanResponse.mass) * 1000 AS 'error(Da)'"
            ", intensity * 1000 / trigCounts as 'CountRate', QuanResponse.amount, QuanCompound.description, dataSource"
            " FROM QuanSample, QuanResponse, QuanCompound"
            " WHERE QuanSample.id = QuanResponse.idSample"
            " AND QuanResponse.idCmpd = QuanCompound.uuid"
            " %1%"
            " ORDER BY QuanCompound.id";
    }


    if ( idx == 0 ) { // All
        execQuery( ( boost::format( query ) % "" ).str() );
    } else if ( idx == 1 ) { // Unknown
        execQuery( ( boost::format( query ) % " AND sampleType = 0" ).str() );
    } else if ( idx == 2 ) { // Standard
        execQuery( ( boost::format( query ) % " AND sampleType = 1" ).str() );
    } else if ( idx == 3 ) { // QC
        execQuery("SELECT QuanCompound.uuid, QuanSample.name, sampleType, QuanCompound.formula"
                  ", QuanCompound.mass AS \"exact mass\", QuanResponse.mass "
                  ", (QuanCompound.mass - QuanResponse.mass) * 1000 AS 'error(mDa)'"
                  ", intensity * 60000 / trigCounts as 'counts/min', QuanSample.level, QuanAmount.amount, QuanCompound.description"
                  ", sampleType, dataSource"
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
                  ", intensity * 60000 / trigCounts as 'counts/min', QuanSample.level, QuanAmount.amount, QuanCompound.description"
                  ", sampleType, dataSource"
                  " FROM QuanSample, QuanResponse, QuanCompound, QuanAmount"
                  " WHERE QuanSample.id = QuanResponse.idSample"
                  " AND QuanResponse.idCmpd = QuanCompound.uuid"
                  " AND sampleType = 3"
                  " AND QuanAmount.idCompound = QuanCompound.id AND QuanAmount.level = QuanSample.level"
                  " ORDER BY QuanCompound.id, QuanSample.level" );
    }
    
}

void
QuanResultWidget::handleIndexChanged( int idx )
{
    if ( auto conn = connection_.lock() ) {
        adfs::stmt sql( conn->db() );
        sql.prepare( "SELECT isCounting, isISTD FROM QuanMethod LIMIT(1)" );
        bool isCounting( false ), isISTD( false );
        while ( sql.step() == adfs::sqlite_row ) {
            isCounting = sql.get_column_value< int64_t >( 0 );
            isISTD = sql.get_column_value< int64_t >( 1 );
        }
        if ( isCounting ) {
            CountingIndexChanged( idx );
            return;
        }
    }

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
}

void
QuanResultWidget::handleCurrentChanged( const QModelIndex& index )
{
    int column = table_->findColumn( "id" );
    if ( column >= 0 ) {
        int respId = table_->model().index( index.row(), column ).data().toInt();
        emit onResponseSelected( respId );
    }
}

void
QuanResultWidget::setCompoundSelected( const std::set< boost::uuids::uuid >& uuids )
{
    int column = table_->findColumn( "uuid" );
    if ( !uuids.empty() ) {
        int rows = table_->model().rowCount();
        for ( int row = 0; row < rows; ++row ) {
            std::string data = table_->model().index( row, column ).data().toString().toStdString();
            auto uuid = boost::lexical_cast<boost::uuids::uuid>(data);
            if ( uuids.find( uuid ) == uuids.end() )
                table_->setRowHidden( row, true );
            else
                table_->setRowHidden( row, false );
        }
    }
}
