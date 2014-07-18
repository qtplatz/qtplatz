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

#include "dataitemselector.hpp"
#include <adcontrols/datafile.hpp>
#include <adcontrols/datasubscriber.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/processeddataset.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/descriptions.hpp>
#include <portfolio/portfolio.hpp>
#include <portfolio/folder.hpp>
#include <portfolio/folium.hpp>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QHeaderView>
#include <functional>
#include <qdebug.h>

namespace quan {
    namespace dataitemselector {

        enum {
            r_datasource
            , r_rawdata
            , r_processed
            , number_of_rows
        };

        class ItemDelegate : public QStyledItemDelegate {
        public:
            void paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
                QStyleOptionViewItem op( option );
                QStyledItemDelegate::paint( painter, op, index );
            }
            void setEditorData( QWidget * editor, const QModelIndex& index ) const {
                QStyledItemDelegate::setEditorData( editor, index );
            }
            void setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex& index ) const {
                QStyledItemDelegate::setModelData( editor, model, index );
                if ( valueChanged_ )
                    valueChanged_( index );
            }
            
            QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
                return QStyledItemDelegate::sizeHint( option, index );
            }

            void register_valueChanged( std::function<void( const QModelIndex& )> f ) { 
                valueChanged_ = f;
            }
        private:
            std::function<void( const QModelIndex& )> valueChanged_;
        };

        class dataSubscriber : public adcontrols::dataSubscriber {
        public:
            dataSubscriber( std::shared_ptr< adcontrols::datafile >& ptr ) : datafile_( ptr )
                                                                           , raw_( 0 ) {
            }
            bool subscribe( const adcontrols::LCMSDataset& d ) override {
                raw_ = &d;
                size_t pos = d.make_pos( 0, 0 ); // find first data for  protoId = 0
                ms_ = std::make_shared< adcontrols::MassSpectrum >();
                d.getSpectrum( -1, pos, *ms_ );
                return true;
            }
            bool subscribe( const adcontrols::ProcessedDataset& d ) override {
                portfolio_ = std::make_shared< portfolio::Portfolio >( d.xml() );
                return true;
            }
            const adcontrols::LCMSDataset * raw() {
                return raw_;
            }
            portfolio::Portfolio * processed() {
                return portfolio_.get();
            }
            adcontrols::MassSpectrum * ms() { return ms_.get(); }

        private:
            std::shared_ptr< adcontrols::MassSpectrum > ms_;
            std::weak_ptr< adcontrols::datafile > datafile_;
            std::shared_ptr< portfolio::Portfolio > portfolio_;
            const adcontrols::LCMSDataset * raw_;
        };
    }
}

using namespace quan;
using namespace quan::dataitemselector;

DataItemSelector::~DataItemSelector()
{
}

DataItemSelector::DataItemSelector(QWidget *parent) :  QTreeView(parent)
                                                     , model_( std::make_shared< QStandardItemModel >(this) )
{
    auto delegate = new ItemDelegate;
    delegate->register_valueChanged( [=] ( const QModelIndex& idx ){ handleValueChanged( idx ); } );
    setItemDelegate( delegate );
    setModel( model_.get() );
    header()->setVisible( false );

    QStandardItemModel& model = *model_;

    model.setColumnCount( 3 );
    model.setRowCount( number_of_rows );
    
    model.setData( model.index( r_datasource, 0 ), "data source" );
    model.setData( model.index( r_datasource, 1 ), "n/a" );

    model.setData( model.index( r_rawdata,    0 ), "raw data" );
    model.setData( model.index( r_rawdata, 1 ), "none" );

    model.setData( model.index( r_processed,    0 ), "processed" );

    resizeColumnToContents( 0 );
}

void
DataItemSelector::clearData()
{
}

void
DataItemSelector::setRaw( QStandardItem * parent )
{
    QStandardItemModel& model = *model_;    
    if ( dataSubscriber * data = dynamic_cast<dataSubscriber *>(subscriber_.get()) ) {
        if ( auto raw = data->raw() ) {
            int n = int( raw->getFunctionCount() );
            parent->setRowCount( n );
            parent->setColumnCount( 3 );

            for ( int fcn = 0; fcn < n; ++fcn )
                model.setData( model.index( fcn, 0, parent->index() ), QString( "TIC.%1" ).arg( fcn + 1 ), Qt::EditRole );

            if ( data->ms() ) {
                adcontrols::segment_wrapper<> segs( *data->ms() );
                int fcn = 0;
                for ( auto& fms : segs ) {
                    std::wstring desc = fms.getDescriptions().toString();
                    model.setData( model.index( fcn++, 1, parent->index() ), QString::fromStdWString( desc ), Qt::EditRole );
                }
            }
        }
    }
}

void
DataItemSelector::setProcessed( QStandardItem * parent )
{
    QStandardItemModel& model = *model_;

    if ( dataSubscriber * data = dynamic_cast<dataSubscriber *>(subscriber_.get()) ) {

        if ( auto pf = data->processed() ) {

            int nrows = int( pf->folders().size() );
            parent->setRowCount( nrows );
            parent->setColumnCount( 3 );
            int row1 = 0;
            for ( auto& folder : pf->folders() ) {

                std::wstring name = folder.name();
                model.setData( model.index( row1, 0, parent->index() ), QString::fromStdWString( name ), Qt::EditRole );
                    
                if ( auto parent2 = model.itemFromIndex( model.index( row1, 0, parent->index() ) ) ) {
                        
                    parent2->setRowCount( int( folder.folio().size() ) );
                    parent2->setColumnCount( 3 );
                    int row2 = 0;
                            
                    for ( auto& folium : folder.folio() ) {
                        model.setData( model.index( row2, 0, parent2->index() ), QString::fromStdWString( folium.name() ), Qt::EditRole );
                            
                        if ( auto parent3 = model.itemFromIndex( model.index( row2, 0, parent2->index() ) ) ) {
                                
                            parent3->setRowCount( int( folium.attachments().size() ) );
                            parent3->setColumnCount( 3 );
                            int row3 = 0;
                                
                            for ( auto& att : folium.attachments() ) {
                                model.setData( model.index( row3, 0, parent3->index() ), QString::fromStdWString( att.name() ) );
                                ++row3;
                            }
                        }
                        ++row2;
                    }
                }
                ++row1;
            }
        }
    }
}

void
DataItemSelector::setData( std::shared_ptr< adcontrols::datafile >& ptr )
{
    if ( ptr ) {
        QStandardItemModel& model = *model_;
        
        model.itemFromIndex( model.index( r_rawdata, 0 ) )->setRowCount( 0 );
        model.itemFromIndex( model.index( r_rawdata + 1, 0 ) )->setRowCount( 0 );

        if ( subscriber_ = std::make_shared< dataSubscriber >( ptr ) ) {

            model_->setData( model_->index( 0, 1 ), QString::fromStdWString( ptr->filename() ), Qt::EditRole );
            ptr->accept( *subscriber_ );

            setRaw( model_->itemFromIndex( model_->index( r_rawdata, 0 ) ) );
            setProcessed( model_->itemFromIndex( model_->index( r_rawdata + 1, 0 ) ) );

        }
    }
    this->expandAll();
    resizeColumnToContents( 0 );
    resizeColumnToContents( 1 );
    resizeColumnToContents( 2 );
}

void
DataItemSelector::handleValueChanged( const QModelIndex& )
{
}
