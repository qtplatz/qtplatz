/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "datareaderchoicedialog.hpp"
#include "tableview.hpp"
#include <adcontrols/datareader.hpp>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QStandardItemModel>
#include <boost/uuid/uuid.hpp>
#include <stdexcept>
#include <sstream>

using namespace adwidgets;

DataReaderChoiceDialog::DataReaderChoiceDialog( QWidget *parent ) : QDialog( parent )
{
    if ( auto layout = new QVBoxLayout( this ) ) {
        auto widget = new TableView( this );
        
        layout->addWidget( widget );
        layout->addWidget( new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel ) );
    }
    
}

DataReaderChoiceDialog::DataReaderChoiceDialog( std::vector< std::shared_ptr< const adcontrols::DataReader > >& readers
                                                , QWidget * parent ) : QDialog( parent )
{
    if ( auto layout = new QVBoxLayout( this ) ) {
        auto table = new TableView( this );
        auto model = new QStandardItemModel();

        model->setColumnCount( 2 );
        model->setRowCount( int( readers.size() ) );
        for ( int row = 0; row < int( readers.size() ); ++row ) {
            const auto& reader = readers[ row ];
            model->setData( model->index( row, 0 ), QString::fromStdString( reader->display_name() ) );
            model->setData( model->index( row, 1 ), QString::fromStdString( reader->objtext() ) );
        }
        table->setModel( model );
        table->setSelectionBehavior( QAbstractItemView::SelectRows );
        table->resizeColumnsToContents();
        table->resizeRowsToContents();
        table->horizontalHeader()->setStretchLastSection( true );
        layout->addWidget( table );
        auto buttons = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
        connect( buttons, &QDialogButtonBox::accepted, this, [&] () { QDialog::accept(); } );
        connect( buttons, &QDialogButtonBox::rejected, this, [&] () { QDialog::reject(); } );
        layout->addWidget( buttons );
        adjustSize();
    }
    
}

int
DataReaderChoiceDialog::currentSelection() const
{
    if ( auto table = findChild< QTableView * >() ) {
         return table->currentIndex().row();
    }
    return 0;
}

