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

#include "mssimulatorwidget.hpp"
#include "mssimulatorform.hpp"
#include "moltable.hpp"
#include <infitofcontrols/constants.hpp> // clsid for massspectrometer
#include <adportable/is_type.hpp>
#include <adportable/debug.hpp>
#include <adcontrols/mssimulatormethod.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <QSplitter>
#include <QBoxLayout>
#include <QMenu>

namespace adwidgets {
    class MSSimulatorWidget::impl {
    public:
        std::weak_ptr< const adcontrols::MassSpectrometer > massSpectrometer_;
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

bool
MSSimulatorWidget::getContents( boost::any& a ) const
{
    if ( auto form = findChild< MSSimulatorForm * >() ) {

        adcontrols::MSSimulatorMethod method;
        form->getContents( method );

        if ( auto table = findChild< MolTable * >() ) {
            table->getContents( method.molecules() );

            if ( auto pm = boost::any_cast<adcontrols::ProcessMethod *>( a ) ) {
                if ( auto m = pm->find< adcontrols::MSSimulatorMethod >() ) {
                    *m = method;
                } else {
                    *pm << method;
                }
                return true;
            }
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
