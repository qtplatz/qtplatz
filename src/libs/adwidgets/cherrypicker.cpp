/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#include "cherrypicker.hpp"
#include <QKeyEvent>
#include <QStyledItemDelegate>
#include <QStandardItemModel>
#include <QHeaderView>

namespace adwidgets {

    enum { c_display_value, c_key };
    
    class CherryPicker::delegate : public QStyledItemDelegate {
        CherryPicker * _this;
    public:

        delegate( CherryPicker * p ) : _this( p ) {
        }
        
        bool editorEvent( QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem& option, const QModelIndex& index ) override {
            bool res = QStyledItemDelegate::editorEvent( event, model, option, index );
            if ( event->type() == QEvent::MouseButtonRelease && model->flags(index) & Qt::ItemIsUserCheckable ) {
                QVariant st = index.data( Qt::CheckStateRole );
                emit _this->stateChanged( model->index( index.row(), c_key ).data( Qt::EditRole ).toString(), st == Qt::Checked ? true : false );
            }
            return res;
        }
    };


    class CherryPicker::impl {
    public:
        QStandardItemModel model;

        void addItem( const QString& key, const QString& displayValue, bool isChecked, bool isEnable );
        void setChecked( const QString& key, bool checked );
        void setEnabled( const QString& key, bool checked );
        bool checked( const QString& key ) const;
        bool enabled( const QString& key ) const;
        QString displayValue( const QString& key) const;        
        QString key( size_t idx ) const;
    private:
        int findRow( const QString& ) const;
    };
    
}

using namespace adwidgets;

CherryPicker::CherryPicker( QWidget * parent ) : TableView( parent )
                                               , impl_( new impl() )
{
    setAllowDelete( false );
    setModel( &impl_->model );
	setItemDelegate( new delegate( this ) );

    impl_->model.setColumnCount( 2 );
    impl_->model.setHeaderData( c_display_value, Qt::Horizontal, QObject::tr( "Module" ) );
    impl_->model.setHeaderData( c_key, Qt::Horizontal, QObject::tr( "key" ) );
    setColumnHidden( c_key, true );
}

CherryPicker::~CherryPicker()
{
    delete impl_;
}

void
CherryPicker::addItem( const QString& key, const QString& displayValue, bool checked, bool enable )
{
    impl_->addItem( key, displayValue, checked, enable );
}

void
CherryPicker::setChecked( const QString& key, bool checked )
{
    impl_->setChecked(  key, checked );
}

void
CherryPicker::setEnabled( const QString& key, bool checked )
{
    impl_->setEnabled( key, checked );
}

bool
CherryPicker::checked( const QString& key ) const
{
    return impl_->checked( key );
}

bool
CherryPicker::enabled( const QString& key ) const
{
    return impl_->enabled( key );
}

size_t
CherryPicker::size() const
{
    return impl_->model.rowCount();
}

QString
CherryPicker::key( size_t idx ) const
{
    return impl_->key( idx );
}

QString
CherryPicker::displayValue( const QString& key ) const
{
    return impl_->displayValue( key );
}


//signals:
//        void stateChanged( const QString& key, bool isChecked );

void
CherryPicker::impl::addItem( const QString& key, const QString& displayValue, bool isChecked, bool isEnable )
{
    int row = model.rowCount();
    model.setRowCount( row + 1 );
    model.setData( model.index( row, c_display_value ), displayValue );
    model.setData( model.index( row, c_key ), key );

    if ( auto item = model.item( row, 0 ) ) {
        item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
        model.setData( model.index( row, c_display_value ), isChecked ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
    }
}

void
CherryPicker::impl::setChecked( const QString& key, bool checked )
{
    int row = findRow( key );
    if ( row >= 0 ) {
        model.setData( model.index( row, c_display_value ), checked ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
    }
}

void
CherryPicker::impl::setEnabled( const QString& key, bool checked )
{
}

bool
CherryPicker::impl::checked( const QString& key ) const
{
    int row = findRow( key );
    if ( row >= 0 ) {
        return model.index( row, c_display_value ).data( Qt::CheckStateRole ).toBool();
    }
    return false;
}

bool
CherryPicker::impl::enabled( const QString& key ) const
{
    return true;
}

QString
CherryPicker::impl::displayValue( const QString& key ) const
{
    int row = findRow( key );
    if ( row >= 0 ) {
        return model.index( row, c_display_value ).data( Qt::EditRole ).toBool();
    }
    return QString();
}

QString
CherryPicker::impl::key( size_t idx ) const
{
    if ( idx < model.rowCount() )
        return model.index( int( idx ), c_key ).data( Qt::EditRole ).toString();
    return QString();
}

int
CherryPicker::impl::findRow( const QString& key ) const
{
    for ( int row = 0; row < model.rowCount(); ++row ) {
        if ( model.index( row, c_key ).data( Qt::EditRole ).toString() == key )
            return row;
    }
    return -1;
}
