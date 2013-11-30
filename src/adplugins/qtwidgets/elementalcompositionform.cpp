// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "elementalcompositionform.hpp"
#include "ui_elementalcompositionform.h"
#include "elementalcompositiondelegate.hpp"
#include "standarditemhelper.hpp"
#include <adportable/configuration.hpp>
#include <adcontrols/elementalcompositionmethod.hpp>
#include <adcontrols/processmethod.hpp>
#include <QStandardItemModel>
#include <boost/any.hpp>

using namespace qtwidgets;

ElementalCompositionForm::ElementalCompositionForm(QWidget *parent) :
    QWidget(parent)
    , ui(new Ui::ElementalCompositionForm)
    , pModel_( new QStandardItemModel )
    , pConfig_( new adportable::Configuration )
    , pDelegate_( new ElementalCompositionDelegate )
    , pMethod_( new adcontrols::ElementalCompositionMethod() )
{
    ui->setupUi(this);
    ui->treeView->setModel( pModel_.get() );
    ui->treeView->setItemDelegate( pDelegate_.get() );
}

ElementalCompositionForm::~ElementalCompositionForm()
{
    delete ui;
}

void
ElementalCompositionForm::OnCreate( const adportable::Configuration& config )
{
    *pConfig_ = config;
}

void
ElementalCompositionForm::OnInitialUpdate()
{
    QStandardItemModel& model = *pModel_;
    QStandardItem * rootNode = model.invisibleRootItem();
    

    rootNode->setColumnCount(2);
    ui->treeView->setColumnWidth( 0, 200 );

    model.setHeaderData( 0, Qt::Horizontal, "Elemental Composition" );
    
    StandardItemHelper::appendRow( rootNode, "Mass", 0.00 ); // 0
    StandardItemHelper::appendRow( rootNode, "Electron Mode", qVariantFromValue( ElementalCompositionDelegate::ElectronMode(0) ) ); // 1

    QStandardItem * tolerance = StandardItemHelper::appendRow( rootNode, "Tolerance", "mDa" ); // 2 choice of mDa/ppm

    StandardItemHelper::appendRow( tolerance, "mDa", 5.0 );
    model.setData( model.index( 2, 1, model.item( 2, 0 )->index() ), 999.0 );

    StandardItemHelper::appendRow( tolerance, "ppm", 10.0 );
    ui->treeView->expand( tolerance->index() );

    QStandardItem * dbe = StandardItemHelper::appendRow( rootNode, "Double Bound Equivalent", "-0.5..200.0" ); // range
    StandardItemHelper::appendRow( dbe, "Minimum", -0.5 );
    StandardItemHelper::appendRow( dbe, "Maximum", 200.0 );

    StandardItemHelper::appendRow( rootNode, "Maximum Results", 100 );
    
    QStandardItem * constraints = StandardItemHelper::appendRow( rootNode, "Composition Constraints", "C..H..N..O.."); // list
    ui->treeView->expand( constraints->index() );
    
    std::vector< std::string > atoms(10);
    atoms[0] = "C";
    atoms[1] = "H";
    atoms[2] = "N";
    atoms[3] = "O";
    atoms[4] = "Na";

    for ( std::vector<std::string>::iterator it = atoms.begin(); it != atoms.end(); ++it ) {
        QStandardItem * atom = StandardItemHelper::appendRow( constraints, it->c_str(), true );
        StandardItemHelper::appendRow( atom, "Minimum", 0 );
        StandardItemHelper::appendRow( atom, "Maximum", 100 );
    }
}

void
ElementalCompositionForm::OnFinalClose()
{
}

bool
ElementalCompositionForm::getContents( boost::any& any ) const
{
	if ( any.type() != typeid( adcontrols::ProcessMethod* ) )
		return false;
	adcontrols::ProcessMethod* pm = boost::any_cast< adcontrols::ProcessMethod* >( any );
    pm->appendMethod< adcontrols::ElementalCompositionMethod >( *pMethod_ );
    return true;
}

bool
ElementalCompositionForm::setContents( boost::any& a )
{
    if ( a.type() != typeid( adcontrols::ProcessMethod ) )
        return false;
    adcontrols::ProcessMethod& pm = boost::any_cast< adcontrols::ProcessMethod& >(a);
	const adcontrols::ElementalCompositionMethod* p = pm.find< adcontrols::ElementalCompositionMethod >();
    if ( ! p )
        return false;
    *pMethod_ = *p;
    // update_data( *p );
    return true;
}

void
ElementalCompositionForm::getContents( adcontrols::ProcessMethod& pm )
{
    pm.appendMethod< adcontrols::ElementalCompositionMethod >( *pMethod_ );
}

QSize
ElementalCompositionForm::sizeHint() const
{
    return QSize( 300, 250 );
}

void
ElementalCompositionForm::getLifeCycle( adplugin::LifeCycle *& p )
{
    p = static_cast< adplugin::LifeCycle *>(this);
}
