/**************************************************************************
** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2019 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "cgenform.hpp"
#include "tableview.hpp"
#include <QBoxLayout>
#include <QByteArray>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPainter>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <adcontrols/datareader.hpp>
#include <adportable/debug.hpp>
#include <boost/uuid/uuid.hpp>
#include <stdexcept>
#include <sstream>

namespace adwidgets {

    namespace {

        enum {
            c_display_name
            , c_fcn
            , c_objtext
        };

        class delegate : public QStyledItemDelegate {
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

    } // namespace
}

using namespace adwidgets;

DataReaderChoiceDialog::DataReaderChoiceDialog( QWidget *parent ) : QDialog( parent )
{
    if ( auto layout = new QVBoxLayout( this ) ) {
        layout->addWidget( new TableView( this ) );
        layout->addWidget( new CGenForm( this ) );
        layout->addWidget( new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel ) );
    }
}

DataReaderChoiceDialog::DataReaderChoiceDialog( std::vector< std::shared_ptr< adcontrols::DataReader > >&& readers
                                                , QWidget * parent ) : QDialog( parent )
{
    if ( auto layout = new QVBoxLayout( this ) ) {

        // Chromatogram width method
        auto form = new CGenForm( this );
        layout->addWidget( form );

        // Data Reader Chooser
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

        table->resizeColumnsToContents(); // performance ???

        layout->addWidget( table );

        form->setLabel( model->index( 0, c_display_name ).data().toString() );

        connect( table->selectionModel()
                 , &QItemSelectionModel::currentRowChanged
                 , [form,table]( const auto& curr, auto& prev ){
                     // set label
                     form->setLabel( curr.model()->index( curr.row(), c_display_name ).data().toString() );

                     // store form value into table
                     table->model()->setData( table->model()->index( prev.row(), c_display_name ), form->massWidth(), Qt::UserRole + 1 );
                     table->model()->setData( table->model()->index( prev.row(), c_display_name ), form->timeWidth(), Qt::UserRole + 2 );
                     table->model()->setData( table->model()->index( prev.row(), c_display_name ), form->enableTime(), Qt::UserRole + 3 );

                     // restore table value into form
                     QSignalBlocker block( form );
                     form->setMassWidth( curr.model()->index( curr.row(), c_display_name ).data( Qt::UserRole + 1 ).toDouble() );
                     form->setTimeWidth( curr.model()->index( curr.row(), c_display_name ).data( Qt::UserRole + 2 ).toDouble() );
                     form->setEnableTime( curr.model()->index( curr.row(), c_display_name ).data( Qt::UserRole + 3 ).toBool() );
                 });

        connect( form, &CGenForm::valueChanged, [table]( int id, double value ){
                                                    table->model()->setData( table->model()->index( table->currentIndex().row(), c_display_name ), value, Qt::UserRole + 1 + id );
                                                });

        connect( form, &CGenForm::enableTimeChanged, [table]( bool enable ){
                                                         table->model()->setData( table->model()->index( table->currentIndex().row(), c_display_name ), enable, Qt::UserRole + 3 );
                                                     });

        auto label = new QLabel;
        label->setText( "Select trace and protocol# from the above table." );
        layout->addWidget( label );

        auto buttons = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
        connect( buttons, &QDialogButtonBox::accepted, this, [&] () { QDialog::accept(); } );
        connect( buttons, &QDialogButtonBox::rejected, this, [&] () { QDialog::reject(); } );
        layout->addWidget( buttons );

    }

#if ! defined Q_OS_DARWIN
    setStyleSheet( "* {font-size: 9pt; }" );
#endif

    adjustSize();
    this->resize( this->size() + QSize( 200, 0 ) );
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

//////// new interface

std::vector< std::pair< int, int > >
DataReaderChoiceDialog::selection() const
{
    std::vector< std::pair< int, int > > res;

    if ( auto table = findChild< TableView * >() ) {
        QModelIndexList indices = table->selectionModel()->selectedRows();
        if ( ! indices.isEmpty() ) {
            std::sort( indices.begin(), indices.end() );
            for ( int i = 0; i < indices.size(); ++i )
                res.emplace_back( indices.at( i ).row(), table->model()->index( indices.at( i ).row(), c_fcn ).data( Qt::EditRole ).toInt() );
        } else { // select all
            for ( int i = 0; i < table->model()->rowCount(); ++i )
                res.emplace_back( i, table->model()->index( i, c_fcn ).data( Qt::EditRole ).toInt() );
        }
    }
    return res;
}

void
DataReaderChoiceDialog::setMassWidth( double width /* dalton */ )
{
    // ADDEBUG() << "setMassWidth(" << width << ")";
    if ( auto w = findChild< CGenForm * >() )
        w->setMassWidth( width );
    if ( auto table = findChild< TableView * >() ) {
        for ( int row = 0; row < table->model()->rowCount(); ++row )
            table->model()->setData( table->model()->index( row, c_display_name ), width, Qt::UserRole + 1 );
    }
}

double
DataReaderChoiceDialog::massWidth() const
{
    if ( auto w = findChild< CGenForm * >() ) {
        // ADDEBUG() << "massWidth = " << w;
        return massWidth();
    }
    return 0;
}

void
DataReaderChoiceDialog::setTimeWidth( double t )
{
    if ( auto w = findChild< CGenForm * >() )
        w->setTimeWidth( t );
    if ( auto table = findChild< TableView * >() ) {
        for ( int row = 0; row < table->model()->rowCount(); ++row )
            table->model()->setData( table->model()->index( row, c_display_name ), t, Qt::UserRole + 2 );
    }
}

double
DataReaderChoiceDialog::timeWidth() const
{
    if ( auto w = findChild< CGenForm * >() )
        return w->timeWidth();
    return 0;
}

void
DataReaderChoiceDialog::setEnableTime( bool enable )
{
    if ( auto w = findChild< CGenForm * >() )
        w->setEnableTime( enable );

    if ( auto table = findChild< TableView * >() ) {
        for ( int row = 0; row < table->model()->rowCount(); ++row )
            table->model()->setData( table->model()->index( row, c_display_name ), enable, Qt::UserRole + 3 );
    }
}

bool
DataReaderChoiceDialog::enableTime() const
{
    if ( auto w = findChild< CGenForm * >() )
        w->enableTime();
    return false;
}

std::vector< QByteArray >
DataReaderChoiceDialog::toJson() const
{
    std::vector< QByteArray > a;

    if ( auto table = findChild< TableView * >() ) {
        for ( int row = 0; row < table->model()->rowCount(); ++row ) {
            QJsonObject obj{
                { "massWidth", table->model()->index( row, c_display_name ).data( Qt::UserRole + 1 ).toDouble() }
                , { "timeWidth", table->model()->index( row, c_display_name ).data( Qt::UserRole + 2 ).toDouble() }
                , { "enableTime", table->model()->index( row, c_display_name ).data( Qt::UserRole + 3 ).toBool() }
            };
            a.emplace_back( QJsonDocument{ obj }.toJson() );
            //-----------
            // ADDEBUG() << a.back().toStdString();
        }
    }

    return a;
}
