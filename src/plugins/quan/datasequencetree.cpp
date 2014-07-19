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

#include "datasequencetree.hpp"
#include <adcontrols/datafile.hpp>
#include <adcontrols/datasubscriber.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/processeddataset.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/descriptions.hpp>
#include <portfolio/portfolio.hpp>
#include <portfolio/folder.hpp>
#include <portfolio/folium.hpp>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QHeaderView>
#include <boost/filesystem.hpp>
#include <functional>
#include <array>

namespace quan {

    namespace datasequencetree {

        static std::array< const wchar_t *, 3 > extensions = { L".adfs", L".csv", L".txt" };

        enum {
            c_datafile
            , c_sample_attr     // standard | unknown | QC
            , c_trace_selection // chromaotgram generation | 
            , c_level
            , number_of_columns
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
using namespace quan::datasequencetree;

DataSequenceTree::DataSequenceTree(QWidget *parent) : QTreeView(parent)
                                                    , model_( new QStandardItemModel )
{
    auto delegate = new ItemDelegate;
    delegate->register_valueChanged( [=] ( const QModelIndex& idx ){ handleValueChanged( idx ); } );
    setItemDelegate( delegate );
    setModel( model_.get() );
    
    QStandardItemModel& model = *model_;
    model.setColumnCount( number_of_columns );
    model.setHeaderData( c_datafile, Qt::Horizontal, tr("Data name") );
    model.setHeaderData( c_sample_attr, Qt::Horizontal, tr("Sample type") );
    model.setHeaderData( c_trace_selection, Qt::Horizontal, tr("Data manipuration") );
    model.setHeaderData( c_level, Qt::Horizontal, tr("Level") );

    setAcceptDrops( true );


}

void
DataSequenceTree::setData( std::shared_ptr< adcontrols::datafile >& )
{
}

void
DataSequenceTree::setData( const QStringList& )
{
}

void
DataSequenceTree::handleValueChanged( const QModelIndex& )
{
}

void
DataSequenceTree::dropIt( const std::wstring& path )
{
    QStandardItemModel& model = *model_;
    int row = model.rowCount();
    model.insertRow( row );
    model.setData( model.index( row, c_datafile ), QString::fromStdWString( path ) );
}

void
DataSequenceTree::dragEnterEvent( QDragEnterEvent * event )
{
	if ( const QMimeData * mimeData = event->mimeData() ) {
        if ( mimeData->hasUrls() ) {
            event->acceptProposedAction();            
            return;

            QList<QUrl> urlList = mimeData->urls();
            for ( int i = 0; i < urlList.size(); ++i ) {
                boost::filesystem::path path( urlList.at(i).toLocalFile().toStdWString() );
                auto it = std::find_if( extensions.begin(), extensions.end(), [path](const wchar_t * ext){ return path.extension() == ext; });
                if ( it != extensions.end() ) {
                    event->acceptProposedAction();
                    return;
                }
            }
        }
    }
}

void
DataSequenceTree::dragMoveEvent( QDragMoveEvent * event )
{
    event->accept();
}

void
DataSequenceTree::dragLeaveEvent( QDragLeaveEvent * event )
{
	event->accept();
}


void
DataSequenceTree::dropEvent( QDropEvent * event )
{
	if ( const QMimeData * mimeData = event->mimeData() ) {
        if ( mimeData->hasUrls() ) {
            QList<QUrl> urlList = mimeData->urls();
            for ( int i = 0; i < urlList.size(); ++i ) {
                QString file( urlList.at(i).toLocalFile() );
                boost::filesystem::path path( file.toStdWString() );
                auto it = std::find_if( extensions.begin(), extensions.end(), [path](const wchar_t * ext){ return path.extension() == ext; });
                if ( it != extensions.end() ) {
                    dropIt( path.wstring() );
                }
            }
        }
    }
}
