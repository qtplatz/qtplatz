/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "timedeventswidget.hpp"
#include "timedtableview.hpp"
#include "delegatehelper.hpp"
#include <adportable/debug.hpp>
#include <adportable/is_type.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/controlmethod/timedevent.hpp>
#include <adcontrols/controlmethod/timedevents.hpp>
#include <adcontrols/controlmethod/modulecap.hpp>
#include <QApplication>
#include <QBoxLayout>
#include <QComboBox>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QPainter>
#include <QSplitter>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QMetaType>
Q_DECLARE_METATYPE( boost::uuids::uuid );
Q_DECLARE_METATYPE( adcontrols::ControlMethod::EventCap::value_type );

namespace adwidgets {

    using adcontrols::ControlMethod::ModuleCap;
    using adcontrols::ControlMethod::EventCap;

    struct TimedEvent_AnyData : public std::enable_shared_from_this< TimedEvent_AnyData > {
        TimedEvent_AnyData( const EventCap* cap, const QModelIndex& index, const adcontrols::ControlMethod::any_type& value ) : cap_( cap ), index_( index ), value_( value ) {}
        TimedEvent_AnyData( const QModelIndex& index, const adcontrols::ControlMethod::any_type& value ) : cap_( 0 ), index_( index ), value_( value ) {}
        const EventCap * cap_;
        QModelIndex index_;
        adcontrols::ControlMethod::any_type value_;

        void callback( const adcontrols::ControlMethod::any_type& a ) {
            value_ = a;
        }
        
        void commit() {
            cap_->invalidate_any();
        }
    };

    class TimedEventsWidget::impl {
        TimedEventsWidget * this_;
    public:
        enum columns { c_clsid, c_time, c_model_name, c_item_name, c_value, c_value_2, ncolumns };
        
        impl( TimedEventsWidget * p ) : this_( p )
                                      , model_( new QStandardItemModel() ) {

            model_->setColumnCount( ncolumns );
            
            model_->setHeaderData( c_clsid,        Qt::Horizontal, QObject::tr( "clsid" ) );
            model_->setHeaderData( c_time,         Qt::Horizontal, QObject::tr( "Time(seconds)" ) );
            model_->setHeaderData( c_model_name,   Qt::Horizontal, QObject::tr( "Module" ) );
            model_->setHeaderData( c_item_name,    Qt::Horizontal, QObject::tr( "Function" ) );
            model_->setHeaderData( c_value,        Qt::Horizontal, QObject::tr( "Value" ) );
            model_->setHeaderData( c_value_2,      Qt::Horizontal, QObject::tr( "Value(width)" ) );
        }

        ~impl() { }
        
        void dataChanged( const QModelIndex& _1, const QModelIndex& _2 ) {
            emit this_->valueChanged();
        }

        const adcontrols::ControlMethod::ModuleCap * findModelCap( const QModelIndex& index ) const {
            auto model = index.model();
            auto clsid = model->data( model->index( index.row(), TimedEventsWidget::impl::c_clsid ), Qt::UserRole + 1 ).value<boost::uuids::uuid>();
            auto it = capList_.find( clsid );
            if ( it != capList_.end() )
                return &it->second;
            return nullptr;
        }

        const adcontrols::ControlMethod::EventCap * findEventCap( const QModelIndex& index ) const {
            auto model = index.model();
            if ( auto modelCap = findModelCap( index ) ) {
                int currIndex = model->data( model->index( index.row(), TimedEventsWidget::impl::c_item_name ), Qt::UserRole + 1 ).toInt();
                if ( currIndex < modelCap->eventCaps().size() )
                    return &modelCap->eventCaps().at( currIndex );
            }
            return nullptr;
        }

        void handleContextMenu( const QPoint& pt );
        void addLine( const adcontrols::ControlMethod::ModuleCap& );
        QList< QStandardItem * > make_row( const adcontrols::ControlMethod::ModuleCap&, const EventCap&, const EventCap::value_type& );

        std::unique_ptr< QStandardItemModel > model_;
        std::map < boost::uuids::uuid, adcontrols::ControlMethod::ModuleCap > capList_;
        std::shared_ptr< TimedEvent_AnyData > any_data_;
    };

    /////////////////////// Paint ////////////////////////////
    struct TimedEventsWidget_painter : boost::static_visitor< void > {
        QPainter * painter_;
        QStyleOptionViewItem option_;
        const QModelIndex& index_;
        TimedEventsWidget_painter( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) : painter_( painter ), option_( option ), index_( index ) {}
        
        template< typename T > void operator()( const T& t ) const {
            if ( index_.column() ==  TimedEventsWidget::impl::c_time )
                painter_->drawText( option_.rect, option_.displayAlignment, QString::number( index_.data( Qt::EditRole ).toDouble(), 'f', 3 /* 0.001 */ ) );
            else
                painter_->fillRect( option_.rect, QColor( 0xff, 0x63, 0x47, 0x40 ) ); // tomato
        }
    };

    template<> void
    TimedEventsWidget_painter::operator()( const adcontrols::ControlMethod::voltage_type& value ) const {
        assert( index_.column() == TimedEventsWidget::impl::c_value );
        painter_->drawText( option_.rect
                            , option_.displayAlignment
                            , QString::number( index_.data( Qt::EditRole ).toDouble(), 'f', 2 /* 0.01V */ ) );
    }

    template<> void
    TimedEventsWidget_painter::operator()( const adcontrols::ControlMethod::elapsed_time_type& value ) const {
        assert( index_.column() == TimedEventsWidget::impl::c_value );
        painter_->drawText( option_.rect
                            , option_.displayAlignment
                            , QString::number( index_.data( Qt::EditRole ).toDouble(), 'f', 2 /* precision */ ) );
    }

    template<> void
    TimedEventsWidget_painter::operator()( const adcontrols::ControlMethod::switch_type& value ) const {
        assert( index_.column() == TimedEventsWidget::impl::c_value );
        QString text = index_.data( Qt::EditRole ).toBool() ? QString::fromStdString( value.choice.first ) : QString::fromStdString( value.choice.second );
        painter_->drawText( option_.rect, option_.displayAlignment, text );
    }

    template<> void
    TimedEventsWidget_painter::operator()( const adcontrols::ControlMethod::delay_width_type& value ) const {
        if ( index_.column() == TimedEventsWidget::impl::c_value ) { // first data (delay)
            painter_->drawText( option_.rect, option_.displayAlignment
                                , QString::number( value.value.first * 1.0e6, 'f', 4 ) ); /* microseconds, .01 (10ns resolution) */
        } else if ( index_.column() == TimedEventsWidget::impl::c_value_2 ) { // second data (width)
            painter_->drawText( option_.rect, option_.displayAlignment
                                , QString::number( value.value.second * 1.0e6, 'f', 4 ) ); /* microseconds, .01 (10ns resolution) */
        }
    }

    template<> void
    TimedEventsWidget_painter::operator()( const adcontrols::ControlMethod::any_type& value ) const {
        assert( index_.column() == TimedEventsWidget::impl::c_value );
        QStyleOptionButton button;
        QRect r = option_.rect;
        int x = r.left() + r.width() - 30;
        button.rect = QRect( x, r.top(), 30, r.height() );
        button.text = "=^.^=";
        button.state = QStyle::State_Enabled;
        QApplication::style()->drawControl( QStyle::CE_PushButton, &button, painter_ );
    }
    
    /////////////////// set model data ////////////////////////////
    struct TimedEventsWidget_setModelData : boost::static_visitor< void > {
        QWidget * editor_;
        QAbstractItemModel * model_;
        const QModelIndex& index_;
        TimedEventsWidget_setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex& index ) : editor_( editor ), model_( model ), index_( index ) {
        }
        template< typename T > void operator()( const T& t ) const { model_->setData( index_, t, Qt::EditRole ); }
    };

    template<> void
    TimedEventsWidget_setModelData::operator()( const adcontrols::ControlMethod::voltage_type& t ) const
    {
        try {
            auto value = index_.data( Qt::UserRole + 1 ).value< EventCap::value_type >();
            auto xvalue = boost::get< adcontrols::ControlMethod::voltage_type >( value );
            if ( auto editor = qobject_cast<QDoubleSpinBox *>( editor_ ) )
                xvalue.value = editor->value();
            QVariant v;
            v.setValue<>( EventCap::value_type( value ) );
            model_->setData( index_, v, Qt::UserRole + 1 );
            model_->setData( index_, xvalue.value, Qt::EditRole );
        } catch ( boost::bad_get& ex ) {
            ADDEBUG() << ex.what();
        }
    }

    template<> void
    TimedEventsWidget_setModelData::operator()( const adcontrols::ControlMethod::switch_type& t ) const
    {
        auto value = index_.data( Qt::UserRole + 1 ).value< EventCap::value_type >();
        auto xvalue = boost::get< adcontrols::ControlMethod::switch_type >( value );
        if ( auto editor = qobject_cast<QComboBox *>( editor_ ) )
            xvalue.value = ( editor->currentIndex() == 0 ) ? false : true;
        QVariant v;
        v.setValue<>( EventCap::value_type( xvalue ) );
        model_->setData( index_, v, Qt::UserRole + 1 );
        model_->setData( index_, xvalue.value, Qt::EditRole );
    }

    template<> void
    TimedEventsWidget_setModelData::operator()( const adcontrols::ControlMethod::choice_type& t ) const
    {
        auto value = index_.data( Qt::UserRole + 1 ).value< EventCap::value_type >();
        if ( value.empty() )
            value = t;
        auto xvalue = boost::get< adcontrols::ControlMethod::choice_type >( value );
        if ( auto editor = qobject_cast<QComboBox *>( editor_ ) )
            xvalue.value = editor->currentIndex();
        QVariant v;
        v.setValue<>( EventCap::value_type( xvalue ) );
        model_->setData( index_, v, Qt::UserRole + 1 );
        model_->setData( index_, QString::fromStdString( xvalue.choice[xvalue.value] ), Qt::DisplayRole );
    }

    template<> void
    TimedEventsWidget_setModelData::operator()( const adcontrols::ControlMethod::elapsed_time_type& t ) const
    {
        auto value = index_.data( Qt::UserRole + 1 ).value< EventCap::value_type >();
        if ( value.empty() )
            value = t;
        auto xvalue = boost::get< adcontrols::ControlMethod::elapsed_time_type >( value );
        if ( auto editor = qobject_cast<QDoubleSpinBox *>( editor_ ) )
            xvalue.value = editor->value();
        QVariant v;
        v.setValue<>( EventCap::value_type( xvalue ) );
        model_->setData( index_, v, Qt::UserRole + 1 );
        model_->setData( index_, xvalue.value, Qt::DisplayRole );
    }

    template<> void
    TimedEventsWidget_setModelData::operator()( const adcontrols::ControlMethod::delay_width_type& t ) const
    {
        double time( 0 );
        if ( auto editor = qobject_cast<QDoubleSpinBox *>( editor_ ) )
            time = editor->value() * 1.0e-6; // us -> s

        auto value = model_->data( model_->index( index_.row(), TimedEventsWidget::impl::c_value ), Qt::UserRole + 1 ).value<EventCap::value_type>();
        if ( value.empty() )
            value = t;
        auto xvalue = boost::get< adcontrols::ControlMethod::delay_width_type >( value );

        if ( index_.column() == TimedEventsWidget::impl::c_value )
            xvalue.value.first = time;
        else if ( index_.column() ==  TimedEventsWidget::impl::c_value_2 )
            xvalue.value.second = time;
        QVariant v;
        v.setValue<>( adcontrols::ControlMethod::EventCap::value_type( xvalue ) );
        model_->setData( model_->index( index_.row(), TimedEventsWidget::impl::c_value ), v, Qt::UserRole + 1 );
        model_->setData( model_->index( index_.row(), TimedEventsWidget::impl::c_value ), xvalue.value.first * 1.0e6 );
        model_->setData( model_->index( index_.row(), TimedEventsWidget::impl::c_value_2 ), xvalue.value.second * 1.0e6 );
    }

    template<> void
    TimedEventsWidget_setModelData::operator()( const adcontrols::ControlMethod::any_type& t ) const
    {
        //if ( auto editor = qobject_cast<QWidget>( editor_ ) )
        //        ;
        model_->setData( index_, QByteArray(t.value.data(), int(t.value.size())), Qt::UserRole + 1 );
        //model_->setData( index_, QString::fromStdString(t.display_value( t.value )), Qt::DisplayRole );
    }

    /////////////////////// Value ////////////////////////////
    struct TimedEventsWidget_createValueEditor : boost::static_visitor< QWidget * > {
        QWidget * parent_;
        QStyleOptionViewItem option_;
        const QModelIndex& index_;
        TimedEventsWidget_createValueEditor( QWidget * p, QStyleOptionViewItem option, const QModelIndex& index )  : parent_( p ), option_( option ), index_( index ) {}
        template< typename T > QWidget * operator()( const T& ) const { return nullptr; }
    };

    template<> QWidget *
    TimedEventsWidget_createValueEditor::operator()( const adcontrols::ControlMethod::switch_type& t ) const {
        auto combo = new QComboBox( parent_ );
        combo->addItem( QString::fromStdString( t.choice.first ) );
        combo->addItem( QString::fromStdString( t.choice.second ) );
        return combo;
    }

    template<> QWidget *
    TimedEventsWidget_createValueEditor::operator()( const adcontrols::ControlMethod::choice_type& t ) const {
        auto combo = new QComboBox( parent_ );
        for ( auto& item: t.choice )
            combo->addItem( QString::fromStdString( item ) );
        return combo;
    }

    template<> QWidget *
    TimedEventsWidget_createValueEditor::operator()( const adcontrols::ControlMethod::voltage_type& t ) const {
        auto spinbox = new QDoubleSpinBox( parent_ );
        spinbox->setMinimum( t.limits.first );
        spinbox->setMaximum( t.limits.second );
        spinbox->setSingleStep( 0.1 ); 
        spinbox->setValue( t.value );
        spinbox->setDecimals( 2 );
        return spinbox;
    }

    template<> QWidget *
    TimedEventsWidget_createValueEditor::operator()( const adcontrols::ControlMethod::delay_width_type& t ) const {
        auto spinbox = new QDoubleSpinBox( parent_ );
        spinbox->setMinimum( 0 );
        spinbox->setMaximum( 1000.0 );
        spinbox->setSingleStep( 0.010 ); // 10ns
        spinbox->setDecimals( 2 );
        if ( index_.column() == TimedEventsWidget::impl::c_value ) {
            spinbox->setValue( t.value.first * 1e6 );
        } else {
            spinbox->setValue( t.value.second * 1e6 );
        }
        return spinbox;        
    }

    /////////////////////// Function ////////////////////////////
    template< enum TimedEventsWidget::impl::columns >
    struct TimedEventsWidget_createEditor {
        TimedEventsWidget::impl& impl_;
        TimedEventsWidget_createEditor( TimedEventsWidget::impl& impl ) : impl_( impl ) {}

        QWidget * operator ()( QWidget * parent, const QStyleOptionViewItem &option, const QModelIndex& index ) const {
            return nullptr;
        }
    };

    template<> QWidget * 
    TimedEventsWidget_createEditor<TimedEventsWidget::impl::c_item_name>::operator()( QWidget * parent, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        int currSel = index.data( Qt::UserRole + 1 ).toInt();
        if ( auto modcap = impl_.findModelCap( index ) ) {
            auto combo = new QComboBox( parent );
            for ( auto& cap : modcap->eventCaps() )
                combo->addItem( QString::fromStdString( cap.item_display_name() ) );
            combo->setCurrentIndex( currSel );
            return combo;
        }
        return nullptr;
    }

    template<> QWidget * 
    TimedEventsWidget_createEditor<TimedEventsWidget::impl::c_value>::operator()( QWidget * parent, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        auto value = index.data( Qt::UserRole + 1 ).value< EventCap::value_type >();
        if ( auto cap = impl_.findEventCap( index ) ) {
            if ( value.type() != cap->default_value().type() )
                value = cap->default_value();
            return boost::apply_visitor( TimedEventsWidget_createValueEditor( parent, option, index ), value );
        }
        return nullptr;
    }

    template<> QWidget * 
    TimedEventsWidget_createEditor<TimedEventsWidget::impl::c_value_2>::operator()( QWidget * parent, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        auto value = index.model()->data( index.model()->index( index.row(), TimedEventsWidget::impl::c_value ), Qt::UserRole + 1 ).value< EventCap::value_type >();
        if ( auto cap = impl_.findEventCap( index ) ) {
            if ( cap->default_value().type() == typeid( adcontrols::ControlMethod::delay_width_type ) ) {
                if ( value.type() != typeid( adcontrols::ControlMethod::delay_width_type ) ) 
                    value = cap->default_value();
                return boost::apply_visitor( TimedEventsWidget_createValueEditor( parent, option, index ), value );
            }
        }
        return nullptr;
    }

    //-----------------------------------------------------------------------
    //-------------------------- delegate -----------------------------------
    //-----------------------------------------------------------------------
    class TimedEventsWidget::delegate : public QStyledItemDelegate {
        impl& impl_;
    public:
        delegate( impl& t ) : impl_( t ) {
        }
        
        void paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
            if ( index.column() == impl::c_time ) {
                TimedEventsWidget_painter( painter, option, index )( index.data( Qt::EditRole ).toDouble() );

            } else if ( index.column() == impl::c_value ) {

                auto value = index.data( Qt::UserRole + 1 ).value< EventCap::value_type >();
                boost::apply_visitor( TimedEventsWidget_painter( painter, option, index ), value );

            } else if ( index.column() == impl::c_value_2 ) {

                auto value = index.model()->data( index.model()->index( index.row(), impl_.c_value ), Qt::UserRole + 1 ).value< EventCap::value_type >();
                if ( value.type() == typeid( adcontrols::ControlMethod::delay_width_type ) )
                    boost::apply_visitor( TimedEventsWidget_painter( painter, option, index ), value );
                else
                    painter->fillRect( option.rect, QColor( 0x7f, 0x7f, 0x7f, 0x40 ) ); // gray

            } else {
                assert( index.column() < TimedEventsWidget::impl::c_value );
                QStyledItemDelegate::paint( painter, option, index );
            }
        }

        void setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex& index ) const override {

            if ( index.column() == impl::c_value ) {
                auto value = index.data( Qt::UserRole + 1 ).value< EventCap::value_type >();
                boost::apply_visitor( TimedEventsWidget_setModelData( editor, model, index ), value );

            } else if ( index.column() == impl::c_value_2 ) {
                auto value = index.model()->data( index.model()->index( index.row(), impl::c_value ), Qt::UserRole + 1 ).value< EventCap::value_type >();
                boost::apply_visitor( TimedEventsWidget_setModelData( editor, model, index ), value );

            } else if ( index.column() >= impl::c_item_name ) {
                // Item selection
                if ( auto combo = qobject_cast<QComboBox *>( editor ) ) {
                    QString currText = combo->currentText();
                    int currIndex = combo->currentIndex();
                    int prevIndex = index.data( Qt::UserRole + 1 ).toInt();
                    model->setData( index, currIndex, Qt::UserRole + 1 );
                    model->setData( index, currText, Qt::EditRole );
                    if ( prevIndex != currIndex ) {
                        if ( auto cap = impl_.findEventCap( index ) ) {
                            QVariant v;
                            v.setValue<>( cap->default_value() );
                            model->setData( model->index( index.row(), impl_.c_value ), v, Qt::UserRole + 1 );
                        }
                    }
                }
            } else {
                QStyledItemDelegate::setModelData( editor, model, index );
            }
        }

        QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem &option, const QModelIndex& index ) const override {
            if ( index.column() == impl::c_item_name ) {
                return TimedEventsWidget_createEditor<TimedEventsWidget::impl::c_item_name>( impl_ )( parent, option, index );
            } else if ( index.column() == impl::c_value ) {
                return TimedEventsWidget_createEditor<TimedEventsWidget::impl::c_value>( impl_ )( parent, option, index );
            } else if ( index.column() == impl::c_value_2 ) {
                return TimedEventsWidget_createEditor<TimedEventsWidget::impl::c_value_2>( impl_ )( parent, option, index );
            }
            return QStyledItemDelegate::createEditor( parent, option, index );
        }

        bool editorEvent( QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem& option, const QModelIndex& index ) override {

            if ( ( index.column() == impl::c_value ) && ( event->type() == QEvent::MouseButtonRelease ) ) {
                auto value = index.data( Qt::UserRole + 1 ).value< EventCap::value_type >();

                if ( value.type() == typeid( adcontrols::ControlMethod::any_type ) ) {
                    QPoint pt = static_cast<QMouseEvent *>( event )->pos();
                    auto rc = option.rect;
                    rc.setLeft( rc.left() + rc.width() - 30 );
                    if ( rc.contains( pt ) ) {
                        if ( auto cap = impl_.findEventCap( index ) ) {
                            const auto& xvalue = boost::get< adcontrols::ControlMethod::any_type& >( value );
                            impl_.any_data_ = std::make_shared< TimedEvent_AnyData >( cap, index, xvalue );
                            cap->edit_any( impl_.any_data_->value_, [this] ( const adcontrols::ControlMethod::any_type& a ) { if ( impl_.any_data_ ) impl_.any_data_->callback( a ); } );
                            return true;
                        }
                    }
                }
            }
            return QStyledItemDelegate::editorEvent( event, model, option, index );
        }

        QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
            QSize sz =  QStyledItemDelegate::sizeHint( option, index );
            return sz;
        }
        
    };

}

using namespace adwidgets;

TimedEventsWidget::TimedEventsWidget(QWidget *parent) : QWidget(parent)
                                                      , impl_( new impl( this ) )
{
    if ( QVBoxLayout * layout = new QVBoxLayout( this ) ) {

        layout->setMargin(0);
        layout->setSpacing(2);

        if ( QSplitter * splitter = new QSplitter ) {
            splitter->addWidget( ( new TimedTableView ) );
            splitter->addWidget( ( new QPushButton() ) ); 
            splitter->setStretchFactor( 0, 0 );
            splitter->setStretchFactor( 1, 5 );
            splitter->setOrientation( Qt::Vertical );
            layout->addWidget( splitter );
        }
    }

    if ( auto table = findChild< TimedTableView * >() ) {
        
        table->setModel( impl_->model_.get() );
        table->setItemDelegate( new delegate( *impl_ ) );
        table->setContextMenuHandler( [this]( const QPoint& pt ){ impl_->handleContextMenu( pt ); } );
        table->setColumnHidden( impl::c_clsid, true );
        table->setSelectionBehavior( QAbstractItemView::SelectRows );
        table->setSelectionMode( QAbstractItemView::SingleSelection );
        connect( table->selectionModel(), &QItemSelectionModel::currentRowChanged, [this] ( const QModelIndex& curr, const QModelIndex& prev ) {
            if ( impl_->any_data_ ) {
                if ( impl_->any_data_->index_ == prev ) {
                    impl_->any_data_->commit();
                    QVariant v;
                    v.setValue<>( EventCap::value_type( impl_->any_data_->value_ ) );
                    impl_->model_->setData( impl_->any_data_->index_, v, Qt::UserRole + 1 );
                }
                impl_->any_data_.reset();
            }

        } );
    }

    connect( impl_->model_.get(), &QStandardItemModel::dataChanged, [this] ( const QModelIndex& _1, const QModelIndex& _2 ) { impl_->dataChanged( _1, _2 ); } );
}

TimedEventsWidget::~TimedEventsWidget()
{
}

void
TimedEventsWidget::OnCreate( const adportable::Configuration& )
{
}

void
TimedEventsWidget::OnInitialUpdate()
{
    if ( auto table = findChild< TimedTableView *>() ) {
        table->onInitialUpdate();
        table->setColumnWidth( impl::c_value, 160 );
        table->setColumnWidth( impl::c_value_2, 160 );
    }

}

void
TimedEventsWidget::onUpdate( boost::any& )
{
}

void
TimedEventsWidget::OnFinalClose()
{
}

bool
TimedEventsWidget::getContents( boost::any& a ) const
{

    if ( adportable::a_type< adcontrols::ControlMethodPtr >::is_a( a ) ) {

        adcontrols::ControlMethod::TimedEvents m;
        getContents( m );

        auto ptr = boost::any_cast< std::shared_ptr< adcontrols::ControlMethod::Method > >( a );
        ptr->append( m );

        return true;
    }

    return false;
}

bool
TimedEventsWidget::setContents( boost::any&& a )
{
    auto pi = adcontrols::ControlMethod::any_cast<>()( a, adcontrols::ControlMethod::TimedEvents::clsid() );
    if ( pi ) {
        adcontrols::ControlMethod::TimedEvents m;
        if ( pi->get( *pi, m ) ) {
            setContents( m );
            return true;
        }
    }
    return false;
}

bool
TimedEventsWidget::getContents( adcontrols::ControlMethod::TimedEvents& m ) const
{
    m.clear();

    const auto& model = *impl_->model_;

    for ( int row = 0; row < model.rowCount(); ++row ) {
        double time = model.data( model.index( row, impl::c_time ), Qt::EditRole ).toDouble();
        auto index = model.index( row, impl::c_item_name );
        if ( auto modelCap = impl_->findModelCap( index ) ) {
            if ( auto cap = impl_->findEventCap( index ) ) {
                auto value = model.data( model.index( row, impl::c_value ), Qt::UserRole + 1 ).value< adcontrols::ControlMethod::EventCap::value_type >();
                adcontrols::ControlMethod::TimedEvent timedEvent( *modelCap, *cap, time, value);
                m << timedEvent;
            }
        }
    }
    
    return true;
}

bool
TimedEventsWidget::setContents( const adcontrols::ControlMethod::TimedEvents& m )
{
    auto& model = *impl_->model_;

    model.setRowCount( 0 );
    int row = 0;

    std::for_each( m.begin(), m.end(), [&] ( const adcontrols::ControlMethod::TimedEvent& timeEvent ) {
        auto it = impl_->capList_.find( timeEvent.modelClsid() );
        if ( it != impl_->capList_.end() ) {
            const ModuleCap& moduleCap = it->second;
            auto capIt = std::find_if( moduleCap.eventCaps().begin(), moduleCap.eventCaps().end(), [&] ( const EventCap& cap ) { return cap.item_name() == timeEvent.item_name(); } );
            if ( capIt != moduleCap.eventCaps().end() ) {
                auto items = impl_->make_row( moduleCap, *capIt, timeEvent.value() );
                model.insertRow( row, items );
                ++row;
            }
        }

    } );

    return true;
}

void
TimedEventsWidget::impl::handleContextMenu( const QPoint& pt )
{
    QMenu menu;
    typedef std::pair< QAction *, std::function< void() > > action_type;
    
    if ( auto table = this_->findChild< TimedTableView * >() ) {

        std::vector< action_type > actions;
        for ( auto& cap: capList_ ) {
            QString text = QString( "Add: %1" ).arg( cap.second.model_display_name().c_str() );
            actions.push_back( std::make_pair( menu.addAction( text ), [=](){ addLine( cap.second ); }) );    
        }
        
        if ( QAction * selected = menu.exec( table->mapToGlobal( pt ) ) ) {
            auto it = std::find_if( actions.begin(), actions.end(), [=]( const action_type& t ){ return t.first == selected; });
            if ( it != actions.end() )
                (it->second)();
        }
    }
}

void
TimedEventsWidget::addModuleCap( const std::vector< adcontrols::ControlMethod::ModuleCap >& cap )
{
    std::for_each( cap.begin(), cap.end(), [&] ( const adcontrols::ControlMethod::ModuleCap& a ) { impl_->capList_ [ a.clsid() ] = a; } );
}

void
TimedEventsWidget::impl::addLine( const adcontrols::ControlMethod::ModuleCap& modcap )
{
    int row = model_->rowCount();

    auto items = make_row( modcap, modcap.eventCaps().at( 0 ), modcap.eventCaps().at( 0 ).default_value() );
    if ( ! items.empty() ) {
        model_->insertRow( row, items );
    } else {
        model_->insertRow( row );
    }
}

QList< QStandardItem * >
TimedEventsWidget::impl::make_row( const adcontrols::ControlMethod::ModuleCap& modcap, const EventCap& cap, const EventCap::value_type& value )
{
    QList< QStandardItem * > items;
    if ( modcap.eventCaps().empty() )
        return items;

    // clsid (hidden)
    if ( auto item = new QStandardItem() ) {
        QVariant v;
        v.setValue<>( modcap.clsid() );
        item->setData( v, Qt::UserRole + 1 );
        item->setEditable( false );
        items.push_back( item );
    }
    
    // time (seconds)
    if ( auto item = new QStandardItem() ) {
        item->setData( 0.0, Qt::EditRole );
        items.push_back( item );
    }

    // module display_name, not editable
    if ( auto item = new QStandardItem() ) {
        item->setData( QString::fromStdString( modcap.model_display_name() ), Qt::EditRole );
        item->setEditable( false );
        items.push_back( item );
    }

    // item display_name    
    if ( auto item = new QStandardItem() ) {
        item->setData( QString::fromStdString( cap.item_display_name() ), Qt::EditRole );
        item->setData( 0, Qt::UserRole + 1 );
        items.push_back( item );
    }

    // value
    if ( auto item = new QStandardItem() ) {
        if ( !modcap.eventCaps().empty() ) {
            QVariant v;
            v.setValue<>( value );
            item->setData( v, Qt::UserRole + 1 );
        } else {
            item->setData( 0.0, Qt::EditRole );
        }
        items.push_back( item );
    }

    // value_2 ( delay_width_type only )
    if ( auto item = new QStandardItem() ) {
        if ( value.type() == typeid( adcontrols::ControlMethod::delay_width_type ) ) {
            auto xvalue = boost::get< adcontrols::ControlMethod::delay_width_type >( value );
            item->setData( xvalue.value.second * 1.0e6 ); // s -> us
        } else {
            item->setData( "" );
            item->setEditable( false );
        }
    }

    return items;
}
