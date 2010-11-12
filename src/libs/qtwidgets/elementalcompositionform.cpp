//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "elementalcompositionform.h"
#include "ui_elementalcompositionform.h"
#include "elementalcompositiondelegate.h"
#include "standarditemhelper.h"
#include <adportable/configuration.h>
#include <QStandardItemModel>

using namespace qtwidgets;

ElementalCompositionForm::ElementalCompositionForm(QWidget *parent) :
    QWidget(parent)
    , ui(new Ui::ElementalCompositionForm)
    , pModel_( new QStandardItemModel )
    , pConfig_( new adportable::Configuration )
    , pDelegate_( new ElementalCompositionDelegate )
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
    
    StandardItemHelper::appendRow( rootNode, "Mass", 0.00 );
    StandardItemHelper::appendRow( rootNode
        , "Electron Mode", qVariantFromValue( ElementalCompositionDelegate::ElectronMode(0) ) );

    QStandardItem * tolerance = 
        StandardItemHelper::appendRow( rootNode, "Tolerance", "mDa" ); // choice of mDa/ppm
    StandardItemHelper::appendRow( tolerance, "mDa", QVariant(5.0) );
    StandardItemHelper::appendRow( tolerance, "ppm", QVariant(10.0) );
    ui->treeView->expand( tolerance->index() );

    QStandardItem * dbe = 
        StandardItemHelper::appendRow( rootNode, "Double Bound Equivalent", "-0.5..200.0" ); // range
    StandardItemHelper::appendRow( dbe, "Minimum", QVariant(-0.5) );
    StandardItemHelper::appendRow( dbe, "Maximum", QVariant(200.0) );

    StandardItemHelper::appendRow( rootNode, "Maximum Results", QVariant(100) );
    
    QStandardItem * constraints
        = StandardItemHelper::appendRow( rootNode, "Composition Constraints" ); // list
    ui->treeView->expand( constraints->index() );
    
    std::vector< std::string > atoms(20);
    atoms[0] = "C";
    atoms[1] = "H";
    atoms[2] = "N";
    atoms[3] = "O";
    atoms[4] = "Na";

    for ( std::vector<std::string>::iterator it = atoms.begin(); it != atoms.end(); ++it ) {
        QStandardItem * atom = StandardItemHelper::appendRow( constraints, it->c_str(), true );
        StandardItemHelper::appendRow( atom, "Minimum", QVariant(0) );
        StandardItemHelper::appendRow( atom, "Maximum", QVariant(100) );
    }
}

void
ElementalCompositionForm::OnFinalClose()
{
}
