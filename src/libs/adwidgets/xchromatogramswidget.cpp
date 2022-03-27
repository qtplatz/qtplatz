/**************************************************************************
** Copyright (C) 2010-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "xchromatogramswidget.hpp"
#include "xchromatogramsform.hpp"
#include "xchromatogramstable.hpp"
#include <adportable/is_type.hpp>
#include <adportable/debug.hpp>
#include <adcontrols/scanlaw.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/controlmethod/tofchromatogrammethod.hpp>
#include <adcontrols/controlmethod/tofchromatogramsmethod.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <QBoxLayout>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QSplitter>
#include <QStandardItemModel>
#include <boost/format.hpp>
#include <boost/json.hpp>
#include <boost/filesystem.hpp>
#include <ratio>
#include <cmath>

namespace adwidgets {

    class XChromatogramsWidget::impl {
        XChromatogramsWidget * this_;
    public:
        // enum columns { c_id, c_formula, c_mass, c_masswindow, c_time, c_timewindow, c_algo, c_protocol, ncolumns };

        QString connString_;

        impl( XChromatogramsWidget * p ) : this_( p )
                                           , model_( std::make_unique< QStandardItemModel >() ) {
            // model_->setColumnCount( ncolumns );
            // model_->setHeaderData( c_id,         Qt::Horizontal, QObject::tr( "id" ) );
            // model_->setHeaderData( c_formula,    Qt::Horizontal, QObject::tr( "Formula" ) );
            // model_->setHeaderData( c_mass,       Qt::Horizontal, QObject::tr( "<i>m/z</i>" ) );
            // model_->setHeaderData( c_masswindow, Qt::Horizontal, QObject::tr( "Window(Da)" ) );
            // model_->setHeaderData( c_time,       Qt::Horizontal, QObject::tr( "Time(&mu;s)" ) );
            // model_->setHeaderData( c_timewindow, Qt::Horizontal, QObject::tr( "Window(&mu;s)" ) );
            // model_->setHeaderData( c_algo,       Qt::Horizontal, QObject::tr( "Method" ) );
            // model_->setHeaderData( c_protocol,   Qt::Horizontal, QObject::tr( "Protocol#" ) );
        }

        ~impl() {
        }

        void dataChanged( const QModelIndex& _1, const QModelIndex& _2 ) {
        }

        void handleContextMenu( const QPoint& pt );
        void addLine();

        std::unique_ptr< QStandardItemModel > model_;
        std::shared_ptr< const adcontrols::MassSpectrometer > spectrometer_;
    };

}

using namespace adwidgets;

XChromatogramsWidget::XChromatogramsWidget(QWidget *parent) : QWidget(parent)
                                                                , impl_( new impl( this ) )
{
    if ( QVBoxLayout * layout = new QVBoxLayout( this ) ) {

        layout->setMargin(0);
        layout->setSpacing(2);

        if ( QSplitter * splitter = new QSplitter ) {
            splitter->addWidget( ( new XChromatogramsForm ) );
            splitter->addWidget( ( new XChromatogramsTable ) );
            splitter->setStretchFactor( 0, 0 );
            splitter->setStretchFactor( 1, 3 );
            splitter->setOrientation ( Qt::Horizontal );
            layout->addWidget( splitter );
        }
    }
}

XChromatogramsWidget::~XChromatogramsWidget()
{
}

void
XChromatogramsWidget::OnCreate( const adportable::Configuration& )
{
}

void
XChromatogramsWidget::OnInitialUpdate()
{
    if ( auto form = findChild< XChromatogramsForm * >() )
        form->OnInitialUpdate();

    // if ( auto table = findChild< MolTableView *>() ) {
    //     table->onInitialUpdate();
    //     // connect( table, &MolTableView::onContextMenu, this, &XChromatogramsWidget::handleContextMenu );
    // }

    setContents( adcontrols::TofChromatogramsMethod() );
}

void
XChromatogramsWidget::onUpdate( boost::any&& )
{
}

void
XChromatogramsWidget::OnFinalClose()
{
}

bool
XChromatogramsWidget::getContents( boost::any& a ) const
{
    adcontrols::TofChromatogramsMethod m;
    getContents( m );

    if ( adportable::a_type< adcontrols::ControlMethodPtr >::is_a( a ) ) {
        auto ptr = boost::any_cast< std::shared_ptr< adcontrols::ControlMethod::Method > >( a );
        ptr->append( m );
    }
    return false;
}

bool
XChromatogramsWidget::setContents( boost::any&& a )
{
    ADDEBUG() << "------ setContents via any ------";
    auto pi = adcontrols::ControlMethod::any_cast<>()( a, adcontrols::TofChromatogramsMethod::clsid() );
    if ( pi ) {
        adcontrols::TofChromatogramsMethod m;
        if ( pi->get( *pi, m ) ) {
            setContents( m );
            return true;
        }
    }
    return false;
}

adcontrols::TofChromatogramsMethod
XChromatogramsWidget::method() const
{
    adcontrols::TofChromatogramsMethod m;
    getContents( m );
    return m;
}


bool
XChromatogramsWidget::getContents( adcontrols::TofChromatogramsMethod& m ) const
{
    m.clear();

    if ( auto form = findChild< XChromatogramsForm *>() )
        form->getContents( m );

    //QSqlDatabase db = QSqlDatabase::database( impl_->connString_ );
    auto& model = *impl_->model_;
    for ( int row = 0; row < model.rowCount(); ++row ) {
        adcontrols::TofChromatogramMethod item;
        namespace xic = adcontrols::xic;

        item.setEnable( model.index( row, c_formula ).data( Qt::CheckStateRole ) == Qt::Checked );
        item.setFormula( model.index( row, c_formula ).data( Qt::EditRole ).toString().toStdString() );
        item.setMass( model.index( row, c_mass ).data( Qt::EditRole ).toDouble() );
        item.setMassWindow( model.index( row, c_masswindow ).data( Qt::EditRole ).toDouble() );
		item.setTime( model.index( row, c_time ).data( Qt::EditRole ).toDouble() / std::micro::den );
        item.setTimeWindow( model.index( row, c_timewindow ).data( Qt::EditRole ).toDouble() / std::micro::den );
        item.setIntensityAlgorithm( xic::eIntensityAlgorishm(  model.index( row, c_algo ).data( Qt::EditRole ).toInt() ) );
        item.setProtocol( model.index( row, c_protocol ).data( Qt::EditRole ).toInt() );
        m << item;
    }

    return true;
}

bool
XChromatogramsWidget::setContents( const adcontrols::TofChromatogramsMethod& m )
{
    ADDEBUG() << "------ setContents ------";

    QSignalBlocker block( this );

    if ( auto form = findChild< XChromatogramsForm *>() )
        form->setContents( m );

    auto& model = *impl_->model_;
    model.setRowCount( m.size() );

    int row(0);
    for ( auto& trace: m ) {
        model.setData( model.index( row, c_formula ), QString::fromStdString( trace.formula() ) );

        if ( auto item = model.item( row, c_formula ) ) {
            item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
            model.setData( model.index( row, c_formula ), trace.enable() ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
        }

        model.setData( model.index( row, c_mass ), trace.mass() );
        model.setData( model.index( row, c_masswindow ), trace.massWindow() );
        model.setData( model.index( row, c_time ), trace.time() * std::micro::den );
        model.setData( model.index( row, c_timewindow ), trace.timeWindow() * std::micro::den );
        model.setData( model.index( row, c_algo ), trace.intensityAlgorithm() );
        model.setData( model.index( row, c_protocol ), trace.protocol() );
        ++row;
    }
    return true;

}

void
XChromatogramsWidget::setDigitizerMode( bool softAverage )
{
    if ( auto form = findChild< XChromatogramsForm * >() ) {
        form->setDigitizerMode( softAverage );
    }
}

void
XChromatogramsWidget::impl::handleContextMenu( const QPoint& pt )
{
}

void
XChromatogramsWidget::setMassSpectrometer( std::shared_ptr< const adcontrols::MassSpectrometer > sp )
{
    impl_->spectrometer_ = sp;
    ADDEBUG() << sp->calibrationFilename();

    boost::filesystem::path path( sp->calibrationFilename() );
    if ( auto form = findChild< XChromatogramsForm * >() ) {
        form->setCalibrationFilename( QString::fromStdString( path.stem().string() )
                                      , QString::fromStdString( sp->calibrationFilename() ) );
    }
}
