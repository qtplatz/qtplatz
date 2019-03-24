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

#include "adtraceswidget.hpp"
#include "tableview.hpp"
#include <adportable/is_type.hpp>
#include <adportable/debug.hpp>
#include <admethods/controlmethod/adtracemethod.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <QBoxLayout>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QMessageBox>
#include <QSplitter>
#include <QStandardItemModel>
#include <ratio>

using admethods::controlmethod::ADTraceMethod;

namespace adwidgets {

    class ADTracesWidget::impl {
        ADTracesWidget * this_;
    public:
        enum columns { c_id, c_legend, c_vOffset, ncolumns };

        impl( ADTracesWidget * p ) : this_( p )
                                   , model_( std::make_unique< QStandardItemModel >() ) {

            model_->setColumnCount( ncolumns );
            model_->setHeaderData( c_id,         Qt::Horizontal, QObject::tr( "id" ) );
            model_->setHeaderData( c_legend,    Qt::Horizontal, QObject::tr( "Legend" ) );
            model_->setHeaderData( c_vOffset,       Qt::Horizontal, QObject::tr( "Offset(V)" ) );
        }

        ~impl() {
        }

        void setMethod( const ADTraceMethod& m ) {
            model_->setRowCount( m.size() );
            for ( size_t row = 0; row < m.size(); ++row ) {
                const auto& t = m[ row ]; // ADTrace
                model_->setData( model_->index( row, c_legend ), QString::fromStdString( t.legend() ) );
                model_->setData( model_->index( row, c_vOffset), t.vOffset() );
            }
        }

        void dataChanged( const QModelIndex& _1, const QModelIndex& _2 ) {
            ADDEBUG() << "**** " << __FUNCTION__ << " **** row: " << _1.row();
            // if ( _1.column() == c_formula ) {
            //     int row = _1.row();
            //     double exactMass = MolTableView::getMonoIsotopicMass( _1.data( Qt::EditRole ).toString() );
            //     if ( exactMass > 0.7 ) {
            //         model_->setData( model_->index( row, c_mass ), exactMass );
            //         model_->setData( model_->index( row, c_masswindow ), 0.005 );
            //     } else {
            //         for ( auto& id : { c_mass, c_masswindow, c_time, c_timewindow } )
            //             model_->setData( model_->index( row, id ), QVariant() );
            //     }
            // }
            emit this_->valueChanged();
        }

        void handleContextMenu( const QPoint& pt );
        void addLine();

        std::unique_ptr< QStandardItemModel > model_;
    };

}

using namespace adwidgets;

ADTracesWidget::ADTracesWidget(QWidget *parent) : QWidget(parent)
                                                , impl_( new impl( this ) )
{
    if ( QVBoxLayout * layout = new QVBoxLayout( this ) ) {

        layout->setMargin(0);
        layout->setSpacing(2);

        if ( QSplitter * splitter = new QSplitter ) {
            //splitter->addWidget( ( new ADTracesForm ) );
            splitter->addWidget( ( new TableView ) );
            //splitter->setStretchFactor( 0, 0 );
            //splitter->setStretchFactor( 1, 3 );
            splitter->setOrientation ( Qt::Horizontal );
            layout->addWidget( splitter );
        }
    }

    if ( auto table = findChild< TableView * >() ) {
        table->setModel( impl_->model_.get() );
        table->setColumnHidden( impl::c_id, true );
    }

    // if ( auto form = findChild< ADTracesForm * >() )  {
    //     connect( form, &ADTracesForm::applyTriggered, [this](){ emit applyTriggered(); } );
    //     connect( form, &ADTracesForm::valueChanged, [this](){ emit valueChanged(); } );
    // }

    connect( impl_->model_.get(), &QStandardItemModel::dataChanged
             , [this] ( const QModelIndex& _1, const QModelIndex& _2 ) { impl_->dataChanged( _1, _2 ); } );
}

ADTracesWidget::~ADTracesWidget()
{
}

void
ADTracesWidget::OnCreate( const adportable::Configuration& )
{
}

void
ADTracesWidget::OnInitialUpdate()
{
    auto m = ADTraceMethod();
    impl_->setMethod( m );

    //if ( auto form = findChild< ADTracesForm * >() )
    //    form->OnInitialUpdate();

    if ( auto table = findChild< TableView *>() ) {
        //table->onInitialUpdate();
        // connect( table, &MolTableView::onContextMenu, this, &ADTracesWidget::handleContextMenu );
    }

    //setContents( adcontrols::ADTracesMethod() );
}

void
ADTracesWidget::onUpdate( boost::any&& )
{
}

void
ADTracesWidget::OnFinalClose()
{
}

bool
ADTracesWidget::getContents( boost::any& a ) const
{
    if ( adportable::a_type< adcontrols::ControlMethodPtr >::is_a( a ) ) {

        ADTraceMethod m;
        getContents( m );

        auto ptr = boost::any_cast< std::shared_ptr< adcontrols::ControlMethod::Method > >( a );
        ADDEBUG() << "**** " << __FUNCTION__ << " ptr = " << (void*)(ptr.get());
        ptr->append( m );

        return true;
    }
    ADDEBUG() << "**** " << __FUNCTION__ << " no pointer";
    return false;
}

bool
ADTracesWidget::setContents( boost::any&& a )
{
    ADDEBUG() << "**** " << __FUNCTION__ << "(boost::any) ****";
    if ( auto pi = adcontrols::ControlMethod::any_cast<>()( a, ADTraceMethod::clsid() ) ) {
        ADTraceMethod m;
        if ( pi->get( *pi, m ) ) {
            setContents( m );
            return true;
        }
    }
    ADDEBUG() << "**** " << __FUNCTION__ << " no ADTraceMethod found ****";
    return false;
}

bool
ADTracesWidget::getContents( ADTraceMethod& m ) const
{
    ADDEBUG() << "**** " << __FUNCTION__ << " ****";
    auto& model = *impl_->model_;

    for ( int row = 0; row < model.rowCount() && row < m.size(); ++row ) {
        auto enable = model.index( row, impl::c_legend ).data( Qt::CheckStateRole ) == Qt::Checked;
        auto legend = model.index( row, impl::c_legend ).data( Qt::EditRole ).toString().toStdString();
        auto vOffset = model.index( row, impl::c_vOffset ).data( Qt::EditRole ).toDouble();
        m[ row ] = std::make_tuple( enable, legend, vOffset );
    }

    return true;
}

bool
ADTracesWidget::setContents( const ADTraceMethod& m )
{
    //if ( auto form = findChild< ADTracesForm *>() )
    //    form->setContents( m );
    ADDEBUG() << "**** " << __FUNCTION__ << "(ADTraceMethod) **** " << m.toJson();

    impl_->setMethod( m );

    return true;

}

void
ADTracesWidget::handleContextMenu( QMenu& menu, const QPoint& pt )
{
    menu.addAction( "Simulate MS Spectrum", this, SLOT( run() ) );
}

void
ADTracesWidget::impl::handleContextMenu( const QPoint& pt )
{
    QMenu menu;
    typedef std::pair< QAction *, std::function< void() > > action_type;

    if ( auto table = this_->findChild< TableView * >() ) {

        std::vector< action_type > actions;
        actions.push_back( std::make_pair( menu.addAction( "add line" ), [this](){ addLine(); }) );

        if ( QAction * selected = menu.exec( table->mapToGlobal( pt ) ) ) {
            auto it = std::find_if( actions.begin(), actions.end(), [=]( const action_type& t ){ return t.first == selected; });
            if ( it != actions.end() )
                (it->second)();
        }
    }
}

void
ADTracesWidget::impl::addLine()
{
    model_->insertRow( model_->rowCount() );
}

void
ADTracesWidget::handleScanLawChanged()
{

}

QByteArray
ADTracesWidget::readJson() const
{
    return QByteArray();
}

void
ADTracesWidget::setJson( const QByteArray& json )
{
}
