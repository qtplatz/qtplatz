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

#include "mscalibrationform.hpp"
#include "ui_mscalibrationform.h"
#include "mscalibratedelegate.hpp"

#include <adcontrols/mscalibratemethod.hpp>
#include <adcontrols/msreferencedefns.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/tableofelements.hpp>
#include <adcontrols/element.hpp>
#include <adportable/configuration.hpp>

#include <QStandardItemModel>
#include "standarditemhelper.hpp"
#include <boost/format.hpp>
#include <qtwrapper/qstring.hpp>
#include <qdebug.h>

using namespace qtwidgets;

MSCalibrationForm::MSCalibrationForm(QWidget *parent) :
    QWidget(parent)
    , ui(new Ui::MSCalibrationForm)
    , pModel_( new QStandardItemModel )
    , pConfig_( new adportable::Configuration )
    , pMethod_( new adcontrols::MSCalibrateMethod ) 
    , pDelegate_( new MSCalibrateDelegate )
{
    ui->setupUi(this);
    ui->treeView->setModel( pModel_.get() );
    ui->treeView->setItemDelegate( pDelegate_.get() );

    ui->spinPolynomials->setMinimum( 1 );
    ui->spinPolynomials->setMaximum( 9 );
    ui->spinHighMass->setMinimum( 1.0 );
    ui->spinHighMass->setMaximum( 10000.0 );
    ui->spinLowMass->setMinimum( 1.0 );
    ui->spinLowMass->setMaximum( 10000.0 );
    ui->spinMassTolerance->setMinimum( 0.0 );
    ui->spinMassTolerance->setMaximum( 1000.0 );
    ui->spinMinimumRA->setMinimum( 0.0 );
    ui->spinMinimumRA->setMaximum( 99.9 );

    connect( pDelegate_.get(), SIGNAL( signalMSReferencesChanged( QModelIndex ) ), this, SLOT( handleMSReferencesChanged( QModelIndex ) ) );
}

MSCalibrationForm::~MSCalibrationForm()
{
    delete ui;
}

void
MSCalibrationForm::OnCreate( const adportable::Configuration& config )
{
    *pConfig_ = config;
}

void
MSCalibrationForm::OnInitialUpdate()
{
    QStandardItemModel& model = *pModel_;
    adcontrols::MSCalibrateMethod& method = *pMethod_;

    QStandardItem * rootNode = model.invisibleRootItem();
    ui->treeView->setItemDelegate( pDelegate_.get() );

    rootNode->setColumnCount(3);
    model.setHeaderData( 0, Qt::Horizontal, "MSCaribrate" );
    model.setHeaderData( 1, Qt::Horizontal, "exact mass" );
    model.setHeaderData( 2, Qt::Horizontal, "enable" );

    ui->spinPolynomials->setValue( method.polynomialDegree() );
    ui->spinMassTolerance->setValue( method.massToleranceDa() );
    ui->spinMinimumRA->setValue( method.minimumRAPercent() );
    ui->spinLowMass->setValue( method.lowMass() );
    ui->spinHighMass->setValue( method.highMass() );
//----
    //StandardItemHelper::appendRow( rootNode, "Polynomial[degree]", method.polynomialDegree() );
    //StandardItemHelper::appendRow( rootNode, "Mass Tolerance[Da]", method.massToleranceDa() );
    //StandardItemHelper::appendRow( rootNode, "Minimum RA[%]",      method.minimumRAPercent() );
    //StandardItemHelper::appendRow( rootNode, "Low Mass[Da]",       method.lowMass() );
    //StandardItemHelper::appendRow( rootNode, "High Mass[Da]",      method.highMass() );

    //------ create Xe reference -------
    adcontrols::MSReferences Xe;
    Xe.name( L"Xe-EI" );
    do {
        adcontrols::MSReferences& ref = Xe;
        const adcontrols::Element& element = adcontrols::TableOfElements::instance()->findElement( L"Xe" );
        for ( adcontrols::Element::vector_type::const_iterator it = element.begin(); it != element.end(); ++it ) {
            std::wstring formula = ( boost::wformat( L"%1%Xe" ) % int( it->mass_ + 0.5 ) ).str();
            bool enable = it->abundance_ > 0.01;
            ref << adcontrols::MSReference( formula, true, L"", enable, it->mass_ );
        }
    } while(0);

    //------ create Ar reference -------
    adcontrols::MSReferences Ar;
    Ar.name( L"Ar-EI" );
    do {
        adcontrols::MSReferences& ref = Ar;
        const adcontrols::Element& element = adcontrols::TableOfElements::instance()->findElement( L"Ar" );
        for ( adcontrols::Element::vector_type::const_iterator it = element.begin(); it != element.end(); ++it ) {
            std::wstring formula = ( boost::wformat( L"%1%Ar" ) % int( it->mass_ + 0.5 ) ).str();
            bool enable = it->abundance_ > 0.01;
            ref << adcontrols::MSReference( formula, true, L"", enable, it->mass_ );
        }
    } while(0);

    // ---------------------------------
    //------ create PFTBA < tris(Perfluorobutyl)amine > reference -------
    adcontrols::MSReferences PFTBA;
    PFTBA.name( L"PFTBA-EI" );
    do {
        adcontrols::MSReferences& ref = PFTBA;
        ref << adcontrols::MSReference( L"CF3",     true,  L"", true );
        ref << adcontrols::MSReference( L"C2F4",    true,  L"", false );
        ref << adcontrols::MSReference( L"C2F5",    true,  L"", false );
        ref << adcontrols::MSReference( L"C3F5",    true,  L"", true );
        ref << adcontrols::MSReference( L"C4F9",    true,  L"", true );
        ref << adcontrols::MSReference( L"C5F10N",  true,  L"", true );
        ref << adcontrols::MSReference( L"C6F12N",  true,  L"", false );
        ref << adcontrols::MSReference( L"C7F12N",  true,  L"", false );
        ref << adcontrols::MSReference( L"C8F14N",  true,  L"", false );
        ref << adcontrols::MSReference( L"C8F16N",  true,  L"", false );
        ref << adcontrols::MSReference( L"C9F16N",  true,  L"", false );
        ref << adcontrols::MSReference( L"C9F18N",  true,  L"", false );
        ref << adcontrols::MSReference( L"C9F20N",  true,  L"", true );
        ref << adcontrols::MSReference( L"C12F22N", true,  L"", false );
        ref << adcontrols::MSReference( L"C12F24N", true,  L"", false );
    } while(0);
    // ---------------------------------

    pDelegate_->refs_[ Ar.name() ] = Ar;
    pDelegate_->refs_[ Xe.name() ] = Xe;
    pDelegate_->refs_[ PFTBA.name() ] = PFTBA;

    pMethod_->references( PFTBA );  // set as default calibration reference

    QStandardItem * refItem = 
        StandardItemHelper::appendRow( rootNode, "Mass References", qVariantFromValue( MSCalibrateDelegate::MSReferences( pMethod_->references().name() ) ) );

    OnMSReferencesUpdated( refItem->index() );

    ui->treeView->expand( refItem->index() );
    ui->treeView->setColumnWidth( 0, 200 );
}

void
MSCalibrationForm::OnFinalClose()
{
}

bool
MSCalibrationForm::getContents( boost::any& ) const
{
    return false;
}

bool
MSCalibrationForm::setContents( boost::any& )
{
    return false;
}

void
MSCalibrationForm::getContents( adcontrols::ProcessMethod& pm )
{
    QStandardItemModel& model = *pModel_;
    QStandardItem * root = model.invisibleRootItem();
    (void)root;

    pMethod_->polynomialDegree( ui->spinPolynomials->value() );
    pMethod_->massToleranceDa( ui->spinMassTolerance->value() );
    pMethod_->minimumRAPercent( ui->spinMinimumRA->value() );
    pMethod_->lowMass( ui->spinLowMass->value() );
    pMethod_->highMass( ui->spinHighMass->value() );
    /*
    QVariant v = model.index( 0, 1, root->index() ).data( Qt::EditRole );
    pMethod_->polynomialDegree( v.toInt() );
    v = model.index( 1, 1, root->index() ).data( Qt::EditRole  );
    pMethod_->massToleranceDa( v.toDouble() );
    v = model.index( 2, 1, root->index() ).data( Qt::EditRole  );
    pMethod_->minimumRAPercent( v.toDouble() );
    v = model.index( 3, 1, root->index() ).data( Qt::EditRole  );
    pMethod_->lowMass( v.toDouble() );
    v = model.index( 4, 1, root->index() ).data( Qt::EditRole  );
    pMethod_->highMass( v.toDouble() );
    */
    adcontrols::MSReferences references;
    
    // QModelIndex index_references = model.index( 5, 0, root->index() ); // QString("Refernces")
    QStandardItem * item = model.item( 0 );
    size_t nRows = item->rowCount();

    for ( size_t row = 0; row < nRows; ++row ) {
        adcontrols::MSReference reference;

        QVariant formula = model.index( row, 0, item->index() ).data( Qt::EditRole );
        QVariant mass    = model.index( row, 1, item->index() ).data( Qt::EditRole );
        QVariant enable  = model.index( row, 2, item->index() ).data( Qt::EditRole );
        reference.formula( qtwrapper::wstring( formula.toString() ) );
        reference.exactMass( mass.toDouble() );
        reference.enable( enable.toBool() );
        references << reference;
    }
    pMethod_->references( references );

    pm.appendMethod< adcontrols::MSCalibrateMethod >( *pMethod_ );
}

void
MSCalibrationForm::handleMSReferencesChanged( const QModelIndex& index )
{
    QStandardItemModel& model = *pModel_;
    std::wstring refname = model.data( index ).value< MSCalibrateDelegate::MSReferences >().methodValue();

    qDebug() << qtwrapper::qstring::copy( refname );

    const adcontrols::MSReferences& refs = pDelegate_->refs_[ refname ];
    pMethod_->references( refs );
    // "Mass Reference" | "PFBA-EI-Positive" ==> left of this index is the top of reference
    OnMSReferencesUpdated( model.index( index.row(), index.column() - 1 ) );  
}

void
MSCalibrationForm::OnMSReferencesUpdated( const QModelIndex& index )
{
    QStandardItemModel& model = *pModel_;
    QStandardItem * item = model.itemFromIndex( index );
    const adcontrols::MSReferences& refs = pMethod_->references();

    if ( item->rowCount() )
        item->removeRows( 0, item->rowCount() );

    size_t row(0);
    for ( adcontrols::MSReferences::vector_type::const_iterator it = refs.begin(); it != refs.end(); ++it, ++row ) {
        int col = 0;
        std::wstring formula = it->formula();
        if ( ! it->adduct_or_loss().empty() )
            formula += std::wstring( it->polarityPositive() ? L" + " : L" - ") + it->adduct_or_loss();

        StandardItemHelper::appendRow( item, formula, true );
        col++;
        if ( item->columnCount() < col + 1 )
            model.insertColumn( item->columnCount(), item->index() );
        model.setData( model.index( row, col, index ), it->exactMass() );

        col++;
        if ( item->columnCount() < col + 1 )
            model.insertColumn( item->columnCount(), item->index() );
        model.setData( model.index( row, col, index ), it->enable() );
    }
}

QSize
MSCalibrationForm::sizeHint() const
{
    return QSize( 300, 250 );
}

void
MSCalibrationForm::getLifeCycle( adplugin::LifeCycle *& p )
{
    p = static_cast< adplugin::LifeCycle *>(this);
}
