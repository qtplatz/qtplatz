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

#include "mschromatogramwidget.hpp"
#include "mschromatogramform.hpp"
#include "mschromatogramtable.hpp"
#include "moltableview.hpp"
#include "moltablehelper.hpp"
#include "targetingadducts.hpp"
#if HAVE_RDKit
# include <adchem/drawing.hpp>
# include <adchem/mol.hpp>
#endif
#include <adcontrols/processmethod.hpp>
#include <adcontrols/mschromatogrammethod.hpp>
#include <adcontrols/moltable.hpp>
#include <adportable/debug.hpp>
#include <adportable/is_type.hpp>
#include <boost/exception/all.hpp>
#include <QSplitter>
#include <QStandardItemModel>
#include <QBoxLayout>
#include <QMenu>
#include <boost/json.hpp>
#include <fstream>

namespace adwidgets {
    class MSChromatogramWidget::impl {
    public:
        impl() {}
    };

}

using namespace adwidgets;

MSChromatogramWidget::MSChromatogramWidget(QWidget *parent) : QWidget(parent)
                                                            , impl_( std::make_unique< impl >() )
{
    if ( QVBoxLayout * layout = new QVBoxLayout( this ) ) {

        layout->setContentsMargins( {} );
        layout->setSpacing(2);

        if ( QSplitter * splitter = new QSplitter ) {
            splitter->addWidget( ( new MSChromatogramForm ) );
            auto table = new MSChromatogramTable(); //new MolTableView();
            // setup( table );
            splitter->addWidget( table );
            splitter->setStretchFactor( 0, 0 );
            splitter->setStretchFactor( 1, 3 );
            splitter->setOrientation ( Qt::Horizontal );
            layout->addWidget( splitter );
        }
    }

    if ( auto form = findChild< MSChromatogramForm * >() ) {
        if ( auto table = findChild< MSChromatogramTable * >() ) {
            connect( form, &MSChromatogramForm::onEnableLockMass
                     , [table]( bool enable ) { table->setColumnHidden( col_msref{}, !enable ); } );
            connect( form, &MSChromatogramForm::onAutoTargetingEnabled, table, &MSChromatogramTable::handleAutoTagetingEnabled );
        }
        connect( form, &MSChromatogramForm::triggerProcess, [this] { run(); } );

        if ( auto table = findChild< MSChromatogramTable *>() ) {
            connect( form, &MSChromatogramForm::polarityToggled, table, &MSChromatogramTable::handlePolarity );
        }
    }
}

MSChromatogramWidget::~MSChromatogramWidget()
{
}

QWidget *
MSChromatogramWidget::create( QWidget * parent )
{
    return new MSChromatogramWidget( parent );
}

void
MSChromatogramWidget::OnCreate( const adportable::Configuration& )
{
}

void
MSChromatogramWidget::OnInitialUpdate()
{
    if ( auto form = findChild< MSChromatogramForm * >() )
        form->OnInitialUpdate();

    if ( auto table = findChild< MSChromatogramTable * >() )
        table->onInitialUpdate();
}

void
MSChromatogramWidget::onUpdate( boost::any&& )
{
}

void
MSChromatogramWidget::OnFinalClose()
{
}

bool
MSChromatogramWidget::getContents( boost::any& a ) const
{
    if ( auto pm = boost::any_cast<adcontrols::ProcessMethod *>( a ) ) {
        adcontrols::MSChromatogramMethod m;
        getContents( m );
        (*pm) *= m;
        return true;
    }
    return false;
}

bool
MSChromatogramWidget::setContents( boost::any&& a )
{
    if ( adportable::a_type< adcontrols::ProcessMethod >::is_a( a ) ) {

        const adcontrols::ProcessMethod& pm = boost::any_cast<adcontrols::ProcessMethod&>( a );

        if ( auto m = pm.find< adcontrols::MSChromatogramMethod >() ) {
            setContents( *m );
            return true;
        }
    }
    return false;
}

void
MSChromatogramWidget::setContents( const adcontrols::MSChromatogramMethod& m )
{
    if ( auto form = findChild< MSChromatogramForm * >() ) {
        form->setContents( m );
    }

    if ( auto table = findChild< MSChromatogramTable * >() ) {
        table->setColumnHidden( col_msref{}, !m.lockmass() );
        table->setValue( m.molecules() );
        table->handleAutoTagetingEnabled( m.enableAutoTargeting() );
    }
}

bool
MSChromatogramWidget::getContents( adcontrols::MSChromatogramMethod& m ) const
{
    if ( auto form = findChild< MSChromatogramForm * >() )
        form->getContents( m );
    if ( auto table = findChild< MSChromatogramTable * >() )
        table->getContents( m.molecules() );
    return true;
}

void
MSChromatogramWidget::handleContextMenu( QMenu& menu, const QPoint& pt )
{
    menu.addAction( "Run generate chromatograms", this, SLOT( run() ) );
}

void
MSChromatogramWidget::run()
{
    emit triggerProcess( "MSChromatogramWidget" );
}
