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

#include "moltable.hpp"
#include "mssimulatorform.hpp"
#include "mssimulatorwidget.hpp"
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/molecule.hpp>
#include <adcontrols/mssimulatormethod.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/targeting.hpp>
#include <adcontrols/moltable.hpp>
#include <adcontrols/isocluster.hpp>
#include <adcontrols/isotopecluster.hpp>
#include <adportable/debug.hpp>
#include <adportable/is_type.hpp>
#include <adportfolio/folium.hpp>
#include <infitofcontrols/constants.hpp> // clsid for massspectrometer
#include <boost/json.hpp>
#include <QBoxLayout>
#include <QMenu>
#include <QSplitter>

namespace adwidgets {
    class MSSimulatorWidget::impl {
    public:
        std::weak_ptr< const adcontrols::MassSpectrometer > massSpectrometer_;
        std::weak_ptr< const adcontrols::MassSpectrum > massSpectrum_;
    };
}

using namespace adwidgets;

MSSimulatorWidget::MSSimulatorWidget(QWidget *parent) : QWidget(parent)
                                                      , impl_( new impl() )
{
    if ( QVBoxLayout * layout = new QVBoxLayout( this ) ) {

        layout->setMargin(0);
        layout->setSpacing(2);

        if ( QSplitter * splitter = new QSplitter ) {
            splitter->addWidget( ( new MSSimulatorForm ) );
            splitter->addWidget( ( new MolTable ) );
            splitter->setStretchFactor( 0, 0 );
            splitter->setStretchFactor( 1, 3 );
            splitter->setOrientation ( Qt::Horizontal );
            layout->addWidget( splitter );
        }
    }
    if ( auto form = findChild< MSSimulatorForm * >() )
        connect( form, &MSSimulatorForm::triggerProcess, [this] { run(); } );
}

MSSimulatorWidget::~MSSimulatorWidget()
{
    delete impl_;
}

QWidget *
MSSimulatorWidget::create( QWidget * parent )
{
    return new MSSimulatorWidget( parent );
}

void
MSSimulatorWidget::OnCreate( const adportable::Configuration& )
{
}

void
MSSimulatorWidget::OnInitialUpdate()
{
    if ( auto form = findChild< MSSimulatorForm * >() )
        form->OnInitialUpdate();

    if ( auto table = findChild< MolTable *>() ) {
        table->onInitialUpdate();
        connect( table, &MolTable::onContextMenu, this, &MSSimulatorWidget::handleContextMenu );
    }
}

void
MSSimulatorWidget::onUpdate( boost::any&& )
{
}

void
MSSimulatorWidget::OnFinalClose()
{
}

std::unique_ptr< adcontrols::MSSimulatorMethod >
MSSimulatorWidget::getMethod() const
{
    auto m = std::make_unique< adcontrols::MSSimulatorMethod >();

    if ( auto form = findChild< MSSimulatorForm * >() )
        form->getContents( *m );

    if ( auto table = findChild< MolTable * >() )
        table->getContents( m->molecules() );

    return m;
}

bool
MSSimulatorWidget::getContents( boost::any& a ) const
{
    if ( auto pm = boost::any_cast<adcontrols::ProcessMethod *>( a ) ) {
        if ( auto method = getMethod() ) {
            if ( auto m = pm->find< adcontrols::MSSimulatorMethod >() ) {
                *m = *method;
            } else {
                *pm << *method;
            }
            return true;
        }
    }
    return false;
}

bool
MSSimulatorWidget::setContents( boost::any&& a )
{
    if ( adportable::a_type< adcontrols::ProcessMethod >::is_a( a ) ) {

        const adcontrols::ProcessMethod& pm = boost::any_cast<adcontrols::ProcessMethod&>( a );
        if ( auto cm = pm.find< adcontrols::MSSimulatorMethod >() ) {

            if ( auto form = findChild< MSSimulatorForm * >() ) {
                form->setContents( *cm );

                if ( auto table = findChild< MolTable *>() )
                    table->setContents( cm->molecules() );

                return true;
            }

        }
    }
    return false;
}

// This API added for mssimulator to determinse profile spectrum;  it used to use the interface for mspeakinfo, which take centroid
// but also nullptr in order to clear existing centroid
bool
MSSimulatorWidget::setContents( boost::any&& a, const std::string& dataSource )
{
    boost::system::error_code ec;
    auto jv = boost::json::parse( dataSource, ec );
    if ( ec )
        ADDEBUG() << ec.message();

    if ( adportable::a_type< std::shared_ptr< adcontrols::MassSpectrum > >::is_a( a ) ) {
        ADDEBUG() << "found mass spectrum from " << dataSource;
        if ( auto ptr = boost::any_cast< std::shared_ptr< adcontrols::MassSpectrum> >( a ) ) {
            impl_->massSpectrum_ = ptr;
            if ( auto form = findChild< MSSimulatorForm * >() )
                form->setMassSpectrum( ptr );
        }
        return true;
    }
    return true;
}

void
MSSimulatorWidget::setTimeSquaredScanLaw( double flength, double acceleratorVoltage, double tdelay )
{
    if ( auto form = findChild< MSSimulatorForm * >() ) {
        adcontrols::MSSimulatorMethod m;
        form->getContents( m );
        m.setLength( flength );
        m.setAcceleratorVoltage( acceleratorVoltage );
        m.setTDelay( tdelay );
        form->setContents( m );
    }
}

void
MSSimulatorWidget::handleContextMenu( QMenu& menu, const QPoint& pt )
{
    menu.addAction( "Simulate MS Spectrum", this, SLOT( run() ) );
}

void
MSSimulatorWidget::run()
{
    emit triggerProcess( "MSSimulatorWidget" );
}

void
MSSimulatorWidget::setMassSpectrometer( std::shared_ptr< const adcontrols::MassSpectrometer > p )
{
    impl_->massSpectrometer_ = p;

    if ( auto form = findChild< MSSimulatorForm * >() ) {
        form->setMassSpectrometer( p );
    }
    if ( p ) {
        if ( p->massSpectrometerClsid() == infitof::iids::uuid_massspectrometer ) {
            ADDEBUG() << "found infiTOF";
        } else {
            ADDEBUG() << p->massSpectrometerName() << "\t" << p->massSpectrometerClsid();
        }
    }
}


std::shared_ptr< adcontrols::MassSpectrum >
MSSimulatorWidget::massSpectrum() const
{
    std::shared_ptr< adcontrols::MassSpectrum > ms = std::make_shared< adcontrols::MassSpectrum >();
    if ( auto src = impl_->massSpectrum_.lock() )
        ms->clone( *src );
    ms->setCentroid( adcontrols::CentroidNative );

    const double abundance_threshold = 1.0e-6;

    if ( auto m = getMethod() ) {
        std::vector< adcontrols::mol::molecule > molecules;
        for ( auto& mol : m->molecules().data() ) {
            if ( mol.enable() ) {
                auto molecule = adcontrols::ChemicalFormula::toMolecule( mol.formula(), mol.adducts() );
                if ( m->chargeStateMin() == 0 ) {
                    adcontrols::isoCluster( abundance_threshold, m->resolvingPower() )( molecule, molecule.charge() );
                    molecules.emplace_back( molecule );
                } else {
                    for ( int charge = m->chargeStateMin(); charge <= m->chargeStateMax(); ++charge ) {
                        molecule.setCharge( charge );
                        adcontrols::isoCluster( abundance_threshold, m->resolvingPower() )( molecule, charge );
                        molecules.emplace_back( molecule );
                    }
                }
            }
        }
        return
            adcontrols::isotopeCluster::toMassSpectrum( molecules, impl_->massSpectrum_.lock(), impl_->massSpectrometer_.lock(), m->mode() );
        return ms;
    }

    return ms;
}
