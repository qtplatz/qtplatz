/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "lapdeconvdlg.hpp"
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/mspeak.hpp>
#include <adcontrols/mspeaks.hpp>
#include <adportable/debug.hpp>
#include <adwidgets/moltable.hpp>
#include <qtwrapper/make_widget.hpp>
#include <QAbstractItemModel>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QMenu>
#include <QMimeData>
#include <QPushButton>
#include <QSplitter>
#include <QStandardItemModel>
#include <boost/uuid/uuid.hpp>
#include <memory>

namespace dataproc {

    class lapDeconvDlg::impl {
    public:
        impl() : selRow_( 0 ) {
        };
        int selRow_;
    };

}

using namespace dataproc;

namespace {
    enum columns {
        c_mass
        , c_nlaps
        , c_error
    };
}

lapDeconvDlg::lapDeconvDlg(QWidget *parent) : QDialog(parent)
                                            , impl_( std::make_unique< impl >() )
{
    if ( QVBoxLayout * layout = new QVBoxLayout( this ) ) {
        if ( auto table = qtwrapper::make_widget< adwidgets::TableView >( "lapMolTable" ) ) {
            table->setSelectionBehavior( QAbstractItemView::SelectRows );
            table->setSelectionMode( QAbstractItemView::SingleSelection );
            auto model = new QStandardItemModel();
            table->setModel( model );
            model->setColumnCount( 3 );
            model->setHeaderData( c_mass, Qt::Horizontal, QObject::tr( "mass" ) );
            model->setHeaderData( c_nlaps, Qt::Horizontal, QObject::tr( "lap#" ) );
            model->setHeaderData( c_error, Qt::Horizontal, QObject::tr( "error (mDa)" ) );
            layout->addWidget( table );
        }
        if ( auto buttons = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel ) ) {
            connect( buttons, &QDialogButtonBox::accepted, this, &QDialog::accept );
            connect( buttons, &QDialogButtonBox::rejected, this, &QDialog::reject );

            layout->addWidget( buttons );
        }
    }
    resize( 400, 340 );
}

lapDeconvDlg::~lapDeconvDlg()
{
}

void
lapDeconvDlg::setData( const std::vector< std::tuple< double, int, double > >& candidates )
{
    if ( auto table = findChild< adwidgets::TableView * >( "lapMolTable" ) ) {
        if ( auto model = qobject_cast< QStandardItemModel * >( table->model() ) ) {
            model->setRowCount( candidates.size() );
            size_t row(0);
            for ( const auto& c: candidates ) {
                using adwidgets::MolTable;
                double mass, error; int lap;
                std::tie( mass, lap, error ) = c;
                model->setData( model->index( row, c_mass ), mass );
                model->setData( model->index( row, c_nlaps ), lap );
                model->setData( model->index( row, c_error ), error * 1000 );
                ++row;
            }
        }
    }

}

void
lapDeconvDlg::setList( const std::vector< std::tuple< double, int > >& list )
{
    if ( auto table = findChild< adwidgets::TableView * >( "lapMolTable" ) ) {
        if ( auto model = table->model() ) {
            model->insertRows( 0, list.size() );
            int row(0);
            for ( const auto& i: list ) {
                model->setData( model->index( row, c_mass ), std::get<0>( i ) );
                model->setData( model->index( row, c_nlaps ), std::get<1>( i ) );
                ++row;
            }
        }
    }
}

boost::optional< std::tuple< double, int, double > >
lapDeconvDlg::getSelection() const
{
    if ( auto table = findChild< adwidgets::TableView * >( "lapMolTable" ) ) {
        auto select = table->selectionModel();
        if ( select->hasSelection() ) {
            auto rows = select->selectedRows();
            if ( !rows.empty() ) {
                int row = rows.front().row();
                auto model = table->model();
                double mass = model->index( row, c_mass ).data( Qt::EditRole ).toDouble();
                int lap = model->index( row, c_nlaps ).data( Qt::EditRole ).toInt();
                double error = model->index( row, c_error ).data( Qt::EditRole ).toDouble() / 1000;
                return {{ mass, lap, error }};
            }
        }
    }
    return {};
}

//////////////
