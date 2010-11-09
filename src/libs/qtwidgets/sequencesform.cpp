//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "sequencesform.h"
#include "ui_sequencesform.h"
#include "sequencesmodel.h"
#include <QStandardItemModel>

using namespace qtwidgets;

SequencesForm::SequencesForm(QWidget *parent) : QWidget(parent)
                                              , ui(new Ui::SequencesForm)
                                              , pModel_( new QStandardItemModel ) 
                                              // , pModel_( new qtwidgets::SequencesModel )
{
    ui->setupUi(this);
    ui->tableView->setModel( pModel_.get() );
}

SequencesForm::~SequencesForm()
{
    delete ui;
}

void
SequencesForm::OnCreate( const adportable::Configuration& config )
{
    config_ = config;
}

void
SequencesForm::OnInitialUpdate()
{
    QStandardItemModel& model = *pModel_;
    QStandardItem * rootNode = model.invisibleRootItem();

    ui->tableView->setRowHeight( 0, 10 );
    rootNode->setColumnCount(10);
    int col = 0;
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr("Sample Type") );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr("Vial") );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr("Description") );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr("Inj.Vol(uL)") );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr("Method Time(min)") );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr("Dataset Name") );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr("Control Method") );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr("Process Method") );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr("Target Formulae") );

    if ( model.rowCount() == 0 ) {
        rootNode->appendRow( new QStandardItem( "STD/QC/UNK" ) );
    }
}

void
SequencesForm::OnFinalClose()
{
}
