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

#include "insttreeview.hpp"
#include <adicontroller/constants.hpp>
#include <adlog/logger.hpp>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMessageBox>
#include <QStyledItemDelegate>
#include <QStandardItemModel>

#if defined _MSC_VER
# pragma warning(disable:4503 4715)
#endif
#include <boost/exception/all.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace adwidgets {

    enum { c_display_value, c_status, c_key, c_num_columns };
    
    class InstTreeView::delegate : public QStyledItemDelegate {
        InstTreeView * _this;
    public:

        delegate( InstTreeView * p ) : _this( p ) {
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


    class InstTreeView::impl {
    public:
        struct Observer;
        
        QStandardItemModel model;

        void addItem( const QString& key, const QString& displayValue, bool isChecked, bool isEnable );
        void setChecked( const QString& key, bool checked );
        void setEnabled( const QString& key, bool checked );
        bool checked( const QString& key ) const;
        bool enabled( const QString& key ) const;
        QString displayValue( const QString& key) const;        
        QString key( size_t idx ) const;

        int findRow( const QString& ) const;
        void setObserver( int row, const Observer& );
        void setObserver( QStandardItem& parent, int row, const Observer& );
        
        struct Observer {
            Observer() : trace_method( adicontroller::SignalObserver::eTRACE_TRACE )
                {}
            
            Observer( const Observer& t ) : objtext( t.objtext )
                                          , objid( t.objid )
                                          , trace_id( t.trace_id )
                                          , trace_display_name( t.trace_display_name )
                                          , trace_method( adicontroller::SignalObserver::eTRACE_TRACE )
                                          , siblings( t.siblings ) {
            }
            
            QString objtext;
            QString objid;
            QString trace_id;
            QString trace_display_name;
            adicontroller::SignalObserver::eTRACE_METHOD trace_method;
            std::vector< Observer > siblings;
            
            void operator()( const boost::property_tree::ptree& pt ) {
                
                if ( boost::optional<std::string> value = pt.get_optional<std::string>( "observer.objtext" ) )
                    objtext = QString::fromStdString( value.get() );
                
                if ( boost::optional<std::string> value = pt.get_optional<std::string>( "observer.objid" ) )
                    objid = QString::fromStdString( value.get() );

                if ( boost::optional< std::string > value = pt.get_optional< std::string >( "observer.desc.trace_id" ) )
                    trace_id = QString::fromStdString( value.get() );

                if ( boost::optional< std::string > value = pt.get_optional< std::string >( "observer.desc.trace_display_name" ) )
                    trace_display_name = QString::fromStdString( value.get() );

                if ( auto value = pt.get_optional< int >( "observer.desc.trace_method" ) )
                    trace_method = static_cast<adicontroller::SignalObserver::eTRACE_METHOD>( value.get() );

                if ( auto childs = pt.get_child_optional("siblings") ) {
                    for ( const boost::property_tree::ptree::value_type& child : pt.get_child( "siblings" ) ) {
                        siblings.push_back( Observer() );
                        siblings.back()( child.second );
                    }
                }
            }
        };

    };
    
}

using namespace adwidgets;

InstTreeView::InstTreeView( QWidget * parent ) : QTreeView( parent )
                                       , impl_( new impl() )
{
    setModel( &impl_->model );
	setItemDelegate( new delegate( this ) );

    impl_->model.setColumnCount( 10 );
    impl_->model.setHeaderData( c_display_value, Qt::Horizontal, QObject::tr( "Module" ) );
    impl_->model.setHeaderData( c_status, Qt::Horizontal, QObject::tr( "Status" ) );
    impl_->model.setHeaderData( c_key, Qt::Horizontal, QObject::tr( "key" ) );

    setColumnHidden( c_key, true );
}

InstTreeView::~InstTreeView()
{
    delete impl_;
}

void
InstTreeView::addItem( const QString& key, const QString& displayValue, bool checked, bool enable )
{
    impl_->addItem( key, displayValue, checked, enable );
}

void
InstTreeView::setChecked( const QString& key, bool checked )
{
    impl_->setChecked(  key, checked );
}

void
InstTreeView::setEnabled( const QString& key, bool checked )
{
    impl_->setEnabled( key, checked );
}

bool
InstTreeView::checked( const QString& key ) const
{
    return impl_->checked( key );
}

bool
InstTreeView::enabled( const QString& key ) const
{
    return impl_->enabled( key );
}

size_t
InstTreeView::size() const
{
    return impl_->model.rowCount();
}

QString
InstTreeView::key( size_t idx ) const
{
    return impl_->key( idx );
}

QString
InstTreeView::displayValue( const QString& key ) const
{
    return impl_->displayValue( key );
}

void
InstTreeView::setInstState( const QString& key, const QString& state_name )
{
    int row = impl_->findRow( key );
    if ( row >= 0 ) {
        auto& model = impl_->model;
        model.setData( model.index( row, c_status ), state_name );
    }
}

//signals:
//        void stateChanged( const QString& key, bool isChecked );

void
InstTreeView::impl::addItem( const QString& key, const QString& displayValue, bool isChecked, bool isEnable )
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
InstTreeView::impl::setChecked( const QString& key, bool checked )
{
    int row = findRow( key );
    if ( row >= 0 ) {
        model.setData( model.index( row, c_display_value ), checked ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
    }
}

void
InstTreeView::impl::setEnabled( const QString& key, bool checked )
{
}

bool
InstTreeView::impl::checked( const QString& key ) const
{
    int row = findRow( key );
    if ( row >= 0 ) {
        return model.index( row, c_display_value ).data( Qt::CheckStateRole ).toBool();
    }
    return false;
}

bool
InstTreeView::impl::enabled( const QString& key ) const
{
    return true;
}

QString
InstTreeView::impl::displayValue( const QString& key ) const
{
    int row = findRow( key );
    if ( row >= 0 ) {
        return model.index( row, c_display_value ).data( Qt::EditRole ).toBool();
    }
    return QString();
}

QString
InstTreeView::impl::key( size_t idx ) const
{
    if ( idx < model.rowCount() )
        return model.index( int( idx ), c_key ).data( Qt::EditRole ).toString();
    return QString();
}

int
InstTreeView::impl::findRow( const QString& key ) const
{
    for ( int row = 0; row < model.rowCount(); ++row ) {
        if ( model.index( row, c_key ).data( Qt::EditRole ).toString() == key )
            return row;
    }
    return -1;
}

void
InstTreeView::setObserverTree( const QString& key, const QString& json )
{
    boost::property_tree::ptree pt;
    std::stringstream ss( json.toStdString() );

    try {
        boost::property_tree::read_json( ss, pt );
    } catch ( std::exception& e ) {
        ADINFO() << ( std::string( "Exception : read_json" ) + boost::diagnostic_information( e ) );
        QMessageBox::warning( this, "InstTreeView", "Json exception" );
        return;
    }

    impl::Observer observer;
    observer( pt );

    int row = impl_->findRow( key );
    if ( row >= 0 ) {
        impl_->setObserver( row, observer );
    }

}

void
InstTreeView::impl::setObserver( int row, const Observer& observer )
{
    if ( auto parent = model.item( row ) ) {
        parent->setRowCount( 1 );
        parent->setColumnCount( 3 );
        setObserver( *parent, 0, observer );
    }
}

void
InstTreeView::impl::setObserver( QStandardItem& parent, int row, const Observer& observer )
{
    model.setData( model.index( row, 0, parent.index() ), observer.objtext, Qt::EditRole );
    model.setData( model.index( row, 1, parent.index() ), observer.trace_display_name, Qt::EditRole );
    model.setData( model.index( row, 2, parent.index() ), observer.objid, Qt::EditRole );

    if ( !observer.siblings.empty() ) {

        auto child = model.itemFromIndex( model.index( row, 0, parent.index() ) );

        child->setRowCount( int( observer.siblings.size() ) );
        child->setColumnCount( 3 );
        for ( int i = 0; i < observer.siblings.size(); ++i ) {
            setObserver( *child, i, observer.siblings[i] );
        }
    }
}

