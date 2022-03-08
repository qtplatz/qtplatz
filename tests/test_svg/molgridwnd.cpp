/**************************************************************************
** Copyright (C) 2022-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "molgridwnd.hpp"
#include "document.hpp"
#include <adcontrols/chemicalformula.hpp>
#include <adportable/debug.hpp>
#include <QHeaderView>
#include <QMenu>
#include <QProgressBar>
#include <QShortcut>
#include <QSignalBlocker>
#include <QStandardItemModel>
#include <QSvgWidget>
#include <QTableWidget>
#include <QUrl>
#include <QVBoxLayout>
#include <algorithm>

class MolGridWnd::impl {
public:
    // QGridLayout * grid_;
    QTableWidget * table_;
    int index_;
    impl() : table_( new QTableWidget ) {
    }
};


namespace {
    struct GridIndex {
        std::tuple< size_t, size_t > addr; // row, column
        std::tuple< size_t, size_t > size; // rowSize, columnSize
        GridIndex( size_t r = 0, size_t c = 1 ) : size{ r, c } {}
        GridIndex& operator ++() {
            if ( std::get<1>(addr) < std::get<1>(size) )
                std::get<1>(addr)++;
            if ( std::get<1>(addr) == std::get<1>(size) ) {
                std::get<1>(addr) = 0;
                std::get<0>(addr)++;
            }
            return *this;
        }
        inline size_t row() const { return std::get<0>( addr ); };
        inline size_t column() const { return std::get<1>( addr ); };
    };
}

MolGridWnd::~MolGridWnd()
{
}

MolGridWnd::MolGridWnd(QWidget *parent) : QWidget( parent )
                                        , impl_( std::make_unique< impl >() )
{
    if ( auto layout = new QVBoxLayout( this ) ) {
        layout->setSpacing( 0 );
        layout->setMargin( 0 );
        layout->addWidget( impl_->table_ );

        impl_->table_->setRowCount( 1 );
        impl_->table_->setColumnCount( 1 );
        impl_->table_->verticalHeader()->hide();
        impl_->table_->horizontalHeader()->hide();
        // auto sz = std::min( impl_->table_->width(), impl_->table_->height() );
        impl_->table_->horizontalHeader()->setDefaultSectionSize( impl_->table_->width()  );
        impl_->table_->verticalHeader()->setDefaultSectionSize( impl_->table_->height() );
    }
}

void
MolGridWnd::handleSVG( const QByteArray& ba )
{
    auto widget = new QSvgWidget();
    widget->load( ba );
    impl_->table_->setCellWidget( 0, 0, widget );
}
