/**************************************************************************
** Copyright (C) 2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2019 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "adtracewidget.hpp"
#include "delegatehelper.hpp"
#include "tableview.hpp"
#include <adportable/is_type.hpp>
#include <adportable/debug.hpp>
#include <admethods/controlmethod/adtracemethod.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <QBoxLayout>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QMessageBox>
#include <QSplitter>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QSignalBlocker>
#include <ratio>

using admethods::controlmethod::ADTraceMethod;

namespace adwidgets {


    class ADTraceWidget::delegate : public QStyledItemDelegate {
    public:
        enum { c_legend, c_vOffset, c_ncolumn };

        delegate() {
        }

        void paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
            QStyleOptionViewItem opt(option);
            initStyleOption( &opt, index );
            opt.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
            if ( index.column() == c_legend )
                DelegateHelper::render_html2( painter, opt, index.data().toString() );
            else
                QStyledItemDelegate::paint( painter, opt, index );
        }

        QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
            if ( index.column() == c_legend ) {
                return DelegateHelper::html_size_hint( option, index );
            } else {
                return QStyledItemDelegate::sizeHint( option, index );
            }
        }

        QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem &option, const QModelIndex& index ) const override {
            if ( index.column() == c_vOffset ) {
                auto widget = new QDoubleSpinBox( parent );
                widget->setMinimum( -2.6 ); widget->setMaximum( 2.6 ); widget->setSingleStep( 0.01 ); widget->setDecimals( 7 );
                widget->setValue( index.data( Qt::EditRole ).toDouble() );
                return widget;
            } else {
                return QStyledItemDelegate::createEditor( parent, option, index );
            }
        }

    };


    class ADTraceWidget::impl {
    public:
        QStandardItemModel model_;
        admethods::controlmethod::ADTraceMethod data_;
        TableView * tableView_;

        impl() : tableView_( new TableView ) {
            tableView_->setModel( &model_ );
            tableView_->setSelectionMode( QAbstractItemView::SingleSelection );
            tableView_->setItemDelegate( new delegate() );
        }

        void init() {
            model_.setColumnCount( delegate::c_ncolumn );
            model_.setHeaderData( delegate::c_legend, Qt::Horizontal, QObject::tr( "Legend" ) );
            model_.setHeaderData( delegate::c_vOffset, Qt::Horizontal, QObject::tr( "Offset(V)" ) );
            setData( ADTraceMethod() );
            tableView_->setColumnWidth( 0, 240 );
        }

        void setData( const ADTraceMethod& m ) {

            QSignalBlocker block( &model_ );

            model_.setRowCount( m.size() );
            for ( size_t row = 0; row < m.size(); ++row ) {
                const auto& t = m[ row ]; // ADTrace

                model_.setData( model_.index( row, 0 ), QString::fromStdString( t.legend() ) );
                model_.setData( model_.index( row, 1 ), t.vOffset() );

                if ( auto item = model_.item( row, 0 ) ) {
                    item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
                    model_.setData( model_.index( row, 0 ), t.enable() ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
                }
            }
        }

        void setVOffsets( const QVector< double >& values ) {

            QSignalBlocker block( &model_ );
            for ( size_t row = 0; row < values.size(); ++row )
                model_.setData( model_.index( row, 1 ), values[ row ] );
            tableView_->viewport()->update();
        }

        bool fetch( ADTraceMethod& data ) const  {
            for ( size_t row = 0; row < model_.rowCount() && row < data.size(); ++ row ) {
                auto enable = model_.index( row, 0 ).data( Qt::CheckStateRole ) == Qt::Checked;
                auto legend = model_.index( row, 0 ).data( Qt::EditRole ).toString().toStdString();
                auto vOffset = model_.index( row, 1 ).data( Qt::EditRole ).toDouble();
                data[ row ] = std::make_tuple( enable, legend, vOffset );
            }
            return true;
        }

    };
}

using namespace adwidgets;

ADTraceWidget::ADTraceWidget(QWidget *parent) : QWidget(parent)
                                                , impl_( new impl )
{
    if ( QVBoxLayout * layout = new QVBoxLayout( this ) ) {

        layout->setMargin(0);
        layout->setSpacing(2);

        if ( QSplitter * splitter = new QSplitter ) {
            splitter->addWidget( impl_->tableView_ );
            splitter->setOrientation ( Qt::Horizontal );
            layout->addWidget( splitter );
        }
    }

    impl_->init();

    connect( &impl_->model_, &QStandardItemModel::dataChanged
             , [this]( const QModelIndex &topLeft, const QModelIndex &bottomRight){ emit dataChanged( topLeft.row(), topLeft.column() ); });
}

ADTraceWidget::~ADTraceWidget()
{
}

void
ADTraceWidget::OnCreate( const adportable::Configuration& )
{
}

void
ADTraceWidget::OnInitialUpdate()
{
}

void
ADTraceWidget::onUpdate( boost::any&& )
{
}

void
ADTraceWidget::OnFinalClose()
{
}

bool
ADTraceWidget::getContents( boost::any& a ) const
{
    if ( adportable::a_type< adcontrols::ControlMethodPtr >::is_a( a ) ) {

        ADTraceMethod m;
        getContents( m );

        auto ptr = boost::any_cast< std::shared_ptr< adcontrols::ControlMethod::Method > >( a );
        ptr->append( m );

        return true;
    }
    return false;
}

bool
ADTraceWidget::setContents( boost::any&& a )
{
    if ( auto pi = adcontrols::ControlMethod::any_cast<>()( a, ADTraceMethod::clsid() ) ) {
        ADTraceMethod m;
        if ( pi->get( *pi, m ) ) {
            setContents( m );
            return true;
        }
    }
    return false;
}

bool
ADTraceWidget::getContents( ADTraceMethod& m ) const
{
    return impl_->fetch( m );
}

bool
ADTraceWidget::setContents( const ADTraceMethod& m )
{
    impl_->setData( m );
    return true;
}

void
ADTraceWidget::handleContextMenu( QMenu& menu, const QPoint& pt )
{
    //menu.addAction( "Simulate MS Spectrum", this, SLOT( run() ) );
}

QByteArray
ADTraceWidget::readJson() const
{
    ADTraceMethod m;
    impl_->fetch( m );
    auto json = m.toJson();
    return QByteArray( json.data(), json.size() );
}

void
ADTraceWidget::setJson( const QByteArray& json )
{
    ADTraceMethod m;
    m.fromJson( json.toStdString() );
}

void
ADTraceWidget::handleVOffsets( const QVector< double >& values )
{
    impl_->setVOffsets( values );
    emit dataChanged( 0, 0 );
}
