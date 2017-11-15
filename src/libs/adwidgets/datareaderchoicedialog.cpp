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

#include "datareaderchoicedialog.hpp"
#include "tableview.hpp"
#include <adcontrols/datareader.hpp>
#include <QBoxLayout>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QPainter>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QLabel>
#include <boost/uuid/uuid.hpp>
#include <stdexcept>
#include <sstream>

namespace adwidgets {
    
    enum {
        c_display_name
        , c_fcn
        , c_objtext
    };


    class DataReaderChoiceDialog::delegate : public QStyledItemDelegate {
    public:
        delegate( QWidget * parent ) : QStyledItemDelegate( parent ) {
        }
        
        QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
            if ( index.column() == c_fcn ) {
                size_t nProto = index.data( Qt::UserRole + 1 ).toInt();
                auto combo = new QComboBox( parent );
                combo->addItem( "*" ); // -1
                for ( int fcn = 0; fcn < nProto; ++fcn )
                    combo->addItem( QString::number( fcn ) );
                return combo;
            }
            return createEditor( parent, option, index );
        }

        void setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex& index ) const override {
            if ( index.column() == c_fcn ) {
                if ( auto combo = qobject_cast< QComboBox * >( editor ) ) {
                    int idx = int( combo->currentIndex() ) - 1;
                    if ( idx < 0 )
                        model->setData( index, -1 ); // protocol select all (*) = -1
                    else if ( idx < index.data( Qt::UserRole + 1 ).toInt() )
                        model->setData( index, idx ); // protocol specified
                }
            }
        }

        void paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
            if ( index.column() == c_fcn ) {
                QStyleOptionViewItem opt(option);
                initStyleOption( &opt, index );
                opt.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
                painter->drawText( option.rect, option.displayAlignment, index.data().toInt() < 0 ? "*" : index.data().toString() );
            } else {
                QStyledItemDelegate::paint( painter, option, index );
            }
        }
    };

}

using namespace adwidgets;

DataReaderChoiceDialog::DataReaderChoiceDialog( QWidget *parent ) : QDialog( parent )
{
    if ( auto layout = new QVBoxLayout( this ) ) {
        auto widget = new TableView( this );
        
        layout->addWidget( widget );
        layout->addWidget( new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel ) );
    }
    
}

DataReaderChoiceDialog::DataReaderChoiceDialog( std::vector< std::shared_ptr< const adcontrols::DataReader > >&& readers
                                                , QWidget * parent ) : QDialog( parent )
{
    if ( auto layout = new QVBoxLayout( this ) ) {
        auto table = new TableView( this );
        auto model = new QStandardItemModel();

        model->setColumnCount( 3 );
        model->setRowCount( int( readers.size() ) );
        model->setHeaderData( c_display_name, Qt::Horizontal, tr( "name" ) );
        model->setHeaderData( c_fcn, Qt::Horizontal, tr( "Protocol#" ) );
        model->setHeaderData( c_objtext, Qt::Horizontal, tr( "id" ) );
        for ( int row = 0; row < int( readers.size() ); ++row ) {
            const auto& reader = readers[ row ];
            model->setData( model->index( row, c_display_name ), QString::fromStdString( reader->display_name() ) );
            model->setData( model->index( row, c_objtext ), QString::fromStdString( reader->objtext() ) );
            model->setData( model->index( row, c_fcn ), -1, Qt::EditRole ); // fcn '*'
            model->setData( model->index( row, c_fcn ), int( reader->fcnCount() ), Qt::UserRole + 1 );
            model->item( row, c_display_name )->setEditable( false );
            model->item( row, c_objtext )->setEditable( false );
        }
        table->setModel( model );
        table->setItemDelegate( new delegate( this ) );
        table->setSelectionBehavior( QAbstractItemView::SelectRows );
        table->resizeColumnsToContents();
        table->resizeRowsToContents();
        table->horizontalHeader()->setStretchLastSection( true );
        table->horizontalHeader()->setSectionResizeMode( QHeaderView::ResizeToContents );

        table->setCurrentIndex( model->index( 0, 0 ) );

        layout->addWidget( table );

        auto label = new QLabel;
        label->setText( "Select trace and protocol# from the above table." );
        layout->addWidget( label );

        auto buttons = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
        connect( buttons, &QDialogButtonBox::accepted, this, [&] () { QDialog::accept(); } );
        connect( buttons, &QDialogButtonBox::rejected, this, [&] () { QDialog::reject(); } );
        layout->addWidget( buttons );

    }
    adjustSize();
}

int
DataReaderChoiceDialog::currentSelection() const
{
    if ( auto table = findChild< QTableView * >() )
        return table->currentIndex().row();
    return 0;
}

void
DataReaderChoiceDialog::setProtocolHidden( bool hide )
{
    if ( auto table = findChild< QTableView * >() ) {    
        table->setColumnHidden( c_fcn, hide );
    }
}

int
DataReaderChoiceDialog::fcn() const
{
    if ( auto table = findChild< QTableView * >() ) {
        auto model = table->model();
        return model->index( table->currentIndex().row(), c_fcn ).data().toInt();
    }
    return 0;
}

