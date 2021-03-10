/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "moleculeswidget.hpp"
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/isocluster.hpp>
#include <adcontrols/isotopecluster.hpp>
#include <adcontrols/lapfinder.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/molecule.hpp>
#include <adcontrols/moltable.hpp>
#include <adcontrols/mssimulatormethod.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/scanlaw.hpp>
#include <adcontrols/targeting.hpp>
#include <adportable/debug.hpp>
#include <adportable/is_type.hpp>
#include <adportfolio/folium.hpp>
#include <adutils/constants.hpp> // clsid for massspectrometer
#include <adwidgets/moltable.hpp>
#include <boost/json.hpp>
#include <QBoxLayout>
#include <QMenu>
#include <QSplitter>
#include <QAbstractItemModel>

namespace accutof {
    class MoleculesWidget::impl {
    public:
        std::weak_ptr< const adcontrols::MassSpectrometer > massSpectrometer_;
    };
}

using namespace accutof;

MoleculesWidget::MoleculesWidget(QWidget *parent) : QWidget(parent)
                                                  , impl_( new impl() )
{
    if ( QVBoxLayout * layout = new QVBoxLayout( this ) ) {

        layout->setMargin(0);
        layout->setSpacing(2);

        if ( QSplitter * splitter = new QSplitter ) {
            splitter->addWidget( ( new adwidgets::MolTable ) );
            //splitter->setStretchFactor( 0, 0 );
            //splitter->setStretchFactor( 1, 3 );
            splitter->setOrientation ( Qt::Horizontal );
            layout->addWidget( splitter );
        }
    }
    // if ( auto form = findChild< MSSimulatorForm * >() ) {
    //     connect( form, &MSSimulatorForm::triggerProcess, [this] { run(); } );
    //     connect( form, &MSSimulatorForm::onLapChanged, this, &MoleculesWidget::handleLapChanged );
    // }
}

MoleculesWidget::~MoleculesWidget()
{
    delete impl_;
}

QWidget *
MoleculesWidget::create( QWidget * parent )
{
    return new MoleculesWidget( parent );
}

void
MoleculesWidget::OnCreate( const adportable::Configuration& )
{
}

void
MoleculesWidget::OnInitialUpdate()
{
    ADDEBUG() << "---------- initial update ------------";
    // if ( auto form = findChild< MSSimulatorForm * >() )
    //     form->OnInitialUpdate();
    if ( auto table = findChild< adwidgets::MolTable *>() ) {
        table->onInitialUpdate();
        // connect( table, &adwidgets::MolTable::onContextMenu, this, &MoleculesWidget::handleContextMenu );
        if ( auto model = table->model() )
            connect( model, &QAbstractItemModel::dataChanged, this, &MoleculesWidget::handleDataChanged ); //
    }
}

void
MoleculesWidget::onUpdate( boost::any&& )
{
}

void
MoleculesWidget::OnFinalClose()
{
}

bool
MoleculesWidget::getContents( boost::any& a ) const
{
    return false;
}

bool
MoleculesWidget::setContents( boost::any&& a )
{
    return false;
}

// This API added for mssimulator to determinse profile spectrum;  it used to use the interface for mspeakinfo, which take centroid
// but also nullptr in order to clear existing centroid
bool
MoleculesWidget::setContents( boost::any&& a, const std::string& dataSource )
{
    boost::system::error_code ec;
    auto jv = boost::json::parse( dataSource, ec );
    if ( ec )
        ADDEBUG() << ec.message();
    return true;
}

void
MoleculesWidget::handleDataChanged(const QModelIndex& topLeft, const QModelIndex& )
{
    if ((topLeft.column() != adwidgets::MolTable::c_mass) ) {
        return;
    }
}

void
MoleculesWidget::setMassSpectrometer( std::shared_ptr< const adcontrols::MassSpectrometer > p )
{
    impl_->massSpectrometer_ = p;

    if ( p ) {
        if ( p->massSpectrometerClsid() == qtplatz::infitof::iids::uuid_massspectrometer ) {
            std::vector< std::pair< adwidgets::MolTable::fields, bool > > hides
                = { { adwidgets::MolTable::c_nlaps, false }, { adwidgets::MolTable::c_apparent_mass, false }, { adwidgets::MolTable::c_time, false } };
            if ( auto table = findChild< adwidgets::MolTable * >() )
                table->setColumHide( hides );
        } else {
            std::vector< std::pair< adwidgets::MolTable::fields, bool > > hides
                = { { adwidgets::MolTable::c_nlaps, true }, { adwidgets::MolTable::c_apparent_mass, true }, { adwidgets::MolTable::c_time, true } };
            if ( auto table = findChild< adwidgets::MolTable * >() )
                table->setColumHide( hides );
        }
    }
}
