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
#include "candidate.hpp"
#include "document.hpp"
#include "molgridwnd.hpp"
#include "simple_mass_spectrum.hpp"
#include <adcontrols/chemicalformula.hpp>
#include <adportable/debug.hpp>
#include <QHeaderView>
#include <QMenu>
#include <QProgressBar>
#include <QSignalBlocker>
#include <QStandardItemModel>
#include <QSvgWidget>
#include <QTableWidget>
#include <QUrl>
#include <QVBoxLayout>
#include <algorithm>

using lipidid::MolGridWnd;

namespace lipidid {

    class MolGridWnd::impl {
    public:
        // QGridLayout * grid_;
        QTableWidget * table_;
        int index_;
        std::weak_ptr< const simple_mass_spectrum > simple_mass_spectrum_;
        impl() : table_( new QTableWidget ) {
        }
    };
}

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
        impl_->table_->verticalHeader()->hide();
        impl_->table_->horizontalHeader()->hide();
    }
}

void
MolGridWnd::handleMatchedSelection( int index )
{
    auto [ ms, ref, simple_mass_spectrum ] = document::instance()->getResultSet();
    if ( simple_mass_spectrum ) {
        // data equality check
        if ( ( index == impl_->index_ ) &&
             ( simple_mass_spectrum == impl_->simple_mass_spectrum_.lock() ) )
            return;

        impl_->simple_mass_spectrum_ = simple_mass_spectrum;
        impl_->index_ = index;

        const auto& candidates = simple_mass_spectrum->candidates( index );
        size_t nmols = std::accumulate( candidates.begin(), candidates.end(), 0, [](const auto& a, const auto& b){ return a + b.inchiKeys().size(); } );
        if ( nmols == 0 )
            return;
        auto width  = impl_->table_->width();
        int aspect_ratio = std::max( 1, int(0.5 + double( impl_->table_->height() ) / double(width) ) );
        size_t columns = nmols <= aspect_ratio ? 1 : std::sqrt(double(nmols) / aspect_ratio) + 0.5;
        size_t rows    = nmols / columns;
        if ( rows * columns < nmols )
            ++rows;
        GridIndex gidx( rows, columns );

        // ADDEBUG() << "nmols: " << nmols << ", " << gidx.size;
        impl_->table_->horizontalHeader()->setDefaultSectionSize( width / columns );
        impl_->table_->verticalHeader()->setDefaultSectionSize( rows == 1 ? width / columns : impl_->table_->height() / rows );

        impl_->table_->setRowCount( rows );
        impl_->table_->setColumnCount( columns );

        for ( const auto& candidate: candidates ) {
            for ( const auto& key: candidate.inchiKeys() ) {
                if ( auto svg = document::instance()->find_svg( key ) ) {
                    auto widget = new QSvgWidget();
                    widget->load( QByteArray( svg->data(), svg->size() ) );
                    impl_->table_->setCellWidget( gidx.row(), gidx.column(), widget );
                } else {
                    impl_->table_->setCellWidget( gidx.row(), gidx.column(), new QSvgWidget ); // make it empty
                }
                ++gidx;
            }
        }
    } else {
        impl_->index_ = (-1);
    }
}
