/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "peptidetable.hpp"
#include <adprot/digestedpeptides.hpp>
#include <adprot/peptides.hpp>
#include <adprot/peptide.hpp>
#include <QStandardItemModel>
#include <QHeaderView>

using namespace adwidgets;

PeptideTable::PeptideTable(QWidget *parent) :  TableView(parent)
                                                , model_( new QStandardItemModel() )
{
    setModel( model_ );
	model_->setColumnCount( 3 );
    model_->setRowCount( 1 );
}

PeptideTable::~PeptideTable()
{
    delete model_;
}

void
PeptideTable::onInitialUpdate()
{
    QStandardItemModel& model = *model_;
    
    model.setColumnCount( 3 );

    model.setHeaderData( 0,        Qt::Horizontal, QObject::tr( "peptide" ) );
    model.setHeaderData( 1,        Qt::Horizontal, QObject::tr( "formula" ) );
    model.setHeaderData( 2,  Qt::Horizontal, QObject::tr( "mass" ) );

    resizeColumnsToContents();
    resizeRowsToContents();

    horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );
}

void
PeptideTable::setContents( const adprot::digestedPeptides& digested )
{
    QStandardItemModel& model = *model_;
    const adprot::peptides& peptides = digested.peptides();

    model.setRowCount( static_cast< int >( peptides.size() ) );

    int row = 0;
    for ( auto& p: peptides ) {
        model.setData( model.index( row, 0 ), QString::fromStdString( p.sequence() ) );
        model.setData( model.index( row, 1 ), QString::fromStdString( p.formula() ) );
        model.setData( model.index( row, 2 ), p.mass() );
		++row;
    }
}

