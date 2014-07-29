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
#include "quanconstants.hpp"
#include <adcontrols/datafile.hpp>
#include <adcontrols/datasubscriber.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/processeddataset.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/quansequence.hpp>
#include <adcontrols/quansample.hpp>
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

#include <boost/filesystem.hpp>
#include <boost/exception/all.hpp>
#include <functional>
#include <array>
#include <thread>
#include <mutex>
#include <set>

namespace quan {

    namespace datasequencetree {

        static std::array< const wchar_t *, 3 > extensions = { { L".adfs", L".csv", L".txt" } };

        enum {
            r_rowdata
            , r_processed
        };

        enum {
            c_datafile
            , c_data_type       // RAW | Spectrum | Chromatogram
            , c_sample_type     // standard | unknown | QC
            , c_process         // chromaotgram generation | 
            , c_level
            , c_channel
            , c_description
            , number_of_columns
        };

        class ItemDelegate : public QStyledItemDelegate {
        public:
            void paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
                QStyleOptionViewItem op( option );
                painter->save();
                std::string data_type = index.model()->index( index.row(), c_data_type, index.parent() ).data().toString().toStdString();
                if ( data_type == "raw" ) {
                    painter->fillRect( option.rect, QColor( 0xff, 0x66, 0x44, 0x10 ) );
                } else if ( data_type == "file" ) {
                    painter->setPen( Qt::gray );
                }
                if ( index.column() == c_datafile ) {
                    op.textElideMode = Qt::ElideLeft;
                    QStyledItemDelegate::paint( painter, op, index );
                    // auto align = Qt::AlignRight | Qt::AlignVCenter;
                    // painter->drawText( op.rect, align, index.data().toString() );
                } else
                    QStyledItemDelegate::paint( painter, op, index );
                painter->restore();                
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

            QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
                if ( index.column() == c_sample_type ) {
                    QComboBox * pCombo = new QComboBox( parent );
                    pCombo->addItems( QStringList() << "UNK" << "STD" << "QC" );
                    return pCombo;
                }
                if ( index.column() == c_process ) {
                    const QAbstractItemModel& model = *index.model();
                    QString data_type = model.index( index.row(), c_data_type, index.parent() ).data().toString();
                    if ( data_type == "raw" ) {
                        QComboBox * pCombo = new QComboBox( parent );
                        pCombo->addItems( QStringList() << Constants::cmbAvgAll << Constants::cmbTake1st << Constants::cmbTake2nd << Constants::cmbTakeLast << Constants::cmbProcEach );
                        return pCombo;
                    } else if ( data_type == "spc" ) {
                        QComboBox * pCombo = new QComboBox( parent );
                        pCombo->addItems( QStringList() << "AS IS" );
                        return pCombo;
                    }
                }
                return QStyledItemDelegate::createEditor( parent, option, index );
            }
            
            void register_valueChanged( std::function<void( const QModelIndex& )> f ) { 
                valueChanged_ = f;
            }
        private:
            std::function<void( const QModelIndex& )> valueChanged_;
        };

        ////////////////

        class dataSubscriber : public adcontrols::dataSubscriber
                             , public std::enable_shared_from_this< dataSubscriber > {
        public:
            dataSubscriber( int row
                            , const std::wstring& filename
                            , std::function<void( dataSubscriber *)> f ) : row_( row )
                                                                         , filename_( filename )
                                                                         , raw_(0)
                                                                         , callback_(f){
            }
            void open() {
                try {
                    if ( ( datafile_ = std::shared_ptr< adcontrols::datafile >( adcontrols::datafile::open( filename_, true ) )  ) )
                        datafile_->accept( *this );
                    callback_( this );
                } catch ( ... ) {
                    ADERROR() << boost::current_exception_diagnostic_information();
                    QMessageBox::warning(0, "Quan dataSubscriber", boost::current_exception_diagnostic_information().c_str() );
                }
            }
            
            // implementation
            bool subscribe( const adcontrols::LCMSDataset& d ) override {
                raw_ = &d;
                try { 
                    size_t pos = d.find_scan( 0, 0 ); // find first data for  protoId = 0
                    ms_ = std::make_shared< adcontrols::MassSpectrum >();
                    d.getSpectrum( -1, pos, *ms_ );
                } catch ( ... ) {
                    ADERROR() << boost::current_exception_diagnostic_information();
                    QMessageBox::warning(0, "Quan dataSubscriber", boost::current_exception_diagnostic_information().c_str() );
                }
                return true;
            }

            bool subscribe( const adcontrols::ProcessedDataset& d ) override {
                portfolio_ = std::make_shared< portfolio::Portfolio >( d.xml() );
                return true;
            }

            // local impl
            const adcontrols::LCMSDataset * raw() {
                return raw_;
            }
            portfolio::Portfolio * processed() {
                return portfolio_.get();
            }
            adcontrols::MassSpectrum * ms() { return ms_.get(); }
            const std::wstring& filename() const { return filename_;  }
            int row() const { return row_; }
        private:
            int row_;
            std::wstring filename_;
            std::shared_ptr< adcontrols::MassSpectrum > ms_;
            std::shared_ptr< adcontrols::datafile > datafile_;
            std::shared_ptr< portfolio::Portfolio > portfolio_;
            const adcontrols::LCMSDataset * raw_;
            std::function< void( dataSubscriber * ) > callback_;
        };

        struct Chromatography {
            static void setRow( QStandardItemModel& model, int row, const QString& data_type, const QModelIndex& parent = QModelIndex()) {
                model.setData( model.index( row, c_data_type, parent ), data_type );
                model.setData( model.index( row, c_sample_type, parent ), "UNK" );
                model.setData( model.index( row, c_process, parent ), "TIC" ); // | Chromatogram Generation
                model.setData( model.index( row, c_level, parent ), 0 ); // level
            }
        };

        struct Infusion {

            static void setRow( QStandardItemModel& model, int row, const QString& data_type, const QModelIndex& parent = QModelIndex(), int ch = 0 ) {
                model.itemFromIndex( model.index( row, c_datafile, parent ) )->setEditable( false ); // not editable
                model.setData( model.index( row, c_data_type, parent ), data_type );
                model.itemFromIndex( model.index( row, c_data_type, parent ) )->setEditable( false );
                model.setData( model.index( row, c_sample_type, parent ), "UNK" );
                if ( data_type == "file" ) {
                    model.itemFromIndex( model.index( row, c_process ) )->setEditable( false );
                } else if ( data_type == "raw" ) {
                    model.setData( model.index( row, c_process, parent ), Constants::cmbAvgAll );
                    if ( parent != QModelIndex() )
                        model.itemFromIndex( model.index( row, c_sample_type, parent ) )->setEditable( false );
                    model.setData( model.index( row, c_channel, parent ), ch );
                } else if ( data_type == "spc" ) {
                    model.setData( model.index( row, c_process, parent ), "AS IS" );
                }
                model.setData( model.index( row, c_level, parent ), 0 ); // level
            }

            static void setRow( QStandardItemModel& model
                                , int row
                                , const adcontrols::QuanSample& sample
                                , const QModelIndex& parent = QModelIndex() ) {

                model.itemFromIndex( model.index( row, c_datafile, parent ) )->setEditable( false ); // not editable

                // name ( datafile )
                model.setData( model.index( row, c_datafile, parent ), QString::fromStdWString( sample.name() ) );

                // data_type
                model.setData( model.index( row, c_data_type, parent ), QString::fromStdWString( sample.dataType() ) );
                model.itemFromIndex( model.index( row, c_data_type, parent ) )->setEditable( false );

                QString sample_type_string = "?";
                switch ( sample.sampleType() ) {
                case adcontrols::QuanSample::SAMPLE_TYPE_UNKNOWN: sample_type_string = "UNK"; break;
                case adcontrols::QuanSample::SAMPLE_TYPE_STD:     sample_type_string = "STD"; break;
                case adcontrols::QuanSample::SAMPLE_TYPE_QC:      sample_type_string = "QC"; break;
                case adcontrols::QuanSample::SAMPLE_TYPE_BLANK:   sample_type_string = "BLANK"; break;
                }
                model.setData( model.index( row, c_sample_type, parent ), sample_type_string );
                model.setData( model.index( row, c_channel, parent ), sample.channel() );
                model.setData( model.index( row, c_level, parent ), sample.level() );

                if ( sample.dataGeneration() == adcontrols::QuanSample::ASIS ) {
                    model.setData( model.index( row, c_process, parent ), "AS IS" );
                }
                else if ( sample.dataGeneration() == adcontrols::QuanSample::GenerateSpectrum ) {
                    std::pair< uint32_t, uint32_t > range = std::make_pair( sample.scan_range_first(), sample.scan_range_second() );
                    if ( range.first == 0 && range.second == 0 ) // take 1st
                        model.setData( model.index( row, c_process, parent ), "Take 1st spc." );
                    if ( range.first == 1 && range.second == 1 ) // take 2nd
                        model.setData( model.index( row, c_process, parent ), "Take 2nd spc." );
                    if ( range.first == uint32_t( -1 ) ) // take last
                        model.setData( model.index( row, c_process, parent ), "Take last spc." );
                    if ( range.first == 0 && range.second == uint32_t( -1 ) ) // average all
                        model.setData( model.index( row, c_process, parent ), "Average all" );
                }
            }
        };

    }
}

using namespace quan;
using namespace quan::datasequencetree;

DataSequenceTree::DataSequenceTree(QWidget *parent) : QTreeView(parent)
                                                    , model_( new QStandardItemModel )
                                                    , dropCount_( 0 )
{
    auto delegate = new ItemDelegate;
    delegate->register_valueChanged( [=] ( const QModelIndex& idx ){ handleValueChanged( idx ); } );
    setItemDelegate( delegate );
    setModel( model_.get() );
    
    QStandardItemModel& model = *model_;
    model.setColumnCount( number_of_columns );
    model.setHeaderData( c_datafile, Qt::Horizontal, tr("Data name") );       // read only
    model.setHeaderData( c_data_type, Qt::Horizontal, tr("Data type") );      // read only
    model.setHeaderData( c_sample_type, Qt::Horizontal, tr("Sample type") );  // UNK/STD/QC
    model.setHeaderData( c_process, Qt::Horizontal, tr("Spectrum method") );  // Average (start,end)
    model.setHeaderData( c_level, Qt::Horizontal, tr("Level") );              // if standard
    model.setHeaderData( c_channel, Qt::Horizontal, tr("ch#") );              // data channel (usually same as protocol#, but depends)
    model.setHeaderData( c_description, Qt::Horizontal, tr("Description") );  // if standard

    connect( this, &DataSequenceTree::onJoin, this, &DataSequenceTree::handleJoin );

    setAcceptDrops( true );
    setContextMenuPolicy( Qt::CustomContextMenu );
    connect( this, &QTreeView::customContextMenuRequested, this, &DataSequenceTree::handleContextMenu );
}

void
DataSequenceTree::setData( std::shared_ptr< adcontrols::datafile >& )
{
}

void
DataSequenceTree::setData( const QStringList& list )
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
DataSequenceTree::handleData( int row )
{
    QStandardItemModel& model = *model_;

    dataSubscriber * data = 0;
    do {
        std::lock_guard< std::mutex > lock( mutex_ );
        auto it = std::find_if( dataSubscribers_.begin(), dataSubscribers_.end(), [row]( const std::shared_ptr< dataSubscriber >& d ){
                return d->row() == row;
            });
        if ( it != dataSubscribers_.end() )
            data = it->get();
    } while(0);

    if ( data ) {
        Infusion::setRow( model, row, "file" ); 
        setRaw( data, model.itemFromIndex( model.index( row, 0 ) ) );
        setProcessed( data, model.itemFromIndex( model.index( row, 0 ) ) );
    }
    expandAll();
}

void
DataSequenceTree::handleValueChanged( const QModelIndex& index )
{
    QStandardItemModel& model = *model_;

    if ( index.column() == c_sample_type ) {

        int level = 0;
        if ( index.data().toString() == "STD" ) {
            if ( (level = model.index( index.row(), c_level, index.parent() ).data().toInt()) == 0 )
                level = 1;
            // Level for STD must be 1 or grater; UNK/QC must be zero
            model.setData( model.index( index.row(), c_level, index.parent() ), level ); 
        }
        
        if ( index.parent() == QModelIndex() ) {
            auto parent = model.item( index.row() );
            for ( int row = 0; row < parent->rowCount(); ++row ) {
                model.setData( model.index( row, c_sample_type, parent->index() ), index.data() );
                model.setData( model.index( row, c_level, parent->index() ), level );  // set parent's level
            }
        }
    }
}

void
DataSequenceTree::handleIt( dataSubscriber * ptr )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    dataSubscribers_.push_back( ptr->shared_from_this() );
    emit onJoin( ptr->row() );
}

void
DataSequenceTree::handleJoin( int row )
{
    if ( dropCount_ && (--dropCount_ == 0) ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        std::for_each( threads_.begin(), threads_.end(), [] ( std::thread& t ){ t.join(); } );
        threads_.clear();
    }
    handleData( row );
}

void
DataSequenceTree::dropIt( const std::wstring& path )
{
    QStandardItemModel& model = *model_;
    int row = model.rowCount();

    // This hits to boost/msvc bug #6320 that is marked as 'fixed' but it seems still exists
    // and reproducible on Boost 1.55 + vc12 (VisualStudio 2013) -- workaround applied on all datafile accessors.

    std::lock_guard< std::mutex > lock( mutex_ );

    auto reader = std::make_shared< dataSubscriber >( row, path, [this] ( dataSubscriber * p ){ handleIt( p ); } );
    auto it = std::find_if( dataSubscribers_.begin(), dataSubscribers_.end()
                            , [=] ( const std::shared_ptr<dataSubscriber>& d ){ return d->filename() == path; } );
    if ( it == dataSubscribers_.end() ) {
        model.insertRow( row );
        model.setData( model.index( row, c_datafile ), QString::fromStdWString( path ) );
        ++dropCount_;
        threads_.push_back( std::thread( [reader] (){ reader->open(); } ) );
    }
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
                if ( it != extensions.end() )
                    dropIt( path.wstring() );
            }
        }
    }
}

size_t // row count
DataSequenceTree::setRaw( dataSubscriber * data, QStandardItem * parent )
{
    QStandardItemModel& model = *model_;    
    if ( data ) {
        if ( auto raw = data->raw() ) {
            int n = int( raw->getFunctionCount() );
            parent->setRowCount( n );
            parent->setColumnCount( number_of_columns );

            for ( int fcn = 0; fcn < n; ++fcn ) {
                model.setData( model.index( fcn, c_datafile, parent->index() ),  QString( "Fcn# %1" ).arg( fcn + 1 ), Qt::EditRole );
                Infusion::setRow( model, fcn, "raw", parent->index(), fcn + 1 );
            }
            
            if ( data->ms() ) {
                adcontrols::segment_wrapper<> segs( *data->ms() );
                int fcn = 0;
                for ( auto& fms : segs ) {
                    std::wstring desc = fms.getDescriptions().toString();
                    model.setData( model.index( fcn++, c_description, parent->index() ), QString::fromStdWString( desc ), Qt::EditRole );
                }
            }
            return n;
        }
    }
    return 0;
}

size_t
DataSequenceTree::setProcessed( dataSubscriber * data, QStandardItem * parent )
{
    QStandardItemModel& model = *model_;

    size_t rowCount = 0;
    if ( data ) {

        if ( auto pf = data->processed() ) {

            for ( auto& folder : pf->folders() ) {

                if ( folder.name() == L"Spectra" ) {

                    int row = parent->rowCount();
                    if ( ( rowCount = folder.folio().size() ) ) {
                        parent->setRowCount( int( rowCount + row ) );
                        parent->setColumnCount( number_of_columns );

                        for ( auto& folium : folder.folio() ) {
                            model.setData( model.index( row, 0, parent->index() ), QString::fromStdWString( folium.name() ), Qt::EditRole );
                            Infusion::setRow( model, row, "spc", parent->index() );
                            ++row;
                        }
                    }
                }
            }
        }
    }
    return rowCount;
}

bool
DataSequenceTree::getContents( adcontrols::QuanSequence& seq )
{
    QStandardItemModel& model = *model_;    
    for ( int row = 0; row < model.rowCount(); ++row ) {
        auto parent = model.item( row, c_datafile );
        std::wstring datafile = parent->data( Qt::EditRole ).toString().toStdWString();

        for ( int subRow = 0; subRow < parent->rowCount(); ++subRow ) {

            adcontrols::QuanSample sample;
            sample.dataSource( datafile.c_str() );
            // dataGuid
            sample.name( model.index( subRow, c_datafile, parent->index() ).data().toString().toStdWString().c_str( ) );

            std::wstring data_type = model.index( subRow, c_data_type, parent->index() ).data().toString().toStdWString();
            sample.dataType( data_type.c_str() );

            std::wstring samp_type = model.index( subRow, c_sample_type, parent->index() ).data().toString().toStdWString();
            if ( samp_type == L"UNK" )
                sample.sampleType( adcontrols::QuanSample::SAMPLE_TYPE_UNKNOWN );
            else if ( samp_type == L"STD" )
                sample.sampleType( adcontrols::QuanSample::SAMPLE_TYPE_STD );
            else if ( samp_type == L"QC" )
                sample.sampleType( adcontrols::QuanSample::SAMPLE_TYPE_QC );
            else if ( samp_type == L"BLANK" )
                sample.sampleType( adcontrols::QuanSample::SAMPLE_TYPE_BLANK );

            int channel = model.index( subRow, c_channel, parent->index() ).data().toInt();
            sample.channel( channel );

            int level = model.index( subRow, c_level, parent->index() ).data().toInt();
            sample.level( level );
            
            QString process = model.index( subRow, c_process, parent->index() ).data().toString();
            if ( process == Constants::cmbAvgAll ) {
                sample.dataGeneration( adcontrols::QuanSample::GenerateSpectrum );
                sample.scan_range( 0, -1 );
            }
            else if ( process == Constants::cmbTake1st ) {
                sample.dataGeneration( adcontrols::QuanSample::GenerateSpectrum );
                sample.scan_range( 0, 0 );
            }
            else if ( process == Constants::cmbTake2nd ) {
                sample.dataGeneration( adcontrols::QuanSample::GenerateSpectrum );
                sample.scan_range( 1, 1 );
            }
            else if ( process == Constants::cmbTakeLast ) {
                sample.dataGeneration( adcontrols::QuanSample::GenerateSpectrum );
                sample.scan_range( -1, -1 );
            }
            else if ( process == Constants::cmbProcEach ) {
                sample.dataGeneration( adcontrols::QuanSample::ProcessRawSpectra );
                sample.scan_range( 0, -1 );
            }
            else if ( process == "AS IS" ) {
                sample.dataGeneration( adcontrols::QuanSample::ASIS );
            }
            seq << sample;
        }
    }
    return true;
}

bool
DataSequenceTree::setContents( const adcontrols::QuanSequence& seq )
{
    QStandardItemModel& model = *model_;    

    if ( seq.size() == 0 )
        return true;

    model.setRowCount( 0 );

    std::map< std::wstring, int > files;

    for ( auto sample: seq ) {
        if ( files.find( sample.dataSource() ) == files.end() ) {
            int row = model.rowCount();
            files[ sample.dataSource() ] = row;
            
            model.insertRow( row );
            model.setData( model.index( row, c_datafile ), QString::fromStdWString( sample.dataSource() ) );
            Infusion::setRow( model, row, "file" );
        }
        int row = files[ sample.dataSource() ];
        if ( auto parent = model.itemFromIndex( model.index( row, c_datafile ) ) ) {
            int subrow = parent->rowCount();
            parent->insertRow( subrow, new QStandardItem() );
            parent->setColumnCount( number_of_columns );
            Infusion::setRow( model, subrow, sample, parent->index() );
        }
    }

    expandAll();

    return true;
}

void
DataSequenceTree::handleContextMenu( const QPoint& pt )
{
    QMenu menu;

    menu.addAction( "Delete line", this, SLOT( delLine() ) );
    menu.addAction( "Add line", this, SLOT( delAll() ) );
    menu.addAction( "Clear all", this, SLOT( delAll() ) );
    
    menu.exec( mapToGlobal( pt ) );
}

void
DataSequenceTree::delAll()
{
    model_->setRowCount( 0 );
}

void
DataSequenceTree::delLine()
{
    QModelIndex index = currentIndex();
    model_->removeRow( index.row(), index.parent() );
}

void
DataSequenceTree::addLine()
{
    // QModelIndex index = currentIndex();
    // model_->removeRow( index.row(), index.parent() );
    QMessageBox::information( 0, "DataSequenceTree", "Not yet implemented, Sorry" );
}

