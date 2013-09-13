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
#include <adportable/is_type.hpp>
#include <QStandardItemModel>
#include "standarditemhelper.hpp"
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <qtwrapper/qstring.hpp>
#include <qdebug.h>
#include <QMenu>

using namespace qtwidgets2;

MSCalibrationForm::MSCalibrationForm(QWidget *parent) :
    QWidget(parent)
    , ui(new Ui::MSCalibrationForm)
    , pModel_( new QStandardItemModel )
    , pConfig_( new adportable::Configuration )
    , pMethod_( new adcontrols::MSCalibrateMethod ) 
    , pDelegate_( new MSCalibrateDelegate )
{
    ui->setupUi(this);

    ui->tableView->setContextMenuPolicy( Qt::CustomContextMenu );

    ui->tableView->setModel( pModel_.get() );
    ui->tableView->setItemDelegate( pDelegate_.get() );

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

    connect( ui->tableView, SIGNAL( selectedRowsDeleted() ), this, SLOT( handle_selected_rows_deleted() ) );
    connect( pDelegate_.get(), SIGNAL( valueChanged( const QModelIndex& ) ), this, SLOT( handle_value_changed( const QModelIndex& ) ) );
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
	QTableView& table = *ui->tableView;

    QStandardItem * rootNode = model.invisibleRootItem();
    table.setItemDelegate( pDelegate_.get() );

    rootNode->setColumnCount( c_num_columns );
    model.setHeaderData( c_formula,     Qt::Horizontal, "Chemical formula" );
    model.setHeaderData( c_exact_mass,  Qt::Horizontal, "exact mass" );
    model.setHeaderData( c_enable,      Qt::Horizontal, "enable" );
    model.setHeaderData( c_description, Qt::Horizontal, "Description" );
	model.setHeaderData( c_charge,      Qt::Horizontal, "charge" );

    setCalibrateMethod( method );

    table.setColumnWidth( 0, 200 );
    table.setColumnWidth( c_charge, 80 );
	table.verticalHeader()->setDefaultSectionSize( 18 );

	table.setSortingEnabled(true);

	ui->comboBoxMaterials->addItem( "Ar", "Ar" );
	ui->comboBoxMaterials->addItem( "Xe", "Xe" );
	ui->comboBoxMaterials->addItem( "PFTBA", "CF3" );
	ui->comboBoxMaterials->addItem( "PEG", "H2O\tC2H4\tH" );
	ui->comboBoxMaterials->addItem( "Recerpine", "C33H40N2O9\t\tH" );
	ui->comboBoxMaterials->addItem( "Polystyrene", "H2O\tC8H8\tH" );
}

void
MSCalibrationForm::OnFinalClose()
{
}

bool
MSCalibrationForm::getContents( boost::any& a ) const
{
    if ( adportable::a_type< adcontrols::ProcessMethod >::is_pointer( a ) ) {
        adcontrols::ProcessMethod* pm = boost::any_cast< adcontrols::ProcessMethod* >( a );
        getContents( *pm );
        return true;
    }
    return false;
}

bool
MSCalibrationForm::setContents( boost::any& a )
{
    if ( adportable::a_type< adcontrols::ProcessMethod >::is_a( a ) ) {
        adcontrols::ProcessMethod& pm = boost::any_cast< adcontrols::ProcessMethod& >( a );
        if ( const adcontrols::MSCalibrateMethod *p = pm.find< adcontrols::MSCalibrateMethod >() ) {
            setCalibrateMethod( *p );
            return true;
        }
    }
    return false;
}

void
MSCalibrationForm::getContents( adcontrols::ProcessMethod& pm ) const
{
	getCalibrateMethod( *pMethod_ );
    pm.appendMethod< adcontrols::MSCalibrateMethod >( *pMethod_ );
}

void
MSCalibrationForm::getCalibrateMethod( adcontrols::MSCalibrateMethod& method ) const
{
    QStandardItemModel& model = *pModel_;

    method.polynomialDegree( ui->spinPolynomials->value() );
    method.massToleranceDa( ui->spinMassTolerance->value() );
    method.minimumRAPercent( ui->spinMinimumRA->value() );
    method.lowMass( ui->spinLowMass->value() );
    method.highMass( ui->spinHighMass->value() );

    adcontrols::MSReferences references;
    
    size_t nRows = model.rowCount();

    for ( size_t row = 0; row < nRows; ++row ) {
        adcontrols::MSReference reference;

		reference.charge_count( model.index( row, c_charge ).data( Qt::EditRole ).toInt() );
		// parse formula that contains adduct or lose followed by '+' or '-' sign
        std::wstring text = model.index( row, c_formula ).data( Qt::EditRole ).toString().toStdWString();
        if ( ! parse_formula( text, reference ) )
            continue;
		reference.exact_mass( model.index( row, c_exact_mass ).data( Qt::EditRole ).toDouble() );
		reference.description( model.index( row, c_description ).data( Qt::EditRole ).toString().toStdWString() );

        QVariant enable  = model.index( row, c_enable ).data( Qt::EditRole );
        reference.enable( enable.toBool() );

        references << reference;
    }
    method.references( references );
}

void
MSCalibrationForm::setCalibrateMethod( const adcontrols::MSCalibrateMethod& method )
{
    QStandardItemModel& model = *pModel_;

    ui->spinPolynomials->setValue( method.polynomialDegree() );
    ui->spinMassTolerance->setValue( method.massToleranceDa() );
    ui->spinMinimumRA->setValue( method.minimumRAPercent() );
    ui->spinLowMass->setValue( method.lowMass() );
    ui->spinHighMass->setValue( method.highMass() );

    const adcontrols::MSReferences& references = method.references();
    int nRows = references.size();
    if ( nRows < model.rowCount() )
        model.removeRows( 0, model.rowCount() ); // make sure all clear

    model.setRowCount( nRows + 1 ); // be sure last empty line

    size_t row = 0;
    for ( auto& ref: references ) {
		std::wstring formula = ref.formula();
		if ( ! ref.adduct_or_loss().empty() )
            formula += ( boost::wformat( L"%1%%2%") % ( ref.polarityPositive() ? '+' : '-' ) % ref.adduct_or_loss() ).str();
        
		model.setData( model.index( row, c_formula ),     qtwrapper::qstring::copy( formula ) );
		model.setData( model.index( row, c_exact_mass ),  ref.exact_mass() );
		model.setData( model.index( row, c_enable ),      ref.enable() );

        QStandardItem * chk = model.itemFromIndex( model.index( row, c_enable ) );
        if ( chk ) {
            chk->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
            chk->setEditable( true );
            model.setData( model.index( row, c_enable ),  ref.enable() ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
        }
		model.setData( model.index( row, c_description ), qtwrapper::qstring::copy( ref.description() ) );
		model.setData( model.index( row, c_charge ),      ref.charge_count() );

        ++row;
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

void
MSCalibrationForm::on_addReference_pressed()
{
	QString endGroup = ui->edtEndGroup->text();
	QString repeat = ui->edtRepeatGroup->text();
	QString adduct_lose = ui->edtAdductLose->text();
	bool isAdduct = ui->comboBoxAdductLose->currentIndex() == 0;

	getCalibrateMethod( *pMethod_ );
	adcontrols::MSReferences ref = pMethod_->references();
	
	if ( ! repeat.isEmpty() ) {
		makeSeries( endGroup.toStdWString(), repeat.toStdWString(), isAdduct, adduct_lose.toStdWString(), ref );
	} else {
        if ( endGroup == "CF3" ) { // assume PFTBA
            ref << adcontrols::MSReference( L"CF3",     true,  L"", false );
            ref << adcontrols::MSReference( L"C2F4",    true,  L"", false );
            ref << adcontrols::MSReference( L"C2F5",    true,  L"", false );
            ref << adcontrols::MSReference( L"C3F5",    true,  L"", false );
            ref << adcontrols::MSReference( L"C4F9",    true,  L"", false );
            ref << adcontrols::MSReference( L"C5F10N",  true,  L"", false );
            ref << adcontrols::MSReference( L"C6F12N",  true,  L"", false );
            ref << adcontrols::MSReference( L"C7F12N",  true,  L"", false );
            ref << adcontrols::MSReference( L"C8F14N",  true,  L"", false );
            ref << adcontrols::MSReference( L"C8F16N",  true,  L"", false );
            ref << adcontrols::MSReference( L"C9F16N",  true,  L"", false );
            ref << adcontrols::MSReference( L"C9F18N",  true,  L"", false );
            ref << adcontrols::MSReference( L"C9F20N",  true,  L"", false );
            ref << adcontrols::MSReference( L"C12F22N", true,  L"", false );
            ref << adcontrols::MSReference( L"C12F24N", true,  L"", false );
        } else {
            // check if an element
            const adcontrols::Element& element = adcontrols::TableOfElements::instance()->findElement( endGroup.toStdWString() );
            if ( element.atomicNumber() >= 1 ) {
                for ( adcontrols::Element::vector_type::const_iterator it = element.begin(); it != element.end(); ++it ) {
                    std::wstring formula = ( boost::wformat( L"%1%%2%" ) % int( it->mass_ + 0.5 ) % element.symbol() ).str();
					std::wstring description = ( boost::wformat( L"%.4f" ) % it->abundance_ ).str();
                    bool enable = it->abundance_ > 0.01;
                    ref << adcontrols::MSReference( formula, true, L"", enable, it->mass_, 1, description );
                } 
            } else {
				// try formula
				ref << adcontrols::MSReference( endGroup.toStdWString(), isAdduct, adduct_lose.toStdWString() );

			}
        }
	}
    pMethod_->references( ref );
    setCalibrateMethod( *pMethod_ );
}

void
MSCalibrationForm::makeSeries( const std::wstring& endGroup
                               , const std::wstring& repeat
                               , bool isAdduct
							   , const std::wstring& adduct_lose
							   , adcontrols::MSReferences& refs )
{
    int lmass = ui->spinLowMass->value();
    int hmass = ui->spinHighMass->value();
    
    int nRepeat = 1;
    adcontrols::MSReference ref;
    do {
		std::wstring formula = ( boost::wformat( L"%1%(%2%)%3%" ) % endGroup % repeat % nRepeat++  ).str();
		ref = adcontrols::MSReference( formula, isAdduct, adduct_lose, true );
		if ( ref.exact_mass() > lmass )
			refs << ref;
    } while( ref.exact_mass() < hmass );
}

bool
MSCalibrationForm::parse_formula( const std::wstring& text, adcontrols::MSReference& ref ) const
{
    std::wstring formula, adduct_lose;
    bool isPositive( false );
    if ( parse_formula( text, formula, adduct_lose, isPositive ) ) {
        ref = adcontrols::MSReference( formula, isPositive, adduct_lose, true );
        return true;
    }
    return false;
}

bool
MSCalibrationForm::parse_formula( const std::wstring& text, std::wstring& formula
							, std::wstring& adduct_lose, bool& isPositive ) const
{
    adduct_lose.clear();

    std::wstring::size_type pos = text.find_first_of( L"+-" );
    if ( pos == std::wstring::npos ) {
        formula = text;
        return !formula.empty();
    } else {
		formula = text.substr( 0, pos);
		if ( text.at( pos ) == L'+' )
			isPositive = true;
		else if ( text.at( pos ) == L'-' )
			isPositive = false;
		else
			return false;
		adduct_lose = text.substr( pos + 1 );
	}
    return true;
}

void qtwidgets2::MSCalibrationForm::on_pushButton_pressed()
{
	ui->edtAdductLose->setText( "" );
	ui->edtEndGroup->setText( "" );
	ui->edtRepeatGroup->setText( "" );
}

void qtwidgets2::MSCalibrationForm::on_pushButtonAdd_pressed()
{

}

void qtwidgets2::MSCalibrationForm::on_comboBoxAdductLose_currentIndexChanged(int index)
{
	(void)index;
}

void qtwidgets2::MSCalibrationForm::on_tableView_customContextMenuRequested(const QPoint &pt)
{
    std::vector< QAction * > actions;
    QMenu menu;

    actions.push_back( menu.addAction( "Refresh" ) );
    actions.push_back( menu.addAction( "Clear" ) );
    actions.push_back( menu.addAction( "Delete line(s)" ) );

    QAction * selected = menu.exec( ui->tableView->mapToGlobal( pt ) );
    if ( selected == actions[0] ) {
        adcontrols::MSCalibrateMethod method;
        getCalibrateMethod( method );
        setCalibrateMethod( method );
    } else if ( selected == actions[1] ) {
		adcontrols::MSCalibrateMethod method;
        getCalibrateMethod( method );
        method.references( adcontrols::MSReferences() ); // empty refernces
        setCalibrateMethod( method );
    } else {
        ui->tableView->handleDeleteSelection();
    }
}


void qtwidgets2::MSCalibrationForm::on_tableView_activated(const QModelIndex &index)
{
	(void)index;
}

void
MSCalibrationForm::handle_selected_rows_deleted()
{
	if ( pModel_->rowCount() == 0 )
		pModel_->setRowCount( 1 ); // make sure at least one blank line for editing
}

void
MSCalibrationForm::handle_value_changed( const QModelIndex& index )
{
    QStandardItemModel& model = *pModel_;
	int row = index.row();
    if ( index.column() == c_formula ) {
        adcontrols::MSReference ref;
        std::wstring text = model.index( row, c_formula ).data( Qt::EditRole ).toString().toStdWString();
        if ( parse_formula( text, ref ) ) {
            model.setData( model.index( row, c_formula ),     qtwrapper::qstring::copy( ref.formula() ) );
            model.setData( model.index( row, c_exact_mass ),  ref.exact_mass() );
            model.setData( model.index( row, c_enable ),      ref.enable() );
            QStandardItem * chk = model.itemFromIndex( model.index( row, c_enable ) );
            if ( chk ) {
                chk->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
                chk->setEditable( true );
                model.setData( model.index( row, c_enable ),  ref.enable() ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
            }
            model.setData( model.index( row, c_charge ),      ref.charge_count() );
        }
    }
	if ( row == model.rowCount() - 1 )
		model.insertRow( row + 1);
}

void qtwidgets2::MSCalibrationForm::on_comboBoxMaterials_currentIndexChanged(int index)
{
	ui->edtEndGroup->setText( "" );
	ui->edtRepeatGroup->setText( "" );
	ui->edtAdductLose->setText( "" );

    std::wstring userData = ui->comboBoxMaterials->itemData( index ).toString().toStdWString();
    if ( userData.empty() )
        return;

    typedef boost::tokenizer< boost::char_separator<wchar_t>
                              , std::wstring::const_iterator
                              , std::wstring > tokenizer_t;
 
	boost::char_separator<wchar_t> separator( L"\t+-", L"", boost::keep_empty_tokens );
    tokenizer_t tokens( userData, separator );
    
    auto token = tokens.begin();
    if ( token != tokens.end() )
        ui->edtEndGroup->setText( qtwrapper::qstring::copy( *token ) );
    
	if ( ++token != tokens.end() )
        ui->edtRepeatGroup->setText( qtwrapper::qstring::copy( *token ) );

    if ( token != tokens.end() && ++token != tokens.end() )
        ui->edtAdductLose->setText( qtwrapper::qstring::copy( *token ) );
}
