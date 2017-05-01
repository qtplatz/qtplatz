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
#include <adcontrols/moltable.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/mscalibratemethod.hpp>
#include <qtwrapper/font.hpp>
#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QMimeData>
#include <QMenu>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QStandardItemModel>
#include <functional>
#include <sstream>

namespace adwidgets {

    class MSReferenceTable::delegate : public QStyledItemDelegate {
        
        std::function< void( const QModelIndex& ) > valueChanged_;

    public:

        delegate( std::function< void( const QModelIndex& ) > f ) {
            valueChanged_ = f;
        }
        
        void paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
            
            QStyleOptionViewItem opt(option);
            initStyleOption( &opt, index );
            opt.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
            
            if ( index.column() == c_formula ) {
                
                std::string formula = adcontrols::ChemicalFormula::formatFormulae( index.data().toString().toStdString() );
                DelegateHelper::render_html2( painter, opt, QString::fromStdString( formula ) );
                
            } else if ( index.column() == c_exact_mass ) {
                
                QTextOption o;
                o.setAlignment( Qt::AlignRight | Qt::AlignHCenter );
                QString text = QString::number( index.data( Qt::EditRole ).toDouble(), 'g', 9 );
                painter->drawText( option.rect, text, o );
                
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

    };

    class MSReferenceTable::impl {
    public:
        static double toMass( const QString& formula ) {
            std::vector< std::pair<std::string, char> > formulae = adcontrols::ChemicalFormula::split( formula.toStdString() );
            return adcontrols::ChemicalFormula().getMonoIsotopicMass( formulae );
        }
        
        static double toMass( const QModelIndex& index ) {
            std::pair< std::string, std::string > adducts;
            return toMass( index.data( Qt::EditRole ).toString() );
        }

    };
    
}

using namespace adwidgets;

MSReferenceTable::MSReferenceTable(QWidget *parent) : TableView(parent)
                                                    , model_( new QStandardItemModel )
{
    setModel( model_ );
    setItemDelegate( new delegate( [this]( const QModelIndex& idx ){ handleValueChanged( idx ); } ) );

    connect( this, &TableView::rowsDeleted, [this]() {
            if ( model_->rowCount() == 0 )
                model_->setRowCount( 1 );                
        });

    setContextMenuPolicy( Qt::CustomContextMenu );
    connect( this, &MSReferenceTable::customContextMenuRequested, this, &MSReferenceTable::handleContextMenu ); 
    
    model_->setRowCount( 1 );
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
}

void
MSReferenceTable::getContents( adcontrols::MSCalibrateMethod& m )
{
    QStandardItemModel& model = *model_;    

    m.references().clear();

    int nRows = model.rowCount();
    for ( int row = 0; row < nRows; ++row ) {

        auto formulae = adcontrols::ChemicalFormula::split( model.data( model.index( row, c_formula ), Qt::EditRole ).toString().toStdString() );
        auto adducts = adcontrols::ChemicalFormula::make_adduct_string( formulae );
        auto formula = adcontrols::ChemicalFormula::make_formula_string( formulae );

        if ( !formula.empty() ) {

            double exactMass = model.data( model.index( row, c_exact_mass ), Qt::EditRole ).toDouble();
            bool enable = model.data( model.index( row, c_enable ), Qt::CheckStateRole ).toBool();
            int charge = model.data( model.index( row, c_charge ) ).toInt();
            std::wstring description = model.data( model.index( row, c_description ) ).toString().toStdWString();
            
            m.references() << adcontrols::MSReference( formula.c_str(), true, adducts.c_str(), enable, exactMass, charge, description.c_str() );
        }
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
    for ( auto& ref: references )
        addReference( ref, row++ );

    resizeColumnsToContents();
    resizeRowsToContents();
}

void
MSReferenceTable::addReference( const adcontrols::MSReference& ref, int row )
{
    QStandardItemModel& model = *model_;

    std::wstring formula = ref.display_formula();
        
    model.setData( model.index( row, c_formula ),     QString::fromStdWString( formula ) );
    model.setData( model.index( row, c_exact_mass ),  ref.exact_mass() );
    if ( auto item = model.item( row, c_exact_mass ) )
        item->setEditable( false );

    model.setData( model.index( row, c_enable ),      ref.enable() );

    if ( QStandardItem * chk = model.itemFromIndex( model.index( row, c_enable ) ) ) {
        chk->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
        chk->setData( ref.enable() ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
    }
    model.setData( model.index( row, c_description ), QString::fromStdWString( ref.description() ) );
    model.setData( model.index( row, c_charge ), ref.charge_count() );
}

void
MSReferenceTable::handleValueChanged( const QModelIndex& index )
{
    QStandardItemModel& model = *model_;

    if ( index.column() == c_formula ) {
        double mass = impl::toMass( index );
        model.setData( model.index( index.row(), c_exact_mass ), mass );
        if ( auto item = model.item( index.row(), c_exact_mass ) )
            item->setEditable( false );

        model.setData( model.index( index.row(), c_charge ), 1 );
        if ( auto item = model.item( index.row(), c_exact_mass ) )
            item->setEditable( false );
        
        if ( mass > 0.9 && ( index.row() == model.rowCount() - 1 ) )
            model.insertRow( model.rowCount() );
    }
}

void
MSReferenceTable::handleAddReference( const adcontrols::MSReference& ref )
{
    QStandardItemModel& model = *model_;
    
    int row = model.rowCount();
    model.insertRow( row );
    addReference( ref, row - 1 );
}

void
MSReferenceTable::handleContextMenu(const QPoint &pt)
{
    QMenu menu;

    addActionsToMenu( menu, pt );
    
    struct action_type { QAction * first; std::function<void()> second; };
        
    action_type actions [] = {
        { menu.addAction("Clear"), [&]() {
                model_->setRowCount(0);
                model_->insertRow( 0 );
            }
        }
        , { menu.addAction("Delete line(s)"), [=](){ 
                handleDeleteSelection();
                if ( model_->rowCount() == 0 || 
                     !model_->index( model_->rowCount() - 1, c_formula ).data( Qt::EditRole ).toString().isEmpty() ) {
                    model_->insertRow( model_->rowCount() );
                }
            }
        }
    };

    if ( indexAt( pt ).isValid() ) {

        if ( QAction * selected = menu.exec( mapToGlobal( pt ) ) ) {
            for ( auto& i: actions )  {
                if ( i.first == selected ) {
                    i.second();
                    return;
                }
            }
        }
    }
}

void
MSReferenceTable::handlePaste()
{
    QString pasted = QApplication::clipboard()->text();
	QStringList texts = pasted.split( "\n" );

    int row = model_->rowCount() - 1;

    auto md = QApplication::clipboard()->mimeData();
    auto data = md->data( "application/moltable-xml" );
    if ( !data.isEmpty() ) {
        QString utf8( QString::fromUtf8( data ) );
        std::wistringstream is( utf8.toStdWString() );

        adcontrols::moltable molecules;
        if ( adcontrols::moltable::xml_restore( is, molecules ) ) {

            model_->setRowCount( row + int( molecules.data().size() + 1 ) ); // add one free line for add formula

            for ( auto& mol : molecules.data() ) {
                adcontrols::MSReference ref( mol.formula().c_str(), true, mol.adducts().c_str(), mol.enable(), 0.0, 1, mol.description().c_str() );
                addReference( ref, row++ );
            }
        }
    } else {
        
        for ( auto text : texts ) {
            auto formulae = adcontrols::ChemicalFormula::split( text.toStdString() );
            if ( !formulae.empty() && !formulae[ 0 ].first.empty() ) {
                adcontrols::MSReference ref( formulae[ 0 ].first.c_str(), true, adcontrols::ChemicalFormula::make_adduct_string( formulae ).c_str() );
                model_->insertRow( row );
                addReference( ref, row );
                ++row;
            }
        }
        
    }
    resizeRowsToContents();
    resizeColumnsToContents();
}
