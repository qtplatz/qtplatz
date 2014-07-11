/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "msreferencetable.hpp"
#include "delegatehelper.hpp"
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/mscalibratemethod.hpp>
#include <qtwrapper/font.hpp>
#include <QHeaderView>
#include <QStyledItemDelegate>
#include <QStandardItemModel>
#include <functional>

namespace adwidgets {
    namespace detail {
        namespace msreferencetable {

            enum colume_define {
                c_formula
                , c_exact_mass
                , c_enable
                , c_description
                , c_charge
                , c_num_columns
            };

            class ItemDelegate : public QStyledItemDelegate {
            public:
                void paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {

                    QStyleOptionViewItem opt(option);
                    initStyleOption( &opt, index );
                    opt.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;

                    if ( index.column() == c_formula ) {

                        std::string formula = adcontrols::ChemicalFormula::formatFormulae( index.data().toString().toStdString() );
                        DelegateHelper::render_html2( painter, opt, QString::fromStdString( formula ) );

                    } else if ( index.column() == c_exact_mass ) {

                        QStyledItemDelegate::paint( painter, opt, index );

                    } else {
                    
                        QStyledItemDelegate::paint( painter, opt, index );

                    }
                }

                void setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex& index ) const override {
                    QStyledItemDelegate::setModelData( editor, model, index );
                    if ( valueChanged_ )
                        valueChanged_( index );
                }

            public:
                void register_handler( std::function< void( const QModelIndex& ) > f ) {
                    valueChanged_ = f;
                }
            private:
                std::function< void( const QModelIndex& ) > valueChanged_;
            };

        }
    }
}

using namespace adwidgets;
using namespace adwidgets::detail::msreferencetable;

MSReferenceTable::MSReferenceTable(QWidget *parent) :  QTableView(parent)
                                                    , model_( new QStandardItemModel )
{
    setModel( model_ );
    if ( auto delegate = new ItemDelegate ) {
        delegate->register_handler( [=]( const QModelIndex& idx ){ handleValueChanged( idx ); } );
        setItemDelegate( delegate );
    }

    QFont font;
    setFont( qtwrapper::font::setFamily( font, qtwrapper::fontTableBody ) );
}

MSReferenceTable::~MSReferenceTable()
{
    delete model_;
}

void
MSReferenceTable::onInitialUpdate()
{
    QStandardItemModel& model = *model_;

    model.setColumnCount( c_num_columns );
    model.setHeaderData( c_formula,     Qt::Horizontal, "Chemical formula" );
    model.setHeaderData( c_exact_mass,  Qt::Horizontal, "exact mass" );
    model.setHeaderData( c_enable,      Qt::Horizontal, "enable" );
    model.setHeaderData( c_description, Qt::Horizontal, "Description" );
	model.setHeaderData( c_charge,      Qt::Horizontal, "charge" );

    setColumnWidth( 0, 200 );
    setColumnWidth( c_charge, 80 );
    setSortingEnabled( true );
    verticalHeader()->setDefaultSectionSize( 18 );

	// ui->comboBoxMaterials->addItem( "Ar", "Ar" );
	// ui->comboBoxMaterials->addItem( "Xe", "Xe" );
	// ui->comboBoxMaterials->addItem( "PFTBA", "PFTBA" );
	// ui->comboBoxMaterials->addItem( "PEG", "H2O\tC2H4O\tH" );
	// ui->comboBoxMaterials->addItem( "Recerpine", "C33H40N2O9\t\tH" );
	// ui->comboBoxMaterials->addItem( "Polystyrene", "H2O\tC8H8\tH" );
	// ui->comboBoxMaterials->addItem( "Jeffamine(D230)", "CH3CH(NH2)CH2NH2\tOCH2CH(CH3)\tH" );
	// ui->comboBoxMaterials->addItem( "Sulfa drug (311)", "C12H14N4O4S\t\tH" );
	// ui->comboBoxMaterials->addItem( "AgilentTOF Mix(+)", "AgilentTOF Mix(+)" );
	// ui->comboBoxMaterials->addItem( "AgilentTOF Mix(-)", "AgilentTOF Mix(-)" );
	// ui->comboBoxMaterials->addItem( "Anionic Surfactants 1(-)", "C12H26SO4\tC2H4O\t-H\t" ); // negative
	// ui->comboBoxMaterials->addItem( "Anionic Surfactants 2(-)", "C13H28SO4\tC2H4O\t-H\t" ); // negative only
	// ui->comboBoxMaterials->addItem( "Sodium acetate", "\tCH3COONa\tNa\t" ); //
}

void
MSReferenceTable::getContents( adcontrols::MSCalibrateMethod& m )
{
    QStandardItemModel& model = *model_;    

    m.references().clear();

    int nRows = model.rowCount();
    for ( int row = 0; row < nRows; ++row ) {

        std::pair< std::wstring, std::wstring > adducts;
        std::wstring value = model.data( model.index( row, c_formula ), Qt::EditRole ).toString().toStdWString();
        std::wstring formula = adcontrols::ChemicalFormula::splitFormula( adducts, value, false );
        std::wstring adduct_string = adcontrols::ChemicalFormula::make_adduct_string( adducts );

        double exactMass = model.data( model.index( row, c_exact_mass ), Qt::EditRole ).toDouble();
        bool enable = model.data( model.index( row, c_formula ), Qt::CheckStateRole ).toBool();
        int charge = model.data( model.index( row, c_charge ) ).toInt();
        std::wstring description = model.data( model.index( row, c_description ) ).toString().toStdWString();

        m.references() << adcontrols::MSReference( formula, true, adduct_string, enable, exactMass, charge, description );
    }    
}

void
MSReferenceTable::setContents( const adcontrols::MSCalibrateMethod& m )
{
    QStandardItemModel& model = *model_;

    const adcontrols::MSReferences& references = m.references();
    int nRows = static_cast<int>( references.size() );
    if ( nRows < model.rowCount() )
        model.removeRows( 0, model.rowCount() ); // make sure all clear

    model.setRowCount( nRows + 1 ); // be sure last empty line
    int row = 0;
    for ( auto& ref: references ) {
		std::wstring formula = ref.display_formula();
        
		model.setData( model.index( row, c_formula ),     QString::fromStdWString( formula ) );
		model.setData( model.index( row, c_exact_mass ),  ref.exact_mass() );
		model.setData( model.index( row, c_enable ),      ref.enable() );

        if ( QStandardItem * chk = model.itemFromIndex( model.index( row, c_enable ) ) ) {
            chk->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
            chk->setEditable( true );
            chk->setData( ref.enable() ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
        }
        model.setData( model.index( row, c_description ), QString::fromStdWString( ref.description() ) );
		model.setData( model.index( row, c_charge ), ref.charge_count() );
        ++row;
    }

    resizeColumnsToContents();
    resizeRowsToContents();
}

void
MSReferenceTable::handleValueChanged( const QModelIndex& index )
{
    QStandardItemModel& model = *model_;

    if ( index.column() == c_formula ) {
        adcontrols::ChemicalFormula formula_parser;
        
        std::pair< std::string, std::string > adducts;
        std::string formula = adcontrols::ChemicalFormula::splitFormula( adducts, index.data( Qt::EditRole ).toString().toStdString(), false );
        
        double mass = formula_parser.getMonoIsotopicMass( formula, adducts );
        model.setData( model.index( index.row(), c_exact_mass ), mass );
    }
}
