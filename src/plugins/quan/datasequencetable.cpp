/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "datasequencetable.hpp"
#include "quanconstants.hpp"
#include "quandocument.hpp"
#include <adcontrols/datafile.hpp>
#include <adcontrols/datasubscriber.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/processeddataset.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/quansequence.hpp>
#include <adcontrols/quansample.hpp>
#include <adcontrols/quanmethod.hpp>
#include <adlog/logger.hpp>
#include <adportable/debug.hpp>
#include <portfolio/portfolio.hpp>
#include <portfolio/folder.hpp>
#include <portfolio/folium.hpp>

#include <QApplication>
#include <QBrush>
#include <QClipboard>
#include <QComboBox>
#include <QDragEnterEvent>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMenu>
#include <QMimeData>
#include <QMessageBox>
#include <QPainter>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QDebug>

#include <boost/exception/all.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <functional>
#include <array>
#include <thread>
#include <mutex>
#include <set>

namespace quan {

    namespace datasequencetable {

        static std::array< const wchar_t *, 3 > extensions = { { L".adfs", L".csv", L".txt" } };

        enum {
            r_rowdata
            , r_processed
        };

        enum {
            c_datafile
            , c_sample_type     // standard | unknown | QC
            , c_process         // chromaotgram generation |
            , c_level
            , c_description
            , number_of_columns
        };

        static const QStringList sample_type_names = { "UNK", "STD", "QC", "BLANK" };    
        static const QStringList process_names = { "AS-IS", "Spectrum", "Chromatogram", "Raw Spectra" };

        class ItemDelegate : public QStyledItemDelegate { 
        public:

            void paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {

                QStyleOptionViewItem op( option );

                painter->save();

                auto samp_type = index.model()->index( index.row(), c_sample_type ).data( Qt::EditRole ).toInt();
                int level = index.model()->index( index.row(), c_level, index.parent() ).data().toInt();
                
                if ( samp_type == adcontrols::QuanSample::SAMPLE_TYPE_STD ) {
                    if ( level <= 0 || level > levels_ )
                        painter->fillRect( option.rect, QColor( 0xff, 0x66, 0x44, 0x40 ) ); // tomato (error; either this is, or number of levels in QuanMethod)
                    else
                        painter->fillRect( option.rect, QColor( 174, 198, 207, 0x20 ) ); // pastel blue
                } else {
                    painter->fillRect( option.rect, QColor( 119, 221, 119, 0x10 ) ); // pastel green (UNK/QC/BLANK)
                }

                if ( index.column() == c_datafile ) {
                    op.textElideMode = Qt::ElideLeft;
                    QStyledItemDelegate::paint( painter, op, index );
                } else if ( index.column() == c_sample_type ) {
                    painter->drawText( op.rect, Qt::AlignHCenter | Qt::AlignVCenter, sample_type_names.at( samp_type ) );
                } else if ( index.column() == c_process ) {
                    painter->drawText( op.rect, Qt::AlignHCenter | Qt::AlignVCenter, process_names.at( index.data( Qt::EditRole ).toInt() ) );
                } else {
                    QStyledItemDelegate::paint( painter, op, index );
                }
                painter->restore();                
            }
            
            void setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex& index ) const {
                if ( index.column() == c_sample_type ) {
                    if ( auto combo = qobject_cast<QComboBox *>( editor ) ) {
                        int idx = combo->currentIndex();
                        model->setData( index, combo->currentIndex(), Qt::EditRole );
                    }
                } else {
                    QStyledItemDelegate::setModelData( editor, model, index );
                }
                if ( valueChanged_ )
                    valueChanged_( index );
            }
            
            QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
                return QStyledItemDelegate::sizeHint( option, index );
            }

            QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
                if ( index.column() == c_sample_type ) {
                    QComboBox * pCombo = new QComboBox( parent );
                    pCombo->addItems( sample_type_names );
                    return pCombo;
                }
                return QStyledItemDelegate::createEditor( parent, option, index );
            }
            
            void register_valueChanged( std::function<void( const QModelIndex& )> f ) { 
                valueChanged_ = f;
            }
            
            void levels( int value ) { levels_ = value; }
            void replicates( int value ) { replicates_ = value; }
        private:
            std::function<void( const QModelIndex& )> valueChanged_;
            int levels_;
            int replicates_;
        };

        ////////////////
        struct Chromatography {
            
            static void setRow( QStandardItemModel& model, int row, const adcontrols::QuanSample& sample ) {
                
                model.setData( model.index( row, c_datafile ), QString::fromStdWString( sample.dataSource() ) );
                model.setData( model.index( row, c_sample_type ), sample.sampleType() );
                model.setData( model.index( row, c_process ), sample.dataGeneration() ); // | Chromatogram Generation
                if ( sample.sampleType() == adcontrols::QuanSample::SAMPLE_TYPE_STD )
                    model.setData( model.index( row, c_level ), sample.level() ); // n/a for UNK
                else
                    model.setData( model.index( row, c_level ), "n/a" );
                model.setData( model.index( row, c_description ), QString::fromStdWString( sample.description() ) );
            }
            
            static void getRow( QStandardItemModel& model, int row, adcontrols::QuanSample& sample ) {
                
                sample.dataSource( model.index( row, c_datafile ).data( Qt::EditRole ).toString().toStdWString().c_str() );
                sample.sampleType( adcontrols::QuanSample::QuanSampleType( model.index( row, c_sample_type ).data( Qt::EditRole ).toInt() ) );
                sample.dataGeneration( adcontrols::QuanSample::QuanDataGeneration( model.index( row, c_process ).data( Qt::EditRole ).toInt() ) );
                if ( sample.sampleType() == adcontrols::QuanSample::SAMPLE_TYPE_STD )
                    sample.level( model.index( row, c_level ).data( Qt::EditRole ).toInt() );
                else
                    sample.level( 0 );
                sample.description( model.index( row, c_description ).data().toString().toStdWString().c_str() );

                sample.inletType( adcontrols::QuanSample::Chromatography );                
            }
        };
    }
}

using namespace quan;
using namespace quan::datasequencetable;

DataSequenceTable::DataSequenceTable(QWidget *parent) : adwidgets::TableView(parent)
                                                      , model_( new QStandardItemModel )
                                                      , dropCount_( 0 )
{
    auto delegate = new ItemDelegate;

    setItemDelegate( delegate );
    setModel( model_.get() );
    
    auto& qm = QuanDocument::instance()->quanMethod();
    delegate->levels( qm.levels() );
    delegate->replicates( qm.replicates() );

    delegate->register_valueChanged( [=] ( const QModelIndex& idx ){ handleValueChanged( idx ); } );

    onInitialUpdate();
}

void
DataSequenceTable::onInitialUpdate()
{
    QStandardItemModel& model = *model_;
    model.setRowCount( 0 );
    model.setColumnCount( number_of_columns );

    model.setHeaderData( c_datafile, Qt::Horizontal, tr("Data name") );       // read only

    model.setHeaderData( c_sample_type, Qt::Horizontal, tr("Sample type") );  // UNK/STD/QC
    model.setHeaderData( c_process, Qt::Horizontal, tr("Data Generation") );  // Average (start,end)
    model.setHeaderData( c_level, Qt::Horizontal, tr("Level") );              // if standard

    model.setHeaderData( c_description, Qt::Horizontal, tr("Description") );  // if standard

    resizeColumnsToContents();
    resizeRowsToContents();
    //horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );

    setAcceptDrops( true );
    setContextMenuPolicy( Qt::CustomContextMenu );
    connect( this, &TableView::customContextMenuRequested, this, &DataSequenceTable::handleContextMenu );
}

void
DataSequenceTable::setData( std::shared_ptr< adcontrols::datafile >& )
{
}

void
DataSequenceTable::setData( const QStringList& list )
{
    for ( int i = 0; i < list.size(); ++i ) {
        boost::filesystem::path path( list.at( i ).toStdWString() );
        auto it = std::find_if( extensions.begin(), extensions.end(), [path](const wchar_t * ext){ return path.extension() == ext; });
        if ( it != extensions.end() ) {
            dropIt( path.wstring() );
        }
    }
}

void
DataSequenceTable::handleValueChanged( const QModelIndex& index )
{
    QStandardItemModel& model = *model_;

    if ( index.column() == c_sample_type ) {

        int level = 0;
        if ( index.data( Qt::EditRole ).toInt() == adcontrols::QuanSample::SAMPLE_TYPE_STD ) {
            if ( ( level = model.index( index.row(), c_level ).data().toInt() ) == 0 )
                level = 1;
            // Level for STD must be >1
            model.setData( model.index( index.row(), c_level, index.parent() ), level );
            model.itemFromIndex( model.index( index.row(), c_level ) )->setEditable( true );
        } else {
            // UNK/QC/BLAND must be 0 (n/a)
            model.setData( model.index( index.row(), c_level ), "n/a" ); 
            model.itemFromIndex( model.index( index.row(), c_level ) )->setEditable( false );
        }
    }
}

void
DataSequenceTable::dropIt( const std::wstring& path )
{
    QStandardItemModel& model = *model_;
    int row = model.rowCount();

    model.insertRow( row );
    adcontrols::QuanSample sample;
    
    sample.dataSource( path.c_str() );
    sample.inletType( adcontrols::QuanSample::Chromatography );
    sample.dataGeneration( adcontrols::QuanSample::GenerateChromatogram );
    sample.level( 1 );
    sample.injVol( 1.0 );
    // sample.addedAmounts( 0 ); // for infusion
    // istd[] to be added
    Chromatography::setRow( model, row, sample );
    // model.setData( model.index( row, c_datafile ), QString::fromStdWString( path ) );
    ++dropCount_;
}

void
DataSequenceTable::dragEnterEvent( QDragEnterEvent * event )
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
DataSequenceTable::dragMoveEvent( QDragMoveEvent * event )
{
    event->accept();
}

void
DataSequenceTable::dragLeaveEvent( QDragLeaveEvent * event )
{
	event->accept();
}

void
DataSequenceTable::dropEvent( QDropEvent * event )
{
	if ( const QMimeData * mimeData = event->mimeData() ) {
        if ( mimeData->hasUrls() ) {
            QList<QUrl> urlList = mimeData->urls();
            for ( int i = 0; i < urlList.size(); ++i ) {
                QString file( urlList.at(i).toLocalFile() );
                boost::filesystem::path path( file.toStdWString() );
                auto it = std::find_if( extensions.begin(), extensions.end(), [path](const wchar_t * ext){ return path.extension() == ext; });
                if ( it != extensions.end() )
                    dropIt( path.wstring() );
            }
        }
    }
}

bool
DataSequenceTable::getContents( adcontrols::QuanSequence& seq )
{
    QStandardItemModel& model = *model_;    
    for ( int row = 0; row < model.rowCount(); ++row ) {

        adcontrols::QuanSample sample;
        Chromatography::getRow( *model_, row, sample );
        seq << sample;

    }
    return true;
}

bool
DataSequenceTable::setContents( const adcontrols::QuanSequence& seq )
{
    QStandardItemModel& model = *model_;

    if ( seq.size() == 0 )
        return true;

    model.setRowCount( int( seq.size() ) );
    int row = 0;
    for ( auto sample : seq ) {
        Chromatography::setRow( model, row++, sample );// QString::fromStdWString( sample.dataSource() ) );
    }
    return true;
}

void
DataSequenceTable::handleContextMenu( const QPoint& pt )
{
    QAction * delete_action;
    QMenu menu;

    delete_action = menu.addAction( tr( "Delete line" ), this, SLOT( delLine() ) );
    auto index = currentIndex();
    if ( !index.isValid() )
        delete_action->setEnabled( false );

    TableView::addActionsToMenu( menu, pt );
    
    menu.exec( this->mapToGlobal( pt ) );
}

void
DataSequenceTable::delAll()
{
    model_->setRowCount( 0 );
}

void
DataSequenceTable::delLine()
{
    QModelIndex index = currentIndex();
    model_->removeRow( index.row(), index.parent() );
}

void
DataSequenceTable::addLine()
{
    // QModelIndex index = currentIndex();
    // model_->removeRow( index.row(), index.parent() );
    QMessageBox::information( 0, "DataSequenceTable", "Not yet implemented, Sorry" );
}

void
DataSequenceTable::handleLevelChanged( int value )
{
    if ( auto delegate = dynamic_cast<ItemDelegate *>( itemDelegate() ) )
        delegate->levels( value );
}

void
DataSequenceTable::handleReplicatesChanged( int value )
{
    if ( auto delegate = dynamic_cast<ItemDelegate *>( itemDelegate() ) )
        delegate->replicates( value );
}

