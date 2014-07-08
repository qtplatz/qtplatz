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

#include "targetingadducts.hpp"
#include "delegatehelper.hpp"
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/targetingmethod.hpp>
#include <QStyledItemDelegate>
#include <QStandardItemModel>
#include <array>

using namespace adwidgets;

namespace adwidgets {
    namespace detail {

        enum eRootColumns {
            c_header
            , nbrColumns
        };

        enum eSubColumns {
            c_formula
            , nbrChildColumns
        };

        enum {
            r_pos_adducts
            , r_neg_adducts
            , nbrRows
        };

        class AdductsDelegate : public QStyledItemDelegate {
        public:
            void paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
                if ( index.parent() != QModelIndex() && index.column() == c_formula ) {
                    
                    QStyleOptionViewItem opt = option;
                    initStyleOption( &opt, index );
                    std::string formula = adcontrols::ChemicalFormula::formatFormulae( index.data().toString().toStdString() );
                    DelegateHelper::render_html2( painter, opt, QString::fromStdString( formula ) );
                } else {
                    QStyledItemDelegate::paint( painter, option, index );
                }
            }
        };

    }
}

TargetingAdducts::~TargetingAdducts()
{
    delete delegate_;
    delete model_;
}

TargetingAdducts::TargetingAdducts(QWidget *parent) : QTreeView(parent)
                                                    , model_( new QStandardItemModel )
                                                    , delegate_( new detail::AdductsDelegate )
{
    setModel( model_ );
    setItemDelegate( delegate_ );
}

void
TargetingAdducts::getContents( adcontrols::TargetingMethod& method )
{
    using namespace adwidgets::detail;
	QStandardItemModel& model = *model_;

    std::array< std::pair< bool, int >, 2 > polarities = { { std::make_pair( true, r_pos_adducts ) 
                                                             , std::make_pair( false, r_neg_adducts ) } };

    for ( auto polarity: polarities ) {
        
        if ( auto parent = model.item( polarity.second, c_header ) ) {

            auto& adducts = method.adducts( polarity.first );
            adducts.clear();

            for ( int row = 0; row < parent->rowCount(); ++row ) {
                if ( auto item = model.itemFromIndex( model.index( row, c_header, parent->index() ) ) ) {
                    bool enable = item->data( Qt::CheckStateRole ).toBool();
                    std::string adduct = item->data( Qt::EditRole ).toString().toStdString();
                    if ( !adduct.empty() )
                        adducts.push_back( std::make_pair( enable, adduct ) );
                }
            }
        }
    }
}

void
TargetingAdducts::setContents( const adcontrols::TargetingMethod& method )
{
    using namespace adwidgets::detail;
	QStandardItemModel& model = *model_;

    std::array< std::pair< bool, int >, 2 > polarities = { { std::make_pair( true, r_pos_adducts )
                                                             , std::make_pair( false, r_neg_adducts ) } };

    for ( auto polarity: polarities ) {

        if ( auto parent = model.item( polarity.second, c_header ) ) {

            parent->setRowCount( int( method.adducts( polarity.first ).size() + 1 ) ); // add empty line at the bottom of table
            parent->setEditable( false );
            parent->setColumnCount( nbrChildColumns );

            int row = 0;
            for ( auto& adduct: method.adducts( polarity.first ) ) {
                if ( auto item = model.itemFromIndex( model.index( row, c_formula, parent->index() ) ) ) {
                    // check box
                    item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
                    item->setEditable( true );
                    item->setData( adduct.first ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
                    // formula
                    item->setData( QString::fromStdString( adduct.second ), Qt::EditRole );
                }
                ++row;
            }
        }
    }

    expandAll();
}

void
TargetingAdducts::OnInitialUpdate()
{
    using namespace adwidgets::detail;
	QStandardItemModel& model = *model_;

    QStandardItem * rootNode = model.invisibleRootItem();
    rootNode->setColumnCount( nbrColumns );

    model.setRowCount( nbrRows );
    
    model.setData( model.index( r_pos_adducts, c_header ),  "Adducts(pos)" );
    model.setData( model.index( r_neg_adducts, c_header ),  "Adducts(neg)" );

}
