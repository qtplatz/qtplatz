/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
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

#include "isotopeform.hpp"
#include "ui_isotopeform.h"
#include "molwidget.hpp"
#include "isotopedelegate.hpp"
#include "standarditemhelper.hpp"
#include <QStandardItemModel>
#include <adportable/configuration.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/isotopemethod.hpp>
#include <qtwrapper/qstring.hpp>
#include <boost/any.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/bind.hpp>

using namespace qtwidgets;

IsotopeForm::IsotopeForm(QWidget *parent) :  QWidget(parent)
                                          ,  ui(new Ui::IsotopeForm)
										  , pMethod_( new adcontrols::IsotopeMethod ) 
										  , pModel_( new QStandardItemModel )
										  , pDelegate_( new IsotopeDelegate )
{
    ui->setupUi(this);
    ui->treeView->setModel( pModel_.get() );
}

IsotopeForm::~IsotopeForm()
{
    delete ui;
}

void
IsotopeForm::OnCreate( const adportable::Configuration& config )
{
	using adcontrols::CTable;
	(void)config;
	//config_ = config;
	connect( ui->molwidget, SIGNAL( molChanged( QString ) ), this, SLOT( onMolChanged( QString ) ) );
	connect( ui->treeView, SIGNAL( activated( const QModelIndex& ) ), this, SLOT( onActivated( const QModelIndex& ) ) );
	connect( ui->treeView, SIGNAL( clicked( const QModelIndex& ) ), this, SLOT( onActivated( const QModelIndex& ) ) );
}

void
IsotopeForm::OnInitialUpdate()
{
    QStandardItemModel& model = *pModel_;
    
    QStandardItem * rootNode = model.invisibleRootItem();
    ui->treeView->setItemDelegate( pDelegate_.get() );

    rootNode->setColumnCount(4);
	model.setHeaderData( 0, Qt::Horizontal, "Structure" );
    model.setHeaderData( 1, Qt::Horizontal, "Formula" );
    model.setHeaderData( 2, Qt::Horizontal, "Exact mass" );

	ui->treeView->setColumnWidth( 0, 100 );
	ui->treeView->setColumnWidth( 1, 80 );
	ui->treeView->setColumnWidth( 2, 80 );
	ui->treeView->setColumnHidden( 3, true );  // CTable
	//----
	//StandardItemHelper::appendRow( rootNode, "Formula", "" );
	//StandardItemHelper::appendRow( rootNode, "Mass Tolerance[Da]", "yy" );
	//StandardItemHelper::appendRow( rootNode, "Minimum RA[%]",      "zz" );

    //--------------
	//ui->treeView->expand( scanType->index() );
}

void
IsotopeForm::OnFinalClose()
{
}

void
IsotopeForm::getLifeCycle( adplugin::LifeCycle *& p )
{
    p = this;
}

bool
IsotopeForm::getContents( boost::any& any ) const
{
	if ( any.type() != typeid( adcontrols::ProcessMethod* ) )
		return false;
	adcontrols::ProcessMethod* pm = boost::any_cast< adcontrols::ProcessMethod* >( any );
    pm->appendMethod( *pMethod_ );
    return true;
}

bool
IsotopeForm::setContents( boost::any& a )
{
    if ( a.type() != typeid( adcontrols::ProcessMethod ) )
        return false;
    adcontrols::ProcessMethod& pm = boost::any_cast< adcontrols::ProcessMethod& >(a);
    const adcontrols::IsotopeMethod* p = pm.find< adcontrols::IsotopeMethod >();
    if ( ! p )
        return false;
    *pMethod_ = *p;
    // update_data( *p );
    return true;
}


void
IsotopeForm::getContents( adcontrols::ProcessMethod& pm )
{
	(void)pm;
	//update_data();
	//pm.appendMethod< adcontrols::CentroidMethod >( *pMethod_ );
}

void
IsotopeForm::onMolChanged( QString key )
{
	using adcontrols::CTable;
	using adcontrols::ChemicalFormula;

    QStandardItemModel& model = *pModel_;
#if 0
	CTable ctab;
	if ( ui->molwidget->getCTable( key, ctab ) ) {
		ctabs_.push_back( std::make_pair( key, ctab ) );

		boost::filesystem::path path( qtwrapper::wstring::copy( key ) );
        QString structure = qtwrapper::qstring::copy( path.leaf().wstring() );
		std::wstring formula = ChemicalFormula::getFormula( ctab );
		std::wstring stdformula = ChemicalFormula().standardFormula( formula );
		double m = ChemicalFormula().getMonoIsotopicMass( stdformula );

		int row = model.rowCount();
		int col = 0;
		model.insertRows( row, 1 );
		model.setData( model.index( row, col++ ), structure );
		model.setData( model.index( row, col++ ), qtwrapper::qstring::copy( stdformula ) );
		model.setData( model.index( row, col++ ), m );
		model.setData( model.index( row, col++ ), key ); // hidden
	}
#endif
}

void
IsotopeForm::onCurrentChanged( const QModelIndex& index, const QModelIndex& prev )
{
	(void)prev;
	int row = index.row();
	QVariant v = index.model()->data( index.model()->index( row, 3 ), Qt::EditRole );
	if ( ! v.isNull() ) {
		QString key = v.toString();
	}
}

void
IsotopeForm::onActivated( const QModelIndex& index )
{
	using adcontrols::CTable;
	using adcontrols::IsotopeMethod;

	int row = index.row();
	QVariant v = index.model()->data( index.model()->index( row, 3 ), Qt::EditRole );
	if ( ! v.isNull() ) {
		QString key = v.toString();
		std::vector< std::pair<QString, CTable> >::iterator it
			= std::find_if( ctabs_.begin(), ctabs_.end()
			, boost::bind( &std::pair<QString, CTable>::first, _1 ) == key );

		if ( it != ctabs_.end() ) {
			ui->molwidget->draw( it->second );

			// forward to upper application (dataproc plugin)
			pMethod_->clear();
			bool positivePolarity = ui->radioPositive->isChecked();
			std::wstring desc = qtwrapper::wstring::copy( index.model()->data( index.model()->index( row, 0 ) ).toString() );
			std::wstring stdformula = qtwrapper::wstring::copy( index.model()->data( index.model()->index( row, 1 ) ).toString() );
			std::wstring adduct = qtwrapper::wstring::copy( ui->lineEdit->text() );
			pMethod_->addFormula( IsotopeMethod::Formula( desc, stdformula, adduct, 1, 1.0, positivePolarity ) );

			adcontrols::ProcessMethod m;
			m.appendMethod< IsotopeMethod >( *pMethod_ );
			emit apply( m );
		}
	}
}
