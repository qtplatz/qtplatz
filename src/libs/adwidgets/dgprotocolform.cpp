/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#include "adwidgets_global.hpp"
#include "dgprotocolform.hpp"
#include <adio/dgprotocol.hpp>
#include <adwidgets/htmlheaderview.hpp>
#include <adwidgets/tableview.hpp>
#include <qtwrapper/make_widget.hpp>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QStandardItemModel>
#include <QTabWidget>
#include <QWidget>

using namespace adwidgets;

dgProtocolForm::~dgProtocolForm()
{
}

dgProtocolForm::dgProtocolForm( QWidget * parent ) : QWidget( parent )
{
    setProperty( "protocol", true );
    setObjectName( "dgProtocolForm" );

    resize( 100, 200 );

    QStringList list;
    list << "CH.1" << "CH.2" << "CH.3" << "CH.4" << "CH.5" << "CH.6" << "CH.7";

    if ( auto layout = new QVBoxLayout( this ) ) {

        if ( auto table = qtwrapper::make_widget< adwidgets::TableView >("protocol") ) {

            table->verticalHeader()->setFixedWidth( 24 * 4 );
            
            table->setModel( new QStandardItemModel() );
            table->setHorizontalHeader( new adwidgets::HtmlHeaderView() );
            layout->addWidget( table );

            if ( auto model = qobject_cast< QStandardItemModel *>(table->model()) ) {
            	model->setColumnCount(2);
            	model->setHeaderData( 0, Qt::Horizontal, tr("Delay(&mu;s)"));
            	model->setHeaderData( 1, Qt::Horizontal, tr("Width(&mu;s)"));
            }
            setVerticalHeader( list );
        }
    }
}

void
dgProtocolForm::setProtocol( size_t replicates, const std::vector< std::pair< double, double > >& pulses )
{
     if ( auto table = findChild< QTableView * >() ) {
         if ( auto model = table->model() ) {
             int row = 0;
             for ( auto& pulse: pulses ) {
                 model->setData( model->index( row, 0 ), (pulse.first * 1.0e6), Qt::EditRole );
                 model->setData( model->index( row, 1 ), (pulse.second * 1.0e6), Qt::EditRole );
                 ++row;
             }
             model->setData( model->index( row, 0 ), int(replicates), Qt::EditRole );
         }
         table->resizeColumnsToContents();
         table->resizeRowsToContents();
     }

}

void
dgProtocolForm::setVerticalHeader( const QStringList& list )
{
    vHeader_ = list;
    if ( auto table = findChild< adwidgets::TableView * >("protocol") ) {
        if ( auto model = qobject_cast< QStandardItemModel * >( table->model() ) ) {
            model->setRowCount( vHeader_.count() + 1 ); // +1 for replicates
            int row = 0;
            for ( auto& label: vHeader_ )
                model->setHeaderData( row++, Qt::Vertical, label );
            model->setHeaderData( vHeader_.count(), Qt::Vertical, tr( "Replicates(N)" ) );
        }
    }
}
