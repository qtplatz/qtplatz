/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@scienceliaison.com
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

#include "targetform.hpp"
#include "ui_targetform.h"
#include "adductsdelegate.hpp"
#include "formulaedelegate.hpp"
#include "standarditemhelper.hpp"
#include <adcontrols/targetingmethod.hpp>
#include <adcontrols/processmethod.hpp>
#include <adportable/debug.hpp>
#include <qtwrapper/spin_t.hpp>
#include <qtwrapper/qstring.hpp>
#include <QStandardItemModel>
#include <QTreeView>
#include <QMenu>

namespace qtwidgets { 
	enum { row_positive, row_negative };
	enum { c_adduct_or_loss = 0 };
    enum { c_adduct_checkbox = 0, c_adduct_num_columns };
	enum { c_formula = 0 };
    enum { c_formula_checkbox = 0, c_formula_num_columns };
}

using namespace qtwidgets;

TargetForm::TargetForm(QWidget *parent) :  QWidget(parent)
                                        , ui(new Ui::TargetForm)
                                        , adductsModel_( new QStandardItemModel )
                                        , formulaeModel_( new QStandardItemModel )
                                        , adductsDelegate_( new AdductsDelegate )
                                        , formulaeDelegate_( new FormulaeDelegate )
                                        , method_( new adcontrols::TargetingMethod )
{
	using qtwrapper::spin_t;

    ui->setupUi(this);
	spin_t< QSpinBox >::init( ui->spinResolvingPower, 100, 1000000 );
	spin_t< QDoubleSpinBox >::init( ui->spinPeakWidth, 0.0001, 1.0 );
	spin_t< QSpinBox >::init( ui->spinChargeStateMin, 1, 1000 );
	spin_t< QSpinBox >::init( ui->spinChargeStateMax, 1, 1000 );
	spin_t< QDoubleSpinBox >::init( ui->spinLowMassLimit, 1.0, 10000.0 );
	spin_t< QDoubleSpinBox >::init( ui->spinHighMassLimit, 1.0, 10000.0 );

	ui->adductsTree->setModel( adductsModel_.get() );
	ui->adductsTree->setItemDelegate( adductsDelegate_.get() );
	ui->adductsTree->setContextMenuPolicy( Qt::CustomContextMenu );

	ui->formulaeTree->setModel( formulaeModel_.get() );
	ui->formulaeTree->setItemDelegate( formulaeDelegate_.get() );
	ui->formulaeTree->setContextMenuPolicy( Qt::CustomContextMenu );
	connect( ui->adductsTree, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( showContextMenu1( const QPoint& ) ) );
	connect( ui->formulaeTree, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( showContextMenu2( const QPoint& ) ) );
}

TargetForm::~TargetForm()
{
    delete ui;
}

void
TargetForm::OnCreate( const adportable::Configuration& )
{
}

void
TargetForm::OnInitialUpdate()
{
    init_adducts( *ui->adductsTree, *adductsModel_ );
    init_formulae( *ui->formulaeTree, *formulaeModel_ );

	set_method( *method_ );
}

void
TargetForm::OnFinalClose()
{
}

bool
TargetForm::getContents( boost::any& any ) const
{
	if ( any.type() != typeid ( adcontrols::ProcessMethod* ) )
        return false;
    adcontrols::ProcessMethod* pm = boost::any_cast< adcontrols::ProcessMethod* >( any );
	get_method( *method_ );
    pm->appendMethod< adcontrols::TargetingMethod >( *method_ );
    return true;
}

bool
TargetForm::setContents( boost::any& any )
{
	if ( any.type() != typeid ( adcontrols::ProcessMethod ) )
        return false;

    const adcontrols::ProcessMethod& pm = boost::any_cast< adcontrols::ProcessMethod& >( any );
    const adcontrols::TargetingMethod * t = pm.find< adcontrols::TargetingMethod >();
    if ( ! t )
        return false;
    *method_ = *t;
    set_method( *method_ );
    return true;
}

void
TargetForm::getLifeCycle( adplugin::LifeCycle *& p )
{
    p = this;
}

void
TargetForm::getContents( adcontrols::ProcessMethod& pm )
{
    get_method( *method_ );
    pm.appendMethod< adcontrols::TargetingMethod >( *method_ );
}

void
TargetForm::update()
{
	// update UI after method modified in outside
}

//
// -- private methods
void
TargetForm::get_method( adcontrols::TargetingMethod& method ) const
{
	method.resolving_power( ui->spinResolvingPower->value() );
	method.peak_width( ui->spinPeakWidth->value() );
    method.chargeState( ui->spinChargeStateMin->value(), ui->spinChargeStateMax->value() );

	method.lowMassLimit( ui->spinLowMassLimit->value() );
	method.highMassLimit( ui->spinHighMassLimit->value() );
    method.is_use_resolving_power( ui->radioButtonRP->isChecked() );

    method.isLowMassLimitEnabled( ui->checkBoxLML->checkState() == Qt::Checked );
    method.isHighMassLimitEnabled( ui->checkBoxHML->checkState() == Qt::Checked );

    get_adducts( *ui->adductsTree, *adductsModel_, adductsModel_->index( 0, 0 ), method, true );
    get_adducts( *ui->adductsTree, *adductsModel_, adductsModel_->index( 1, 0 ), method, false );
	get_formulae( *ui->formulaeTree, *formulaeModel_, method );
}

void
TargetForm::set_method( const adcontrols::TargetingMethod& method )
{
	ui->spinResolvingPower->setValue( method.resolving_power() );
	ui->spinPeakWidth->setValue( method.peak_width() );
    std::pair< unsigned int, unsigned int > chargeState = method.chargeState();
	ui->spinChargeStateMin->setValue( chargeState.first );
	ui->spinChargeStateMax->setValue( chargeState.second );
	ui->spinLowMassLimit->setValue( method.lowMassLimit() );
	ui->spinHighMassLimit->setValue( method.highMassLimit() );

    QRadioButton * radio = method.is_use_resolving_power() ? ui->radioButtonRP : ui->radioButtonPW;
    radio->setChecked( true );

    std::pair< bool, bool > massLimits = method.isMassLimitsEnabled();
    ui->checkBoxLML->setCheckState( massLimits.first ? Qt::Checked : Qt::Unchecked );
    ui->checkBoxHML->setCheckState( massLimits.second ? Qt::Checked : Qt::Unchecked );

    update_adducts( *ui->adductsTree, *adductsModel_, adductsModel_->index( 0, 0 ), method, true );
    update_adducts( *ui->adductsTree, *adductsModel_, adductsModel_->index( 1, 0 ), method, false );
	update_formulae( *ui->formulaeTree, *formulaeModel_, method );
}

void
TargetForm::init_adducts( QTreeView&, QStandardItemModel& model )
{
	model.setColumnCount( c_adduct_num_columns );
    model.setHeaderData( c_adduct_or_loss, Qt::Horizontal, "Adducts/Losses" );
    if ( c_adduct_or_loss != c_adduct_checkbox )
        model.setHeaderData( c_adduct_checkbox, Qt::Horizontal, "" );

    QStandardItem * rootNode = model.invisibleRootItem();
	rootNode->setColumnCount( c_adduct_num_columns );
	
	do {
	  QStandardItem * pos = new QStandardItem( "Positive ion mode" );
	  pos->setEditable( false );
	  rootNode->appendRow( pos );
	} while(0);

	do {
	  QStandardItem * neg = new QStandardItem( "Negative ion mode" );
	  neg->setEditable( false );
	  rootNode->appendRow( neg );
	} while(0);
}

void
TargetForm::init_formulae( QTreeView& tree, QStandardItemModel& model )
{
    QStandardItem * rootNode = model.invisibleRootItem();
	model.setColumnCount( c_formula_num_columns );
    rootNode->setColumnCount( c_formula_num_columns );
    tree.setColumnWidth( c_formula, 120 );
    tree.setColumnWidth( c_formula_checkbox, 16 );

	model.setHeaderData( c_formula, Qt::Horizontal, "Formulae" );
    if ( c_formula != c_formula_checkbox )
        model.setHeaderData( c_formula_checkbox, Qt::Horizontal, "" );
}

void
TargetForm::update_adducts( QTreeView& tree
							, QStandardItemModel& model
                            , const QModelIndex& index
                            , const adcontrols::TargetingMethod& method
                            , bool positiveMode )
{
	QStandardItem * item = model.itemFromIndex( index );
	auto vec = method.adducts( positiveMode );
	
	item->removeRows( 0, item->rowCount() );
	item->setRowCount( vec.empty() ? 1 : vec.size() );
	item->setColumnCount( c_adduct_num_columns );

	int row = 0;
	for ( auto it: vec ) {
		model.setData( model.index( row, c_adduct_or_loss, index ), qtwrapper::qstring::copy( it.first ), Qt::EditRole );
		enable_checkbox( model, model.index( row, c_adduct_checkbox, index ), it.second );
	    ++row;
	}
	tree.expandAll();
}

void
TargetForm::get_adducts( QTreeView&
                         , QStandardItemModel& model
                         , const QModelIndex& index
                         , adcontrols::TargetingMethod& method, bool positiveMode )
{
	QStandardItem * item = model.itemFromIndex( index );
    size_t nRows = item->rowCount();
	
	auto &vec = method.adducts( positiveMode );
	vec.clear();

	for ( size_t row = 0; row < nRows; ++row ) {
		std::wstring adduct_or_loss
            = qtwrapper::wstring::copy( model.index( row, c_adduct_or_loss, index ).data( Qt::EditRole ).toString() );
		bool enable = ( model.index( row, c_adduct_checkbox, index ).data( Qt::CheckStateRole ) ) == Qt::Checked;
		if ( ! adduct_or_loss.empty() )
			vec.push_back( adcontrols::TargetingMethod::value_type( adduct_or_loss, enable ) );
	}
}

void
TargetForm::update_formulae( QTreeView&, QStandardItemModel& model
                            , const adcontrols::TargetingMethod& method )
{
    auto vec = method.formulae();
	model.removeRows( 0, model.rowCount() );
	model.setRowCount( vec.size() + 1 ); // keep empty line on the bottom

	int row = 0;
	for ( auto it: vec ) {
		model.setData( model.index( row, c_formula ), qtwrapper::qstring::copy( it.first ), Qt::EditRole );
        enable_checkbox( model, model.index( row, c_formula_checkbox ), it.second );
	    ++row;
	}
	model.setData( model.index( row, c_formula ), QString( "-- edit here --"), Qt::EditRole );
	enable_checkbox( model, model.index( row, c_formula_checkbox ), false );
}

void
TargetForm::get_formulae( QTreeView&, QStandardItemModel& model, adcontrols::TargetingMethod& method )
{
    size_t nRows = model.rowCount();
    auto vec = method.formulae();
    vec.clear();

    for ( size_t row = 0; row < nRows; ++row ) {
        std::wstring formula = qtwrapper::wstring::copy( model.index( row, c_formula ).data( Qt::EditRole ).toString() );
        bool enable = is_checked( model.index( row, c_formula_checkbox ) );
        if ( ! formula.empty() )
            vec.push_back( adcontrols::TargetingMethod::value_type( formula, enable ) );
    }
}

void
TargetForm::showContextMenu1( const QPoint& pt )
{
    QMenu menu;
    QTreeView& tree = *ui->adductsTree;
    QModelIndex index = tree.currentIndex();
    QModelIndex parent = index.parent();
    int nRootRow = parent.isValid() ? parent.row() : index.row();
    
    if ( tree.indexAt( pt ).isValid() ) {
        if ( nRootRow == 0 )
            menu.addAction( "Add adduct for positive ion mode", this, SLOT( addAdductOrLoss() ) );
        else
            menu.addAction( "Add adduct or loss for negative ion mode", this, SLOT( addAdductOrLoss() ) );
        if ( parent.isValid() ) {
            QString adducts = index.model()->index( index.row(), c_adduct_or_loss, parent ).data( Qt::EditRole ).toString();
            menu.addAction( "Delete \"" + adducts + "\"", this, SLOT( delAddcutOrLoss() ) );
        }
        menu.exec( tree.mapToGlobal( pt ) );
    }
}

void
TargetForm::showContextMenu2( const QPoint& pt )
{
    QMenu menu;
    QTreeView& tree = *ui->formulaeTree;

    menu.addAction( "Add new line", this, SLOT( addFormula() ) );
    if ( tree.indexAt( pt ).isValid() ) {
        QModelIndex index = tree.currentIndex();
        QString formula = index.model()->index( index.row(), c_formula ).data( Qt::EditRole ).toString();
        menu.addAction( "Delete \"" + formula + "\"", this, SLOT( delFormula() ) );
    }
    menu.exec( tree.mapToGlobal( pt ) );
}

void
TargetForm::addAdductOrLoss()
{
    QStandardItemModel& model = *adductsModel_;
    QModelIndex index = ui->adductsTree->currentIndex();

    if ( index.parent().isValid() ) {
        model.insertRow( index.row() + 1, index.parent() );
		enable_checkbox( model, model.index( index.row() + 1, c_adduct_checkbox, index.parent() ), false );
    } else {
        QStandardItem * item = model.itemFromIndex( index );
        model.insertRow( item->rowCount(), index );
        enable_checkbox( model, model.index( item->rowCount() - 1, c_adduct_checkbox, index ), false );
    }
}

void
TargetForm::delAdductOrLoss()
{
    QStandardItemModel& model = *adductsModel_;
    QModelIndex index = ui->adductsTree->currentIndex();

    if ( index.parent().isValid() )
        model.removeRow( index.row(), index.parent() );
}

void
TargetForm::addFormula()
{
    QModelIndex index = ui->formulaeTree->currentIndex();
    QStandardItemModel& model = *formulaeModel_;

    if ( index.isValid() ) {
        model.insertRow( index.row() + 1 );
        enable_checkbox( model, model.index( index.row() + 1, c_formula_checkbox ), false );
    } else {
        model.insertRow( model.rowCount() );
        enable_checkbox( model, model.index( model.rowCount() - 1, c_formula_checkbox ), false );        
    }
}

void
TargetForm::delFormula()
{
    QStandardItemModel& model = *formulaeModel_;
    QModelIndex index = ui->formulaeTree->currentIndex();
    if ( index.isValid() )
        model.removeRow( index.row() );
}

// static
void
TargetForm::enable_checkbox( QStandardItemModel& model, QModelIndex& index, bool isChecked, bool editable )
{
    if ( index.isValid() ) {
        QStandardItem * chk = model.itemFromIndex( index );
        if ( chk ) {
			chk->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
			chk->setEditable( editable );
			model.setData( index, isChecked ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
        }
    }
}

bool
TargetForm::is_checked( const QModelIndex& index )
{
    return index.data( Qt::CheckStateRole ) == Qt::Checked;
}

