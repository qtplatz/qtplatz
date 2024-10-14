/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "csvtable.hpp"
#include <QtCore/qnamespace.h>
#include <QtGui/qstandarditemmodel.h>

namespace figshare {

    class CSVTable::impl {
    public:
        std::vector< adportable::csv::list_string_type > vlist_;
    };

}

using namespace figshare;

CSVTable::CSVTable( QWidget *parent ) : TableView( parent )
                                      , impl_( new impl{} )
{
    auto model = new QStandardItemModel{};
    model->setColumnCount( 2 );
    model->setRowCount( 1 );
    this->setModel( model );
}

CSVTable::~CSVTable()
{
    delete impl_;
}

void
CSVTable::setData( const std::vector< adportable::csv::list_string_type >& vlist )
{
    impl_->vlist_ = vlist;

    if ( not vlist.empty() ) {
        auto model = qobject_cast< QStandardItemModel * >( this->model() );
        model->setColumnCount( vlist.at(0).size() );
        model->setRowCount( vlist.size() );

        size_t row{0};
        for ( const auto& alist: vlist ) {
            size_t col{0};
            for ( const auto& value: alist ) {
                model->setData( model->index(row, col), QString::fromStdString( std::get<1>(value) ), Qt::EditRole );
                ++col;
            }
            ++row;
        }
    }

    this->resizeColumnsToContents();
}
