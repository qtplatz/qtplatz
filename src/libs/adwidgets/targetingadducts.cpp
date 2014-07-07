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
#include <adcontrols/targetingmethod.hpp>
#include <QStyledItemDelegate>
#include <QStandardItemModel>
#include <array>

using namespace adwidgets;

namespace adwidgets {
    namespace detail {

        enum {
            c_header
            , nbrColums
        };

        enum {
            r_pos_adducts
            , r_neg_adducts
            , nbrRows
        };

        class AdductsDelegate : public QStyledItemDelegate {
        public:
            
        };

    }
}

TargetingAdducts::TargetingAdducts(QWidget *parent) : QTreeView(parent)
                                                    , model_( new QStandardItemModel )
                                                    , delegate_( new detail::AdductsDelegate )
{
    setModel( model_.get() );
    setItemDelegate( delegate_.get() );
}

void
TargetingAdducts::getContents( adcontrols::TargetingMethod& method )
{
    using namespace adwidgets::detail;
	QStandardItemModel& model = *model_;

    std::array< bool, 2 > polarities = { true, false };
    int n = 0;

    for ( auto polarity: polarities ) {
        
        if ( auto parent = model.item( r_pos_adducts + n, c_header ) ) {

            auto& adducts = method.adducts( polarity );
            adducts.clear();

            for ( int row = 0; parent->rowCount(); ++row ) {
                if ( auto item = model.itemFromIndex( model.index( row, 0, parent->index() ) ) ) {
                    std::string adduct = item->data( Qt::EditRole ).toString().toStdString();
                    bool enable = item->data( Qt::CheckStateRole ).toBool();
                    if ( !adducts.empty() ) {
                        adducts.push_back( std::make_pair( enable, adduct ) );
                    }
                }
            }
        }
        ++n;
    }
}

void
TargetingAdducts::setContents( const adcontrols::TargetingMethod& method )
{
    using namespace adwidgets::detail;
	QStandardItemModel& model = *model_;

    std::array< bool, 2 > polarities = { true, false };
    int n = 0;

    for ( auto polarity: polarities ) {

        if ( auto parent = model.item( r_pos_adducts + n, c_header ) ) {

            parent->setRowCount( int( method.adducts( polarity ).size() + 1 ) ); // add empty line at the bottom of table
            parent->setEditable( false );
            parent->setColumnCount( 1 );

            int row = 0;
            for ( auto& adduct: method.adducts( polarity ) ) {
                if ( auto item = model.itemFromIndex( model.index( row, 0, parent->index() ) ) ) {
                    item->setData( QString::fromStdString( adduct.second ), Qt::EditRole );
                    item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
                    item->setEditable( true );
                    item->setData( adduct.first ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
                }
                ++row;
            }
        }
        ++n;
    }

    expandAll();
}

void
TargetingAdducts::OnInitialUpdate()
{
    using namespace adwidgets::detail;
	QStandardItemModel& model = *model_;

    QStandardItem * rootNode = model.invisibleRootItem();
    rootNode->setColumnCount( nbrColums );

	// model.setHeaderData( c_header, Qt::Horizontal, "formula" );
	// model.setHeaderData( c_value, Qt::Horizontal, "Value" );

    model.setRowCount( nbrRows );
    
    model.setData( model.index( r_pos_adducts, c_header ),  "Adducts(pos)" );
    model.setData( model.index( r_neg_adducts, c_header ),  "Adducts(neg)" );

}
