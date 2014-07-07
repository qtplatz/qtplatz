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

#include "adductslosetree.hpp"
#include <adcontrols/targetingmethod.hpp>
#include <QStyledItemDelegate>
#include <QStandardItemModel>

using namespace adwidgets;

namespace adwidgets {
    namespace detail {

        enum {
            c_header
            , c_value
            , nbrColums
        };

        enum {
            r_polarity  // {pos|neg}
            , r_adducts
            , r_lose
            , nbrRows
        };

        class ItemDelegate : public QStyledItemDelegate {
        public:
            
        };

    }
}

AdductsLoseTree::AdductsLoseTree(QWidget *parent) : QTreeView(parent)
                                                  , model_( new QStandardItemModel )
                                                  , delegate_( new detail::ItemDelegate )
{
    setModel( model_.get() );
    setItemDelegate( delegate_.get() );
}

void
AdductsLoseTree::getContents( adcontrols::TargetingMethod& )
{
}

void
AdductsLoseTree::OnInitialUpdate()
{
    using namespace adwidgets::detail;

	QStandardItemModel& model = *model_;

    QStandardItem * rootNode = model.invisibleRootItem();
    rootNode->setColumnCount( nbrColums );

	model.setHeaderData( c_header, Qt::Horizontal, "Function" );
	model.setHeaderData( c_value, Qt::Horizontal, "Value" );

    model.setRowCount( nbrRows );
    
    model.setData( model.index( r_polarity, c_header ), "Polarity" );
    model.setData( model.index( r_adducts, c_header ),  "Adducts" );
    model.setData( model.index( r_lose, c_header ),     "Lose" );

    // do {
	// 	QStandardItem * adducts = new QStandardItem( "Adducts" );
	// 	adducts->setEditable( false );
    //     rootNode->appendRow( adducts );
	// 	StandardItemHelper::appendRow( adducts, "H", "H" );
	// } while( 0 );

    // do {
	// 	QStandardItem * loss = new QStandardItem( "Loss" );
	// 	loss->setEditable( false );
	// 	rootNode->appendRow( loss );
	// } while( 0 );
}
