/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "controlmethodtable.hpp"
#include "controlmethodwidget.hpp"
#include <adcontrols/controlmethod.hpp>
#if defined _MSC_VER
# pragma warning( disable:4251 )
#endif
#include <QHeaderView>
#include <QMenu>
#include <QPainter>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <boost/exception/all.hpp>
#include <functional>
#include <QVariant>

namespace adwidgets {
    namespace controlmethodtable {

        class ItemDelegate : public QStyledItemDelegate {
        public:
            void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {

                if ( index.column() == 0 ) {
                    if ( index.data().toDouble() < 0 )
                        painter->drawText( option.rect, Qt::AlignRight | Qt::AlignVCenter, "Initial" );
                    else
                        painter->drawText( option.rect, Qt::AlignRight | Qt::AlignVCenter, QString::number( index.data().toDouble(), 'f', 4 ) );
                } else {
                    QStyledItemDelegate::paint( painter, option, index );
                }
            }

            void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {

                QStyledItemDelegate::setModelData( editor, model, index );

                auto v = model->index( index.row(), 0 ).data( Qt::UserRole );
                if ( v.canConvert< adcontrols::ControlMethod::MethodItem >() ) {
                    auto mi = v.value< adcontrols::ControlMethod::MethodItem >();
                    if ( index.column() == 0 && !mi.isInitialCondition() )
                        mi.setTime( index.data().toDouble() * 60.0 );  // seconds is the internal format
                    model->setData( model->index( index.row(), 0 ), QVariant::fromValue<adcontrols::ControlMethod::MethodItem>( mi ), Qt::UserRole );
                }
            }

        };

    }
}

using namespace adwidgets;

ControlMethodTable::ControlMethodTable( ControlMethodWidget * parent ) : adwidgets::TableView( parent )
                                                                       , parent_( parent )
                                                                       , model_( new QStandardItemModel )
{
    setModel( model_ );

    auto delegate = new controlmethodtable::ItemDelegate;
    setItemDelegate( delegate );

    setSelectionMode( QAbstractItemView::SingleSelection );
    setSelectionBehavior( QAbstractItemView::SelectRows );

    setContextMenuPolicy( Qt::CustomContextMenu );
    connect( this, &QTableView::customContextMenuRequested, this, &ControlMethodTable::showContextMenu );

    onInitialUpdate();
}

QStandardItemModel&
ControlMethodTable::model()
{
    return *model_;
}

void
ControlMethodTable::onInitialUpdate()
{
    QStandardItemModel& model = *model_;

	model.setColumnCount( 5 );
    model.setRowCount( 1 );

    model.setHeaderData( 0, Qt::Horizontal, QObject::tr( "time(min)" ) );
    model.setHeaderData( 1, Qt::Horizontal, QObject::tr( "module" ) );
    model.setHeaderData( 2, Qt::Horizontal, QObject::tr( "id" ) );
    model.setHeaderData( 3, Qt::Horizontal, QObject::tr( "function" ) );
    model.setHeaderData( 4, Qt::Horizontal, QObject::tr( "description" ) );

    resizeColumnsToContents();
    resizeRowsToContents();

    setColumnHidden( 2, true );

    horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );
}

void
ControlMethodTable::setSharedPointer( std::shared_ptr< adcontrols::ControlMethod::Method > ptr )
{
    method_ = ptr;
    if ( ptr )
        setContents( *ptr );
}

bool
ControlMethodTable::setContents( const adcontrols::ControlMethod::Method& m )
{
    QStandardItemModel& model = *model_;
    model.setRowCount( int( m.size() ) );
    int row = 0;
    for ( auto& mi : m ) {
        setData( mi, row );
        ++row;
    }
    if ( m.size() ) {
        if ( currentIndex().row() != 0 )
            selectRow( 0 );
        else {
            auto mi = data( 0 );
            parent_->setMethod( *m.begin() );
        }
    }
    return true;
}

void
ControlMethodTable::addEditor( const QString& text )
{
    items_ << text;
}

void
ControlMethodTable::clearAllEditors()
{
    items_.clear();
}

void
ControlMethodTable::commit()
{
    int row = selectionModel()->currentIndex().row();
    auto mi = data( row );
    parent_->getMethod( mi ); // load details from current selected UI, which match up with modelname,funcid
    setData( mi, row );       // set it to master table

    if ( auto ptr = method_.lock() ) {

        ptr->clear();
        for ( int row = 0; row < model_->rowCount(); ++row ) {
            mi = data( row );
            ptr->push_back( mi );
        }
    }
}

void
ControlMethodTable::currentChanged( const QModelIndex& curr, const QModelIndex& prev )
{
    if ( prev.isValid() ) {
        auto mi = data( prev.row() );
        parent_->getMethod( mi );
        setData( mi, prev.row() );
    }
    if ( curr.isValid() ) {
        auto mi = data( curr.row() );
        parent_->setMethod( mi );
    }
}

void
ControlMethodTable::sort()
{
    commit();
    if ( auto ptr = method_.lock() ) {
        ptr->sort();
        setContents( *ptr );
    }
}

void
ControlMethodTable::delLine( int /* row */)
{
    handleDeleteSelection();
}

void
ControlMethodTable::showContextMenu( const QPoint& pt )
{
    std::vector< QAction * > actions;
    QAction * delete_action, * sort_action;
    QMenu menu;

    for ( auto item : items_ )
        actions.push_back( menu.addAction( "Add: " + item ) );

    menu.addSeparator();
    delete_action = menu.addAction( "Delete line" );
    sort_action = menu.addAction( "Sort" );

    QAction * import_init = menu.addAction( "Import Inital Condition" );

    TableView::addActionsToContextMenu( menu, pt );

    if ( QAction * selected = menu.exec( this->mapToGlobal( pt ) ) ) {
        if ( selected == delete_action )
            delLine( indexAt( pt ).row() );
        else if ( selected == sort_action )
            sort();
        else if ( selected == import_init )
            emit parent_->onImportInitialCondition();
        else {
            for ( size_t i = 0; i < actions.size(); ++i ) {
                if ( actions[ i ] == selected ) {
                    emit onAddMethod( items_[ int( i ) ] );
                    break;
                }
            }
        }
    }
}

bool
ControlMethodTable::append( const adcontrols::ControlMethod::MethodItem& mi )
{
    QStandardItemModel& model = *model_;

    if ( auto ptr = method_.lock() ) {

        ptr->push_back( mi );

        int row = model.rowCount();
        model.setRowCount( row + 1 );
        setData( mi, row );
    }

    return true;
}

void
ControlMethodTable::setData( const adcontrols::ControlMethod::MethodItem& mi, int row )
{
    QStandardItemModel& model = *model_;

    if ( row < 0 || row >= model.rowCount() )
        return;

    if ( mi.isInitialCondition() ) {
        model.setData( model.index( row, 0 ), -1 );
        model.item( row, 0 )->setEditable( false );
    } else {
        model.setData( model.index( row, 0 ), mi.time() / 60.0 );
        model.item( row, 0 )->setEditable( true );
    }

    auto item = model.itemFromIndex( model.index( row, 0 ) );
    item->setData( QVariant::fromValue<adcontrols::ControlMethod::MethodItem>( mi ), Qt::UserRole );

    model.setData( model.index( row, 1 ), QString::fromStdString( mi.modelname() ) );
    model.setData( model.index( row, 2 ), mi.funcid() );
    model.setData( model.index( row, 3 ), QString::fromStdString( mi.itemLabel() ) );
    model.setData( model.index( row, 4 ), QString::fromStdString( mi.description() ) ); // utf8

    model.item( row, 1 )->setEditable( false );
    model.item( row, 2 )->setEditable( false );
    model.item( row, 3 )->setEditable( false );
    model.item( row, 4 )->setEditable( true );
}

adcontrols::ControlMethod::MethodItem
ControlMethodTable::data( int row ) const
{
    if ( auto item = model_->item( row, 0 ) ) {
        auto v = item->data( Qt::UserRole );
        if ( v.canConvert< adcontrols::ControlMethod::MethodItem >() ) {
            auto mi = v.value< adcontrols::ControlMethod::MethodItem >();

            // time
            double seconds = model_->data( model_->index( row, 0 ) ).toDouble() * 60.0;
            if ( seconds < 0 )
                mi.setIsInitialCondition( true );
            else
                mi.setTime( seconds );

            // description
            auto desc = model_->data( model_->index( row, 4 ) ).toString();
            mi.setDescription( desc.toStdString().c_str() );

            return mi;
        }
    }
    return adcontrols::ControlMethod::MethodItem();
}

Q_DECLARE_METATYPE( adcontrols::ControlMethod::MethodItem )
